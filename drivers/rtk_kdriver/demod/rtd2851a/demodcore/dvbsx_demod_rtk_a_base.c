/*****************************************************************************
Copyright 2018, Realtek Inc. All rights reserved.
File Name: dvbsx_demod_rtk_a_base.c
Version: 1.0.9
Released 2018-03-15
Description: programming interface for RTK Demod
******************************************************************************/
#include "tv_osal.h"
#include "dvbsx_demod_rtk_a.h"

stRtkDemodDvbConfig gstRtkDemodDvbConfig;
unsigned char gRtkDemodPowerOnInited = 0;

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
Function:    RTK_A_DVBSX_RbusRead
Parameters:  u8BaseAddr Chip Base Address,eg.0x88; u8OffsetAddr:Chip Sub Address;
u8RegValue:The Pointer Which Return The Data Read Form Chip.
Return:      Read  Status:RTK_DEMOD_SUCCESSFUL is Normal and other is abnormal.

Description:
This function used to read  data form chip register
****************************************************************************/
RTK_A_DVBSX_STATUS   RTK_A_DVBSX_Read(DVBSX_DEMOD_MODULE *pDemod, unsigned char u8BaseAddr, unsigned char u8OffsetAddr, unsigned char *u8RegValue)
{
	RTK_A_DVBSX_STATUS ui8Status = RTK_DEMOD_SUCCESSFUL;

	ui8Status |= RTK_A_DVBSX_I2CRead(pDemod, u8BaseAddr, u8OffsetAddr, u8RegValue);
	//ui8Status |= rbus_demod_read(u8BaseAddr, u8OffsetAddr, u8RegValue);
	
	return ui8Status;
}

/****************************************************************************
Function:    RTK_A_DVBSX_RbusWrite
Parameters:  u8BaseAddr Chip Base Address,eg.0x88; u8OffsetAddr:Chip Sub Address;
ui8data:Data which will be write to chip.
Return:      Write status:RTK_DEMOD_SUCCESSFUL is normal and other is abnormal.

Description:
This function used to write data to chip register
****************************************************************************/
RTK_A_DVBSX_STATUS   RTK_A_DVBSX_Write(DVBSX_DEMOD_MODULE *pDemod, unsigned char u8BaseAddr, unsigned char u8OffsetAddr, unsigned char ui8data)
{
	RTK_A_DVBSX_STATUS ui8Status = RTK_DEMOD_SUCCESSFUL;
	
	ui8Status |= RTK_A_DVBSX_I2CWrite(pDemod, u8BaseAddr, u8OffsetAddr, ui8data);
	//ui8Status |= rbus_demod_write(u8BaseAddr, u8OffsetAddr, ui8data);

	return ui8Status;
}

/****************************************************************************
Function:    RTK_A_DVBSX_WriteRegArray
Parameters:  *ui8ARegTable, i32TableLen
Return:      I2C Write Status   0 is Success,Other is Failed

Description:
This function writes a group of registers to demodulator
****************************************************************************/
RTK_A_DVBSX_STATUS  RTK_A_DVBSX_WriteRegArray(DVBSX_DEMOD_MODULE *pDemod, unsigned char *ui8ARegTable, int i32TableLen)
{
	RTK_A_DVBSX_STATUS ui8Status = RTK_DEMOD_SUCCESSFUL;
	int i32Iter = 0;

	for (i32Iter = 0; i32Iter < i32TableLen; i32Iter += 3)
	{
		if ((i32Iter + 2) < i32TableLen)
		{
			ui8Status |= RTK_A_DVBSX_Write(pDemod, ui8ARegTable[i32Iter], ui8ARegTable[i32Iter + 1], ui8ARegTable[i32Iter + 2]);
		}
		/*RTK_DEMOD_INFO("%x %x %x\n", ui8ARegTable[i32Iter],ui8ARegTable[i32Iter+1], ui8ARegTable[i32Iter+2]); */
	}

	return ui8Status;
}

/****************************************************************************
Function:   LeftTable
Description:
use for RTK_A_DVBSX_Log10ConvertdB
******************************************************************************/

static int LeftTable(int i32InValue)
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

/**********************Fixed version of 100 * log10(i32InValue), internally used function ************************/
unsigned int RTK_A_DVBSX_Log10ConvertdBx10(unsigned int i32InValue)
{
	unsigned char ui8Index = 0;
	unsigned int ui32Tmp = 1;
	unsigned int ui32InValueLeft = 1;
	unsigned int ui32LogValue;
	unsigned int ui32Dvalue = 0;

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
	ui32LogValue = ui32LogValue * 10 + LeftTable(ui32InValueLeft);
	/*RTK_DEMOD_INFO(" i32InValue = %d  log ret = %d\n", i32InValue,  ui32LogValue);*/

	return ui32LogValue;
}

/**********************Fixed version of 10 * log10(i32InValue), internally used function ***********************/
unsigned int RTK_A_DVBSX_Log10ConvertdB(unsigned int i32InValue)
{
	unsigned char ui8Index = 0;
	unsigned int ui32Tmp = 1;
	unsigned int ui32InValueLeft = 1;
	unsigned int ui32LogValue = 0;
	unsigned int ui32Dvalue = 0;

	if (i32InValue == 0) {
		return 0;
	}

	do {
		ui32Tmp = ui32Tmp << 1;
		if (i32InValue < ui32Tmp) {
			break;
		}
	} while (++ui8Index < 31);//} while (++ui8Index < 32); for coverity issue
	ui32LogValue = 301 * ui8Index / 100;

	ui32Dvalue = (1 << ui8Index);
	if (ui32Dvalue != 0) {
		ui32InValueLeft = i32InValue * 100 / ui32Dvalue;
	}
	ui32LogValue = ui32LogValue + LeftTable(ui32InValueLeft) / 10;
	/*RTK_DEMOD_INFO(" i32InValue = %d  log ret = %d\n", i32InValue,  ui32LogValue);*/

	return ui32LogValue;
}

/********************* Fixed version of pow(x,y), y should greater than 0, internally used function **************/
unsigned int  RTK_A_DVBSX_FixPow(unsigned int x, unsigned int y)
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

/********************* If 32bit add calculation overflow ,use this function, internally used function **************/
void RTK_A_DVBSX_64Add(RtkDemod64Data *pstSum, unsigned int ui32Addend)
{
	unsigned int ui32LowPart = 0;

	ui32LowPart = pstSum->ui32Low;
	pstSum->ui32Low += ui32Addend;
	if (ui32LowPart > pstSum->ui32Low) {
		pstSum->ui32High++;
	}
}

/********************** 2 32bit data multiply, use this function, internally used function *****************/
void RTK_A_DVBSX_64Mult(RtkDemod64Data *pstRst, unsigned int m1, unsigned int m2)
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

/**********************  internally used function *************************************************************/
unsigned char  RTK_A_DVBSX_64ComPare(RtkDemod64Data stPara, RtkDemod64Data stPara1)
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

