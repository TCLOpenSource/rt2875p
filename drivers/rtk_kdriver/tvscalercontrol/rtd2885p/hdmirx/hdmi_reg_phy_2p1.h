//#include <rbus/hdmirx_2p1_phy_reg.h>
#include <rbus/efuse_reg.h>

//////////////////////
//Copy from Mac8p

//#include <rbus/hdmirx_2p0_phy_reg.h>  //From Mac7p need replace TBD
//#include <rbus/gdirx_hdmi20_phy_p0_reg.h>  //Mac8p
#include <rbus/hdmirx_phy_reg.h>  //Mac8p-> Merlin8

//#include <rbus/gdirx_hdmi20_dp14_phy_p1_regNew.h>  //Mac8p
//#include <rbus/gdirx_hdmi20_dp14_phy_p1_regNew.h>  //Mac8p


//////////////////////
//Copy from Mark2
#define HDMI_APHY0_BASE	    (0xB800DA04)   //DA04 ~ DAC0 (P0)   
#define HDMI_APHY1_BASE	    (0xB803BA04) 
#define HDMI_APHY2_BASE	    (0xB803CA04)   
#define HDMI_APHY3_BASE	    (0xB80B2A04) 

#define HDMI_DPHY0_BASE	    (0xB800DB00)   //DAFC ~ DD84 (P0) 
#define HDMI_DPHY1_BASE	    (0xB803BB00)  
#define HDMI_DPHY2_BASE	    (0xB803CB00)    
#define HDMI_DPHY3_BASE	    (0xB80B2B00)   

#define HDMI_APHY0_RSTB_BASE	    (0xB800DAFC) 
#define HDMI_APHY1_RSTB_BASE	    (0xB803BAFC) 
#define HDMI_APHY2_RSTB_BASE	    (0xB803CAFC) 
#define HDMI_APHY3_RSTB_BASE	    (0xB80B2AFC) 


struct hdmi21_aphy_reg_st {
	uint32_t z0k;  //BA04
	uint32_t z0pow;  //BA08
	uint32_t rsvd0;  //BA0C
	uint32_t top_in_0;  //BA10
	uint32_t top_in_1;  //BA14	
	uint32_t top_in_2;  //BA18	
	uint32_t rsvd1;  //BA1C	
	uint32_t top_out_0;  //BA20
	uint32_t top_out_1; //BA24
	uint32_t rsvd2;  //BA28
	uint32_t rsvd3;  //BA2C
	uint32_t ck0; //BA30
	uint32_t ck1; //BA34
	uint32_t ck2; //BA38
	uint32_t ck3; //BA3C
	uint32_t ck4; //BA40
	uint32_t acdr_ck0;  //BA44
	uint32_t acdr_ck1;  //BA48
	uint32_t acdr_ck2;  //BA4C
	uint32_t b0;  //BA50
	uint32_t b1;  //BA54
	uint32_t b2;  //BA58
	uint32_t b3;  //BA5C
	uint32_t b4;  //BA60
	uint32_t acdr_b0;  //BA64
	uint32_t acdr_b1;  //BA68
	uint32_t acdr_b2;  //BA6C
	uint32_t g0;  //BA70
	uint32_t g1;  //BA74
	uint32_t g2;  //BA78
	uint32_t g3;  //BA7C
	uint32_t g4;  //BA80
	uint32_t acdr_g0;  //BA84
	uint32_t acdr_g1;  //BA88
	uint32_t acdr_g2;  //BA8C
	uint32_t r0;  //BA90
	uint32_t r1;  //BA94
	uint32_t r2;  //BA98
	uint32_t r3;  //BA9C
	uint32_t r4;  //BAA0
	uint32_t acdr_r0;  //BAA4
	uint32_t acdr_r1;  //BAA8
	uint32_t acdr_r2;  //BAAC
	uint32_t rsvd4;  //BAB0
	uint32_t ck_cmu_1;  //BAB4;
	uint32_t ck_cmu_2;  //BAB8
	uint32_t ck_cmu_3;  //BABC
	uint32_t ck_cmu_4;  //BAC0
};

extern const struct hdmi21_aphy_reg_st *hd21_aphy[4];


///////////////////////////////////////////////////
#ifdef CONFIG_ARM64

