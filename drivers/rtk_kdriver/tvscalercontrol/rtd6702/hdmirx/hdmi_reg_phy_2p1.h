#include <rbus/hdmirx_phy_reg.h>
#include <rbus/efuse_reg.h>

#define HDMI_APHY0_BASE	    (0xB800DA04)   //DA04 ~ DAC0 (P0)   
#define HDMI_APHY1_BASE	    (0xB803BA04) 
#define HDMI_APHY2_BASE	    (0xB803CA04)   
#define HDMI_APHY3_BASE	    (0xB80B2A04) 

#define HDMI_DPHY0_BASE	    (0xB800DC00)   //DAFC ~ DD84 (P0) 
#define HDMI_DPHY1_BASE	    (0xB803BC00)  
#define HDMI_DPHY2_BASE	    (0xB803CC00)    
#define HDMI_DPHY3_BASE	    (0xB80B2C00)   

#define HDMI_APHY0_RSTB_BASE	    (0xB800DAFC) 
#define HDMI_APHY1_RSTB_BASE	    (0xB803BAFC) 
#define HDMI_APHY2_RSTB_BASE	    (0xB803CAFC) 
#define HDMI_APHY3_RSTB_BASE	    (0xB80B2AFC) 




struct hdmi21_aphy_reg_st {
	uint32_t z0k;  //DA04
	uint32_t z0pow;  //DA08
	uint32_t rsvd0;  //DA0C
	uint32_t top_in_0;  //DA10
	uint32_t top_in_1;  //DA14	
	uint32_t top_in_2;  //DA18	
	uint32_t rsvd1;  //DA1C	
	uint32_t top_out_0;  //DA20
	uint32_t top_out_1; //DA24
	uint32_t rsvd2;  //DA28
	uint32_t rsvd3;  //DA2C
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
	uint32_t rsvd4;  //DAB0
	uint32_t ck_cmu_1;  //DAB4;
	uint32_t ck_cmu_2;  //DAB8
	uint32_t ck_cmu_3;  //DABC
	uint32_t ck_cmu_4;  //DAC0
};

extern const struct hdmi21_aphy_reg_st *hd21_aphy[4];


///////////////////////////////////////////////////


