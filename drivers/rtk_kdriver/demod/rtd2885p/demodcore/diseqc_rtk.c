#include <rtk_kdriver/io.h>
#include "tv_osal.h"
#include "foundation.h"
#include "diseqc_rtk.h"
#include "dvbsx_demod_rtk_a.h"
#include <rtk_kdriver/rtk_crt.h>
#include <rtk_kdriver/rtk_gpio.h>
#include <rbus/sys_reg_reg.h>
#include "rbus/diseqc_reg.h"

#define GPIO_HI_LO
#define DISEQC_RX_RETRY_CNT 2
#define DISEQC_RX_RETRY_DELAY_TIME 100

unsigned int DvbS2_Keep_22k_on_off = 0;
extern struct RtkDemodDvbSxBsSpecialParams gDvbSxBsSpecialParams;

int
realtek_Diseqc_SetDiseqcContinue22kOnOff(
	unsigned int Diseqc22kOnOff
)
{
	REALTEK_DISEQC_INFO("\033[1;32;31m" "realtek_Diseqc_SetDiseqcContinue22kOnOff (%u)\n" "\033[m", Diseqc22kOnOff);
	DvbS2_Keep_22k_on_off = Diseqc22kOnOff;
	
	if (!(rtd_inl(SYS_REG_SYS_CLKEN2_reg) & SYS_REG_SYS_CLKEN2_clken_dtv_demod_mask))
	{
		REALTEK_DISEQC_INFO("Enable DTV DEMOD clk\n");
		CRT_CLK_OnOff(DTVDEMOD, CLK_ON, NULL);
	}
	if (!(rtd_inl(SYS_REG_SYS_CLKEN0_reg) & SYS_REG_SYS_CLKEN0_clken_atb_demod_mask))
	{
		REALTEK_DISEQC_INFO("Enable DTV ATB clk\n");
		CRT_CLK_OnOff(DTVATB, CLK_ON, NULL);
	}
	if (!(rtd_inl(SYS_REG_SYS_CLKEN0_reg) & SYS_REG_SYS_CLKEN0_clken_diseqc_mask))
	{
		REALTEK_DISEQC_INFO("Enable DISEQC clk\n");
		CRT_CLK_OnOff(DISEQC, CLK_ON, NULL);
	}
	
	rtd_maskl(DISEQC_DSCU_TX_CTL_reg, ~(DISEQC_DSCU_TX_CTL_tx_go_mask), DISEQC_DSCU_TX_CTL_tx_go(0x0));//Tx Off
	
	rtd_maskl(DISEQC_DSCU_TX_CTL_reg, ~(DISEQC_DSCU_TX_CTL_loop_back_mode_mask), DISEQC_DSCU_TX_CTL_loop_back_mode(0x1));//En loop back mode
	
	#ifdef CONFIG_CUSTOMER_TV006
	rtd_maskl(DISEQC_DSCU_CTL_reg, ~(DISEQC_DSCU_CTL_tx_env_mode_mask), DISEQC_DSCU_CTL_tx_env_mode(0x1));//Tx envelop mode
	#else
	rtd_maskl(DISEQC_DSCU_CTL_reg, ~(DISEQC_DSCU_CTL_tx_env_mode_mask), DISEQC_DSCU_CTL_tx_env_mode(0x0));//Tx pulse mode
	#endif
	
	rtd_maskl(DISEQC_DSCU_TX_CTL_reg, ~(DISEQC_DSCU_TX_CTL_xmt_mode_mask), DISEQC_DSCU_TX_CTL_xmt_mode(0x3));//Continue Tone Mode
	
	//rtd_maskl(DISEQC_DSCU_TX_CTL_reg, ~(DISEQC_DSCU_TX_CTL_tone_div_mask), DISEQC_DSCU_TX_CTL_tone_div(0x27f));//22k freq divider
	
	tv_osal_msleep(1); 
	
	if(Diseqc22kOnOff == 1)
	{
		rtd_maskl(DISEQC_DSCU_TX_CTL_reg, ~(DISEQC_DSCU_TX_CTL_tx_go_mask), DISEQC_DSCU_TX_CTL_tx_go(0x1));//Tx Go
	}
	else
	{
		rtd_maskl(DISEQC_DSCU_TX_CTL_reg, ~(DISEQC_DSCU_TX_CTL_tx_go_mask), DISEQC_DSCU_TX_CTL_tx_go(0x1));//Tx Go
		rtd_maskl(DISEQC_DSCU_TX_CTL_reg, ~(DISEQC_DSCU_TX_CTL_tx_go_mask), DISEQC_DSCU_TX_CTL_tx_go(0x0));//Tx Off
	}
	
	return FUNCTION_SUCCESS;
}

