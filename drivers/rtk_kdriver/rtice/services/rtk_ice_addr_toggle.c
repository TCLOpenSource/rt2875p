#include "rtk_ice_addr_toggle.h"
#include <linux/kthread.h>
#include <linux/slab.h> //kmalloc
#include <linux/delay.h>
#include <rtk_kdriver/io.h>
#include <rbus/dc1_mc_reg.h>
#include <linux/types.h>
static DECLARE_WAIT_QUEUE_HEAD(rtice_addr_togl_wait);

//#define CONFIG_RTICE_ADDRTOGGLE

static ToggleInfo sToggleInfo={VSYNC_MEASURE_INTERVAL,0,0,0,false,false};
#define kAddressCount 32//can't use const values const in c doesn't mean constant. It means "read only"
static u64 addrToggleCnt[kAddressCount] = {0};
#ifdef CONFIG_RTICE_ADDRTOGGLE
static unsigned int toggleAddress[kAddressCount] = {
	DC1_MC_ADR00_TOGL_reg,
	DC1_MC_ADR01_TOGL_reg,
	DC1_MC_ADR02_TOGL_reg,
	DC1_MC_ADR03_TOGL_reg,
	DC1_MC_ADR04_TOGL_reg,
	DC1_MC_ADR05_TOGL_reg,
	DC1_MC_ADR06_TOGL_reg,
	DC1_MC_ADR07_TOGL_reg,
	DC1_MC_ADR08_TOGL_reg,
	DC1_MC_ADR09_TOGL_reg,
	DC1_MC_ADR10_TOGL_reg,
	DC1_MC_ADR11_TOGL_reg,
	DC1_MC_ADR12_TOGL_reg,
	DC1_MC_ADR13_TOGL_reg,
	DC1_MC_ADR14_TOGL_reg,
	DC1_MC_ADR15_TOGL_reg,
	DC1_MC_ADR16_TOGL_reg,
	DC1_MC_ADR17_TOGL_reg,
	DC1_MC_ADR18_TOGL_reg,
	DC1_MC_ADR19_TOGL_reg,
	DC1_MC_ADR20_TOGL_reg,
	DC1_MC_ADR21_TOGL_reg,
	DC1_MC_ADR22_TOGL_reg,
	DC1_MC_ADR23_TOGL_reg,
	DC1_MC_ADR24_TOGL_reg,
	DC1_MC_ADR25_TOGL_reg,
	DC1_MC_ADR26_TOGL_reg,
	DC1_MC_ADR27_TOGL_reg,
	DC1_MC_ADR28_TOGL_reg,
	DC1_MC_ADR29_TOGL_reg,
	DC1_MC_ADR30_TOGL_reg,
	DC1_MC_ADR31_TOGL_reg,
};
#endif
static int address_toggle_thread(void * data)
{
#ifdef CONFIG_RTICE_ADDRTOGGLE
	int i=0,curCnt=0;
	unsigned int timeoutCnt=0;
	dc1_mc_eff_meas_ctrl_RBUS dc1_mc_eff_meas_reg_val;
	u64 prevToglCnt[kAddressCount];
	/*get data*/
	ToggleInfo * pinfo = (ToggleInfo*)data;
	if(pinfo == NULL)
		return -RTICE_ADDR_TOGGLE_INVALID_PARAM;
	RTICE_DEBUG("[%s:%d]start address_toggle_thread\r\n", __func__,__LINE__);
	pinfo->f_is_running = true;
	wake_up(&rtice_addr_togl_wait);
	/*measure address toggle count*/
	while(!pinfo->f_stop_thread && curCnt < pinfo->totlaCount){
		RTICE_DEBUG("[%s:%d]curreent time: %d\r\n", __func__,__LINE__,curCnt);
		/*set vsync mode*/
		dc1_mc_eff_meas_reg_val.regValue = rtd_inl(DC1_MC_EFF_MEAS_CTRL_reg);
		dc1_mc_eff_meas_reg_val.meas_int_src = pinfo->toggleMode;
		dc1_mc_eff_meas_reg_val.eff_meas_en = 1;
		rtd_outl(DC1_MC_EFF_MEAS_CTRL_reg,dc1_mc_eff_meas_reg_val.regValue);
		/*wait finish*/
		while(!pinfo->f_stop_thread){
			dc1_mc_eff_meas_reg_val.regValue = rtd_inl(DC1_MC_EFF_MEAS_CTRL_reg);
			if(!dc1_mc_eff_meas_reg_val.eff_meas_en)
				break;
			timeoutCnt++;
			if(timeoutCnt >= 0x80000000)
				return -RTICE_ADDR_TOGGLE_TIME_OUT;
			msleep(10);
		}
		if(pinfo->f_stop_thread)
			break;
		/*read address toggle count*/
		memset(prevToglCnt,0,kAddressCount*sizeof(prevToglCnt[0]));
		for(i=0;i<kAddressCount;i++){
			prevToglCnt[i] = DC1_MC_ADR00_TOGL_get_adr00_togl(rtd_inl(toggleAddress[i]));
		}
		if(pinfo->f_stop_thread)
			break;
		/*calc toggle count's sum*/
		for(i=0;i<kAddressCount;i++){
			addrToggleCnt[i] += prevToglCnt[i];
		}
		curCnt++;
		pinfo->curCount = curCnt;
		if(pinfo->f_stop_thread)
			break;
		msleep(10);
	}
	pinfo->f_is_running = false;
	return 0;
#else
	return 0;
#endif
}

