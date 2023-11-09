#include <linux/string.h>
#include <linux/delay.h>
#include <io.h>
#include "dtmb_demod_rtk_type.h"
#include "dtmb_demod_rtk.h"
#include "dtmb_demod_rtk_user.h"
#include "dtmb_demod_base.h"
//#include <math.h>

#define DTMBIP_DEBUG_INFO_OPEN	        0
#define SYSTEM_NOT_SUPPORT_LOG10	    1
#define DTMBIP_SPECTRUM_ADJUST_OPEN	    0
#define FREQUENCY_SAMPLE                30.375
#define FREQUENCY_SYMBOL                7.56
//#define DTMB_S_Calculate
/**********************************************
6M SystemBandWidth switch
**********************************************/
//#define DTMBIP_BANDWIDTH_6M             0   //enable bandwidth 6M support turn off

/*********************************************
define RBUS commit
**********************************************/
/*
#define rtd_outl(addr, value)   pli_writeReg32(addr, value)
#define rtd_inl(addr)           pli_readReg32(addr)
#define rtd_maskl(addr, mask, value)    rtd_outl(addr, ((rtd_inl(addr) & mask) | value))
*/

//static UINT8 HD2311_WorkMode = HDIC2311_DTMB_SELECT;

#if SYSTEM_NOT_SUPPORT_LOG10
static UINT16 SNR_log_table_int[] = {
	0, 100,
	2, 104,
	4, 109,
	6, 114,
	8, 120,
	10, 125,
	12, 131,
	14, 138,
	16, 144,
	18, 151,
	20, 158,
	22, 166,
	24, 173,
	26, 182,
	28, 190,
	30, 199,
	32, 208
};

static UINT16 Left_Table(UINT16 InValue)
{
	UINT16 OutValue = 0;
	UINT16 TableSize = sizeof(SNR_log_table_int) / sizeof(UINT16);
	UINT16 i;
	for (i = 0; i < TableSize; i += 2) {
		if (InValue <= SNR_log_table_int[i + 1]) {
			OutValue = SNR_log_table_int[i];
			return OutValue;
		}
	}

	OutValue = 32;
	return OutValue;
}

static UINT32 Log10_Convert(UINT32 InValue)
{
	UINT32 u32_temp = 1;
	UINT8  indx = 0;
	UINT32 InValue_left;
	UINT32 log_ret;

	do {
		u32_temp = u32_temp << 1;
		if (InValue < u32_temp)
			break;
	} while (++indx < 31); //} while (++indx < 32); for coverity issue

	log_ret = (301 * indx + 50) / 100;
	InValue_left = InValue * 100 / (1 << indx);
	//log_ret = log_ret + Left_Table(InValue_left) / 10;
	log_ret = log_ret * 10000 + Left_Table(InValue_left) * 1000 ;
	return log_ret;
}
#endif



UINT8 DTMBIP_SetWorkMode()
{
	UINT8 err = DTMB_NO_ERROR;

		DTMBIP_WriteRegister(0x029F, 0x10);
		DTMBIP_WriteRegister(0x029C, 0x10);
		DTMBIP_WriteRegister(0x028A, 0xF7);
		DTMBIP_WriteRegister(0x0286, 0xF5);
		DTMBIP_WriteRegister(0x0280, 0xE7);
		DTMBIP_WriteRegister(0x02FA, 0x20);
		DTMBIP_WriteRegister(0x02FE, 0x40);
		DTMBIP_WriteRegister(0x02B2, 0x20);
		DTMBIP_WriteRegister(0x02EF, 0x30);
		DTMBIP_WriteRegister(0x0260, 0x00);
		DTMBIP_WriteRegister(0x0275, 0x01);
		DTMBIP_WriteRegister(0x0276, 0x3E);
        /*
        DTMBIP_WriteRegister(0x02e0, 0x90);//fix 482M
        DTMBIP_WriteRegister(0x02EA, 0x10);
        DTMBIP_WriteRegister(0x00cc, 0x06);//fix722Mhz
        DTMBIP_WriteRegister(0x0186, 0x01);
        DTMBIP_WriteRegister(0x0187, 0x80);

        DTMBIP_WriteRegister(0x02e0, 0xa0);
        DTMBIP_WriteRegister(0x0243, 0x02);
        DTMBIP_WriteRegister(0x0244, 0x07);
        DTMBIP_WriteRegister(0x0246, 0x88);
        DTMBIP_WriteRegister(0x0122, 0x9f);
		*/
		//fixed ch522MHz_3D
		//DTMBIP_WriteRegister(0x02EA, 0x14); //  default is 0x04
		DTMBIP_WriteRegister(0x0227, 0x04); //  default is 0x18
		DTMBIP_WriteRegister(0x0300, 0x08); //  set to 0x08
		DTMBIP_WriteRegister(0x0037, 0x42); //  default is 0x22
		DTMBIP_WriteRegister(0x0042, 0x10); //  default is 0x15
		DTMBIP_WriteRegister(0x0303, 0x00); //  set to 0x00
	    //fixed Guangzhou_482MHz
	    DTMBIP_WriteRegister(0x02E0, 0x90); //  default is 0x60
	    DTMBIP_WriteRegister(0x0243, 0x02); //  default is 0x04
	    DTMBIP_WriteRegister(0x0244, 0x07); //  default is 0x05
	    DTMBIP_WriteRegister(0x0246, 0x80); //  default is 0x78
	    //fixed Changsha_722MHz
	    DTMBIP_WriteRegister(0x00CC, 0x06); //  default is 0x0A
	    DTMBIP_WriteRegister(0x0186, 0x00); //  default is 0x03
	    DTMBIP_WriteRegister(0x0187, 0x80); //  default is 0xFB
	    
	
	return err;
}

/*********************************************
Add GetPER function
**********************************************/
//UINT8 DTMBIP_GetPer(double  *per)
UINT8 DTMBIP_GetPer(U32BITS  *per)
{
    UINT8 err = DTMB_NO_ERROR;
    UINT8 ReadData1 = 0, ReadData2 = 0;
	*per = 0;
	//*per_ber = 0;

    DTMBIP_ReadRegister(0x0814, &ReadData1);
    DTMBIP_ReadRegister(0x0813, &ReadData2);

    *per = (ReadData2  * 256 + ReadData1);
    //*per /= 32768;

       return err;
}

static U32BITS Cal_PER_BER_TABLE[] = 
{ 
// (*Per)*10000/32768 / PER_BER*10e7
0, 0, 	
10, 66, 	
20, 133, 	
30, 199, 	
40, 266, 	
50, 333, 	
60, 400, 	
70, 467, 	
80, 531, 	
90, 601, 	
100, 668, 	
200, 1343, 	
300, 2025, 	
400, 2714, 	
500, 3410, 	
600, 4114, 	
700, 4825, 	
800, 5544, 	
900, 6270, 	
1000, 7005, 	
2000, 14840, 
3000, 23710, 
4000, 33960, 
5000, 46080, 
6000, 60910, 
7000, 80020, 
8000, 107000,
9000, 153000,
/*
0.000, 0.000e+000, 
0.001, 6.652e-007, 
0.002, 1.331e-006, 
0.003, 1.998e-006, 
0.004, 2.665e-006, 
0.005, 3.333e-006, 
0.006, 4.001e-006, 
0.007, 4.671e-006, 
0.008, 5.341e-006, 
0.009, 6.011e-006, 
0.010, 6.682e-006, 
0.020, 1.343e-005, 
0.030, 2.025e-005, 
0.040, 2.714e-005, 
0.050, 3.410e-005, 
0.060, 4.114e-005, 
0.070, 4.825e-005, 
0.080, 5.544e-005, 
0.090, 6.270e-005, 
0.100, 7.005e-005, 
0.200, 1.484e-004, 
0.300, 2.371e-004, 
0.400, 3.396e-004, 
0.500, 4.608e-004, 
0.600, 6.091e-004, 
0.700, 8.002e-004, 
0.800, 1.070e-003, 
0.900, 1.530e-003, 
*/
}; 