int
realtek_Diseqc_SetDiseqcCmd(
	unsigned int DataLength,
	unsigned char *Data
)
{
	int DataCount;
	
	REALTEK_DISEQC_INFO("\033[1;32;31m" "realtek_Diseqc_SetDiseqcCmd\n" "\033[m");
	
	if(*(Data+0) == 0xe0 && *(Data+1) == 0x10 && (*(Data+2) == 0x5a || *(Data+2) == 0x5c))
	{
		REALTEK_DISEQC_INFO("\033[1;32;35m" "Unicable v1 Enabled\n" "\033[m");
		gDvbSxBsSpecialParams.u8UnicableEnabled = 1;
	}
	else if(*(Data+0) == 0x70 || *(Data+0) == 0x71)
	{
		REALTEK_DISEQC_INFO("\033[1;32;35m" "Unicable v2 Enabled\n" "\033[m");
		gDvbSxBsSpecialParams.u8UnicableEnabled = 1;
	}
	else
	{
		REALTEK_DISEQC_INFO("\033[1;32;35m" "Unicable Disabled\n" "\033[m");
		gDvbSxBsSpecialParams.u8UnicableEnabled = 0;
	}
	
	if (!(rtd_inl(SYS_REG_SYS_CLKEN2_reg) & SYS_REG_SYS_CLKEN2_clken_dtv_demod_mask))
	{
		REALTEK_DISEQC_INFO("Enable DTV DEMOD clk\n");
		CRT_CLK_OnOff(DTVDEMOD, CLK_ON, NULL);
	}
	if (!(rtd_inl(SYS_REG_SYS_CLKEN0_reg) & SYS_REG_SYS_CLKEN0_clken_atb_demod_mask))
	{
		REALTEK_DISEQC_INFO("Enable DTV ATB clk\n");
		CRT_CLK_OnOff(DTVATB, CLK_ON, NULL);
	}
	if (!(rtd_inl(SYS_REG_SYS_CLKEN0_reg) & SYS_REG_SYS_CLKEN0_clken_diseqc_mask))
	{
		REALTEK_DISEQC_INFO("Enable DISEQC clk\n");
		CRT_CLK_OnOff(DISEQC, CLK_ON, NULL);
	}
	
	rtd_maskl(DISEQC_DSCU_TX_CTL_reg, ~(DISEQC_DSCU_TX_CTL_tx_go_mask), DISEQC_DSCU_TX_CTL_tx_go(0x0));//Tx Off
	
	rtd_maskl(DISEQC_DSCU_TX_CTL_reg, ~(DISEQC_DSCU_TX_CTL_loop_back_mode_mask), DISEQC_DSCU_TX_CTL_loop_back_mode(0x1));//En loop back mode
	
	#ifdef CONFIG_CUSTOMER_TV006
	rtd_maskl(DISEQC_DSCU_CTL_reg, ~(DISEQC_DSCU_CTL_tx_env_mode_mask), DISEQC_DSCU_CTL_tx_env_mode(0x1));//Tx envelop mode
	#else
	rtd_maskl(DISEQC_DSCU_CTL_reg, ~(DISEQC_DSCU_CTL_tx_env_mode_mask), DISEQC_DSCU_CTL_tx_env_mode(0x0));//Tx pulse mode
	#endif
	
	rtd_maskl(DISEQC_DSCU_TX_CTL_reg, ~(DISEQC_DSCU_TX_CTL_xmt_mode_mask), DISEQC_DSCU_TX_CTL_xmt_mode(0x2));//Diseqc Cmd Mode
	
	//rtd_maskl(DISEQC_DSCU_TX_CTL_reg, ~(DISEQC_DSCU_TX_CTL_tone_div_mask), DISEQC_DSCU_TX_CTL_tone_div(0x27f));//22k freq divider
	
	rtd_maskl(DISEQC_DSCU_TX_CTL_reg, ~(DISEQC_DSCU_TX_CTL_tx_pty_mask), DISEQC_DSCU_TX_CTL_tx_pty(0x1));//Odd parity
	
	rtd_maskl(DISEQC_DSCU_TX_CTL_reg, ~(DISEQC_DSCU_TX_CTL_msg_len_mask), DISEQC_DSCU_TX_CTL_msg_len(DataLength-1));//Set Data Length(0:1Byte, 1:2Byte, ...)
	
	for(DataCount=0; DataCount<DataLength; DataCount++)
	{
		rtd_outl((DISEQC_DSCU_TX_BUF0_reg+(4*DataCount)), *(Data+DataCount));
	}
	realtek_diseqc_PrintTxCmd(DataLength);
	
	//delay for td => move to APP layer
	//mdelay(10);
	
	REALTEK_DISEQC_INFO("CMD Delay %u ms\n", (135*DataLength+5)/10);
	
	rtd_maskl(DISEQC_DSCU_TX_CTL_reg, ~(DISEQC_DSCU_TX_CTL_tx_go_mask), DISEQC_DSCU_TX_CTL_tx_go(0x1));//Tx Go
	
	//delay for command finish
	mdelay((135*DataLength+5)/10);
	
	//delay for ta => move to APP layer
	//mdelay(30);
	
	return FUNCTION_SUCCESS;
}