#define HDMIRX_PHY_z0k_reg     	 	(unsigned int)(unsigned long)(&hd21_aphy[nport]->z0k)//BA04            
#define HDMIRX_PHY_z0pow_reg    	(unsigned int)(unsigned long)(&hd21_aphy[nport]->z0pow)//BA08	        
#define HDMIRX_PHY_top_in_0_reg 	(unsigned int)(unsigned long)(&hd21_aphy[nport]->top_in_0)//BA10       
#define HDMIRX_PHY_top_in_1_reg 	(unsigned int)(unsigned long)(&hd21_aphy[nport]->top_in_1)//BA14	      
#define HDMIRX_PHY_top_in_2_reg 	(unsigned int)(unsigned long)(&hd21_aphy[nport]->top_in_2)//BA18	      
#define HDMIRX_PHY_top_out_0_reg 	(unsigned int)(unsigned long)(&hd21_aphy[nport]->top_out_0)//BA20      
#define HDMIRX_PHY_top_out_1_reg 	(unsigned int)(unsigned long)(&hd21_aphy[nport]->top_out_1)//BA24       
#define HDMIRX_PHY_rsvd0_reg    		(unsigned int)(unsigned long)(&hd21_aphy[nport]->rsvd0)//BA28          
#define HDMIRX_PHY_rsvd1_reg		(unsigned int)(unsigned long)(&hd21_aphy[nport]->rsvd1)//BA2C          
#define HDMIRX_PHY_ck0_reg			(unsigned int)(unsigned long)(&hd21_aphy[nport]->ck0)//BA30             
#define HDMIRX_PHY_ck1_reg			(unsigned int)(unsigned long)(&hd21_aphy[nport]->ck1)//BA34             
#define HDMIRX_PHY_ck2_reg 		(unsigned int)(unsigned long)(&hd21_aphy[nport]->ck2)//BA38             
#define HDMIRX_PHY_ck3_reg 		(unsigned int)(unsigned long)(&hd21_aphy[nport]->ck3)//BA3C             
#define HDMIRX_PHY_ck4_reg			(unsigned int)(unsigned long)(&hd21_aphy[nport]->ck4)//BA40             
#define HDMIRX_PHY_acdr_ck0_reg	 (unsigned int)(unsigned long)(&hd21_aphy[nport]->acdr_ck0)//BA44       
#define HDMIRX_PHY_acdr_ck1_reg	 (unsigned int)(unsigned long)(&hd21_aphy[nport]->acdr_ck1)//BA48       
#define HDMIRX_PHY_acdr_ck2_reg 	(unsigned int)(unsigned long)(&hd21_aphy[nport]->acdr_ck2)//BA4C       
#define HDMIRX_PHY_b0_reg       		(unsigned int)(unsigned long)(&hd21_aphy[nport]->b0)//BA50             
#define HDMIRX_PHY_b1_reg       		(unsigned int)(unsigned long)(&hd21_aphy[nport]->b1)//BA54             
#define HDMIRX_PHY_b2_reg       		(unsigned int)(unsigned long)(&hd21_aphy[nport]->b2)//BA58             
#define HDMIRX_PHY_b3_reg       		(unsigned int)(unsigned long)(&hd21_aphy[nport]->b3)//BA5C             
#define HDMIRX_PHY_b4_reg      		 (unsigned int)(unsigned long)(&hd21_aphy[nport]->b4)//DA60             
#define HDMIRX_PHY_acdr_b0_reg  	(unsigned int)(unsigned long)(&hd21_aphy[nport]->acdr_b0)//BA64        
#define HDMIRX_PHY_acdr_b1_reg  	(unsigned int)(unsigned long)(&hd21_aphy[nport]->acdr_b1)//BA68        
#define HDMIRX_PHY_acdr_b2_reg 	 (unsigned int)(unsigned long)(&hd21_aphy[nport]->acdr_b2)//BA6C        
#define HDMIRX_PHY_g0_reg       		(unsigned int)(unsigned long)(&hd21_aphy[nport]->g0)//BA70             
#define HDMIRX_PHY_g1_reg       		(unsigned int)(unsigned long)(&hd21_aphy[nport]->g1)//BA74             
#define HDMIRX_PHY_g2_reg      		(unsigned int)(unsigned long)(&hd21_aphy[nport]->g2)//BA78             
#define HDMIRX_PHY_g3_reg       		(unsigned int)(unsigned long)(&hd21_aphy[nport]->g3)//BA7C             
#define HDMIRX_PHY_g4_reg			(unsigned int)(unsigned long)(&hd21_aphy[nport]->g4)//BA80             
#define HDMIRX_PHY_acdr_g0_reg  	(unsigned int)(unsigned long)(&hd21_aphy[nport]->acdr_g0)//BA84        
#define HDMIRX_PHY_acdr_g1_reg		(unsigned int)(unsigned long)(&hd21_aphy[nport]->acdr_g1)//BA88        
#define HDMIRX_PHY_acdr_g2_reg		(unsigned int)(unsigned long)(&hd21_aphy[nport]->acdr_g2)//BA8C        
#define HDMIRX_PHY_r0_reg			(unsigned int)(unsigned long)(&hd21_aphy[nport]->r0)//BA90             
#define HDMIRX_PHY_r1_reg			(unsigned int)(unsigned long)(&hd21_aphy[nport]->r1)//BA94             
#define HDMIRX_PHY_r2_reg			(unsigned int)(unsigned long)(&hd21_aphy[nport]->r2)//BA98             
#define HDMIRX_PHY_r3_reg			(unsigned int)(unsigned long)(&hd21_aphy[nport]->r3)//BA9C             
#define HDMIRX_PHY_r4_reg			(unsigned int)(unsigned long)(&hd21_aphy[nport]->r4)//BAA0             
#define HDMIRX_PHY_acdr_r0_reg		(unsigned int)(unsigned long)(&hd21_aphy[nport]->acdr_r0)//BAA4        
#define HDMIRX_PHY_acdr_r1_reg		(unsigned int)(unsigned long)(&hd21_aphy[nport]->acdr_r1)//BAA8        
#define HDMIRX_PHY_acdr_r2_reg		(unsigned int)(unsigned long)(&hd21_aphy[nport]->acdr_r2)//BAAC        
#define HDMIRX_PHY_rsvd2_reg		(unsigned int)(unsigned long)(&hd21_aphy[nport]->rsvd2)//BAB0          
#define HDMIRX_PHY_ck_cmu_1_reg	(unsigned int)(unsigned long)(&hd21_aphy[nport]->ck_cmu_1)//BAB4      
#define HDMIRX_PHY_ck_cmu_2_reg	(unsigned int)(unsigned long)(&hd21_aphy[nport]->ck_cmu_2)//BAB8       
#define HDMIRX_PHY_ck_cmu_3_reg	(unsigned int)(unsigned long)(&hd21_aphy[nport]->ck_cmu_3)//BABC       
#define HDMIRX_PHY_ck_cmu_4_reg	(unsigned int)(unsigned long)(&hd21_aphy[nport]->ck_cmu_4)//BAC0

#else

