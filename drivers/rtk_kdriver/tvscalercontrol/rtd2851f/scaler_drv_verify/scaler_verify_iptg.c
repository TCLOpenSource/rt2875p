/*This is for ADC PTG setting*/
//Kernel Header file
#include <linux/semaphore.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/delay.h>


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
#include <tvscalercontrol/adcsource/adcctrl.h>
#include <tvscalercontrol/i3ddma/i3ddma.h>
#include <rtd_log/rtd_module_log.h>

static ModeTableType verify_PTG_TABLE[] =
{
	// if we move YPbPr mode to another place,
	// remember to change the mapping of mode numbe in CModeSetup()
	// function, because 480i & 576i should enter IP, others should not.
	{   // Mode 92 : 1080p60

		_MODE_1080P60,
		0 | _SYNC_HN_VN | _SYNC_HP_VN | _SYNC_HN_VP | _SYNC_HP_VP,	// Polarity Flags,
		1920, 1080,//1026+48+4+2+2,													// InputWidth, InputHeight,
		675, 600,														// HFreq in kHz, VFreq in Hz,
		_HFREQ_TOLERANCE, _VFREQ_TOLERANCE,						// HFreqTolerance in kHz, VFreqTolerance in Hz,
		2200, 1125,													// HTotal, VTotal,
		229,38,														// HStartPos, VStartPos,
	},

	{   // Mode 91 : 1080p50
		_MODE_1080P50,
		0 | _SYNC_HN_VN | _SYNC_HP_VN | _SYNC_HN_VP | _SYNC_HP_VP,	// Polarity Flags,
		1920, 1080,												// InputWidth, InputHeight,
		563, 501,														// HFreq in kHz, VFreq in Hz,
		_HFREQ_TOLERANCE, _VFREQ_TOLERANCE,						// HFreqTolerance in kHz, VFreqTolerance in Hz,
		2640, 1125,													// HTotal, VTotal,
		176, 38,														// HStartPos, VStartPos,
	},

	{   // Mode 90 : 1080i30
		_MODE_1080I30,
		0 | _SYNC_HN_VN | _SYNC_HP_VN | _SYNC_HN_VP | _SYNC_HP_VP,	// Polarity Flags,
		1920, 540,												// InputWidth, InputHeight,
		337, 600,														// HFreq in kHz, VFreq in Hz,
		_HFREQ_TOLERANCE, _VFREQ_TOLERANCE,						// HFreqTolerance in kHz, VFreqTolerance in Hz,
		2200, 562,													// HTotal, VTotal,
		229, 17,														// HStartPos, VStartPos,
	},

	{   // Mode 89: 1080i25
		_MODE_1080I25,
		0 | _SYNC_HN_VN | _SYNC_HP_VN | _SYNC_HN_VP | _SYNC_HP_VP,	// Polarity Flags,
		1920, 540,													// InputWidth, InputHeight,
		281, 500,														// HFreq in kHz, VFreq in Hz,
		_HFREQ_TOLERANCE, _VFREQ_TOLERANCE,						// HFreqTolerance in kHz, VFreqTolerance in Hz,
		2640, 562,													// HTotal, VTotal,
		180, 17,														// HStartPos, VStartPos,
	},


	{   // Mode 94 : 1080p24 (24 Hz)
		_MODE_1080P24,
		0 | _SYNC_HN_VN | _SYNC_HP_VN | _SYNC_HN_VP | _SYNC_HP_VP,	// Polarity Flags,
		1920, 1080,												// InputWidth, InputHeight,
		0x010e, 0x00f0,												// IHFreq in 100Hz, VFreq in Hz,
		_HFREQ_TOLERANCE, 5,						// HFreqTolerance in 100Hz, VFreqTolerance in 0.1Hz,
		0x0ABE, 0x0465,												// HTotal, VTotal,
		196, 42,												// HStartPos, VStartPos,
	},


	{   // Mode 95 : 1080p25 (25 Hz)
		_MODE_1080P25,
		0 | _SYNC_HN_VN | _SYNC_HP_VN | _SYNC_HN_VP | _SYNC_HP_VP,	// Polarity Flags,
		1920, 1080,												// InputWidth, InputHeight,
		0x0119, 0x00fa,												// IHFreq in 100Hz, VFreq in Hz,
		_HFREQ_TOLERANCE, 5,						// HFreqTolerance in 100Hz, VFreqTolerance in 0.1Hz,
		0x0A50, 0x0465,												// HTotal, VTotal,
		182, 38,												// HStartPos, VStartPos,
	},


	{   // Mode 97 : 1080p30 (30 Hz)
		_MODE_1080P30,
		0 | _SYNC_HN_VN | _SYNC_HP_VN | _SYNC_HN_VP | _SYNC_HP_VP,	// Polarity Flags,
		1920, 1080,												// InputWidth, InputHeight,
		0x0151, 0x012c,												// IHFreq in 100Hz, VFreq in Hz,
		_HFREQ_TOLERANCE, _VFREQ_TOLERANCE,						// HFreqTolerance in 100Hz, VFreqTolerance in 0.1Hz,
		0x0898, 0x0465,												// HTotal, VTotal,
		180, 38,												// HStartPos, VStartPos,
	},

	{   // Mode 87 : 720p50

		_MODE_720P50,
		0 | _SYNC_HN_VN | _SYNC_HP_VN | _SYNC_HN_VP | _SYNC_HP_VP,	// Polarity Flags,
		1280, 720,													// InputWidth, InputHeight,
		376, 502,														// HFreq in kHz, VFreq in Hz,
		_HFREQ_TOLERANCE, _VFREQ_TOLERANCE,						// HFreqTolerance in kHz, VFreqTolerance in Hz,
		1980, 750,													// HTotal, VTotal,
		244, 21,														// HStartPos, VStartPos,
	},

	{   // Mode 88 : 720p60
		_MODE_720P60,
		0 | _SYNC_HN_VN | _SYNC_HP_VN | _SYNC_HN_VP | _SYNC_HP_VP,	// Polarity Flags,
		1280, 720,													// InputWidth, InputHeight,
		450, 600,														// HFreq in kHz, VFreq in Hz,
		_HFREQ_TOLERANCE, _VFREQ_TOLERANCE,						// HFreqTolerance in kHz, VFreqTolerance in Hz,
		1650, 750,													// HTotal, VTotal,
		242,21,														// HStartPos, VStartPos,
	},

	{
	// Mode 83 : 480i
		_MODE_480I,
		0 | _SYNC_HN_VN | _SYNC_HP_VN | _SYNC_HN_VP | _SYNC_HP_VP,	// Polarity Flags,
		720, 242,														// InputWidth, InputHeight,
		157, 600,														// HFreq in kHz, VFreq in Hz,
		_HFREQ_TOLERANCE, _VFREQ_TOLERANCE,						// HFreqTolerance in kHz, VFreqTolerance in Hz,
		858, 262,														// HTotal, VTotal,
		67, 13,														// HStartPos, VStartPos,
	},

	{   // Mode 84 : 576i
		_MODE_576I,
		0 | _SYNC_HN_VN | _SYNC_HP_VN | _SYNC_HN_VP | _SYNC_HP_VP,	// Polarity Flags,
		720, 288,														// InputWidth, InputHeight,
		156, 500,														// HFreq in kHz, VFreq in Hz,
		_HFREQ_TOLERANCE, _VFREQ_TOLERANCE,						// HFreqTolerance in kHz, VFreqTolerance in Hz,
		864, 312,														// HTotal, VTotal,
		77, 19,														// HStartPos, VStartPos,
	},

	{   // Mode 85 : 480p

		_MODE_480P,
		0 | _SYNC_HN_VN | _SYNC_HP_VN | _SYNC_HN_VP | _SYNC_HP_VP,	// Polarity Flags,
		720, 482,														// InputWidth, InputHeight,
		315, 600,														// HFreq in kHz, VFreq in Hz,
		_HFREQ_TOLERANCE, _VFREQ_TOLERANCE,						// HFreqTolerance in kHz, VFreqTolerance in Hz,
		858, 525,														// HTotal, VTotal,
		67, 33,														// HStartPos, VStartPos,
	},

	{   // Mode 86 : 576p
		_MODE_576P,
		0 | _SYNC_HN_VN | _SYNC_HP_VN | _SYNC_HN_VP | _SYNC_HP_VP,	// Polarity Flags,
		720, 576,														// InputWidth, InputHeight,
		312, 500,														// HFreq in kHz, VFreq in Hz,
		_HFREQ_TOLERANCE, _VFREQ_TOLERANCE,						// HFreqTolerance in kHz, VFreqTolerance in Hz,
		864, 624,														// HTotal, VTotal,
		77, 41,														// HStartPos, VStartPos,
	},

	{   // Mode _MODE_3840_540_60P

		_MODE_3840_540_60P,
		0 | _SYNC_HN_VN | _SYNC_HP_VN | _SYNC_HN_VP | _SYNC_HP_VP,	// Polarity Flags,
		3840, 540,//1026+48+4+2+2,													// InputWidth, InputHeight,
		675, 600,														// HFreq in kHz, VFreq in Hz,
		_HFREQ_TOLERANCE, _VFREQ_TOLERANCE,						// HFreqTolerance in kHz, VFreqTolerance in Hz,
		4400, 562,													// HTotal, VTotal,
		458,19,														// HStartPos, VStartPos,
	},

	{   // Mode _MODE_3840_540_60P

		_MODE_1080_1920_60P,
		0 | _SYNC_HN_VN | _SYNC_HP_VN | _SYNC_HN_VP | _SYNC_HP_VP,	// Polarity Flags,
		1080, 1920,//1026+48+4+2+2,													// InputWidth, InputHeight,
		675, 600,														// HFreq in kHz, VFreq in Hz,
		_HFREQ_TOLERANCE, _VFREQ_TOLERANCE,						// HFreqTolerance in kHz, VFreqTolerance in Hz,
		1125, 2200,													// HTotal, VTotal,
		37,230,														// HStartPos, VStartPos,
	},
};

