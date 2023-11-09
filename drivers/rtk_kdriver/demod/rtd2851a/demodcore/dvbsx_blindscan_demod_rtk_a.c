/***************************************************************************** 
Copyright 2018, Realtek Inc. All rights reserved.
File Name: dvbs2_blindscan_demod_rtk.c
Version: 1.0.5
Released 2018-03-15
Description: DVB-S/S2 blind scan
***********************************************************************************************************/

#include "dvbsx_demod_rtk_a.h"
#include "tv_osal.h"

volatile unsigned char DvbSxBlindScanState = 0;
unsigned char DvbSxBlindScanMode = 0;
unsigned char gDvbSxUniInverseFlag = 0;
struct RtkDemodDvbSxChannelScanParams gDvbSxScanParams;
struct RtkDemodDvbSxBsLocalParams gDvbSxBsLocalParams;
struct RtkDemodDvbSxBsSpecialParams gDvbSxBsSpecialParams;

extern unsigned int u32DefaultStandard;
extern stBlindScanCheckedParams gScanedParams;

stBlindScanedSignal gScanedAllSymbols;

//Add scaned symbols to symbol list.
void RTK_A_DVBSX_AddChannel(DVBSX_DEMOD_MODULE *pDemod, unsigned int u32CenterFrequencyk)
{
	unsigned int u32Iter = 0;
	unsigned int u32FindIter = 0;
	unsigned int u32index = 0;
	unsigned char u8SameSymbolFound = 0;
	unsigned int u32FreqOffsetKHz = 0;
	unsigned int u32SymbolRFFreq = 0, u32InvertSymbolRFFreq = 0;
	int i32TmpFreq = 0;
	unsigned int u32MaxSymbolRateLimit = 0;
	unsigned int u32MinSymbolRateLimit = 0;
	
	if(gDvbSxBsSpecialParams.u8UnicableEnabled)
	{
		u32MaxSymbolRateLimit = MAX_UNICABLE_BS_SYMBOL_RATE_KHZ;
		u32MinSymbolRateLimit = MIN_UNICABLE_BS_SYMBOL_RATE_KHZ;
	}
	else
	{
		u32MaxSymbolRateLimit = MAX_NORMAL_BS_SYMBOL_RATE_KHZ;
		u32MinSymbolRateLimit = MIN_NORMAL_BS_SYMBOL_RATE_KHZ;
	}
	#ifdef BS_SCAN_DEBUG
	REALTEK_A_DVBSX_INFO("[AddChannel] Scaned Symbol Rate Limited: %u to %u\n", u32MinSymbolRateLimit, u32MaxSymbolRateLimit);
	#endif

	#ifdef BS_SCAN_DEBUG
	REALTEK_A_DVBSX_INFO("[AddChannel] Scaned symbol num: %d\n", gScanedParams.u8ScanedSymbolNum);
	#endif
	for(u32Iter = 0; u32Iter < gScanedParams.u8ScanedSymbolNum; u32Iter++)
	{
		u32FindIter = gScanedAllSymbols.u32ScanedSignalNum;
		u8SameSymbolFound = 0;
		i32TmpFreq = u32CenterFrequencyk;
		i32TmpFreq = i32TmpFreq + gScanedParams.ScanedSymbols[u32Iter].i32ScanedCFO;
		u32SymbolRFFreq = i32TmpFreq;

		i32TmpFreq = u32CenterFrequencyk;
		i32TmpFreq = i32TmpFreq - gScanedParams.ScanedSymbols[u32Iter].i32ScanedCFO;
		u32InvertSymbolRFFreq = i32TmpFreq;

		#ifdef BS_SCAN_DEBUG
		REALTEK_A_DVBSX_INFO("u32FindIter %d\n", u32FindIter);
		#endif
		//Iterate saved channels, try to found equal signal and save time.
		while(u32FindIter > 0)
		{
			#ifdef BS_SCAN_DEBUG
			REALTEK_A_DVBSX_INFO("NewSymbolrate: %d PreSavedSymbolrate: %d\n", gScanedParams.ScanedSymbols[u32Iter].u32SymbolRateKHz, gScanedAllSymbols.ScanedSignalList[u32FindIter -1].u32SymbolRateKHz);
			#endif
			//Symbol rate < 10000PPM && CFO < 2M. should be same signal. as there no two same symbol signal with CFO < 2MHz, compared with Satellite CFO magin
			if(RTK_A_DVBSX_AbsSub(gScanedParams.ScanedSymbols[u32Iter].u32SymbolRateKHz, gScanedAllSymbols.ScanedSignalList[u32FindIter -1].u32SymbolRateKHz) < (gScanedParams.ScanedSymbols[u32Iter].u32SymbolRateKHz/900))
			{
				#ifdef BS_SCAN_DEBUG
				REALTEK_A_DVBSX_INFO("u32SymbolRFFreq: %d Iterfreq: %d\n", u32SymbolRFFreq, gScanedAllSymbols.ScanedSignalList[u32FindIter - 1].u32ScanedFreqKHz);
				REALTEK_A_DVBSX_INFO("u32InvertSymbolRFFreq: %d Iterfreq: %d\n", u32InvertSymbolRFFreq, gScanedAllSymbols.ScanedSignalList[u32FindIter - 1].u32ScanedFreqKHz);
				#endif
				u32FreqOffsetKHz = (gScanedParams.ScanedSymbols[u32Iter].u32SymbolRateKHz/2);

				if(gDvbSxBsSpecialParams.u8UnicableEnabled)
				{
					if(gScanedParams.ScanedSymbols[u32Iter].u32SymbolRateKHz > 11000)
					{
						if(u32FreqOffsetKHz > 4900)
						{
							#ifdef BS_SCAN_DEBUG
							REALTEK_A_DVBSX_INFO("Unicable BS fix range u32FreqOffsetKHz > 4900\n");
							#endif
							u32FreqOffsetKHz = 4900;
						}
					}
					else
					{
				if(u32FreqOffsetKHz > 8000)
				{
							#ifdef BS_SCAN_DEBUG
							REALTEK_A_DVBSX_INFO("Unicable BS fix range u32FreqOffsetKHz > 8000\n");
							#endif
					u32FreqOffsetKHz = 8000;
				}
					}
				}
				else
				{
					if(u32FreqOffsetKHz > 8000)
					{
						#ifdef BS_SCAN_DEBUG
						REALTEK_A_DVBSX_INFO("Normal BS fix range u32FreqOffsetKHz > 8000\n");
						#endif
						u32FreqOffsetKHz = 8000;
					}
				}
				if(RTK_A_DVBSX_AbsSub(u32SymbolRFFreq, gScanedAllSymbols.ScanedSignalList[u32FindIter - 1].u32ScanedFreqKHz) < u32FreqOffsetKHz)
				{
					u8SameSymbolFound = 1;
					//#ifdef BS_SCAN_DEBUG
					REALTEK_A_DVBSX_INFO("Delete repeat symbol frequency: %d Symbolrate: %d\n", u32SymbolRFFreq, gScanedParams.ScanedSymbols[u32Iter].u32SymbolRateKHz);
					//#endif
					break;
				}
			}
			//At least find back 20MHz.
			if(RTK_A_DVBSX_AbsSub(u32SymbolRFFreq, gScanedAllSymbols.ScanedSignalList[u32FindIter - 1].u32ScanedFreqKHz) > 20000)
			{
				break;
			}
			u32FindIter--;
		};

		//Add new symbol to Symbol list.
		if((!u8SameSymbolFound) && (gScanedParams.ScanedSymbols[u32Iter].u32SymbolRateKHz > u32MinSymbolRateLimit) && (gScanedParams.ScanedSymbols[u32Iter].u32SymbolRateKHz < u32MaxSymbolRateLimit))
		{
			if(gDvbSxBsSpecialParams.u8UnicableEnabled) //Avoid adding out of UB BW signals
			{
				if(RTK_A_DVBSX_AbsSub(u32SymbolRFFreq, u32CenterFrequencyk) <= 6500)
				{
					//Add new symbol to symbol list.
					u32index = gScanedAllSymbols.u32ScanedSignalNum;
					if(u32index < TOTAL_MAX_SCANED_SYMBOL)
					{
						gScanedAllSymbols.ScanedSignalList[u32index].u32ScanedFreqKHz = u32SymbolRFFreq;
						gScanedAllSymbols.ScanedSignalList[u32index].u32SymbolRateKHz = gScanedParams.ScanedSymbols[u32Iter].u32SymbolRateKHz;
						gScanedAllSymbols.u32ScanedSignalNum = gScanedAllSymbols.u32ScanedSignalNum + 1;
						//#ifdef BS_SCAN_DEBUG
						REALTEK_A_DVBSX_INFO("Unicable BS add founded new signal frequency: %d Symbolrate: %d (Spectrum Normal)\n", u32SymbolRFFreq, gScanedParams.ScanedSymbols[u32Iter].u32SymbolRateKHz);
						//#endif
					}

					if(gDvbSxBsSpecialParams.pCheckedSpectrum == RTK_DVBSx_SIGNAL_SPECTRUM_UNKNOWN)
					{
						//Add new invert symbol to symbol list.
						u32index = gScanedAllSymbols.u32ScanedSignalNum;
						if(u32index < TOTAL_MAX_SCANED_SYMBOL)
						{
							gScanedAllSymbols.ScanedSignalList[u32index].u32ScanedFreqKHz = u32InvertSymbolRFFreq;
							gScanedAllSymbols.ScanedSignalList[u32index].u32SymbolRateKHz = gScanedParams.ScanedSymbols[u32Iter].u32SymbolRateKHz;
							gScanedAllSymbols.u32ScanedSignalNum = gScanedAllSymbols.u32ScanedSignalNum + 1;
							//#ifdef BS_SCAN_DEBUG
							REALTEK_A_DVBSX_INFO("Unicable BS add founded new signal frequency: %d Symbolrate: %d (Spectrum Invert)\n", u32InvertSymbolRFFreq, gScanedParams.ScanedSymbols[u32Iter].u32SymbolRateKHz);
							//#endif
						}
					}
				}
				else
				{
					//#ifdef BS_SCAN_DEBUG
					REALTEK_A_DVBSX_INFO("Unicable BS out of UB range, remove founded signal frequency: %d Symbolrate: %d\n", u32SymbolRFFreq, gScanedParams.ScanedSymbols[u32Iter].u32SymbolRateKHz);
					//#endif
				}
			}
			else
			{
				//Add new symbol to symbol list.
				u32index = gScanedAllSymbols.u32ScanedSignalNum;
				if(u32index < TOTAL_MAX_SCANED_SYMBOL)
				{
					gScanedAllSymbols.ScanedSignalList[u32index].u32ScanedFreqKHz = u32SymbolRFFreq;
					gScanedAllSymbols.ScanedSignalList[u32index].u32SymbolRateKHz = gScanedParams.ScanedSymbols[u32Iter].u32SymbolRateKHz;
					gScanedAllSymbols.u32ScanedSignalNum = gScanedAllSymbols.u32ScanedSignalNum + 1;
					//#ifdef BS_SCAN_DEBUG
					REALTEK_A_DVBSX_INFO("Normal BS add founded new signal frequency: %d Symbolrate: %d\n", u32SymbolRFFreq, gScanedParams.ScanedSymbols[u32Iter].u32SymbolRateKHz);
					//#endif
				}
			}
		}
	}
}