#define HDMIRX_PHY_z0k_reg     	 	(unsigned int)(&hd21_aphy[nport]->z0k)//BA04            
#define HDMIRX_PHY_z0pow_reg    	(unsigned int)(&hd21_aphy[nport]->z0pow)//BA08	        
#define HDMIRX_PHY_top_in_0_reg 	(unsigned int)(&hd21_aphy[nport]->top_in_0)//BA10       
#define HDMIRX_PHY_top_in_1_reg 	(unsigned int)(&hd21_aphy[nport]->top_in_1)//BA14	      
#define HDMIRX_PHY_top_in_2_reg 	(unsigned int)(&hd21_aphy[nport]->top_in_2)//BA18	      
#define HDMIRX_PHY_top_out_0_reg 	(unsigned int)(&hd21_aphy[nport]->top_out_0)//BA20      
#define HDMIRX_PHY_top_out_1_reg 	(unsigned int)(&hd21_aphy[nport]->top_out_1)//BA24       
#define HDMIRX_PHY_rsvd0_reg    		(unsigned int)(&hd21_aphy[nport]->rsvd0)//BA28          
#define HDMIRX_PHY_rsvd1_reg		(unsigned int)(&hd21_aphy[nport]->rsvd1)//BA2C          
#define HDMIRX_PHY_ck0_reg			(unsigned int)(&hd21_aphy[nport]->ck0)//BA30             
#define HDMIRX_PHY_ck1_reg			(unsigned int)(&hd21_aphy[nport]->ck1)//BA34             
#define HDMIRX_PHY_ck2_reg 		(unsigned int)(&hd21_aphy[nport]->ck2)//BA38             
#define HDMIRX_PHY_ck3_reg 		(unsigned int)(&hd21_aphy[nport]->ck3)//BA3C             
#define HDMIRX_PHY_ck4_reg			(unsigned int)(&hd21_aphy[nport]->ck4)//BA40             
#define HDMIRX_PHY_acdr_ck0_reg	 (unsigned int)(&hd21_aphy[nport]->acdr_ck0)//BA44       
#define HDMIRX_PHY_acdr_ck1_reg	 (unsigned int)(&hd21_aphy[nport]->acdr_ck1)//BA48       
#define HDMIRX_PHY_acdr_ck2_reg 	(unsigned int)(&hd21_aphy[nport]->acdr_ck2)//BA4C       
#define HDMIRX_PHY_b0_reg       		(unsigned int)(&hd21_aphy[nport]->b0)//BA50             
#define HDMIRX_PHY_b1_reg       		(unsigned int)(&hd21_aphy[nport]->b1)//BA54             
#define HDMIRX_PHY_b2_reg       		(unsigned int)(&hd21_aphy[nport]->b2)//BA58             
#define HDMIRX_PHY_b3_reg       		(unsigned int)(&hd21_aphy[nport]->b3)//BA5C             
#define HDMIRX_PHY_b4_reg      		 (unsigned int)(&hd21_aphy[nport]->b4)//DA60             
#define HDMIRX_PHY_acdr_b0_reg  	(unsigned int)(&hd21_aphy[nport]->acdr_b0)//BA64        
#define HDMIRX_PHY_acdr_b1_reg  	(unsigned int)(&hd21_aphy[nport]->acdr_b1)//BA68        
#define HDMIRX_PHY_acdr_b2_reg 	 (unsigned int)(&hd21_aphy[nport]->acdr_b2)//BA6C        
#define HDMIRX_PHY_g0_reg       		(unsigned int)(&hd21_aphy[nport]->g0)//BA70             
#define HDMIRX_PHY_g1_reg       		(unsigned int)(&hd21_aphy[nport]->g1)//BA74             
#define HDMIRX_PHY_g2_reg      		(unsigned int)(&hd21_aphy[nport]->g2)//BA78             
#define HDMIRX_PHY_g3_reg       		(unsigned int)(&hd21_aphy[nport]->g3)//BA7C             
#define HDMIRX_PHY_g4_reg			(unsigned int)(&hd21_aphy[nport]->g4)//BA80             
#define HDMIRX_PHY_acdr_g0_reg  	(unsigned int)(&hd21_aphy[nport]->acdr_g0)//BA84        
#define HDMIRX_PHY_acdr_g1_reg		(unsigned int)(&hd21_aphy[nport]->acdr_g1)//BA88        
#define HDMIRX_PHY_acdr_g2_reg		(unsigned int)(&hd21_aphy[nport]->acdr_g2)//BA8C        
#define HDMIRX_PHY_r0_reg			(unsigned int)(&hd21_aphy[nport]->r0)//BA90             
#define HDMIRX_PHY_r1_reg			(unsigned int)(&hd21_aphy[nport]->r1)//BA94             
#define HDMIRX_PHY_r2_reg			(unsigned int)(&hd21_aphy[nport]->r2)//BA98             
#define HDMIRX_PHY_r3_reg			(unsigned int)(&hd21_aphy[nport]->r3)//BA9C             
#define HDMIRX_PHY_r4_reg			(unsigned int)(&hd21_aphy[nport]->r4)//BAA0             
#define HDMIRX_PHY_acdr_r0_reg		(unsigned int)(&hd21_aphy[nport]->acdr_r0)//BAA4        
#define HDMIRX_PHY_acdr_r1_reg		(unsigned int)(&hd21_aphy[nport]->acdr_r1)//BAA8        
#define HDMIRX_PHY_acdr_r2_reg		(unsigned int)(&hd21_aphy[nport]->acdr_r2)//BAAC        
#define HDMIRX_PHY_rsvd2_reg		(unsigned int)(&hd21_aphy[nport]->rsvd2)//BAB0          
#define HDMIRX_PHY_ck_cmu_1_reg	(unsigned int)(&hd21_aphy[nport]->ck_cmu_1)//BAB4      
#define HDMIRX_PHY_ck_cmu_2_reg	(unsigned int)(&hd21_aphy[nport]->ck_cmu_2)//BAB8       
#define HDMIRX_PHY_ck_cmu_3_reg	(unsigned int)(&hd21_aphy[nport]->ck_cmu_3)//BABC       
#define HDMIRX_PHY_ck_cmu_4_reg	(unsigned int)(&hd21_aphy[nport]->ck_cmu_4)//BAC0
#endif
/////////////////////


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




//////////////////////
struct hdmi21_aphy_rstb_reg_st {
	uint32_t rega00;  //BAFC	
};

extern const struct hdmi21_aphy_rstb_reg_st *hd21_aphy_rstb[4];

#define HDMIRX_PHY_rega00	(unsigned int)(unsigned long)(&hd21_aphy_rstb[nport]->rega00)//BAFC	                

/////////////////////////////////////////////////

struct hdmi21_dphy_reg_st {
	
	uint32_t cdr_regd00;  //BB00
	uint32_t cdr_regd01;  //BB04
	uint32_t cdr_regd02;  //BB08
	uint32_t cdr_regd03;  //BB0C
//CDR_B	
	uint32_t cdr_regd04;  //BB10
//CDR_G	
	uint32_t cdr_regd05;  //BB14
//CDR_R	
	uint32_t cdr_regd06;  //BB18
//CDR_CK	
	uint32_t cdr_regd07;  //BB1C
//CDR_B		
	uint32_t cdr_regd08;  //BB20
//CDR_G
	uint32_t cdr_regd09;  //BB24
//CDR_R	
	uint32_t cdr_regd10;  //BB28
//CDR_CK	
	uint32_t cdr_regd11;  //BB2C
//CDR_B	
	uint32_t cdr_regd12;  //BB30
//CDR_G	
	uint32_t cdr_regd13;  //BB34
//CDR_R	
	uint32_t cdr_regd14;  //BB38
//CDR_CK	
	uint32_t cdr_regd15;  //BB3C
	
	uint32_t clk_regd00;  //BB40
       //KOFF B
	uint32_t koff_regd00;  //BB44
	uint32_t koff_regd01;  //BB48
	uint32_t koff_regd02;  //BB4C
	uint32_t koff_regd03;  //BB50
       //KOFF G	
	uint32_t koff_regd04;  //BB54
	uint32_t koff_regd05;  //BB58
	uint32_t koff_regd06;  //BB5C
	uint32_t koff_regd07;  //BB60
       //KOFF R	
	uint32_t koff_regd08;  //BB64
	uint32_t koff_regd09;  //BB68
	uint32_t koff_regd10;  //BB6C
	uint32_t koff_regd11;  //BB70
       //KOFF CK	
	uint32_t koff_regd12;  //BB74
	uint32_t koff_regd13;  //BB78
	uint32_t koff_regd14; //BB7C
	uint32_t koff_regd15; //BB80
	
	uint32_t koff_regd16; //BB84
	uint32_t data_regd00; //BB88
	uint32_t data_regd01; //BB8C
	uint32_t data_regd02; //BB90
	uint32_t data_regd03; //BB94
	
	uint32_t data_regd04; //BB98
	
