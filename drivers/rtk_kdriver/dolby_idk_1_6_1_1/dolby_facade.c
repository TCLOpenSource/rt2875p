#include <io.h>
#include <mach/timex.h>
#include <tvscalercontrol/scaler/scalerstruct.h>
#include <tvscalercontrol/hdmirx/hdmi_vfe.h>
#include <tvscalercontrol/scalerdrv/scalerdrv.h>
#include <tvscalercontrol/scalerdrv/scalermemory.h>
#include <tvscalercontrol/vip/scalerColor.h>
#include <tvscalercontrol/hdmirx/hdmifun.h>
#include <asm/cacheflush.h>
#include <tvscalercontrol/scaler/scalercolorlib.h>
#include <crypto/akcipher.h>
#include <rtk_kdriver/rtk_otp_region_api.h>
#include <rtk_mcp.h>
#define CFG_RTK_CMA_MAP 0
#include <linux/tee_drv.h>
#include "dolby_device.h"
#include "dolby_facade.h"
#include "dolby_gdbs_delay.h"
#include <rtk_kdriver/rtk_pwm.h>

//extern void dmac_inv_range(const void *, const void *);
//extern void dmac_flush_range(const void *, const void *);

int dolby_facade_rtk_timer_set_value(unsigned char id, unsigned int value)
{
	return rtk_timer_set_value(id, value);
}
EXPORT_SYMBOL(dolby_facade_rtk_timer_set_value);

void dolby_facade_set_OTT_HDR_mode(HDR_MODE mode)
{
	if (mode == HDR_DOLBY_COMPOSER)
		Scaler_color_set_HDR_Type(HDR_DM_DOLBY);

	return set_OTT_HDR_mode(mode);
}
EXPORT_SYMBOL(dolby_facade_set_OTT_HDR_mode);

HDR_MODE dolby_facade_get_OTT_HDR_mode(void)
{
	return get_OTT_HDR_mode();
}
EXPORT_SYMBOL(dolby_facade_get_OTT_HDR_mode);

HDR_MODE dolby_facade_get_HDMI_HDR_mode(void)
{
	return get_HDMI_HDR_mode();
}
EXPORT_SYMBOL(dolby_facade_get_HDMI_HDR_mode);

unsigned int dolby_facade_drvif_memory_get_data_align(unsigned int Value, unsigned int unit)
{
	return drvif_memory_get_data_align(Value, unit);
}
EXPORT_SYMBOL(dolby_facade_drvif_memory_get_data_align);

unsigned char dolby_facade_get_cur_hdmi_dolby_apply_state(void)
{
	extern unsigned char get_cur_hdmi_dolby_apply_state(void);
	return get_cur_hdmi_dolby_apply_state();
}
EXPORT_SYMBOL(dolby_facade_get_cur_hdmi_dolby_apply_state);

int dolby_facade_rtk_timer_control(unsigned char id, unsigned int cmd)
{
	return rtk_timer_control(id, cmd);
}
EXPORT_SYMBOL(dolby_facade_rtk_timer_control);

int dolby_facade_rtk_timer_set_mode(unsigned char id, unsigned char mode)
{
	return rtk_timer_set_mode(id, mode);
}
EXPORT_SYMBOL(dolby_facade_rtk_timer_set_mode);

int dolby_facade_rtk_timer_set_target(unsigned char id, unsigned int value)
{
	return rtk_timer_set_target(id, value);
}
EXPORT_SYMBOL(dolby_facade_rtk_timer_set_target);

void dolby_facade_dolby_ott_dm_init(void)
{
	dolby_ott_dm_init();
}
EXPORT_SYMBOL(dolby_facade_dolby_ott_dm_init);

unsigned short dolby_facade_Scaler_DispGetInputInfoByDisp(unsigned char channel, SLR_INPUT_INFO infoList)
{
	return Scaler_DispGetInputInfoByDisp(channel, infoList);
}
EXPORT_SYMBOL(dolby_facade_Scaler_DispGetInputInfoByDisp);

void *dolby_facade_Scaler_GetShareMemVirAddr(unsigned int a_ulCmd)
{
	return (void *)Scaler_GetShareMemVirAddr(a_ulCmd);
}
EXPORT_SYMBOL(dolby_facade_Scaler_GetShareMemVirAddr);

unsigned int dolby_facade_get_query_start_address(unsigned char idx)
{
//	extern unsigned int get_query_start_address(unsigned char idx);
	return get_query_start_address(idx);
}
EXPORT_SYMBOL(dolby_facade_get_query_start_address);

