#include "rtkdemux_pvr.h"

#ifndef abs64
#define abs64 abs
#endif

void PVR_CheckPvrAudioStable(demux_channel *pCh, int64_t pts)
{
	REFCLOCK * const pRefClock = DEMUX_GetReferenceClock(pCh);
	int64_t videoSystemPTS = DEMUX_reverseLongInteger(pRefClock->videoSystemPTS);
	long audioFreeRunThreshold = DEMUX_reverseInteger(pRefClock->audioFreeRunThreshold);
	if (videoSystemPTS == -1)
		return;

	if (pts > videoSystemPTS || videoSystemPTS - pts >= audioFreeRunThreshold)
		SET_FLAG(pCh->status, STATUS_PVR_AUDIO_IS_STABLE);
	dmx_mask_print(DEMUX_AUDIO_STABLE_DEBUG,"(((vSysPTS:%lld, aDemuxPTS:%lld, stable = %d)))\n", videoSystemPTS,    pts, HAS_FLAG(pCh->status, STATUS_PVR_AUDIO_IS_STABLE) != 0);

}

void PVR_PvrStartToPlay(demux_channel *pCh)
{
	REFCLOCK * const refClock = DEMUX_GetReferenceClock(pCh);

	/* stop to do buffering */
	pCh->bPendingOnPTSCheck = false;
	pCh->bPendingOnFullness = false;

	/* let master starts to play */
	REFCLK_SetTimeout(refClock, 0);
	PVR_EnableTPRateControl(pCh, true);

	/* from now on, we want to take only 200 ms to check if audio can be stable or not */
	pCh->startCheckAudioStable = CLOCK_GetPTS();

	/* start to check if it is possible that audio can catch video or not */
	SET_FLAG(pCh->status, STATUS_CHECK_PVR_AUDIO_STABLE);

	/* audio is not master at the beginning of presentation */
	RESET_FLAG(pCh->status, STATUS_PVR_AUDIO_IS_STABLE);
}

#define ENABLE_FRAME_RATE_CONTROL
void PVR_EnableTPRateControl(demux_channel *pCh, bool bEnable)
{
	TPK_TP_STATUS_T tpStatus;
	// NOTICE: caller should unlock pCh->mutex first to avoid dead lock happen.
	mutex_lock(&pCh->mutex);
#ifdef ENABLE_FRAME_RATE_CONTROL
	if(bEnable && HAS_FLAG(pCh->status, STATUS_APVR_PLAYBACK_START))
	{
		/* Do not enable TP rate control when APVR file playback */
		mutex_unlock(&pCh->mutex);
		return;
	}
	if (RHAL_GetTpStatus(pCh->tunerHandle, &tpStatus) == TPK_SUCCESS) {
		/*Enable frame rate control if timestamp is available */
		bool bCanTurnOn = bEnable && (tpStatus.tp_param.ts_in_tsp_len == byte_192);
		RHAL_MTPFrameControl(pCh->tunerHandle, bCanTurnOn);
		if (bCanTurnOn)
			SET_FLAG(pCh->status, STATUS_TP_RATE_CONTROL_ENABLED);
		else
			RESET_FLAG(pCh->status, STATUS_TP_RATE_CONTROL_ENABLED);
	}
	else {
		RHAL_MTPFrameControl(pCh->tunerHandle, false);
		RESET_FLAG(pCh->status, STATUS_TP_RATE_CONTROL_ENABLED);
		dmx_crit("[%s %d] Error!RHAL_GetTpStatus fail! Can't start TP Rate Control !\n", __func__, __LINE__);
	}
#else
	RHAL_MTPFrameControl(pCh->tunerHandle, false);
	RESET_FLAG(pCh->status, STATUS_TP_RATE_CONTROL_ENABLED);
#endif
	mutex_unlock(&pCh->mutex);
}

void PVR_DetectAudioOnlyFile(demux_channel *pCh)
{
	if (pCh->tp_src == MTP) {
		if (pCh->activeTarget[DEMUX_TARGET_VIDEO].pid == -1 && pCh->activeTarget[DEMUX_TARGET_VIDEO_2ND].pid == -1 && pCh->avSyncMode != NAV_AVSYNC_AUDIO_ONLY)
			DEMUX_ResetAVSync(pCh, NAV_AVSYNC_AUDIO_ONLY);
	}
}