/**********************64bit divide function, return 32bit value, internally used function *************************************/
unsigned int RTK_A_DVBSX_64Div(RtkDemod64Data stDivisor, RtkDemod64Data stDividend)
{
	unsigned char  ui8DivLeftShit = 0;
	unsigned int ui32Rst = 0;
	unsigned int ui32flag = 1;
	RtkDemod64Data stDataTmp = stDividend;

	if (stDivisor.ui32High == 0 && stDivisor.ui32Low == 0) {
		return 0;
	} else if (stDividend.ui32High == 0 && stDividend.ui32Low == 0) {
		return 0;
	} else if (stDividend.ui32High > stDivisor.ui32High) {
		return 0;
	} else if (!RTK_A_DVBSX_64ComPare(stDivisor, stDividend)) {
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

			if (!RTK_A_DVBSX_64ComPare(stDivisor, stDataTmp) || stDataTmp.ui32High >= 0x80000000) {
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
		ui32Rst += RTK_A_DVBSX_64Div(stDivisor, stDividend);
	}

	return ui32Rst;
}

/**********************64bit divide function,return 64bit value internally used function *******************************/
RtkDemod64Data RTK_A_DVBSX_64DivReturn64(RtkDemod64Data stDivisor, RtkDemod64Data stDividend)
{
	unsigned char  ui8DivLeftShit = 0;
	unsigned int ui32LowPart = 0;
	unsigned int ui32flag = 1;
	RtkDemod64Data i64Data, i64Data1;
	RtkDemod64Data stDataTmp = stDividend;

	i64Data.ui32High = 0;
	i64Data.ui32Low = 0;
	if (stDivisor.ui32High == 0 && stDivisor.ui32Low == 0) {
		return i64Data;
	} else if (stDividend.ui32High == 0 && stDividend.ui32Low == 0) {
		return i64Data;
	} else if (stDividend.ui32High > stDivisor.ui32High) {
		return i64Data;
	} else if (!RTK_A_DVBSX_64ComPare(stDivisor, stDividend)) {
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
			if (!RTK_A_DVBSX_64ComPare(stDivisor, stDataTmp) || stDataTmp.ui32High >= 0x80000000) {
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
		i64Data1 = RTK_A_DVBSX_64DivReturn64(stDivisor, stDividend);
		ui32LowPart = i64Data.ui32Low;
		i64Data.ui32Low += i64Data1.ui32Low;
		i64Data.ui32High += i64Data1.ui32High;
		if (ui32LowPart > i64Data.ui32Low) {
			i64Data.ui32High += 1;
		}

	}

	return i64Data;
}

/*Get one middle value in three input value*/
void  RTK_A_DVBSX_MiddleValueu8(unsigned char u8ValueA, unsigned char u8ValueB, unsigned char u8ValueC, unsigned char *MidValue)
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

/*****************Sr optimal function, internally used function **************************/
RTK_A_DVBSX_STATUS  RTK_A_DVBSX_SetSrOptimalTable(DVBSX_DEMOD_MODULE *pDemod, unsigned int u32SrHz, unsigned int *ui32RegTable, unsigned int i32TableLen)
{
	RTK_A_DVBSX_STATUS ui8Status = RTK_DEMOD_SUCCESSFUL;
	unsigned char ui8Iter = 0;

	for (ui8Iter = 0; ui8Iter < i32TableLen; ui8Iter += 2)
	{
		if ((ui32RegTable[ui8Iter]) == u32SrHz)
		{
			ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x2a, 0x54, 0x82);
			ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x2a, 0x55, 0x34);
			ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x2a, 0x56, 0xff);
			ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x2a, 0x57, 0xd1);
			//RTK_DEMOD_INFO("%u %u %u\n", u32SrHz,ui32RegTable[ui8Iter], ui32RegTable[ui8Iter+1]);
		}
	}

	return ui8Status;
}

/****************************************************************************
Function:   RTK_A_DVBSX_LatchOn
Parameters: none
Return:    Read status:RTK_DEMOD_SUCCESSFUL is normal and other is abnormal.
Description:
latch on the register for reading,make sure the register group during reading period not change
******************************************************************************/
RTK_A_DVBSX_STATUS  RTK_A_DVBSX_LatchOn(DVBSX_DEMOD_MODULE *pDemod)
{
	return RTK_A_DVBSX_Write(pDemod, 0x01, 0x0d, 1);
}

/****************************************************************************
Function:   RTK_A_DVBSX_LatchOff
Parameters: none
Return:    Read status:RTK_DEMOD_SUCCESSFUL is normal and other is abnormal.
Description:
latch off the register for reading
******************************************************************************/
RTK_A_DVBSX_STATUS  RTK_A_DVBSX_LatchOff(DVBSX_DEMOD_MODULE *pDemod)
{
	return RTK_A_DVBSX_Write(pDemod, 0x01, 0x0d, 0);
}

/****************************************************************************
Function:    RTK_A_DVBSX_HoldDSP
Parameters:  none
Return:      I2C Write Status   0 is Success,Other is Failed

Description:
This function should be called before set tuner  frequency.
****************************************************************************/
RTK_A_DVBSX_STATUS  RTK_A_DVBSX_HoldDSP(DVBSX_DEMOD_MODULE *pDemod)
{
	return RTK_A_DVBSX_Write(pDemod, 0x00, 0x05, 0x01);
}

/****************************************************************************
Function:    RTK_A_DVBSX_StartDSP
Parameters:  none
Return:      I2C Write Status   0 is Success,Other is Failed

Description:
This function should be called after set tuner  frequency.
Attention: RTK_A_DVBSX_HoldDSP and RTK_A_DVBSX_StartDSP should be used together.
For example:
RTK_A_DVBSX_HoldDSP();
SetTunerFrequency(Frequency_KHz);
RTK_A_DVBSX_StartDSP();
****************************************************************************/
RTK_A_DVBSX_STATUS  RTK_A_DVBSX_StartDSP(DVBSX_DEMOD_MODULE *pDemod)
{
	RTK_A_DVBSX_STATUS status = RTK_DEMOD_SUCCESSFUL;
	unsigned char ui8SxMode = 0;

	status = RTK_A_DVBSX_Read(pDemod, 0x00, 0x2b, &ui8SxMode);
	status |= RTK_A_DVBSX_Write(pDemod, 0x00, 0x05, 0x00);
	if (ui8SxMode & 0x01) { /*S/S2 mode*/

		status |= RTK_A_DVBSX_Write(pDemod, 0x18, 0x11, 0x04); /*BCH counter window, default value*/
		status |= RTK_A_DVBSX_Write(pDemod, 0x18, 0x10, 0x00); /*BCH counter window, default value*/
		status |= RTK_A_DVBSX_Write(pDemod, 0x17, 0xc3, DVBS2_BCH_L); /*BCH counter window*/
		status |= RTK_A_DVBSX_Write(pDemod, 0x17, 0xc4, DVBS2_BCH_H); /*BCH counter window*/

	}

	status |= RTK_A_DVBSX_Write(pDemod, 0x00, 0x05, 0x01);
	status |= RTK_A_DVBSX_Write(pDemod, 0x00, 0x05, 0x00);
	status |= RTK_A_DVBSX_Write(pDemod, 0x00, 0x05, 0x01);
	status |= RTK_A_DVBSX_Write(pDemod, 0x00, 0x05, 0x00);

	return status;
}

/****************************************************************************
Function:    RTK_A_DVBSX_ChipID
Parameters:  ui8ChipID:chip id will return by this pointer
Return:      I2C read Status   0 is Success,Other is Failed
Description:
Get the demodulator's chip id
****************************************************************************/
RTK_A_DVBSX_STATUS  RTK_A_DVBSX_ChipID(DVBSX_DEMOD_MODULE *pDemod, unsigned char *ui8ChipID)
{
	return RTK_A_DVBSX_Read(pDemod, 0x00, 0x00, ui8ChipID);
}

RTK_A_DVBSX_STATUS RTK_A_DVBSX_DisableTSOutput(DVBSX_DEMOD_MODULE *pDemod)
{
	RTK_A_DVBSX_STATUS status = RTK_DEMOD_SUCCESSFUL;

	status |= RTK_A_DVBSX_Write(pDemod, 0x26, 0x25, 0x00);
	status |= RTK_A_DVBSX_Write(pDemod, 0x26, 0x26, 0x00); /*Disable TS output*/

	return status;
}

RTK_A_DVBSX_STATUS RTK_A_DVBSX_EnableTSOutput(DVBSX_DEMOD_MODULE *pDemod)
{
	RTK_A_DVBSX_STATUS status = RTK_DEMOD_SUCCESSFUL;

	status |= RTK_A_DVBSX_Write(pDemod, 0x26, 0x25, 0xff);
	status |= RTK_A_DVBSX_Write(pDemod, 0x26, 0x26, 0x07); /*Enable TS output*/

	return status;
}

RTK_A_DVBSX_STATUS RTK_A_DVBSX_SetTSOutputOnOff(DVBSX_DEMOD_MODULE *pDemod, unsigned char ui8TSOnOff)
{
	RTK_A_DVBSX_STATUS status = RTK_DEMOD_SUCCESSFUL;

	if (ui8TSOnOff == RTK_DEMOD_TS_OFF) {
		status |= RTK_A_DVBSX_DisableTSOutput(pDemod);
	} else {
		status |= RTK_A_DVBSX_EnableTSOutput(pDemod);
	}

	return status;
}