/* drv_memory_wait_cap_last_write_done() */
void dolby_facade_drv_memory_wait_cap_last_write_done(unsigned char display, unsigned char wait_frame, unsigned char game_mode_case)
{
	drv_memory_wait_cap_last_write_done(display, wait_frame, game_mode_case);
}
EXPORT_SYMBOL(dolby_facade_drv_memory_wait_cap_last_write_done);

void dolby_facade_fwif_color_disable_VIP(unsigned char flag)
{
	fwif_color_disable_VIP(flag);
}
EXPORT_SYMBOL(dolby_facade_fwif_color_disable_VIP);

void *dolby_facade_get_dolby_device(void)
{
	extern void *get_dolby_vision_devices(void);
	return get_dolby_vision_devices();
}
EXPORT_SYMBOL(dolby_facade_get_dolby_device);

void dolby_facade_dmac_inv_range(const void *start, const void *end)
{
	dmac_inv_range(start, end);
}
EXPORT_SYMBOL(dolby_facade_dmac_inv_range);

void dolby_facade_dmac_flush_range(const void *start, const void *end)
{
	dmac_flush_range(start, end);
}
EXPORT_SYMBOL(dolby_facade_dmac_flush_range);

DOLBY_HDMI_VSIF_T dolby_facade_get_HDMI_Dolby_VSIF_mode(void)
{
	return get_HDMI_Dolby_VSIF_mode();
}
EXPORT_SYMBOL(dolby_facade_get_HDMI_Dolby_VSIF_mode);

void *dolby_facade_get_hdmi_dolby_vsi_content(void)
{
	extern vfe_hdmi_vsi_t hdmi_dolby_vsi_content;
	return &hdmi_dolby_vsi_content;
}
EXPORT_SYMBOL(dolby_facade_get_hdmi_dolby_vsi_content);
VSC_INPUT_TYPE_T dolby_facade_Get_DisplayMode_Src(unsigned char display)
{
    return Get_DisplayMode_Src(display);
}
EXPORT_SYMBOL(dolby_facade_Get_DisplayMode_Src);

void dolby_facade_set_hdr_type_dolby(void)
{
	Scaler_color_set_HDR_Type(HDR_DM_DOLBY);
}
EXPORT_SYMBOL(dolby_facade_set_hdr_type_dolby);

bool dolby_facade_rtk_is_dobly_vision_supported(void)
{
	return rtk_is_dobly_Vision_supported();
}
EXPORT_SYMBOL(dolby_facade_rtk_is_dobly_vision_supported);

bool dolby_facade_is_memc_low_latency(void)
{
    extern bool is_memc_low_latency(void);
    return is_memc_low_latency();
}
EXPORT_SYMBOL(dolby_facade_is_memc_low_latency);

void dolby_facade_get_region(KADP_VIDEO_RECT_T *in_region, KADP_VIDEO_RECT_T *origin_region)
{
	extern KADP_VIDEO_RECT_T ap_main_inregion_parm;
	extern KADP_VIDEO_RECT_T ap_main_originalInput_parm;

	if (in_region != NULL) {
		in_region->x = ap_main_inregion_parm.x;
		in_region->y = ap_main_inregion_parm.y;
		in_region->w = ap_main_inregion_parm.w;
		in_region->h = ap_main_inregion_parm.h;
	}

	if (origin_region != NULL) {
		origin_region->x = ap_main_originalInput_parm.x;
		origin_region->y = ap_main_originalInput_parm.y;
		origin_region->w = ap_main_originalInput_parm.w;
		origin_region->h = ap_main_originalInput_parm.h;
	}
}
EXPORT_SYMBOL(dolby_facade_get_region);

unsigned char dolby_facade_get_hdmi_4k_hfr_mode(void)
{
	return get_hdmi_4k_hfr_mode();
}
EXPORT_SYMBOL(dolby_facade_get_hdmi_4k_hfr_mode);

HDMI_COLOR_SPACE_T dolby_facade_drvif_Hdmi_GetColorSpace(void)
{
	return drvif_Hdmi_GetColorSpace();
}
EXPORT_SYMBOL(dolby_facade_drvif_Hdmi_GetColorSpace);

uint8_t *dolby_facade_get_hdmi_metadata_base_address(void)
{
	uint8_t *get_hdmi_metadata_base_address(void);
	return get_hdmi_metadata_base_address();
}
EXPORT_SYMBOL(dolby_facade_get_hdmi_metadata_base_address);

