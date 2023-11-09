/**

@file

@brief   ISDB-T demod default function definition

ISDB-T demod default functions.

*/

#include "isdbt_demod_rtk_r_base.h"





/**

@see   ISDBT_DEMOD_FP_SET_REG_BYTES

*/
int
isdbt_demod_default_SetRegBytes(
	ISDBT_DEMOD_MODULE *pDemod,
	unsigned short RegStartAddr,
	const unsigned char *pWritingBytes,
	unsigned char ByteNum
)
{
	BASE_INTERFACE_MODULE *pBaseInterface;

	U32BITS i, j;

	unsigned char DeviceAddr;
	unsigned char WritingBuffer[I2C_BUFFER_LEN];
	U32BITS WritingByteNum, WritingByteNumMax, WritingByteNumRem;
	unsigned short RegWritingAddr;



	// Get base interface.
	pBaseInterface = pDemod->pBaseInterface;


	// Get demod I2C device address.
	pDemod->GetDeviceAddr(pDemod, &DeviceAddr);


	// Calculate maximum writing byte number.
	WritingByteNumMax = pBaseInterface->I2cWritingByteNumMax - LEN_2_BYTE;


	// Set demod register bytes with writing bytes.
	// Note: Set demod register bytes considering maximum writing byte number.
	for (i = 0; i < ByteNum; i += WritingByteNumMax) {
		// Set register writing address.
		RegWritingAddr = (unsigned short)(RegStartAddr + i);

		// Calculate remainder writing byte number.
		WritingByteNumRem = ByteNum - i;

		// Determine writing byte number.
		WritingByteNum = (WritingByteNumRem > WritingByteNumMax) ? WritingByteNumMax : WritingByteNumRem;


		// Set writing buffer.
		// Note: The I2C format of demod register byte setting is as follows:
		//       start_bit + (DeviceAddr | writing_bit) + RegWritingAddrMsb + RegWritingAddrLsb +
		//       writing_bytes (WritingByteNum bytes) + stop_bit
		WritingBuffer[0] = (RegWritingAddr >> BYTE_SHIFT) & BYTE_MASK;
		WritingBuffer[1] = RegWritingAddr & BYTE_MASK;

		for (j = 0; j < WritingByteNum; j++)
			WritingBuffer[LEN_2_BYTE + j] = pWritingBytes[i + j];


		// Set demod register bytes with writing buffer.
		if (pBaseInterface->I2cWrite(pBaseInterface, DeviceAddr, WritingBuffer, WritingByteNum + LEN_2_BYTE) !=
			FUNCTION_SUCCESS)
			goto error_status;
	}


	return FUNCTION_SUCCESS;


error_status:
	return FUNCTION_ERROR;
}





/**

@see   ISDBT_DEMOD_FP_GET_REG_BYTES

*/
int
isdbt_demod_default_GetRegBytes(
	ISDBT_DEMOD_MODULE *pDemod,
	unsigned short RegStartAddr,
	unsigned char *pReadingBytes,
	unsigned char ByteNum
)
{
	BASE_INTERFACE_MODULE *pBaseInterface;

	U32BITS i;
	unsigned char DeviceAddr;
	U32BITS ReadingByteNum, ReadingByteNumMax, ReadingByteNumRem;
	unsigned short RegReadingAddr;
	unsigned char WritingBuffer[LEN_2_BYTE];



	// Get base interface.
	pBaseInterface = pDemod->pBaseInterface;


	// Get demod I2C device address.
	pDemod->GetDeviceAddr(pDemod, &DeviceAddr);


	// Calculate maximum reading byte number.
	ReadingByteNumMax = pBaseInterface->I2cReadingByteNumMax;


	// Get demod register bytes.
	// Note: Get demod register bytes considering maximum reading byte number.
	for (i = 0; i < ByteNum; i += ReadingByteNumMax) {
		// Set register reading address.
		RegReadingAddr = (unsigned short)(RegStartAddr + i);

		// Calculate remainder reading byte number.
		ReadingByteNumRem = ByteNum - i;

		// Determine reading byte number.
		ReadingByteNum = (ReadingByteNumRem > ReadingByteNumMax) ? ReadingByteNumMax : ReadingByteNumRem;


		// Set demod register reading address.
		// Note: The I2C format of demod register reading address setting is as follows:
		//       start_bit + (DeviceAddr | writing_bit) + RegReadingAddrMsb + RegReadingAddrLsb + stop_bit
		WritingBuffer[0] = (RegReadingAddr >> BYTE_SHIFT) & BYTE_MASK;
		WritingBuffer[1] = RegReadingAddr & BYTE_MASK;

		//if (pBaseInterface->I2cWrite(pBaseInterface, DeviceAddr, WritingBuffer, LEN_2_BYTE) != FUNCTION_SUCCESS)
		//	goto error_status;

		// Get demod register bytes.
		// Note: The I2C format of demod register byte getting is as follows:
		//       start_bit + (DeviceAddr | reading_bit) + reading_bytes (ReadingByteNum bytes) + stop_bit
		//if (pBaseInterface->I2cRead(pBaseInterface, DeviceAddr, NULL, 0, &pReadingBytes[i], ReadingByteNum) != FUNCTION_SUCCESS)
		//	goto error_status;

		if (pBaseInterface->I2cRead(pBaseInterface, DeviceAddr, WritingBuffer, LEN_2_BYTE, &pReadingBytes[i], ReadingByteNum) != FUNCTION_SUCCESS)
			goto error_status;

	}


	return FUNCTION_SUCCESS;


error_status:
	return FUNCTION_ERROR;
}





