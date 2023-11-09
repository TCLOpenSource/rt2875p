#include <io.h>
#include <mach/rtk_platform.h>

#include <rtk_kdriver/tvscalercontrol/hdmitx/hdmitx_lib.h>
#include <rtk_kdriver/tvscalercontrol/hdmitx/hdmitx.h>
#include <rtk_kdriver/tvscalercontrol/scaler/scalerstruct.h>
#ifdef CONFIG_RTK_HDMI_RX
#include <rtk_kdriver/tvscalercontrol/hdmirx/hdmifun.h>
#endif
static EnumHDMITXDscSrc hdmitxDscSrc = DSC_SRC_TXSOC;
static unsigned int hdmitxRlinkStatus=0;
static unsigned char hdmitxReNewIndex_in=0, hdmitxReNewIndex_out=0;
extern unsigned short ScalerHdmiTxGetOutputInfo(SLR_INPUT_INFO infoList);
extern void ScalerHdmiTxPhy_SetFeeTable(HDMI_TX_FFE_TABLE_LIST tableId);
extern unsigned char ScalerHdmiTxGetStreamStateReady(void);
extern void ScalerHdmiTxSetMute(unsigned char enable);
extern unsigned char ScalerHdmiTxGetMute(void);
//extern HOST_PLAFTORM_TYPE ScalerHdmiTxGetH5DPlatformType(void);

unsigned int HDMITX_Get_Output_Width(void)
{
    return ScalerHdmiTxGetOutputInfo(SLR_INPUT_H_LEN);
}

EnumHDMITXDscSrc HDMITX_Get_DscSrc(void)
{
    return hdmitxDscSrc;
}

unsigned int HDMITX_Get_Output_Height(void)
{
    return ScalerHdmiTxGetOutputInfo(SLR_INPUT_V_LEN);
}

// HZ = HDMITX_Get_Output_VFreq()/10
unsigned int HDMITX_Get_Output_VFreq(void)
{
    return ScalerHdmiTxGetOutputInfo(SLR_INPUT_V_FREQ);
}

// 0: RGB, 1: YUV422, 2: YUV444, 3: YUV 420
unsigned int HDMITX_Get_Output_ColorSpace(void)
{
    return ScalerHdmiTxGetOutputInfo(SLR_INPUT_COLOR_SPACE);
}

// 8~12 bits
unsigned int HDMITX_Get_Output_ColorDepth(void)
{
    return ScalerHdmiTxGetOutputInfo(SLR_INPUT_COLOR_DEPTH);
}

void HDMITX_Set_DscSrc(EnumHDMITXDscSrc srcType)
{
    if((unsigned char) srcType > DSC_SRC_DISPD)
        return;

    if(hdmitxDscSrc != srcType){
        NoteMessageHDMITx("[HDMITX] DSC Input %d->%d\n", hdmitxDscSrc, srcType);
        hdmitxDscSrc = srcType;
    }

    return;
}


void HDMITX_Set_ReNewIndex_in(unsigned int index)
{
    ErrorMessageHDMITx("[HDMITX] hdmitxReNewIndex_in=%d->%d\n", hdmitxReNewIndex_in, index);
    hdmitxReNewIndex_in = index;
    return;
}

unsigned char HDMITX_Get_ReNewIndex_in(void)
{
    return hdmitxReNewIndex_in;
}


void HDMITX_Set_ReNewIndex_out(unsigned int index)
{
    ErrorMessageHDMITx("[HDMITX] hdmitxReNewIndex_out=%d->%d\n", hdmitxReNewIndex_out, index);
    hdmitxReNewIndex_out = index;
    return;
}

unsigned char HDMITX_Get_ReNewIndex_out(void)
{
    return hdmitxReNewIndex_out;
}



void HDMITX_Set_FfeTable(HDMI_TX_FFE_TABLE_LIST tableId)
{
    ScalerHdmiTxPhy_SetFeeTable(tableId);
    return;
}


void HDMITX_Set_MuteEn(unsigned char enable)
{
    ScalerHdmiTxSetMute(enable);
    return;
}

unsigned char HDMITX_Get_MuteEn(void)
{
    return ScalerHdmiTxGetMute();
}


void HDMITX_Set_HdmiTxRLinkStatus(unsigned int status)
{
    if(hdmitxRlinkStatus != status)
        NoteMessageHDMITx("[HDMITX] HDMITX RLink Status %d->%d\n", hdmitxRlinkStatus, status);
    hdmitxRlinkStatus = status;
    return;
}


unsigned int HDMITX_Get_HdmiTxRLinkStatus(void)
{
    return hdmitxRlinkStatus;
}

unsigned char HDMITX_Get_HdmiTxStreamReady(void)
{
    return ScalerHdmiTxGetStreamStateReady();
}


