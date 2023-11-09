#include "hdmi_common.h"
#include "hdmi_hdr.h"

#if defined(CONFIG_ENABLE_DOLBY_VISION_HDMI_AUTO_DETECT)|| defined(CONFIG_ENABLE_HDR10_HDMI_AUTO_DETECT)
#include <tvscalercontrol/scalerdrv/scaler_hdr_ctrl.h>
#endif


#ifndef _ENABLE
#define _ENABLE			1
#endif

#ifndef _DISABLE
#define _DISABLE		0
#endif

extern HDMI_PORT_INFO_T hdmi_rx[HDMI_PORT_TOTAL_NUM];

void newbase_hdmi_dv_hdr_enable(unsigned char detectEn)
{
#ifdef CONFIG_ENABLE_DOLBY_VISION_HDMI_AUTO_DETECT

  #ifdef CONFIG_FORCE_RUN_I3DDMA
	if (!newbase_hdmi_is_dispsrc(SLR_MAIN_DISPLAY))
		return;
  #endif
  
	if (detectEn == _ENABLE)
	{//enable
		if (drvif_Hdmi_GetColorSpace() == COLOR_RGB)
		{
			HDMI_PRINTF("[DolbyVision test COLOR_RGB] HDMI Auto Detet enable...\n");
			Scaler_HDR_DolbyVision_Hdmi_SetDetectEnable(_ENABLE, _DISABLE);
		}
	}
	else
	{//disable
		Scaler_HDR_DolbyVision_Hdmi_SetDetectEnable(_DISABLE, _DISABLE);
	}
#endif // CONFIG_ENABLE_DOLBY_VISION_HDMI_AUTO_DETECT
}

void newbase_hdmi_hdr10_enable(unsigned char detectEn)
{
#ifdef CONFIG_ENABLE_DOLBY_VISION_HDMI_AUTO_DETECT
	HDMI_PRINTF("[HDR10] HDMI Auto Detet En:%d...\n",detectEn);
	if(detectEn == _ENABLE)
    {//enable
		Scaler_HDR10_Hdmi_Set_Detect_Status(SLR_HDR10_HDMI_DETECT_ON);
	}
	else
	{//disable
	    Scaler_HDR10_Hdmi_Set_Detect_Status(SLR_HDR10_HDMI_DETECT_OFF);
	}
#endif
}


HDMI_bool newbase_hdmi_is_drm_info_ready(void)
{
#ifdef CONFIG_ENABLE_DOLBY_VISION_HDMI_AUTO_DETECT
	unsigned char port = newbase_hdmi_get_current_display_port();
	HDMI_DRM_T drm_info;

	if ( newbase_hdmi_get_drm_infoframe(port, &drm_info)) {
		if (drm_info.len == 0 && drm_info.ver == 0) {
			return FALSE;
		}

		if (drm_info.eEOTFtype == 0) {
			return FALSE; //no EOTF
		}

		if (drm_info.display_primaries_x0==0 &&
			drm_info.display_primaries_x1==0 && drm_info.display_primaries_x2==0) {
			return FALSE;
		}
	} else {
		return FALSE;
	}
	
	return TRUE;
#endif
	
	return FALSE;
}


unsigned char newbase_hdmi_hdr10_md_info_check(void)
{
    return TRUE;
}

void newbase_hdmi_hdr_disable(void)
{

#ifdef CONFIG_FORCE_RUN_I3DDMA
	 if (newbase_hdmi_is_dispsrc(SLR_MAIN_DISPLAY))//Need main connect HDMI
#endif
    	newbase_hdmi_dv_hdr_enable(_DISABLE);
    newbase_hdmi_hdr10_enable(_DISABLE);
}