int
realtek_Diseqc_SetDiseqc2p0Cmd(
	unsigned int *DataLength,
	unsigned char *Data
)
{
	int DataCount = 0;
	unsigned char RxRecDone1 = 0, RxRecDone2 = 0, RxDataCnt = 0, RxRetryCnt = 0;
	unsigned long stime = 0;
	unsigned int BufferedTxLength = 0;
	unsigned char BufferedTxCmd[8] = {0};
	
	REALTEK_DISEQC_INFO("\033[1;32;31m" "realtek_Diseqc_SetDiseqc2p0Cmd\n" "\033[m");
	
	if(*(Data+0) == 0xe0 && *(Data+1) == 0x10 && (*(Data+2) == 0x5a || *(Data+2) == 0x5c))
	{
		REALTEK_DISEQC_INFO("\033[1;32;35m" "Unicable v1 Enabled\n" "\033[m");
		gDvbSxBsSpecialParams.u8UnicableEnabled = 1;
	}
	else if(*(Data+0) == 0x70 || *(Data+0) == 0x71)
	{
		REALTEK_DISEQC_INFO("\033[1;32;35m" "Unicable v2 Enabled\n" "\033[m");
		gDvbSxBsSpecialParams.u8UnicableEnabled = 1;
	}
	else if(*(Data+0) >= 0x7A && *(Data+0) <= 0x7F)
	{
		REALTEK_DISEQC_INFO("\033[1;32;35m" "Unicable v2 RX Enabled\n" "\033[m");
		RxRetryCnt = DISEQC_RX_RETRY_CNT;
	}
	else
	{
		REALTEK_DISEQC_INFO("\033[1;32;35m" "Unicable Disabled\n" "\033[m");
		gDvbSxBsSpecialParams.u8UnicableEnabled = 0;
	}
	
	if (!(rtd_inl(SYS_REG_SYS_CLKEN2_reg) & SYS_REG_SYS_CLKEN2_clken_dtv_demod_mask))
	{
		REALTEK_DISEQC_INFO("Enable DTV DEMOD clk\n");
		CRT_CLK_OnOff(DTVDEMOD, CLK_ON, NULL);
	}
	if (!(rtd_inl(SYS_REG_SYS_CLKEN0_reg) & SYS_REG_SYS_CLKEN0_clken_atb_demod_mask))
	{
		REALTEK_DISEQC_INFO("Enable DTV ATB clk\n");
		CRT_CLK_OnOff(DTVATB, CLK_ON, NULL);
	}
	if (!(rtd_inl(SYS_REG_SYS_CLKEN0_reg) & SYS_REG_SYS_CLKEN0_clken_diseqc_mask))
	{
		REALTEK_DISEQC_INFO("Enable DISEQC clk\n");
		CRT_CLK_OnOff(DISEQC, CLK_ON, NULL);
	}
	
	BufferedTxLength = *DataLength;
	memcpy(BufferedTxCmd, Data, sizeof(unsigned char)*BufferedTxLength);
	
diseqc_rx_retry:
	
	rtd_maskl(DISEQC_DSCU_FLT_reg, ~(DISEQC_DSCU_FLT_src_mask), DISEQC_DSCU_FLT_src(0x1));//Sel RX DSC_IN pin
	
	rtd_maskl(DISEQC_DSCU_FLT_reg, ~(DISEQC_DSCU_FLT_threshold_mask), DISEQC_DSCU_FLT_threshold(0x280));//Modify Spike TH
	
	//rtd_outl(MIS_GPIO_GP2DIR_reg, MIS_GPIO_GP2DIR_gpdir80(0x1));//DSC_IN GPIO output sel //Need modify to correct setting
	
	rtd_maskl(DISEQC_DSCU_TX_CTL_reg, ~(DISEQC_DSCU_TX_CTL_tx_go_mask), DISEQC_DSCU_TX_CTL_tx_go(0x0));//Tx Off
	
	rtd_maskl(DISEQC_DSCU_TX_CTL_reg, ~(DISEQC_DSCU_TX_CTL_loop_back_mode_mask), DISEQC_DSCU_TX_CTL_loop_back_mode(0x1));//En loop back mode
	
	#ifdef CONFIG_CUSTOMER_TV006
	rtd_maskl(DISEQC_DSCU_CTL_reg, ~(DISEQC_DSCU_CTL_tx_env_mode_mask), DISEQC_DSCU_CTL_tx_env_mode(0x1));//Tx envelop mode
	#else
	rtd_maskl(DISEQC_DSCU_CTL_reg, ~(DISEQC_DSCU_CTL_tx_env_mode_mask), DISEQC_DSCU_CTL_tx_env_mode(0x0));//Tx pulse mode
	#endif
	
	rtd_maskl(DISEQC_DSCU_TX_CTL_reg, ~(DISEQC_DSCU_TX_CTL_xmt_mode_mask), DISEQC_DSCU_TX_CTL_xmt_mode(0x2));//Diseqc Cmd Mode
	
	//rtd_maskl(DISEQC_DSCU_TX_CTL_reg, ~(DISEQC_DSCU_TX_CTL_tone_div_mask), DISEQC_DSCU_TX_CTL_tone_div(0x27f));//22k freq divider
	
	rtd_maskl(DISEQC_DSCU_TX_CTL_reg, ~(DISEQC_DSCU_TX_CTL_tx_pty_mask), DISEQC_DSCU_TX_CTL_tx_pty(0x1));//Odd parity
	
	rtd_maskl(DISEQC_DSCU_TX_CTL_reg, ~(DISEQC_DSCU_TX_CTL_msg_len_mask), DISEQC_DSCU_TX_CTL_msg_len(BufferedTxLength-1));//Set Data Length(0:1Byte, 1:2Byte, ...)
	
	for(DataCount=0; DataCount<BufferedTxLength; DataCount++)
	{
		rtd_outl((DISEQC_DSCU_TX_BUF0_reg+(4*DataCount)), BufferedTxCmd[DataCount]);
	}
	
	realtek_diseqc_PrintTxCmd(BufferedTxLength);
	
	#ifdef GPIO_HI_LO
	realtek_Lnb_Dc_Enable(1);
	#endif

	rtd_maskl(DISEQC_DSCU_BIT_DCT_COND_reg, ~(DISEQC_DSCU_BIT_DCT_COND_bit_dct_cond2_th_mask), DISEQC_DSCU_BIT_DCT_COND_bit_dct_cond2_th(0x3));//Set bit_dct_cond2_th = 3
	rtd_maskl(DISEQC_DSCU_BIT_DCT_COND_reg, ~(DISEQC_DSCU_BIT_DCT_COND_bit_dct_cond2_mask), DISEQC_DSCU_BIT_DCT_COND_bit_dct_cond2(0x1));//Enable RX bit_dct_cond2

	//delay for td => move to APP layer
	//mdelay(10);
	
	REALTEK_DISEQC_INFO("CMD Delay %u ms\n", (135*BufferedTxLength+5)/10);
	
	rtd_maskl(DISEQC_DSCU_TX_CTL_reg, ~(DISEQC_DSCU_TX_CTL_tx_go_mask), DISEQC_DSCU_TX_CTL_tx_go(0x1));//Tx Go
	
	//delay for command finish
	mdelay((135*BufferedTxLength+5)/10);
	
	#ifdef GPIO_HI_LO
	realtek_Lnb_Dc_Enable(0);
	#endif
	
	*DataLength = 0;
	
	//rtd_maskl(PINMUX_GPIO_LEFT_CFG_5_reg, ~(PINMUX_GPIO_LEFT_CFG_5_gpio_80_ps_mask), PINMUX_GPIO_LEFT_CFG_5_gpio_80_ps(0x4);//DSC_IN pinmux enable  //Need modify to correct setting
	//rtd_outl(MIS_GPIO_GP2DIR_reg, (MIS_GPIO_GP2DIR_gpdir80_mask|MIS_GPIO_GP2DIR_write_data_mask));//DSC_IN GPIO input sel  //Need modify to correct setting
	
	stime = tv_osal_time();
	while((tv_osal_time() - stime) < 50)//RX time out 50 ms
	{
		RxRecDone1 = rtd_inl(DISEQC_DSCU_INTR_reg);
		RxRecDone2 = rtd_inl(DISEQC_DSCU_RX_STATUS_reg);
		if((RxRecDone2 & DISEQC_DSCU_RX_STATUS_eorm_mask) == DISEQC_DSCU_RX_STATUS_eorm_mask)
		{
			mdelay(60);
			RxDataCnt = DISEQC_DSCU_RX_STATUS_get_rx_count(rtd_inl(DISEQC_DSCU_RX_STATUS_reg));
			*DataLength = RxDataCnt;
			
			REALTEK_DISEQC_INFO("\033[1;32;33m" "Diseqc RX Done: %u (0x%02x) (0x%02x)\n" "\033[m", (RxRecDone2 & DISEQC_DSCU_RX_STATUS_eorm_mask)>>1, RxRecDone1, RxRecDone2);
			
			for(DataCount=0; DataCount<RxDataCnt; DataCount++)
			{
				*(Data+DataCount) = rtd_inl(DISEQC_DSCU_RX_BUF0_reg+(4*DataCount)) & DISEQC_DSCU_RX_BUF0_byte0_mask;
			}
			realtek_diseqc_PrintRxCmd(*DataLength, Data);
			break;
		}
		else
		{
			REALTEK_DISEQC_DBG("\033[1;32;34m" "%u (0x%02x) (0x%02x) Waiting....\n" "\033[m", (RxRecDone2 & DISEQC_DSCU_RX_STATUS_eorm_mask)>>1, RxRecDone1, RxRecDone2);
		}
		tv_osal_msleep(5);
	}
	//rtd_maskl(PINMUX_GPIO_LEFT_CFG_5_reg, ~(PINMUX_GPIO_LEFT_CFG_5_gpio_80_ps_mask), PINMUX_GPIO_LEFT_CFG_5_gpio_80_ps(0xf));//DSC_IN pinmux disable  //Need modify to correct setting
	rtd_maskl(DISEQC_DSCU_TX_CTL_reg, ~(DISEQC_DSCU_TX_CTL_tx_go_mask), DISEQC_DSCU_TX_CTL_tx_go(0x0));//Tx Off
	
	//delay for ta => move to APP layer
	//mdelay(10);
	
	#ifdef GPIO_HI_LO
	realtek_Lnb_Dc_Enable(1);
	#endif
	
	if(RxRetryCnt > 0 && *(Data+0) != 0x74)
	{
		REALTEK_DISEQC_INFO("\033[1;32;34m" "RX Data0 = 0x%02x(!=0x74) DISEQC RX RETRY!!! RxRetryCnt = %u\n" "\033[m", *(Data+0), RxRetryCnt);
		CRT_CLK_OnOff(DISEQC, CLK_OFF, NULL);
		tv_osal_msleep(1);
		CRT_CLK_OnOff(DISEQC, CLK_ON, NULL);
		tv_osal_msleep(DISEQC_RX_RETRY_DELAY_TIME);
		RxRetryCnt--;
		goto diseqc_rx_retry;
	}
	
	return FUNCTION_SUCCESS;
}