static U32BITS Cal_PER_BEC_TABLE[] = 
{ 
// (*Per)*10000/32768 / PER_BER*32768*188*8
0, 0,
10, 32,
20, 65,
30, 98,
40, 131,
50, 164,
60, 197,
70, 230,
80, 263,
90, 296,
100, 329,
200, 661,
300, 998,
400, 1337,
500, 1680,
600, 2027,
700, 2377,
800, 2732,
900, 3090,
1000, 3452,
2000, 7313,
3000, 11685,
4000, 16736,
5000, 22709,
6000, 30018,
7000, 39436,
8000, 52732,
9000, 75403,
};


static U32BITS single_cn_dtmb[5][3] =    {{0,0,440000},
                                                                    {340000,520000,730000},
						                        {870000,1100000,1370000},
						                        {0,0,1670000},
						                        {1310000,1600000,1940000}};

static U32BITS mutile_cn_dtmb[5][3] =    {{0,0,440000},
						                        {340000,520000,740000},
						                        {930000,1120000,1380000},
						                        {0,0,1710000},
						                        {1390000,1640000,1970000}};

static int single_pref_dtmb[5][3] =  {{0,0,-954},{-964,-946,-925},{-911,-888,-861},{0,0,-831},{-867,-838,-804}};
static int mutile_pref_dtmb[5][3] = {{0,0,-954},{-964,-946,-924},{-905,-886,-860},{0,0,-827},{-859,-834,-801}};

UINT8 DTMBIP_GetBer(U32BITS *ber) 
{ //error numbers of bit
    UINT8 err = DTMB_NO_ERROR; 
    U32BITS per, per_temp; 
    //UINT8 TableSize = sizeof(PER_BER_TABLE) / sizeof(U32BITS); 
    UINT8 TableSize = sizeof(Cal_PER_BER_TABLE) / sizeof(U32BITS);
    UINT8 i; 

    err = DTMBIP_GetPer(&per); 
    per_temp = (per *10000)/32768;

    for (i = 0; i < TableSize; i += 2) 
    { 
        if (per_temp <= Cal_PER_BER_TABLE[i]) 
        { 
            *ber = Cal_PER_BER_TABLE[i+1]; 
            return err; 
        } 
    } 
    
    *ber = 153000;
    //*ber = 1.530e-003; 
    return err; 
}

UINT8 DTMBIP_GetBec(U32BITS *bec) 
{ 
    UINT8 err = DTMB_NO_ERROR; 
    U32BITS per, per_temp; 
    //UINT8 TableSize = sizeof(PER_BER_TABLE) / sizeof(U32BITS); 
    UINT8 TableSize = sizeof(Cal_PER_BEC_TABLE) / sizeof(U32BITS);
    UINT8 i; 

    err = DTMBIP_GetPer(&per); 
    per_temp = (per *10000)/32768;

    for (i = 0; i < TableSize; i += 2) 
    { 
        if (per_temp <= Cal_PER_BEC_TABLE[i]) 
        { 
        *bec = Cal_PER_BEC_TABLE[i+1]; 
        return err; 
        } 
    } 
    
    *bec = 75403;
    return err; 
}

UINT8 DTMBIP_SetAutoMode()
{
	UINT8 err = DTMB_NO_ERROR;

	REALTEK_H_DTMB_BASE_DBG("Set AutoMode ");

	/*����Ϊ�Զ�ģʽ*/
	DTMBIP_WriteRegister(0x0002, 0x00);
	return err;
}


BOOL DTMBIP_IsDemodLocked(BOOL PrintInfo, UINT8 *presult, UINT8 *locked, UINT8 *Status)
{
#if DTMBIP_DEBUG_INFO_OPEN
	S32BITS SNR, BER, LBER;
	UINT8 Carrier, Mode, Rate, InterleaverLength, PnMode, PnVariable;
	UINT32 FieldStrength, NCOValue;
#endif
	 DTMBIP_ReadRegister(0x0011, locked);
	 DTMBIP_ReadRegister(0x0010, Status);
	

#if DTMBIP_DEBUG_INFO_OPEN
	DTMBIP_GetSignalSNR(&SNR);
	DTMBIP_GetSignalBER(&BER);
	DTMBIP_GetParameters(&Carrier, &Mode, &Rate, &InterleaverLength, &PnMode, &PnVariable);
	DTMBIP_GetFieldStrength(&FieldStrength);
	DTMBIP_GetLdpcBER(&LBER);
	DTMBIP_GetNCOValue(&NCOValue);
	//REALTEK_H_DTMB_BASE_DBG("SNR: %f,BER: %f,LBER: %f,FS: %d,NCO: 0x%06x", SNR, BER, LBER, FieldStrength, NCOValue);

	REALTEK_H_DTMB_BASE_DBG("Mode:");
	switch (Mode) {
	case DTMB_DTMB_4QAM_NR:
		REALTEK_H_DTMB_BASE_DBG("4QAM_NR, ");
		break;
	case DTMB_DTMB_4QAM:
		REALTEK_H_DTMB_BASE_DBG("4QAM, ");
		break;
	case DTMB_DTMB_16QAM:
		REALTEK_H_DTMB_BASE_DBG("16QAM, ");
		break;
	case DTMB_DTMB_32QAM:
		REALTEK_H_DTMB_BASE_DBG("32QAM, ");
		break;
	case DTMB_DTMB_64QAM:
		REALTEK_H_DTMB_BASE_DBG("64QAM, ");
		break;
	default:
		break;
	}

	REALTEK_H_DTMB_BASE_DBG("PNMode:");
	switch (PnMode) {
	case DTMB_PN_945:
		REALTEK_H_DTMB_BASE_DBG("945, ");
		break;
	case DTMB_PN_595:
		REALTEK_H_DTMB_BASE_DBG("595, ");
		break;
	case DTMB_PN_420:
		REALTEK_H_DTMB_BASE_DBG("420, ");
		break;
	default:
		break;
	}

	REALTEK_H_DTMB_BASE_DBG("Rate:");
	switch (Rate) {
	case DTMB_RATE_04:
		REALTEK_H_DTMB_BASE_DBG("0.4, ");
		break;
	case DTMB_RATE_06:
		REALTEK_H_DTMB_BASE_DBG("0.6, ");
		break;
	case DTMB_RATE_08:
		REALTEK_H_DTMB_BASE_DBG("0.8, ");
		break;
	default:
		break;
	}

	REALTEK_H_DTMB_BASE_DBG("Carrier:");
	switch (Carrier) {
	case DTMB_CARRIER_SINGLE:
		REALTEK_H_DTMB_BASE_DBG("Single, ");
		break;
	case DTMB_CARRIER_MULTI:
		REALTEK_H_DTMB_BASE_DBG("Multi, ");
		break;
	default:
		break;
	}

	REALTEK_H_DTMB_BASE_DBG("Interleave:");
	switch (InterleaverLength) {
	case DTMB_INTERLEAVER_720:
		REALTEK_H_DTMB_BASE_DBG("720, ");
		break;
	case DTMB_INTERLEAVER_240:
		REALTEK_H_DTMB_BASE_DBG("240, ");
		break;
	default:
		break;
	}

	REALTEK_H_DTMB_BASE_DBG("PNVariable:");
	switch (PnVariable) {
	case DTMB_PN_VARIABLE:
		REALTEK_H_DTMB_BASE_DBG("Variable ");
		break;
	case DTMB_PN_FIXED:
		REALTEK_H_DTMB_BASE_DBG("Fixed ");
		break;
	default:
		break;
	}

	REALTEK_H_DTMB_BASE_DBG("\n");
#endif

	if (PrintInfo)
		REALTEK_H_DTMB_BASE_DBG("0x%02x 0x%02x!", *Status, *locked);

	if ((*locked) & 0x01) {
		*presult = 1;
		return TRUE;
	} else {
		*presult = 0;
		return FALSE;
	}
}