#define HDMIRX_PHY_z0k_reg     	 	(unsigned int)(&hd21_aphy[nport]->z0k)//DA04            
#define HDMIRX_PHY_z0pow_reg    	(unsigned int)(&hd21_aphy[nport]->z0pow)//DA08	        
#define HDMIRX_PHY_top_in_0_reg 	(unsigned int)(&hd21_aphy[nport]->top_in_0)//DA10       
#define HDMIRX_PHY_top_in_1_reg 	(unsigned int)(&hd21_aphy[nport]->top_in_1)//DA14	      
#define HDMIRX_PHY_top_in_2_reg 	(unsigned int)(&hd21_aphy[nport]->top_in_2)//DA18	      
#define HDMIRX_PHY_top_out_0_re 	(unsigned int)(&hd21_aphy[nport]->top_out_0)//DA20      
#define HDMIRX_PHY_top_out_1_re 	(unsigned int)(&hd21_aphy[nport]->top_out_1)//DA24       
#define HDMIRX_PHY_rsvd0_reg    		(unsigned int)(&hd21_aphy[nport]->rsvd0)//DA28          
#define HDMIRX_PHY_rsvd1_reg		(unsigned int)(&hd21_aphy[nport]->rsvd1)//DA2C          
#define HDMIRX_PHY_ck0_reg			(unsigned int)(&hd21_aphy[nport]->ck0)//DA30             
#define HDMIRX_PHY_ck1_reg			(unsigned int)(&hd21_aphy[nport]->ck1)//DA34             
#define HDMIRX_PHY_ck2_reg 		(unsigned int)(&hd21_aphy[nport]->ck2)//DA38             
#define HDMIRX_PHY_ck3_reg 		(unsigned int)(&hd21_aphy[nport]->ck3)//DA3C             
#define HDMIRX_PHY_ck4_reg			(unsigned int)(&hd21_aphy[nport]->ck4)//DA40             
#define HDMIRX_PHY_acdr_ck0_reg	 (unsigned int)(&hd21_aphy[nport]->acdr_ck0)//DA44       
#define HDMIRX_PHY_acdr_ck1_reg	 (unsigned int)(&hd21_aphy[nport]->acdr_ck1)//DA48       
#define HDMIRX_PHY_acdr_ck2_reg 	(unsigned int)(&hd21_aphy[nport]->acdr_ck2)//DA4C       
#define HDMIRX_PHY_b0_reg       		(unsigned int)(&hd21_aphy[nport]->b0)//DA50             
#define HDMIRX_PHY_b1_reg       		(unsigned int)(&hd21_aphy[nport]->b1)//DA54             
#define HDMIRX_PHY_b2_reg       		(unsigned int)(&hd21_aphy[nport]->b2)//DA58             
#define HDMIRX_PHY_b3_reg       		(unsigned int)(&hd21_aphy[nport]->b3)//DA5C             
#define HDMIRX_PHY_b4_reg      		 (unsigned int)(&hd21_aphy[nport]->b4)//DA60             
#define HDMIRX_PHY_acdr_b0_reg  	(unsigned int)(&hd21_aphy[nport]->acdr_b0)//DA64        
#define HDMIRX_PHY_acdr_b1_reg  	(unsigned int)(&hd21_aphy[nport]->acdr_b1)//DA68        
#define HDMIRX_PHY_acdr_b2_reg 	 (unsigned int)(&hd21_aphy[nport]->acdr_b2)//DA6C        
#define HDMIRX_PHY_g0_reg       		(unsigned int)(&hd21_aphy[nport]->g0)//DA70             
#define HDMIRX_PHY_g1_reg       		(unsigned int)(&hd21_aphy[nport]->g1)//DA74             
#define HDMIRX_PHY_g2_reg      		(unsigned int)(&hd21_aphy[nport]->g2)//DA78             
#define HDMIRX_PHY_g3_reg       		(unsigned int)(&hd21_aphy[nport]->g3)//DA7C             
#define HDMIRX_PHY_g4_reg			(unsigned int)(&hd21_aphy[nport]->g4)//DA80             
#define HDMIRX_PHY_acdr_g0_reg  	(unsigned int)(&hd21_aphy[nport]->acdr_g0)//DA84        
#define HDMIRX_PHY_acdr_g1_reg		(unsigned int)(&hd21_aphy[nport]->acdr_g1)//DA88        
#define HDMIRX_PHY_acdr_g2_reg		(unsigned int)(&hd21_aphy[nport]->acdr_g2)//DA8C        
#define HDMIRX_PHY_r0_reg			(unsigned int)(&hd21_aphy[nport]->r0)//DA90             
#define HDMIRX_PHY_r1_reg			(unsigned int)(&hd21_aphy[nport]->r1)//DA94             
#define HDMIRX_PHY_r2_reg			(unsigned int)(&hd21_aphy[nport]->r2)//DA98             
#define HDMIRX_PHY_r3_reg			(unsigned int)(&hd21_aphy[nport]->r3)//DA9C             
#define HDMIRX_PHY_r4_reg			(unsigned int)(&hd21_aphy[nport]->r4)//DAA0             
#define HDMIRX_PHY_acdr_r0_reg		(unsigned int)(&hd21_aphy[nport]->acdr_r0)//DAA4        
#define HDMIRX_PHY_acdr_r1_reg		(unsigned int)(&hd21_aphy[nport]->acdr_r1)//DAA8        
#define HDMIRX_PHY_acdr_r2_reg		(unsigned int)(&hd21_aphy[nport]->acdr_r2)//DAAC        
#define HDMIRX_PHY_rsvd2_reg		(unsigned int)(&hd21_aphy[nport]->rsvd2)//DAB0          
#define HDMIRX_PHY_ck_cmu_1_reg	(unsigned int)(&hd21_aphy[nport]->ck_cmu_1)//DAB4      
#define HDMIRX_PHY_ck_cmu_2_reg	(unsigned int)(&hd21_aphy[nport]->ck_cmu_2)//DAB8       
#define HDMIRX_PHY_ck_cmu_3_reg	(unsigned int)(&hd21_aphy[nport]->ck_cmu_3)//DABC       
#define HDMIRX_PHY_ck_cmu_4_reg	(unsigned int)(&hd21_aphy[nport]->ck_cmu_4)//DAC0


struct hdmi21_aphy_rstb_reg_st {
	uint32_t rega00;  //DAFC	
};

