
#include <rbus/tv_sb1_ana_reg.h>
#include <tvscalercontrol/vdac/vdac.h>
//#include <rbus/adc_reg.h>//Fix Me. ADC register spec out

void drvif_vdac_init(void)
{
	tv_sb1_ana_apll_adc_vadc_idaco_ctrl0_RBUS tv_sb1_ana_apll_adc_vadc_idaco_ctrl0_reg;
	tv_sb1_ana_apll_adc_vadc_idaco_ctrl0_reg.regValue = IoReg_Read32(TV_SB1_ANA_APLL_ADC_VADC_IDACO_CTRL0_reg);

	tv_sb1_ana_apll_adc_vadc_idaco_ctrl0_reg.reg_vid_mbias_ibxsel = 0x2;	//[1:0]REG_VID_MBIAS_IBXSEL = 10
	tv_sb1_ana_apll_adc_vadc_idaco_ctrl0_reg.reg_vid_powvdac = 1;		//[4]REG_VID_POWVDAC = 1
	tv_sb1_ana_apll_adc_vadc_idaco_ctrl0_reg.reg_vid_vdac_dacisel = 0x3;	//[10:8]REG_VID_VDAC_DACISEL = 011
	tv_sb1_ana_apll_adc_vadc_idaco_ctrl0_reg.reg_vid_vdac_ckmode = 0;	//[12]REG_VID_VDAC_CKMODE = 0
	tv_sb1_ana_apll_adc_vadc_idaco_ctrl0_reg.reg_vid_powmbias = 1;		//[16]REG_VID_POWMBIAS = 1

	IoReg_Write32(TV_SB1_ANA_APLL_ADC_VADC_IDACO_CTRL0_reg,tv_sb1_ana_apll_adc_vadc_idaco_ctrl0_reg.regValue);
	rtd_outl(TV_SB1_ANA_VDAC_SRC_SEL_reg, 0x00000013); //[4]REG_VDAC_CLK_POLAR, [2:0]REG_VDAC_SRC_SEL, 0:from VD, 3:from ADC2X_DOUT
	return;
}

void drvif_vdac_tvdac_power_on(unsigned char enable)
{
	tv_sb1_ana_apll_adc_vadc_idaco_ctrl0_RBUS tv_sb1_ana_apll_adc_vadc_idaco_ctrl0_reg;
	tv_sb1_ana_apll_adc_vadc_idaco_ctrl0_reg.regValue = IoReg_Read32(TV_SB1_ANA_APLL_ADC_VADC_IDACO_CTRL0_reg);
	if(enable) {
		tv_sb1_ana_apll_adc_vadc_idaco_ctrl0_reg.reg_vid_powvdac = 1;
		tv_sb1_ana_apll_adc_vadc_idaco_ctrl0_reg.reg_vid_powmbias = 1;
	}
	else {
		tv_sb1_ana_apll_adc_vadc_idaco_ctrl0_reg.reg_vid_powvdac = 0;
		tv_sb1_ana_apll_adc_vadc_idaco_ctrl0_reg.reg_vid_powmbias = 0;
	}

	IoReg_Write32(TV_SB1_ANA_APLL_ADC_VADC_IDACO_CTRL0_reg,tv_sb1_ana_apll_adc_vadc_idaco_ctrl0_reg.regValue);
}
void drvif_vdac_yppdac_power_on(unsigned char enable)
{
//remove on mac3
    /*
    	VDAC_YPP_CTRL3_RBUS VDAC_TV_CTRL3_reg;
    	VDAC_TV_CTRL3_reg.regValue = IoReg_Read32(TV_SB1_ANA_VDAC_YPP_CTRL3_reg);
    	if(enable)
    		VDAC_TV_CTRL3_reg.reg_vdac_vdac_en= 0x7;
    	else
    		VDAC_TV_CTRL3_reg.reg_vdac_vdac_en = 0;
    	IoReg_Write32(TV_SB1_ANA_VDAC_YPP_CTRL3_reg,VDAC_TV_CTRL3_reg.regValue);
    */
}
void drvif_vdac_source_sel(VDAC_TV_SRC_SEL sel)
{
    //fix me;
    tv_sb1_ana_vdac_src_sel_RBUS VDAC_SRC_SEL_reg;
    VDAC_SRC_SEL_reg.regValue=IoReg_Read32(TV_SB1_ANA_VDAC_SRC_SEL_reg);
    VDAC_SRC_SEL_reg.reg_vdac_src_sel=sel;
    IoReg_Write32(TV_SB1_ANA_VDAC_SRC_SEL_reg,VDAC_SRC_SEL_reg.regValue);
}