void RTK_A_DVBSX_SortScanValidSymbolRateFreq(DVBSX_DEMOD_MODULE *pDemod)
{
	unsigned int u32SymbolRFFreq = 0, u32RFFreqMin = 0;
	unsigned int u32SymbolTmp = 0, u32sortNumber = 0;
	unsigned int i = 0, j = 0;

	if(gScanedAllSymbols.u32ScanedSignalNum <= 1)
	{
		if(gDvbSxBsSpecialParams.u8UnicableEnabled)
		{
			gScanedAllSymbols.u32ScanedSortStartIdx = gScanedAllSymbols.u32ScanedSignalNum;
		}
		return;
	}

	if(gDvbSxBsSpecialParams.u8UnicableEnabled)
	{
		u32sortNumber = gScanedAllSymbols.u32ScanedSortStartIdx;//Uni BS only sort current scanned signals
	}
	else
	{
	if(gScanedAllSymbols.u32ScanedSignalNum > (unsigned int)(gScanedParams.u8ScanedSymbolNum + 5)) //Not need to sort all symbols
	{
		u32sortNumber = gScanedAllSymbols.u32ScanedSignalNum - (unsigned int)(gScanedParams.u8ScanedSymbolNum + 5); //sort only new added symbols
	}
	else
	{
		u32sortNumber = 0; //sort from first signal.
	}
	}
	#ifdef BS_SCAN_DEBUG
	REALTEK_A_DVBSX_INFO("RTK_A_DVBSX_SortScanValidSymbolRateFreq\n");
	REALTEK_A_DVBSX_INFO("u32sortNumber = %u\n", u32sortNumber);
	REALTEK_A_DVBSX_INFO("=============================\n");
	for(i = u32sortNumber; i < gScanedAllSymbols.u32ScanedSignalNum; i++)
	{
		REALTEK_A_DVBSX_INFO("gScanedAllSymbols.ScanedSignalList[%u].u32ScanedFreqKHz = %u; gScanedAllSymbols.ScanedSignalList[%u].u32SymbolRateKHz = %u\n", i, gScanedAllSymbols.ScanedSignalList[i].u32ScanedFreqKHz, i, gScanedAllSymbols.ScanedSignalList[i].u32SymbolRateKHz);
	}
	REALTEK_A_DVBSX_INFO("=============================\n");
	#endif
	for(i = u32sortNumber; i < gScanedAllSymbols.u32ScanedSignalNum; i++)
	{
		u32RFFreqMin = gScanedAllSymbols.ScanedSignalList[i].u32ScanedFreqKHz;

		for(j = i; j < gScanedAllSymbols.u32ScanedSignalNum; j++)
		{
			if(gScanedAllSymbols.ScanedSignalList[j].u32ScanedFreqKHz < u32RFFreqMin)
			{
				//Swap
				u32RFFreqMin = gScanedAllSymbols.ScanedSignalList[j].u32ScanedFreqKHz;
				u32SymbolRFFreq = gScanedAllSymbols.ScanedSignalList[i].u32ScanedFreqKHz;
				u32SymbolTmp = gScanedAllSymbols.ScanedSignalList[i].u32SymbolRateKHz;

				gScanedAllSymbols.ScanedSignalList[i].u32ScanedFreqKHz = gScanedAllSymbols.ScanedSignalList[j].u32ScanedFreqKHz;
				gScanedAllSymbols.ScanedSignalList[i].u32SymbolRateKHz = gScanedAllSymbols.ScanedSignalList[j].u32SymbolRateKHz;

				gScanedAllSymbols.ScanedSignalList[j].u32ScanedFreqKHz = u32SymbolRFFreq;
				gScanedAllSymbols.ScanedSignalList[j].u32SymbolRateKHz = u32SymbolTmp;
			}
		}
	}
	#ifdef BS_SCAN_DEBUG
	for(i = u32sortNumber; i < gScanedAllSymbols.u32ScanedSignalNum; i++)
	{
		REALTEK_A_DVBSX_INFO("gScanedAllSymbols.ScanedSignalList[%u].u32ScanedFreqKHz = %u; gScanedAllSymbols.ScanedSignalList[%u].u32SymbolRateKHz = %u\n", i, gScanedAllSymbols.ScanedSignalList[i].u32ScanedFreqKHz, i, gScanedAllSymbols.ScanedSignalList[i].u32SymbolRateKHz);
	}
	REALTEK_A_DVBSX_INFO("=============================\n");
	#endif
	gScanedAllSymbols.u32ScanedSortStartIdx = gScanedAllSymbols.u32ScanedSignalNum;
}


/****************************************************************************
Function   :RTK_A_DVBSX_DvBSxBlindScanInit
Parameters : none,
Return     : 0 is Success,Other is Failed
Description:
This function Initialize blind scan, the parameters can be adjusted depends
on scanning range and tuner used.
****************************************************************************/
RTK_A_DVBSX_STATUS RTK_A_DVBSX_DvBSxBlindScanInit(DVBSX_DEMOD_MODULE *pDemod)
{
	RTK_A_DVBSX_STATUS status = RTK_DEMOD_SUCCESSFUL;
	unsigned int u32Iter = 0;

	gDvbSxBsSpecialParams.u8IsScanSymbolMode = 1;//First blindscan is scan symbol mode
	gDvbSxBsSpecialParams.u8IsCheckTPTunerDone = 0;
	gDvbSxBsSpecialParams.u8IsTPLock = 0;
	gDvbSxBsSpecialParams.u8BsLnbMode = 0;
	gDvbSxBsSpecialParams.u8IsSpectrumCheckDone = 0;
	gDvbSxBsSpecialParams.u8NoSignalInSpectrum = 0;
	gDvbSxBsSpecialParams.u8SpectrumModeCnt = 0;

	/*The following setting maybe changed be DTV application, such scan a small frequency range.*/
	gDvbSxScanParams.u32ScanStartFrequencyKHz = 950*1000; /*Blind scan start frequency, may be changed.*/
	gDvbSxScanParams.u32ScanStopFrequencyKHz = 2150*1000; /*Blind Scan stop frequency.*/
	//gDvbSxScanParams.u32MinSymbolRateHz = 2000000; /*optional:999K--->5000Km, set the minimal symbol rate that will be received. */
	gDvbSxScanParams.u32MinSymbolRateHz = 999000; /*optional:999K--->5000Km, set the minimal symbol rate that will be received. */
	gDvbSxScanParams.u32MaxSymbolRateHz = 45000000;
	gDvbSxScanParams.u32BlindScanStepKHz = 20000;//30000
	gDvbSxScanParams.u8SingalInLockingState = 0;
	gDvbSxScanParams.u32MaxCarrierOffsetKHz = 12000;
	gDvbSxScanParams.i32CFOLimitKHz = 12000; /*Maximum CFO limit in scan symbols */
	gDvbSxScanParams.i32SpectrumRangeLimit = 40000; /*Maximum half spectrum range limit in scan symbols */
	gDvbSxScanParams.u32CurrentTunedFreKHz = 950*1000;
	gDvbSxScanParams.u32ScanPreCheckedFreqKHz = 950*1000;
	gDvbSxScanParams.u32PreTunedSymbolRateHz = 2000000;
	gDvbSxScanParams.u8SignalRequiredCheckTimes = 0;

	gDvbSxScanParams.u32CurrentSymbolScanFreKHz = gDvbSxScanParams.u32ScanStartFrequencyKHz;
	gDvbSxScanParams.u32CurrentWorkFreKHzProcessed = gDvbSxScanParams.u32ScanStartFrequencyKHz;

	gDvbSxScanParams.u32BlindScanCheckedSignalIndex = 0;

	if(gDvbSxScanParams.u32BlindScanStepKHz > 20000)
	{
		gDvbSxScanParams.u32BlindScanStepKHz = 20000;//30000
	}

	gDvbSxScanParams.u32ValidChannelNumber = 0;
	gDvbSxScanParams.ui32ScanedValidChannelNum = 0;
	gDvbSxScanParams.u32GetFoundChannelNumber = 0;
	for(u32Iter = 0; u32Iter < MAX_SCANED_CHANNEL_NUMBER; u32Iter++)
	{
		gDvbSxScanParams.DvbSxChannel[u32Iter].u32FrequencyKHz = 0;
		gDvbSxScanParams.DvbSxChannel[u32Iter].u32SymbolRateHz = 0;
		gDvbSxScanParams.GetFoundChannel[u32Iter].u32FrequencyKHz = 0;
		gDvbSxScanParams.GetFoundChannel[u32Iter].u32SymbolRateHz = 0;

		//gDvbSxScanParams.DvbSxChannel[u32Iter].stDvbSxSignalPrams.DVBS2CodeRate = RTK_DVBS2_CODE_RATE_DUMMY;
		//gDvbSxScanParams.DvbSxChannel[u32Iter].stDvbSxSignalPrams.DVBSCodeRate = RTK_DVBS_CODE_RATE_UNKNOWN;
		//gDvbSxScanParams.DvbSxChannel[u32Iter].stDvbSxSignalPrams.DVBSxModu = RTK_DVBSx_MODU_UNKNOWN;
		//gDvbSxScanParams.DvbSxChannel[u32Iter].stDvbSxSignalPrams.DVBSxSymbolRateHz = 0;
		//gDvbSxScanParams.DvbSxChannel[u32Iter].stDvbSxSignalPrams.DVBSxType = RTK_DVBS_CODE_RATE;
		//gDvbSxScanParams.DvbSxChannel[u32Iter].stDvbSxSignalPrams.DVBS2XCodeRate = RTK_DVBS2X_CODE_RATE_UNKNOWN;
	}

	gScanedAllSymbols.u32ScanedSignalNum = 0;
	gScanedAllSymbols.u32ScanedSortStartIdx = 0;
	for(u32Iter = 0; u32Iter < TOTAL_MAX_SCANED_SYMBOL; u32Iter++)
	{
		gScanedAllSymbols.ScanedSignalList[u32Iter].u32ScanedFreqKHz = 0;
		gScanedAllSymbols.ScanedSignalList[u32Iter].u32SymbolRateKHz = 0;
	}
	gScanedParams.u8ScanedSymbolNum = 0;

	gDvbSxScanParams.u8FirstfrequencyScaned = 0;
	gDvbSxScanParams.u32MaxSignalRightRangeFreqKHz = gDvbSxScanParams.u32ScanStartFrequencyKHz;
	#ifdef BS_SCAN_DEBUG
	REALTEK_A_DVBSX_INFO("RTK_A_DVBSX_DvBSxBlindScanInit done!\n");
	#endif
	DvbSxBlindScanState = 1;
	gDvbSxUniInverseFlag = 0;
	gDvbSxBsSpecialParams.u8UnicableEnabled = 0;
	gDvbSxBsSpecialParams.pSignalSpectrum = RTK_DVBSx_SIGNAL_SPECTRUM_INVERSE;
	gDvbSxBsSpecialParams.pCheckedSpectrum = RTK_DVBSx_SIGNAL_SPECTRUM_UNKNOWN;
	return status;
}