extern const struct hdmi21_aphy_rstb_reg_st *hd21_aphy_rstb[4];

#define HDMIRX_PHY_rega00	(unsigned int)(&hd21_aphy_rstb[nport]->rega00)//DAFC	                

/////////////////////////////////////////////////

struct hdmi21_dphy_reg_st {
	
	uint32_t cdr_regd00;  //DC00
	uint32_t cdr_regd01;  //DC04
	uint32_t cdr_regd02;  //DC08
	uint32_t cdr_regd03;  //DC0C
//CDR_B	
	uint32_t cdr_regd04;  //DC10
//CDR_G	
	uint32_t cdr_regd05;  //DC14
//CDR_R	
	uint32_t cdr_regd06;  //DC18
//CDR_CK	
	uint32_t cdr_regd07;  //DC1C
//CDR_B		
	uint32_t cdr_regd08;  //DC20
//CDR_G
	uint32_t cdr_regd09;  //DC24
//CDR_R	
	uint32_t cdr_regd10;  //DC28
//CDR_CK	
	uint32_t cdr_regd11;  //DC2C
//CDR_B	
	uint32_t cdr_regd12;  //DC30
//CDR_G	
	uint32_t cdr_regd13;  //DC34
//CDR_R	
	uint32_t cdr_regd14;  //DC38
//CDR_CK	
	uint32_t cdr_regd15;  //DC3C
	
	uint32_t clk_regd00;  //DC40
       //KOFF B
	uint32_t koff_regd00;  //DC44
	uint32_t koff_regd01;  //DC48
	uint32_t koff_regd02;  //DC4C
	uint32_t koff_regd03;  //DC50
       //KOFF G	
	uint32_t koff_regd04;  //DC54
	uint32_t koff_regd05;  //DC58
	uint32_t koff_regd06;  //DC5C
	uint32_t koff_regd07;  //DC60
       //KOFF R	
	uint32_t koff_regd08;  //DC64
	uint32_t koff_regd09;  //DC68
	uint32_t koff_regd10;  //DC6C
	uint32_t koff_regd11;  //DC70
       //KOFF CK	
	uint32_t koff_regd12;  //DC74
	uint32_t koff_regd13;  //DC78
	uint32_t koff_regd14; //DC7C
	uint32_t koff_regd15; //DC80
	
	uint32_t koff_regd16; //DC84
	uint32_t data_regd00; //DC88
	uint32_t data_regd01; //DC8C
	uint32_t data_regd02; //DC90
	uint32_t data_regd03; //DC94
	
	uint32_t data_regd04; //DC98
	
	uint32_t data_regd05;  //DC9C
	uint32_t data_regd06;  //DCA0
	uint32_t data_regd07;  //DCA4
	uint32_t data_regd08;  //DCA8
	uint32_t data_regd09;  //DCAC
	uint32_t data_regd10;  //DCB0
	uint32_t rsvd4;  //DCB4
	//FLD_B
	uint32_t fld_regd00;  //DCB8
	uint32_t fld_regd01;  //DCBC
	uint32_t fld_regd02;  //DCC0
	uint32_t fld_regd03;  //DCC4
	uint32_t fld_regd04;  //DCC8
	uint32_t fld_regd05;  //DCCC
	uint32_t fld_regd06;  //DCD0
	
	uint32_t rsvd5;  //DCD4
	//FLD_G
	uint32_t fld_regd07;  //DCD8
	uint32_t fld_regd08;  //DCDC	
	uint32_t fld_regd09;  //DCE0		
	uint32_t fld_regd10;  //DCE4		
	uint32_t fld_regd11;  //DCE8		
	uint32_t fld_regd12;  //DCEC		
	uint32_t fld_regd13;  //DCF0		
	//FLD_R	
	uint32_t fld_regd14;  //DCF4
	uint32_t fld_regd15;  //DCF8		
	uint32_t fld_regd16;  //DCFC		
	uint32_t fld_regd17;  //DD00		
	uint32_t fld_regd18;  //DD04		
	uint32_t fld_regd19;  //DD08		
	uint32_t fld_regd20;  //DD0C		
	//FLD_CK	
	uint32_t fld_regd21;  //DD10		
	uint32_t fld_regd22;  //DD14		
	uint32_t fld_regd23;  //DD18		
	uint32_t fld_regd24;  //DD1C		
	uint32_t fld_regd25;  //DD20		
	uint32_t fld_regd26;  //DD24		
	uint32_t fld_regd27;  //DD28
	
