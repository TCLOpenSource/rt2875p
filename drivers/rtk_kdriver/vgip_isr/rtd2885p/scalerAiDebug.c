#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/string.h>
#include <linux/version.h>
#include <rtd_log/rtd_module_log.h>
#include <tvscalercontrol/vip/ai_pq.h>
#include "rtk_ai.h"



#define AI_PROC_DIR   "ai"
#define AI_PROC_ENTRY "debug"

#define MAX_CMD_LEN   256

extern RTK_AI_PQ_mode aipq_mode;
extern int scalerAI_pq_mode_ctrl(RTK_AI_PQ_mode ai_pq_mode, unsigned char dcValue);
extern AI_AUDIO_STATE aiAudioState;

enum {
	AI_CMD_AI_MODE = 0,
	AI_CMD_FACE_MODE = 1,
	AI_CMD_SCENE_MODE = 2,
	AI_CMD_SQM_MODE = 3,
	AI_CMD_DEPTH_MODE = 4,
	AI_CMD_OBJECT_MODE = 5,
	AI_CMD_SEMANTIC_MODE = 6,
	AI_CMD_GENRE_MODE = 7,
	AI_CMD_LOGO_MODE = 8,
	AI_CMD_NOISE_MODE = 9,
	AI_CMD_AUDIO_MODE = 10,
	AI_CMD_AUDIO_VOCAL = 11,
	AI_CMD_AUDIO_MUSIC = 12,
	AI_CMD_AUDIO_EMOTION = 13,

	AI_CMD_NUM
} AI_DEBUG_CMD_E;

static const char* ai_cmd_str[] = {
	"ai_mode=",         /*AI_CMD_AI_MODE*/
	"face_mode=",       /*AI_CMD_FACE_MODE*/
	"scene_mode=",      /*AI_CMD_SCENE_MODE*/
	"sqm_mode=",        /*AI_CMD_SQM_MODE*/
	"depth_mode=",      /*AI_CMD_DEPTH_MODE*/
	"obj_mode=",        /*AI_CMD_OBJECT_MODE*/
	"semantic_mode=",   /*AI_CMD_SEMANTIC_MODE*/
	"genre_mode=",      /*AI_CMD_GENRE_MODE*/
	"logo_mode=",      /*AI_CMD_LOGO_MODE*/
	"noise_mode=",      /*AI_CMD_NOISE_MODE*/
	"audio_mode=",      /*AI_CMD_AUDIO_MODE*/
	"audio_vocal=",      /*AI_CMD_AUDIO_VOCAL*/
	"audio_music=",      /*AI_CMD_AUDIO_MUSIC*/
	"audio_emotion=",    /*AI_CMD_AUDIO_EMOTION*/
};
static 	char value_ptr[MAX_CMD_LEN];