/****************************************************************************
Function   : RTK_A_DVBSX_BlindScanVerifyChannel
Parameters : u32CenterFrequency, pu8Exist: 0 indicate no same channel exist in range.
Return     : 0 is Success,Other is Failed
Description:
This function is internal function
****************************************************************************/
RTK_A_DVBSX_STATUS RTK_A_DVBSX_BlindScanVerifyChannel(DVBSX_DEMOD_MODULE *pDemod, unsigned int u32CenterFrequencyKHz, unsigned char *pu8Exist)
{
	unsigned int u32iter = 0;
	int i32IterCenterFreq = 0;
	int i32CenterFrequency = 0;
	int i32MinAciOffsetKHz = 1000; //Minimal frequency interval.
	int i32HalfBandOfValidSignal = 0;
	RTK_A_DVBSX_STATUS status = RTK_DEMOD_SUCCESSFUL;
	*pu8Exist = 0;

	i32CenterFrequency = u32CenterFrequencyKHz;
	if(gDvbSxScanParams.u32ValidChannelNumber == 0)
	{
		*pu8Exist = 0;
		status = RTK_DEMOD_SUCCESSFUL;
		return status;
	}

	for(u32iter = 0; u32iter < gDvbSxScanParams.u32ValidChannelNumber; u32iter++)
	{
		i32IterCenterFreq = gDvbSxScanParams.DvbSxChannel[u32iter].u32FrequencyKHz;

		i32MinAciOffsetKHz = 1000;

		//if(gDvbSxScanParams.DvbSxChannel[u32iter].stDvbSxSignalPrams.DVBSxType == RTK_DVBS_CODE_RATE)
		//{
		//	i32HalfBandOfValidSignal = (int)((gDvbSxScanParams.DvbSxChannel[u32iter].u32SymbolRateHz*27)/40000);
		//}
		//else if(gDvbSxScanParams.DvbSxChannel[u32iter].stDvbSxSignalPrams.DVBSxType == RTK_DEMOD_DVBS2_MODE)
		//{
		//	i32HalfBandOfValidSignal = (int)((gDvbSxScanParams.DvbSxChannel[u32iter].u32SymbolRateHz * 23)/40000);
		//}
		//else
		//{
		//	i32HalfBandOfValidSignal = (int)((gDvbSxScanParams.DvbSxChannel[u32iter].u32SymbolRateHz * 23)/40000);
		//}
		i32HalfBandOfValidSignal = (int)((gDvbSxScanParams.DvbSxChannel[u32iter].u32SymbolRateHz * 23)/40000);

		if(i32MinAciOffsetKHz < i32HalfBandOfValidSignal)
		{
			i32MinAciOffsetKHz = i32HalfBandOfValidSignal;
		} 

		if((i32CenterFrequency < i32IterCenterFreq + i32MinAciOffsetKHz) && (i32CenterFrequency > i32IterCenterFreq - i32MinAciOffsetKHz)) /*carrier offset < 1000KHz*/
		{
			REALTEK_A_DVBSX_INFO("Skip i32CenterFrequency: %d i32IterCenterFreq: %d i32MinAciOffsetKHz: %d\n", i32CenterFrequency, i32IterCenterFreq, i32MinAciOffsetKHz);
			*pu8Exist = 1; /*found one channel which have equal center frequency in channel list, should skip it.*/
			return status;
		}
	}
	/*Check for Channel list memory for blind scan*/
	if(gDvbSxScanParams.u32ValidChannelNumber + 1 < MAX_SCANED_CHANNEL_NUMBER)
	{
		*pu8Exist = 0;
		status = RTK_DEMOD_SUCCESSFUL;
	}else{
		*pu8Exist = 1;
		status = RTK_DEMOD_FAILED;
	}
	return status;
}


/*Set Blind scan start frequency and stop frequency if required 
Called after RTK_A_DVBSX_DvBSxBlindScanInit to change default parameters. */
void RTK_A_DVBSX_SetDvBSxBlindScanParams(DVBSX_DEMOD_MODULE *pDemod, unsigned int u32StartFreqKHz, unsigned int u32StopFreqKHz)
{
	gDvbSxScanParams.u32ScanStartFrequencyKHz = u32StartFreqKHz;
	gDvbSxScanParams.u32ScanStopFrequencyKHz = u32StopFreqKHz;
	gDvbSxScanParams.u32CurrentSymbolScanFreKHz = gDvbSxScanParams.u32ScanStartFrequencyKHz;/*used for process */
	
	gDvbSxBsLocalParams.u32StopFrequency = gDvbSxScanParams.u32ScanStopFrequencyKHz;

	gDvbSxBsLocalParams.u32SymbolScanStep = gDvbSxScanParams.u32BlindScanStepKHz; /*Blind scan step to find signals*/

	gDvbSxBsLocalParams.u32TmpSymbolStep = 0; /*Used for scan step jump, when signal locked, calculation jump step based on lock signal's bandwidth*/

	gDvbSxBsLocalParams.u32TuneCenterFrequencyKHz = gDvbSxScanParams.u32ScanStartFrequencyKHz; //Default value.
	gDvbSxBsLocalParams.u32TuneCheckSymbolRateKHz = 0;

	gDvbSxBsLocalParams.u32tmpSymbolrate = 0;
	gDvbSxBsLocalParams.u32NextSearchingfreq = 0; //Record last locked signal's spectrum edge.

	gDvbSxBsLocalParams.u8NewChannelFound = 0; /*If u8NewChannelFound == 1, new signal locked.*/
	gDvbSxBsLocalParams.ValidChannlIndex = 0; //Array index, Number - 1.
	gDvbSxBsLocalParams.i32BlindScanStepKHz = 0; /*Step size of center frequency between two symbols scan*/

	#ifdef CONFIG_CUSTOMER_TV006
	if(gDvbSxScanParams.u32ScanStartFrequencyKHz == 950000 && gDvbSxScanParams.u32ScanStopFrequencyKHz == 1750000)
	#else
	if(gDvbSxScanParams.u32ScanStartFrequencyKHz == 950000 && gDvbSxScanParams.u32ScanStopFrequencyKHz == 1460000)
	#endif
	{
		gDvbSxBsSpecialParams.u8BsLnbMode = 1;//C Band
		REALTEK_A_DVBSX_INFO("Satellite Type: C Band\n");
	}
	else
	{
		gDvbSxBsSpecialParams.u8BsLnbMode = 2;//Ku Band
		REALTEK_A_DVBSX_INFO("Satellite Type: KU Band & Others Band\n");
	}
}


RTK_A_DVBSX_STATUS RTK_A_DVBSX_BlindScan_GetTunerFreq(DVBSX_DEMOD_MODULE *pDemod, unsigned int *TunerCenterFreqMhz, unsigned int *TunerCutOffFreqKhz)
{
	RTK_A_DVBSX_STATUS status = RTK_DEMOD_SUCCESSFUL;
	#ifdef BS_SCAN_DEBUG
	REALTEK_A_DVBSX_INFO("[GetTunerFreq] ScanSymbolMode: %d CheckTpMode: %d IsTPLock: %d [u32BlindScanCheckedSignalIndex: %d u32ScanedSignalNum: %d]\n", 
		gDvbSxBsSpecialParams.u8IsScanSymbolMode, gDvbSxBsSpecialParams.u8IsCheckTPTunerDone, gDvbSxBsSpecialParams.u8IsTPLock, gDvbSxScanParams.u32BlindScanCheckedSignalIndex, gScanedAllSymbols.u32ScanedSignalNum);
	#endif

	if(gDvbSxBsSpecialParams.u8IsScanSymbolMode)
	{
		gDvbSxBsLocalParams.u32TmpSymbolStep = 0;
		gDvbSxBsSpecialParams.u8IsTPLock = 0;

		// All symbols have been checked or no valid symbols can be used will scan new symbols 
		if(gDvbSxScanParams.u32BlindScanCheckedSignalIndex >= gScanedAllSymbols.u32ScanedSignalNum)
		{
			#ifdef BS_SCAN_DEBUG
			REALTEK_A_DVBSX_INFO("u32MaxSignalRightRangeFreqKHz: %d u32CurrentSymbolScanFreKHz: %d\n", gDvbSxScanParams.u32MaxSignalRightRangeFreqKHz, gDvbSxScanParams.u32CurrentSymbolScanFreKHz);
			#endif
			if(gDvbSxScanParams.u32MaxSignalRightRangeFreqKHz > gDvbSxScanParams.u32CurrentSymbolScanFreKHz)
			{
				gDvbSxBsLocalParams.u32TmpSymbolStep = gDvbSxScanParams.u32MaxSignalRightRangeFreqKHz - gDvbSxScanParams.u32CurrentSymbolScanFreKHz;
			}

			if(gDvbSxBsLocalParams.u32TmpSymbolStep > 30000) /*Limit maximum step 30MHz*/
			{
				gDvbSxBsLocalParams.u32TmpSymbolStep = 30000;
			}

			if(gDvbSxBsSpecialParams.u8UnicableEnabled)
			{
				REALTEK_A_DVBSX_INFO("[Unicable Blindscan][Auto Spectrum Detection]\n");
				
				#ifdef CONFIG_CUSTOMER_TV006
				//Force MAX Tuner Step
				if(gDvbSxBsSpecialParams.u8BsLnbMode == 1)
				{
					gDvbSxScanParams.u32CurrentSymbolScanFreKHz = gDvbSxScanParams.u32CurrentSymbolScanFreKHz + 20000;
				}
				else
				{
					gDvbSxScanParams.u32CurrentSymbolScanFreKHz = gDvbSxScanParams.u32CurrentSymbolScanFreKHz + 10000;
				}
				#else
				gDvbSxScanParams.u32CurrentSymbolScanFreKHz = gDvbSxScanParams.u32CurrentSymbolScanFreKHz + 10000;
				#endif
			}
			else
			{
				REALTEK_A_DVBSX_INFO("[Normal Blindscan]\n");
				#ifdef BS_SCAN_DEBUG
				REALTEK_A_DVBSX_INFO("u32TmpSymbolStep: %d u32SymbolScanStep: %d\n", gDvbSxBsLocalParams.u32TmpSymbolStep, gDvbSxBsLocalParams.u32SymbolScanStep);
				#endif
				if(gDvbSxBsLocalParams.u32TmpSymbolStep > gDvbSxBsLocalParams.u32SymbolScanStep) 
				{
					gDvbSxScanParams.u32CurrentSymbolScanFreKHz = gDvbSxScanParams.u32CurrentSymbolScanFreKHz + gDvbSxBsLocalParams.u32TmpSymbolStep;
				}
				else
				{
					gDvbSxScanParams.u32CurrentSymbolScanFreKHz = gDvbSxScanParams.u32CurrentSymbolScanFreKHz + gDvbSxBsLocalParams.u32SymbolScanStep;
				}
			}
			if(gDvbSxScanParams.u8FirstfrequencyScaned == 0)
			{
				gDvbSxScanParams.u8FirstfrequencyScaned = 1;
				gDvbSxScanParams.u32CurrentSymbolScanFreKHz = gDvbSxScanParams.u32ScanStartFrequencyKHz;
			}
			#ifdef BS_SCAN_DEBUG
			REALTEK_A_DVBSX_INFO("u32CurrentSymbolScanFreKHz: %d\n", gDvbSxScanParams.u32CurrentSymbolScanFreKHz);
			#endif
		}
		*TunerCenterFreqMhz = gDvbSxScanParams.u32CurrentSymbolScanFreKHz/1000;
		*TunerCutOffFreqKhz = 70000;
	}
	else
	{
		//Set tuner to TP freq and SR
		*TunerCenterFreqMhz = (gDvbSxBsLocalParams.u32TuneCenterFrequencyKHz+500)/1000;
		*TunerCutOffFreqKhz = gDvbSxBsLocalParams.u32TuneCheckSymbolRateKHz;
		gDvbSxBsSpecialParams.u8IsCheckTPTunerDone = 1;
	}

	return status;
}