unsigned int get_adc_ptg_table_size(void)
{
	return (sizeof(verify_PTG_TABLE)/sizeof(ModeTableType));
}

ModeTableType *get_adc_ptg_table_from_index(unsigned short index)
{
	return (verify_PTG_TABLE + index);
}


unsigned char search_set_form_verify_modetable(unsigned short ptg_timing)
{//return verify_PTG_TABLE index
	StructDisplayInfo *p_adc_struct = Get_ADC_Dispinfo();
	unsigned int index = 0;
	unsigned int modetablesize = get_adc_ptg_table_size();
	if(p_adc_struct)
	{
		memset(p_adc_struct, 0, sizeof(StructDisplayInfo));
	}
	for(index = 0; index < modetablesize; index++)
	{
		if(verify_PTG_TABLE[index].ModeID == ptg_timing)
			break;
	}
	if(index == modetablesize)
	{

		rtd_pr_vsc_err("\r\n [error] search_verify_modetable can not find timing table (%d)\r\n", ptg_timing);
		return FALSE;
	}
	else
	{
		if(p_adc_struct)
		{
			p_adc_struct->INPUT_ADC_CLOCK = verify_PTG_TABLE[index].IHTotal;
			p_adc_struct->IPV_ACT_LEN_PRE = verify_PTG_TABLE[index].IVHeight;
			p_adc_struct->IPV_ACT_STA_PRE = verify_PTG_TABLE[index].IVStartPos;
			p_adc_struct->IPH_ACT_WID_PRE = verify_PTG_TABLE[index].IHWidth;
			p_adc_struct->IPH_ACT_STA_PRE = verify_PTG_TABLE[index].IHStartPos;
			p_adc_struct->Hsync = verify_PTG_TABLE[index].IHTotal;
			p_adc_struct->Vsync = verify_PTG_TABLE[index].IVTotal;
			p_adc_struct->ucMode_Curr = verify_PTG_TABLE[index].ModeID;
			p_adc_struct->ucMode_Table_Index = verify_PTG_TABLE[index].ModeID;
			p_adc_struct->IHFreq = verify_PTG_TABLE[index].IHFreq;
			p_adc_struct->IVFreq = verify_PTG_TABLE[index].IVFreq;
			p_adc_struct->IHSyncPulseCount = 2;//need to check//no use
			p_adc_struct->IHTotal = verify_PTG_TABLE[index].IHTotal;
			p_adc_struct->IVTotal = verify_PTG_TABLE[index].IVTotal;
			p_adc_struct->IHCount_PRE = verify_PTG_TABLE[index].IHTotal;
			p_adc_struct->IVCount_PRE = verify_PTG_TABLE[index].IVTotal;
			p_adc_struct->color_space = I3DDMA_COLOR_YUV444;
			p_adc_struct->IVSyncPulseCount = 2;//need to check
			switch(ptg_timing)
			{
				case _MODE_480I:
				case _MODE_576I:
				case _MODE_1080I25:
				case _MODE_1080I30:
					Scaler_SetDispStatusFromSource(p_adc_struct, SLR_DISP_INTERLACE, TRUE);
				break;

				default:
					Scaler_SetDispStatusFromSource(p_adc_struct, SLR_DISP_INTERLACE, FALSE);
				break;
			}
		}
	}
	return TRUE;
}