RTK_A_DVBSX_STATUS RTK_A_DVBSX_GetTSOutputOnOff(DVBSX_DEMOD_MODULE *pDemod, unsigned char *ui8TSOnOff)
{
	RTK_A_DVBSX_STATUS status = RTK_DEMOD_SUCCESSFUL;
	unsigned char ui8TSModeFlag1;
	unsigned char ui8TSModeFlag2;

	status |= RTK_A_DVBSX_Read(pDemod, 0x26, 0x25, &ui8TSModeFlag1);
	status |= RTK_A_DVBSX_Read(pDemod, 0x26, 0x26, &ui8TSModeFlag2);
	if ((ui8TSModeFlag1 & 0xff) && (ui8TSModeFlag2 & 0x07)) { //TS On
		*ui8TSOnOff = RTK_DEMOD_TS_ON;
	} else {
		*ui8TSOnOff = RTK_DEMOD_TS_OFF;
	}

	return status;
}

/**
Function:    RTK_A_DVBSX_GetTSModeFlag
Parameters:  ui8TSMode:TS Mode(Serial/Parallel) return by this pointer
             0¡GSERIAL_MODE¡A 1¡GPARALLEL_MODE
Return:      I2C read Status   0 is Success,Other is Failed
Description:
get TS mode,  RTK_DEMOD_TS_SERIAL_MODE/RTK_DEMOD_TS_PARALLEL_MODE
**/
RTK_A_DVBSX_STATUS RTK_A_DVBSX_GetTSModeFlag(DVBSX_DEMOD_MODULE *pDemod, unsigned char *ui8TSModeFlag)
{
	RTK_A_DVBSX_STATUS status = RTK_DEMOD_SUCCESSFUL;
	unsigned char ui8TSMode;

	status |= RTK_A_DVBSX_Read(pDemod, 0x18, 0x01, &ui8TSMode);

	if (ui8TSMode & 0x01) { // Serial Mode
		*ui8TSModeFlag = RTK_DEMOD_TS_SERIAL_MODE;
	} else { // Parallel Mode
		*ui8TSModeFlag = RTK_DEMOD_TS_PARALLEL_MODE;
	}

	return status;
}

/**
Function:    RTK_A_DVBSX_SetTSModeFlag
Parameters:  ui8TSMode:TS Mode(Serial/Parallel)
             0¡GSERIAL_MODE¡A 1¡GPARALLEL_MODE
Return:      I2C read Status   0 is Success,Other is Failed
Description:
Set TS output mode,  RTK_DEMOD_TS_SERIAL_MODE/RTK_DEMOD_TS_PARALLEL_MODE
**/
RTK_A_DVBSX_STATUS RTK_A_DVBSX_SetTSModeFlag(DVBSX_DEMOD_MODULE *pDemod, unsigned char ui8TSModeFlag)
{
	RTK_A_DVBSX_STATUS status = RTK_DEMOD_SUCCESSFUL;
	unsigned char ui8SerialEnabled = 0x0; /*parallel clock out */

	if (ui8TSModeFlag == RTK_DEMOD_TS_SERIAL_MODE) {
		ui8SerialEnabled = 0x01;    /*enable serial TS mode */
	}

	/*0:disable serial mode; 1: enable serial mode */
	status |= RTK_A_DVBSX_Write(pDemod, 0x18, 0x01, ui8SerialEnabled);

	return status;
}

RTK_A_DVBSX_STATUS RTK_A_DVBSX_SetTsInterfaceMode(DVBSX_DEMOD_MODULE *pDemod, RTK_DEMOD_MPEGTSMode stTSMode)
{
	int ret = TUNER_CTRL_OK;

	gstRtkDemodDvbConfig.stMPEGTSMode.ui8TSTransferType = stTSMode.ui8TSTransferType;
	gstRtkDemodDvbConfig.stMPEGTSMode.ui8OutputEdge = stTSMode.ui8OutputEdge;
	gstRtkDemodDvbConfig.stMPEGTSMode.ui8TSSPIMSBSelection= stTSMode.ui8TSSPIMSBSelection;
	gstRtkDemodDvbConfig.stMPEGTSMode.ui8TSSSIOutputSelection = stTSMode.ui8TSSSIOutputSelection;
	gstRtkDemodDvbConfig.stMPEGTSMode.ui8SPIClockConstantOutput = stTSMode.ui8SPIClockConstantOutput;
	gstRtkDemodDvbConfig.stMPEGTSMode.ui8DVBSxTSClockVariableMode = stTSMode.ui8DVBSxTSClockVariableMode;

	return ret;
}

RTK_A_DVBSX_STATUS RTK_A_DVBSX_SetTSMode(DVBSX_DEMOD_MODULE *pDemod, RTK_DEMOD_MPEGTSMode stTSMode)
{
	unsigned char ui8SerialEnabled = 0x0; /*parallel clock out */
	unsigned char ui8TSOutputEdegeMode = stTSMode.ui8OutputEdge;
	unsigned char ui8TsClockMode = stTSMode.ui8SPIClockConstantOutput;
	unsigned char ui8TsReg = 0;
	RTK_A_DVBSX_STATUS status = RTK_DEMOD_SUCCESSFUL;

	/*0x00: SPI data align with rising edge of TS CLK; 0x01: SPI data align with falling edge of TS CLK*/
	status |= RTK_A_DVBSX_Write(pDemod, 0x18, 0x02, ui8TSOutputEdegeMode);
	status |= RTK_A_DVBSX_Write(pDemod, 0x18, 0x03, ui8TsClockMode); /*either free running or only active when data are valid*/

	status |= RTK_A_DVBSX_Write(pDemod, 0x26, 0x24, 0x05);
	status |= RTK_A_DVBSX_Write(pDemod, 0x26, 0x25, 0xff);//set TS output on
	status |= RTK_A_DVBSX_Write(pDemod, 0x26, 0x26, 0x7); //set TS output on

	if (stTSMode.ui8TSTransferType == RTK_DEMOD_TS_SERIAL_MODE) {
		ui8SerialEnabled = 0x01;    /*enable serial TS mode */

		/*SSI_sync_hold_cycle = 0x00; //0x00:1bit sync; 0x01: 8bit sync*/
		/*0: serial sync  holds 1 cycle; 1: serial sync holds 8 cycles*/
		/*RTK_A_DVBSX_Write(pDemod, 0x17,0xb0, SSI_sync_hold_cycle);  */

		/*0:disable serial mode; 1: enable serial mode */
		status |= RTK_A_DVBSX_Write(pDemod, 0x18, 0x01, ui8SerialEnabled);
		if (stTSMode.ui8TSSSIOutputSelection == RTK_DEMOD_TS_SSI_OUTPUT_ON_DATA_BIT0) {
			status |= RTK_A_DVBSX_Write(pDemod, 0x18, 0x04, 0x00);
		} else {
			status |= RTK_A_DVBSX_Write(pDemod, 0x18, 0x04, 0x01);
		}

		status |= RTK_A_DVBSX_Write(pDemod, 0x26, 0x27, 0x00); /*TS delay clock*/
		status |= RTK_A_DVBSX_Write(pDemod, 0x26, 0x29, 0x03); /*Disable TS delay*/
	} else {
		ui8SerialEnabled = 0x0; /*parallel clock output */
		/*0: disable serial mode; 1:enable serial mode */
		status |= RTK_A_DVBSX_Write(pDemod, 0x18, 0x01, ui8SerialEnabled);
		if (stTSMode.ui8TSSPIMSBSelection == RTK_DEMOD_TS_SPI_MSB_ON_DATA_BIT7) {
			status |= RTK_A_DVBSX_Write(pDemod, 0x18, 0x04, 0x00);
		} else {
			status |= RTK_A_DVBSX_Write(pDemod, 0x18, 0x04, 0x01);
		}

		status |= RTK_A_DVBSX_Write(pDemod, 0x26, 0x27, 0x00);
		status |= RTK_A_DVBSX_Write(pDemod, 0x26, 0x29, 0x03);
	}

	if(stTSMode.ui8DVBSxTSClockVariableMode == RTK_A_DEMOD_TS_CLOCK_VARIABLE)
	{
		status |= RTK_A_DVBSX_Write(pDemod, 0x00, 0x3c, 0x70); //VCM adaptive TS 2017.1.23, fixme. IF TP set ts, not required
		//u8Status |= RTK_A_DVBSX_Read(pDemod, 0x06, 0x06, &ui8TsReg);
		ui8TsReg = 0x28;
		ui8TsReg &= 0x7f; //ts_n_fb
		status |= RTK_A_DVBSX_Write(pDemod, 0x18, 0x9e, ui8TsReg);
		RTK_A_DVBSX_SetTSVarThresh(pDemod, ui8TsReg);
		//u8Status |= RTK_A_DVBSX_Read(pDemod, 0x06, 0x0a, &ui8TsReg);
		ui8TsReg = 0x06;
		ui8TsReg &= 0x1f; //ts_m_fb, divider
		status |= RTK_A_DVBSX_Write(pDemod, 0x18, 0x9f, ui8TsReg);
	}
	else
	{
		status |= RTK_A_DVBSX_Write(pDemod, 0x00, 0x3c, 0x00);
	}

	return status;
}