void dolby_facade_set_pwm_duty(int duty)
{
	dolby_set_pwm_duty(duty);
}
EXPORT_SYMBOL(dolby_facade_set_pwm_duty);

int dolby_facade_MCP_AES_CBC_Decryption(
		unsigned char               key[16],
		unsigned char               iv[16],
		unsigned char*              p_in,
		unsigned char*              p_out,
		unsigned long               len)
{
	return MCP_AES_CBC_Decryption(key, iv, p_in, p_out, len);
}
EXPORT_SYMBOL(dolby_facade_MCP_AES_CBC_Decryption);

struct ta {
	struct tee_context *ctx;
	__u32 session;
};

static int optee_dolby_vision_match(struct tee_ioctl_version_data *data, const void *vers)
{
	return -1;
}

int dolby_facade_ta_init(void *data, uuid_t *uuid)
{
	struct ta *ta_data = data;

	int ret = 0, rc = 0;

	struct tee_ioctl_open_session_arg arg;
	struct tee_ioctl_version_data vers = {
		.impl_id = TEE_IMPL_ID_OPTEE,
		.impl_caps = TEE_OPTEE_CAP_TZ,
		.gen_caps = TEE_GEN_CAP_GP,
	};

	memset(ta_data, 0, sizeof(*ta_data));
	ta_data->ctx = tee_client_open_context(NULL, optee_dolby_vision_match, NULL, &vers);
	if(IS_ERR_OR_NULL(ta_data->ctx)) {
		rtd_pr_hdr_emerg("[DolbyVision]: no ta context\n");
		ret = -EINVAL;
		goto dolby_facade_ta_open_err;
	}

	memset(&arg, 0, sizeof(arg));
	memcpy(&arg.uuid, uuid, sizeof(uuid_t));
	arg.clnt_login = TEE_IOCTL_LOGIN_PUBLIC;
	rtd_pr_hdr_debug("arg uuid %pUl \n", arg.uuid);

	rc = tee_client_open_session(ta_data->ctx, &arg, NULL);
	if(rc){
		rtd_pr_hdr_emerg("[DolbyVision]: open ta session failed ret %x arg %x", rc, arg.ret);
		ret = -EINVAL;
		goto dolby_facade_ta_open_session_err;
	}

	if (arg.ret) {
		rtd_pr_hdr_emerg("[DolbyVision]: open ta session failed ret %x arg %x", rc, arg.ret);
		ret = -EINVAL;
		goto dolby_facade_ta_init_err;
	}

	ta_data->session = arg.session;

	return ret;

dolby_facade_ta_init_err:
	if (ta_data->session) {
		tee_client_close_session(ta_data->ctx, ta_data->session);
		rtd_pr_hdr_emerg("optee_dolby_vision: open failed close session \n");
	}

dolby_facade_ta_open_session_err:
	if (!IS_ERR_OR_NULL(ta_data->ctx)) {
		tee_client_close_context(ta_data->ctx);
		rtd_pr_hdr_emerg("optee_dolby_vision: open failed close context\n");
	}

dolby_facade_ta_open_err:
	ta_data->ctx = NULL;
	ta_data->session = 0;

	rtd_pr_hdr_emerg("[DolbyVision] open_session fail\n");

	return ret;
}
EXPORT_SYMBOL(dolby_facade_ta_init);

int dolby_facade_ta_exit(void *data)
{
	struct ta *ta_data = data;

	if (ta_data->session) {
		tee_client_close_session(ta_data->ctx, ta_data->session);
	}

	if (!IS_ERR_OR_NULL(ta_data->ctx)) {
		tee_client_close_context(ta_data->ctx);
	}

	ta_data->session = 0;
	ta_data->ctx = NULL;

	return 0;
}
EXPORT_SYMBOL(dolby_facade_ta_exit);

struct share_memory {
	struct tee_shm *shm;
	uint8_t *buffer;
	size_t size;
};