UINT8 DTMBIP_SetManualMode(UINT8 CarrierMode, UINT8 mode, UINT8 rate, UINT8 interleaverLength, UINT8 pnmode, UINT8 PnVariable)
{
	UINT8 err = DTMB_NO_ERROR;
	UINT8 ReadData = 0, WriteData = 0;

	REALTEK_H_DTMB_BASE_DBG("Set Manual Mode !");

	DTMBIP_WriteRegister(0x0002, 0xFF);

	DTMBIP_ReadRegister(0x0003, &ReadData);
	WriteData = ReadData & 0xE8;
	WriteData |= pnmode;
	WriteData |= (PnVariable << 2);
	WriteData |= (CarrierMode << 4);
	DTMBIP_WriteRegister(0x0003, WriteData);
	REALTEK_H_DTMB_BASE_DBG("0xB817000C  0x%02x !", WriteData);

	DTMBIP_ReadRegister(0x0004, &ReadData);
	WriteData = ReadData & 0x88;
	WriteData |= mode;
	WriteData |= (rate << 4);
	WriteData |= (interleaverLength << 6);
  DTMBIP_WriteRegister(0x0004, WriteData);
	REALTEK_H_DTMB_BASE_DBG("0xB8170010  0x%02x !", WriteData);

	return err;
}


UINT8 DTMBIP_GetNCOValue(UINT32 *NCOValue)
{
	UINT8 err = DTMB_NO_ERROR;
	UINT8 ReadData = 0;

	DTMBIP_ReadRegister(0x0059, &ReadData);
	*NCOValue = ReadData;

	DTMBIP_ReadRegister(0x005A, &ReadData);
	*NCOValue <<= 8;
	*NCOValue |= ReadData;

	DTMBIP_ReadRegister(0x005B, &ReadData);
	*NCOValue <<= 8;
	*NCOValue |= ReadData;

	REALTEK_H_DTMB_BASE_DBG("DemodIP_NCOValue value = %x\n", *NCOValue);
	REALTEK_H_DTMB_BASE_DBG("DemodIP_NCOValue value = %x\n", *NCOValue);
	return err;
}


UINT8 DTMBIP_SetNCOValue(UINT32 NCOValue, UINT32 CO1, UINT32 CO2)
{
	UINT8 err = DTMB_NO_ERROR;
	UINT8 temp1 = 0, temp2 = 0, temp3 = 0;

	temp1 = (UINT8)((NCOValue & 0x00ff0000) >> 16);
	temp2 = (UINT8)((NCOValue & 0x0000ff00) >> 8);
	temp3 = (UINT8)((NCOValue & 0x000000ff));

	REALTEK_H_DTMB_BASE_DBG("Updage NCO 0x%02x,0x%02x,0x%02x", temp1, temp2, temp3);

	/*Set DTMB related registers*/
	/*��l�W������r�p��*/
	DTMBIP_WriteRegister(0x0028, temp1);
	DTMBIP_WriteRegister(0x0029, temp2);
	DTMBIP_WriteRegister(0x002A, temp3);
	
	temp1 = (UINT8)((CO1 & 0x0000ff00) >> 8);
	temp2 = (UINT8)((CO1 & 0x000000ff));
	REALTEK_H_DTMB_BASE_DBG("Updage CO1 0x%02x,0x%02x", temp1, temp2);

	/*���W����r�p��*/
	DTMBIP_WriteRegister(0x0188, temp1);
	DTMBIP_WriteRegister(0x0189, temp2);
	
	temp1 = (UINT8)((CO2 & 0x0000ff00) >> 8);
	temp2 = (UINT8)((CO2 & 0x000000ff));
	REALTEK_H_DTMB_BASE_DBG("Updage CO2 0x%02x,0x%02x", temp1, temp2);

	/*�����W�I���v�p��*/
	DTMBIP_WriteRegister(0x0186, temp1);
	DTMBIP_WriteRegister(0x0187, temp2);
	
	return err;
}

UINT8 DTMBIP_SetTimeFreq()
{
	UINT8 err = DTMB_NO_ERROR;
	S32BITS FrequencySample = 30.375*1000;
	U32BITS TF = 0;
	UINT8 temp1 = 0, temp2 = 0, temp3 = 0, temp4 = 0;

	//TF = (UINT32)((FrequencySample-30.24)/30.24*0x40000000);
	//REALTEK_H_DTMB_BASE_DBG("Demod FrequencySample = "PT_U32BITS" \n", FrequencySample);
	TF = (U32BITS)(((FrequencySample-(30.24*1000))*32768)/(30.24*1000))*32768;
    REALTEK_H_DTMB_BASE_DBG("Demod Timing Offset TF = "PT_U32BITS" \n", TF);
        
	temp1 = (UINT8)((TF & 0xff000000) >> 24);
	temp2 = (UINT8)((TF & 0x00ff0000) >> 16);
	temp3 = (UINT8)((TF & 0x0000ff00) >> 8);
	temp4 = (UINT8)((TF & 0x000000ff));
	REALTEK_H_DTMB_BASE_DBG("Updage Time Frequency 0x"PT_H32BITS",0x"PT_H32BITS",0x"PT_H32BITS",0x"PT_H32BITS"\n", (U32BITS)temp1, (U32BITS)temp2, (U32BITS)temp3, (U32BITS)temp4);

	DTMBIP_WriteRegister(0x0038, temp1);
	DTMBIP_WriteRegister(0x0039, temp2);
	DTMBIP_WriteRegister(0x003A, temp3);
	DTMBIP_WriteRegister(0x003B, temp4);

	// b.�X����p��
    temp1 = (UINT8)((FrequencySample*1000)*25/(30.24*1000));
	temp1 &= 0x3f;
	temp1 |= 0x80;
	DTMBIP_WriteRegister(0x0197, temp1);   
	DTMBIP_ReadRegister(0x0022, &temp1);
	temp1 |= 0x0A;  // bit3 = 1  and bit1 = 1 adc_data format = unsigned
	DTMBIP_WriteRegister(0x0022, temp1);

	return err;
}

UINT8 DTMBIP_SetAXIBus(UINT32 AXIBus)
{
	UINT8 err = DTMB_NO_ERROR;
	UINT8 temp1 = 0, temp2 = 0, temp3 = 0, temp4 = 0;

	temp1 = (UINT8)((AXIBus & 0xff000000) >> 24);
	temp2 = (UINT8)((AXIBus & 0x00ff0000) >> 16);
	temp3 = (UINT8)((AXIBus & 0x0000ff00) >> 8);
	temp4 = (UINT8)((AXIBus & 0x000000ff));

	REALTEK_H_DTMB_BASE_DBG("Updage AXI Offset 0x%02x,0x%02x,0x%02x,0x%02x", temp1, temp2, temp3, temp4);

	DTMBIP_WriteRegister(0x0809, temp1);
	DTMBIP_WriteRegister(0x0808, temp2);
	DTMBIP_WriteRegister(0x0807, temp3);
	DTMBIP_WriteRegister(0x0806, temp4);
	
	return err;
}

UINT8 DTMBIP_SetSpectrum(UINT8 SpectrumMode)
{
	UINT8 err = DTMB_NO_ERROR;
	UINT8 ReadData = 0, WriteData = 0;

	DTMBIP_ReadRegister(0x0021, &ReadData);
	if (SpectrumMode == DTMB_SPECTRUM_NEGATIVE) {
		WriteData = ReadData & 0xDF;
	} else if (DTMB_SPECTRUM_POSITIVE) {
		WriteData = ReadData | 0x20;
	}

	DTMBIP_WriteRegister(0x0021, WriteData);

	return err;
}

#if 0
static S32BITS DTMBIP_ABS(long x)
{
	return x < 0 ? -x : x;
}
#endif