int
realtek_Diseqc_SetDiseqcUnModToneBurst(void)
{
	REALTEK_DISEQC_INFO("\033[1;32;31m" "realtek_Diseqc_SetDiseqcUnModToneBurst\n" "\033[m");
	
	if (!(rtd_inl(SYS_REG_SYS_CLKEN2_reg) & SYS_REG_SYS_CLKEN2_clken_dtv_demod_mask))
	{
		REALTEK_DISEQC_INFO("Enable DTV DEMOD clk\n");
		CRT_CLK_OnOff(DTVDEMOD, CLK_ON, NULL);
	}
	if (!(rtd_inl(SYS_REG_SYS_CLKEN0_reg) & SYS_REG_SYS_CLKEN0_clken_atb_demod_mask))
	{
		REALTEK_DISEQC_INFO("Enable DTV ATB clk\n");
		CRT_CLK_OnOff(DTVATB, CLK_ON, NULL);
	}
	if (!(rtd_inl(SYS_REG_SYS_CLKEN0_reg) & SYS_REG_SYS_CLKEN0_clken_diseqc_mask))
	{
		REALTEK_DISEQC_INFO("Enable DISEQC clk\n");
		CRT_CLK_OnOff(DISEQC, CLK_ON, NULL);
	}
	
	rtd_maskl(DISEQC_DSCU_TX_CTL_reg, ~(DISEQC_DSCU_TX_CTL_tx_go_mask), DISEQC_DSCU_TX_CTL_tx_go(0x0));//Tx Off
	
	rtd_maskl(DISEQC_DSCU_TX_CTL_reg, ~(DISEQC_DSCU_TX_CTL_loop_back_mode_mask), DISEQC_DSCU_TX_CTL_loop_back_mode(0x1));//En loop back mode
	
	#ifdef CONFIG_CUSTOMER_TV006
	rtd_maskl(DISEQC_DSCU_CTL_reg, ~(DISEQC_DSCU_CTL_tx_env_mode_mask), DISEQC_DSCU_CTL_tx_env_mode(0x1));//Tx envelop mode
	#else
	rtd_maskl(DISEQC_DSCU_CTL_reg, ~(DISEQC_DSCU_CTL_tx_env_mode_mask), DISEQC_DSCU_CTL_tx_env_mode(0x0));//Tx pulse mode
	#endif
	
	rtd_maskl(DISEQC_DSCU_TX_CTL_reg, ~(DISEQC_DSCU_TX_CTL_xmt_mode_mask), DISEQC_DSCU_TX_CTL_xmt_mode(0x1));//Un-Modulated Tone Burst Mode
	
	//rtd_maskl(DISEQC_DSCU_TX_CTL_reg, ~(DISEQC_DSCU_TX_CTL_tone_div_mask), DISEQC_DSCU_TX_CTL_tone_div(0x27f));//22k freq divider
	
	tv_osal_msleep(1); 
	
	rtd_maskl(DISEQC_DSCU_TX_CTL_reg, ~(DISEQC_DSCU_TX_CTL_tx_go_mask), DISEQC_DSCU_TX_CTL_tx_go(0x1));//Tx Go
	
	tv_osal_msleep(14);
	
	return FUNCTION_SUCCESS;
}

