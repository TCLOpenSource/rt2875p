/**

@file

@brief   DVB-T demod default function definition

DVB-T demod default functions.

*/

#include <linux/slab.h>
#include "dvbtx_demod_rtk_a_base.h"

static int RTK_A_DVBTX_LeftTable(int i32InValue);

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



/****************************************************************************
Function:    RTK_A_DVBTX_Read
Parameters:  ui8BaseAddr Chip Base Address,eg.0x88; ui8RegisterAddr:Chip Sub Address;
ui8RegValue:The Pointer Which Return The Data Read Form Chip.
Return:      Read  Status:RTK_A_DVBTX_SUCCESSFUL is Normal and other is abnormal.

Description:
This function used to read  data form chip register
****************************************************************************/
RTK_A_DVBTX_STATUS   RTK_A_DVBTX_Read(DVBTX_DEMOD_MODULE *pDemod, unsigned char ui8BaseAddr, unsigned char ui8RegisterAddr, unsigned char *ui8RegValue)
{
	RTK_A_DVBTX_STATUS ui8Status = RTK_A_DVBTX_SUCCESSFUL;

	ui8Status |= RTK_A_DVBTX_I2CRead(pDemod, ui8BaseAddr, ui8RegisterAddr, ui8RegValue);
	//RTK_DEMOD_INFO("[ATB][RTK_A_DVBTX_Read] 0x%02x%02x = 0x%02x\n",ui8BaseAddr,ui8RegisterAddr, *ui8RegValue);
	return ui8Status;
}


/****************************************************************************
Function:    RTK_A_DVBTX_Write
Parameters:  ui8BaseAddr Chip Base Address,eg.0x88; ui8RegisterAddr:Chip Sub Address;
ui8data:Data which will be write to chip.
Return:      Write status:RTK_A_DVBTX_SUCCESSFUL is normal and other is abnormal.

Description:
This function used to write data to chip register
****************************************************************************/
RTK_A_DVBTX_STATUS   RTK_A_DVBTX_Write(DVBTX_DEMOD_MODULE *pDemod, unsigned char ui8BaseAddr, unsigned char ui8RegisterAddr, unsigned char ui8data)
{
	RTK_A_DVBTX_STATUS ui8Status = RTK_A_DVBTX_SUCCESSFUL;
	ui8Status |= RTK_A_DVBTX_I2CWrite(pDemod, ui8BaseAddr, ui8RegisterAddr, ui8data);
	//RTK_DEMOD_INFO("[ATB][RTK_A_DVBTX_Write] 0x%02x%02x = 0x%02x\n",ui8BaseAddr,ui8RegisterAddr, ui8data);
	//RTK_DEMOD_INFO("%2x  %2x  %2x\n",ui8BaseAddr,ui8RegisterAddr,ui8data);
	return ui8Status;
}

/****************************************************************************
Function:    RTK_A_DVBTX_WriteRegArray
Parameters:  *ui8ARegTable, i32TableLen
Return:      I2C Write Status   0 is Success,Other is Failed

Description:
This function writes a group of registers to demodulator
****************************************************************************/
RTK_A_DVBTX_STATUS  RTK_A_DVBTX_WriteRegArray(DVBTX_DEMOD_MODULE *pDemod, unsigned char *ui8ARegTable, int i32TableLen)
{
	RTK_A_DVBTX_STATUS ui8Status = RTK_A_DVBTX_SUCCESSFUL;
	int i32Iter = 0;
	for (i32Iter = 0; i32Iter < i32TableLen; i32Iter += 3) {
		if ((i32Iter + 2) < i32TableLen) {
			ui8Status |= RTK_A_DVBTX_Write(pDemod, ui8ARegTable[i32Iter], ui8ARegTable[i32Iter + 1], ui8ARegTable[i32Iter + 2]);
		}
		/*RTK_DEMOD_INFO("%x %x %x\n", ui8ARegTable[i32Iter],ui8ARegTable[i32Iter+1], ui8ARegTable[i32Iter+2]); 		*/
	}
	return ui8Status;
}