	uint32_t data_regd05;  //BB9C
	uint32_t data_regd06;  //BBA0
	uint32_t data_regd07;  //BBA4
	uint32_t data_regd08;  //BBA8
	uint32_t data_regd09;  //BBAC
	uint32_t data_regd10;  //BBB0
	uint32_t rsvd4;  //BBB4
	//FLD_B
	uint32_t fld_regd00;  //BBB8
	uint32_t fld_regd01;  //BBBC
	uint32_t fld_regd02;  //BBC0
	uint32_t fld_regd03;  //BBC4
	uint32_t fld_regd04;  //BBC8
	uint32_t fld_regd05;  //BBCC
	uint32_t fld_regd06;  //BBD0
	
	uint32_t rsvd5;  //BBD4
	//FLD_G
	uint32_t fld_regd07;  //BBD8
	uint32_t fld_regd08;  //BBDC	
	uint32_t fld_regd09;  //BBE0		
	uint32_t fld_regd10;  //BBE4		
	uint32_t fld_regd11;  //BBE8		
	uint32_t fld_regd12;  //BBEC		
	uint32_t fld_regd13;  //BBF0		
	//FLD_R	
	uint32_t fld_regd14;  //BBF4
	uint32_t fld_regd15;  //BBF8		
	uint32_t fld_regd16;  //BBFC		
	uint32_t fld_regd17;  //BC00		
	uint32_t fld_regd18;  //BC04		
	uint32_t fld_regd19;  //BC08		
	uint32_t fld_regd20;  //BC0C		
	//FLD_CK	
	uint32_t fld_regd21;  //BC10		
	uint32_t fld_regd22;  //BC14		
	uint32_t fld_regd23;  //BC18		
	uint32_t fld_regd24;  //BC1C		
	uint32_t fld_regd25;  //BC20		
	uint32_t fld_regd26;  //BC4		
	uint32_t fld_regd27;  //BC8
	
	uint32_t mod_regd00;  //BCC	
	uint32_t mod_regd01;  //BC0	
	uint32_t pllcdr_regd00;  //BC34	
	uint32_t pllcdr_regd01;  //BC38	
	//PLLCDR_B	
	uint32_t pllcdr_regd02;  //BC3C	
	uint32_t pllcdr_regd03;  //BC40
	uint32_t pllcdr_regd04;  //BC44
	//PLLCDR_G	
	uint32_t pllcdr_regd05;  //BC48
	uint32_t pllcdr_regd06;  //BC4C
	uint32_t pllcdr_regd07;  //BC50
	//PLLCDR_R	
	uint32_t pllcdr_regd08;  //BC54
	uint32_t pllcdr_regd09;  //BC58
	uint32_t pllcdr_regd10;  //BC5C
	//PLLCDR_CK	
	uint32_t pllcdr_regd11;  //BC60
	uint32_t pllcdr_regd12;  //BC64
	uint32_t pllcdr_regd13;  //BC68
	
	uint32_t psave_regd00;  //BC6C
	uint32_t dfe_regd00;  //BC70
	uint32_t dfe_regd01;  //BC74
	uint32_t dfe_regd02;  //BC78
	uint32_t misc_regd00;  //BC7C
	uint32_t misc_regd01;  //BC80
	uint32_t misc_regd02;  //BC84
};


extern const struct hdmi21_dphy_reg_st *hd21_dphy[4];


/////////////////////////////////////////////////////
#ifdef CONFIG_ARM64
#define HDMIRX_PHY_cdr_regd00				(unsigned int)(unsigned long)(&hd21_dphy[nport]->cdr_regd00)//BB00         
#define HDMIRX_PHY_cdr_regd01				(unsigned int)(unsigned long)(&hd21_dphy[nport]->cdr_regd01)//BB04         
#define HDMIRX_PHY_cdr_regd02				(unsigned int)(unsigned long)(&hd21_dphy[nport]->cdr_regd02)//BB08         
#define HDMIRX_PHY_cdr_regd03				(unsigned int)(unsigned long)(&hd21_dphy[nport]->cdr_regd03)//BB0C         
#define HDMIRX_PHY_cdr_regd04				(unsigned int)(unsigned long)(&hd21_dphy[nport]->cdr_regd04)//BB10         
#define HDMIRX_PHY_cdr_regd05				(unsigned int)(unsigned long)(&hd21_dphy[nport]->cdr_regd05)//BB14         
#define HDMIRX_PHY_cdr_regd06				(unsigned int)(unsigned long)(&hd21_dphy[nport]->cdr_regd06)//BB18         
#define HDMIRX_PHY_cdr_regd07				(unsigned int)(unsigned long)(&hd21_dphy[nport]->cdr_regd07)//BB1C         
#define HDMIRX_PHY_cdr_regd08				(unsigned int)(unsigned long)(&hd21_dphy[nport]->cdr_regd08)//BB20         
#define HDMIRX_PHY_cdr_regd09				(unsigned int)(unsigned long)(&hd21_dphy[nport]->cdr_regd09)//BB24         
#define HDMIRX_PHY_cdr_regd10				(unsigned int)(unsigned long)(&hd21_dphy[nport]->cdr_regd10)//BB28         
#define HDMIRX_PHY_cdr_regd11				(unsigned int)(unsigned long)(&hd21_dphy[nport]->cdr_regd11)//BB2C         
#define HDMIRX_PHY_cdr_regd12				(unsigned int)(unsigned long)(&hd21_dphy[nport]->cdr_regd12)//BB30         
#define HDMIRX_PHY_cdr_regd13				(unsigned int)(unsigned long)(&hd21_dphy[nport]->cdr_regd13)//BB34         
#define HDMIRX_PHY_cdr_regd14				(unsigned int)(unsigned long)(&hd21_dphy[nport]->cdr_regd14)//BB38         
#define HDMIRX_PHY_cdr_regd15				(unsigned int)(unsigned long)(&hd21_dphy[nport]->cdr_regd15)//BB3C         
#define HDMIRX_PHY_clk_regd00				(unsigned int)(unsigned long)(&hd21_dphy[nport]->clk_regd00)//BB40         
//KOFF B                                                                                            
#define HDMIRX_PHY_koff_regd00				(unsigned int)(unsigned long)(&hd21_dphy[nport]->koff_regd00)//BB44        
#define HDMIRX_PHY_koff_regd01				(unsigned int)(unsigned long)(&hd21_dphy[nport]->koff_regd01)//BB48        
#define HDMIRX_PHY_koff_regd02				(unsigned int)(unsigned long)(&hd21_dphy[nport]->koff_regd02)//BB4C        
#define HDMIRX_PHY_koff_regd03				(unsigned int)(unsigned long)(&hd21_dphy[nport]->koff_regd03)//BB50        
 //KOFF G                                                                                           