int
realtek_Diseqc_SetDiseqcModToneBurst(void)
{
	REALTEK_DISEQC_INFO("\033[1;32;31m" "realtek_Diseqc_SetDiseqcModToneBurst\n" "\033[m");
	
	if (!(rtd_inl(SYS_REG_SYS_CLKEN2_reg) & SYS_REG_SYS_CLKEN2_clken_dtv_demod_mask))
	{
		REALTEK_DISEQC_INFO("Enable DTV DEMOD clk\n");
		CRT_CLK_OnOff(DTVDEMOD, CLK_ON, NULL);
	}
	if (!(rtd_inl(SYS_REG_SYS_CLKEN0_reg) & SYS_REG_SYS_CLKEN0_clken_atb_demod_mask))
	{
		REALTEK_DISEQC_INFO("Enable DTV ATB clk\n");
		CRT_CLK_OnOff(DTVATB, CLK_ON, NULL);
	}
	if (!(rtd_inl(SYS_REG_SYS_CLKEN0_reg) & SYS_REG_SYS_CLKEN0_clken_diseqc_mask))
	{
		REALTEK_DISEQC_INFO("Enable DISEQC clk\n");
		CRT_CLK_OnOff(DISEQC, CLK_ON, NULL);
	}
	
	rtd_maskl(DISEQC_DSCU_TX_CTL_reg, ~(DISEQC_DSCU_TX_CTL_tx_go_mask), DISEQC_DSCU_TX_CTL_tx_go(0x0));//Tx Off
	
	rtd_maskl(DISEQC_DSCU_TX_CTL_reg, ~(DISEQC_DSCU_TX_CTL_loop_back_mode_mask), DISEQC_DSCU_TX_CTL_loop_back_mode(0x1));//En loop back mode
	
	#ifdef CONFIG_CUSTOMER_TV006
	rtd_maskl(DISEQC_DSCU_CTL_reg, ~(DISEQC_DSCU_CTL_tx_env_mode_mask), DISEQC_DSCU_CTL_tx_env_mode(0x1));//Tx envelop mode
	#else
	rtd_maskl(DISEQC_DSCU_CTL_reg, ~(DISEQC_DSCU_CTL_tx_env_mode_mask), DISEQC_DSCU_CTL_tx_env_mode(0x0));//Tx pulse mode
	#endif
	
	rtd_maskl(DISEQC_DSCU_TX_CTL_reg, ~(DISEQC_DSCU_TX_CTL_xmt_mode_mask), DISEQC_DSCU_TX_CTL_xmt_mode(0x0));//Modulated Tone Burst Mode
	
	//rtd_maskl(DISEQC_DSCU_TX_CTL_reg, ~(DISEQC_DSCU_TX_CTL_tone_div_mask), DISEQC_DSCU_TX_CTL_tone_div(0x27f));//22k freq divider
	
	tv_osal_msleep(1); 
	
	rtd_maskl(DISEQC_DSCU_TX_CTL_reg, ~(DISEQC_DSCU_TX_CTL_tx_go_mask), DISEQC_DSCU_TX_CTL_tx_go(0x1));//Tx Go
	
	tv_osal_msleep(14);
	
	return FUNCTION_SUCCESS;
}