	uint32_t mod_regd00;  //DD2C	
	uint32_t mod_regd01;  //DD30	
	uint32_t pllcdr_regd00;  //DD34	
	uint32_t pllcdr_regd01;  //DD38	
	//PLLCDR_B	
	uint32_t pllcdr_regd02;  //DD3C	
	uint32_t pllcdr_regd03;  //DD40
	uint32_t pllcdr_regd04;  //DD44
	//PLLCDR_G	
	uint32_t pllcdr_regd05;  //DD48
	uint32_t pllcdr_regd06;  //DD4C
	uint32_t pllcdr_regd07;  //DD50
	//PLLCDR_R	
	uint32_t pllcdr_regd08;  //DD54
	uint32_t pllcdr_regd09;  //DD58
	uint32_t pllcdr_regd10;  //DD5C
	//PLLCDR_CK	
	uint32_t pllcdr_regd11;  //DD60
	uint32_t pllcdr_regd12;  //DD64
	uint32_t pllcdr_regd13;  //DD68
	
	uint32_t psave_regd00;  //DD6C
	uint32_t dfe_regd00;  //DD70
	uint32_t dfe_regd01;  //DD74
	uint32_t dfe_regd02;  //DD78
	uint32_t misc_regd00;  //DD7C
	uint32_t misc_regd01;  //DD80
	uint32_t misc_regd02;  //DD84
};


extern const struct hdmi21_dphy_reg_st *hd21_dphy[4];


/////////////////////////////////////////////////////

                                                                                            
#define HDMIRX_PHY_cdr_regd00				(unsigned int)(&hd21_dphy[nport]->cdr_regd00)//DC00         
#define HDMIRX_PHY_cdr_regd01				(unsigned int)(&hd21_dphy[nport]->cdr_regd01)//DC04         
#define HDMIRX_PHY_cdr_regd02				(unsigned int)(&hd21_dphy[nport]->cdr_regd02)//DC08         
#define HDMIRX_PHY_cdr_regd03				(unsigned int)(&hd21_dphy[nport]->cdr_regd03)//DC0C         
#define HDMIRX_PHY_cdr_regd04				(unsigned int)(&hd21_dphy[nport]->cdr_regd04)//DC10         
#define HDMIRX_PHY_cdr_regd05				(unsigned int)(&hd21_dphy[nport]->cdr_regd05)//DC14         
#define HDMIRX_PHY_cdr_regd06				(unsigned int)(&hd21_dphy[nport]->cdr_regd06)//DC18         
#define HDMIRX_PHY_cdr_regd07				(unsigned int)(&hd21_dphy[nport]->cdr_regd07)//DC1C         
#define HDMIRX_PHY_cdr_regd08				(unsigned int)(&hd21_dphy[nport]->cdr_regd08)//DC20         
#define HDMIRX_PHY_cdr_regd09				(unsigned int)(&hd21_dphy[nport]->cdr_regd09)//DC24         
#define HDMIRX_PHY_cdr_regd10				(unsigned int)(&hd21_dphy[nport]->cdr_regd10)//DC28         
#define HDMIRX_PHY_cdr_regd11				(unsigned int)(&hd21_dphy[nport]->cdr_regd11)//DC2C         
#define HDMIRX_PHY_cdr_regd12				(unsigned int)(&hd21_dphy[nport]->cdr_regd12)//DC30         
#define HDMIRX_PHY_cdr_regd13				(unsigned int)(&hd21_dphy[nport]->cdr_regd13)//DC34         
#define HDMIRX_PHY_cdr_regd14				(unsigned int)(&hd21_dphy[nport]->cdr_regd14)//DC38         
#define HDMIRX_PHY_cdr_regd15				(unsigned int)(&hd21_dphy[nport]->cdr_regd15)//DC3C         
#define HDMIRX_PHY_clk_regd00				(unsigned int)(&hd21_dphy[nport]->clk_regd00)//DC40         
//KOFF B                                                                                            
#define HDMIRX_PHY_koff_regd00				(unsigned int)(&hd21_dphy[nport]->koff_regd00)//DC44        
#define HDMIRX_PHY_koff_regd01				(unsigned int)(&hd21_dphy[nport]->koff_regd01)//DC48        
#define HDMIRX_PHY_koff_regd02				(unsigned int)(&hd21_dphy[nport]->koff_regd02)//DC4C        
#define HDMIRX_PHY_koff_regd03				(unsigned int)(&hd21_dphy[nport]->koff_regd03)//DC50        
 //KOFF G                                                                                           