long dolby_facade_register_tee_share_memory(struct tee_context *ctx, void *data)
{
	struct share_memory *shm_buffer = data;

	if (shm_buffer == NULL) {
		rtd_pr_hdr_info("[DOLBY][%s:%d]\n", __func__, __LINE__);
		return -1;
	}

	if(shm_buffer->size > 0) {
		/* alloc send share memory */
		shm_buffer->shm = tee_shm_alloc(ctx, shm_buffer->size, TEE_SHM_MAPPED | TEE_SHM_DMA_BUF);
		if (IS_ERR(shm_buffer->shm)) {
			rtd_pr_hdr_emerg("optee_dolby_vision: no shm_buffer\n");
			goto register_tee_share_memory_out;
		}

		/* get share memory virtual addr for data accessing */
		shm_buffer->buffer = tee_shm_get_va(shm_buffer->shm, 0);
		if (shm_buffer->buffer == NULL) {
			goto register_tee_share_memory_out;
		}
		//rtd_pr_hdr_debug("[DolbyVision][%s:%s:%d] shm  %p size %d buffer %p\n", __FILE__, __func__, __LINE__, shm_buffer->shm, shm_buffer->size, shm_buffer->buffer);//fix error
	}

	return 0;

register_tee_share_memory_out:

	if (!IS_ERR(shm_buffer->shm))
		tee_shm_free(shm_buffer->shm);

	shm_buffer->shm = NULL;

	return -1;
}
EXPORT_SYMBOL(dolby_facade_register_tee_share_memory);

void dolby_facade_release_tee_share_memory(void *data)
{
	struct share_memory *shm_buffer = data;
	if (shm_buffer->shm) {
		tee_shm_free(shm_buffer->shm);
		memset(shm_buffer, 0, sizeof(struct share_memory));
	}
}
EXPORT_SYMBOL(dolby_facade_release_tee_share_memory);

#define TEE_NUM_PARAM 4
int dolby_facade_execute_ta_command(void *ta, int command, void *param1, void *param2)
{
	struct ta *ta_data = ta;
	struct share_memory *shm_param1 = param1;
	struct share_memory *shm_param2 = param2;

	struct tee_ioctl_invoke_arg arg;
	struct tee_param *param = NULL;

	int ret = 0, rc = 0;

	memset(&arg, 0, sizeof(arg));
	arg.func = command;
	arg.session = ta_data->session;
	arg.num_params = TEE_NUM_PARAM;

	param = kcalloc(TEE_NUM_PARAM, sizeof(struct tee_param), GFP_KERNEL);
	if(param == NULL)
	{
		rtd_pr_hdr_emerg("%s kcalloc param fail\n",__FUNCTION__);
		ret = -ENOMEM;
		return ret;
	}

	memset(param, 0, sizeof(struct tee_param) * TEE_NUM_PARAM);

	if (shm_param1 != NULL) {
		param[0].attr = TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_INOUT;
		param[0].u.memref.shm = shm_param1->shm;
		param[0].u.memref.size = shm_param1->size;
	} else {
		param[0].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;
	}

	if (shm_param2 != NULL) {
		param[1].attr = TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_INOUT;
		param[1].u.memref.shm = shm_param2->shm;
		param[1].u.memref.size = shm_param2->size;
	} else {
		param[1].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;
	}

	param[2].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;
	param[3].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;

	rc = tee_client_invoke_func(ta_data->ctx, &arg, param);
	kfree(param);

	if (rc || arg.ret) {
		rtd_pr_hdr_emerg("optee_dolby_vision: invoke failed ret %x arg.ret %x\n", rc, arg.ret);
		ret = -EINVAL;
	}

	return ret;
}
EXPORT_SYMBOL(dolby_facade_execute_ta_command);

#ifdef CONFIG_DOLBY_BACKLIGHT_CTRL_ADIM_PDIM
void dolby_facade_set_dolby_duty_by_dim(int duty)
{
	set_dolby_duty_by_adim_pdim(duty);
}
EXPORT_SYMBOL(dolby_facade_set_dolby_duty_by_dim);
#endif

int dolby_facade_rtk_pwm_backlight_set_dolby_duty(unsigned int dolby_duty)
{
    extern unsigned int is_pwm_ctrl_by_dolby;
    if (is_pwm_ctrl_by_dolby) {
#ifdef CONFIG_DOLBY_BACKLIGHT_CTRL_ADIM_PDIM
    return set_dolby_duty_by_adim_pdim(dolby_duty);
#else
    return rtk_pwm_backlight_set_duty(dolby_duty);
#endif
    }

    return 0;
}
EXPORT_SYMBOL(dolby_facade_rtk_pwm_backlight_set_dolby_duty);

ssize_t dolby_facade_kernel_read(struct file *file, void *buf, size_t count, loff_t *pos)
{
#ifdef CONFIG_RTK_FEATURE_FOR_GKI
	return kernel_read(file, buf, count, pos);
#else
	return 0;
#endif
}
EXPORT_SYMBOL(dolby_facade_kernel_read);

