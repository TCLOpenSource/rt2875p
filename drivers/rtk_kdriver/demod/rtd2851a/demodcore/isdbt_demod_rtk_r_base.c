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

		if (pBaseInterface->I2cWrite(pBaseInterface, DeviceAddr, WritingBuffer, LEN_2_BYTE) != FUNCTION_SUCCESS)
			goto error_status;

		// Get demod register bytes.
		// Note: The I2C format of demod register byte getting is as follows:
		//       start_bit + (DeviceAddr | reading_bit) + reading_bytes (ReadingByteNum bytes) + stop_bit
		if (pBaseInterface->I2cRead(pBaseInterface, DeviceAddr, &pReadingBytes[i], ReadingByteNum) != FUNCTION_SUCCESS)
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