UINT8 DTMBIP_CalculateNCOValue(S32BITS TunerIF, UINT32 *NCOValue, UINT32 *CO1, UINT32 *CO2)
{
	UINT8 err = DTMB_NO_ERROR;
	S32BITS FrequencySample = (S32BITS)FREQUENCY_SAMPLE;
	S32BITS FrequencySymble = (S32BITS)FREQUENCY_SYMBOL;
	U32BITS Temp;

    REALTEK_H_DTMB_BASE_DBG("Demod TunerIF = "PT_U32BITS"\n",TunerIF);

	if (TunerIF == 0) {
		*NCOValue = 0;
		/*
				return err;
			}

			if(CrystalSelect == HDIC2311_CRYSTAL_27M )
			{
				FrequencySample = 30.375;
			}
			else if(CrystalSelect == HDIC2311_CRYSTAL_304M )
			{
				FrequencySample = 30.4;
		*/
	} else {
		//Temp = DTMBIP_ABS(FrequencySample - TunerIF);
		Temp = ABS(FrequencySample,TunerIF);
		if (Temp < TunerIF) {
			//*NCOValue = (UINT32)(Temp / FrequencySample * 0x800000);                  //0x800000 = 32768 * 256
			*NCOValue = (UINT32)(((Temp) *1000 *32768)/ (FrequencySample *1000)* 256);
            REALTEK_H_DTMB_BASE_DBG("Demod NCO value = "PT_U32BITS"\n", (U32BITS)*NCOValue);
		} else {
			//*NCOValue = (UINT32)(TunerIF / FrequencySample * 0x800000);
		    *NCOValue = (UINT32)((TunerIF *1000 *32768) / (FrequencySample *1000) * 256);
            REALTEK_H_DTMB_BASE_DBG("Demod NCO value = "PT_U32BITS"\n", (U32BITS)*NCOValue);
		}
	
	}
	//*CO1 = (UINT32)((FrequencySymble * 100) / (5000 * FrequencySample) / 100 * 0x800000);
	//*CO2 = (UINT32)((FrequencySymble * 100) / (2048 * FrequencySample) / 100 * 0x800000);
    *CO1 = (UINT32)(((FrequencySymble * 100)*32768 / (5000 * FrequencySample)) *256 / 100);
    *CO2 = (UINT32)(((FrequencySymble * 100)*32768 / (2048 * FrequencySample)) *256 / 100);
    REALTEK_H_DTMB_BASE_DBG("Demod CO1 value = "PT_U32BITS"\n",  (U32BITS)*CO1);
    REALTEK_H_DTMB_BASE_DBG("Demod CO2 value = "PT_U32BITS"\n",  (U32BITS)*CO2);
    
	//*NCOValue = 0x1511e8;     //original value
	//*CO1 = 0x01a2;
	//*CO2 = 0x03fb;

	return err;
}


UINT8 DTMBIP_GetParameters(UINT8 *CarrierMode, UINT8 *pmode,
						   UINT8 *prate, UINT8 *pinterleaverLength, UINT8 *Pnmode, UINT8 *PnVariable)
{
	UINT8 err = DTMB_NO_ERROR;
	UINT8 ReadData1 = 0, ReadData2 = 0;

	//REALTEK_H_DTMB_BASE_DBG("DTMB_DemodIP_GetParameters\n");

	DTMBIP_ReadRegister(0x0005, &ReadData1);
	DTMBIP_ReadRegister(0x0006, &ReadData2);

	*Pnmode = ReadData1 & 0x03;
	//REALTEK_H_DTMB_BASE_INFO("DTMB_DemodIP_PNmode = %d, Pnmode_tmp = %d\n",*Pnmode);
	*PnVariable = ReadData1 & 0x04;
	*PnVariable >>= 2;
	//REALTEK_H_DTMB_BASE_INFO("DTMB_DemodIP_PNVariable = %d\n",*PnVariable );
	*CarrierMode = ReadData1 & 0x10;
	*CarrierMode >>= 4;
	//REALTEK_H_DTMB_BASE_INFO("DTMB_DemodIP_CarrierMode = %d\n",*CarrierMode );

	*pmode = ReadData2 & 0x07;
	//REALTEK_H_DTMB_BASE_INFO("DTMB_DemodIP_pmode = %d\n",*pmode );
	*prate = ReadData2 & 0x30;
	*prate >>= 4;
	//REALTEK_H_DTMB_BASE_INFO("DDTMB_DemodIP_prate = %d\n",*prate );
	*pinterleaverLength = ReadData2 & 0x40;
	*pinterleaverLength >>= 6;
	//REALTEK_H_DTMB_BASE_INFO("DTMB_DemodIP_pinterleaverLength = %d\n",*pinterleaverLength );

	return err;
}


UINT8 DTMBIP_GetSignalSNRMC(S32BITS *SignalSNR)
{
	UINT8 err = DTMB_NO_ERROR;
	UINT8 ReadData1 = 0, ReadData2 = 0;
	UINT32 value_reg;

	DTMBIP_ReadRegister(0x02C9, &ReadData1);
	DTMBIP_ReadRegister(0x02C8, &ReadData2);

	value_reg = ReadData1 * 256 + ReadData2;

	if ((value_reg != 0) && (value_reg != 0xFFFF)) {
		if (value_reg < 16) {
			value_reg = 16;
		}

#if SYSTEM_NOT_SUPPORT_LOG10
		*SignalSNR = Log10_Convert((UINT32)(value_reg / 16));
#else
		*SignalSNR = 10 * log10((long)(value_reg) / 16);
#endif
	}

	//REALTEK_H_DTMB_BASE_DBG("SNR_MC %d ",(UINT8)(*SignalSNR));

	return err;
}

UINT8 DTMBIP_GetSignalSNRSC(S32BITS *SignalSNR)
{
	UINT8 err = DTMB_NO_ERROR;
	UINT8 ReadData1 = 0, ReadData2 = 0;
	UINT32 value_reg;
	UINT8 Pnmode;
	UINT32 PnValue = 0;
	S32BITS value_temp = 0;

	DTMBIP_ReadRegister(0x0005, &ReadData1);
	Pnmode = ReadData1 & 0x03;

	if (Pnmode == DTMB_PN_945) {
		PnValue = 945;
	} else if (Pnmode == DTMB_PN_595) {
		PnValue = 595;
	} else if (Pnmode == DTMB_PN_420) {
		PnValue = 420;
	}

	DTMBIP_ReadRegister(0x02A5, &ReadData1);
	DTMBIP_ReadRegister(0x02A4, &ReadData2);

	value_reg = ReadData1 * 256 + ReadData2;
	if ((value_reg != 0) && (value_reg != 0xFFFF)) {
		value_temp = (81 * PnValue) / value_reg / 2;


		if (value_temp < 1) {
			value_temp = 1;
		}

#if SYSTEM_NOT_SUPPORT_LOG10
		*SignalSNR = Log10_Convert((UINT32)value_temp);
#else
		*SignalSNR = 10 * log10(value_temp);
#endif

	} else {
		*SignalSNR = 0;
	}

	//REALTEK_H_DTMB_BASE_INFO("DTMB_DemodIP_SNR_SC = %d\n ",*SignalSNR); //Get SNR_SC
	return (err);
}



/****************************
Get Signal SNR
****************************/
UINT8 DTMBIP_GetSignalSNR(S32BITS *SignalSNR)
{
	UINT8 err = DTMB_NO_ERROR;
	UINT8 ReadData1 = 0;
	UINT8 CarrierMode;

	/*Get Working Mode,DTMB or Cable*/
	DTMBIP_ReadRegister(0x0005, &ReadData1);
	CarrierMode = ReadData1 & 0x10;
	CarrierMode >>= 4;

	if (CarrierMode == DTMB_CARRIER_SINGLE) {
		DTMBIP_GetSignalSNRSC(SignalSNR);
	} else if (CarrierMode == DTMB_CARRIER_MULTI) {
		DTMBIP_GetSignalSNRMC(SignalSNR);
	}
	return err;
}