void execute_ai_cmd(int cmd_id, char* value_ptr)
{

	rtd_pr_ai_dbg_emerg("[%s %d]cmd_id = %d, value = %s\n", __func__, __LINE__, cmd_id, value_ptr);
#if IS_ENABLED(CONFIG_RTK_AI_DRV)
	switch(cmd_id)
	{
	case AI_CMD_AI_MODE:
	{
		int len = strlen(value_ptr);
		RTK_AI_PQ_mode ai_mode;
		memset(&ai_mode, 0, sizeof(ai_mode));
		rtd_pr_ai_dbg_emerg("[%s %d]len = %d\n", __func__, __LINE__, len);
		if(len >= 1)
		{
			ai_mode.clock_status = value_ptr[0] - '0';
		}
		if(len >= 2)
		{
			ai_mode.ap_mode = value_ptr[1] - '0';
		}
		if(len >= 3)
		{
			ai_mode.face_mode = value_ptr[2] - '0';;
			if(ai_mode.face_mode > 9 || ai_mode.face_mode < 0)
			{
				rtd_pr_ai_dbg_emerg("[%s %d] value_ptr[2] = %c\n", __func__, __LINE__, value_ptr[2]);
				break;
			}
		}
		if(len >= 4)
		{
			ai_mode.sqm_mode = value_ptr[3] - '0';
			if(ai_mode.sqm_mode > 9 || ai_mode.sqm_mode < 0)
			{
				rtd_pr_ai_dbg_emerg("[%s %d] value_ptr[3] = %c\n", __func__, __LINE__, value_ptr[3]);
				break;
			}
		}
		if(len >= 5)
		{
			ai_mode.scene_mode = value_ptr[4] - '0';
			if(ai_mode.scene_mode > 9 || ai_mode.scene_mode < 0)
			{
				rtd_pr_ai_dbg_emerg("[%s %d] value_ptr[4] = %c\n", __func__, __LINE__, value_ptr[4]);
				break;
			}
		}

		if(len >= 6)
		{
			ai_mode.genre_mode = value_ptr[5] - '0';
			if(ai_mode.genre_mode > 9 || ai_mode.genre_mode < 0)
			{
				rtd_pr_ai_dbg_emerg("[%s %d] value_ptr[5] = %c\n", __func__, __LINE__, value_ptr[5]);
				break;
			}
		}
		if(len >= 7)
		{
			ai_mode.depth_mode = value_ptr[6] - '0';
			if(ai_mode.depth_mode > 9 || ai_mode.depth_mode < 0)
			{
				rtd_pr_ai_dbg_emerg("[%s %d] value_ptr[6] = %c\n", __func__, __LINE__, value_ptr[6]);
				break;
			}
		}

		if(len >= 8)
		{
			ai_mode.obj_mode = value_ptr[7] - '0';
			if(ai_mode.obj_mode > 9 || ai_mode.obj_mode < 0)
			{
				rtd_pr_ai_dbg_emerg("[%s %d] value_ptr[7] = %c\n", __func__, __LINE__, value_ptr[7]);
				break;
			}
		}

		if(len >= 9)
		{
			ai_mode.pqmask_mode = value_ptr[8] - '0';
			if(ai_mode.pqmask_mode > 9 || ai_mode.pqmask_mode < 0)
			{
				rtd_pr_ai_dbg_emerg("[%s %d] value_ptr[8] = %c\n", __func__, __LINE__, value_ptr[8]);
				break;
			}
		}
		if(len >= 10)
		{
			ai_mode.semantic_mode = value_ptr[9] - '0';
			if(ai_mode.semantic_mode > 9 || ai_mode.semantic_mode < 0)
			{
				rtd_pr_ai_dbg_emerg("[%s %d] value_ptr[9] = %c\n", __func__, __LINE__, value_ptr[9]);
				break;
			}
		}

		if(len >= 11)
		{
			ai_mode.logo_mode = value_ptr[10] - '0';
			if(ai_mode.logo_mode > 9 || ai_mode.logo_mode < 0)
			{
				rtd_pr_ai_dbg_emerg("[%s %d] value_ptr[10] = %c\n", __func__, __LINE__, value_ptr[10]);
				break;
			}
		}

		if(len >= 12)
		{
			ai_mode.noise_mode = value_ptr[11] - '0';
			if(ai_mode.noise_mode > 9 || ai_mode.noise_mode < 0)
			{
				rtd_pr_ai_dbg_emerg("[%s %d] value_ptr[11] = %c\n", __func__, __LINE__, value_ptr[11]);
				break;
			}
		}

		rtd_pr_ai_dbg_emerg("ap_mode=%d, face_mode=%d, scene_mode=%d, genre_mode=%d, depth_mode=%d, sqm_mode=%d, obj_mode=%d, clock_status = %d\n", ai_mode.ap_mode, ai_mode.face_mode, ai_mode.scene_mode, ai_mode.genre_mode, ai_mode.depth_mode, ai_mode.sqm_mode, ai_mode.obj_mode, ai_mode.clock_status);
                scalerAI_pq_mode_ctrl(ai_mode, 0);

		break;
	}
	case AI_CMD_FACE_MODE:
	{
		RTK_AI_PQ_mode ai_mode = aipq_mode;
		if(kstrtoint(value_ptr, 0, &ai_mode.face_mode) != 0)
			break;

		ai_mode.ap_mode = (ai_mode.face_mode != 0);
		scalerAI_pq_mode_ctrl(ai_mode, 0);
		break;
	}
	case AI_CMD_SCENE_MODE:
	{
		RTK_AI_PQ_mode ai_mode = aipq_mode;
		if(kstrtoint(value_ptr, 0, &ai_mode.scene_mode) != 0)
			break;

		ai_mode.ap_mode = (ai_mode.scene_mode != 0);
		scalerAI_pq_mode_ctrl(ai_mode, 0);
		break;
	}
	case AI_CMD_SQM_MODE:
	{
		RTK_AI_PQ_mode ai_mode = aipq_mode;
		if(kstrtoint(value_ptr, 0, &ai_mode.sqm_mode) != 0)
			break;

		ai_mode.ap_mode = (ai_mode.sqm_mode != 0);
		scalerAI_pq_mode_ctrl(ai_mode, 0);
		break;
	}
	case AI_CMD_OBJECT_MODE:
	{
		RTK_AI_PQ_mode ai_mode = aipq_mode;
		if(kstrtoint(value_ptr, 0, &ai_mode.obj_mode) != 0)
			break;

		ai_mode.ap_mode = (ai_mode.obj_mode != 0);
		scalerAI_pq_mode_ctrl(ai_mode, 0);
		break;
	}
	case AI_CMD_DEPTH_MODE:
	{
		RTK_AI_PQ_mode ai_mode = aipq_mode;
		if(kstrtoint(value_ptr, 0, &ai_mode.depth_mode) != 0)
			break;

		ai_mode.ap_mode = (ai_mode.depth_mode != 0);
		scalerAI_pq_mode_ctrl(ai_mode, 0);
		break;
	}
	case AI_CMD_SEMANTIC_MODE:
	{
		RTK_AI_PQ_mode ai_mode = aipq_mode;
		if(kstrtoint(value_ptr, 0, &ai_mode.semantic_mode) != 0)
			break;

		ai_mode.ap_mode = (ai_mode.semantic_mode != 0);
		scalerAI_pq_mode_ctrl(ai_mode, 0);
		break;
	}
	case AI_CMD_GENRE_MODE:
	{
		RTK_AI_PQ_mode ai_mode = aipq_mode;
		if(kstrtoint(value_ptr, 0, &ai_mode.genre_mode) != 0)
			break;

		ai_mode.ap_mode = (ai_mode.genre_mode != 0);
		scalerAI_pq_mode_ctrl(ai_mode, 0);
		break;
	}

	case AI_CMD_NOISE_MODE:
	{
		RTK_AI_PQ_mode ai_mode = aipq_mode;
		if(kstrtoint(value_ptr, 0, &ai_mode.noise_mode) != 0)
			break;

		ai_mode.ap_mode = (ai_mode.noise_mode != 0);
		scalerAI_pq_mode_ctrl(ai_mode, 0);
		break;
	}

	case AI_CMD_AUDIO_MODE:
	{
		int mode;
		if(kstrtoint(value_ptr, 0, &mode) != 0)
			break;

		aiAudioState.dbgOsdOn = (unsigned int)mode;
		rtd_pr_ai_dbg_emerg("[%s %d]mode = %d\n", __func__, __LINE__, mode);

		break;
	}
	case AI_CMD_AUDIO_VOCAL:
	{
		int vol;
		if(kstrtoint(value_ptr, 0, &vol) != 0)
			break;

		aiAudioState.vocalVol = (unsigned int)vol;
		rtd_pr_ai_dbg_emerg("[%s %d]vocalVol = %d\n", __func__, __LINE__, vol);

		break;
	}
	case AI_CMD_AUDIO_MUSIC:
	{
		int vol;
		if(kstrtoint(value_ptr, 0, &vol) != 0)
			break;

		aiAudioState.musicVol = (unsigned int)vol;
		rtd_pr_ai_dbg_emerg("[%s %d]musicVol = %d\n", __func__, __LINE__, vol);

		break;
	}
	case AI_CMD_AUDIO_EMOTION:
	{
		int emotion;
		if(kstrtoint(value_ptr, 0, &emotion) != 0)
			break;

		aiAudioState.emotion = (unsigned int)emotion;
		rtd_pr_ai_dbg_emerg("[%s %d]emotion = %d\n", __func__, __LINE__, emotion);

		break;
	}
	default:
		break;
	}
#endif
}