#define HDMIRX_PHY_koff_regd04				(unsigned int)(unsigned long)(&hd21_dphy[nport]->koff_regd04)//BB54        
#define HDMIRX_PHY_koff_regd05				(unsigned int)(unsigned long)(&hd21_dphy[nport]->koff_regd05)//BB58        
#define HDMIRX_PHY_koff_regd06				(unsigned int)(unsigned long)(&hd21_dphy[nport]->koff_regd06)//BB5C        
#define HDMIRX_PHY_koff_regd07				(unsigned int)(unsigned long)(&hd21_dphy[nport]->koff_regd07)//BB60        
 //KOFF R                                                                                           
#define HDMIRX_PHY_koff_regd08				(unsigned int)(unsigned long)(&hd21_dphy[nport]->koff_regd08)//BB64        
#define HDMIRX_PHY_koff_regd09				(unsigned int)(unsigned long)(&hd21_dphy[nport]->koff_regd09)//BB68        
#define HDMIRX_PHY_koff_regd10				(unsigned int)(unsigned long)(&hd21_dphy[nport]->koff_regd10)//BB6C        
#define HDMIRX_PHY_koff_regd11				(unsigned int)(unsigned long)(&hd21_dphy[nport]->koff_regd11)//BB70        
 //KOFF CK                                                                                           
#define HDMIRX_PHY_koff_regd12				(unsigned int)(unsigned long)(&hd21_dphy[nport]->koff_regd12)//BB74        
#define HDMIRX_PHY_koff_regd13				(unsigned int)(unsigned long)(&hd21_dphy[nport]->koff_regd13)//BB78        
#define HDMIRX_PHY_koff_regd14				(unsigned int)(unsigned long)(&hd21_dphy[nport]->koff_regd14)//BB7C         
#define HDMIRX_PHY_koff_regd15				(unsigned int)(unsigned long)(&hd21_dphy[nport]->koff_regd15)//BB80         
                                                                                            
#define HDMIRX_PHY_koff_regd16				(unsigned int)(unsigned long)(&hd21_dphy[nport]->koff_regd16)//BB84         
#define HDMIRX_PHY_data_regd00				(unsigned int)(unsigned long)(&hd21_dphy[nport]->data_regd00)//BB88         
#define HDMIRX_PHY_data_regd01				(unsigned int)(unsigned long)(&hd21_dphy[nport]->data_regd01)//BB8C         
#define HDMIRX_PHY_data_regd02				(unsigned int)(unsigned long)(&hd21_dphy[nport]->data_regd02)//BB90         
#define HDMIRX_PHY_data_regd03				(unsigned int)(unsigned long)(&hd21_dphy[nport]->data_regd03)//BB94         
                                                                                            
#define HDMIRX_PHY_data_regd04				(unsigned int)(unsigned long)(&hd21_dphy[nport]->data_regd04)//BB98         
                           
#define HDMIRX_PHY_data_regd05				(unsigned int)(unsigned long)(&hd21_dphy[nport]->data_regd05)//BB9C        
#define HDMIRX_PHY_data_regd06				(unsigned int)(unsigned long)(&hd21_dphy[nport]->data_regd06)//BBA0        
#define HDMIRX_PHY_data_regd07				(unsigned int)(unsigned long)(&hd21_dphy[nport]->data_regd07)//BBA4        
#define HDMIRX_PHY_data_regd08				(unsigned int)(unsigned long)(&hd21_dphy[nport]->data_regd08)//BBA8        
#define HDMIRX_PHY_data_regd09				(unsigned int)(unsigned long)(&hd21_dphy[nport]->data_regd09)//BBC        
#define HDMIRX_PHY_data_regd10				(unsigned int)(unsigned long)(&hd21_dphy[nport]->data_regd10)//BBB0        
#define HDMIRX_PHY_rsvd4					(unsigned int)(unsigned long)(&hd21_dphy[nport]->rsvd4)//BBB4                    
 //FLD_B                                                                                            
#define HDMIRX_PHY_fld_regd00				(unsigned int)(unsigned long)(&hd21_dphy[nport]->fld_regd00)//BBB8          
#define HDMIRX_PHY_fld_regd01				(unsigned int)(unsigned long)(&hd21_dphy[nport]->fld_regd01)//BBBC          
#define HDMIRX_PHY_fld_regd02				(unsigned int)(unsigned long)(&hd21_dphy[nport]->fld_regd02)//BBC0          
#define HDMIRX_PHY_fld_regd03				(unsigned int)(unsigned long)(&hd21_dphy[nport]->fld_regd03)//BBC4          
#define HDMIRX_PHY_fld_regd04				(unsigned int)(unsigned long)(&hd21_dphy[nport]->fld_regd04)//BBC8          
#define HDMIRX_PHY_fld_regd05				(unsigned int)(unsigned long)(&hd21_dphy[nport]->fld_regd05)//BBCC          
#define HDMIRX_PHY_fld_regd06				(unsigned int)(unsigned long)(&hd21_dphy[nport]->fld_regd06)//BBD0          
                                                                                            
#define HDMIRX_PHY_rsvd5					(unsigned int)(unsigned long)(&hd21_dphy[nport]->rsvd5)//BBD4                    
 //FLD_G                                                                                           
#define HDMIRX_PHY_fld_regd07				(unsigned int)(unsigned long)(&hd21_dphy[nport]->fld_regd07)//BBD8          
#define HDMIRX_PHY_fld_regd08				(unsigned int)(unsigned long)(&hd21_dphy[nport]->fld_regd08)//BBDC          
#define HDMIRX_PHY_fld_regd09				(unsigned int)(unsigned long)(&hd21_dphy[nport]->fld_regd09)//BBE0		      
#define HDMIRX_PHY_fld_regd10				(unsigned int)(unsigned long)(&hd21_dphy[nport]->fld_regd10)//BBE4		      
#define HDMIRX_PHY_fld_regd11				(unsigned int)(unsigned long)(&hd21_dphy[nport]->fld_regd11)//BBE8		      
#define HDMIRX_PHY_fld_regd12				(unsigned int)(unsigned long)(&hd21_dphy[nport]->fld_regd12)//BBEC		      
#define HDMIRX_PHY_fld_regd13				(unsigned int)(unsigned long)(&hd21_dphy[nport]->fld_regd13)//BBF0		      
 //FLD_R                                                                                            
#define HDMIRX_PHY_fld_regd14				(unsigned int)(unsigned long)(&hd21_dphy[nport]->fld_regd14)//BBF4          
#define HDMIRX_PHY_fld_regd15				(unsigned int)(unsigned long)(&hd21_dphy[nport]->fld_regd15)//BBF8		      
#define HDMIRX_PHY_fld_regd16				(unsigned int)(unsigned long)(&hd21_dphy[nport]->fld_regd16)//BBFC		      
#define HDMIRX_PHY_fld_regd17				(unsigned int)(unsigned long)(&hd21_dphy[nport]->fld_regd17)//BC00          
#define HDMIRX_PHY_fld_regd18				(unsigned int)(unsigned long)(&hd21_dphy[nport]->fld_regd18)//BC004		      
#define HDMIRX_PHY_fld_regd19				(unsigned int)(unsigned long)(&hd21_dphy[nport]->fld_regd19)//BC008		      
#define HDMIRX_PHY_fld_regd20				(unsigned int)(unsigned long)(&hd21_dphy[nport]->fld_regd20)//BC00C		      
 //FLD_CK                                                                                           