void PVR_CheckPTSDiff(demux_channel *pCh)
{
	DEMUX_PCRSYNC_T *pPcrProcesser;
	REFCLOCK * const refClock = DEMUX_GetReferenceClock(pCh);
	if (!pCh->bPendingOnPTSCheck)
		return;

	PVR_DetectAudioOnlyFile(pCh);
	pPcrProcesser = &pCh->pcrProcesser;

	if (((pPcrProcesser->startVideoPts > pPcrProcesser->startAudioPts) && (pPcrProcesser->lastDemuxAudioPTS - pPcrProcesser->startVideoPts) >= pCh->avSyncStartupPTS)
		|| (pPcrProcesser->startVideoPts <= pPcrProcesser->startAudioPts && (pPcrProcesser->lastDemuxVideoPTS-pPcrProcesser->startAudioPts) >= pCh->avSyncStartupPTS)) {
		PVR_PvrStartToPlay(pCh);
		dmx_crit("[%s %d] Start to play: APTS = %lld, VPTS = %lld\n", __func__, __LINE__, pPcrProcesser->lastDemuxAudioPTS, pPcrProcesser->lastDemuxVideoPTS);
	}

	if (pCh->bPendingOnPTSCheck && REFCLK_GetRCD(refClock) != -1) {
		PVR_PvrStartToPlay(pCh);
		dmx_crit("[%s %d] Check PTS timeout: APTS = %lld, VPTS = %lld\n", __func__, __LINE__, pPcrProcesser->lastDemuxAudioPTS, pPcrProcesser->lastDemuxVideoPTS);
	}

	if ((REFCLK_GetRCD(refClock) == -1 && pPcrProcesser->startVideoPts != -1 && pPcrProcesser->startAudioPts != -1 && abs64(pPcrProcesser->startVideoPts -pPcrProcesser->startAudioPts) >= 3000*DEMUX_CLOCKS_PER_MS)
		|| (REFCLK_GetRCD(refClock) == -1 && pPcrProcesser->startAudioPts == -1 && pCh->activeTarget[DEMUX_TARGET_AUDIO].pid == -1 && pCh->activeTarget[DEMUX_TARGET_AUDIO_2ND].pid == -1)){
		pCh->avSyncStartupFullness = 512 * 1024;
		pCh->bPendingOnFullness = true;
		pCh->avSyncStartupPTS = 0;
		pCh->bPendingOnPTSCheck = false;
		dmx_crit("[%s %d] A/V startPTs diff >= 270000 (3 secs)\n", __func__, __LINE__);
	}
}

// void PVR_EnableEsBufMonitor(demux_channel *pCh, bool bEnable)
// {
// 	mutex_lock(&pCh->mutex);
// 	dmx_crit("((([%s, %d] bEnable=%d)))\n", __func__, __LINE__, bEnable);
// 	/* start to monitor ES buffer */
// 	if(bEnable)
// 		SET_FLAG(pCh->status, STATUS_PVR_ESBUF_MONITOR_ENABLED);
// 	else
// 		RESET_FLAG(pCh->status, STATUS_PVR_ESBUF_MONITOR_ENABLED);
	
// 	RESET_FLAG(pCh->status, STATUS_PVR_ESBUF_IS_STABLE);
// 	mutex_unlock(&pCh->mutex);
// }