/****************************
In Dtmb Mode, Get LDPC BER
****************************/
UINT8 DTMBIP_GetLdpcBER(S32BITS *pLdpcBER)
{
	UINT8 err = DTMB_NO_ERROR;
	UINT8 ReadData1 = 0, ReadData2 = 0;

	DTMBIP_ReadRegister(0x00AA, &ReadData1);
	DTMBIP_ReadRegister(0x00A9, &ReadData2);

	//*pLdpcBER = (double)((ReadData2 & 0x0f) * 256 + ReadData1) / 4096;
	*pLdpcBER = ((ReadData2 & 0x0f) * 256 + ReadData1);
	//REALTEK_H_DTMB_BASE_INFO("DTMB_DemodIP_GetLBER = %d\n",*pLdpcBER);  // printf DTMBIP_LBER

	return err;
}

/****************************
In Dtmb Mode, Get Signal BER
****************************/
UINT8 DTMBIP_GetSignalBER(S32BITS *pSignalBER)
{
	UINT8 err = DTMB_NO_ERROR;
	UINT8 ReadData1 = 0, ReadData2 = 0;

	DTMBIP_ReadRegister(0x00A7, &ReadData1);
	DTMBIP_ReadRegister(0x00A8, &ReadData2);

	//*pSignalBER = (S32BITS)(ReadData1 * 256 + ReadData2) / 6666664;
	*pSignalBER = (ReadData1 * 256 + ReadData2);

	return (err);
}
/****************************
****************************/
UINT8 DTMBIP_GetFieldStrength(UINT32 *FieldStrength)
{
	UINT8 err = DTMB_NO_ERROR;
	UINT8 ReadData1 = 0, ReadData2 = 0;

	DTMBIP_ReadRegister(0x0061, &ReadData1);
	DTMBIP_ReadRegister(0x0062, &ReadData2);

	*FieldStrength = (UINT16)(ReadData1 * 256 + ReadData2);

	//REALTEK_H_DTMB_BASE_DBG("DTMB_DemodIP_FieldStrength =%d\n",*FieldStrength);

	return (err);
}
/************************************
*************************************/
UINT8 DTMBIP_SoftReset()
{
	UINT8 err = DTMB_NO_ERROR;

	/*��ȡ����ģʽ,DTMB*/
	
		DTMBIP_WriteRegister(0x0009, 0x10);
		//DTMBIP_Wait(10);
		tv_osal_msleep(10);
		DTMBIP_WriteRegister(0x0009, 0x00);
	
	return (err);
}

/************************************
*************************************/
UINT8 DTMBIP_SetTunerType(UINT8 Type)
{
	UINT8 err = DTMB_NO_ERROR;
	UINT8 ReadData = 0, WriteData = 0;

	DTMBIP_ReadRegister(0x0022, &ReadData);

	if (Type == DTMB_TUNER_ZERO_IF) {
		WriteData = ReadData | 0x01;
	} else {
		WriteData = ReadData & 0xFE;
	}
	DTMBIP_WriteRegister(0x0022, WriteData);

	return (err);
}
/************************************
*************************************/
UINT8 DTMBIP_SetTsFormat(UINT8 Serial, UINT8 CLK_Polarity, UINT8 CLK_Valid, UINT8 Null_Packet, UINT8 Serial_Output, UINT8 Serial_Order, UINT8 Serial_Sync)
{
	UINT8 err = DTMB_NO_ERROR;
	UINT8 ReadData = 0, WriteData = 0;

	DTMBIP_ReadRegister(0x00A3, &ReadData);

	WriteData = ReadData & 0x60;

	if (Serial == DTMB_TS_OUTPUT_PARALLEL) {
		WriteData |= 0x08;
	}

	if (CLK_Polarity == DTMB_TS_DATA_SAMPLE_RISING) {
		WriteData |= 0x04;
	}

	if (CLK_Valid == DTMB_TS_CLK_WITH_TSVLD) {
		WriteData |= 0x02;
	}

	if (Null_Packet == DTMB_TS_NULLPACKET_DELETED) {
		WriteData |= 0x01;
	}

	if (Serial_Output == DTMB_TS_SERIAL_OUTPUT_D0) {
		WriteData |= 0x10;
	}

	if (Serial_Order == DTMB_TS_SERIAL_LSB_TO_MSB) {
		WriteData |= 0x80;
	}

	DTMBIP_WriteRegister(0x00A3, WriteData);

	REALTEK_H_DTMB_BASE_DBG("DTMB_DemodIP_TS Format 0x%02x\n", WriteData);

	return (err);
}

/****************************
Open I2C Transmitted Function
****************************/
/*
UINT8 HDIC2311_OpenTunerI2C(UINT8 Address)
{
	UINT8 err = HDIC_NO_ERROR;
	UINT8 ReadData = 0;

	HDICTBX_Print(("Set 2311 Tuner I2C Address 0x%02x!!\n",Address));
	Address &= 0xFE;
	HDIC2311_WriteRegister(0x000F,Address);
	HDIC2311_WriteRegister(0x000E,0x01);
	return (err);

}
*/
/****************************
Close I2C Transmitted Function
****************************/
/*
UINT8 HDIC2311_CloseTunerI2C(void)
{
	UINT8 err = HDIC_NO_ERROR;

	HDICTBX_Print(("2311 Close Tuner I2C!"));

	err = HDIC2311_WriteRegister(0x000E,0x00);

	return (err);
}
*/

/********************************
Get Signal Stength
If tuner AGC is auto control, please using tuner signal strength function
********************************/
UINT8 DTMBIP_GetSignalStrength(UINT8 *SignalStrength,int* RF_Level)
{
	UINT8 err = DTMB_NO_ERROR;

      #ifndef  DTMB_S_Calculate
	UINT32 FieldStrength;
	DTMBIP_GetFieldStrength(&FieldStrength);
	//REALTEK_H_DTMB_BASE_INFO("FieldStrength = %u\n",FieldStrength);

	/* It can modify this function to get expect reasonable signal strength, the range is 0~100
	*/
	if (FieldStrength >= 0x800)
		FieldStrength = 0x800;
	if (FieldStrength <= 0x418)
		FieldStrength = 0x418;
	*SignalStrength = (UINT8)((0x800 - FieldStrength) / 12);
	#else
	        int Texx_ref, P_rel,ssi;
		Texx_ref = *RF_Level;
		 REALTEK_H_DTMB_BASE_INFO("RFl = %d\n",Texx_ref);
		//*************************************************//
	        DTMBIP_GetPrel(&Texx_ref, &P_rel);
		if(P_rel<-15)ssi = 0;
		else if(P_rel<0 && P_rel>=-15)
		{
	               ssi = (P_rel+15)*20/30;
		}
		else if(P_rel<20 && P_rel>=0)
		{
	               ssi = 4*P_rel +10;
		}
		else if(P_rel<35 && P_rel>=20)
		{
	              ssi = (P_rel-20)*20/30+90;
		}
		else ssi = 100;

		*SignalStrength = (UINT8)ssi;

	         REALTEK_H_DTMB_BASE_INFO("Prel = %d ,SSI = %u\n",P_rel,*SignalStrength);
		//*************************************************//
	REALTEK_H_DTMB_BASE_DBG("DTMB_DemodIP_SignalStrength %d !!!", *SignalStrength);
        #endif

	return err;
}
//******Get CN_Ref*******************//
UINT8 DTMBIP_GetCNrel(S32BITS *CNrel)
{

       UINT8 err = DTMB_NO_ERROR;
	UINT8 ReadData1 = 0;
	UINT8 CarrierMode,Constellation,LDPC_rate;
	U32BITS cn_dtmb=0;
	S32BITS CN_input,temp;

	DTMBIP_GetSignalSNR(&CN_input);
	CN_input = CN_input*10;



	//Get Working Mode,DTMB or Cable
	DTMBIP_ReadRegister(0x0005, &ReadData1);
	CarrierMode = ReadData1 & 0x10;
	CarrierMode >>= 4;

	ReadData1 = 0;
	DTMBIP_ReadRegister(0x0006, &ReadData1);
	Constellation = ReadData1 & 0x07;

	//0->4QAM-NR
       //  1->4QAM
        //->16QAM
        //->32QAM
        //->64QAM



	ReadData1 = 0;
	DTMBIP_ReadRegister(0x0006, &ReadData1);
	LDPC_rate = ReadData1 & 0x30; //B[2:0]: constellation
	LDPC_rate>>= 4;// 0->0.4 , 1->0.6 , 2->0.8

	if(CarrierMode==DTMB_CARRIER_SINGLE)
	{
             cn_dtmb = single_cn_dtmb[Constellation][LDPC_rate];
		//*p_ref = single_pref_dtmb[Constellation][LDPC_rate];
	}
	else if(CarrierMode==DTMB_CARRIER_MULTI)
	{
             cn_dtmb = mutile_cn_dtmb[Constellation][LDPC_rate];
		//*p_ref = mutile_pref_dtmb[Constellation][LDPC_rate];
	}
	//REALTEK_H_DTMB_BASE_INFO("CN_DTMB = %u, CN_input = %d \n",cn_dtmb , CN_input);
	temp = cn_dtmb;

	*CNrel = CN_input - temp;
	//REALTEK_H_DTMB_BASE_INFO("CN_rel = %d\n",*CNrel);
	return err;

}
//********************************************//