/****************************************************************************
Function:    RTK_A_DVBSX_TSLockFlag()
Parameters:
LockStatus: pointer to lock status
Return:      status
Description: Get TS lock status.
****************************************************************************/
RTK_A_DVBSX_STATUS RTK_A_DVBSX_TSLockFlag(DVBSX_DEMOD_MODULE *pDemod, unsigned char *LockStatus)
{
	/*
	Lock Flag indicate current lock status
	1, Must be locked, TS output or FEC locked in stable state.
	2, For occasional TS error, should give lock flag.
	*/
	RTK_A_DVBSX_STATUS status = RTK_DEMOD_SUCCESSFUL;
	unsigned char u8TSLockFlag = 0;

	*LockStatus = 0;

	status |= RTK_A_DVBSX_Read(pDemod, 0x18, 0x2A, &u8TSLockFlag);
	if ((u8TSLockFlag & 0x01) && (status == RTK_DEMOD_SUCCESSFUL)) {
		*LockStatus = 1;
	}

	return status;
}

/****************************************************************************
Function:    RTK_A_DVBSX_FECLockFlag()
Parameters:
LockStatus: pointer to FEC lock status
Return:      status
Description:
Get FEC lock status. can be used for DVB-T/T2, DVB-C, DVBS/S2 FEC
lock flag check.
****************************************************************************/
RTK_A_DVBSX_STATUS RTK_A_DVBSX_FECLockFlag(DVBSX_DEMOD_MODULE *pDemod, unsigned char *LockStatus)
{
	RTK_A_DVBSX_STATUS status = RTK_DEMOD_SUCCESSFUL;
	unsigned char u8DvbcMode = 0, u8DVBSxMode = 0;
	unsigned char u8SS2Mode = 0;
	unsigned char u8BlockFEC = 0;

	status |= RTK_A_DVBSX_Read(pDemod, 0x00, 0x13, &u8DvbcMode);
	status |= RTK_A_DVBSX_Read(pDemod, 0x00, 0x2b, &u8DVBSxMode);

	*LockStatus = 0;

	if (u8DVBSxMode & 0x01) {
		status |= RTK_A_DVBSX_DVBSxModeDetected(pDemod, &u8SS2Mode);
		if (u8SS2Mode == RTK_DEMOD_DVBS_MODE) {
			status |= RTK_A_DVBSX_Read(pDemod, 0x95, 0x37, &u8BlockFEC);
			if ((u8BlockFEC & 0x01)) {
				*LockStatus = 1;
				return status;           //DVB-S2 FEC locked
			}
		}
		if (u8SS2Mode == RTK_DEMOD_DVBS2_MODE) {
			status |= RTK_A_DVBSX_Read(pDemod, 0x17, 0xEC, &u8BlockFEC);
			if ((u8BlockFEC & 0x01)) {
				*LockStatus = 1;
				return status;           //DVB-S2 FEC locked
			}
		}
	}

	return status;
}

/****************************************************************************
Function:    RTK_A_DVBSX_TimingLockFlag()
Parameters:
LockStatus: pointer to timing lock status
Return:      status
Description:
Get FEC lock status. can be used for DVBS/S2 Timing lock flag check.
****************************************************************************/
RTK_A_DVBSX_STATUS RTK_A_DVBSX_TimingLockFlag(DVBSX_DEMOD_MODULE *pDemod, unsigned char *LockStatus)
{
	RTK_A_DVBSX_STATUS status = RTK_DEMOD_SUCCESSFUL;
	unsigned char u8TimingLockFlag = 0;

	*LockStatus = 0;

	//Check Timing lock status
	status |= RTK_A_DVBSX_Read(pDemod, 0x2b, 0x0d, &u8TimingLockFlag);
	if(((u8TimingLockFlag&0x03) == 0x02) && (status == RTK_DEMOD_SUCCESSFUL))
	{
		*LockStatus = 1;
	}

	return status;
}

/****************************************************************************
Function:   RTK_A_DVBSX_GetPER
Parameters:   ui32PER: return PER by this pointer for all modes (DVB-T/T2/C/S/S2).
Return:       Operate  Status:RTK_DEMOD_SUCCESSFUL is Normal and other is abnormal
Description:
           Call this function to get PER value ratio, can be used to evaluate TS
           output error rate.
           multiply scalar value(10E9) to avoid Soc Fixed-point of float type
Error code:
           ui32PER: 10E9: statistic window parameter setting error, please check
           register 0x188f.
******************************************************************************/
RTK_A_DVBSX_STATUS  RTK_A_DVBSX_GetPER(DVBSX_DEMOD_MODULE *pDemod, unsigned int *ui32PER)
{
	unsigned char ui8Tmp = 0;
	RTK_A_DVBSX_STATUS ui8Status = RTK_DEMOD_SUCCESSFUL;
	unsigned int ui32Tmp = 0, ui32WinLen = 0;
	RtkDemod64Data i64Data, i64Data1;

	ui8Status = RTK_A_DVBSX_LatchOn(pDemod);
	ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x18, 0x92, &ui8Tmp);
	ui32WinLen = (ui8Tmp & 0x0f) << 16;

	ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x18, 0x91, &ui8Tmp);
	ui32WinLen = ui32WinLen + (ui8Tmp << 8);

	ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x18, 0x90, &ui8Tmp);
	ui32WinLen = ui32WinLen + ui8Tmp;

	ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x18, 0x95, &ui8Tmp);
	ui32Tmp = (ui8Tmp & 0x0f) << 16;

	ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x18, 0x94, &ui8Tmp);
	ui32Tmp = ui32Tmp + (ui8Tmp << 8);

	ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x18, 0x93, &ui8Tmp);
	ui32Tmp = ui32Tmp + ui8Tmp;

	ui8Status |= RTK_A_DVBSX_LatchOff(pDemod);

	if (ui32WinLen != 0) {
		RTK_A_DVBSX_64Mult(&i64Data, ui32Tmp, 1000000000);
		RTK_A_DVBSX_64Mult(&i64Data1, ui32WinLen, 1);
		*ui32PER = RTK_A_DVBSX_64Div(i64Data, i64Data1);
	} else {
		*ui32PER  = 1000000000;
	}

	return ui8Status;
}

RTK_A_DVBSX_STATUS  RTK_A_DVBSX_GetPEC(DVBSX_DEMOD_MODULE *pDemod, unsigned int *ui32PEC_NUM, unsigned int *ui32PEC_DEN)
{
	unsigned char ui8Tmp = 0;
	RTK_A_DVBSX_STATUS ui8Status = RTK_DEMOD_SUCCESSFUL;
	unsigned int ui32Tmp = 0, ui32WinLen = 0;

	ui8Status = RTK_A_DVBSX_LatchOn(pDemod);
	ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x18, 0x92, &ui8Tmp);
	ui32WinLen = (ui8Tmp & 0x0f) << 16;

	ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x18, 0x91, &ui8Tmp);
	ui32WinLen = ui32WinLen + (ui8Tmp << 8);

	ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x18, 0x90, &ui8Tmp);
	ui32WinLen = ui32WinLen + ui8Tmp;

	ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x18, 0x95, &ui8Tmp);
	ui32Tmp = (ui8Tmp & 0x0f) << 16;

	ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x18, 0x94, &ui8Tmp);
	ui32Tmp = ui32Tmp + (ui8Tmp << 8);

	ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x18, 0x93, &ui8Tmp);
	ui32Tmp = ui32Tmp + ui8Tmp;

	ui8Status |= RTK_A_DVBSX_LatchOff(pDemod);

	if (ui32WinLen != 0) {
		*ui32PEC_NUM = ui32Tmp;
		*ui32PEC_DEN = ui32WinLen;
	} else {
		*ui32PEC_NUM = -1;
		*ui32PEC_DEN = -1;
	}

	return ui8Status;
}