inline bool xEsMonitor_checkPTS(const int64_t aSysPTS, const int64_t vSysPTS, const int64_t lastDemuxAudioPTS, const int64_t lastDemuxVideoPTS)
{
	if ((aSysPTS !=-1 || vSysPTS != -1) && (lastDemuxAudioPTS !=-1 || lastDemuxVideoPTS != -1) )
		return true;
	else 
		return false;
}
inline bool xEsMonitor_checkAudio(const int64_t aSysPTS, const int64_t vSysPTS, const int64_t lastDemuxAudioPTS, const int64_t lastDemuxVideoPTS)
{
	if (lastDemuxAudioPTS ==-1) 
		return true;
	if (aSysPTS ==-1)/* audio not start to play */
	{
		if(vSysPTS == -1)
			return false;
		if((lastDemuxAudioPTS-vSysPTS)/90>200)
			return true;
		else 
			return false;
	}
	else{
		//if (lastDemuxAudioPTS < aSysPTS)/*stream wrap around*/
		//	return true;

		if ((lastDemuxAudioPTS-vSysPTS)/90>200)
			return true;
		else
			return false;
	}
}
inline bool xEsMonitor_checkVideio(const int64_t aSysPTS, const int64_t vSysPTS, const int64_t lastDemuxAudioPTS, const int64_t lastDemuxVideoPTS)
{
	if(lastDemuxVideoPTS ==-1)
		return true;
	if (vSysPTS == -1)
		return false;
	//if (lastDemuxVideoPTS < vSysPTS)/*stream wrap around*/
	//	return true;
	if ((int)(lastDemuxVideoPTS - vSysPTS)/90 >= 500)
		return true;
	else
		return false;
}