void dolby_facade_trigger_dolby_vision_worker(void)
{
	extern void trigger_dolby_vision_worker(void);
	trigger_dolby_vision_worker();
}
EXPORT_SYMBOL(dolby_facade_trigger_dolby_vision_worker);

bool is_hdmi_dolby_vision_rgb(void)
{
	bool ret_val = false;

	if ((VSC_INPUTSRC_HDMI == Get_DisplayMode_Src(SLR_MAIN_DISPLAY))
			&& (get_HDMI_Dolby_VSIF_mode() == DOLBY_HDMI_VSIF_LL)
			&& (drvif_Hdmi_GetColorSpace() == COLOR_RGB)
			)
		ret_val = true;

	return ret_val;

}

bool check_hdmi_dolby_vision_rgb_and_full_hd(void)
{
	bool ret_val = false;

	if ((VSC_INPUTSRC_HDMI == Get_DisplayMode_Src(SLR_MAIN_DISPLAY))
			&& (get_HDMI_Dolby_VSIF_mode() == DOLBY_HDMI_VSIF_LL)) {
		vfe_hdmi_timing_info_t hdmi_info = {};
		int width = 0, height = 0;

		vfe_hdmi_drv_get_display_timing_info(&hdmi_info, SLR_MAIN_DISPLAY);

		width = hdmi_info.active.w;
		height = hdmi_info.active.h;

		if ((drvif_Hdmi_GetColorSpace() == COLOR_RGB) && (width * height <= 1920 * 1080))
			ret_val = true;
	}

	return ret_val;

}

bool check_hdmi_dolby_vision_FRT_one_drop_case(void)
{
    bool ret_val = false;

    if ((VSC_INPUTSRC_HDMI == Get_DisplayMode_Src(SLR_MAIN_DISPLAY))
            && (get_HDMI_Dolby_VSIF_mode() != DOLBY_HDMI_VSIF_DISABLE)) {
        vfe_hdmi_timing_info_t hdmi_info = {};
        int hdmiVSync;

        vfe_hdmi_drv_get_display_timing_info(&hdmi_info, SLR_MAIN_DISPLAY);
        hdmiVSync = hdmi_info.v_freq;

        if(hdmiVSync <= 305) //less than 30 hz
            ret_val = true;
    }

    return ret_val;
}

unsigned int dolby_facade_get_dm_src_color_format(void)
{
	unsigned int color = 0;

	if (is_hdmi_dolby_vision_rgb())
		color = 1;

	return color;
}
EXPORT_SYMBOL(dolby_facade_get_dm_src_color_format);

HDMI_COLOR_DEPTH_T dolby_facade_drvif_Hdmi_GetColorDepth(void)
{
	return drvif_Hdmi_GetColorDepth();
}
EXPORT_SYMBOL(dolby_facade_drvif_Hdmi_GetColorDepth);

#ifdef CONFIG_RTK_KDRV_DV_IDK_DUMP
unsigned int dolby_facade_get_bit_depth(void)
{
	extern unsigned int get_bit_depth(void);
	return get_bit_depth();
}
EXPORT_SYMBOL(dolby_facade_get_bit_depth);

unsigned int dolby_facade_get_signal_format(void)
{
	extern unsigned int get_signal_format(void);

	return get_signal_format();
}
EXPORT_SYMBOL(dolby_facade_get_signal_format);

unsigned int dolby_facade_is_force_dolby(void)
{
	extern unsigned int is_force_dolby(void);

	return is_force_dolby();
}
EXPORT_SYMBOL(dolby_facade_is_force_dolby);

unsigned int dolby_facade_is_dump_dm_register_enable(void)
{
	extern bool is_dump_dm_register_enable(void);

	return is_dump_dm_register_enable();
}
EXPORT_SYMBOL(dolby_facade_is_dump_dm_register_enable);
#endif

unsigned int dolby_facade_rtd_inl(unsigned long addr)
{
	return rtd_inl(addr);
}
EXPORT_SYMBOL(dolby_facade_rtd_inl);

void dolby_facade_rtd_outl(unsigned long addr, unsigned int val)
{
	rtd_outl(addr, val);
}
EXPORT_SYMBOL(dolby_facade_rtd_outl);

void dolby_facade_rtd_clearbits(unsigned long addr, unsigned int val)
{
	rtd_clearbits(addr, val);
}
EXPORT_SYMBOL(dolby_facade_rtd_clearbits);

void dolby_facade_rtd_setbits(unsigned long addr, unsigned int val)
{
	rtd_setbits(addr, val);
}
EXPORT_SYMBOL(dolby_facade_rtd_setbits);