//USER:LewisLee DATE : 201811/22
//Judge Dolby mode
void newbase_hdmi_dolby_vision_enable(unsigned char port, DOLBY_HDMI_VSIF_T HDR_Type)
{
#ifdef CONFIG_SUPPORT_DOLBY_VSIF
    unsigned char check_dolby_h14b_result = TRUE;
    int idx_PB; // HDMI 1.4b VSIF PB[] index

    switch(HDR_Type)
    {
        case DOLBY_HDMI_VSIF_STD:
        case DOLBY_HDMI_VSIF_LL:
            if(0x1b == hdmi_rx[port].dvsi_t.Length)
            {
                if(0x02 == (hdmi_rx[port].dvsi_t.Payload[0] & 0x03))
                {
                    // Dolby_Vision Signal = 1
                    // Low_Latency = 0
                    check_and_set_dolby_vision_detect();
                    set_HDMI_Dolby_VSIF_mode(DOLBY_HDMI_VSIF_STD);
//                    HDMI_EMG("==> VSIF_STD\n");
                }
                else if(0x03 == (hdmi_rx[port].dvsi_t.Payload[0] & 0x03))
                {
                    // Dolby_Vision Signal = 1
                    // Low_Latency = 1
                    SET_HDMI_DOLBY_VSIF_MODE(DOLBY_HDMI_VSIF_LL);
//                    HDMI_EMG("==> VSIF_LL\n");
                }
                else
                {
                    // Dolby_Vision Signal = 0
                    SET_HDMI_DOLBY_VSIF_MODE(DOLBY_HDMI_VSIF_DISABLE);
//                    HDMI_EMG("==> VSIF_DIS\n");
                }
            }
            else
            {
                HDMI_EMG("[HDMI] P(%d), Dolby len(%d) error\n", port, hdmi_rx[port].dvsi_t.Length);
                SET_HDMI_DOLBY_VSIF_MODE(DOLBY_HDMI_VSIF_DISABLE);
            }
#if 0
            if(_FALSE == hdmi_get_infoframe_thread_stop())
            {            
                SET_PRE_HDMI_DOLBY_VSIF_MODE(get_HDMI_Dolby_VSIF_mode());
            }
#endif
            break;

        case DOLBY_HDMI_h14B_VSIF:
            if(0x18 == hdmi_rx[port].vsi_t.Length)
            {
                // HDMI1.4b VSIF 24-bit IEEE OUI (=0x000C03)
                for(idx_PB = 5; idx_PB < 24; idx_PB++)
                {
                    if(0 != hdmi_rx[port].vsi_t.Payload[idx_PB])
                    {
                        check_dolby_h14b_result = FALSE;
                        break;
                    }
                }    
            }else if (hdmi_rx[port].vsi_t.Length == 0x5) {
		     /*
                check_dolby_h14b_result = FALSE;*/
                check_dolby_h14b_result = FALSE;
	        }
            else// if(0x18 != hdmi_rx[port].vsi_t.Length)
            {
                check_dolby_h14b_result = FALSE;
                //HDMI_EMG("[HDMI] P(%d), VSIF 1p4 Len(%d) error\n", port, hdmi_rx[port].vsi_t.Length);
            }

            if(TRUE == check_dolby_h14b_result)
            {//h14b dolby format
		    check_and_set_dolby_vision_detect();
		    set_HDMI_Dolby_H14B_VSIF_mode(DOLBY_HDMI_h14B_VSIF);
//                HDMI_EMG("==> VSIF_1p4_STD\n");
            }
            else// if(FALSE == check_dolby_h14b_result)
            {//h14b not dolby
		    set_HDMI_Dolby_H14B_VSIF_mode(DOLBY_HDMI_VSIF_DISABLE);
//                HDMI_EMG("==> VSIF_1p4_DIS\n");
            }

            //if(_FALSE == hdmi_get_infoframe_thread_stop())
            //{
            //    SET_PRE_HDMI_DOLBY_VSIF_MODE(get_HDMI_Dolby_VSIF_mode());
            //}
        break;

//        case DOLBY_HDMI_VSIF_DISABLE:
        default:
//            HDMI_EMG("==> VSIF_Non_DIS\n");
            SET_HDMI_DOLBY_VSIF_MODE(DOLBY_HDMI_VSIF_DISABLE);

        break;
    }

    if(_FALSE == hdmi_get_infoframe_thread_stop())
	    set_PreHDMI_Dolby_VSIF_mode(get_HDMI_Dolby_VSIF_mode());
#endif //#ifdef CONFIG_SUPPORT_DOLBY_VSIF
}

bool is_available_dolby_vsif(void)
{
	unsigned char port = newbase_hdmi_get_current_display_port();
	bool result = false;

	if (hdmi_rx[port].dvsi_t.VSIF_TypeCode == TYPE_CODE_VS_PACKET) {
		result = true;
	}

#if 0
	{
		static bool pre_result;
		if (pre_result != result) {
			rtd_pr_hdmi_info("[Dolby][%s:%d] %s available dolby vsif (%d)\n", __func__, __LINE__, (result) ? "had" : "no", port);
		}
		pre_result = result;
	}
#endif
	return result;
}

bool is_available_dolby_h14b_vsif(void)
{
	unsigned char port = newbase_hdmi_get_current_display_port();
	bool result = false;

	if (hdmi_rx[port].vsi_t.VSIF_TypeCode == TYPE_CODE_VS_PACKET) {
		result = true;
	}

#if 0
	{
		static bool pre_result;
		if (pre_result != result) {
			rtd_pr_hdmi_info("[Dolby][%s:%d] %s available h14b vsif (%d)\n", __func__, __LINE__, (result) ? "had" : "no", port);
		}
		pre_result = result;
	}
#endif

	return result;
}

#ifdef CONFIG_ENABLE_DOLBY_VISION_VSEM
bool is_available_dolby_vsem(unsigned char port)
{
	bool result = false;

	if (hdmi_rx[port].hdr_emp_mode == HDMI_HDR_EMP_RX_MODE_DOLBY_VISION_VSEM) {
		result = true;
	}

	return result;
}
#endif