int
realtek_Diseqc_SetDiseqcToneBurst(DISEQC_TONE_MODE mode)
{
	REALTEK_DISEQC_INFO("\033[1;32;31m" "realtek_Diseqc_SetDiseqcToneBurst (%u)\n" "\033[m", (unsigned int)mode);
	
	if(mode == DISEQC_TONE_MODE_A)
		realtek_Diseqc_SetDiseqcUnModToneBurst();
	else
		realtek_Diseqc_SetDiseqcModToneBurst();
	
	return FUNCTION_SUCCESS;
}

int
realtek_Lnb_Dc_Enable(unsigned char enable)
{
	int pinindex = 0;
	//int ret = 0;
	int invert = 0;
	RTK_GPIO_ID s_gpio;
	unsigned long long param;
	
	if (pcb_mgr_get_enum_info_byname("PIN_LNB_DC_EN", &param) == 0)
	{
		if (GET_PIN_TYPE(param) == PCB_PIN_TYPE_ISO_GPIO) {
			pinindex = GET_PIN_INDEX(param);
			invert = GET_GPIO_INVERT(param);
			enable = (invert) ? (!enable) : (enable);
			s_gpio = rtk_gpio_id(ISO_GPIO, pinindex);
			
			//REALTEK_DISEQC_INFO("ISO_GPIO gpio_num: %d\n", pinindex);
			//REALTEK_DISEQC_INFO("enable: %d\n", enable);
			
			//ret = rtk_gpio_output(s_gpio, enable);
			//rtk_gpio_set_dir(s_gpio,1);
			
			//if (ret < 0)
			//	return 0;
		}
		else if (GET_PIN_TYPE(param) == PCB_PIN_TYPE_GPIO) {
			pinindex = GET_PIN_INDEX(param);
			invert = GET_GPIO_INVERT(param);
			enable = (invert) ? (!enable) : (enable);
			s_gpio = rtk_gpio_id(MIS_GPIO, pinindex);
			
			//REALTEK_DISEQC_INFO("MISC_GPIO gpio_num: %d\n", pinindex);
			
			//ret = rtk_gpio_output(s_gpio, enable);
			//rtk_gpio_set_dir(s_gpio,1);
			
			//if (ret < 0)
			//	return 0;
		}
	}
	return 1;
}