#define HDMIRX_PHY_koff_regd04				(unsigned int)(&hd21_dphy[nport]->koff_regd04)//DC54        
#define HDMIRX_PHY_koff_regd05				(unsigned int)(&hd21_dphy[nport]->koff_regd05)//DC58        
#define HDMIRX_PHY_koff_regd06				(unsigned int)(&hd21_dphy[nport]->koff_regd06)//DC5C        
#define HDMIRX_PHY_koff_regd07				(unsigned int)(&hd21_dphy[nport]->koff_regd07)//DC60        
 //KOFF R                                                                                           
#define HDMIRX_PHY_koff_regd08				(unsigned int)(&hd21_dphy[nport]->koff_regd08)//DC64        
#define HDMIRX_PHY_koff_regd09				(unsigned int)(&hd21_dphy[nport]->koff_regd09)//DC68        
#define HDMIRX_PHY_koff_regd10				(unsigned int)(&hd21_dphy[nport]->koff_regd10)//DC6C        
#define HDMIRX_PHY_koff_regd11				(unsigned int)(&hd21_dphy[nport]->koff_regd11)//DC70        
 //KOFF CK                                                                                           
#define HDMIRX_PHY_koff_regd12				(unsigned int)(&hd21_dphy[nport]->koff_regd12)//DC74        
#define HDMIRX_PHY_koff_regd13				(unsigned int)(&hd21_dphy[nport]->koff_regd13)//DC78        
#define HDMIRX_PHY_koff_regd14				(unsigned int)(&hd21_dphy[nport]->koff_regd14)//DC7C         
#define HDMIRX_PHY_koff_regd15				(unsigned int)(&hd21_dphy[nport]->koff_regd15)//DC80         
                                                                                            
#define HDMIRX_PHY_koff_regd16				(unsigned int)(&hd21_dphy[nport]->koff_regd16)//DC84         
#define HDMIRX_PHY_data_regd00				(unsigned int)(&hd21_dphy[nport]->data_regd00)//DC88         
#define HDMIRX_PHY_data_regd01				(unsigned int)(&hd21_dphy[nport]->data_regd01)//DC8C         
#define HDMIRX_PHY_data_regd02				(unsigned int)(&hd21_dphy[nport]->data_regd02)//DC90         
#define HDMIRX_PHY_data_regd03				(unsigned int)(&hd21_dphy[nport]->data_regd03)//DC94         
                                                                                            
#define HDMIRX_PHY_data_regd04				(unsigned int)(&hd21_dphy[nport]->data_regd04)//DC98         
                           
#define HDMIRX_PHY_data_regd05				(unsigned int)(&hd21_dphy[nport]->data_regd05)//DC9C        
#define HDMIRX_PHY_data_regd06				(unsigned int)(&hd21_dphy[nport]->data_regd06)//DCA0        
#define HDMIRX_PHY_data_regd07				(unsigned int)(&hd21_dphy[nport]->data_regd07)//DCA4        
#define HDMIRX_PHY_data_regd08				(unsigned int)(&hd21_dphy[nport]->data_regd08)//DCA8        
#define HDMIRX_PHY_data_regd09				(unsigned int)(&hd21_dphy[nport]->data_regd09)//DCAC        
#define HDMIRX_PHY_data_regd10				(unsigned int)(&hd21_dphy[nport]->data_regd10)//DCB0        
#define HDMIRX_PHY_rsvd4					(unsigned int)(&hd21_dphy[nport]->rsvd4)//DCB4                    
 //FLD_B                                                                                            