#define HDMIRX_PHY_fld_regd21				(unsigned int)(unsigned long)(&hd21_dphy[nport]->fld_regd21)//BC010		      
#define HDMIRX_PHY_fld_regd22				(unsigned int)(unsigned long)(&hd21_dphy[nport]->fld_regd22)//BC014		      
#define HDMIRX_PHY_fld_regd23				(unsigned int)(unsigned long)(&hd21_dphy[nport]->fld_regd23)//BC018		      
#define HDMIRX_PHY_fld_regd24				(unsigned int)(unsigned long)(&hd21_dphy[nport]->fld_regd24)//BC01C		      
#define HDMIRX_PHY_fld_regd25				(unsigned int)(unsigned long)(&hd21_dphy[nport]->fld_regd25)//BC020		      
#define HDMIRX_PHY_fld_regd26				(unsigned int)(unsigned long)(&hd21_dphy[nport]->fld_regd26)//BC024          
#define HDMIRX_PHY_fld_regd27				(unsigned int)(unsigned long)(&hd21_dphy[nport]->fld_regd27)//BC028          
                                                                                            
#define HDMIRX_PHY_mod_regd00				(unsigned int)(unsigned long)(&hd21_dphy[nport]->mod_regd00)//BC02C	        
#define HDMIRX_PHY_mod_regd01				(unsigned int)(unsigned long)(&hd21_dphy[nport]->mod_regd01)//BC030	        
#define HDMIRX_PHY_pllcdr_regd00			(unsigned int)(unsigned long)(&hd21_dphy[nport]->pllcdr_regd00)//BC034	  
#define HDMIRX_PHY_pllcdr_regd01			(unsigned int)(unsigned long)(&hd21_dphy[nport]->pllcdr_regd01)//BC038	  
                                                                                            
#define HDMIRX_PHY_pllcdr_regd02			(unsigned int)(unsigned long)(&hd21_dphy[nport]->pllcdr_regd02)//BC03C	  
#define HDMIRX_PHY_pllcdr_regd03			(unsigned int)(unsigned long)(&hd21_dphy[nport]->pllcdr_regd03)//BC040    
#define HDMIRX_PHY_pllcdr_regd04			(unsigned int)(unsigned long)(&hd21_dphy[nport]->pllcdr_regd04)//BC044    
                                                                                            
#define HDMIRX_PHY_pllcdr_regd05			(unsigned int)(unsigned long)(&hd21_dphy[nport]->pllcdr_regd05)//BC048    
#define HDMIRX_PHY_pllcdr_regd06			(unsigned int)(unsigned long)(&hd21_dphy[nport]->pllcdr_regd06)//BC04C    
#define HDMIRX_PHY_pllcdr_regd07			(unsigned int)(unsigned long)(&hd21_dphy[nport]->pllcdr_regd07)//BC050    
                                                                                            
#define HDMIRX_PHY_pllcdr_regd08			(unsigned int)(unsigned long)(&hd21_dphy[nport]->pllcdr_regd08)//BC054    
#define HDMIRX_PHY_pllcdr_regd09			(unsigned int)(unsigned long)(&hd21_dphy[nport]->pllcdr_regd09)//BC058    
#define HDMIRX_PHY_pllcdr_regd10			(unsigned int)(unsigned long)(&hd21_dphy[nport]->pllcdr_regd10)//BC05C    
                                                                                            
#define HDMIRX_PHY_pllcdr_regd11			(unsigned int)(unsigned long)(&hd21_dphy[nport]->pllcdr_regd11)//BC060    
#define HDMIRX_PHY_pllcdr_regd12			(unsigned int)(unsigned long)(&hd21_dphy[nport]->pllcdr_regd12)//BC064    
#define HDMIRX_PHY_pllcdr_regd13			(unsigned int)(unsigned long)(&hd21_dphy[nport]->pllcdr_regd13)//BC068    
                                                                                            
#define HDMIRX_PHY_psave_regd00			(unsigned int)(unsigned long)(&hd21_dphy[nport]->psave_regd00)//BC06C      
#define HDMIRX_PHY_dfe_regd00			(unsigned int)(unsigned long)(&hd21_dphy[nport]->dfe_regd00)//BC070          
#define HDMIRX_PHY_dfe_regd01			(unsigned int)(unsigned long)(&hd21_dphy[nport]->dfe_regd01)//BC074          
#define HDMIRX_PHY_dfe_regd02			(unsigned int)(unsigned long)(&hd21_dphy[nport]->dfe_regd02)//BC078          
#define HDMIRX_PHY_misc_regd00			(unsigned int)(unsigned long)(&hd21_dphy[nport]->misc_regd00)//BC07C        
#define HDMIRX_PHY_misc_regd01			(unsigned int)(unsigned long)(&hd21_dphy[nport]->misc_regd01)//BC080        
#define HDMIRX_PHY_misc_regd02			(unsigned int)(unsigned long)(&hd21_dphy[nport]->misc_regd02)//BC084        

#else
                                                                                            
#define HDMIRX_PHY_cdr_regd00				(unsigned int)(&hd21_dphy[nport]->cdr_regd00)//BB00         
#define HDMIRX_PHY_cdr_regd01				(unsigned int)(&hd21_dphy[nport]->cdr_regd01)//BB04         
#define HDMIRX_PHY_cdr_regd02				(unsigned int)(&hd21_dphy[nport]->cdr_regd02)//BB08         
#define HDMIRX_PHY_cdr_regd03				(unsigned int)(&hd21_dphy[nport]->cdr_regd03)//BB0C         
#define HDMIRX_PHY_cdr_regd04				(unsigned int)(&hd21_dphy[nport]->cdr_regd04)//BB10         
#define HDMIRX_PHY_cdr_regd05				(unsigned int)(&hd21_dphy[nport]->cdr_regd05)//BB14         
#define HDMIRX_PHY_cdr_regd06				(unsigned int)(&hd21_dphy[nport]->cdr_regd06)//BB18         
#define HDMIRX_PHY_cdr_regd07				(unsigned int)(&hd21_dphy[nport]->cdr_regd07)//BB1C         
#define HDMIRX_PHY_cdr_regd08				(unsigned int)(&hd21_dphy[nport]->cdr_regd08)//BB20         
#define HDMIRX_PHY_cdr_regd09				(unsigned int)(&hd21_dphy[nport]->cdr_regd09)//BB24         
#define HDMIRX_PHY_cdr_regd10				(unsigned int)(&hd21_dphy[nport]->cdr_regd10)//BB28         
#define HDMIRX_PHY_cdr_regd11				(unsigned int)(&hd21_dphy[nport]->cdr_regd11)//BB2C         
#define HDMIRX_PHY_cdr_regd12				(unsigned int)(&hd21_dphy[nport]->cdr_regd12)//BB30         
#define HDMIRX_PHY_cdr_regd13				(unsigned int)(&hd21_dphy[nport]->cdr_regd13)//BB34         
#define HDMIRX_PHY_cdr_regd14				(unsigned int)(&hd21_dphy[nport]->cdr_regd14)//BB38         
#define HDMIRX_PHY_cdr_regd15				(unsigned int)(&hd21_dphy[nport]->cdr_regd15)//BB3C         
#define HDMIRX_PHY_clk_regd00				(unsigned int)(&hd21_dphy[nport]->clk_regd00)//BB40         
//KOFF B                                                                                            
#define HDMIRX_PHY_koff_regd00				(unsigned int)(&hd21_dphy[nport]->koff_regd00)//BB44        
#define HDMIRX_PHY_koff_regd01				(unsigned int)(&hd21_dphy[nport]->koff_regd01)//BB48        
#define HDMIRX_PHY_koff_regd02				(unsigned int)(&hd21_dphy[nport]->koff_regd02)//BB4C        
#define HDMIRX_PHY_koff_regd03				(unsigned int)(&hd21_dphy[nport]->koff_regd03)//BB50        
 //KOFF G                                                                                           