UINT8 DTMBIP_GetPrel(int* p_input, int *p_rel)
{
   UINT8 err = DTMB_NO_ERROR;
   int p_ref=0;
   UINT8 ReadData1 = 0;
   UINT8 CarrierMode,Constellation,LDPC_rate;

       DTMBIP_ReadRegister(0x0005, &ReadData1);
       CarrierMode = ReadData1 & 0x10;
	CarrierMode >>= 4;

	ReadData1 = 0;
	DTMBIP_ReadRegister(0x0006, &ReadData1);
	Constellation = ReadData1 & 0x07;

	//0->4QAM-NR
       //  1->4QAM
        //->16QAM
        //->32QAM
        //->64QAM



	ReadData1 = 0;
	DTMBIP_ReadRegister(0x0006, &ReadData1);
	LDPC_rate = ReadData1 & 0x30; //B[2:0]: constellation
	LDPC_rate>>= 4;// 0->0.4 , 1->0.6 , 2->0.8

	if(CarrierMode==DTMB_CARRIER_SINGLE)
	{
	    p_ref = single_pref_dtmb[Constellation][LDPC_rate];
	}
	else if(CarrierMode==DTMB_CARRIER_MULTI)
	{
	    p_ref = mutile_pref_dtmb[Constellation][LDPC_rate];
	}
	REALTEK_H_DTMB_BASE_INFO("p_ref = %d  ,p_input = %d \n",p_ref,*p_input);
	*p_rel = ( (*p_input*10) - p_ref)/10;

	return err;

}
//********Get Ber SQI*****************//
UINT8 DTMBIP_GetBerSQI(S32BITS *Ber_SQI)
{
        UINT8 err = DTMB_NO_ERROR;
	U32BITS ber_nu,temp;
	int ber_temp,uns_to_sig;
	DTMBIP_GetBer(&ber_nu);
	ber_temp = ber_nu;
	REALTEK_H_DTMB_BASE_INFO("ber = %d\n",ber_temp);

	if(ber_temp>=107000)
	{
             *Ber_SQI = 0;
	}
	else if(ber_temp==0)
	{
              *Ber_SQI = 100;
	}
	else
	{
	   REALTEK_H_DTMB_BASE_INFO("ber_temp1 = %d\n",ber_temp);
           ber_temp = 100000000/ber_temp;// -->1/BER
	   REALTEK_H_DTMB_BASE_INFO("ber_temp = %d\n",ber_temp);
           temp = ber_temp;
	   temp = Log10_Convert(temp); //log(1/ber)
	   REALTEK_H_DTMB_BASE_INFO("temp_log10 = %lu\n",temp);
	   uns_to_sig = temp;
	   uns_to_sig = ((uns_to_sig*100)/100000);
	   REALTEK_H_DTMB_BASE_INFO("uns_to_sig = %d\n",uns_to_sig);
	   *Ber_SQI = (uns_to_sig*20)/100 -40;
	}
    return err;
}
//*****************************//
/********************************
Get Signal Quality
********************************/
UINT8 DTMBIP_GetSignalQuality(UINT8 *SignalQuality)
{
	UINT8 err = DTMB_NO_ERROR;

 #ifndef  DTMB_S_Calculate
        S32BITS SQI, SignalSNR;
	DTMBIP_GetSignalSNR(&SignalSNR);

	*SignalQuality = (UINT8)SignalSNR;
	(*SignalQuality) *= 3;

	SQI = (SignalSNR * 3) / 10000;

	*SignalQuality = (UINT8) SQI ;

	if (*SignalQuality >= 98) {
		*SignalQuality = 98;
	}

	//REALTEK_H_DTMB_BASE_DBG("DTMB_DemodIP_SignalQuality %d !!!", *SignalQuality);
 #else
	//////////////////////////////////////////////////////////
	S32BITS SQI,CNrel,Ber_SQI;
       DTMBIP_GetCNrel(&CNrel);
       DTMBIP_GetBerSQI(&Ber_SQI);
	CNrel = (CNrel*100)/100000;
	REALTEK_H_DTMB_BASE_INFO("CNrel= %d\n",CNrel);
	if(CNrel>300)
	{
            SQI = Ber_SQI;
	}
	else if(CNrel<-700)
	{
	   SQI = 0;
	}
	else
	{
           SQI = (((CNrel-300) + 1000)*Ber_SQI)/1000;
	    REALTEK_H_DTMB_BASE_INFO("SQI= %d , Ber_SQI = %d \n",SQI,Ber_SQI);
	}
       *SignalQuality = (UINT8) SQI ;
#endif
	///////////////////////////////////////////////////////////
	return err;
}
//#if DTMBIP_BANDWIDTH_6M
/************************************
** ����ϵͳ������
**DTMBIP_BANDWIDTH_6M
*************************************/
UINT8 DTMBIP_SetSytemBandwidth6M()
{
	UINT8 err = DTMB_NO_ERROR;
	UINT8 ReadData = 0;


	DTMBIP_ReadRegister(0x0022, &ReadData);
	ReadData |= 0x08;                           //bit3 : timing_cw_manual_en set to 1 then could modify
	DTMBIP_WriteRegister(0x0022, ReadData);

	err = DTMBIP_WriteRegister(0x0038, 0x15);   //update in 2018/04/02
	err = DTMBIP_WriteRegister(0x0039, 0xB6);
	err = DTMBIP_WriteRegister(0x003A, 0xDB);
	err = DTMBIP_WriteRegister(0x003B, 0x6D);
	err = DTMBIP_WriteRegister(0x0180, 0x0E);
	err = DTMBIP_WriteRegister(0x0181, 0xCB);			
	err = DTMBIP_WriteRegister(0x0182, 0x02);
	err = DTMBIP_WriteRegister(0x0183, 0xE4);			
	err = DTMBIP_WriteRegister(0x0184, 0x06);
	err = DTMBIP_WriteRegister(0x0185, 0x75);			
	err = DTMBIP_WriteRegister(0x0186, 0x03);
	err = DTMBIP_WriteRegister(0x0187, 0x5C);			
	err = DTMBIP_WriteRegister(0x0188, 0x01);
	err = DTMBIP_WriteRegister(0x0189, 0x39);			
	err = DTMBIP_WriteRegister(0x0197, 0x2B);						

	return err;
}
//#endif 