int start_address_toggle(unsigned char *cmd_data, unsigned long cmd_len)
{
	int nres = 0,i=0,j=0;
	if(cmd_len < 5)
		return -RTICE_ADDR_TOGGLE_CMD_LEN_ERROR;
	/*init toggle info*/
	sToggleInfo.toggleMode = cmd_data[i];//byte[0] is measure interval source. 0 is vsync mode,1 is refresh mode
	i++;
	sToggleInfo.totlaCount = 0;
	for(j=0;j<4;j++){
		sToggleInfo.totlaCount |= (cmd_data[i+j]<<(8*(3-j)));
	}
	RTICE_DEBUG("[%s:%d]totlaCount = %d.\r\n", __func__,__LINE__,sToggleInfo.totlaCount);
	i+=4;
	sToggleInfo.curCount=0;
	sToggleInfo.f_is_running = false;
	sToggleInfo.f_stop_thread = false;
	/*clear toggle count buf*/
	memset(addrToggleCnt,0,kAddressCount*sizeof(addrToggleCnt[0]));
	/*start thread*/
	stop_address_toggle();
	sToggleInfo.p_thread = kthread_run(address_toggle_thread,&sToggleInfo,"address_toggle_thread");
	nres = wait_event_timeout(rtice_addr_togl_wait,(sToggleInfo.f_is_running == true),100);
	if(!nres){
		RTICE_DEBUG("[%s:%d]Failed start address_toggle_thread.\r\n", __func__,__LINE__);
		return -RTICE_ADDR_TOGGLE_CREATE_THREAD;
	}
	return 0;
}

int get_address_toggle(unsigned char *out_buf, unsigned int buf_size)
{
	unsigned int i=0,j=0,t32=0;
	u64 t64=0;
	unsigned char* presData = out_buf;
	RTICE_DEBUG("[%s:%d]buf_size = %d.\r\n", __func__,__LINE__,buf_size);
	if(!out_buf/* || buf_size < kAddressCount*8+4*/)
		return -RTICE_ADDR_TOGGLE_INVALID_PARAM;
	/*out current measure times*/
	t32 = sToggleInfo.curCount;
	LTB4(t32 , presData[0], presData[1], presData[2],presData[3]);
	presData += 4;
	i+=4;
	/*out address out*/
	for(j=0;j<kAddressCount;j++){
		t64 = addrToggleCnt[j];
		LTB8(t64,presData[0],presData[1],presData[2],presData[3]\
				,presData[4],presData[5],presData[6],presData[7]);
		presData+=8;
		i+=8;
	}
	return i;
}

int stop_address_toggle(void)
{
	int nres = 0;
	if(sToggleInfo.f_is_running){
		RTICE_DEBUG("[%s:%d]set f_stop_thread is true.\r\n", __func__,__LINE__);
		sToggleInfo.f_stop_thread = true;
		nres = wait_event_timeout(rtice_addr_togl_wait,(sToggleInfo.f_is_running==false),100);
		if(!nres){
			RTICE_DEBUG("[%s:%d]Failed stop address_toggle_thread.\r\n", __func__,__LINE__);
			return -RTICE_ADDR_TOGGLE_CREATE_THREAD;
		}
	}
	sToggleInfo.p_thread = NULL;
	sToggleInfo.f_stop_thread = false;
	return 0;
}
