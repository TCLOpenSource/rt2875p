#ifndef __DISEQC_REALTEK_H
#define __DISEQC_REALTEK_H


#ifdef __cplusplus
extern "C"{
#endif

//#define rtd_outl(addr, value)           pli_writeReg32(addr, value)
//#define rtd_inl(addr)                   pli_readReg32(addr)
//#define rtd_maskl(addr, mask, value)    rtd_outl(addr, ((rtd_inl(addr) & mask) | value))


int 
realtek_Diseqc_SetDiseqcContinue22kOnOff(
	unsigned int Diseqc22kOnOff
);

int 
realtek_Diseqc_SetDiseqcCmd(
	unsigned int DataLength,
	unsigned char *Data
);

int 
realtek_Diseqc_SetDiseqc2p0Cmd(
	unsigned int *DataLength,
	unsigned char *Data
);

int
realtek_Diseqc_SetDiseqcUnModToneBurst(void);

int
realtek_Diseqc_SetDiseqcModToneBurst(void);

int
realtek_Diseqc_SetDiseqcToneBurst(DISEQC_TONE_MODE mode);

int
realtek_Lnb_Dc_Enable(unsigned char enable);

void
realtek_diseqc_PrintTxCmd(unsigned int DataLength);

void
realtek_diseqc_PrintRxCmd(unsigned int DataLength, unsigned char *Data);


#define REALTEK_DISEQC_PRINT_LVL_DBG      0
#define REALTEK_DISEQC_PRINT_LVL_INFO     1
#define REALTEK_DISEQC_PRINT_LVL_WARNING  2


#ifndef REALTEK_DISEQC_PRINT_LVL
#define REALTEK_DISEQC_PRINT_LVL          REALTEK_I_DISEQC_PRINT_LVL_DBG
#endif

#define REALTEK_DISEQC_DBG(fmt, args...)        rtd_demod_print(KERN_DEBUG,"DISEQC_DEBUG, " fmt, ##args);
#define REALTEK_DISEQC_INFO(fmt, args...)       rtd_demod_print(KERN_INFO,"DISEQC_INFO, " fmt, ##args);
#define REALTEK_DISEQC_WARNING(fmt, args...)    rtd_demod_print(KERN_WARNING,"DISEQC_WARNING, " fmt, ##args);


#ifdef __cplusplus
}
#endif
#endif