#define HDMIRX_PHY_koff_regd04				(unsigned int)(&hd21_dphy[nport]->koff_regd04)//BB54        
#define HDMIRX_PHY_koff_regd05				(unsigned int)(&hd21_dphy[nport]->koff_regd05)//BB58        
#define HDMIRX_PHY_koff_regd06				(unsigned int)(&hd21_dphy[nport]->koff_regd06)//BB5C        
#define HDMIRX_PHY_koff_regd07				(unsigned int)(&hd21_dphy[nport]->koff_regd07)//BB60        
 //KOFF R                                                                                           
#define HDMIRX_PHY_koff_regd08				(unsigned int)(&hd21_dphy[nport]->koff_regd08)//BB64        
#define HDMIRX_PHY_koff_regd09				(unsigned int)(&hd21_dphy[nport]->koff_regd09)//BB68        
#define HDMIRX_PHY_koff_regd10				(unsigned int)(&hd21_dphy[nport]->koff_regd10)//BB6C        
#define HDMIRX_PHY_koff_regd11				(unsigned int)(&hd21_dphy[nport]->koff_regd11)//BB70        
 //KOFF CK                                                                                           
#define HDMIRX_PHY_koff_regd12				(unsigned int)(&hd21_dphy[nport]->koff_regd12)//BB74        
#define HDMIRX_PHY_koff_regd13				(unsigned int)(&hd21_dphy[nport]->koff_regd13)//BB78        
#define HDMIRX_PHY_koff_regd14				(unsigned int)(&hd21_dphy[nport]->koff_regd14)//BB7C         
#define HDMIRX_PHY_koff_regd15				(unsigned int)(&hd21_dphy[nport]->koff_regd15)//BB80         
                                                                                            
#define HDMIRX_PHY_koff_regd16				(unsigned int)(&hd21_dphy[nport]->koff_regd16)//BB84         
#define HDMIRX_PHY_data_regd00				(unsigned int)(&hd21_dphy[nport]->data_regd00)//BB88         
#define HDMIRX_PHY_data_regd01				(unsigned int)(&hd21_dphy[nport]->data_regd01)//BB8C         
#define HDMIRX_PHY_data_regd02				(unsigned int)(&hd21_dphy[nport]->data_regd02)//BB90         
#define HDMIRX_PHY_data_regd03				(unsigned int)(&hd21_dphy[nport]->data_regd03)//BB94         
                                                                                            
#define HDMIRX_PHY_data_regd04				(unsigned int)(&hd21_dphy[nport]->data_regd04)//BB98         
                           
#define HDMIRX_PHY_data_regd05				(unsigned int)(&hd21_dphy[nport]->data_regd05)//BB9C        
#define HDMIRX_PHY_data_regd06				(unsigned int)(&hd21_dphy[nport]->data_regd06)//BBA0        
#define HDMIRX_PHY_data_regd07				(unsigned int)(&hd21_dphy[nport]->data_regd07)//BBA4        
#define HDMIRX_PHY_data_regd08				(unsigned int)(&hd21_dphy[nport]->data_regd08)//BBA8        
#define HDMIRX_PHY_data_regd09				(unsigned int)(&hd21_dphy[nport]->data_regd09)//BBC        
#define HDMIRX_PHY_data_regd10				(unsigned int)(&hd21_dphy[nport]->data_regd10)//BBB0        
#define HDMIRX_PHY_rsvd4					(unsigned int)(&hd21_dphy[nport]->rsvd4)//BBB4                    
 //FLD_B                                                                                            
#define HDMIRX_PHY_fld_regd00				(unsigned int)(&hd21_dphy[nport]->fld_regd00)//BBB8          
#define HDMIRX_PHY_fld_regd01				(unsigned int)(&hd21_dphy[nport]->fld_regd01)//BBBC          
#define HDMIRX_PHY_fld_regd02				(unsigned int)(&hd21_dphy[nport]->fld_regd02)//BBC0          
#define HDMIRX_PHY_fld_regd03				(unsigned int)(&hd21_dphy[nport]->fld_regd03)//BBC4          
#define HDMIRX_PHY_fld_regd04				(unsigned int)(&hd21_dphy[nport]->fld_regd04)//BBC8          
#define HDMIRX_PHY_fld_regd05				(unsigned int)(&hd21_dphy[nport]->fld_regd05)//BBCC          
#define HDMIRX_PHY_fld_regd06				(unsigned int)(&hd21_dphy[nport]->fld_regd06)//BBD0          
                                                                                            
#define HDMIRX_PHY_rsvd5					(unsigned int)(&hd21_dphy[nport]->rsvd5)//BBD4                    
 //FLD_G                                                                                           
#define HDMIRX_PHY_fld_regd07				(unsigned int)(&hd21_dphy[nport]->fld_regd07)//BBD8          
#define HDMIRX_PHY_fld_regd08				(unsigned int)(&hd21_dphy[nport]->fld_regd08)//BBDC          
#define HDMIRX_PHY_fld_regd09				(unsigned int)(&hd21_dphy[nport]->fld_regd09)//BBE0		      
#define HDMIRX_PHY_fld_regd10				(unsigned int)(&hd21_dphy[nport]->fld_regd10)//BBE4		      
#define HDMIRX_PHY_fld_regd11				(unsigned int)(&hd21_dphy[nport]->fld_regd11)//BBE8		      
#define HDMIRX_PHY_fld_regd12				(unsigned int)(&hd21_dphy[nport]->fld_regd12)//BBEC		      
#define HDMIRX_PHY_fld_regd13				(unsigned int)(&hd21_dphy[nport]->fld_regd13)//BBF0		      
 //FLD_R                                                                                            