bool PVR_CheckESBufStable(demux_channel *pCh)
{
	bool ret = false;
	static unsigned int actionMonitor= 0;
	static unsigned int actionMonitor_subCount =0;
	bool print_info= false;
	static int64_t 	_last	 = 0;
	const int64_t _current = CLOCK_GetPTS();
	const int64_t _passed  = abs64(_current - _last);
	unsigned int conditionMonitor = 0;

	REFCLOCK * const pRefClock = DEMUX_GetReferenceClock(pCh);
	int64_t vSysPTS = DEMUX_reverseLongInteger(pRefClock->videoSystemPTS);
	int64_t aSysPTS = DEMUX_reverseLongInteger(pRefClock->audioSystemPTS);
	//static long aStableFullness = 1024;
	//long aFullness = -1;
	//long vFullness = -1;
	// a. Check PTS difference to determine ES buffer stable or not
/*
TRUE: stop feed data to a/v 
FALSE: allowed feed data to a/v



						|----Y---- (TRUE)
						|
						|										|--Y-- (FALSE)
audioDemuxPts ==-1  ----|					|Y---(vSysPTS==-1)--|
						|					|					|
						|					|					|									|---Y-- (TRUE)
						|					|					|N--(audioDemuxPts-vSysPTS>200ms)---|
						|					|														|---N-- (FALSE)
						|					|
						|-N-(aSysPTS==-1)----|
											|
											|									|--Y-- (TRUE)
											|N---(audioDemuxPts-aSysPTS>200ms)--|
																				|--N-- (FALSE)

						|----Y---- (TRUE)
						|
						|					
videoDemuxPts ==-1  ----|					|---Y-- (FALSE)
						|					|
						|					|
						|-N-(vSysPTS==-1)----|
											|
											|										|--Y-- (TRUE)
											|N---(videoDemuxPts-vSysPTS>500ms)------|
																					|--N-- (FALSE)

	*/
	if (HAS_FLAG(pCh->status, STATUS_AUDIO_PAUSED)) /*paused*/
	{
		conditionMonitor = (1<<4) | 0x1;// 0x11
		ret =  true;
		goto END;
	}
	if ((pCh->avSyncMode != NAV_AVSYNC_VIDEO_ONLY) && (pCh->pvrSpeed != 0) )/*nomalPlay*/
	{
#if 0
		if ((aSysPTS !=-1 || vSysPTS != -1) 
	 		 	&& (pCh->pcrProcesser.lastDemuxAudioPTS !=-1 || pCh->pcrProcesser.lastDemuxVideoPTS != -1) 
	 		 	&& ((pCh->pcrProcesser.lastDemuxAudioPTS ==-1)/*==>audio*/
	 		 		?true /*no audio data */
	 		 		:((aSysPTS ==-1)/* audio not start to play */
	 		 			?((vSysPTS == -1)
	 		 				?false
	 		 				:(((pCh->pcrProcesser.lastDemuxAudioPTS-vSysPTS)/90>200)
		 		 				?true
		 		 				:false)) /*audio buffer: 200ms, if audio not playing ,use videosystem to test audio buffer , to avoid audio is not master.*/
	 		 			: (pCh->pcrProcesser.lastDemuxAudioPTS  < aSysPTS)/*stream wrap around*/
	 		 				?true
	 		 				:((int)(pCh->pcrProcesser.lastDemuxAudioPTS - aSysPTS)/90 >= 200)
	 		 					?true
	 		 					:false))
	 	
	 		 	&& ((pCh->pcrProcesser.lastDemuxVideoPTS ==-1) /*==>video*/
	 		 		?true/*no video data */
	 		 		:((vSysPTS == -1)
	 		 			?false/*no videoSys pts not change */
	 		 			:((pCh->pcrProcesser.lastDemuxVideoPTS < vSysPTS)/*stream wrap around*/
	 		 				?true
	 		 				:((int)(pCh->pcrProcesser.lastDemuxVideoPTS - vSysPTS)/90 >= 500)?true:false)))/*vidoe buffer : 500ms*/
	 		)
#endif
	 	if (xEsMonitor_checkPTS(aSysPTS, vSysPTS,pCh->pcrProcesser.lastDemuxAudioPTS,pCh->pcrProcesser.lastDemuxVideoPTS) 
			&& xEsMonitor_checkAudio(aSysPTS, vSysPTS,pCh->pcrProcesser.lastDemuxAudioPTS,pCh->pcrProcesser.lastDemuxVideoPTS) 
			&&xEsMonitor_checkVideio(aSysPTS, vSysPTS,pCh->pcrProcesser.lastDemuxAudioPTS,pCh->pcrProcesser.lastDemuxVideoPTS))
		{
			if (//((pCh->pcrProcesser.lastDemuxAudioPTS !=-1 ) &&(aSysPTS!=-1) && (DEMUX_reverseInteger(pRefClock->AO_Underflow)) )/* for step play deliver data*/ /*for audio channel wrap around*/
				//||
				((pCh->pcrProcesser.lastDemuxVideoPTS !=-1) && (vSysPTS != -1) &&(DEMUX_reverseInteger(pRefClock->VO_Underflow)))) {
				ret = false;
				conditionMonitor = 0x00;// 0x00
			}
			else{
				ret =  true;
				conditionMonitor =( (1<<4) | 0x2);// 0x12
			}
			goto END;
		}
		else
		{
			conditionMonitor = 0x1;// 0x01
			ret = false;
			goto END;
		} 
	}
	if ((pCh->pvrSpeed == 0)/*step play*/
		||((pCh->avSyncMode == NAV_AVSYNC_VIDEO_ONLY)&&((pCh->pvrSpeed == 128)||(pCh->pvrSpeed == 512)))//0.5 x, 2x
		//||((pCh->pcrProcesser.lastDemuxAudioPTS ==-1) && (aSysPTS == -1) && (pCh->pvrSpeed == 256) && (pCh->pcrProcesser.lastDemuxVideoPTS != -1) &&(vSysPTS != -1)) //step play deliver data with speed=256
		)
	{//only check video status
		if ( (vSysPTS != -1  )
			&&(pCh->pcrProcesser.lastDemuxVideoPTS != -1) 
			&& (((int)(pCh->pcrProcesser.lastDemuxVideoPTS - vSysPTS)/90 >= 500)?true:false))
		{
			if (DEMUX_reverseInteger(pRefClock->VO_Underflow)) {
				ret = false;
				conditionMonitor = 0x02;// 0x02
			}
			else{
				ret =  true;
				conditionMonitor = ( (1<<4) | 0x3);// 0x13
			}
			goto END;
		}
		else
		{
			conditionMonitor = 0x3; //0x03
			ret =  false;
			goto END;
		}
	}
	conditionMonitor = 0x4;//0x04


END:


	if(_passed >= 45000)
	{
		// if(HAS_FLAG(pCh->events, EVENT_PVR_ESBUF_IS_STABLE)) {
		// 	dmx_crit("\n((( STABLE )))\n");
		// } else {
		// 	dmx_crit("\n((( NOT STABLE !!)))\n");
		// }
/*		dmx_mask_print(DEMUX_PVR_ES_MONITOR_DEBUG,"(((**[audio] apts(%lld)-aSysPts(%lld)=%d msec, master=%d, fullness=%d*** ret=%d )))\n",
			pCh->pcrProcesser.lastDemuxAudioPTS,
			aSysPTS,
			(int)(pCh->pcrProcesser.lastDemuxAudioPTS-aSysPTS)/90,
			pRefClock->mastership.masterState,
			aFullness,
			ret);
		
		dmx_mask_print(DEMUX_PVR_ES_MONITOR_DEBUG,"((([video] vpts(%lld)-vSysPts(%lld)=%d msec, fullness=%d  )))\n", 
			pCh->pcrProcesser.lastDemuxVideoPTS, 
			vSysPTS, 
			(int)(pCh->pcrProcesser.lastDemuxVideoPTS-vSysPTS)/90,
			vFullness); 
		*/

		print_info = true;
	}


	

	if (ret)
	{
		if(actionMonitor%2 == 0){// true : odd
			actionMonitor ++;
			print_info = true;
		}
		else
			actionMonitor_subCount++;
	}
	else
	{

		if(actionMonitor%2 == 1){ // false: even
			actionMonitor ++;
			print_info = true;
		}
		else
			actionMonitor_subCount++;
	}
	
	if (print_info){//( (pCh->pvrSpeed == 0) ||((int)(pCh->pcrProcesser.lastDemuxAudioPTS-aSysPTS)/90 >200 &&((int)(pCh->pcrProcesser.lastDemuxVideoPTS-vSysPTS)/9 >500))){
		if (ret){
					dmx_mask_print(DEMUX_PVR_ES_MONITOR_DEBUG,"\033[1;33m""ch:%d, a:%d ms, v:%d ms, synMode:%d, speed:%d, isPause:%d, pts:(%lld, %lld), sysPts:(%lld, %lld), underflow(%ld,%ld) ===>ret = %d (%d--%d): (0x%02x) \033[m\n", 
						pCh->tunerHandle,(int)(pCh->pcrProcesser.lastDemuxAudioPTS-aSysPTS)/90,(int)(pCh->pcrProcesser.lastDemuxVideoPTS-vSysPTS)/90,pCh->avSyncMode,pCh->pvrSpeed , HAS_FLAG(pCh->status, STATUS_AUDIO_PAUSED),
						pCh->pcrProcesser.lastDemuxAudioPTS,pCh->pcrProcesser.lastDemuxVideoPTS,aSysPTS,vSysPTS,DEMUX_reverseInteger(pRefClock->AO_Underflow),DEMUX_reverseInteger(pRefClock->VO_Underflow),ret, actionMonitor ,actionMonitor_subCount, conditionMonitor);
		}
		else{
			dmx_mask_print(DEMUX_PVR_ES_MONITOR_DEBUG,"\033[1;33;46m""ch:%d, a:%d ms, v:%d ms, synMode:%d, speed:%d, isPause:%d, pts:(%lld, %lld), sysPts:(%lld, %lld), underflow(%ld,%ld) ===>ret = %d (%d--%d): (0x%02x) \033[m\n", 
				pCh->tunerHandle,(int)(pCh->pcrProcesser.lastDemuxAudioPTS-aSysPTS)/90,(int)(pCh->pcrProcesser.lastDemuxVideoPTS-vSysPTS)/90,pCh->avSyncMode,pCh->pvrSpeed , HAS_FLAG(pCh->status, STATUS_AUDIO_PAUSED),
				pCh->pcrProcesser.lastDemuxAudioPTS,pCh->pcrProcesser.lastDemuxVideoPTS,aSysPTS,vSysPTS,DEMUX_reverseInteger(pRefClock->AO_Underflow),DEMUX_reverseInteger(pRefClock->VO_Underflow),ret, actionMonitor ,actionMonitor_subCount, conditionMonitor);
		}
		actionMonitor_subCount=1;
		_last = _current;
	}
	return ret;

}