RTK_A_DVBSX_STATUS RTK_A_DVBSX_BlindScan_NextFreq(DVBSX_DEMOD_MODULE *pDemod, unsigned int *bBlindScanEnd)
{
	RTK_A_DVBSX_STATUS status = RTK_DEMOD_SUCCESSFUL;
	REALTEK_A_DVBSX_INFO("[NextFreq] ScanSymbolMode: %d CheckTpMode: %d IsTPLock: %d\n", gDvbSxBsSpecialParams.u8IsScanSymbolMode, gDvbSxBsSpecialParams.u8IsCheckTPTunerDone, gDvbSxBsSpecialParams.u8IsTPLock);

	if(gDvbSxBsSpecialParams.u8IsCheckTPTunerDone)
	{
		if(gDvbSxBsSpecialParams.u8UnicableEnabled)
		{
			#ifdef BS_SCAN_DEBUG
			REALTEK_A_DVBSX_INFO("Unicable delay before RTK_A_DVBSX_BlindScanOneChannel\n");
			#endif
			tv_osal_msleep(60);
		}
		
		gDvbSxBsSpecialParams.u8IsTPLock = 0;
		gDvbSxBsLocalParams.u8NewChannelFound = 0;
		
		#ifdef BS_SCAN_DEBUG
		REALTEK_A_DVBSX_INFO("Check Index: %u/%u\n", gDvbSxScanParams.u32BlindScanCheckedSignalIndex, gScanedAllSymbols.u32ScanedSignalNum);
		REALTEK_A_DVBSX_INFO("u32TuneCenterFrequencyKHz: %d u32TuneCheckSymbolRateKHz: %d\n", gDvbSxBsLocalParams.u32TuneCenterFrequencyKHz, gDvbSxBsLocalParams.u32TuneCheckSymbolRateKHz);
		#endif
		if(RTK_A_DVBSX_NotDuplicateTpCheck(pDemod, 0, gDvbSxBsLocalParams.u32TuneCenterFrequencyKHz, gDvbSxBsLocalParams.u32TuneCheckSymbolRateKHz*1000))
		{
			#ifdef BS_SCAN_DEBUG
			REALTEK_A_DVBSX_INFO("RTK_A_DVBSX_BlindScanOneChannel\n");
			#endif
			status |= RTK_A_DVBSX_BlindScanOneChannel(pDemod, gDvbSxBsLocalParams.u32TuneCenterFrequencyKHz, gDvbSxBsLocalParams.u32TuneCheckSymbolRateKHz, &gDvbSxBsLocalParams.u8NewChannelFound);
		}

		REALTEK_A_DVBSX_INFO("\033[1;32;34m" "gDvbSxUniInverseFlag=%u DvbSxBlindScanState=%u [pSignalSpectrum:%u pCheckedSpectrum:%u] \n" "\033[m", gDvbSxUniInverseFlag, DvbSxBlindScanState, gDvbSxBsSpecialParams.pSignalSpectrum, gDvbSxBsSpecialParams.pCheckedSpectrum);
		if(gDvbSxBsSpecialParams.u8UnicableEnabled)
		{
			if(gDvbSxBsSpecialParams.u8BsLnbMode == 1) //C Band
			{
				if(gDvbSxBsSpecialParams.pSignalSpectrum == RTK_DVBSx_SIGNAL_SPECTRUM_INVERSE && gDvbSxBsSpecialParams.pCheckedSpectrum == RTK_DVBSx_SIGNAL_SPECTRUM_NORMAL && gDvbSxBsSpecialParams.u8IsSpectrumCheckDone == 1)
				{
					gDvbSxUniInverseFlag = 1;
					gDvbSxBsSpecialParams.pSignalSpectrum = RTK_DVBSx_SIGNAL_SPECTRUM_MANUAL_CHANGE;
					REALTEK_A_DVBSX_INFO("\033[1;32;34m" "C Band, Inverse spectrum & scan again\n" "\033[m");
					RTK_A_DVBSX_BlindScan_SelfInit(pDemod);
					RTK_A_DVBSX_SetDvBSxBlindScanParams(pDemod, gDvbSxScanParams.u32ScanStartFrequencyKHz, gDvbSxScanParams.u32ScanStopFrequencyKHz);

					goto BlindscanEndCheck;
				}
				else if(gDvbSxBsSpecialParams.pSignalSpectrum == RTK_DVBSx_SIGNAL_SPECTRUM_INVERSE && gDvbSxBsSpecialParams.pCheckedSpectrum == RTK_DVBSx_SIGNAL_SPECTRUM_INVERSE && gDvbSxBsSpecialParams.u8IsSpectrumCheckDone == 1)
				{
					gDvbSxBsSpecialParams.u8SpectrumModeCnt++;
					REALTEK_A_DVBSX_INFO("\033[1;32;34m" "C Band, u8SpectrumModeCnt %u\n" "\033[m", gDvbSxBsSpecialParams.u8SpectrumModeCnt);

					if(gDvbSxBsSpecialParams.u8SpectrumModeCnt == 2)
					{
						gDvbSxBsSpecialParams.pSignalSpectrum = RTK_DVBSx_SIGNAL_SPECTRUM_MANUAL_CHANGE;//spectrum mode is confirmed
						REALTEK_A_DVBSX_INFO("\033[1;32;34m" "C Band, confirmed, no need inverse\n" "\033[m");
					}
				}
				else
				{
					REALTEK_A_DVBSX_INFO("C Band, inverse flow done(other case)\n");
				}
			}
			else if(gDvbSxBsSpecialParams.u8BsLnbMode == 2) //Ku Band
			{
				if(gDvbSxBsSpecialParams.pSignalSpectrum == RTK_DVBSx_SIGNAL_SPECTRUM_INVERSE && gDvbSxBsSpecialParams.pCheckedSpectrum == RTK_DVBSx_SIGNAL_SPECTRUM_INVERSE && gDvbSxBsSpecialParams.u8IsSpectrumCheckDone == 1)
				{
					gDvbSxUniInverseFlag = 1;
					gDvbSxBsSpecialParams.pSignalSpectrum = RTK_DVBSx_SIGNAL_SPECTRUM_MANUAL_CHANGE;
					REALTEK_A_DVBSX_INFO("\033[1;32;36m" "KU Band, Inverse spectrum & scan again\n" "\033[m");
					RTK_A_DVBSX_BlindScan_SelfInit(pDemod);
					RTK_A_DVBSX_SetDvBSxBlindScanParams(pDemod, gDvbSxScanParams.u32ScanStartFrequencyKHz, gDvbSxScanParams.u32ScanStopFrequencyKHz);

					goto BlindscanEndCheck;
				}
				else if(gDvbSxBsSpecialParams.pSignalSpectrum == RTK_DVBSx_SIGNAL_SPECTRUM_INVERSE && gDvbSxBsSpecialParams.pCheckedSpectrum == RTK_DVBSx_SIGNAL_SPECTRUM_NORMAL && gDvbSxBsSpecialParams.u8IsSpectrumCheckDone == 1)
				{
					gDvbSxBsSpecialParams.u8SpectrumModeCnt++;
					REALTEK_A_DVBSX_INFO("\033[1;32;34m" "KU Band, u8SpectrumModeCnt %u\n" "\033[m", gDvbSxBsSpecialParams.u8SpectrumModeCnt);
					
					if(gDvbSxBsSpecialParams.u8SpectrumModeCnt == 2)
					{
						gDvbSxBsSpecialParams.pSignalSpectrum = RTK_DVBSx_SIGNAL_SPECTRUM_MANUAL_CHANGE;//spectrum mode is confirmed
						REALTEK_A_DVBSX_INFO("\033[1;32;34m" "KU Band, confirmed, no need inverse\n" "\033[m");
					}
				}
				else
				{
					REALTEK_A_DVBSX_INFO("KU Band, inverse flow done(other case)\n");
				}
			}
		}

		gDvbSxScanParams.u32BlindScanCheckedSignalIndex++; /*After checked one signal, index + 1.*/
		
		#if 1 //Fast Check Duplicate TP Enabled
		#ifdef BS_SCAN_DEBUG
		REALTEK_A_DVBSX_INFO("Next Check Index: %u/%u\n", gDvbSxScanParams.u32BlindScanCheckedSignalIndex, gScanedAllSymbols.u32ScanedSignalNum);
		#endif
		if(gDvbSxScanParams.u32BlindScanCheckedSignalIndex <= gScanedAllSymbols.u32ScanedSignalNum &&
			gScanedAllSymbols.ScanedSignalList[gDvbSxScanParams.u32BlindScanCheckedSignalIndex].u32SymbolRateKHz >= 5000)
		{
			unsigned char ChannelExist = 0;
			#ifdef BS_SCAN_DEBUG
			REALTEK_A_DVBSX_INFO("Special Verify Channels!!!!\n");
			REALTEK_A_DVBSX_INFO("Index: %u/%u Freq: %u SR: %u\n", 
				gDvbSxScanParams.u32BlindScanCheckedSignalIndex, gScanedAllSymbols.u32ScanedSignalNum, 
				gScanedAllSymbols.ScanedSignalList[gDvbSxScanParams.u32BlindScanCheckedSignalIndex].u32ScanedFreqKHz, 
				gScanedAllSymbols.ScanedSignalList[gDvbSxScanParams.u32BlindScanCheckedSignalIndex].u32SymbolRateKHz);
			#endif
			
			//status = RTK_A_DVBSX_BlindScanVerifyChannel(pDemod, gScanedAllSymbols.ScanedSignalList[gDvbSxScanParams.u32BlindScanCheckedSignalIndex].u32ScanedFreqKHz, &ChannelExist);
			if(ChannelExist)
			{
				#ifdef BS_SCAN_DEBUG
				REALTEK_A_DVBSX_INFO("ChannelExist!!!!\n");
				REALTEK_A_DVBSX_INFO("Check Index: %u/%u\n", gDvbSxScanParams.u32BlindScanCheckedSignalIndex, gScanedAllSymbols.u32ScanedSignalNum);
				#endif
				if(gDvbSxScanParams.u32BlindScanCheckedSignalIndex < gScanedAllSymbols.u32ScanedSignalNum)
				{
					#ifdef BS_SCAN_DEBUG
					REALTEK_A_DVBSX_INFO("u32BlindScanCheckedSignalIndex < u32ScanedSignalNum\n");
					#endif
					gDvbSxScanParams.u32BlindScanCheckedSignalIndex++;
				}
				#ifdef BS_SCAN_DEBUG
				REALTEK_A_DVBSX_INFO("Next Check Index: %u/%u\n", gDvbSxScanParams.u32BlindScanCheckedSignalIndex, gScanedAllSymbols.u32ScanedSignalNum);
				#endif
			}
		}
		#endif

		gDvbSxScanParams.u32CurrentTunedFreKHz = gDvbSxBsLocalParams.u32TuneCenterFrequencyKHz;/*used for process */

		#ifdef BS_SCAN_DEBUG
		REALTEK_A_DVBSX_INFO("gDvbSxScanParams.u32CurrentWorkFreKHz: %d\n", gDvbSxScanParams.u32CurrentTunedFreKHz);
		#endif
		/*Process if new channel found */ 
		if((gDvbSxBsLocalParams.u8NewChannelFound == 1) && (gDvbSxScanParams.u32ValidChannelNumber > 0) && (gDvbSxScanParams.u32ValidChannelNumber <= MAX_SCANED_CHANNEL_NUMBER))
		{
			gDvbSxBsSpecialParams.u8IsTPLock = 1;

			gDvbSxBsLocalParams.ValidChannlIndex = gDvbSxScanParams.u32ValidChannelNumber - 1;

			/*ValidChannlIndex is last locked signal's array index*/
			gDvbSxBsLocalParams.u32tmpSymbolrate = gDvbSxScanParams.DvbSxChannel[gDvbSxBsLocalParams.ValidChannlIndex].u32SymbolRateHz;

			//if(gDvbSxScanParams.DvbSxChannel[gDvbSxBsLocalParams.ValidChannlIndex].stDvbSxSignalPrams.DVBSxType == RTK_DVBS_CODE_RATE)
			//{
			//	gDvbSxBsLocalParams.u32NextSearchingfreq = gDvbSxScanParams.DvbSxChannel[gDvbSxBsLocalParams.ValidChannlIndex].u32FrequencyKHz + (gDvbSxBsLocalParams.u32tmpSymbolrate*27)/40000 + 10000; //roll off *1.35 for DVB-S
			//}
			//else
			//{
			//	gDvbSxBsLocalParams.u32NextSearchingfreq = gDvbSxScanParams.DvbSxChannel[gDvbSxBsLocalParams.ValidChannlIndex].u32FrequencyKHz + (gDvbSxBsLocalParams.u32tmpSymbolrate*23)/40000 + 10000; //roll off *1.15
			//}
			gDvbSxBsLocalParams.u32NextSearchingfreq = gDvbSxScanParams.DvbSxChannel[gDvbSxBsLocalParams.ValidChannlIndex].u32FrequencyKHz + (gDvbSxBsLocalParams.u32tmpSymbolrate*23)/40000 + 10000; //roll off *1.15

			gDvbSxScanParams.u32MaxSignalRightRangeFreqKHz = gDvbSxBsLocalParams.u32NextSearchingfreq; 

			//gDvbSxScanParams.u32CurrentWorkFreKHz = u32ScanCenterFrequencyKHz;/*used for process */

		} /*End if found new channel*/

		/*Scan next channel */
		gDvbSxScanParams.ui32ScanedValidChannelNum = gDvbSxScanParams.u32ValidChannelNumber;

		/*Tune frequency may be smaller or bigger than blind scan frequency. Display aways select the bigger value.*/
		if(gDvbSxScanParams.u32CurrentTunedFreKHz > gDvbSxScanParams.u32CurrentWorkFreKHzProcessed) //For display
		{
			gDvbSxScanParams.u32CurrentWorkFreKHzProcessed = gDvbSxScanParams.u32CurrentTunedFreKHz;
		}

		#ifdef BS_SCAN_DEBUG
		REALTEK_A_DVBSX_INFO("u32CurrentSymbolScanFreKHz: %d u32StopFrequency: %d u32BlindScanCheckedSignalIndex: %d u32ScanedSignalNum: %d DvbSxBlindScanState:%d\n",
			gDvbSxScanParams.u32CurrentSymbolScanFreKHz, gDvbSxBsLocalParams.u32StopFrequency,
			gDvbSxScanParams.u32BlindScanCheckedSignalIndex, gScanedAllSymbols.u32ScanedSignalNum,
			DvbSxBlindScanState);
		#endif
	}
	else //scan mode
	{
		if(gDvbSxScanParams.u32BlindScanCheckedSignalIndex >= gScanedAllSymbols.u32ScanedSignalNum)
		{
			//Scan on new frequency for all signals with reception range and save new scaned signals.
			status = RTK_A_DVBSX_SetDvbSxMode(pDemod, u32DefaultStandard, 45000000, DUMMY_FREQ_KHZ);
			if(gDvbSxBsSpecialParams.u8UnicableEnabled)
			{
				if(gDvbSxUniInverseFlag == 0)
					RTK_A_DVBSX_SetSpectrumManual(pDemod, RTK_DVBSx_SIGNAL_SPECTRUM_INVERSE);
				else
					RTK_A_DVBSX_SetSpectrumManual(pDemod, RTK_DVBSx_SIGNAL_SPECTRUM_NORMAL);
			}
			RTK_A_DVBSX_ScanSignals(pDemod, gDvbSxScanParams.u32CurrentSymbolScanFreKHz, &gDvbSxBsSpecialParams.u8NoSignalInSpectrum);
			#ifdef BS_SCAN_DEBUG
			REALTEK_A_DVBSX_INFO("Scan symbols done!, u32CurrentSymbolScanFreKHz: %d, u8NoSignalInSpectrum: %u\n", gDvbSxScanParams.u32CurrentSymbolScanFreKHz, gDvbSxBsSpecialParams.u8NoSignalInSpectrum);
			#endif
		}
	}

	//Have new found signals which require re-tune and check
	if(gDvbSxScanParams.u32BlindScanCheckedSignalIndex < gScanedAllSymbols.u32ScanedSignalNum)
	{
		gDvbSxBsLocalParams.u32TuneCenterFrequencyKHz = gScanedAllSymbols.ScanedSignalList[gDvbSxScanParams.u32BlindScanCheckedSignalIndex].u32ScanedFreqKHz;
		gDvbSxBsLocalParams.u32TuneCheckSymbolRateKHz = gScanedAllSymbols.ScanedSignalList[gDvbSxScanParams.u32BlindScanCheckedSignalIndex].u32SymbolRateKHz;

		gDvbSxBsSpecialParams.u8IsScanSymbolMode = 0;
		gDvbSxBsSpecialParams.u8IsCheckTPTunerDone = 1;//Check TP

		goto BlindscanEndCheck;
	}
	else
	{
		#ifdef BS_SCAN_DEBUG
		REALTEK_A_DVBSX_INFO("All symbol has been checked! Do next freq scan!\n");
		#endif
		if((gDvbSxScanParams.u32CurrentWorkFreKHzProcessed < gDvbSxScanParams.u32CurrentSymbolScanFreKHz) && (gScanedParams.u8ScanedSymbolNum == 0))
		{
			gDvbSxScanParams.u32CurrentWorkFreKHzProcessed = gDvbSxScanParams.u32CurrentSymbolScanFreKHz;
		}

		gDvbSxBsSpecialParams.u8IsScanSymbolMode = 1;//Scan new symbols
		gDvbSxBsSpecialParams.u8IsCheckTPTunerDone = 0;

		goto BlindscanEndCheck;
	}

BlindscanEndCheck:

	if(
		(gDvbSxScanParams.u32CurrentSymbolScanFreKHz >= gDvbSxBsLocalParams.u32StopFrequency + 10000) ||
		(gDvbSxScanParams.u32BlindScanCheckedSignalIndex > gScanedAllSymbols.u32ScanedSignalNum)
	)
	{
		*bBlindScanEnd = 1;
	}
	else
	{
		*bBlindScanEnd = 0;
	}

	return status;
}