void
realtek_diseqc_PrintTxCmd(unsigned int DataLength)
{
	switch(DataLength)
	{
		case 8:
		REALTEK_DISEQC_INFO("\033[1;32;32m" "(TX Length: %u) 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n" "\033[m", 
			DataLength, rtd_inl(DISEQC_DSCU_TX_BUF0_reg), rtd_inl(DISEQC_DSCU_TX_BUF1_reg), rtd_inl(DISEQC_DSCU_TX_BUF2_reg), rtd_inl(DISEQC_DSCU_TX_BUF3_reg), rtd_inl(DISEQC_DSCU_TX_BUF4_reg), rtd_inl(DISEQC_DSCU_TX_BUF5_reg), rtd_inl(DISEQC_DSCU_TX_BUF6_reg), rtd_inl(DISEQC_DSCU_TX_BUF7_reg));
		break;
		case 7:
		REALTEK_DISEQC_INFO("\033[1;32;32m" "(TX Length: %u) 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n" "\033[m", 
			DataLength, rtd_inl(DISEQC_DSCU_TX_BUF0_reg), rtd_inl(DISEQC_DSCU_TX_BUF1_reg), rtd_inl(DISEQC_DSCU_TX_BUF2_reg), rtd_inl(DISEQC_DSCU_TX_BUF3_reg), rtd_inl(DISEQC_DSCU_TX_BUF4_reg), rtd_inl(DISEQC_DSCU_TX_BUF5_reg), rtd_inl(DISEQC_DSCU_TX_BUF6_reg));
		break;
		case 6:
		REALTEK_DISEQC_INFO("\033[1;32;32m" "(TX Length: %u) 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n" "\033[m", 
			DataLength, rtd_inl(DISEQC_DSCU_TX_BUF0_reg), rtd_inl(DISEQC_DSCU_TX_BUF1_reg), rtd_inl(DISEQC_DSCU_TX_BUF2_reg), rtd_inl(DISEQC_DSCU_TX_BUF3_reg), rtd_inl(DISEQC_DSCU_TX_BUF4_reg), rtd_inl(DISEQC_DSCU_TX_BUF5_reg));
		break;
		case 5:
		REALTEK_DISEQC_INFO("\033[1;32;32m" "(TX Length: %u) 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n" "\033[m", 
			DataLength, rtd_inl(DISEQC_DSCU_TX_BUF0_reg), rtd_inl(DISEQC_DSCU_TX_BUF1_reg), rtd_inl(DISEQC_DSCU_TX_BUF2_reg), rtd_inl(DISEQC_DSCU_TX_BUF3_reg), rtd_inl(DISEQC_DSCU_TX_BUF4_reg));
		break;
		case 4:
		REALTEK_DISEQC_INFO("\033[1;32;32m" "(TX Length: %u) 0x%02x 0x%02x 0x%02x 0x%02x\n" "\033[m", 
			DataLength, rtd_inl(DISEQC_DSCU_TX_BUF0_reg), rtd_inl(DISEQC_DSCU_TX_BUF1_reg), rtd_inl(DISEQC_DSCU_TX_BUF2_reg), rtd_inl(DISEQC_DSCU_TX_BUF3_reg));
		break;
		case 3:
		REALTEK_DISEQC_INFO("\033[1;32;32m" "(TX Length: %u) 0x%02x 0x%02x 0x%02x\n" "\033[m", 
			DataLength, rtd_inl(DISEQC_DSCU_TX_BUF0_reg), rtd_inl(DISEQC_DSCU_TX_BUF1_reg), rtd_inl(DISEQC_DSCU_TX_BUF2_reg));
		break;
		case 2:
		REALTEK_DISEQC_INFO("\033[1;32;32m" "(TX Length: %u) 0x%02x 0x%02x\n" "\033[m", 
			DataLength, rtd_inl(DISEQC_DSCU_TX_BUF0_reg), rtd_inl(DISEQC_DSCU_TX_BUF1_reg));
		break;
		case 1:
		REALTEK_DISEQC_INFO("\033[1;32;32m" "(TX Length: %u) 0x%02x \n" "\033[m", 
			DataLength, rtd_inl(DISEQC_DSCU_TX_BUF0_reg));
		break;
		default:
		REALTEK_DISEQC_INFO("\033[1;32;32m" "(wrong data length) ERROR!\n" "\033[m");
		break;
	}
}