unsigned char ScalerStreamAudioGetChannelCount(EnumOutputPort ucOutputPort)
{
#ifdef ENABLE_MARK2_HDMITX_AUDIO
#ifdef CONFIG_RTK_HDMI_RX
    HDMI_DRVIF_AUDIO_STATUS_T pHDMIRX_AudioStatus;
    //return 1;//

    if(drvif_Hdmi_GetAudioStatus(&pHDMIRX_AudioStatus))
    {
        return pHDMIRX_AudioStatus.ch_num -1;
    }
#endif
#endif // #ifdef MARK2_HDMITX_AUDIO
    return getHdmitxAudioChanel();//
}
unsigned char ScalerStreamAudioGetCodingType(EnumOutputPort ucOutputPort)
{

#ifdef ENABLE_MARK2_HDMITX_AUDIO
#ifdef CONFIG_RTK_HDMI_RX
    HDMI_DRVIF_AUDIO_STATUS_T pHDMIRX_AudioStatus;
    if(drvif_Hdmi_GetAudioStatus(&pHDMIRX_AudioStatus))
    {
        if(pHDMIRX_AudioStatus.is_received_hbr)
            return _HBR_ASP;
    }
#endif
#endif // #ifdef ENABLE_MARK2_HDMITX_AUDIO
    return getHdmitxAudioType();//

}
unsigned char ScalerStreamAudioGetSamplingFrequency(EnumOutputPort ucOutputPort)
{
#ifdef ENABLE_MARK2_HDMITX_AUDIO
#ifdef CONFIG_RTK_HDMI_RX
    //HDMI_DRVIF_AUDIO_STATUS_T pHDMIRX_AudioStatus;

    if(drvif_Hdmi_GetAudioStatus(&pHDMIRX_AudioStatus))
    {
        switch (pHDMIRX_AudioStatus.acr_sample_freq)
        {
            case 32000:
                return _AUDIO_FREQ_32K;
            case 64000:
                return _AUDIO_FREQ_64K;
            case 128000:
                return _AUDIO_FREQ_128K;
            case 44100:
                return _AUDIO_FREQ_44_1K;
            case 88200:
                return _AUDIO_FREQ_88_2K;
            case 176400:
                return _AUDIO_FREQ_176_4K;
            case 48000:
                return _AUDIO_FREQ_48K;
            case 96000:
                return _AUDIO_FREQ_96K;
            case 192000:
                return _AUDIO_FREQ_192K;
            case 256000:
                return _AUDIO_FREQ_256K;
            case 352800:
                return _AUDIO_FREQ_352_8K;
            case 384000:
                return _AUDIO_FREQ_384K;
            case 512000:
                return _AUDIO_FREQ_512K;
            case 705600:
                return _AUDIO_FREQ_705_6K;
            case 768000:
                return _AUDIO_FREQ_768K;
            default:
                return _AUDIO_FREQ_48K;
                break;

        }
    }
#endif
#endif // #ifdef ENABLE_MARK2_HDMITX_AUDIO
	switch (getHdmitxAudioFreq())
	{
		case 32000:
			return _AUDIO_FREQ_32K;
		case 64000:
			return _AUDIO_FREQ_64K;
		case 128000:
			return _AUDIO_FREQ_128K;
		case 44100:
			return _AUDIO_FREQ_44_1K;
		case 88200:
			return _AUDIO_FREQ_88_2K;
		case 176400:
			return _AUDIO_FREQ_176_4K;
		case 48000:
			return _AUDIO_FREQ_48K;
		case 96000:
			return _AUDIO_FREQ_96K;
		case 192000:
			return _AUDIO_FREQ_192K;
		case 256000:
			return _AUDIO_FREQ_256K;
		case 352800:
			return _AUDIO_FREQ_352_8K;
		case 384000:
			return _AUDIO_FREQ_384K;
		case 512000:
			return _AUDIO_FREQ_512K;
		case 705600:
			return _AUDIO_FREQ_705_6K;
		case 768000:
			return _AUDIO_FREQ_768K;
		default:
			return _AUDIO_FREQ_NO_AUDIO;
			break;
	}

}
unsigned char ScalerStreamGetAudioMuteFlag(EnumOutputPort ucOutputPort)
{
#ifdef ENABLE_MARK2_HDMITX_AUDIO
#ifdef CONFIG_RTK_HDMI_RX
    HDMI_DRVIF_AUDIO_STATUS_T pHDMIRX_AudioStatus;
    if(drvif_Hdmi_GetAudioStatus(&pHDMIRX_AudioStatus))
    {
        if(!pHDMIRX_AudioStatus.is_rx_output_audio)
        {
            return _TRUE;
        }
    }
#endif
#endif // #ifdef ENABLE_MARK2_HDMITX_AUDIO
    return getHdmitxAudioMute();//
}

#if 0 // for TV006 platform
unsigned char HdmiTx_Get_H5D_PlatformType(void) {
    HOST_PLAFTORM_TYPE platformType;

    platformType = ScalerHdmiTxGetH5DPlatformType();

    return platformType;
}
#endif