RTK_A_DVBSX_STATUS RTK_A_DVBSX_BlindScan_GetStatus(DVBSX_DEMOD_MODULE *pDemod, unsigned char *Status, unsigned int *bBlindScanLock)
{
	RTK_A_DVBSX_STATUS status = RTK_DEMOD_SUCCESSFUL;

	*Status = 0;
	*bBlindScanLock = gDvbSxBsSpecialParams.u8IsTPLock;

	return status;
}


RTK_A_DVBSX_STATUS RTK_A_DVBSX_BlindScan_GetFoundTP(DVBSX_DEMOD_MODULE *pDemod, unsigned int *FreqMhz, unsigned int *SymbolRateKhz)
{
	RTK_A_DVBSX_STATUS status = RTK_DEMOD_SUCCESSFUL;

	*FreqMhz = gDvbSxScanParams.DvbSxChannel[gDvbSxScanParams.u32ValidChannelNumber - 1].u32FrequencyKHz/1000;
	*SymbolRateKhz = gDvbSxScanParams.DvbSxChannel[gDvbSxScanParams.u32ValidChannelNumber - 1].u32SymbolRateHz/1000;
	gDvbSxScanParams.GetFoundChannel[gDvbSxScanParams.u32GetFoundChannelNumber].u32FrequencyKHz = gDvbSxScanParams.DvbSxChannel[gDvbSxScanParams.u32ValidChannelNumber - 1].u32FrequencyKHz;
	gDvbSxScanParams.GetFoundChannel[gDvbSxScanParams.u32GetFoundChannelNumber].u32SymbolRateHz = gDvbSxScanParams.DvbSxChannel[gDvbSxScanParams.u32ValidChannelNumber - 1].u32SymbolRateHz;
	gDvbSxScanParams.u32GetFoundChannelNumber++;

	return status;
}


RTK_A_DVBSX_STATUS RTK_A_DVBSX_BlindScan_End(DVBSX_DEMOD_MODULE *pDemod)
{
	RTK_A_DVBSX_STATUS status = RTK_DEMOD_SUCCESSFUL;
	unsigned int u8i = 0;

	gDvbSxBsSpecialParams.u8UnicableEnabled = 0;
	for(u8i=0; u8i<gDvbSxScanParams.u32GetFoundChannelNumber; u8i++)
	{
		REALTEK_A_DVBSX_INFO("(%02u)\tGetFoundTP\t%04u\t%u\n", u8i+1, gDvbSxScanParams.GetFoundChannel[u8i].u32FrequencyKHz/1000, gDvbSxScanParams.GetFoundChannel[u8i].u32SymbolRateHz/1000);
	}

	return status;
}


/*Set Blind scan minimal symbol rate and maximum symbol rate, typically 2M-45M (2000000Hz--45000000Hz)
Called after RTK_A_DVBSX_DvBSxBlindScanInit to change default parameters.
*/
void RTK_A_DVBSX_SetBlindScanParamsSymbolRange(DVBSX_DEMOD_MODULE *pDemod, unsigned int ui32MinSymbolRateHz, unsigned int ui32MaxSymbolRateHz)
{
	gDvbSxScanParams.u32MinSymbolRateHz = ui32MinSymbolRateHz;
	gDvbSxScanParams.u32MaxSymbolRateHz = ui32MaxSymbolRateHz;
}


