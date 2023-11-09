#ifndef _MC_LBMC_SWITCH_DEF_H
#define _MC_LBMC_SWITCH_DEF_H

enum _LBMC_MODE_ {_MC_NORMAL_=0, _MC_SINGLE_UP_, _MC_SINGLE_DOWN_, _MC_LBMC_MODE_NUM_};
enum _LBMC_BAND_ {_LBMC_LF_=0, _LBMC_HF_, _LBMC_BAND_NUM_};
typedef struct  
{
	unsigned char  u8_lbmcSwitch_bypass;

	unsigned char  u8_lf_Ovlap_s2n_cntTh;
	unsigned char  u8_lf_Lmv_s2n_cntTh;
	unsigned char  u8_lf_Ovlap_n2s_cntTh;
	unsigned char  u8_lf_Lmv_n2s_cntTh;

	unsigned char  u8_hf_Ovlap_s2n_cntTh;
	unsigned char  u8_hf_Lmv_s2n_cntTh;
	unsigned char  u8_hf_Ovlap_n2s_cntTh;
	unsigned char  u8_hf_Lmv_n2s_cntTh;

}_PARAM_MC_LBMC_SWITCH;

typedef struct  
{	
	unsigned char  u8_lf_lbmcMode;
	unsigned char  u8_hf_lbmcMode;
	
}_OUTPUT_MC_LBMC_SWITCH;

VOID Mc_lbmcSwitch_Init(_OUTPUT_MC_LBMC_SWITCH *pOutput);
VOID Mc_lbmcSwitch_Proc(const _PARAM_MC_LBMC_SWITCH *pParam, _OUTPUT_MC_LBMC_SWITCH *pOutput);

/////////////////////////////////////////////////////////////////////////////
DEBUGMODULE_BEGIN(_PARAM_MC_LBMC_SWITCH,_OUTPUT_MC_LBMC_SWITCH)
    ADD_PARAM_DES(unsigned char, u8_lbmcSwitch_bypass,    1, "0:PQL, 1:nor, 2:sigUp, 3:sigDn")

    ADD_DUMMY_PARAM()
	ADD_PARAM_DES(unsigned char, u8_lf_Ovlap_s2n_cntTh,   248, "u0.8, <, single 2 normal")
	ADD_PARAM_DES(unsigned char, u8_lf_Lmv_s2n_cntTh,      10, " < ")
	ADD_PARAM_DES(unsigned char, u8_lf_Ovlap_n2s_cntTh,   252, "u0.8, >, normal 2 single")
	ADD_PARAM_DES(unsigned char, u8_lf_Lmv_n2s_cntTh,      20, " > ")

	ADD_DUMMY_PARAM()
	ADD_PARAM_DES(unsigned char, u8_hf_Ovlap_s2n_cntTh,   248, "u0.8, <, single 2 normal")
	ADD_PARAM_DES(unsigned char, u8_hf_Lmv_s2n_cntTh,      10, " < ")
	ADD_PARAM_DES(unsigned char, u8_hf_Ovlap_n2s_cntTh,   252, "u0.8, >, normal 2 single")
	ADD_PARAM_DES(unsigned char, u8_hf_Lmv_n2s_cntTh,      20, " > ")

	// output
	ADD_OUTPUT(unsigned char,  u8_lf_lbmcMode)
    ADD_OUTPUT(unsigned char,  u8_hf_lbmcMode)

	DEBUGMODULE_END

#endif