/************************************
** ����ϵͳ������
**DTMBIP_BANDWIDTH_8M
*************************************/
UINT8 DTMBIP_SetSytemBandwidth8M()
{
	UINT8 err = DTMB_NO_ERROR;
	UINT8 ReadData = 0;


	DTMBIP_ReadRegister(0x0022, &ReadData);
	ReadData |= 0x08;                           //bit3 : timing_cw_manual_en set to 1 then could modify
	DTMBIP_WriteRegister(0x0022, ReadData);

	err = DTMBIP_WriteRegister(0x0038, 0x00);   //update in 2018/04/02
	err = DTMBIP_WriteRegister(0x0039, 0x49);
	err = DTMBIP_WriteRegister(0x003A, 0x24);
	err = DTMBIP_WriteRegister(0x003B, 0x92);
	err = DTMBIP_WriteRegister(0x0180, 0x0f);
	err = DTMBIP_WriteRegister(0x0181, 0xfa);			
	err = DTMBIP_WriteRegister(0x0182, 0x02);
	err = DTMBIP_WriteRegister(0x0183, 0x2b);			
	err = DTMBIP_WriteRegister(0x0184, 0x04);
	err = DTMBIP_WriteRegister(0x0185, 0xd8);			
	err = DTMBIP_WriteRegister(0x0186, 0x00);
	err = DTMBIP_WriteRegister(0x0187, 0x80);			
	err = DTMBIP_WriteRegister(0x0188, 0x01);
	err = DTMBIP_WriteRegister(0x0189, 0xa2);			
	err = DTMBIP_WriteRegister(0x0197, 0x99);						

	return err;
}

/********************************
IC Initialization, it is called during system initialization
********************************/
UINT8 DTMBIP_Init()
{
	UINT8 err = DTMB_NO_ERROR;
	UINT32 NCOValue = 0;
	UINT32 CO1 = 0;
	UINT32 CO2 = 0;
	UINT32 AXIBus = 0x00000000;
	DTMBIP_Init_Info InitInfo;

	/*Initialize HD2311 Allocation*/
	//InitInfo.WorkMode         = HDIC2311_DTMB_SELECT;			/*Working Mode: Terrestrial and Cable*/
	InitInfo.TunerType          = DTMB_TUNER_NORMAL;			/*LNB Type, Zero-IF or Other*/
	/*the IF of the LNB, Units: MHz, Zero-IF LNB is assigned to 0 */
	InitInfo.TunerIF            = 5;							/*the IF of the LNB, Units: MHz, Zero-IF LNB is assigned to 0 */
	//InitInfo.CrystalSelect    = HDIC2311_CRYSTAL_24M;			/*Crystal Selection, Units: MHZ, 16Mhz/24Mhz/27Mhz/30.4Mhz */
	InitInfo.TsType             = DTMB_TS_OUTPUT_PARALLEL;		/*TS Output Mode, serial or parallel mode*/
	InitInfo.TsClockPolarity    = DTMB_TS_DATA_SAMPLE_RISING;	/*Sample at rising or falling edges*/
	InitInfo.TsClockValid       = DTMB_TS_CLK_WITH_TSVLD;		/* Valid Clock Method, always valid or depends on valid signals*/
	InitInfo.TsNullPacket       = DTMB_TS_NULLPACKET_DELETED;	/*Null Packets detected enable */
	InitInfo.TsSerialOutPut     = DTMB_TS_SERIAL_OUTPUT_D7;		/*In Serial TS Mode, output pin is TS_D7 or TS_D0 */
	InitInfo.TsSerialOrder      = DTMB_TS_SERIAL_MSB_TO_LSB;
	InitInfo.TsSerialSyncMode   = DTMB_TS_SERIAL_SYNC_BIT;

	/* HWReset DTMBIP*/
	//DTMBIP_HWReset();

	//DTMBIP_TunerInit();
	/* According Tuner IF and Crystals to count NCO */
	DTMBIP_CalculateNCOValue(InitInfo.TunerIF, &NCOValue, &CO1, &CO2);
	/* Set NCO */
	DTMBIP_SetNCOValue(NCOValue, CO1, CO2);

	DTMBIP_SetTimeFreq();

	/* Set Tuner Type, Zero-IF or Other*/
	DTMBIP_SetTunerType(InitInfo.TunerType);

	DTMBIP_SetSpectrum(DTMB_SPECTRUM_POSITIVE);

	/* Set TS Ouput Format*/
	DTMBIP_SetTsFormat(InitInfo.TsType, InitInfo.TsClockPolarity, InitInfo.TsClockValid, InitInfo.TsNullPacket, InitInfo.TsSerialOutPut, InitInfo.TsSerialOrder, InitInfo.TsSerialSyncMode);
	//DTMBIP_SetIfInverse();
	/* Set Working Mode */

	DTMBIP_SetAXIBus(AXIBus); //set AXI

	DTMBIP_SetAutoMode();

	DTMBIP_SetWorkMode();

	DTMBIP_SoftReset();
	//tv_osal_msleep(100);

  /*  #if DTMBIP_BANDWIDTH_6M
	     //Set Bandwidth6M
	    DTMBIP_SetSytemBandwidth6M();
	#endif */
	REALTEK_H_DTMB_BASE_DBG("RealtekH_DTMB init done!!");
	return (err);
}