unsigned char iptg_driver_setting(unsigned short ptg_timing)
{
	extern void ADC_CRT_Setting(unsigned char enable);
	ADC_CRT_Setting(_ENABLE);/* Will add. Enable ADC CRT */
	switch(ptg_timing)
	{
		case _MODE_1080P60:
		case _MODE_1080P50:
		case _MODE_3840_540_60P:
		case _MODE_1080_1920_60P:
			//apll_ipg148M
			IoReg_Write32(0xb8020100, 0x0302011e);	// M=33,N=3,K=0,Charge pump current=7.5uA(VCO 100Mhz~200Mhz)
			IoReg_Write32(0xb8020104, 0x00050609);	// enable LDO
			IoReg_Write32(0xb8020108, 0x00000897);	// Divider=2200
			//IoReg_Write32(0xb802010c, 0x00430000); // ADC clk=(1/4)*VCO clk
			IoReg_Write32(0xb8020400, 0x80000400);	// APLL DIV control by I_smartFit
			IoReg_Write32(0xb8020428, 0x00430000);	// ADC clk=(1/4)*VCO clk
			IoReg_Write32(0xb8020110, 0x00000120);	// PLL phase interpolation control load
			IoReg_Write32(0xb8020114, 0x00000040);	// G value=64, P code over flow 3bit
			IoReg_Write32(0xb8020118, 0x00000002);	// Adaptive Tracking (Default value)
			IoReg_Write32(0xb802011c, 0x30353433);	// P code max=1743467 and new mode disable
			IoReg_Write32(0xb8020120, 0x000002a1);	// Delay chain select (Default value)
			IoReg_Write32(0xb8020124, 0x00000008);	// DDS tracking HS positive edge (Default value)
			IoReg_Write32(0xb8020128, 0x000003a6);	// I code=934
			IoReg_Write32(0xb802012c, 0x000000aa);	// P code=174347, over flow 3bit
			IoReg_Write32(0xb8020130, 0x00000000);	// Calibration disable (Default value)
			IoReg_Write32(0xb8020134, 0x00000000);	// PE measure disable (Default value)
			IoReg_Write32(0xb8020138, 0x00000000);	// PE Max measure disable (Default value)
			IoReg_Write32(0xb802013c, 0x00000040);	// enable double buffer load
			IoReg_Write32(0xb8020104, 0x00050608);	// enable APLL & dual clk

			//sp_ipg
			IoReg_Write32(0xb8021000, 0x80a30001);	// ADC HS source = HSFB , CS source = SOY0
			msleep(200);		// wait system stable
			IoReg_Write32(0xb8021000, 0x80a30701);	// enable SP auto run
			IoReg_Write32(0xb802106c, 0x00000001);	// SP output = PTG data

			if(ptg_timing == _MODE_1080P60)
			{//2k 60p
				//set IPTG
				IoReg_Write32(0xb8020000, 0x000001ff);	// global power on
				IoReg_Write32(0xb8022100, 0x08980465);	// Total pixel = 2200*1125
				IoReg_Write32(0xb8022104, 0x00e70780);	// Horizontal active start = 231, active width = 1920
				IoReg_Write32(0xb8022108, 0x00270438);	// Vertical active start = 39, active width = 1080
				IoReg_Write32(0xb802210c, 0x00000000);	// Disable timing generator

				// 0xb8022110 status read
				IoReg_Write32(0xb8022114, 0x00e70780);	// Pattern Gen display Hstart & Hactive
				IoReg_Write32(0xb8022118, 0x00270438);	// Pattern Gen display Vstart & Vactive
				IoReg_Write32(0xb802211C, 0x00000003);	// PTG control, select pattern = veritcal color bar
				IoReg_Write32(0xb8022120, 0x000000f0);	// Color bar width	=59
				IoReg_Write32(0xb802210c, 0x00000001);	// Enable timing generator
			}
			else if(ptg_timing == _MODE_1080P50)
			{//2k 50p
				//set IPTG
				IoReg_Write32(0xb8020000, 0x000001ff);	// global power on
				IoReg_Write32(0xb8022100, 0x0a500465);	// Total pixel = 2640*1125
				IoReg_Write32(0xb8022104, 0x00b20780);	// Horizontal active start = 178, active width = 1920
				IoReg_Write32(0xb8022108, 0x00270438);	// Vertical active start = 39, active width = 1080
				IoReg_Write32(0xb802210c, 0x00000000);	// Disable timing generator

				// 0xb8022110 status read
				IoReg_Write32(0xb8022114, 0x00b20780);	// Pattern Gen display Hstart & Hactive
				IoReg_Write32(0xb8022118, 0x00270438);	// Pattern Gen display Vstart & Vactive
				IoReg_Write32(0xb802211C, 0x00000003);	// PTG control, select pattern = veritcal color bar
				IoReg_Write32(0xb8022120, 0x000000f0);	// Color bar width	=59
				IoReg_Write32(0xb802210c, 0x00000001);	// Enable timing generator
			}
			else if(ptg_timing == _MODE_3840_540_60P)
			{//4k 0.5k
				//set IPTG
				IoReg_Write32(0xb8020000, 0x000001ff);	// global power on
				IoReg_Write32(0xb8022100, 0x11300232);	// Total pixel = 4400*625
				IoReg_Write32(0xb8022104, 0x01cc0f00);	// Horizontal active start = 178, active width = 1920
				IoReg_Write32(0xb8022108, 0x0014021c);	// Vertical active start = 39, active width = 1080
				IoReg_Write32(0xb802210c, 0x00000000);	// Disable timing generator

				// 0xb8022110 status read
				IoReg_Write32(0xb8022114, 0x01cc0f00);	// Pattern Gen display Hstart & Hactive
				IoReg_Write32(0xb8022118, 0x0014021c);	// Pattern Gen display Vstart & Vactive
				IoReg_Write32(0xb802211C, 0x00000003);	// PTG control, select pattern = veritcal color bar
				IoReg_Write32(0xb8022120, 0x000001e0);	// Color bar width	=59
				IoReg_Write32(0xb802210c, 0x00000001);	// Enable timing generator
			}
			else if(ptg_timing == _MODE_1080_1920_60P)
			{//1k 2k
				//set IPTG
				IoReg_Write32(0xb8020000, 0x000001ff);	// global power on
				IoReg_Write32(0xb8022100, 0x04650898);	// Total pixel = 1125*2200
				IoReg_Write32(0xb8022104, 0x00270438);	// Horizontal active start = 39, active width = 1080
				IoReg_Write32(0xb8022108, 0x00e70780);	// Vertical active start = 231, active width = 1920
				IoReg_Write32(0xb802210c, 0x00000000);	// Disable timing generator

				// 0xb8022110 status read
				IoReg_Write32(0xb8022114, 0x00270438);	// Pattern Gen display Hstart & Hactive
				IoReg_Write32(0xb8022118, 0x00e70780);	// Pattern Gen display Vstart & Vactive
				IoReg_Write32(0xb802211C, 0x00000003);	// PTG control, select pattern = veritcal color bar
				IoReg_Write32(0xb8022120, 0x00000087);	// Color bar width	=59
				IoReg_Write32(0xb802210c, 0x00000001);	// Enable timing generator
			}
		break;

		case _MODE_1080I30:
		case _MODE_1080I25:
			//apll_ipg148M
			IoReg_Write32(0xb8020100, 0x0302011e);	// M=33,N=3,K=0,Charge pump current=7.5uA(VCO 100Mhz~200Mhz)
			IoReg_Write32(0xb8020104, 0x00050609);	// enable LDO
			IoReg_Write32(0xb8020108, 0x00000897);	// Divider=2200
			//IoReg_Write32(0xb802010c, 0x00430000); // ADC clk=(1/4)*VCO clk
			IoReg_Write32(0xb8020400, 0x80000400);	// APLL DIV control by I_smartFit
			IoReg_Write32(0xb8020428, 0x00530000);	// ADC clk=(1/4)*VCO clk
			IoReg_Write32(0xb8020110, 0x00000120);	// PLL phase interpolation control load
			IoReg_Write32(0xb8020114, 0x00000040);	// G value=64, P code over flow 3bit
			IoReg_Write32(0xb8020118, 0x00000002);	// Adaptive Tracking (Default value)
			IoReg_Write32(0xb802011c, 0x30353433);	// P code max=1743467 and new mode disable
			IoReg_Write32(0xb8020120, 0x000002a1);	// Delay chain select (Default value)
			IoReg_Write32(0xb8020124, 0x00000008);	// DDS tracking HS positive edge (Default value)
			IoReg_Write32(0xb8020128, 0x000003a6);	// I code=934
			IoReg_Write32(0xb802012c, 0x000000aa);	// P code=174347, over flow 3bit
			IoReg_Write32(0xb8020130, 0x00000000);	// Calibration disable (Default value)
			IoReg_Write32(0xb8020134, 0x00000000);	// PE measure disable (Default value)
			IoReg_Write32(0xb8020138, 0x00000000);	// PE Max measure disable (Default value)
			IoReg_Write32(0xb802013c, 0x00000040);	// enable double buffer load
			IoReg_Write32(0xb8020104, 0x00050608);	// enable APLL & dual clk

			//sp_ipg
			IoReg_Write32(0xb8021000, 0x80a30001);	// ADC HS source = HSFB , CS source = SOY0
			msleep(200);		// wait system stable
			IoReg_Write32(0xb8021000, 0x80a30701);	// enable SP auto run
			IoReg_Write32(0xb802106c, 0x00000001);	// SP output = PTG data


			//set IPTG
			if(ptg_timing == _MODE_1080I30)
			{
				IoReg_Write32(0xb8020000, 0x000001ff);	// global power on
				IoReg_Write32(0xb8022100, 0x08980232);	// Total pixel = 2200*562
				IoReg_Write32(0xb8022104, 0x00e70780);	// Horizontal active start = 231, active width = 1920
				IoReg_Write32(0xb8022108, 0x0012021c);	// Vertical active start = 18, active width = 1080
				IoReg_Write32(0xb802210c, 0x00000002);	// Disable timing generator

				// 0xb8022110 status read
				IoReg_Write32(0xb8022114, 0x00e70780);	// Pattern Gen display Hstart & Hactive
				IoReg_Write32(0xb8022118, 0x0012021c);	// Pattern Gen display Vstart & Vactive
			}
			else
			{
				IoReg_Write32(0xb8020000, 0x000001ff);	// global power on
				IoReg_Write32(0xb8022100, 0x0a500232);	// Total pixel = 2640*562
				IoReg_Write32(0xb8022104, 0x00b60780);	// Horizontal active start = 182, active width = 1920
				IoReg_Write32(0xb8022108, 0x0012021c);	// Vertical active start = 18, active width = 1080
				IoReg_Write32(0xb802210c, 0x00000002);	// Disable timing generator

				// 0xb8022110 status read
				IoReg_Write32(0xb8022114, 0x00b60780);	// Pattern Gen display Hstart & Hactive
				IoReg_Write32(0xb8022118, 0x0012021c);	// Pattern Gen display Vstart & Vactive
			}
			IoReg_Write32(0xb802211C, 0x00000003);	// PTG control, select pattern = veritcal color bar
			//IoReg_Write32(0xb802211C, 0x00000080);   // PTG control, select pattern = Random generator
			//IoReg_Write32(0xb802211C, 0x00000004);   // PTG control, select pattern = horizontal color bar
			//IoReg_Write32(0xb802211C, 0x00000006);   // PTG control, select pattern = horizontal & vertical gradient
			IoReg_Write32(0xb8022120, 0x000000f0);	// Color bar width	=59
			//IoReg_Write32(0xb8022124, 0x00FFFFFF);   // color set 0 W: White
			//IoReg_Write32(0xb8022128, 0x0000FF00);   // color set 1 R: Red
			//IoReg_Write32(0xb802212C, 0x00FF0000);   // color set 2 G: Green
			//IoReg_Write32(0xb8022130, 0x0000FF00);   // color set 3 B: Blue
			//IoReg_Write32(0xb8022134, 0xFF00FF00);   // color set 4 C: Cyan
			//IoReg_Write32(0xb8022138, 0x00FFFF00);   // color set 5 M: Megnta
			//IoReg_Write32(0xb802213C, 0xFFFF00AA);   // color set 6 Y: Yellow
			//IoReg_Write32(0xb8022140, 0x00000000);   // color set 7 K: Black
			//IoReg_Write32(0xb8022144, 0x07000fff);   // color set (copy from defualt value)
			IoReg_Write32(0xb802210c, 0x00000003);	// Enable timing generator
		break;

		case _MODE_1080P24:
		case _MODE_1080P25:
		case _MODE_1080P30:
		case _MODE_720P50:
		case _MODE_720P60:
			IoReg_Write32(0xb8020100, 0x0302011e);	// M=33,N=3,K=0,Charge pump current=7.5uA(VCO 100Mhz~200Mhz)
			IoReg_Write32(0xb8020104, 0x00050609);	// enable LDO
			IoReg_Write32(0xb8020108, 0x00000897);	// Divider=2200
			//IoReg_Write32(0xb802010c, 0x00430000); // ADC clk=(1/4)*VCO clk
			IoReg_Write32(0xb8020400, 0x80000400);	// APLL DIV control by I_smartFit
			IoReg_Write32(0xb8020428, 0x00530000);	// ADC clk=(1/4)*VCO clk
			IoReg_Write32(0xb8020110, 0x00000120);	// PLL phase interpolation control load
			IoReg_Write32(0xb8020114, 0x00000040);	// G value=64, P code over flow 3bit
			IoReg_Write32(0xb8020118, 0x00000002);	// Adaptive Tracking (Default value)
			IoReg_Write32(0xb802011c, 0x30353433);	// P code max=1743467 and new mode disable
			IoReg_Write32(0xb8020120, 0x000002a1);	// Delay chain select (Default value)
			IoReg_Write32(0xb8020124, 0x00000008);	// DDS tracking HS positive edge (Default value)
			IoReg_Write32(0xb8020128, 0x000003a6);	// I code=934
			IoReg_Write32(0xb802012c, 0x000000aa);	// P code=174347, over flow 3bit
			IoReg_Write32(0xb8020130, 0x00000000);	// Calibration disable (Default value)
			IoReg_Write32(0xb8020134, 0x00000000);	// PE measure disable (Default value)
			IoReg_Write32(0xb8020138, 0x00000000);	// PE Max measure disable (Default value)
			IoReg_Write32(0xb802013c, 0x00000040);	// enable double buffer load
			IoReg_Write32(0xb8020104, 0x00050608);	// enable APLL & dual clk

			//sp_ipg
			IoReg_Write32(0xb8021000, 0x80a30001);	// ADC HS source = HSFB , CS source = SOY0
			msleep(200);		// wait system stable
			IoReg_Write32(0xb8021000, 0x80a30701);	// enable SP auto run
			IoReg_Write32(0xb802106c, 0x00000001);	// SP output = PTG data

			switch(ptg_timing)
			{
				case _MODE_1080P24:
				//set IPTG
				IoReg_Write32(0xb8020000, 0x000001ff);	// global power on
				IoReg_Write32(0xb8022100, 0x0abe0465);	// Total pixel = 2750*1125
				IoReg_Write32(0xb8022104, 0x00c60780);	// Horizontal active start = 198, active width = 1920
				IoReg_Write32(0xb8022108, 0x002b0438);	// Vertical active start = 43, active width = 1080
				IoReg_Write32(0xb802210c, 0x00000000);	// Disable timing generator

				// 0xb8022110 status read
				IoReg_Write32(0xb8022114, 0x00c60780);	// Pattern Gen display Hstart & Hactive
				IoReg_Write32(0xb8022118, 0x002b0438);	// Pattern Gen display Vstart & Vactive
				IoReg_Write32(0xb802211C, 0x00000003);	// PTG control, select pattern = veritcal color bar
				IoReg_Write32(0xb8022120, 0x000000f0);	// Color bar width	=59
				IoReg_Write32(0xb802210c, 0x00000001);	// Enable timing generator
				break;

				case _MODE_1080P25:
				//set IPTG
				IoReg_Write32(0xb8020000, 0x000001ff);	// global power on
				IoReg_Write32(0xb8022100, 0x0A500465);	// Total pixel = 2640*1125
				IoReg_Write32(0xb8022104, 0x00b80780);	// Horizontal active start = 184, active width = 1920
				IoReg_Write32(0xb8022108, 0x00270438);	// Vertical active start = 39, active width = 1080
				IoReg_Write32(0xb802210c, 0x00000000);	// Disable timing generator

				// 0xb8022110 status read
				IoReg_Write32(0xb8022114, 0x00b80780);	// Pattern Gen display Hstart & Hactive
				IoReg_Write32(0xb8022118, 0x00270438);	// Pattern Gen display Vstart & Vactive
				IoReg_Write32(0xb802211C, 0x00000003);	// PTG control, select pattern = veritcal color bar
				IoReg_Write32(0xb8022120, 0x000000f0);	// Color bar width	=59
				IoReg_Write32(0xb802210c, 0x00000001);	// Enable timing generator
				break;

				case _MODE_1080P30:
				//set IPTG
				IoReg_Write32(0xb8020000, 0x000001ff);	// global power on
				IoReg_Write32(0xb8022100, 0x08980465);	// Total pixel = 2200*1125
				IoReg_Write32(0xb8022104, 0x00b60780);	// Horizontal active start = 182, active width = 1920
				IoReg_Write32(0xb8022108, 0x00270438);	// Vertical active start = 21, active width = 1080
				IoReg_Write32(0xb802210c, 0x00000000);	// Disable timing generator

				// 0xb8022110 status read
				IoReg_Write32(0xb8022114, 0x00b60780);	// Pattern Gen display Hstart & Hactive
				IoReg_Write32(0xb8022118, 0x00270438);	// Pattern Gen display Vstart & Vactive
				IoReg_Write32(0xb802211C, 0x00000003);	// PTG control, select pattern = veritcal color bar
				IoReg_Write32(0xb8022120, 0x000000f0);	// Color bar width	=59
				IoReg_Write32(0xb802210c, 0x00000001);	// Enable timing generator
				break;


				case _MODE_720P50:
				//set IPTG
				IoReg_Write32(0xb8020000, 0x000001ff);	// global power on
				IoReg_Write32(0xb8022100, 0x07bc02ee);	// Total pixel = 1980*750
				IoReg_Write32(0xb8022104, 0x00f60500);	// Horizontal active start = 246, active width = 1280
				IoReg_Write32(0xb8022108, 0x0016030c);	// Vertical active start = 22, active width = 780
				IoReg_Write32(0xb802210c, 0x00000000);	// Disable timing generator

				// 0xb8022110 status read
				IoReg_Write32(0xb8022114, 0x00f60500);	// Pattern Gen display Hstart & Hactive
				IoReg_Write32(0xb8022118, 0x0016030c);	// Pattern Gen display Vstart & Vactive
				IoReg_Write32(0xb802211C, 0x00000003);	// PTG control, select pattern = veritcal color bar
				IoReg_Write32(0xb8022120, 0x000000A0);	// Color bar width	=59
				IoReg_Write32(0xb802210c, 0x00000001);	// Enable timing generator
				break;

				case _MODE_720P60:
				//set IPTG
				IoReg_Write32(0xb8020000, 0x000001ff);	// global power on
				IoReg_Write32(0xb8022100, 0x067202ee);	// Total pixel = 1650*750
				IoReg_Write32(0xb8022104, 0x00f40500);	// Horizontal active start = 244, active width = 1280
				IoReg_Write32(0xb8022108, 0x0016030c);	// Vertical active start = 22, active width = 780
				IoReg_Write32(0xb802210c, 0x00000000);	// Disable timing generator

				// 0xb8022110 status read
				IoReg_Write32(0xb8022114, 0x00f40500);	// Pattern Gen display Hstart & Hactive
				IoReg_Write32(0xb8022118, 0x0016030c);	// Pattern Gen display Vstart & Vactive
				IoReg_Write32(0xb802211C, 0x00000003);	// PTG control, select pattern = veritcal color bar
				IoReg_Write32(0xb8022120, 0x000000A0);	// Color bar width	=59
				IoReg_Write32(0xb802210c, 0x00000001);	// Enable timing generator
				break;

				default:
					break;
			}
			break;

		case _MODE_480I:
		case _MODE_576I:
			//apll_ipg148M
			IoReg_Write32(0xb8020100, 0x03030103);	// M=12,N=3,K=0
			IoReg_Write32(0xb8020104, 0x00050609);	// enable LDO
			IoReg_Write32(0xb8020108, 0x00000359);	// Divider=858
			//rtd_outl(0xb802010c, 0x00530040);	// ADC clk=(1/4)*VCO clk
			IoReg_Write32(0xb8020400, 0x80000400);	// APLL DIV control by I_smartFit
			IoReg_Write32(0xb8020428, 0x00530040);	// ADC clk=(1/4)*VCO clk
			IoReg_Write32(0xb8020110, 0x00000020);	// PLL phase interpolation control load
			IoReg_Write32(0xb8020114, 0x00000010);	// G value=1, P code no over flow 
			IoReg_Write32(0xb8020118, 0x00000002);	// Adaptive Tracking (Default value)
			IoReg_Write32(0xb802011c, 0x3009a733);	// P code max=316321 and new mode disable
			IoReg_Write32(0xb8020120, 0x000002a1);	// Delay chain select (Default value)
			IoReg_Write32(0xb8020124, 0x00000008);	// DDS tracking HS positive edge (Default value)
			IoReg_Write32(0xb8020128, 0x0000000f);	// I code=79
			IoReg_Write32(0xb802012c, 0x000000f7);	// P code=31632, no over flow 
			IoReg_Write32(0xb8020130, 0x00000000);	// Calibration disable (Default value)
			IoReg_Write32(0xb8020134, 0x00000000);	// PE measure disable (Default value)
			IoReg_Write32(0xb8020138, 0x00000000);	// PE Max measure disable (Default value)
			IoReg_Write32(0xb802013c, 0x00000040);	// enable double buffer load
			IoReg_Write32(0xb8020104, 0x00050608);	// enable APLL & dual clk

			//sp_ipg
			IoReg_Write32(0xb8021000, 0x80a30001);	// ADC HS source = HSFB , CS source = SOY0
			msleep(200);		// wait system stable
			IoReg_Write32(0xb8021000, 0x80a30701);	// enable SP auto run
			IoReg_Write32(0xb802106c, 0x00000001);	// SP output = PTG data


			//set IPTG
			if(ptg_timing == _MODE_480I)
			{
				IoReg_Write32(0xb8020000, 0x000001ff);	// global power on
				IoReg_Write32(0xb8022100, 0x035a0106);	// Total pixel = 858*262
				IoReg_Write32(0xb8022104, 0x004602d0);	// Horizontal active start = 70, active width = 720
				IoReg_Write32(0xb8022108, 0x000e00f2);	// Vertical active start = 14, active width = 242
				IoReg_Write32(0xb802210c, 0x00000002);	// Disable timing generator

				// 0xb8022110 status read
				IoReg_Write32(0xb8022114, 0x004602d0);	// Pattern Gen display Hstart & Hactive
				IoReg_Write32(0xb8022118, 0x000e00f2);	// Pattern Gen display Vstart & Vactive
			}
			else
			{
				IoReg_Write32(0xb8020000, 0x000001ff);	// global power on
				IoReg_Write32(0xb8022100, 0x03600138);	// Total pixel = 864*312
				IoReg_Write32(0xb8022104, 0x005002d0);	// Horizontal active start = 80, active width = 720
				IoReg_Write32(0xb8022108, 0x00140120);	// Vertical active start = 20, active width = 288
				IoReg_Write32(0xb802210c, 0x00000002);	// Disable timing generator

				// 0xb8022110 status read
				IoReg_Write32(0xb8022114, 0x005002d0);	// Pattern Gen display Hstart & Hactive
				IoReg_Write32(0xb8022118, 0x00140120);	// Pattern Gen display Vstart & Vactive
			}
			IoReg_Write32(0xb802211C, 0x00000003);	// PTG control, select pattern = veritcal color bar
			IoReg_Write32(0xb8022120, 0x0000005a);	// Color bar width	=59
			IoReg_Write32(0xb802210c, 0x00000003);	// Enable timing generator
		break;

		case _MODE_480P:
		case _MODE_576P:
			//apll_ipg148M
			IoReg_Write32(0xb8020100, 0x03030109);	// M=12,N=3,K=0
			IoReg_Write32(0xb8020104, 0x00050609);	// enable LDO
			IoReg_Write32(0xb8020108, 0x00000359);	// Divider=858
			//rtd_outl(0xb802010c, 0x00530040);	// ADC clk=(1/4)*VCO clk
			IoReg_Write32(0xb8020400, 0x80000400);	// APLL DIV control by I_smartFit
			IoReg_Write32(0xb8020428, 0x00530040);	// ADC clk=(1/4)*VCO clk
			IoReg_Write32(0xb8020110, 0x00000020);	// PLL phase interpolation control load
			IoReg_Write32(0xb8020114, 0x00000010);	// G value=1, P code no over flow 
			IoReg_Write32(0xb8020118, 0x00000002);	// Adaptive Tracking (Default value)
			IoReg_Write32(0xb802011c, 0x3009a733);	// P code max=316321 and new mode disable
			IoReg_Write32(0xb8020120, 0x000002a1);	// Delay chain select (Default value)
			IoReg_Write32(0xb8020124, 0x00000008);	// DDS tracking HS positive edge (Default value)
			IoReg_Write32(0xb8020128, 0x0000000f);	// I code=79
			IoReg_Write32(0xb802012c, 0x000000f7);	// P code=31632, no over flow 
			IoReg_Write32(0xb8020130, 0x00000000);	// Calibration disable (Default value)
			IoReg_Write32(0xb8020134, 0x00000000);	// PE measure disable (Default value)
			IoReg_Write32(0xb8020138, 0x00000000);	// PE Max measure disable (Default value)
			IoReg_Write32(0xb802013c, 0x00000040);	// enable double buffer load
			IoReg_Write32(0xb8020104, 0x00050608);	// enable APLL & dual clk

			//sp_ipg
			IoReg_Write32(0xb8021000, 0x80a30001);	// ADC HS source = HSFB , CS source = SOY0
			msleep(200);		// wait system stable
			IoReg_Write32(0xb8021000, 0x80a30701);	// enable SP auto run
			IoReg_Write32(0xb802106c, 0x00000001);	// SP output = PTG data


			//set IPTG
			if(ptg_timing == _MODE_480P)
			{
				IoReg_Write32(0xb8020000, 0x000001ff);	// global power on
				IoReg_Write32(0xb8022100, 0x035a020d);	// Total pixel = 858*525
				IoReg_Write32(0xb8022104, 0x004602d0);	// Horizontal active start = 70, active width = 720
				IoReg_Write32(0xb8022108, 0x002201e2);	// Vertical active start = 34, active width = 482
				IoReg_Write32(0xb802210c, 0x00000000);	// Disable timing generator

				// 0xb8022110 status read
				IoReg_Write32(0xb8022114, 0x004602d0);	// Pattern Gen display Hstart & Hactive
				IoReg_Write32(0xb8022118, 0x002201e2);	// Pattern Gen display Vstart & Vactive
			}
			else
			{
				IoReg_Write32(0xb8020000, 0x000001ff);	// global power on
				IoReg_Write32(0xb8022100, 0x03600270);	// Total pixel = 864*624
				IoReg_Write32(0xb8022104, 0x005002d0);	// Horizontal active start = 80, active width = 720
				IoReg_Write32(0xb8022108, 0x002a0240);	// Vertical active start = 42, active width = 576
				IoReg_Write32(0xb802210c, 0x00000000);	// Disable timing generator

				// 0xb8022110 status read
				IoReg_Write32(0xb8022114, 0x005002d0);	// Pattern Gen display Hstart & Hactive
				IoReg_Write32(0xb8022118, 0x002a0240);	// Pattern Gen display Vstart & Vactive
			}
			IoReg_Write32(0xb802211C, 0x00000003);	// PTG control, select pattern = veritcal color bar
			IoReg_Write32(0xb8022120, 0x0000005a);	// Color bar width	=59
			IoReg_Write32(0xb802210c, 0x00000001);	// Enable timing generator
		break;

		default:
			return FALSE;
		break;
	}
	return search_set_form_verify_modetable(ptg_timing);
}