/*Internal function used for small symbol rate lock checking, may takes more than 1000ms to be locked.*/
RTK_A_DVBSX_STATUS RTK_A_DVBSX_BlindScanPreCheckSignal(DVBSX_DEMOD_MODULE *pDemod, unsigned int u32SymbolRateHz, unsigned char *u8FecLockFlag, unsigned char *u8SignalValidFlag)
{
	/*The checked signal is symbol detected signal, so here DVBSxSynLockFlag will most get locked.*/
	RTK_A_DVBSX_STATUS status = RTK_DEMOD_SUCCESSFUL;
	unsigned char u8iter = 0;
	stRTK_LOCKING_STATUS sRTKLockingStatus;
	*u8FecLockFlag = 0;
	*u8SignalValidFlag = 0;

	if(u32SymbolRateHz < 1200000) //Small symbol rate like 1M take more time to sync.
	{
		tv_osal_msleep(100);
	}
	//Check maximum 500ms for symbol rate < 5M
	for(u8iter = 0; u8iter < 10; u8iter++)
	{
		status |= RTK_A_DVBSX_GetLockingStatus(pDemod, &sRTKLockingStatus);

		if(sRTKLockingStatus.u8TimingLockFlag) /*If never sync will quit after 200ms*/
		{
			//*u8SignalValidFlag = 1;
			*u8FecLockFlag = 1;
			*u8SignalValidFlag = 1;
			REALTEK_A_DVBSX_INFO("RTK_A_DVBSX_BlindScanPreCheckSignal locked\n");
			return status;
		}
		#if 0
		if(sRTKLockingStatus.u8FecLockFlag) /*Signal locked*/
		{
			*u8FecLockFlag = 1;
			*u8SignalValidFlag = 1;
			REALTEK_A_DVBSX_INFO("RTK_A_DVBSX_BlindScanPreCheckSignal locked\n");
			return status;
		}
		#endif
		tv_osal_msleep(50);
	}
	return status;
}


/****************************************************************************
Function   : RTK_A_DVBSX_BlindScanOneChannel
Parameters : u32CenterFrequency, pNewChannel:1 indicate new channel locked.
Return     : 0 is Success, Other is Failed
Description:
This function is internal function.
****************************************************************************/
RTK_A_DVBSX_STATUS RTK_A_DVBSX_BlindScanOneChannel(DVBSX_DEMOD_MODULE *pDemod, unsigned int u32CenterFrequencyKHz, unsigned int u32SymbolRateKHz, unsigned char *pNewChannel)
{
	RTK_A_DVBSX_STATUS status = RTK_DEMOD_SUCCESSFUL;
	int i32CFOKHz = 0;
	unsigned int u32SymbolrateHz = 0;
	unsigned char ChannelExist = 0;
	unsigned char u8DemodLockedFlag = 0;
	unsigned char ReCheckCnt = 0;
	int i32LockedCFO = 0;
	int i32TmpCenterFreqKHz = 0;
	*pNewChannel = 0;

	gDvbSxScanParams.u8SingalInLockingState = 0; /*default */
	gDvbSxScanParams.u8SignalRequiredCheckTimes = 0;

	/*Check whether current center frequency were in scanning frequency range.*/
	if(gDvbSxScanParams.u32BlindScanCheckedSignalIndex >= gScanedAllSymbols.u32ScanedSignalNum)
	{
		return status;
	}

	//Tune frequency is out of scan range.
	if(((u32CenterFrequencyKHz + 6000) < gDvbSxScanParams.u32ScanStartFrequencyKHz)||(u32CenterFrequencyKHz > (gDvbSxScanParams.u32ScanStopFrequencyKHz + 6000)))
	{
		status = RTK_DEMOD_FAILED;
		REALTEK_A_DVBSX_INFO("Freq is out of scan range(start: %d end: %d center: %d)\n", gDvbSxScanParams.u32ScanStartFrequencyKHz, gDvbSxScanParams.u32ScanStopFrequencyKHz, u32CenterFrequencyKHz);
		return status;
	}

	if(u32SymbolRateKHz >= 5000)
	{
		#ifdef BS_SCAN_DEBUG
		REALTEK_A_DVBSX_INFO("Verify Channels!\n");
		#endif
		//status = RTK_A_DVBSX_BlindScanVerifyChannel(pDemod, u32CenterFrequencyKHz, &ChannelExist);
	}
	else
	{
		#ifdef BS_SCAN_DEBUG
		REALTEK_A_DVBSX_INFO("NOT Verify Channels!\n");
		#endif
		ChannelExist = 0;
	}

	if(ChannelExist) /*Current searched frequency is within locked signal*/
	{
		status = RTK_DEMOD_SUCCESSFUL;
		return status;
	}

re_check_signal:
	RTK_A_DVBSX_SetDvbSxMode(pDemod, u32DefaultStandard, u32SymbolRateKHz*1000, DUMMY_FREQ_KHZ);
	if(gDvbSxBsSpecialParams.u8UnicableEnabled)
	{
		if(gDvbSxUniInverseFlag == 0)
			RTK_A_DVBSX_SetSpectrumManual(pDemod, RTK_DVBSx_SIGNAL_SPECTRUM_INVERSE);
		else
			RTK_A_DVBSX_SetSpectrumManual(pDemod, RTK_DVBSx_SIGNAL_SPECTRUM_NORMAL);
	}
	tv_osal_msleep(50); //add 50ms
	
	/*Check Tuner is locked*/
	if((status == RTK_DEMOD_SUCCESSFUL) && (ChannelExist != 1))
	{
		status |= RTK_A_DVBSX_BlindScanCheckSymbolRate(pDemod, &u32SymbolrateHz);
		status |= RTK_A_DVBSX_GetDVBSxCoarseCFO(pDemod, &i32CFOKHz);
		#ifdef BS_SCAN_DEBUG
		REALTEK_A_DVBSX_INFO("SymbolrateHz: %d\n", u32SymbolrateHz);
		REALTEK_A_DVBSX_INFO("CoarseCFO: %d\n", i32CFOKHz);
		#endif
		if(gDvbSxBsSpecialParams.u8UnicableEnabled == 0 && u32SymbolrateHz < 6000000 && ((i32CFOKHz > 5500)||(i32CFOKHz < -5500)) && ReCheckCnt < 3)
		{
			ReCheckCnt++;
			REALTEK_A_DVBSX_INFO("SR < 6M & ABS(CFO) > 5.5M, Re-Check Signal, i32CFOKHz = %d #(%u)\n", i32CFOKHz, ReCheckCnt);
			goto re_check_signal;
		}

		//if(gDvbSxBsSpecialParams.u8UnicableEnabled)
		//{
		//	if(gDvbSxBsSpecialParams.u8BsLnbMode == 1) //C Band
		//	{
		//		if(gDvbSxBsSpecialParams.pSignalSpectrum == RTK_DVBSx_SIGNAL_SPECTRUM_NORMAL)
		//		{
		//			i32CFOKHz = 0 - i32CFOKHz;
		//			REALTEK_A_DVBSX_INFO("C Band, inversed CoarseCFO: %d\n", i32CFOKHz);
		//		}
		//		else//if(gDvbSxBsSpecialParams.pSignalSpectrum == RTK_DVBSx_SIGNAL_SPECTRUM_INVERSE || gDvbSxBsSpecialParams.pSignalSpectrum == RTK_DVBSx_SIGNAL_SPECTRUM_UNKNOWN)
		//		{
		//			REALTEK_A_DVBSX_INFO("C Band, CoarseCFO doesn't need inverse\n");
		//		}
		//	}
		//	else if(gDvbSxBsSpecialParams.u8BsLnbMode == 2) //Ku Band
		//	{
		//		if(gDvbSxBsSpecialParams.pSignalSpectrum == RTK_DVBSx_SIGNAL_SPECTRUM_INVERSE)
		//		{
		//			i32CFOKHz = 0 - i32CFOKHz;
		//			REALTEK_A_DVBSX_INFO("KU Band, inversed CoarseCFO: %d\n", i32CFOKHz);
		//		}
		//		else//if(gDvbSxBsSpecialParams.pSignalSpectrum == RTK_DVBSx_SIGNAL_SPECTRUM_NORMAL || gDvbSxBsSpecialParams.pSignalSpectrum == RTK_DVBSx_SIGNAL_SPECTRUM_UNKNOWN)
		//		{
		//			REALTEK_A_DVBSX_INFO("KU Band, CoarseCFO doesn't need inverse\n");
		//		}
		//	}
		//}

		gDvbSxScanParams.u32ScanPreCheckedFreqKHz = u32CenterFrequencyKHz;

		REALTEK_A_DVBSX_INFO("Scan Tune: FreqKHz: %d SymbolRate: %d CoarseCFO: %d\n", u32CenterFrequencyKHz, u32SymbolrateHz, i32CFOKHz);

		gDvbSxScanParams.u32PreTunedSymbolRateHz = u32SymbolrateHz;

		if(gDvbSxBsSpecialParams.u8UnicableEnabled && u32SymbolrateHz > MAX_UNICABLE_BS_SYMBOL_RATE_KHZ*1000)
		{
			gDvbSxScanParams.u32ScanPreCheckedFreqKHz = u32CenterFrequencyKHz; /*Record the checked frequency.*/
			REALTEK_A_DVBSX_INFO("Unicable Symbol higher than max value limited will not be received!\n");
			u8DemodLockedFlag = 0;
			return status;
		}
		else if(u32SymbolrateHz > 990000) //Symbol rate > 0.99M, will lock within one check cycle.
		{
			status |= RTK_A_DVBSX_BlindscanChannelLockCheck(pDemod, &u8DemodLockedFlag);
		}
		else
		{
			REALTEK_A_DVBSX_INFO("SymbolRate Over Range, Demodulator NOT Check Lock!\n");
			u8DemodLockedFlag = 0;
		}

		gDvbSxScanParams.u32ScanPreCheckedFreqKHz = u32CenterFrequencyKHz; /*Record the checked frequency.*/
		REALTEK_A_DVBSX_INFO("Step2:Freq: %d Symbol rate: %d CFO: %d u8DemodLockedFlag: %d\n", u32CenterFrequencyKHz, u32SymbolrateHz, i32CFOKHz, u8DemodLockedFlag);
		if(u8DemodLockedFlag)
		{
			status |= RTK_A_DVBSX_GetDVBSxLockedCFO(pDemod, &i32LockedCFO);

			//if(gDvbSxBsSpecialParams.u8UnicableEnabled)
			//{
			//	if(gDvbSxBsSpecialParams.u8BsLnbMode == 1) //C Band
			//	{
			//		if(gDvbSxBsSpecialParams.pSignalSpectrum == RTK_DVBSx_SIGNAL_SPECTRUM_NORMAL)
			//		{
			//			i32LockedCFO = 0 - i32LockedCFO;
			//			REALTEK_A_DVBSX_INFO("C Band, inversed LockedCFO: %d\n", i32LockedCFO);
			//		}
			//		else//if(gDvbSxBsSpecialParams.pSignalSpectrum == RTK_DVBSx_SIGNAL_SPECTRUM_INVERSE || gDvbSxBsSpecialParams.pSignalSpectrum == RTK_DVBSx_SIGNAL_SPECTRUM_UNKNOWN)
			//		{
			//			REALTEK_A_DVBSX_INFO("C Band, LockedCFO doesn't need inverse\n");
			//		}
			//	}
			//	else if(gDvbSxBsSpecialParams.u8BsLnbMode == 2) //Ku Band
			//	{
			//		if(gDvbSxBsSpecialParams.pSignalSpectrum == RTK_DVBSx_SIGNAL_SPECTRUM_INVERSE)
			//		{
			//			i32LockedCFO = 0 - i32LockedCFO;
			//			REALTEK_A_DVBSX_INFO("KU Band, inversed LockedCFO: %d\n", i32LockedCFO);
			//		}
			//		else//if(gDvbSxBsSpecialParams.pSignalSpectrum == RTK_DVBSx_SIGNAL_SPECTRUM_NORMAL || gDvbSxBsSpecialParams.pSignalSpectrum == RTK_DVBSx_SIGNAL_SPECTRUM_UNKNOWN)
			//		{
			//			REALTEK_A_DVBSX_INFO("KU Band, LockedCFO doesn't need inverse\n");
			//		}
			//	}
			//}

			status |= RTK_A_DVBSX_GetDVBSxSymbolRate(pDemod, &u32SymbolrateHz);

			/*Calibrate tuned frequency by locked CFO*/
			i32TmpCenterFreqKHz = ((u32CenterFrequencyKHz+500)/1000)*1000;
			if(i32LockedCFO == 0)
			{
				if(gDvbSxBsSpecialParams.u8UnicableEnabled)
				{
					tv_osal_msleep(60);
					status |= RTK_A_DVBSX_GetDVBSxLockedCFO(pDemod, &i32LockedCFO);
					if(i32LockedCFO == 0)
					{
						i32TmpCenterFreqKHz = i32TmpCenterFreqKHz + i32CFOKHz;
						REALTEK_A_DVBSX_INFO("UniBs Re-get CFO, Use CoarseCFO for CFO LockedCFO: %d CoarseCFO: %d\n", i32LockedCFO, i32CFOKHz);
					}
					else
					{
						i32TmpCenterFreqKHz = i32TmpCenterFreqKHz + i32LockedCFO;
						REALTEK_A_DVBSX_INFO("UniBs Re-get CFO, Use LockedCFO for CFO LockedCFO: %d CoarseCFO: %d\n", i32LockedCFO, i32CFOKHz);
					}
				}
				else
				{
					i32TmpCenterFreqKHz = i32TmpCenterFreqKHz + i32CFOKHz;
					REALTEK_A_DVBSX_INFO("Use CoarseCFO for CFO LockedCFO: %d CoarseCFO: %d\n", i32LockedCFO, i32CFOKHz);
				}
			}
			else
			{
				i32TmpCenterFreqKHz = i32TmpCenterFreqKHz + i32LockedCFO;
				REALTEK_A_DVBSX_INFO("Use LockedCFO for CFO LockedCFO: %d CoarseCFO: %d\n", i32LockedCFO, i32CFOKHz);
			}

			u32CenterFrequencyKHz = (unsigned int)i32TmpCenterFreqKHz;
		}
		else
		{
			status |= RTK_A_DVBSX_GetDVBSxCoarseCFO(pDemod, &i32CFOKHz);
			status |= RTK_A_DVBSX_GetDVBSxSymbolRate(pDemod, &u32SymbolrateHz);
			REALTEK_A_DVBSX_INFO("Step3:Result of TuneAgain Freq: %d Symbol rate: %d CoarseCFO: %d u8DemodLockedFlag: %d\n", u32CenterFrequencyKHz, u32SymbolrateHz, i32CFOKHz, u8DemodLockedFlag);
		}
	}
	else
	{
		REALTEK_A_DVBSX_INFO("ChannelExist: %d!\n", ChannelExist);
		/*Have saved the same channel*/
	}
	
	if(u32SymbolrateHz >= 5000000)
	{
		#ifdef BS_SCAN_DEBUG
		REALTEK_A_DVBSX_INFO("Verify Channels!\n");
		#endif
		//status = RTK_A_DVBSX_BlindScanVerifyChannel(pDemod, u32CenterFrequencyKHz, &ChannelExist);
	}
	else
	{
		#ifdef BS_SCAN_DEBUG
		REALTEK_A_DVBSX_INFO("NOT Verify Channels!\n");
		#endif
		ChannelExist = 0;
	}

	if((status == RTK_DEMOD_SUCCESSFUL) && (ChannelExist != 1) && (u8DemodLockedFlag) && (RTK_A_DVBSX_NotDuplicateTpCheck(pDemod, 1, u32CenterFrequencyKHz, u32SymbolrateHz))) /*Save a new channel*/
	{
		gDvbSxScanParams.DvbSxChannel[gDvbSxScanParams.u32ValidChannelNumber].u32FrequencyKHz = (u32CenterFrequencyKHz+500)/1000*1000;
		gDvbSxScanParams.DvbSxChannel[gDvbSxScanParams.u32ValidChannelNumber].u32SymbolRateHz = u32SymbolrateHz;
		//gDvbSxScanParams.DvbSxChannel[gDvbSxScanParams.u32ValidChannelNumber].stDvbSxSignalPrams.DVBSxSymbolRateHz = u32SymbolrateHz;
		gDvbSxScanParams.u32ValidChannelNumber = gDvbSxScanParams.u32ValidChannelNumber + 1;
		*pNewChannel = 1;
		REALTEK_A_DVBSX_INFO("Scan Save channel:FinalFreq: %d i32LockedCFO: %d Symbol rate: %d u32ValidChannelNumber: %d\n", u32CenterFrequencyKHz, i32LockedCFO, u32SymbolrateHz, gDvbSxScanParams.u32ValidChannelNumber);
	}

	return status;
} 