#define HDMIRX_PHY_fld_regd14				(unsigned int)(&hd21_dphy[nport]->fld_regd14)//BBF4          
#define HDMIRX_PHY_fld_regd15				(unsigned int)(&hd21_dphy[nport]->fld_regd15)//BBF8		      
#define HDMIRX_PHY_fld_regd16				(unsigned int)(&hd21_dphy[nport]->fld_regd16)//BBFC		      
#define HDMIRX_PHY_fld_regd17				(unsigned int)(&hd21_dphy[nport]->fld_regd17)//BC00          
#define HDMIRX_PHY_fld_regd18				(unsigned int)(&hd21_dphy[nport]->fld_regd18)//BC004		      
#define HDMIRX_PHY_fld_regd19				(unsigned int)(&hd21_dphy[nport]->fld_regd19)//BC008		      
#define HDMIRX_PHY_fld_regd20				(unsigned int)(&hd21_dphy[nport]->fld_regd20)//BC00C		      
 //FLD_CK                                                                                           
#define HDMIRX_PHY_fld_regd21				(unsigned int)(&hd21_dphy[nport]->fld_regd21)//BC010		      
#define HDMIRX_PHY_fld_regd22				(unsigned int)(&hd21_dphy[nport]->fld_regd22)//BC014		      
#define HDMIRX_PHY_fld_regd23				(unsigned int)(&hd21_dphy[nport]->fld_regd23)//BC018		      
#define HDMIRX_PHY_fld_regd24				(unsigned int)(&hd21_dphy[nport]->fld_regd24)//BC01C		      
#define HDMIRX_PHY_fld_regd25				(unsigned int)(&hd21_dphy[nport]->fld_regd25)//BC020		      
#define HDMIRX_PHY_fld_regd26				(unsigned int)(&hd21_dphy[nport]->fld_regd26)//BC024          
#define HDMIRX_PHY_fld_regd27				(unsigned int)(&hd21_dphy[nport]->fld_regd27)//BC028          
                                                                                            
#define HDMIRX_PHY_mod_regd00				(unsigned int)(&hd21_dphy[nport]->mod_regd00)//BC02C	        
#define HDMIRX_PHY_mod_regd01				(unsigned int)(&hd21_dphy[nport]->mod_regd01)//BC030	        
#define HDMIRX_PHY_pllcdr_regd00			(unsigned int)(&hd21_dphy[nport]->pllcdr_regd00)//BC034	  
#define HDMIRX_PHY_pllcdr_regd01			(unsigned int)(&hd21_dphy[nport]->pllcdr_regd01)//BC038	  
                                                                                            
#define HDMIRX_PHY_pllcdr_regd02			(unsigned int)(&hd21_dphy[nport]->pllcdr_regd02)//BC03C	  
#define HDMIRX_PHY_pllcdr_regd03			(unsigned int)(&hd21_dphy[nport]->pllcdr_regd03)//BC040    
#define HDMIRX_PHY_pllcdr_regd04			(unsigned int)(&hd21_dphy[nport]->pllcdr_regd04)//BC044    
                                                                                            
#define HDMIRX_PHY_pllcdr_regd05			(unsigned int)(&hd21_dphy[nport]->pllcdr_regd05)//BC048    
#define HDMIRX_PHY_pllcdr_regd06			(unsigned int)(&hd21_dphy[nport]->pllcdr_regd06)//BC04C    
#define HDMIRX_PHY_pllcdr_regd07			(unsigned int)(&hd21_dphy[nport]->pllcdr_regd07)//BC050    
                                                                                            
#define HDMIRX_PHY_pllcdr_regd08			(unsigned int)(&hd21_dphy[nport]->pllcdr_regd08)//BC054    
#define HDMIRX_PHY_pllcdr_regd09			(unsigned int)(&hd21_dphy[nport]->pllcdr_regd09)//BC058    
#define HDMIRX_PHY_pllcdr_regd10			(unsigned int)(&hd21_dphy[nport]->pllcdr_regd10)//BC05C    
                                                                                            
#define HDMIRX_PHY_pllcdr_regd11			(unsigned int)(&hd21_dphy[nport]->pllcdr_regd11)//BC060    
#define HDMIRX_PHY_pllcdr_regd12			(unsigned int)(&hd21_dphy[nport]->pllcdr_regd12)//BC064    
#define HDMIRX_PHY_pllcdr_regd13			(unsigned int)(&hd21_dphy[nport]->pllcdr_regd13)//BC068    
                                                                                            
#define HDMIRX_PHY_psave_regd00			(unsigned int)(&hd21_dphy[nport]->psave_regd00)//BC06C      
#define HDMIRX_PHY_dfe_regd00			(unsigned int)(&hd21_dphy[nport]->dfe_regd00)//BC070          
#define HDMIRX_PHY_dfe_regd01			(unsigned int)(&hd21_dphy[nport]->dfe_regd01)//BC074          
#define HDMIRX_PHY_dfe_regd02			(unsigned int)(&hd21_dphy[nport]->dfe_regd02)//BC078          
#define HDMIRX_PHY_misc_regd00			(unsigned int)(&hd21_dphy[nport]->misc_regd00)//BC07C        
#define HDMIRX_PHY_misc_regd01			(unsigned int)(&hd21_dphy[nport]->misc_regd01)//BC080        
#define HDMIRX_PHY_misc_regd02			(unsigned int)(&hd21_dphy[nport]->misc_regd02)//BC084        
#endif

///
#define  RS_CAL_EN					   (_BIT21)			//Mac8p
#define  POW_LEQ					   (_BIT24)			//Mac8p

/////////////////////////

#define acdr_manual_RDY		(_BIT30)  //Mac8p
#define acdr_manual_RDY_FROM_REG	(_BIT30)  //Mac8p
#define acdr_manual_CMU_SEL	(_BIT31)  //Mac8p

#define acdr_PFD_RSTB						(_BIT0)  //Mac8p
#define acdr_DIV_IQ_RSTB					(_BIT16)  //Mac8p
#define acdr_VCO_EN					              (_BIT8)  //Mac8p


#define HD21_TOP_IN_Z0_FIX_SELECT_0 (_BIT3|_BIT2)
#define HD21_TOP_IN_Z0_FIX_SELECT_1 (_BIT5|_BIT4)
#define HD21_TOP_IN_Z0_FIX_SELECT_10 (_BIT3)



#define CK_LATCH						(_BIT1)
#define REG_CK_LATCH					(_BIT1)   //From Mac8p


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
#define  p0_ck_2_CMU_CKIN_SEL                                                                   (_BIT1) //From Mac8p
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