void
realtek_diseqc_PrintRxCmd(unsigned int DataLength, unsigned char *Data)
{
	switch(DataLength)
	{
		case 8:
		REALTEK_DISEQC_INFO("\033[1;32;33m" "(RX Length: %u) 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x (REG: 0x%03x 0x%03x 0x%03x 0x%03x 0x%03x 0x%03x 0x%03x 0x%03x)\n" "\033[m",
			DataLength, *(Data+0), *(Data+1), *(Data+2), *(Data+3), *(Data+4), *(Data+5), *(Data+6), *(Data+7),
			rtd_inl(DISEQC_DSCU_RX_BUF0_reg), rtd_inl(DISEQC_DSCU_RX_BUF1_reg), rtd_inl(DISEQC_DSCU_RX_BUF2_reg), rtd_inl(DISEQC_DSCU_RX_BUF3_reg), rtd_inl(DISEQC_DSCU_RX_BUF4_reg), rtd_inl(DISEQC_DSCU_RX_BUF5_reg), rtd_inl(DISEQC_DSCU_RX_BUF6_reg), rtd_inl(DISEQC_DSCU_RX_BUF7_reg));
		break;
		case 7:
		REALTEK_DISEQC_INFO("\033[1;32;33m" "(RX Length: %u) 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x (REG: 0x%03x 0x%03x 0x%03x 0x%03x 0x%03x 0x%03x 0x%03x)\n" "\033[m",
			DataLength, *(Data+0), *(Data+1), *(Data+2), *(Data+3), *(Data+4), *(Data+5), *(Data+6),
			rtd_inl(DISEQC_DSCU_RX_BUF0_reg), rtd_inl(DISEQC_DSCU_RX_BUF1_reg), rtd_inl(DISEQC_DSCU_RX_BUF2_reg), rtd_inl(DISEQC_DSCU_RX_BUF3_reg), rtd_inl(DISEQC_DSCU_RX_BUF4_reg), rtd_inl(DISEQC_DSCU_RX_BUF5_reg), rtd_inl(DISEQC_DSCU_RX_BUF6_reg));
		break;
		case 6:
		REALTEK_DISEQC_INFO("\033[1;32;33m" "(RX Length: %u) 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x (REG: 0x%03x 0x%03x 0x%03x 0x%03x 0x%03x 0x%03x)\n" "\033[m",
			DataLength, *(Data+0), *(Data+1), *(Data+2), *(Data+3), *(Data+4), *(Data+5),
			rtd_inl(DISEQC_DSCU_RX_BUF0_reg), rtd_inl(DISEQC_DSCU_RX_BUF1_reg), rtd_inl(DISEQC_DSCU_RX_BUF2_reg), rtd_inl(DISEQC_DSCU_RX_BUF3_reg), rtd_inl(DISEQC_DSCU_RX_BUF4_reg), rtd_inl(DISEQC_DSCU_RX_BUF5_reg));
		break;
		case 5:
		REALTEK_DISEQC_INFO("\033[1;32;33m" "(RX Length: %u) 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x (REG: 0x%03x 0x%03x 0x%03x 0x%03x 0x%03x)\n" "\033[m",
			DataLength, *(Data+0), *(Data+1), *(Data+2), *(Data+3), *(Data+4),
			rtd_inl(DISEQC_DSCU_RX_BUF0_reg), rtd_inl(DISEQC_DSCU_RX_BUF1_reg), rtd_inl(DISEQC_DSCU_RX_BUF2_reg), rtd_inl(DISEQC_DSCU_RX_BUF3_reg), rtd_inl(DISEQC_DSCU_RX_BUF4_reg));
		break;
		case 4:
		REALTEK_DISEQC_INFO("\033[1;32;33m" "(RX Length: %u) 0x%02x 0x%02x 0x%02x 0x%02x (REG: 0x%03x 0x%03x 0x%03x 0x%03x)\n" "\033[m",
			DataLength, *(Data+0), *(Data+1), *(Data+2), *(Data+3),
			rtd_inl(DISEQC_DSCU_RX_BUF0_reg), rtd_inl(DISEQC_DSCU_RX_BUF1_reg), rtd_inl(DISEQC_DSCU_RX_BUF2_reg), rtd_inl(DISEQC_DSCU_RX_BUF3_reg));
		break;
		case 3:
		REALTEK_DISEQC_INFO("\033[1;32;33m" "(RX Length: %u) 0x%02x 0x%02x 0x%02x (REG: 0x%03x 0x%03x 0x%03x)\n" "\033[m",
			DataLength, *(Data+0), *(Data+1), *(Data+2),
			rtd_inl(DISEQC_DSCU_RX_BUF0_reg), rtd_inl(DISEQC_DSCU_RX_BUF1_reg), rtd_inl(DISEQC_DSCU_RX_BUF2_reg));
		break;
		case 2:
		REALTEK_DISEQC_INFO("\033[1;32;33m" "(RX Length: %u) 0x%02x 0x%02x (REG: 0x%03x 0x%03x)\n" "\033[m",
			DataLength, *(Data+0), *(Data+1),
			rtd_inl(DISEQC_DSCU_RX_BUF0_reg), rtd_inl(DISEQC_DSCU_RX_BUF1_reg));
		break;
		case 1:
		REALTEK_DISEQC_INFO("\033[1;32;33m" "(RX Length: %u) 0x%02x(REG: 0x%03x)\n" "\033[m",
			DataLength, *(Data+0),
			rtd_inl(DISEQC_DSCU_RX_BUF0_reg));
		break;
		default:
		REALTEK_DISEQC_INFO("\033[1;32;32m" "(wrong data length) ERROR!\n" "\033[m");
		break;
	}
}
