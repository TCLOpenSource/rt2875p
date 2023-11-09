/************************************************************************
 *  Include files
 ************************************************************************/
#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/semaphore.h>
#include <linux/slab.h>
#include <asm-generic/io.h>
#include "rtk_ai.h"
#include "rtk_ai_common.h"

/************************************************************************
 *  Definitions
 ************************************************************************/


/************************************************************************
*  Public variables
************************************************************************/
extern AI_AUDIO_STATE aiAudioState;

/************************************************************************
*  Function body
************************************************************************/

BOOL rtk_ai_audio_init(void){
	
	if(aiAudioState.inited==1)
	{
		AI_ERROR("already inited\n");
		return TRUE;
	}
	
	memset(&aiAudioState,0,sizeof(AI_AUDIO_STATE));
	
	aiAudioState.inited=1;

	return TRUE;
}

BOOL rtk_ai_audio_unInit(void){

	aiAudioState.inited=0;
	
	return TRUE;
}

BOOL rtk_ai_audio_vs_set_volume(int isVocal,unsigned int vol){

	if(aiAudioState.inited==0)
	{
		AI_ERROR("ai audio is not inited\n");
		return FALSE;
	}

	if(isVocal)
		aiAudioState.vocalVol=vol;
	else	
		aiAudioState.musicVol=vol;
	
	return TRUE;
}

