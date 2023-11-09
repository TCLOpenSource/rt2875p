/* ----------------------------------------------------------------------------
 File Name: DTMBIP.h

 Description:

 Version 1.0 : Created   2012.03.21

 ---------------------------------------------------------------------------- */ 
#ifndef __DTMB_DEMOD_REALTEK_H_H__
#define __DTMB_DEMOD_REALTEK_H_H__
//#ifndef __DTMBIP_H__
//#define __DTMBIP_H__


#include "dtmb_demod_rtk_type.h"
#include "dtmb_demod_base.h"
//#include <rtd_log/rtd_module_log.h>

/*HD2311 I2C Device Address */
#define DTMBIP_I2C_ADDRESS				0xE0

#ifndef ABS
#define ABS(x,y)				((x > y) ? (x-y) : (y-x))
#endif

typedef struct DTMBIP_Init_Info_t
{
	UINT8 WorkMode;			/* Working Mode, Dtmb or Cable */
	UINT8 TunerType;			/* Tuner Type, Zero-IF or Other */
	S32BITS TunerIF;				/* Tuner IF Frequency, Units: MHZ*/
	UINT8 CrystalSelect;			/* Crystal Selection */	
	UINT8 TsType;				/* Serial or Parallel */
	UINT8 TsClockPolarity;		/* Clock Sampling is at rising or falling edges */
	UINT8 TsClockValid;			/* TS Clock Valid Method, One is always valid, another is valid with valid signal */
	UINT8 TsNullPacket;			/* TS Null Packet is enable ? */
	UINT8 TsSerialOutPut;			/* In serial TS mode, out pin is in TS_D7 or TS_D0 */
    UINT8 TsSerialOrder;
    UINT8 TsSerialSyncMode;
}DTMBIP_Init_Info;

/*
HD2311 Internal related function
*/
UINT8 DTMBIP_SetSytemBandwidth6M(void);
UINT8 DTMBIP_SetSytemBandwidth8M(void);
UINT8 DTMBIP_GetPer(U32BITS *per);
UINT8 DTMBIP_GetBec(U32BITS *bec);		//new add 
UINT8 DTMBIP_GetBer(U32BITS *ber) ;    //new add
UINT8 DTMBIP_SetWorkMode(void);
UINT8 DTMBIP_SetAutoMode(void);
UINT8 DTMBIP_SetNCOValue(UINT32 NCOValue, UINT32 CO1, UINT32 CO2);
UINT8 DTMBIP_SetTimeFreq(void);
UINT8 DTMBIP_SetAXIBus(UINT32 AXIBus);
UINT8 DTMBIP_SetTunerType(UINT8 Type);
UINT8 DTMBIP_SetTsFormat(UINT8 Serial, UINT8 CLK_Polarity, UINT8 CLK_Valid, UINT8 Null_Packet, UINT8 Serial_Output, UINT8 Serial_Order, UINT8 Serial_Sync);
UINT8 DTMBIP_SetManualMode(UINT8 CarrierMode, UINT8 mode,UINT8 rate, UINT8 interleaverLength,UINT8 pnmode,UINT8 PnVariable);
UINT8 DTMBIP_GetNCOValue(UINT32 *NCOValue);
UINT8 DTMBIP_GetParameters(UINT8 *CarrierMode, UINT8 *pmode,UINT8 *prate, UINT8 *pinterleaverLength,UINT8 *Pnmode,UINT8 *PnVariable);
UINT8 DTMBIP_GetSignalSNRSC(S32BITS *SignalSNR);
UINT8 DTMBIP_GetSignalSNRMC(S32BITS *SignalSNR);
UINT8 DTMBIP_GetLdpcBER(S32BITS *pLdpcBER);
UINT8 DTMBIP_GetSignalBER(S32BITS *pSignalBER);
UINT8 DTMBIP_GetFieldStrength(UINT32 *FieldStrength);
UINT8 DTMBIP_GetSignalSNR(S32BITS *SignalSNR);
UINT8 DTMBIP_CalculateNCOValue(S32BITS TunerIF, UINT32 *NCOValue, UINT32 *CO1, UINT32 *CO2);
UINT8 DTMBIP_SoftReset(void);
UINT8 DTMBIP_SetSpectrum(UINT8 SpectrumMode);
UINT8 DTMBIP_GetBerSQI(S32BITS *Ber_SQI);
UINT8 DTMBIP_GetCNrel(S32BITS *CNrel);
UINT8 DTMBIP_GetPrel(int* p_input, int *p_rel);



void HDIC2311_SetIfInverse(void);

#endif