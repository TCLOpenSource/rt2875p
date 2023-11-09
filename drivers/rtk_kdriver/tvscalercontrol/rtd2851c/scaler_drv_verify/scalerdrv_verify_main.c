//Kernel Header file
#include <linux/semaphore.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/freezer.h>//For try_to_freeze



/* RBUS Header file*/

/* Tvscalercontrol Header file */
#ifdef CONFIG_KDRIVER_USE_NEW_COMMON
	#include <scaler/scalerCommon.h>
	#include <scaler/scalerDrvCommon.h>	
#else
	#include <scalercommon/scalerCommon.h>
	#include <scalercommon/scalerDrvCommon.h>	
#endif


#include <tvscalercontrol/scaler/scalerstruct.h>
#include <tvscalercontrol/scalerdrv/mode.h>
#include <tvscalercontrol/scaler_drv_verify/scaler_verify_common.h>
#include <rtd_log/rtd_module_log.h>

static struct semaphore scaler_verifymode_ctrl_semaphore;

static unsigned char scaler_verify_mode_enable = scaler_verify_mode_off;//record current scaler verify mode

void set_scaler_verify_mode(unsigned char verifymode)
{//set the verify mode
	down(&scaler_verifymode_ctrl_semaphore);
	scaler_verify_mode_enable = verifymode;
	up(&scaler_verifymode_ctrl_semaphore);
}

unsigned char get_scaler_verify_mode(void)
{//get current verfiy mode
	return scaler_verify_mode_enable;
}

unsigned char check_verify_mode_enable(void)
{//if TRUE, means current use verfify mode
	return ((scaler_verify_mode_enable == scaler_verify_mode_off) ? FALSE : TRUE);
}

void init_scaler_verifymode_semaphore(void)
{
	sema_init(&scaler_verifymode_ctrl_semaphore, 1);
}


static bool scaler_verify_tsk_running_flag = FALSE;//Record status scaler_verify_tsk. True: Task is running
static struct task_struct *p_scaler_verify_tsk = NULL;
static int scaler_verify_tsk(void *p);


void create_scaler_verify_tsk(void)
{
	int err;
	if (scaler_verify_tsk_running_flag == FALSE) {
		p_scaler_verify_tsk = kthread_create(scaler_verify_tsk, NULL, "scaler_verify_tsk");

	    if (p_scaler_verify_tsk) {
			wake_up_process(p_scaler_verify_tsk);
			scaler_verify_tsk_running_flag = TRUE;
		} else {
			err = PTR_ERR(p_scaler_verify_tsk);
			rtd_pr_vsc_err("Unable to start scaler_verify_tsk (err_id = %d)./n", err);
		}
	}
}



static int scaler_verify_tsk(void *p)//This task run scaler driver verify
{
	unsigned int finalresult = 0;
	extern void verify_source_force_connect(void);//conenct verify mode source
    extern void source_restore_from_verify_source(void);//restore original source
#ifdef dummy_reg_debug//for debug test
	int respons_time = 50;
#endif
    rtd_pr_vsc_debug("scaler_verify_tsk()\n");
	current->flags &= ~PF_NOFREEZE;
    while (1)
    {
		msleep(50);
		down(&scaler_verifymode_ctrl_semaphore);

#ifdef dummy_reg_debug//for debug test
		if((IoReg_Read32(0xb802020c) & 0xf) != 0)
		{
			if(!respons_time)
			{
				respons_time = 50;
				scaler_verify_mode_enable = IoReg_Read32(0xb802020c) & 0xf;
				IoReg_ClearBits(0xb802020c, 0xf);
			}
			else
				respons_time --;
		}
#endif

		switch(scaler_verify_mode_enable)
		{
			case scaler_capture_verify_mode:
				verify_source_force_connect();
				rtd_pr_vsc_notice("\r\n## go scaler_capture_verify_mode start ##\r\n");
				finalresult = run_capture_verify_mode();
				rtd_pr_vsc_notice("\r\n## go scaler_capture_verify_mode finish result:0x%x##\r\n", finalresult);
				source_restore_from_verify_source();
				break;

			case scaler_smoothtoggle_verifymode:
				rtd_pr_vsc_notice("\r\n## go scaler_smoothtoggle_verifymode start ##\r\n");
				rtd_pr_vsc_notice("\r\n## go scaler_smoothtoggle_verifymode finish ##\r\n");
				break;

			default:
				if(scaler_verify_mode_enable != scaler_verify_mode_off)
					rtd_pr_vsc_notice("\r\n## can not find the verify mode ##\r\n");
				break;
		}
		scaler_verify_mode_enable = scaler_verify_mode_off;
		up(&scaler_verifymode_ctrl_semaphore);
		if (freezing(current))
		{
			try_to_freeze();
		}
		if (kthread_should_stop()) {
			break;
		}
    }
    rtd_pr_vsc_debug("\r\n####scaler_verify_tsk: exit...####\n");
    do_exit(0);
    return 0;
}