/****************************************************************************
Function:   RTK_A_DVBSX_GetPERNum
Parameters:   ui32S2PERNum: return PER number by this pointer for all modes (DVB-T/T2/C/S/S2).
Return:       Operate  Status:RTK_DEMOD_SUCCESSFUL is Normal and other is abnormal
Description:
           Call this function to get PER value, can be used to evaluate TS output
           error.
******************************************************************************/
RTK_A_DVBSX_STATUS  RTK_A_DVBSX_GetPERNum(DVBSX_DEMOD_MODULE *pDemod, unsigned int *ui32PERNum)
{
	unsigned char ui8Tmp = 0;
	RTK_A_DVBSX_STATUS ui8Status = RTK_DEMOD_SUCCESSFUL;
	unsigned int ui32Tmp = 0;

	ui8Status = RTK_A_DVBSX_LatchOn(pDemod);

	ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x18, 0x95, &ui8Tmp);
	ui32Tmp = (ui8Tmp & 0x0f) << 16;

	ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x18, 0x94, &ui8Tmp);
	ui32Tmp = ui32Tmp + (ui8Tmp << 8);

	ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x18, 0x93, &ui8Tmp);
	ui32Tmp = ui32Tmp + ui8Tmp;
	ui8Status |= RTK_A_DVBSX_LatchOff(pDemod);

	*ui32PERNum = ui32Tmp;

	return ui8Status;
}

/********************************************************************************
function    : RTK_A_DVBSX_SignalIFAGC
parameter   :ui16CalcValue:if AGC return by this pointer
return      :Operate  Status:RTK_DEMOD_SUCCESSFUL is Normal and other is abnormal.
Description:Call this function to get IF AGC value, value range: 0-1024
             IF AGC voltage = (IFAGCvalue *3.3)/1024
*********************************************************************************/
RTK_A_DVBSX_STATUS   RTK_A_DVBSX_SignalIFAGC(DVBSX_DEMOD_MODULE *pDemod, unsigned short *ui16CalcValue)
{
	RTK_A_DVBSX_STATUS ui8Status = RTK_DEMOD_SUCCESSFUL;
	unsigned char ui8Pwm0 = 0, ui8Pwm1 = 0, u8AGCPolarity;
	unsigned short u16AGCValue = 0;

	ui8Status |= RTK_A_DVBSX_LatchOn(pDemod);
	ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x00, 0x20, &u8AGCPolarity);
	ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x10, 0x28, &ui8Pwm0);
	ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x10, 0x29, &ui8Pwm1);
	ui8Status |= RTK_A_DVBSX_LatchOff(pDemod);
	if (ui8Status) {
		*ui16CalcValue = 0;
		return ui8Status;
	}
	u16AGCValue = ((ui8Pwm1 & 0x03) << 8) | ui8Pwm0;
	if ((u8AGCPolarity & 0x02) == 0x02) { //AGV inverted
		*ui16CalcValue = 1024 - u16AGCValue;
	} else {
		*ui16CalcValue  = u16AGCValue;
	}

	return ui8Status;
}

/*Get counted TS bit rate*/
RTK_A_DVBSX_STATUS   RTK_A_DVBSX_TSBitAccu(DVBSX_DEMOD_MODULE *pDemod, unsigned int *ui32TSBitRateK)
{
	RTK_A_DVBSX_STATUS ui8Status = RTK_DEMOD_SUCCESSFUL;
	unsigned char ui8PData[4] = {0};
	unsigned int u32Data = 0;
	unsigned char ui8TSwin = 0;

	/*
	18H     8CH     ts_cal_res[7:0] [7:0]   Read Only Latch 8'b0000_0000
	18H     8DH     ts_cal_res[15:8]        [7:0]   Read Only Latch 8'b0000_0000
	18H     8EH     ts_cal_res[19:16]       [3:0]   Read Only Latch 4'b0000
	18H     8FH     per_cal_win     [7:0]   Read Write      8'b0111_0010    1s as
	*/
	ui8Status = RTK_A_DVBSX_LatchOn(pDemod);
	ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x18, 0x8c, &ui8PData[0]);
	ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x18, 0x8d, &ui8PData[1]);
	ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x18, 0x8e, &ui8PData[2]);
	ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x18, 0x8b, &ui8TSwin);
	ui8Status |= RTK_A_DVBSX_LatchOff(pDemod);
	u32Data = ((ui8PData[2] & 0x0f) << 16) + (ui8PData[1] << 8) + ui8PData[0];
	*ui32TSBitRateK = (u32Data*188*8*115)/(1000*(1+ui8TSwin));
	
	return ui8Status;
}

RTK_A_DVBSX_STATUS   RTK_A_DVBSX_GetDVBPktErrorCounter(DVBSX_DEMOD_MODULE *pDemod, unsigned short *uiPktErrCounter)
{
	RTK_A_DVBSX_STATUS ui8Status = RTK_DEMOD_SUCCESSFUL;
	unsigned short ui16Tmp = 0;
	unsigned char ui8Tmp = 0;

	*uiPktErrCounter = 0;

	ui8Status = RTK_A_DVBSX_LatchOn(pDemod);
	ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x18, 0x9b, &ui8Tmp);
	ui16Tmp = ui8Tmp << 8;
	ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x18, 0x9a, &ui8Tmp);
	ui16Tmp = ui16Tmp + ui8Tmp;
	ui8Status |= RTK_A_DVBSX_LatchOff(pDemod);
	*uiPktErrCounter = ui16Tmp;

	return ui8Status;
}

/****************************************************************************
Function:    RTK_A_DVBSX_PowerOnInit(void)
Parameters:  none
Return:      status
Description: This function is used to initialize default configuration.
****************************************************************************/
RTK_A_DVBSX_STATUS RTK_A_DVBSX_PowerOnInit(DVBSX_DEMOD_MODULE *pDemod)
{
	RTK_A_DVBSX_STATUS status = RTK_DEMOD_SUCCESSFUL;

	/*Demodulator DVB-S/S2 setting*/
	/*
	Please refer to the following steps to make sure IQ configuration is right.
	1,  Set gstRtkDemodDvbConfig.DvbSxTunerInput.ui8IQSwapMode = RTK_DEMOD_INPUT_IQ_NORMAL, //make sure demodulator works.
	2,  Generate Normal spectrum DVB-S signal,when ui8IQSwapMode set correctly, the test result should be 0x29ff bit0 = 1, other bits be zero
	*/
	#ifdef CONFIG_CUSTOMER_TV006
	gstRtkDemodDvbConfig.DvbSxTunerInput.ui8IQSwapMode      = RTK_DEMOD_INPUT_IQ_SWAP;
	REALTEK_A_DVBSX_INFO("RTK_DEMOD_INPUT_IQ_SWAP\n");
	#else
	gstRtkDemodDvbConfig.DvbSxTunerInput.ui8IQSwapMode      = RTK_DEMOD_INPUT_IQ_NORMAL;
	REALTEK_A_DVBSX_INFO("RTK_DEMOD_INPUT_IQ_NORMAL\n");
	#endif
	gstRtkDemodDvbConfig.DvbSxTunerInput.ui8SxIFAgcPolarity = RTK_DEMOD_IF_AGC_POLARITY_INVERTED;
	//gstRtkDemodDvbConfig.DvbSxTunerInput.ui8IQAutoDetection = RTK_DEMOD_IQ_AUTO_ENABLE;
	gstRtkDemodDvbConfig.DvbSxTunerInput.ui32SymbolRateHz = 27000000; /*Actually not used, demodulator auto detection.*/

	/*Demodulator Clock Setting           */
	gstRtkDemodDvbConfig.ui32DemodClkKHz = 24000; /*KHz, 24MHz, or 30400 for 30.4MHz*/

	//No Setting DVBSx Clock Here
	/* RTK_A_DVBSX_SetSysClkClock();*/
	//status |= RTK_A_DVBSX_SetSysClkClockDVBSx(pDemod);
	//status |= RTK_A_DVBSX_Write(pDemod, 0x00, 0x04, 0x00); /*set register to default value,  RTK_A_DVBSX_PowerOnInit should be called once after power up */

	gRtkDemodPowerOnInited = 1;

	return status;
}