RTK_A_DVBSX_STATUS RTK_A_DVBSX_ScanSignals(DVBSX_DEMOD_MODULE *pDemod, unsigned int u32ScanFreKHz, unsigned char *u32LowPwrNoSig)
{
	RTK_A_DVBSX_STATUS status = RTK_DEMOD_SUCCESSFUL;
	unsigned int i = 0;
	
	status = RTK_A_DVBSX_SetDvBSxSymbolScan(pDemod, u32LowPwrNoSig);
	
	//No Signal Check Here
	//if(*u32LowPwrNoSig == 1)
		//return status;
	
	status |= RTK_A_DVBSX_GetScanedSymbols(pDemod);
	RTK_A_DVBSX_AddChannel(pDemod, u32ScanFreKHz);
	RTK_A_DVBSX_SortScanValidSymbolRateFreq(pDemod);

	//Retry Scan 1 Times
	if(!gDvbSxBsSpecialParams.u8UnicableEnabled)
	{
		if(gScanedParams.u8ScanedSymbolNum > 0)
		{
			RTK_A_DVBSX_HoldDSP(pDemod);
			RTK_A_DVBSX_StartDSP(pDemod);
			RTK_A_DVBSX_GetScanedSymbols(pDemod);
			RTK_A_DVBSX_AddChannel(pDemod, u32ScanFreKHz);
			RTK_A_DVBSX_SortScanValidSymbolRateFreq(pDemod);
		}
	}
	
	if(gDvbSxBsSpecialParams.u8UnicableEnabled)//Clear each UB's result in unicable blindscan
	{
		#ifdef BS_SCAN_DEBUG
		REALTEK_A_DVBSX_INFO("Clear each UB's result in unicable blindscan\n");
		#endif
		for(i = 0; i < MAX_SCANED_SYMBOL_NUM; i++)
		{
			gScanedParams.ScanedSymbols[i].i32ScanedCFO = 0;
			gScanedParams.ScanedSymbols[i].u32SymbolRateKHz = 0;
			gScanedParams.ScanedSymbols[i].u32CoarseSymbolRateKHz = 0;
			gScanedParams.ScanedSymbols[i].u32absCFO = 0;
		}
	}
	
	return status;
}


unsigned int RTK_A_DVBSX_NotDuplicateTpCheck(DVBSX_DEMOD_MODULE *pDemod, unsigned int mode, unsigned int check_freq, unsigned int check_sr)
{
	unsigned int i = 0;
	
	REALTEK_A_DVBSX_INFO("RTK_A_DVBSX_NotDuplicateTpCheck  Freq=%u SR=%u ===\n", check_freq, check_sr);
	
	//if(mode == 0)//Coarse SR Mode
	//{	
		for(i=0; i<gDvbSxScanParams.u32GetFoundChannelNumber; i++)
		{
			#ifdef BS_SCAN_DEBUG
			REALTEK_A_DVBSX_INFO("Compare Freq=%u SR=%u\n", gDvbSxScanParams.GetFoundChannel[i].u32FrequencyKHz, gDvbSxScanParams.GetFoundChannel[i].u32SymbolRateHz);
			#endif
			if(check_sr <= 10000000)
			{
				if(gDvbSxBsSpecialParams.u8UnicableEnabled)
				{
					if(check_freq <= gDvbSxScanParams.GetFoundChannel[i].u32FrequencyKHz+3500 && check_freq >= gDvbSxScanParams.GetFoundChannel[i].u32FrequencyKHz-3500 &&
					check_sr <= gDvbSxScanParams.GetFoundChannel[i].u32SymbolRateHz+100000 && check_sr >= gDvbSxScanParams.GetFoundChannel[i].u32SymbolRateHz-100000)
				{
						REALTEK_A_DVBSX_INFO("Unicable BS Low Duplicate Tp\n");
					return 0;
				}
			}
			else
			{
				if(check_freq <= gDvbSxScanParams.GetFoundChannel[i].u32FrequencyKHz+2500 && check_freq >= gDvbSxScanParams.GetFoundChannel[i].u32FrequencyKHz-2500 &&
						check_sr <= gDvbSxScanParams.GetFoundChannel[i].u32SymbolRateHz+100000 && check_sr >= gDvbSxScanParams.GetFoundChannel[i].u32SymbolRateHz-100000)
					{
						REALTEK_A_DVBSX_INFO("Normal BS Low Duplicate Tp\n");
						return 0;
					}
				}
			}
			else
			{
				if(gDvbSxBsSpecialParams.u8UnicableEnabled)
				{
					if(check_freq <= gDvbSxScanParams.GetFoundChannel[i].u32FrequencyKHz+5500 && check_freq >= gDvbSxScanParams.GetFoundChannel[i].u32FrequencyKHz-5500 &&
					check_sr <= gDvbSxScanParams.GetFoundChannel[i].u32SymbolRateHz+300000 && check_sr >= gDvbSxScanParams.GetFoundChannel[i].u32SymbolRateHz-300000)
				{
						REALTEK_A_DVBSX_INFO("Unicable BS High Duplicate Tp\n");
					return 0;
				}
			}
				else
				{
					if(check_freq <= gDvbSxScanParams.GetFoundChannel[i].u32FrequencyKHz+4500 && check_freq >= gDvbSxScanParams.GetFoundChannel[i].u32FrequencyKHz-4500 &&
						check_sr <= gDvbSxScanParams.GetFoundChannel[i].u32SymbolRateHz+300000 && check_sr >= gDvbSxScanParams.GetFoundChannel[i].u32SymbolRateHz-300000)
					{
						REALTEK_A_DVBSX_INFO("Normal BS High Duplicate Tp\n");
						return 0;
					}
				}
			}
		}
	//}
	//else //Fine SR Mode
	//{
	//
	//}
	
	REALTEK_A_DVBSX_INFO("NOT Duplicate Tp\n");
	return 1;
}