/********************************
the function that set frequency in cable signal
********************************/
BOOL DTMBIP_DTMBSetFrequency(UINT32 Frequency)
{
	UINT8 CheckLockTemp, LockResult, locked, status;
	BOOL SignalDetect = FALSE;
	//UINT8 ReadData = 0;
#if DTMBIP_SPECTRUM_ADJUST_OPEN
	UINT8 ReadData0 = 0, ReadData1 = 0, ReadData2 = 0;
#endif

	/********************
	** It needs to set Tuner Frequency first
	********************/
	if (DTMBIP_TunerLock(Frequency) != TRUE) {
		return FALSE;
	}

	/* Setting the work mode is auto*/
	DTMBIP_SetAutoMode();

#if DTMBIP_SPECTRUM_ADJUST_OPEN
	DTMBIP_SetSpectrum(DTMB_SPECTRUM_NEGATIVE);
	DTMBIP_SoftReset();
	for (CheckLockTemp = 0; CheckLockTemp < 20; CheckLockTemp++) {
		tv_osal_msleep(100); /* wait 100 ms */
		DTMBIP_IsDemodLocked(TRUE, &LockResult, &locked, &status);
		if ((status & 0xf0) > 0x20) {
			SignalDetect = TRUE;
		}
		if ((SignalDetect == FALSE) && (CheckLockTemp == 15)) {
			REALTEK_H_DTMB_BASE_WARNING("---> [DTMBIP] DTMBIP DTMB No Signal \n");
			return FALSE;
		}
		if (LockResult == 1) {
			DTMBIP_ReadRegister(0x0006, &ReadData0);
			tv_osal_msleep(100); /* wait 100 ms */
			DTMBIP_ReadRegister(0x0006, &ReadData1);
			tv_osal_msleep(100); /* wait 100 ms */
			DTMBIP_ReadRegister(0x0006, &ReadData2);
			CheckLockTemp++;
			CheckLockTemp++;
			if ((ReadData0 == ReadData1) && (ReadData1 == ReadData2)) {
				
				REALTEK_H_DTMB_BASE_DBG("---> [DTMBIP] DTMBIP DTMB is locked \n");
				return TRUE;
				
			} else {
				
				break;
			}
		}
	}
	/*Do Spectrum Adjust Try*/
	DTMBIP_SetSpectrum(DTMB_SPECTRUM_POSITIVE);
	DTMBIP_SoftReset();
	SignalDetect = FALSE;

	for (CheckLockTemp = 0; CheckLockTemp < 20; CheckLockTemp++) {
		tv_osal_msleep(100); /* wait 100 ms */
		DTMBIP_IsDemodLocked(TRUE, &LockResult, &locked, &status);
		if ((status & 0xf0) > 0x20) {
			SignalDetect = TRUE;
		}
		if ((SignalDetect == FALSE) && (CheckLockTemp == 15)) {
			REALTEK_H_DTMB_BASE_WARNING("---> [DTMBIP] DTMBIP DTMB No Signal \n");
			return FALSE;
		}
		if (LockResult == 1)
		{
			DTMBIP_ReadRegister(0x0006, &ReadData0);
			tv_osal_msleep(100); /* wait 100 ms */
			DTMBIP_ReadRegister(0x0006, &ReadData1);
			tv_osal_msleep(100); /* wait 100 ms */
			DTMBIP_ReadRegister(0x0006, &ReadData2);
			CheckLockTemp++;
			CheckLockTemp++;
			if ((ReadData0 == ReadData1) && (ReadData1 == ReadData2)) {
			/*
				ReadData0 &= 0x07;
				if (ReadData0 == 0x02) {
					//rtd_outl(0xB8142CFC, 0x00000000);
					//rtd_outl(0xB8142770, 0x00000000);
					DTMBIP_WriteRegister(0x0b3f, 0x00);
					DTMBIP_WriteRegister(0x09d0, 0x00);
				} else {
					//rtd_outl(0xB8142CFC, 0x00000001);
					//rtd_outl(0xB8142770, 0x00000001);
					DTMBIP_WriteRegister(0x0b3f, 0x01);
					DTMBIP_WriteRegister(0x09d0, 0x01);
				}
			*/
				REALTEK_H_DTMB_BASE_DBG("---> [DTMBIP] DTMBIP DTMB is locked \n");
				return TRUE;
			} 
			else
			{	
				break;
			}
		}
	}
	DTMBIP_SetSpectrum(DTMB_SPECTRUM_NEGATIVE);
	DTMBIP_SoftReset();
#else
	DTMBIP_SoftReset();
	for (CheckLockTemp = 0; CheckLockTemp < 30; CheckLockTemp++) {
		tv_osal_msleep(100); /* wait 100 ms */
		DTMBIP_IsDemodLocked(TRUE, &LockResult, &locked, &status);
		if ((status & 0xf0) > 0x20) {
			SignalDetect = TRUE;
		}
		if ((SignalDetect == FALSE) && (CheckLockTemp == 15)) {
			REALTEK_H_DTMB_BASE_WARNING("---> [DTMBIP] DTMBIP DTMB No Signal \n");
			return FALSE;
		}
		if (LockResult == 1) 
			{
			REALTEK_H_DTMB_BASE_DBG("---> [DTMBIP] DTMBIP DTMB is locked \n");
			return TRUE;
		}
	}
#endif
	return FALSE;
}

/********************************
Manual Set Dtmb Frequency
********************************/
BOOL DTMBIP_DTMBManualSetFrequency(UINT32 Frequency, UINT8 CarrierMode, UINT8 mode, UINT8 rate, UINT8 interleaverLength, UINT8 pnmode, UINT8 PnVariable)
{
	UINT8 CheckLockTemp, LockResult, locked, status = 0;
	BOOL SignalDetect = FALSE;
#if DTMBIP_SPECTRUM_ADJUST_OPEN
	UINT8 ReadData0 = 0, ReadData1 = 0, ReadData2 = 0;
#endif

	/********************
	** It needs to set tuner first
	********************/
	if (DTMBIP_TunerLock(Frequency) != TRUE) {
		return FALSE;
	}

	/* Set Manual Mode */
	DTMBIP_SetManualMode(CarrierMode, mode, rate, interleaverLength, pnmode, PnVariable);
#if DTMBIP_SPECTRUM_ADJUST_OPEN
	DTMBIP_SetSpectrum(DTMB_SPECTRUM_NEGATIVE);
	DTMBIP_SoftReset();

	for (CheckLockTemp = 0; CheckLockTemp < 50; CheckLockTemp++) {
		//DTMBIP_Wait(100); /* wait 100 ms */
		tv_osal_msleep(100); /* wait 100 ms */
		DTMBIP_IsDemodLocked(TRUE, &LockResult, &locked, &status);
		if ((status & 0xf0) > 0x20) { /* When no signal continues 2 secs, it will exit */
			SignalDetect = TRUE;
		}
		if ((SignalDetect == FALSE) && (CheckLockTemp == 15)) {
			REALTEK_H_DTMB_BASE_WARNING("---> [DTMBIP] DTMBIP DTMB No Signal \n");
			return FALSE;
		}
		if (LockResult == 1) {
			DTMBIP_ReadRegister(0x0006, &ReadData0);
			tv_osal_msleep(100); /* wait 100 ms */
			DTMBIP_ReadRegister(0x0006, &ReadData1);
			tv_osal_msleep(100); /* wait 100 ms */
			DTMBIP_ReadRegister(0x0006, &ReadData2);
			CheckLockTemp++;
			CheckLockTemp++;
			if ((ReadData0 == ReadData1) && (ReadData1 == ReadData2)) {
				REALTEK_H_DTMB_BASE_DBG("---> [DTMBIP] DTMBIP DTMB is locked \n");
				return TRUE;
			} else {
				break;
			}
		}
	}
	/*Do Spectrum Adjust Try*/
	DTMBIP_SetSpectrum(DTMB_SPECTRUM_POSITIVE);
	DTMBIP_SoftReset();
	SignalDetect = FALSE;

	for (CheckLockTemp = 0; CheckLockTemp < 20; CheckLockTemp++) {
		tv_osal_msleep(100); /* wait 100 ms */
		DTMBIP_IsDemodLocked(TRUE, &LockResult, &locked, &status);
		if ((status & 0xf0) > 0x20) {
			SignalDetect = TRUE;
		}
		if ((SignalDetect == FALSE) && (CheckLockTemp == 15)) {
			REALTEK_H_DTMB_BASE_WARNING("---> [DTMBIP] DTMBIP DTMB No Signal \n");
			return FALSE;
		}
		if (LockResult == 1) {
			//ReadData0 = rtd_inl(0xB8140018);
			DTMBIP_ReadRegister(0x0006, &ReadData0);
			tv_osal_msleep(100); /* wait 100 ms */
			//ReadData1 = rtd_inl(0xB8140018);
			DTMBIP_ReadRegister(0x0006, &ReadData1);
			tv_osal_msleep(100); /* wait 100 ms */
			//ReadData0 = rtd_inl(0xB8140018);
			DTMBIP_ReadRegister(0x0006, &ReadData2);
			CheckLockTemp++;
			CheckLockTemp++;
			if ((ReadData0 == ReadData1) && (ReadData1 == ReadData2)) {
				REALTEK_H_DTMB_BASE_DBG("---> [DTMBIP] DTMBIP DTMB is locked \n");
				return TRUE;
			} else {
				break;
			}
		}
	}
	DTMBIP_SetSpectrum(DTMB_SPECTRUM_NEGATIVE);
	DTMBIP_SoftReset();
#else
	DTMBIP_SoftReset();
	for (CheckLockTemp = 0; CheckLockTemp < 30; CheckLockTemp++) {
		tv_osal_msleep(100); /* wait 100 ms */
		DTMBIP_IsDemodLocked(TRUE, &LockResult, &locked, &status);
		if ((status & 0xf0) > 0x20) { /*���ź�2���˳�*/
			SignalDetect = TRUE;
		}
		if ((SignalDetect == FALSE) && (CheckLockTemp == 15)) {
			REALTEK_H_DTMB_BASE_WARNING("---> [DTMBIP] DTMBIP DTMB No Signal \n");
			return FALSE;
		}
		if (LockResult == 1) {
			REALTEK_H_DTMB_BASE_DBG("---> [DTMBIP] DTMBIP DTMB is locked \n");
			return TRUE;
		}
	}
#endif
	return FALSE;
}