/****************************************************************************
Function:    RTK_A_DVBSX_StandBy(void)
Parameters:  none
Return:      status
Description: Set chip in standby mode. I2C still works.
****************************************************************************/
RTK_A_DVBSX_STATUS RTK_A_DVBSX_StandBy(DVBSX_DEMOD_MODULE *pDemod)
{
	RTK_A_DVBSX_STATUS ui8Status = RTK_DEMOD_SUCCESSFUL;
	#if 0
	unsigned char ui8ChipID = 0;
	unsigned char u8SAdc = 0, u8TAdc = 0;

	ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x00, 0x00, &ui8ChipID);
	//fixme GPIO setting??
	//SetGpioPinsHiZ();
	
	ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x00, 0x05, 0x01); /*DSP Reset down, TS FEC will not be locked.*/
	ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x01, 0x0C, 0x00); /*I2C on crystal*/

	ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x26, 0x25, 0x00); /*TS data OE*/
	ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x26, 0x26, 0x00); /*TS sync OE*/
	ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x00, 0x19, 0x00); /*DSP clock close*/
	//fixme, the above is digial registers, recommend to set.
	
	if (ui8ChipID == 0xa4) {
		ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x06, 0x1c, &u8SAdc);
	}
	ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x06, 0x00, &u8TAdc);
	//ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x06, 0x05,&u8RssiAdc);
	//ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x06, 0x08,&u8PLL);

	u8SAdc |= 0x01;    /*Set bit0 = 1*/
	u8TAdc |= 0x02;    /*Set bit1 = 1*/
	//u8RssiAdc |= 0x02; /*Set bit1 = 1*/
	//u8PLL|= 0x01;      /*Set bit0 = 1*/
	if (ui8ChipID == 0xa4) {
		ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x06, 0x1c, u8SAdc);
	}
	ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x06, 0x00, u8TAdc);
	//ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x06, 0x05,u8RssiAdc);
	//ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x06, 0x08,u8PLL);
	#endif
	return ui8Status;
}

/********************************************************************************
function    : RTK_A_DVBSX_StandByWakeUp(void)
parameter   :none
return      :Operate  Status:RTK_DEMOD_SUCCESSFUL is Normal and other is abnormal.
Description:
*********************************************************************************/
RTK_A_DVBSX_STATUS  RTK_A_DVBSX_StandByWakeUp(DVBSX_DEMOD_MODULE *pDemod)
{
	RTK_A_DVBSX_STATUS ui8Status = RTK_DEMOD_SUCCESSFUL;
	#if 0
	unsigned char ui8ChipID = 0, ui8Reg0630 = 0, ui8CfgDone = 0;
	unsigned char u8SAdc = 0, u8TAdc = 0, u8dspclk;

	ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x00, 0x19, &u8dspclk);
	if (u8dspclk == 1) { //not in standby mode. no need to wakeup anymore.
		return ui8Status;
	}

	/*unsigned char u8RssiAdc;*/
	ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x00, 0x00, &ui8ChipID);
	if (ui8ChipID == 0xa4) {
		ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x06, 0x1c, &u8SAdc);
	}
	ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x06, 0x00, &u8TAdc);
	/*ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x06, 0x05,&u8RssiAdc);*/
	//ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x06, 0x08,&u8PLL);

	if ((gstRtkDemodDvbConfig.ui8ConfigDVBType == RTK_DEMOD_DVBS_MODE) || (gstRtkDemodDvbConfig.ui8ConfigDVBType == RTK_DEMOD_DVBS2_MODE)) {
		if (ui8ChipID == 0xa4) {
			u8SAdc &= 0xFE;     /*Set bit0 = 0 Open S S2 ADC */
		}
	}
	u8TAdc &= 0xFD;       /*Set bit1 = 0 Open T/T2 C S/S2 ADC Switch*/
	/*u8RssiAdc &= 0xFD;  //Set bit1 = 0, Currently RSSI ADC is not used.*/
	//u8PLL &= 0xFE;     /*Set bit0 = 0*/
	//ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x06, 0x08,u8PLL);
	/*
	if(ui8ChipID == 0xa8)
	{
		ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x06, 0x21, 0x7b);
		ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x06, 0x21, 0x73);
	}
	if(ui8ChipID == 0xa4)
	{
		ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x06, 0x1c,u8SAdc);
	}*/

	ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x06, 0x00, u8TAdc);
	
	//fixme, the following is digital setting.
	ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x00, 0x19, 0x01); /*Enable DSP clock*/

	ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x26, 0x25, 0xFF); /*TS data OE, Enable TS output*/
	ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x26, 0x26, 0x07); /*TS sync OE, Enable TS output*/
	
	//Set GPIO Functions for AGC Out Pin
	RTK_A_DVBSX_Write(pDemod, 0x0, 0x23, 0x00);
	RTK_A_DVBSX_Write(pDemod, 0x0, 0x21, 0x03);
	RTK_A_DVBSX_Write(pDemod, 0x0, 0x22, 0x00);
	RTK_A_DVBSX_Write(pDemod, 0x0, 0x37, 0x00);

	ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x00, 0x05, 0x00); /*DSP Reset up.*/

	//fixme, the following ADC check is not needed.
	tv_osal_msleep(1); /*Delay 1 millisecond.*/
	/*ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x06, 0x05,u8RssiAdc);*/
	ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x06, 0x30, &ui8Reg0630);
	ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x00, 0x04, &ui8CfgDone);
	if (((ui8Reg0630 & 0x01) != 0x01) && ((ui8CfgDone & 0x01) == 0x01)) { /*Check PLL lock status.*/
		tv_osal_msleep(10); /*Delay 1 millisecond.*/
		ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x06, 0x30, &ui8Reg0630);
	}
	if (((ui8Reg0630 & 0x01) == 0x01) && ((ui8CfgDone & 0x01) == 0x01)) { /*Check PLL lock status.*/
		//ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x01, 0x0C, 0x01); /*I2C on PLL*/
	}
	#endif
	return ui8Status;
}

/****************************************************************************
Function:    RTK_A_DVBSX_SusPend()
Parameters:  none
Return:      status
Description: Set chip in suspend mode.I2C will not work, Hardware reset or Power reset is required to active the demodulator.
****************************************************************************/
RTK_A_DVBSX_STATUS RTK_A_DVBSX_Suspend(DVBSX_DEMOD_MODULE *pDemod)
{
	RTK_A_DVBSX_STATUS status = RTK_DEMOD_SUCCESSFUL;
	#if 0
	unsigned char u8CrystalPd = 0;

	status = RTK_A_DVBSX_StandBy(pDemod);

	//fixme, ATBM783x suspend is based on standby and shut crystal clock.
	/*After clock is shut,I2C is not work, Chip can only be started up by using reset PIN or Power Reset.*/
	status |= RTK_A_DVBSX_Read(pDemod, 0x06, 0x0E, &u8CrystalPd);
	u8CrystalPd |= 0x01;
	status |= RTK_A_DVBSX_Write(pDemod, 0x06, 0x0E, u8CrystalPd);
	#endif
	return status;
}

typedef struct {
	unsigned char N_FB;//feedback
	unsigned char N_TS; //transport stream
	unsigned char N_SYS;//system
	unsigned char N_ADC; //ADC
} WagtailDVBSxClk_t;
typedef struct {
	unsigned char N_FB; //feedback
	unsigned char N_TS; //transport stream clk
	unsigned char N_SYS; //system
	unsigned char N_SD; //sdram
} WagtailDVBTxClk_t;
typedef struct {
	unsigned char N_FB;//feedback
	unsigned char N_TS;//transport
	unsigned char N_SYS;//system
	unsigned char N_IP; //IP
} WagtailDVBCxClk_t;