unsigned char RTK_A_DVBSX_BlindScanCompletedFlag(DVBSX_DEMOD_MODULE *pDemod)
{
	unsigned char u8ScanCompletedFlag = 0;
	if((DvbSxBlindScanState == 0)||(gDvbSxScanParams.u32CurrentTunedFreKHz >= gDvbSxScanParams.u32ScanStopFrequencyKHz))
	{
		u8ScanCompletedFlag = 1;
	}
	return u8ScanCompletedFlag;
}


/*Stop the Blind scan process*/
void RTK_A_DVBSX_BlindScanStop(DVBSX_DEMOD_MODULE *pDemod)
{
	DvbSxBlindScanState = 0;
	DvbSxBlindScanMode = 0;
	gDvbSxUniInverseFlag = 0;
}


/*Sample of blind scan completed ration, 0 -100*/
unsigned char RTK_A_DVBSX_GetBlindScanProcessRatio(DVBSX_DEMOD_MODULE *pDemod)
{
	unsigned int ui32CurrentScanedFreq = 0;
	unsigned int ui32TotalFre = 0;
	unsigned char u8ratio = 0;
	if(gDvbSxScanParams.u32CurrentWorkFreKHzProcessed > gDvbSxScanParams.u32ScanStartFrequencyKHz)
	{
		ui32CurrentScanedFreq = gDvbSxScanParams.u32CurrentWorkFreKHzProcessed - gDvbSxScanParams.u32ScanStartFrequencyKHz;
	}
	else
	{
		ui32CurrentScanedFreq = 0;
	}
	ui32TotalFre = gDvbSxScanParams.u32ScanStopFrequencyKHz - gDvbSxScanParams.u32ScanStartFrequencyKHz;
	if(ui32TotalFre ==0)
	{
		ui32TotalFre = ui32CurrentScanedFreq = 1;
		return 0;
	}
	REALTEK_A_DVBSX_INFO("process Ratio %d %d\n", ui32CurrentScanedFreq, ui32TotalFre);
	u8ratio = (unsigned char)(ui32CurrentScanedFreq*100/ui32TotalFre);
	if(u8ratio > 100)
	{
		u8ratio = 100;
	}
	return u8ratio;
}


/*Get BlindScan lock transponders number, 0 -100*/
unsigned int RTK_A_DVBSX_GetScanLockChannelNumber(DVBSX_DEMOD_MODULE *pDemod)
{
	return gDvbSxScanParams.ui32ScanedValidChannelNum;
}


/*Get blind scan result based on transponder channel index*/
#if 0 //Remove not using code
void RTK_A_DVBSX_GetBlindScanResult(DVBSX_DEMOD_MODULE *pDemod, stDvbSxChannel *pDvbSxChannelParams, unsigned int u32TranspoderChannelIndex)
{
	/*Set as default value*/
	pDvbSxChannelParams->u32FrequencyKHz = 950000;
	pDvbSxChannelParams->u32SymbolRateHz = 450000000;
	//pDvbSxChannelParams->stDvbSxSignalPrams.DVBSxType = RTK_DEMOD_DVB_UNKNOWN;
	//pDvbSxChannelParams->stDvbSxSignalPrams.DVBSxModu = RTK_DVBSx_MODU_UNKNOWN;
	//pDvbSxChannelParams->stDvbSxSignalPrams.DVBSCodeRate = RTK_DVBS_CODE_RATE_UNKNOWN;
	//pDvbSxChannelParams->stDvbSxSignalPrams.DVBS2CodeRate = RTK_DVBS2_CODE_RATE_UNKNOWN;
	//pDvbSxChannelParams->stDvbSxSignalPrams.DVBS2XCodeRate = RTK_DVBS2X_CODE_RATE_UNKNOWN;

	if((u32TranspoderChannelIndex > gDvbSxScanParams.u32ValidChannelNumber)||(u32TranspoderChannelIndex > MAX_SCANED_CHANNEL_NUMBER))
	{
		return;
	}
	else
	{
		pDvbSxChannelParams->u32FrequencyKHz = gDvbSxScanParams.DvbSxChannel[u32TranspoderChannelIndex].u32FrequencyKHz;
		pDvbSxChannelParams->u32SymbolRateHz = gDvbSxScanParams.DvbSxChannel[u32TranspoderChannelIndex].u32SymbolRateHz;

		//pDvbSxChannelParams->stDvbSxSignalPrams.DVBSxType = gDvbSxScanParams.DvbSxChannel[u32TranspoderChannelIndex].stDvbSxSignalPrams.DVBSxType;
		//pDvbSxChannelParams->stDvbSxSignalPrams.DVBSxModu = gDvbSxScanParams.DvbSxChannel[u32TranspoderChannelIndex].stDvbSxSignalPrams.DVBSxModu;
		//pDvbSxChannelParams->stDvbSxSignalPrams.DVBSCodeRate = gDvbSxScanParams.DvbSxChannel[u32TranspoderChannelIndex].stDvbSxSignalPrams.DVBSCodeRate;
		//pDvbSxChannelParams->stDvbSxSignalPrams.DVBS2CodeRate = gDvbSxScanParams.DvbSxChannel[u32TranspoderChannelIndex].stDvbSxSignalPrams.DVBS2CodeRate;
		//pDvbSxChannelParams->stDvbSxSignalPrams.DVBS2XCodeRate = gDvbSxScanParams.DvbSxChannel[u32TranspoderChannelIndex].stDvbSxSignalPrams.DVBS2XCodeRate;
		//pDvbSxChannelParams->stDvbSxSignalPrams.DVBSxSymbolRateHz = gDvbSxScanParams.DvbSxChannel[u32TranspoderChannelIndex].stDvbSxSignalPrams.DVBSxSymbolRateHz;
	}
}
#endif

/*Blind scan start*/
void RTK_A_DVBSX_BlindScanStart(DVBSX_DEMOD_MODULE *pDemod)
{
	RTK_A_DVBSX_STATUS status = RTK_DEMOD_SUCCESSFUL;
	//status |= RTK_A_DVBSX_BlindScanProcess();
	REALTEK_A_DVBSX_INFO("RTK_A_DVBSX_BlindScanStart\n");
	if(status != RTK_DEMOD_SUCCESSFUL)
	{
		/*REALTEK_A_DVBSX_INFO("RTK_A_DVBSX_BlindScanStart failed\n"); */
	}
}

/*Blind scan self init for clear checked channel parameters */
RTK_A_DVBSX_STATUS RTK_A_DVBSX_BlindScan_SelfInit(DVBSX_DEMOD_MODULE *pDemod)
{
	RTK_A_DVBSX_STATUS status = RTK_DEMOD_SUCCESSFUL;
	unsigned int u32Iter = 0;

	gDvbSxBsSpecialParams.u8IsScanSymbolMode = 1;//First blindscan is scan symbol mode
	gDvbSxBsSpecialParams.u8IsCheckTPTunerDone = 0;
	gDvbSxBsSpecialParams.u8IsTPLock = 0;
	gDvbSxBsSpecialParams.u8BsLnbMode = 0;
	//gDvbSxBsSpecialParams.u8IsSpectrumCheckDone = 0;
	gDvbSxBsSpecialParams.u8NoSignalInSpectrum = 0;

	/*The following setting maybe changed be DTV application, such scan a small frequency range.*/
	//gDvbSxScanParams.u32ScanStartFrequencyKHz = 950*1000; /*Blind scan start frequency, may be changed.*/
	//gDvbSxScanParams.u32ScanStopFrequencyKHz = 2150*1000; /*Blind Scan stop frequency.*/
	//gDvbSxScanParams.u32MinSymbolRateHz = 2000000; /*optional:999K--->5000Km, set the minimal symbol rate that will be received. */
	gDvbSxScanParams.u32MinSymbolRateHz = 999000; /*optional:999K--->5000Km, set the minimal symbol rate that will be received. */
	gDvbSxScanParams.u32MaxSymbolRateHz = 45000000;
	gDvbSxScanParams.u32BlindScanStepKHz = 20000;//30000
	gDvbSxScanParams.u8SingalInLockingState = 0;
	gDvbSxScanParams.u32MaxCarrierOffsetKHz = 12000;
	gDvbSxScanParams.i32CFOLimitKHz = 12000; /*Maximum CFO limit in scan symbols */
	gDvbSxScanParams.i32SpectrumRangeLimit = 40000; /*Maximum half spectrum range limit in scan symbols */
	gDvbSxScanParams.u32CurrentTunedFreKHz = 950*1000;
	gDvbSxScanParams.u32ScanPreCheckedFreqKHz = 950*1000;
	gDvbSxScanParams.u32PreTunedSymbolRateHz = 2000000;
	gDvbSxScanParams.u8SignalRequiredCheckTimes = 0;

	gDvbSxScanParams.u32CurrentSymbolScanFreKHz = gDvbSxScanParams.u32ScanStartFrequencyKHz;
	gDvbSxScanParams.u32CurrentWorkFreKHzProcessed = gDvbSxScanParams.u32ScanStartFrequencyKHz;

	gDvbSxScanParams.u32BlindScanCheckedSignalIndex = 0;

	if(gDvbSxScanParams.u32BlindScanStepKHz > 20000)
	{
		gDvbSxScanParams.u32BlindScanStepKHz = 20000;//30000
	}

	gDvbSxScanParams.u32ValidChannelNumber = 0;
	gDvbSxScanParams.ui32ScanedValidChannelNum = 0;
	gDvbSxScanParams.u32GetFoundChannelNumber = 0;
	for(u32Iter = 0; u32Iter < MAX_SCANED_CHANNEL_NUMBER; u32Iter++)
	{
		gDvbSxScanParams.DvbSxChannel[u32Iter].u32FrequencyKHz = 0;
		gDvbSxScanParams.DvbSxChannel[u32Iter].u32SymbolRateHz = 0;
		gDvbSxScanParams.GetFoundChannel[u32Iter].u32FrequencyKHz = 0;
		gDvbSxScanParams.GetFoundChannel[u32Iter].u32SymbolRateHz = 0;
	}

	gScanedAllSymbols.u32ScanedSignalNum = 0;
	gScanedAllSymbols.u32ScanedSortStartIdx = 0;
	for(u32Iter = 0; u32Iter < TOTAL_MAX_SCANED_SYMBOL; u32Iter++)
	{
		gScanedAllSymbols.ScanedSignalList[u32Iter].u32ScanedFreqKHz = 0;
		gScanedAllSymbols.ScanedSignalList[u32Iter].u32SymbolRateKHz = 0;
	}
	gScanedParams.u8ScanedSymbolNum = 0;

	gDvbSxScanParams.u8FirstfrequencyScaned = 0;
	gDvbSxScanParams.u32MaxSignalRightRangeFreqKHz = gDvbSxScanParams.u32ScanStartFrequencyKHz;

	REALTEK_A_DVBSX_INFO("RTK_A_DVBSX_BlindScan_SelfInit %u %u =========================================\n", gDvbSxScanParams.u32ScanStartFrequencyKHz, gDvbSxScanParams.u32ScanStopFrequencyKHz);

	DvbSxBlindScanState = 1;
	//gDvbSxBsSpecialParams.u8UnicableEnabled = 0;
	//gDvbSxBsSpecialParams.pSignalSpectrum = RTK_DVBSx_SIGNAL_SPECTRUM_NORMAL;
	//gDvbSxBsSpecialParams.pCheckedSpectrum = RTK_DVBSx_SIGNAL_SPECTRUM_UNKNOWN;
	return status;
}