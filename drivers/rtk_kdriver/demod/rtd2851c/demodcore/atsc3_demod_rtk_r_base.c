/**

@file

@brief   DVB-T demod default function definition

DVB-T demod default functions.

*/

#include <linux/slab.h>
#include "atsc3_demod_rtk_r_base.h"

static int RTK_R_ATSC3_LeftTable(int i32InValue);

static int i32SNRLogTable[] = {
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



/********************** 2 32bit data multiply, use this function, internally used function *****************/
void RTK_R_ATSC3_64Mult(RTK_R_ATSC3_64Data *pstRst, unsigned int m1, unsigned int m2)
{
	unsigned int ui32LowPart = 0, ui32Tmp = 0;
	pstRst->ui32Low = (m1 & 0xffff) * (m2 & 0xffff);
	pstRst->ui32High = (m1 >> 16) * (m2 >> 16);

	ui32LowPart = pstRst->ui32Low;
	ui32Tmp = (m1 >> 16) * (m2 & 0xffff);
	pstRst->ui32Low  += (ui32Tmp << 16);
	pstRst->ui32High += (ui32Tmp >> 16);
	if (ui32LowPart > pstRst->ui32Low) {
		pstRst->ui32High++;
	}

	ui32LowPart = pstRst->ui32Low;
	ui32Tmp = (m2 >> 16) * (m1 & 0xffff);
	pstRst->ui32Low  += (ui32Tmp << 16);
	pstRst->ui32High += (ui32Tmp >> 16);
	if (ui32LowPart > pstRst->ui32Low) {
		pstRst->ui32High++;
	}
}



/**********************64bit divide function, return 32bit value, internally used function *************************************/
unsigned int RTK_R_ATSC3_64Div(RTK_R_ATSC3_64Data stDivisor, RTK_R_ATSC3_64Data stDividend)
{
	unsigned char  ui8DivLeftShit = 0;
	unsigned int ui32Rst = 0;
	unsigned int ui32flag = 1;
	RTK_R_ATSC3_64Data stDataTmp = stDividend;
	if (stDivisor.ui32High == 0 && stDivisor.ui32Low == 0) {
		return 0;
	} else if (stDividend.ui32High == 0 && stDividend.ui32Low == 0) {
		return 0;
	} else if (stDividend.ui32High > stDivisor.ui32High) {
		return 0;
	} else if (!RTK_R_ATSC3_64ComPare(stDivisor, stDividend)) {
		return 0;
	} else if (stDividend.ui32High == 0 && stDivisor.ui32High == 0) {
		return stDivisor.ui32Low / stDividend.ui32Low;
	} else { /* normal condition  stDivisor >= stDividend*/
		while (ui32flag) {
			++ui8DivLeftShit;
			stDataTmp.ui32High <<= 1;
			if (stDataTmp.ui32Low & 0x80000000) {
				stDataTmp.ui32High += 1;
			}
			stDataTmp.ui32Low <<= 1;

			if (!RTK_R_ATSC3_64ComPare(stDivisor, stDataTmp) || stDataTmp.ui32High >= 0x80000000) {
				if (stDataTmp.ui32High < 0x80000000) {
					stDataTmp.ui32Low >>= 1;
					if (stDataTmp.ui32High & 1) {
						stDataTmp.ui32Low |= 0x80000000;
					}
					stDataTmp.ui32High >>= 1;
					ui8DivLeftShit--;
				}


				stDivisor.ui32High -= stDataTmp.ui32High;
				if (stDivisor.ui32Low >= stDataTmp.ui32Low) {
					stDivisor.ui32Low -= stDataTmp.ui32Low;
				} else {
					stDivisor.ui32Low += 0xffffffff - stDataTmp.ui32Low;
					stDivisor.ui32Low += 1;
					stDivisor.ui32High -= 1;
				}
				ui32Rst = (1 << ui8DivLeftShit);
				break;
			}
		}
		ui32Rst += RTK_R_ATSC3_64Div(stDivisor, stDividend);
	}
	return ui32Rst;
}

/**********************64bit divide function,return 64bit value internally used function *******************************/
RTK_R_ATSC3_64Data RTK_R_ATSC3_64DivReturn64(RTK_R_ATSC3_64Data stDivisor, RTK_R_ATSC3_64Data stDividend)
{
	unsigned char  ui8DivLeftShit = 0;
	unsigned int ui32LowPart = 0;
	unsigned int ui32flag = 1;
	RTK_R_ATSC3_64Data i64Data, i64Data1;
	RTK_R_ATSC3_64Data stDataTmp = stDividend;
	i64Data.ui32High = 0;
	i64Data.ui32Low = 0;
	if (stDivisor.ui32High == 0 && stDivisor.ui32Low == 0) {
		return i64Data;
	} else if (stDividend.ui32High == 0 && stDividend.ui32Low == 0) {
		return i64Data;
	} else if (stDividend.ui32High > stDivisor.ui32High) {
		return i64Data;
	} else if (!RTK_R_ATSC3_64ComPare(stDivisor, stDividend)) {
		return i64Data;
	} else if (stDividend.ui32High == 0 && stDivisor.ui32High == 0) {
		i64Data.ui32Low = stDivisor.ui32Low / stDividend.ui32Low;
		return i64Data;
	} else { /* normal condition  stDivisor >= stDividend*/
		while (ui32flag) {
			++ui8DivLeftShit;
			stDataTmp.ui32High <<= 1;
			if (stDataTmp.ui32Low & 0x80000000) {
				stDataTmp.ui32High += 1;
			}
			stDataTmp.ui32Low <<= 1;
			if (!RTK_R_ATSC3_64ComPare(stDivisor, stDataTmp) || stDataTmp.ui32High >= 0x80000000) {
				if (stDataTmp.ui32High < 0x80000000) {
					stDataTmp.ui32Low >>= 1;
					if (stDataTmp.ui32High & 1) {
						stDataTmp.ui32Low |= 0x80000000;
					}
					stDataTmp.ui32High >>= 1;
					ui8DivLeftShit--;
				}


				stDivisor.ui32High -= stDataTmp.ui32High;
				if (stDivisor.ui32Low >= stDataTmp.ui32Low) {
					stDivisor.ui32Low -= stDataTmp.ui32Low;
				} else {
					stDivisor.ui32Low += 0xffffffff - stDataTmp.ui32Low;
					stDivisor.ui32Low += 1;
					stDivisor.ui32High -= 1;
				}
				if (ui8DivLeftShit >= 32) {
					i64Data.ui32High = (1 << (ui8DivLeftShit - 32));
					i64Data.ui32Low = 0;
				} else {
					i64Data.ui32Low = (1 << ui8DivLeftShit);
					i64Data.ui32High = 0;
				}
				break;
			}
		}
		i64Data1 = RTK_R_ATSC3_64DivReturn64(stDivisor, stDividend);
		ui32LowPart = i64Data.ui32Low;
		i64Data.ui32Low += i64Data1.ui32Low;
		i64Data.ui32High += i64Data1.ui32High;
		if (ui32LowPart > i64Data.ui32Low) {
			i64Data.ui32High += 1;
		}

	}
	return i64Data;
}


/**********************Fixed version of 100 * log10(i32InValue), internally used function ************************/
unsigned int RTK_R_ATSC3_Log10ConvertdBx10(unsigned int i32InValue)
{
	unsigned char       		ui8Index = 0;
	unsigned int		        ui32Tmp = 1;
	unsigned int              ui32InValueLeft = 1;
	unsigned int              ui32LogValue;
	unsigned int              ui32Dvalue = 0;
	if (i32InValue == 0) {
		return 0;
	}

	do {
		ui32Tmp = ui32Tmp << 1;
		if (i32InValue < ui32Tmp) {
			break;
		}
	} while (++ui8Index < 31); //} while (++ui8Index < 32); for coverity issue
	ui32LogValue = 301 * ui8Index / 100;
	ui32Dvalue = (1 << ui8Index);
	if (ui32Dvalue != 0) {
		ui32InValueLeft = i32InValue * 100 / ui32Dvalue;
	}
	ui32LogValue = ui32LogValue * 10 + RTK_R_ATSC3_LeftTable(ui32InValueLeft);
	/*RTK_DEMOD_INFO(" i32InValue = %d  log ret = %d\n", i32InValue,  ui32LogValue);*/
	return ui32LogValue;
}

/**********************Fixed version of 10 * log10(i32InValue), internally used function ***********************/
unsigned int RTK_R_ATSC3_Log10ConvertdB(unsigned int i32InValue)
{
	unsigned char       		ui8Index = 0;
	unsigned int		        ui32Tmp = 1;
	unsigned int              ui32InValueLeft = 1;
	unsigned int              ui32LogValue = 0;
	unsigned int              ui32Dvalue = 0;
	if (i32InValue == 0) {
		return 0;
	}

	do {
		ui32Tmp = ui32Tmp << 1;
		if (i32InValue < ui32Tmp) {
			break;
		}
	} while (++ui8Index < 31); //} while (++ui8Index < 32); for coverity issue
	ui32LogValue = 301 * ui8Index / 100;

	ui32Dvalue = (1 << ui8Index);
	if (ui32Dvalue != 0) {
		ui32InValueLeft = i32InValue * 100 / ui32Dvalue;
	}
	ui32LogValue = ui32LogValue + RTK_R_ATSC3_LeftTable(ui32InValueLeft) / 10;
	/*RTK_DEMOD_INFO(" i32InValue = %d  log ret = %d\n", i32InValue,  ui32LogValue);*/
	return ui32LogValue;
}


/********************* Fixed version of pow(x,y), y should greater than 0, internally used function **************/
unsigned int  RTK_R_ATSC3_FixPow(unsigned int x, unsigned int y)
{
	unsigned int ui32Rst = x;
	unsigned int ui32Iter = 0;
	if (y == 0) {
		return 1;
	}

	for (ui32Iter = 1; ui32Iter < y; ui32Iter++) {
		ui32Rst *= x;
	}
	return ui32Rst;
}



/*Get one middle value in three input value*/
void  RTK_R_ATSC3_MiddleValueu8(unsigned char u8ValueA, unsigned char u8ValueB, unsigned char u8ValueC, unsigned char *MidValue)
{
	if ((u8ValueA == 0) && (u8ValueB == 0)) { //u8ValueC may be start value after locked
		*MidValue = u8ValueC;
		return;
	}

	if ((u8ValueA == u8ValueB) || (u8ValueA == u8ValueC)) { /*if have equal value, select one of them.*/
		*MidValue = u8ValueA;
		return;
	}
	if (u8ValueB == u8ValueC) {
		*MidValue = u8ValueB;
		return;
	}
	/*No equal value.*/
	if (u8ValueA > u8ValueB) {
		if (u8ValueA > u8ValueC) {
			*MidValue  = u8ValueB > u8ValueC ?  u8ValueB : u8ValueC; /*Get big value*/
		} else {
			*MidValue  = u8ValueA;
		}
	} else {
		if (u8ValueB > u8ValueC) {
			*MidValue  = u8ValueA > u8ValueC ? u8ValueA : u8ValueC; /*Get big value*/
		} else {
			*MidValue  = u8ValueB;
		}
	}
}


/**********************  internally used function *************************************************************/
unsigned char  RTK_R_ATSC3_64ComPare(RTK_R_ATSC3_64Data stPara, RTK_R_ATSC3_64Data stPara1)
{
	unsigned char ui8Flag = 0;/*default :stPara<=stPara1*/
	if (stPara.ui32Low >= stPara1.ui32Low) {
		if (stPara.ui32High >= stPara1.ui32High) {
			ui8Flag = 1;
		}
	} else {
		if (stPara.ui32High > stPara1.ui32High) {
			ui8Flag = 1;
		}
	}
	return ui8Flag;

}

/****************************************************************************
Function:   LeftTable
Description:
use for ATBM783xLog10ConvertdB
******************************************************************************/

static int RTK_R_ATSC3_LeftTable(int i32InValue)
{
	int i32OutValue = 0;
	int i32Iter = 0;
	int i32TableSize = sizeof(i32SNRLogTable) / sizeof(int);
	for (i32Iter = 0; i32Iter < i32TableSize; i32Iter += 2) {
		if ((i32Iter + 1) < i32TableSize) {
			if (i32InValue <= i32SNRLogTable[i32Iter + 1]) {
				i32OutValue = i32SNRLogTable[i32Iter];
				return i32OutValue;
			}
		}
		/*	printf("iter = %d, size:%d\n", i32Iter,  i32TableSize);*/
	}
	i32OutValue = 32;
	return i32OutValue;
}


//////  I2C   ////////

int rtd_4bytes_i2c_wr(ATSC3_DEMOD_MODULE *pDemod, unsigned int addr, unsigned int data)
{

	BASE_INTERFACE_MODULE *pBaseInterface;
	unsigned char DeviceAddr;
	unsigned char WritingBuffer[8];
	unsigned int WritingByteNum = 8;


	// Get base interface.
	pBaseInterface = pDemod->pBaseInterface;


	// Get demod I2C device address.
	pDemod->GetDeviceAddr(pDemod, &DeviceAddr);


       addr &= 0x1FFFFFFF;  


	WritingBuffer[0] = addr & BYTE_MASK;
	WritingBuffer[1] = (addr >> 8) & BYTE_MASK;
	WritingBuffer[2] = (addr >> 16) & BYTE_MASK;
	WritingBuffer[3] = (addr >> 24) & BYTE_MASK;

	WritingBuffer[4] = data & BYTE_MASK;
	WritingBuffer[5] = (data >> 8) & BYTE_MASK;
	WritingBuffer[6] = (data >> 16) & BYTE_MASK;
	WritingBuffer[7] = (data >> 24) & BYTE_MASK;

	if (pBaseInterface->I2cWrite(pBaseInterface, DeviceAddr, WritingBuffer, WritingByteNum) != FUNCTION_SUCCESS)
		goto error_4bytes_i2c_wr;


	return FUNCTION_SUCCESS;

error_4bytes_i2c_wr:
	return FUNCTION_ERROR;

}


int rtd_4bytes_i2c_rd(ATSC3_DEMOD_MODULE *pDemod, unsigned int addr, unsigned int* data)
{

	BASE_INTERFACE_MODULE *pBaseInterface;
	unsigned char DeviceAddr;
	unsigned char WritingBuffer[4], ReadingBuffer[4];
	unsigned int ReadingByteNum = 4, WritingByteNum = 4;



	// Get base interface.
	pBaseInterface = pDemod->pBaseInterface;

	// Get demod I2C device address.
	pDemod->GetDeviceAddr(pDemod, &DeviceAddr);

       addr &= 0x1FFFFFFF; 

	WritingBuffer[0] = addr & BYTE_MASK;
	WritingBuffer[1] = (addr >> 8) & BYTE_MASK;
	WritingBuffer[2] = (addr >> 16) & BYTE_MASK;
	WritingBuffer[3] = (addr >> 24) & BYTE_MASK;

	// Set demod register reading address.
	// Note: The I2C format of demod register reading address setting is as follows:
	//       start_bit + (DeviceAddr | writing_bit) + RegReadingAddrMsb + RegReadingAddrLsb + stop_bit

	//if (pBaseInterface->I2cWrite(pBaseInterface, DeviceAddr, WritingBuffer, 4) != FUNCTION_SUCCESS)
	//	goto error_status_set_demod_register_reading_address;

	// Get demod register bytes.
	// Note: The I2C format of demod register byte getting is as follows:
	//       start_bit + (DeviceAddr | reading_bit) + reading_bytes (ReadingByteNum bytes) + stop_bit
	if (pBaseInterface->I2cRead(pBaseInterface, DeviceAddr, WritingBuffer, WritingByteNum, ReadingBuffer, ReadingByteNum) != FUNCTION_SUCCESS)
		goto error_status_get_demod_registers;


	*data = (ReadingBuffer[3] << 24) | (ReadingBuffer[2] << 16) | (ReadingBuffer[1] << 8) | (ReadingBuffer[0]);


	return FUNCTION_SUCCESS;


error_status_get_demod_registers:
//error_status_set_demod_register_reading_address:
	return FUNCTION_ERROR;

}


int rtd_4bytes_i2c_mask(ATSC3_DEMOD_MODULE *pDemod, unsigned int addr, unsigned int mask, unsigned int data)
{

	int ret = FUNCTION_SUCCESS;
	unsigned int readingData = 0, writingData = 0;

	ret = rtd_4bytes_i2c_rd(pDemod, addr, &readingData);
	writingData = (readingData & mask) | data;
	ret |= rtd_4bytes_i2c_wr(pDemod, addr, writingData);

	return ret;

}




/**

@see   ATSC_DEMOD_FP_GET_DEMOD_TYPE

*/
void
atsc3_demod_default_GetDemodType(
	ATSC3_DEMOD_MODULE *pDemod,
	int *pDemodType
)
{
	// Get demod type from demod module.
	*pDemodType = pDemod->DemodType;


	return;
}





/**

@see   ATSC_DEMOD_FP_GET_DEVICE_ADDR

*/
void
atsc3_demod_default_GetDeviceAddr(
	ATSC3_DEMOD_MODULE *pDemod,
	unsigned char *pDeviceAddr
)
{
	// Get demod I2C device address from demod module.
	*pDeviceAddr = pDemod->DeviceAddr;


	return;
}





/**

@see   ATSC_DEMOD_FP_GET_CRYSTAL_FREQ_HZ

*/
void
atsc3_demod_default_GetCrystalFreqHz(
	ATSC3_DEMOD_MODULE *pDemod,
	unsigned int *pCrystalFreqHz
)
{
	// Get demod crystal frequency in Hz from demod module.
	*pCrystalFreqHz = pDemod->CrystalFreqHz;


	return;
}





/**

@see   ATSC_DEMOD_FP_GET_IF_FREQ_HZ

*/
int
atsc3_demod_default_GetIfFreqHz(
	ATSC3_DEMOD_MODULE *pDemod,
	unsigned int *pIfFreqHz
)
{
	// Get demod IF frequency in Hz from demod module.
	if (pDemod->IsIfFreqHzSet != YES)
		goto error_status_get_demod_if_frequency;

	*pIfFreqHz = pDemod->IfFreqHz;


	return FUNCTION_SUCCESS;


error_status_get_demod_if_frequency:
	return FUNCTION_ERROR;
}





/**

@see   ATSC_DEMOD_FP_GET_SPECTRUM_MODE

*/
int
atsc3_demod_default_GetSpectrumMode(
	ATSC3_DEMOD_MODULE *pDemod,
	int *pSpectrumMode
)
{
	// Get demod spectrum mode from demod module.
	if (pDemod->IsSpectrumModeSet != YES)
		goto error_status_get_demod_spectrum_mode;

	*pSpectrumMode = pDemod->SpectrumMode;


	return FUNCTION_SUCCESS;


error_status_get_demod_spectrum_mode:
	return FUNCTION_ERROR;
}