//fixme, most of register should be replaced.
void RTK_A_DVBSX_WagtailDVBSxClkSet(DVBSX_DEMOD_MODULE *pDemod)
{
	RtkDemod64Data i64Data, i64Data1;
	unsigned int AdcClkKHz = gstRtkDemodDvbConfig.ui32DVBSAdcClkKHz;
	unsigned int inv_fs_adc_dvbs = 0, fs_adc_dvbs = 0, dvb_fs_adc = 0, dvb_fs_sys = 0;

	RTK_A_DVBSX_64Mult(&i64Data, 2000, RTK_A_DVBSX_FixPow(2, 23));
	RTK_A_DVBSX_64Add(&i64Data, AdcClkKHz);
	RTK_A_DVBSX_64Mult(&i64Data1, 2, AdcClkKHz);
	inv_fs_adc_dvbs = RTK_A_DVBSX_64Div(i64Data, i64Data1);
	//inv_fs_adc_dvbs = ((0x01<<23)+AdcClkKHz/2000.0)/(AdcClkKHz/1000.0); // 1/AdcClk * 2^23

	RTK_A_DVBSX_64Mult(&i64Data, AdcClkKHz, RTK_A_DVBSX_FixPow(2, 15));
	RTK_A_DVBSX_64Mult(&i64Data1, 1, 1000);
	fs_adc_dvbs = RTK_A_DVBSX_64Div(i64Data, i64Data1);
	//fs_adc_dvbs = (AdcClkKHz/1000.0)*(0x01<<15); // AdcClk * 2^15

	RTK_A_DVBSX_64Mult(&i64Data, AdcClkKHz, 1000000);
	RTK_A_DVBSX_64Mult(&i64Data1, 1, 1000);
	dvb_fs_adc = RTK_A_DVBSX_64Div(i64Data, i64Data1);
	dvb_fs_sys = dvb_fs_adc;  //dvb_fs_sys not used in wagtail
	//dvb_fs_adc = (AdcClkKHz/1000.0)*1000000; // AdcClk * 1e6
	//dvb_fs_sys = (AdcClkKHz/1000.0)*1000000; // AdcClk * 1e6

	if (inv_fs_adc_dvbs > 0x1FFFF) {
		inv_fs_adc_dvbs = 0x1FFFF;
	}
	//fixme, set digital ADC related registers, no related with symbol rate.
	RTK_A_DVBSX_Write(pDemod, 0x09, 0xB1, (unsigned char)inv_fs_adc_dvbs & 0xFF);
	RTK_A_DVBSX_Write(pDemod, 0x09, 0xB2, (unsigned char)(inv_fs_adc_dvbs >> 8) & 0xFF);
	RTK_A_DVBSX_Write(pDemod, 0x09, 0xB3, (unsigned char)(inv_fs_adc_dvbs >> 16) & 0x01);

	RTK_A_DVBSX_Write(pDemod, 0x09, 0xBE, (unsigned char)fs_adc_dvbs & 0xFF);
	RTK_A_DVBSX_Write(pDemod, 0x09, 0xBF, (unsigned char)(fs_adc_dvbs >> 8) & 0xFF);
	RTK_A_DVBSX_Write(pDemod, 0x09, 0xC0, (unsigned char)(fs_adc_dvbs >> 16) & 0x3F);


	RTK_A_DVBSX_Write(pDemod, 0x15, 0x86, (unsigned char)dvb_fs_adc & 0xFF);
	RTK_A_DVBSX_Write(pDemod, 0x15, 0x87, (unsigned char)(dvb_fs_adc >> 8) & 0xFF);
	RTK_A_DVBSX_Write(pDemod, 0x15, 0x88, (unsigned char)(dvb_fs_adc >> 16) & 0xFF);
	RTK_A_DVBSX_Write(pDemod, 0x15, 0x89, (unsigned char)(dvb_fs_adc >> 24) & 0x0F);
}

//fixme, most of register should be replaced.
RTK_A_DVBSX_STATUS RTK_A_DVBSX_SetSysClkClockDVBSx(DVBSX_DEMOD_MODULE *pDemod)
{
	RTK_A_DVBSX_STATUS ui8Status = RTK_DEMOD_SUCCESSFUL;

	/*I2C using Crystal clock*/
	ui8Status   =  RTK_A_DVBSX_Write(pDemod, 0x01, 0x0c, 0x00);

	tv_osal_msleep(1); //Delay 1 millisecond.

	RTK_A_DVBSX_WagtailDVBSxClkSet(pDemod);

	tv_osal_msleep(1); /*Delay 1 millisecond. */

	return ui8Status;
}

/********************************************************************************
function    : RTK_A_DVBSX_DynamicCheckProcess
parameter   :none
return      :Operate  Status:RTK_DEMOD_SUCCESSFUL is Normal and other is abnormal.
Description:
This  function should be called dynamically every 500 milliseconds.
will check signal status or changes PER setting dynamically.
*********************************************************************************/
RTK_A_DVBSX_STATUS RTK_A_DVBSX_DynamicCheckProcess(DVBSX_DEMOD_MODULE *pDemod)/*API should be called on 500ms period.*/
{
	RTK_A_DVBSX_STATUS status = RTK_DEMOD_SUCCESSFUL;
	unsigned char ui8ConfigDone = 0;
	unsigned char ui8SxMode = 0, ui8DVBCMode = 0;

	status = RTK_A_DVBSX_Read(pDemod, 0x00, 0x04, &ui8ConfigDone);

	if (ui8ConfigDone & 0x01) { /*Config done*/
		status |= RTK_A_DVBSX_Read(pDemod, 0x00, 0x2b, &ui8SxMode);

		if (ui8SxMode & 0x01) { /*S/S2 mode, put DVB-SS2 periodically called function here*/
			RTK_A_DVBSX_DPLLTrackingSpeedPatch(pDemod);
			//status |= RTK_A_DVBSX_GetDvbSxSpectrumAuto();
			//status |= RTK_A_DVBSX_CheckForValidSymbolRate();
			//status |= RTK_A_DVBSX_UpdateDVBSxBerPerWinWithPeroid(500);
		}
		if ((ui8DVBCMode & 0x01) == 0x01) { /*DVB-C mode, put DVB-C periodically called function here*/
			//status |=realtek_a_dvbc_UpdateDVBCBerPerWinWithPeroid(500);
		}
	}

	return status;
}