#define HDMIRX_PHY_fld_regd00				(unsigned int)(&hd21_dphy[nport]->fld_regd00)//DCB8          
#define HDMIRX_PHY_fld_regd01				(unsigned int)(&hd21_dphy[nport]->fld_regd01)//DCBC          
#define HDMIRX_PHY_fld_regd02				(unsigned int)(&hd21_dphy[nport]->fld_regd02)//DCC0          
#define HDMIRX_PHY_fld_regd03				(unsigned int)(&hd21_dphy[nport]->fld_regd03)//DCC4          
#define HDMIRX_PHY_fld_regd04				(unsigned int)(&hd21_dphy[nport]->fld_regd04)//DCC8          
#define HDMIRX_PHY_fld_regd05				(unsigned int)(&hd21_dphy[nport]->fld_regd05)//DCCC          
#define HDMIRX_PHY_fld_regd06				(unsigned int)(&hd21_dphy[nport]->fld_regd06)//DCD0          
                                                                                            
#define HDMIRX_PHY_rsvd5					(unsigned int)(&hd21_dphy[nport]->rsvd5)//DCD4                    
 //FLD_G                                                                                           
#define HDMIRX_PHY_fld_regd07				(unsigned int)(&hd21_dphy[nport]->fld_regd07)//DCD8          
#define HDMIRX_PHY_fld_regd08				(unsigned int)(&hd21_dphy[nport]->fld_regd08)//DCDC          
#define HDMIRX_PHY_fld_regd09				(unsigned int)(&hd21_dphy[nport]->fld_regd09)//DCE0		      
#define HDMIRX_PHY_fld_regd10				(unsigned int)(&hd21_dphy[nport]->fld_regd10)//DCE4		      
#define HDMIRX_PHY_fld_regd11				(unsigned int)(&hd21_dphy[nport]->fld_regd11)//DCE8		      
#define HDMIRX_PHY_fld_regd12				(unsigned int)(&hd21_dphy[nport]->fld_regd12)//DCEC		      
#define HDMIRX_PHY_fld_regd13				(unsigned int)(&hd21_dphy[nport]->fld_regd13)//DCF0		      
 //FLD_R                                                                                            
#define HDMIRX_PHY_fld_regd14				(unsigned int)(&hd21_dphy[nport]->fld_regd14)//DCF4          
#define HDMIRX_PHY_fld_regd15				(unsigned int)(&hd21_dphy[nport]->fld_regd15)//DCF8		      
#define HDMIRX_PHY_fld_regd16				(unsigned int)(&hd21_dphy[nport]->fld_regd16)//DCFC		      
#define HDMIRX_PHY_fld_regd17				(unsigned int)(&hd21_dphy[nport]->fld_regd17)//DD00          
#define HDMIRX_PHY_fld_regd18				(unsigned int)(&hd21_dphy[nport]->fld_regd18)//DD04		      
#define HDMIRX_PHY_fld_regd19				(unsigned int)(&hd21_dphy[nport]->fld_regd19)//DD08		      
#define HDMIRX_PHY_fld_regd20				(unsigned int)(&hd21_dphy[nport]->fld_regd20)//DD0C		      
 //FLD_CK                                                                                           
#define HDMIRX_PHY_fld_regd21				(unsigned int)(&hd21_dphy[nport]->fld_regd21)//DD10		      
#define HDMIRX_PHY_fld_regd22				(unsigned int)(&hd21_dphy[nport]->fld_regd22)//DD14		      
#define HDMIRX_PHY_fld_regd23				(unsigned int)(&hd21_dphy[nport]->fld_regd23)//DD18		      
#define HDMIRX_PHY_fld_regd24				(unsigned int)(&hd21_dphy[nport]->fld_regd24)//DD1C		      
#define HDMIRX_PHY_fld_regd25				(unsigned int)(&hd21_dphy[nport]->fld_regd25)//DD20		      
#define HDMIRX_PHY_fld_regd26				(unsigned int)(&hd21_dphy[nport]->fld_regd26)//DD24          
#define HDMIRX_PHY_fld_regd27				(unsigned int)(&hd21_dphy[nport]->fld_regd27)//DD28          
                                                                                            
#define HDMIRX_PHY_mod_regd00				(unsigned int)(&hd21_dphy[nport]->mod_regd00)//DD2C	        
#define HDMIRX_PHY_mod_regd01				(unsigned int)(&hd21_dphy[nport]->mod_regd01)//DD30	        
#define HDMIRX_PHY_pllcdr_regd00			(unsigned int)(&hd21_dphy[nport]->pllcdr_regd00)//DD34	  
#define HDMIRX_PHY_pllcdr_regd01			(unsigned int)(&hd21_dphy[nport]->pllcdr_regd01)//DD38	  
                                                                                            
