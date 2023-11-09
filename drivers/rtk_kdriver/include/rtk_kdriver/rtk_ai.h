#ifndef _RTK_AI_H_
#define _RTK_AI_H_

/************************************************************************
 *  Include files
 ************************************************************************/
#include <ioctrl/ai/ai_cmd_id.h>
#include <rtd_log/rtd_module_log.h>

/************************************************************************
 *  Definitions
 ************************************************************************/

#define AI_DEBUG(fmt, args...) \
{ \
        if (0)  \
        {   \
                rtd_pr_ai_dbg_notice(fmt, ## args); \
        }   \
}
#define AI_INFO(fmt, args...)   rtd_pr_ai_dbg_info(fmt, ## args)
#define AI_WARN(fmt, args...)   rtd_pr_ai_dbg_warn(fmt, ## args)
#define AI_ERROR(fmt, args...)  rtd_pr_ai_dbg_err(fmt, ## args)

#ifndef TRUE
#define TRUE                                    (1)
#endif

#ifndef FALSE
#define FALSE                                   (0)
#endif

#ifndef BOOL
#define BOOL                        bool
#endif

#ifndef AI_IOC_AUDIO_SET_EMOTION
typedef enum{
	AI_EMOTION_SILENCE=0,
	AI_EMOTION_OTHER,
	AI_EMOTION_ANGERY,
	AI_EMOTION_HAPPY,
	AI_EMOTION_NEUTRAL,
	AI_EMOTION_SAD,
	AI_EMOTION_TYPE_NUM,
} AI_EMOTION_TYPE;

#endif

typedef struct _AI_AUDIO_STATE {
	unsigned int inited;
	unsigned int enable;
	unsigned int dbgOsdOn;	
	unsigned int vocalVol;
	unsigned int musicVol;
	unsigned int emotion;
	unsigned int emotionScore[AI_EMOTION_TYPE_NUM];
}AI_AUDIO_STATE;

bool rtk_ai_mc_init(void);
BOOL rtk_ai_mc_unInit(void);
BOOL rtk_ai_mc_registModel(AI_PROCESS_INFO *info,int handle);
BOOL rtk_ai_mc_unRegistModel(int handle);
BOOL rtk_ai_mc_getShareMem(unsigned int *phy_addr);
BOOL rtk_ai_mc_getGrant(int handle,AI_HW_INDEX index);
BOOL rtk_ai_mc_releaseGrant(int handle,AI_HW_INDEX index);
BOOL rtk_ai_mc_getCurModVpqCtrl(AI_MC_VPQ_INDEX *index);
int rtk_ai_mc_getNextHandle(void);
BOOL rtk_ai_mc_crashRelease(int handle);
BOOL rtk_ai_mc_backup(void);
BOOL rtk_ai_mc_restore(void);
BOOL rtk_ai_mc_setClockRatio(int ratio);
BOOL rtk_ai_mc_enableNpuLoadCheck(int enable);
int rtk_ai_mc_getNpuLoad(void);

BOOL rtk_ai_audio_init(void);
BOOL rtk_ai_audio_unInit(void);
BOOL rtk_ai_audio_vs_set_volume(int isVocal,unsigned int vol);

int rtk_ai_set_enable_status(AI_ENABLE_STATUS *status);

#endif	/* _RTK_AI_H_ */