static ssize_t ai_proc_write(
        struct file *file,
        const char __user *buf,
        size_t count,
        loff_t *ppos)
{
	char str[MAX_CMD_LEN];
	int cmd_id;

	if(buf == 0)
	{
		rtd_pr_ai_dbg_emerg("[%s %d]invalid argument\n", __func__, __LINE__);
		return -EFAULT;
	}
	if(count > MAX_CMD_LEN-1)
		count = MAX_CMD_LEN-1;
	if (copy_from_user(str, buf, count))
	{
		rtd_pr_ai_dbg_emerg("[%s %d]copy cmd fail\n", __func__, __LINE__);
		return -EFAULT;
	}
	if(count > 0)
		str[count-1] = '\0';

	for(cmd_id = 0; cmd_id < AI_CMD_NUM; cmd_id++)
	{
		if(strncmp(str, ai_cmd_str[cmd_id], strlen(ai_cmd_str[cmd_id])) == 0)
			break;
	}
	if(cmd_id == AI_CMD_NUM)
		return -EFAULT;
	if(strlen(ai_cmd_str[cmd_id]) >= MAX_CMD_LEN)
		return -EFAULT;

	memcpy((void *)value_ptr,(void *)&str[strlen(ai_cmd_str[cmd_id])],MAX_CMD_LEN-strlen(ai_cmd_str[cmd_id]));
	
	execute_ai_cmd(cmd_id, value_ptr);

	return count;
}