/**

@see   ISDBT_DEMOD_FP_SET_REG_MASK_BITS

*/
int
isdbt_demod_default_SetRegMaskBits(
	ISDBT_DEMOD_MODULE *pDemod,
	unsigned short RegStartAddr,
	unsigned char Msb,
	unsigned char Lsb,
	const U32BITS WritingValue
)
{
	int i;

	unsigned char ReadingBytes[LEN_4_BYTE];
	unsigned char WritingBytes[LEN_4_BYTE];

	unsigned char ByteNum;
	U32BITS Mask;
	unsigned char Shift;

	U32BITS Value;


	// Calculate writing byte number according to MSB.
	ByteNum = Msb / BYTE_BIT_NUM + LEN_1_BYTE;


	// Generate mask and shift according to MSB and LSB.
	Mask = 0;

	for (i = Lsb; i < (unsigned char)(Msb + 1); i++)
		Mask |= 0x1 << i;

	Shift = Lsb;


	// Get demod register bytes according to register start adddress and byte number.
	if (pDemod->GetRegBytes(pDemod, RegStartAddr, ReadingBytes, ByteNum) != FUNCTION_SUCCESS)
		goto error_status;


	// Combine reading bytes into an unsigned integer value.
	// Note: Put lower address byte on value LSB.
	//       Put upper address byte on value MSB.
	Value = 0;

	for (i = 0; i < ByteNum; i++)
		Value |= (U32BITS)ReadingBytes[i] << (BYTE_SHIFT * i);


	// Reserve unsigned integer value unmask bit with mask and inlay writing value into it.
	Value &= ~Mask;
	Value |= (WritingValue << Shift) & Mask;


	// Separate unsigned integer value into writing bytes.
	// Note: Pick up lower address byte from value LSB.
	//       Pick up upper address byte from value MSB.
	for (i = 0; i < ByteNum; i++)
		WritingBytes[i] = (unsigned char)((Value >> (BYTE_SHIFT * i)) & BYTE_MASK);


	// Write demod register bytes with writing bytes.
	if (pDemod->SetRegBytes(pDemod, RegStartAddr, WritingBytes, ByteNum) != FUNCTION_SUCCESS)
		goto error_status;


	return FUNCTION_SUCCESS;


error_status:
	return FUNCTION_ERROR;
}





/**

@see   ISDBT_DEMOD_FP_GET_REG_MASK_BITS

*/
int
isdbt_demod_default_GetRegMaskBits(
	ISDBT_DEMOD_MODULE *pDemod,
	unsigned short RegStartAddr,
	unsigned char Msb,
	unsigned char Lsb,
	U32BITS *pReadingValue
)
{
	int i;

	unsigned char ReadingBytes[LEN_4_BYTE];

	unsigned char ByteNum;
	U32BITS Mask;
	unsigned char Shift;

	U32BITS Value;


	// Calculate writing byte number according to MSB.
	ByteNum = Msb / BYTE_BIT_NUM + LEN_1_BYTE;


	// Generate mask and shift according to MSB and LSB.
	Mask = 0;

	for (i = Lsb; i < (unsigned char)(Msb + 1); i++)
		Mask |= 0x1 << i;

	Shift = Lsb;


	// Get demod register bytes according to register start adddress and byte number.
	if (pDemod->GetRegBytes(pDemod, RegStartAddr, ReadingBytes, ByteNum) != FUNCTION_SUCCESS)
		goto error_status;


	// Combine reading bytes into an unsigned integer value.
	// Note: Put lower address byte on value LSB.
	//       Put upper address byte on value MSB.
	Value = 0;

	for (i = 0; i < ByteNum; i++)
		Value |= (U32BITS)ReadingBytes[i] << (BYTE_SHIFT * i);


	// Get register bits from unsigned integaer value with mask and shift
	*pReadingValue = (Value & Mask) >> Shift;


	return FUNCTION_SUCCESS;


error_status:
	return FUNCTION_ERROR;
}