#define HDMIRX_PHY_pllcdr_regd02			(unsigned int)(&hd21_dphy[nport]->pllcdr_regd02)//DD3C	  
#define HDMIRX_PHY_pllcdr_regd03			(unsigned int)(&hd21_dphy[nport]->pllcdr_regd03)//DD40    
#define HDMIRX_PHY_pllcdr_regd04			(unsigned int)(&hd21_dphy[nport]->pllcdr_regd04)//DD44    
                                                                                            
#define HDMIRX_PHY_pllcdr_regd05			(unsigned int)(&hd21_dphy[nport]->pllcdr_regd05)//DD48    
#define HDMIRX_PHY_pllcdr_regd06			(unsigned int)(&hd21_dphy[nport]->pllcdr_regd06)//DD4C    
#define HDMIRX_PHY_pllcdr_regd07			(unsigned int)(&hd21_dphy[nport]->pllcdr_regd07)//DD50    
                                                                                            
#define HDMIRX_PHY_pllcdr_regd08			(unsigned int)(&hd21_dphy[nport]->pllcdr_regd08)//DD54    
#define HDMIRX_PHY_pllcdr_regd09			(unsigned int)(&hd21_dphy[nport]->pllcdr_regd09)//DD58    
#define HDMIRX_PHY_pllcdr_regd10			(unsigned int)(&hd21_dphy[nport]->pllcdr_regd10)//DD5C    
                                                                                            
#define HDMIRX_PHY_pllcdr_regd11			(unsigned int)(&hd21_dphy[nport]->pllcdr_regd11)//DD60    
#define HDMIRX_PHY_pllcdr_regd12			(unsigned int)(&hd21_dphy[nport]->pllcdr_regd12)//DD64    
#define HDMIRX_PHY_pllcdr_regd13			(unsigned int)(&hd21_dphy[nport]->pllcdr_regd13)//DD68    
                                                                                            
#define HDMIRX_PHY_psave_regd00			(unsigned int)(&hd21_dphy[nport]->psave_regd00)//DD6C      
#define HDMIRX_PHY_dfe_regd00			(unsigned int)(&hd21_dphy[nport]->dfe_regd00)//DD70          
#define HDMIRX_PHY_dfe_regd01			(unsigned int)(&hd21_dphy[nport]->dfe_regd01)//DD74          
#define HDMIRX_PHY_dfe_regd02			(unsigned int)(&hd21_dphy[nport]->dfe_regd02)//DD78          
#define HDMIRX_PHY_misc_regd00			(unsigned int)(&hd21_dphy[nport]->misc_regd00)//DD7C        
#define HDMIRX_PHY_misc_regd01			(unsigned int)(&hd21_dphy[nport]->misc_regd01)//DD80        
#define HDMIRX_PHY_misc_regd02			(unsigned int)(&hd21_dphy[nport]->misc_regd02)//DD84        


#define Z0_Z0POW 					(_BIT12|_BIT8|_BIT4|_BIT0)

#define DFE_TAP_ICOM_EN 			(_BIT28)
#define DFE_TAP_EN 					(_BIT3|_BIT4|_BIT5|_BIT6)

#define DFE_ADAPTION_POW_EN 		(_BIT7)
#define HD21_RX_PHY_P0_DFE_ADAPTION_EN(data)							(0x00000080&((data)<<7))

#define LANE_IPNUT_OFF                       (_BIT0)
#define POW_CMFB_1P8_CDM                 (_BIT0)

#define POW_LEQ 			(_BIT24)
#define POW_LEQ_KOFF 			(_BIT23)
#define bPOW_NC 			(_BIT22)



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



#define TOP_IN_2_P0_FS_SIGNAL_DLY_TIME	(_BIT12|_BIT11|_BIT10)
#define TOP_IN_2_P0_FS_SIGNAL_DLY_TIME_100	(_BIT10)
#define TOP_IN_2_P0_FS_SIGNAL_DLY	(_BIT9)
#define TOP_IN_2_P0_FS_SIGNAL_SEL	(_BIT8)