/*
 * init/de-init
 *
 * */
struct proc_dir_entry *ai_proc_dir = 0;
struct proc_dir_entry *ai_proc_entry = 0;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0))
static const struct proc_ops ai_proc_fops = {
       .proc_write = (void*)ai_proc_write,
};
#else
static const struct file_operations ai_proc_fops = {
        .owner = THIS_MODULE,
        .write = ai_proc_write,
};
#endif


bool ai_init_debug_proc(void)
{
	if(ai_proc_dir != 0)
		return true;

	ai_proc_dir = proc_mkdir(AI_PROC_DIR , NULL);
	if(ai_proc_dir == 0)
	{
		rtd_pr_ai_dbg_emerg("create /proc/%s faile\n", AI_PROC_DIR);
		return false;
	}

	ai_proc_entry = proc_create(AI_PROC_ENTRY, 0666, ai_proc_dir, &ai_proc_fops);
	if(ai_proc_entry == 0)
	{
		proc_remove(ai_proc_dir);
		ai_proc_dir = 0;
		rtd_pr_ai_dbg_emerg("create /proc/%s/%s faile\n", AI_PROC_DIR, AI_PROC_ENTRY);
		return false;
	}
	return true;
}

void ai_deinit_debug_proc(void)
{
	if(ai_proc_entry)
	{
		proc_remove(ai_proc_entry);
		ai_proc_entry = 0;
	}
	if(ai_proc_dir)
	{
		proc_remove(ai_proc_dir);
		ai_proc_dir = 0;
	}
}