/**

@see   ISDBT_DEMOD_FP_SET_REG_BITS

*/
int
isdbt_demod_default_SetRegBits(
	ISDBT_DEMOD_MODULE *pDemod,
	int RegBitName,
	const U32BITS WritingValue
)
{
	unsigned short RegStartAddr;
	unsigned char Msb;
	unsigned char Lsb;


	// Check if register bit name is available.
	if (pDemod->RegTable[RegBitName].IsAvailable == NO)
		goto error_status;


	// Get register start address, MSB, and LSB from register table with register bit name key.
	RegStartAddr = pDemod->RegTable[RegBitName].RegStartAddr;
	Msb          = pDemod->RegTable[RegBitName].Msb;
	Lsb          = pDemod->RegTable[RegBitName].Lsb;


	// Set register mask bits.
	if (pDemod->SetRegMaskBits(pDemod, RegStartAddr, Msb, Lsb, WritingValue) != FUNCTION_SUCCESS)
		goto error_status;


	return FUNCTION_SUCCESS;


error_status:
	return FUNCTION_ERROR;
}





/**

@see   ISDBT_DEMOD_FP_GET_REG_BITS

*/
int
isdbt_demod_default_GetRegBits(
	ISDBT_DEMOD_MODULE *pDemod,
	int RegBitName,
	U32BITS *pReadingValue
)
{
	unsigned short RegStartAddr;
	unsigned char Msb;
	unsigned char Lsb;


	// Check if register bit name is available.
	if (pDemod->RegTable[RegBitName].IsAvailable == NO)
		goto error_status;


	// Get register start address, MSB, and LSB from register table with register bit name key.
	RegStartAddr = pDemod->RegTable[RegBitName].RegStartAddr;
	Msb          = pDemod->RegTable[RegBitName].Msb;
	Lsb          = pDemod->RegTable[RegBitName].Lsb;


	// Get register mask bits.
	if (pDemod->GetRegMaskBits(pDemod, RegStartAddr, Msb, Lsb, pReadingValue) != FUNCTION_SUCCESS)
		goto error_status;


	return FUNCTION_SUCCESS;


error_status:
	return FUNCTION_ERROR;
}





/**

@see   ISDBT_DEMOD_FP_GET_DEMOD_TYPE

*/
void
isdbt_demod_default_GetDemodType(
	ISDBT_DEMOD_MODULE *pDemod,
	int *pDemodType
)
{
	// Get demod type from demod module.
	*pDemodType = pDemod->DemodType;


	return;
}





/**

@see   ISDBT_DEMOD_FP_GET_DEVICE_ADDR

*/
void
isdbt_demod_default_GetDeviceAddr(
	ISDBT_DEMOD_MODULE *pDemod,
	unsigned char *pDeviceAddr
)
{
	// Get demod I2C device address from demod module.
	*pDeviceAddr = pDemod->DeviceAddr;


	return;
}





/**

@see   ISDBT_DEMOD_FP_GET_CRYSTAL_FREQ_HZ

*/
void
isdbt_demod_default_GetCrystalFreqHz(
	ISDBT_DEMOD_MODULE *pDemod,
	U32BITS *pCrystalFreqHz
)
{
	// Get demod crystal frequency in Hz from demod module.
	*pCrystalFreqHz = pDemod->CrystalFreqHz;


	return;
}





/**

@see   ISDBT_DEMOD_FP_GET_BANDWIDTH_MODE

*/
int
isdbt_demod_default_GetBandwidthMode(
	ISDBT_DEMOD_MODULE *pDemod,
	int *pBandwidthMode
)
{
	// Get demod bandwidth mode from demod module.
	if (pDemod->IsBandwidthModeSet != YES)
		goto error_status;

	*pBandwidthMode = pDemod->BandwidthMode;


	return FUNCTION_SUCCESS;


error_status:
	return FUNCTION_ERROR;
}