void RTK_A_DVBSX_SetTSVarThresh(DVBSX_DEMOD_MODULE *pDemod, unsigned char Nvco)
{
	unsigned char ui8nVco = Nvco;
	unsigned char u8BitN = 0;
	unsigned short u16TsVarCtl = 0xffff, u16tmp = 0;

	/*TS range changed */
	if ((gstRtkDemodDvbConfig.ui8ConfigDVBType == RTK_DEMOD_DVBS_MODE) || (gstRtkDemodDvbConfig.ui8ConfigDVBType == RTK_DEMOD_DVBS2_MODE)) {
		if (Nvco == 51) { //mode 3
			RTK_A_DVBSX_Write(pDemod, 0x18, 0xA0, 0xc0);
			RTK_A_DVBSX_Write(pDemod, 0x18, 0xA1, 0xff);
			RTK_A_DVBSX_Write(pDemod, 0x18, 0xA2, 0xff);
			RTK_A_DVBSX_Write(pDemod, 0x18, 0xA3, 0xff);
			return;
		}
	}
	/*if ((gstRtkDemodDvbConfig.ui8ConfigDVBType == RTK_DEMOD_DVBT_MODE) || (gstRtkDemodDvbConfig.ui8ConfigDVBType == RTK_DEMOD_DVBT2_MODE)) {
		if (Nvco == 50) { //mode 2
			RTK_A_DVBSX_Write(pDemod, 0x18, 0xA0, 0x00);
			RTK_A_DVBSX_Write(pDemod, 0x18, 0xA1, 0x00);
			RTK_A_DVBSX_Write(pDemod, 0x18, 0xA2, 0xff);
			RTK_A_DVBSX_Write(pDemod, 0x18, 0xA3, 0xff);
			return;
		}
	}
	if (gstRtkDemodDvbConfig.ui8ConfigDVBType == RTK_DEMOD_DVBC_MODE) {
		if (Nvco == 40) { //mode 4
			RTK_A_DVBSX_Write(pDemod, 0x18, 0xA0, 0x00);
			RTK_A_DVBSX_Write(pDemod, 0x18, 0xA1, 0x00);
			RTK_A_DVBSX_Write(pDemod, 0x18, 0xA2, 0xff);
			RTK_A_DVBSX_Write(pDemod, 0x18, 0xA3, 0xff);
			return;
		}
	}*/

	if (ui8nVco < 40) {
		ui8nVco = 40;
	}
	if (ui8nVco > 55) {
		ui8nVco = 55;
	}
	u8BitN = 13 - (ui8nVco - 40) / 2; //Bits should clear

	u16tmp = 16 - u8BitN;
	u16tmp = (unsigned short)(RTK_A_DVBSX_FixPow(2, u16tmp));
	u16TsVarCtl = u16tmp - 1;

	//Under Thresh
	RTK_A_DVBSX_Write(pDemod, 0x18, 0xA3, ((u16TsVarCtl >> 8) & 0xff));
	RTK_A_DVBSX_Write(pDemod, 0x18, 0xA2, (u16TsVarCtl & 0xff));

	//Upper thresh
	if ((gstRtkDemodDvbConfig.ui8ConfigDVBType == RTK_DEMOD_DVBS_MODE) || (gstRtkDemodDvbConfig.ui8ConfigDVBType == RTK_DEMOD_DVBS2_MODE)) {
		if (Nvco <= 40) { //DVB-S/SS2 less than 138M
			RTK_A_DVBSX_Write(pDemod, 0x18, 0xA1, 0xff);
			RTK_A_DVBSX_Write(pDemod, 0x18, 0xA0, 0x80);
		} else if (Nvco <= 46) {
			RTK_A_DVBSX_Write(pDemod, 0x18, 0xA1, 0xff);
			RTK_A_DVBSX_Write(pDemod, 0x18, 0xA0, 0x00);
		} else if (Nvco <= 51) {
			RTK_A_DVBSX_Write(pDemod, 0x18, 0xA1, 0xfe);
			RTK_A_DVBSX_Write(pDemod, 0x18, 0xA0, 0x00);
		} else {
			RTK_A_DVBSX_Write(pDemod, 0x18, 0xA1, 0xfc);
			RTK_A_DVBSX_Write(pDemod, 0x18, 0xA0, 0x00);
		}
	} else { //DVB-T/T2 DVB-C/C2 less than 96M
		if (Nvco <= 44) {
			RTK_A_DVBSX_Write(pDemod, 0x18, 0xA1, 0xf8);
			RTK_A_DVBSX_Write(pDemod, 0x18, 0xA0, 0x00);
		} else if (Nvco <= 48) {
			RTK_A_DVBSX_Write(pDemod, 0x18, 0xA1, 0xf0);
			RTK_A_DVBSX_Write(pDemod, 0x18, 0xA0, 0x00);
		} else {
			RTK_A_DVBSX_Write(pDemod, 0x18, 0xA1, 0xe0);
			RTK_A_DVBSX_Write(pDemod, 0x18, 0xA0, 0x00);
		}
	}
}

/********************************************************************************
function    :RTK_A_DVBSX_SwapIQ
parameter   :ui8IQ:0:not swap and 1 swap
return      :Operate  Status:RTK_DEMOD_SUCCESSFUL is Normal and other is abnormal.
Description :Call this function to swap demodulator's IQ or not.
             1: swap IQ;  0: no swap.
*********************************************************************************/
RTK_A_DVBSX_STATUS    RTK_A_DVBSX_SwapIQ(DVBSX_DEMOD_MODULE *pDemod, unsigned char ui8IQ)
{
	REALTEK_A_DVBSX_INFO("\033[1;32;36m" "Set IQ swap: %u\n" "\033[m", ui8IQ);
	return RTK_A_DVBSX_Write(pDemod, 0x03, 0x02, ui8IQ);
}

//Set external demod I2C control bus and ATBM externel demod I2C address
#define DVBSX_DEMOD_I2C_8bits_ADDR 0x80
/****************************************************************************
Function:    RTK_A_DVBSX_I2CRead
Parameters:  ui8BaseAddr register Base Address, ui8RegisterAddr:register offset Address;
ui8RegValue: The pointer which return the data read form Chip.
Return:      Read  Status:RTK_DEMOD_SUCCESSFUL is Normal and other is abnormal.

Description:
This function used I2CRead to read  data form chip register, it should be implemented by user
****************************************************************************/
RTK_A_DVBSX_STATUS   RTK_A_DVBSX_I2CRead(DVBSX_DEMOD_MODULE *pDemod, unsigned char ui8BaseAddr, unsigned char ui8RegisterAddr, unsigned char *ui8RegValue)
{
	BASE_INTERFACE_MODULE *pBaseInterface;
	unsigned char ui8AddrData[2] = {0};

	pBaseInterface = pDemod->pBaseInterface;

	ui8AddrData[0] = ui8BaseAddr;
	ui8AddrData[1] = ui8RegisterAddr;


	if (pBaseInterface->I2cWrite(pBaseInterface, DVBSX_DEMOD_I2C_8bits_ADDR, ui8AddrData, 2) != FUNCTION_SUCCESS)
		return RTK_DEMOD_I2C_ERROR;

	if (pBaseInterface->I2cRead(pBaseInterface, DVBSX_DEMOD_I2C_8bits_ADDR, ui8RegValue, 1) != FUNCTION_SUCCESS)
		return RTK_DEMOD_I2C_ERROR;


	//LGIT_LIB_I2cWrite(0x80, ui8AddrData, 2);      //lgit
	//LGIT_LIB_I2cRead(0x80, ui8RegValue, 1);       //lgit


	/*Debug RTK_A_DVBSX_ I2C used print the chip ID*/
	//if((ui8BaseAddr == 0x00)&&(ui8RegisterAddr == 0x00))
	//*ui8RegValue = 0xa8;

	//ATBMDemodI2CRead(ui8RTK_A_DVBSX_SlaveAddress,ui8BaseAddr, ui8RegisterAddr,1,ui8RegValue);

	/*One step I2C format, please port this function according to I2C format of this function..*/
	/*return I2CRead(ui8RTK_A_DVBSX_SlaveAddr, 2, ui8AddrData, 1, ui8RegValue);*/

	return RTK_DEMOD_SUCCESSFUL;
}

/****************************************************************************
Function:    RTK_A_DVBSX_I2CWrite
Parameters:  ui8BaseAddr register Base Address, ui8RegisterAddr:register offset Address;
ui8data:  Data which will be write to chip.
Return:         Write status:RTK_A_DVBSX_SUCCESSFUL is normal and other is abnormal.

Description:
This function used I2CWrite to write data to chip register,it should be implemented by user
****************************************************************************/
RTK_A_DVBSX_STATUS   RTK_A_DVBSX_I2CWrite(DVBSX_DEMOD_MODULE *pDemod, unsigned char ui8BaseAddr, unsigned char ui8RegisterAddr, unsigned char ui8data)
{
	RTK_A_DVBSX_STATUS ui8Status =  RTK_DEMOD_SUCCESSFUL;
	BASE_INTERFACE_MODULE *pBaseInterface;

	unsigned char ui8I2CData[3] = {0};

	pBaseInterface = pDemod->pBaseInterface;

	ui8I2CData[0] = ui8BaseAddr;
	ui8I2CData[1] = ui8RegisterAddr;
	ui8I2CData[2] = ui8data;

	if (pBaseInterface->I2cWrite(pBaseInterface, DVBSX_DEMOD_I2C_8bits_ADDR, ui8I2CData, 3) != FUNCTION_SUCCESS)
		return RTK_DEMOD_I2C_ERROR;

	return ui8Status;
}