/********************** 2 32bit data multiply, use this function, internally used function *****************/
void RTK_A_DVBTX_64Mult(RTK_A_DVBTX_64Data *pstRst, unsigned int m1, unsigned int m2)
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
unsigned int RTK_A_DVBTX_64Div(RTK_A_DVBTX_64Data stDivisor, RTK_A_DVBTX_64Data stDividend)
{
	unsigned char  ui8DivLeftShit = 0;
	unsigned int ui32Rst = 0;
	unsigned int ui32flag = 1;
	RTK_A_DVBTX_64Data stDataTmp = stDividend;
	if (stDivisor.ui32High == 0 && stDivisor.ui32Low == 0) {
		return 0;
	} else if (stDividend.ui32High == 0 && stDividend.ui32Low == 0) {
		return 0;
	} else if (stDividend.ui32High > stDivisor.ui32High) {
		return 0;
	} else if (!RTK_A_DVBTX_64ComPare(stDivisor, stDividend)) {
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

			if (!RTK_A_DVBTX_64ComPare(stDivisor, stDataTmp) || stDataTmp.ui32High >= 0x80000000) {
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
		ui32Rst += RTK_A_DVBTX_64Div(stDivisor, stDividend);
	}
	return ui32Rst;
}

/**********************64bit divide function,return 64bit value internally used function *******************************/
RTK_A_DVBTX_64Data RTK_A_DVBTX_64DivReturn64(RTK_A_DVBTX_64Data stDivisor, RTK_A_DVBTX_64Data stDividend)
{
	unsigned char  ui8DivLeftShit = 0;
	unsigned int ui32LowPart = 0;
	unsigned int ui32flag = 1;
	RTK_A_DVBTX_64Data i64Data, i64Data1;
	RTK_A_DVBTX_64Data stDataTmp = stDividend;
	i64Data.ui32High = 0;
	i64Data.ui32Low = 0;
	if (stDivisor.ui32High == 0 && stDivisor.ui32Low == 0) {
		return i64Data;
	} else if (stDividend.ui32High == 0 && stDividend.ui32Low == 0) {
		return i64Data;
	} else if (stDividend.ui32High > stDivisor.ui32High) {
		return i64Data;
	} else if (!RTK_A_DVBTX_64ComPare(stDivisor, stDividend)) {
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
			if (!RTK_A_DVBTX_64ComPare(stDivisor, stDataTmp) || stDataTmp.ui32High >= 0x80000000) {
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
		i64Data1 = RTK_A_DVBTX_64DivReturn64(stDivisor, stDividend);
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
unsigned int RTK_A_DVBTX_Log10ConvertdBx10(unsigned int i32InValue)
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
	ui32LogValue = ui32LogValue * 10 + RTK_A_DVBTX_LeftTable(ui32InValueLeft);
	/*RTK_DEMOD_INFO(" i32InValue = %d  log ret = %d\n", i32InValue,  ui32LogValue);*/
	return ui32LogValue;
}

/**********************Fixed version of 10 * log10(i32InValue), internally used function ***********************/
unsigned int RTK_A_DVBTX_Log10ConvertdB(unsigned int i32InValue)
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
	ui32LogValue = ui32LogValue + RTK_A_DVBTX_LeftTable(ui32InValueLeft) / 10;
	/*RTK_DEMOD_INFO(" i32InValue = %d  log ret = %d\n", i32InValue,  ui32LogValue);*/
	return ui32LogValue;
}


/********************* Fixed version of pow(x,y), y should greater than 0, internally used function **************/
unsigned int  RTK_A_DVBTX_FixPow(unsigned int x, unsigned int y)
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
void  RTK_A_DVBTX_MiddleValueu8(unsigned char u8ValueA, unsigned char u8ValueB, unsigned char u8ValueC, unsigned char *MidValue)
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
unsigned char  RTK_A_DVBTX_64ComPare(RTK_A_DVBTX_64Data stPara, RTK_A_DVBTX_64Data stPara1)
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

static int RTK_A_DVBTX_LeftTable(int i32InValue)
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


//I2C
//Set external demod I2C control bus and ATBM externel demod I2C address
#define ATB_EXT_DEMOD_I2C_8bits_ADDR 0x80

/****************************************************************************
Function:    RTK_A_DVBTX_I2CRead
Parameters:   ui8BaseAddr register Base Address, ui8RegisterAddr:register offset Address;
ui8RegValue: The pointer which return the data read form Chip.
Return:      Read  Status:RTK_A_DVBTX_SUCCESSFUL is Normal and other is abnormal.

Description:
This function used I2CRead to read  data form chip register, it should be implemented by user
****************************************************************************/
RTK_A_DVBTX_STATUS   RTK_A_DVBTX_I2CRead(DVBTX_DEMOD_MODULE *pDemod, unsigned char ui8BaseAddr, unsigned char ui8RegisterAddr, unsigned char *ui8RegValue)
{

       BASE_INTERFACE_MODULE *pBaseInterface;
	unsigned char ui8AddrData[2] = {0};

	pBaseInterface = pDemod->pBaseInterface;

	ui8AddrData[0] = ui8BaseAddr;
	ui8AddrData[1] = ui8RegisterAddr;


	if (pBaseInterface->I2cWrite(pBaseInterface, ATB_EXT_DEMOD_I2C_8bits_ADDR, ui8AddrData, 2) != FUNCTION_SUCCESS)
		return RTK_A_DVBTX_I2C_ERROR;

	if (pBaseInterface->I2cRead(pBaseInterface, ATB_EXT_DEMOD_I2C_8bits_ADDR, NULL, 0, ui8RegValue, 1) != FUNCTION_SUCCESS)
		return RTK_A_DVBTX_I2C_ERROR;


	//LGIT_LIB_I2cWrite(0x80, ui8AddrData, 2);		//lgit
	//LGIT_LIB_I2cRead(0x80, ui8RegValue, 1);		//lgit


	/*Debug RTK_A_DVBTX_ I2C used print the chip ID*/
	//if((ui8BaseAddr == 0x00)&&(ui8RegisterAddr == 0x00))
	//*ui8RegValue = 0xa8;

	//ATBMDemodI2CRead(ui8RTK_A_DVBTX_SlaveAddress,ui8BaseAddr, ui8RegisterAddr,1,ui8RegValue);

	/*One step I2C format, please port this function according to I2C format of this function..*/
	/*return I2CRead(ui8RTK_A_DVBTX_SlaveAddr, 2, ui8AddrData, 1, ui8RegValue);*/

	return RTK_A_DVBTX_SUCCESSFUL;
}
/****************************************************************************
Function:    RTK_A_DVBTX_I2CWrite
Parameters:  ui8BaseAddr register Base Address, ui8RegisterAddr:register offset Address;
ui8data:  Data which will be write to chip.
Return:         Write status:RTK_A_DVBTX_SUCCESSFUL is normal and other is abnormal.

Description:
This function used I2CWrite to write data to chip register,it should be implemented by user
****************************************************************************/
RTK_A_DVBTX_STATUS   RTK_A_DVBTX_I2CWrite(DVBTX_DEMOD_MODULE *pDemod, unsigned char ui8BaseAddr, unsigned char ui8RegisterAddr, unsigned char ui8data)
{
	RTK_A_DVBTX_STATUS ui8Status =  RTK_A_DVBTX_SUCCESSFUL;
	BASE_INTERFACE_MODULE *pBaseInterface;

	unsigned char ui8I2CData[3] = {0};

	pBaseInterface = pDemod->pBaseInterface;

	ui8I2CData[0] = ui8BaseAddr;
	ui8I2CData[1] = ui8RegisterAddr;
	ui8I2CData[2] = ui8data;


	if (pBaseInterface->I2cWrite(pBaseInterface, ATB_EXT_DEMOD_I2C_8bits_ADDR, ui8I2CData, 3) != FUNCTION_SUCCESS)
		return RTK_A_DVBTX_I2C_ERROR;


	return ui8Status;
}