/**

@see   ISDBT_DEMOD_FP_GET_SEGMENT_NUM

*/
int
isdbt_demod_default_GetSegmentNum(
	ISDBT_DEMOD_MODULE *pDemod,
	unsigned char *pSegmentNum
)
{
	// Get demod segment number from demod module.
	if (pDemod->IsSegmentNumSet != YES)
		goto error_status;

	*pSegmentNum = pDemod->SegmentNum;


	return FUNCTION_SUCCESS;


error_status:
	return FUNCTION_ERROR;
}





/**

@see   ISDBT_DEMOD_FP_GET_SUB_CHANNEL_NUM

*/
int
isdbt_demod_default_GetSubChannelNum(
	ISDBT_DEMOD_MODULE *pDemod,
	unsigned char *pSubChannelNum
)
{
	// Get demod sub-channel number from demod module.
	if (pDemod->IsSubChannelNumSet != YES)
		goto error_status;

	*pSubChannelNum = pDemod->SubChannelNum;


	return FUNCTION_SUCCESS;


error_status:
	return FUNCTION_ERROR;
}



/********************** 2 32bit data multiply, use this function, internally used function *****************/
void RTK_R_ISDBT_64Mult(RTK_R_ISDBT_64Data *pstRst, unsigned int m1, unsigned int m2)
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
unsigned int RTK_R_ISDBT_64Div(RTK_R_ISDBT_64Data stDivisor, RTK_R_ISDBT_64Data stDividend)
{
	unsigned char  ui8DivLeftShit = 0;
	unsigned int ui32Rst = 0;
	unsigned int ui32flag = 1;
	RTK_R_ISDBT_64Data stDataTmp = stDividend;
	if (stDivisor.ui32High == 0 && stDivisor.ui32Low == 0) {
		return 0;
	} else if (stDividend.ui32High == 0 && stDividend.ui32Low == 0) {
		return 0;
	} else if (stDividend.ui32High > stDivisor.ui32High) {
		return 0;
	} else if (!RTK_R_ISDBT_64ComPare(stDivisor, stDividend)) {
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

			if (!RTK_R_ISDBT_64ComPare(stDivisor, stDataTmp) || stDataTmp.ui32High >= 0x80000000) {
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
		ui32Rst += RTK_R_ISDBT_64Div(stDivisor, stDividend);
	}
	return ui32Rst;
}

/**********************64bit divide function,return 64bit value internally used function *******************************/
RTK_R_ISDBT_64Data RTK_R_ISDBT_64DivReturn64(RTK_R_ISDBT_64Data stDivisor, RTK_R_ISDBT_64Data stDividend)
{
	unsigned char  ui8DivLeftShit = 0;
	unsigned int ui32LowPart = 0;
	unsigned int ui32flag = 1;
	RTK_R_ISDBT_64Data i64Data, i64Data1;
	RTK_R_ISDBT_64Data stDataTmp = stDividend;
	i64Data.ui32High = 0;
	i64Data.ui32Low = 0;
	if (stDivisor.ui32High == 0 && stDivisor.ui32Low == 0) {
		return i64Data;
	} else if (stDividend.ui32High == 0 && stDividend.ui32Low == 0) {
		return i64Data;
	} else if (stDividend.ui32High > stDivisor.ui32High) {
		return i64Data;
	} else if (!RTK_R_ISDBT_64ComPare(stDivisor, stDividend)) {
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
			if (!RTK_R_ISDBT_64ComPare(stDivisor, stDataTmp) || stDataTmp.ui32High >= 0x80000000) {
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
		i64Data1 = RTK_R_ISDBT_64DivReturn64(stDivisor, stDividend);
		ui32LowPart = i64Data.ui32Low;
		i64Data.ui32Low += i64Data1.ui32Low;
		i64Data.ui32High += i64Data1.ui32High;
		if (ui32LowPart > i64Data.ui32Low) {
			i64Data.ui32High += 1;
		}

	}
	return i64Data;
}


/********************* Fixed version of pow(x,y), y should greater than 0, internally used function **************/
unsigned int  RTK_R_ISDBT_FixPow(unsigned int x, unsigned int y)
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


/**********************  internally used function *************************************************************/
unsigned char  RTK_R_ISDBT_64ComPare(RTK_R_ISDBT_64Data stPara, RTK_R_ISDBT_64Data stPara1)
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