#define Divider_Enable				(_BIT23)
#define AC_Couple_Enable			(_BIT21)
#define CK_Detect_Enable			(_BIT20)
#define Hysteresis_Window_Sel	(_BIT19|_BIT18)
#define BG_ENVBGUP					(_BIT17)      //h5
#define BG_RBG2					(_BIT15|_BIT14)   //h5
#define BG_RBG2_DEFAULT			(_BIT15)       //h5
#define BG_RBG					(_BIT13|_BIT12|_BIT11)    //h5
#define BG_RBG_DEFAULT			(_BIT13)   //h5
#define BG_RBGLOOP2				(_BIT10|_BIT9|_BIT8)  //h5
#define BG_RBGLOOP2_DEFAULT		(_BIT10)    //h5
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

#define  RXVCM_SEL							(_BIT6|_BIT5)		//Mark2
#define  RXVCM_10							(_BIT6)   //Mark2



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
#define  b_7_PI_ISEL                                                                                              (_BIT16|_BIT17)  //h5
#define  b_6_EQ_POW															(_BIT9)			//h5
#define  b_6_RS_CAL_EN														(_BIT8)			//h5
//#define  b_6_LEQ_BIT_RATE                                                                                    (_BIT14|_BIT15)  //h5
//#define  b_6_LEQ_BIT_RATE_HBR                                                                            (_BIT15)  //h5
//#define  b_6_LEQ_BIT_RATE_MBR                                                                            (_BIT14)  //h5
//#define  b_6_TAP0_HBR                                                                              	        (_BIT13)  //h5
//#define  b_6_LEQ_PASSIVE_CORNER                                                                      (_BIT12|_BIT11)
//#define  b_6_TAP0_LE_ISO							(_BIT8)


#define  RS_CAL_EN					   (_BIT21)			//Mark2
#define  POW_LEQ					   (_BIT24)			//Mark2



#define  b_5_TRANS_CNT_EN													 (_BIT1)    //h5
#define  b_5_AC_COPULE_POW                                                                                            (_BIT0)    //h5

#define  AC_COPULE_POW       (_BIT3)    //Mark2


#define b_9_DFE_ADAPTION_POW_EN 								              (_BIT7)
#define b_9_DFE_TAP_EN 								                           (_BIT3|_BIT4|_BIT5|_BIT6)
#define b_9_DFE_TAP1_EN 								                           (_BIT3)
#define HD21_RX_PHY_P0_DFE_ADAPTION_EN(data)							(0x00000080&((data)<<7))
#define HD21_RX_PHY_P0_b_tap_en(data)								(0x00000078&((data)<<3))


#define acdr_b_8_FS_SIGNAL_DLY_EN				(_BIT26)  //merlin5
#define acdr_b_8_FS_EN							(_BIT25)  //merlin5
#define acdr_b_8_DELAY_CELL_SEL				(_BIT24)  //h5


#define acdr_b_7_FS_SIGNAL_DLY_EN				(_BIT7)  //mark2
#define acdr_b_6_FS_EN							(_BIT6)  //mark2

#define acdr_g_7_FS_SIGNAL_DLY_EN				(_BIT7)  //mark2
#define acdr_g_6_FS_EN							(_BIT6)  //mark2

#define acdr_r_7_FS_SIGNAL_DLY_EN				(_BIT7)  //mark2
#define acdr_r_6_FS_EN							(_BIT6)  //mark2

#define acdr_ck_7_FS_SIGNAL_DLY_EN			(_BIT7)  //mark2
#define acdr_ck_6_FS_EN							(_BIT6)  //mark2

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


#define acdr_manual_RDY		(_BIT30)  //Mark2
#define acdr_manual_RDY_FROM_REG	(_BIT30)  //Mark2
#define acdr_manual_CMU_SEL	(_BIT31)  //Mark2





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
#define  ck_7_PI_ISEL                                                                                              (_BIT16|_BIT17)  //h5

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
#define  g_7_PI_ISEL                                                                                              (_BIT16|_BIT17)  //h5
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
#define  r_7_PI_ISEL                                                                                              (_BIT16|_BIT17)  //h5
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


#define acdr_PFD_RSTB						(_BIT0)  //Mark2
#define acdr_DIV_IQ_RSTB					(_BIT16)  //Mark2
#define acdr_VCO_EN					              (_BIT8)  //Mark2


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


