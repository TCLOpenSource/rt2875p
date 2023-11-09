#include "optee_private.h"
#include "rtk_optee_smc.h"

int optee_rtc_set_info_send(struct optee_rpc_param *p_rtc_set_info)
{
	struct arm_smccc_res res;

	p_rtc_set_info->a0 = OPTEE_SMC_SET_DRTC;

	arm_smccc_smc(p_rtc_set_info->a0,p_rtc_set_info->a1,p_rtc_set_info->a2,
				  p_rtc_set_info->a3,p_rtc_set_info->a4,p_rtc_set_info->a5,
				  p_rtc_set_info->a6,0,&res);

	if(res.a0 == 0)
		return OPTEE_SMC_RETURN_OK;
	else
		return -1;
}
EXPORT_SYMBOL(optee_rtc_set_info_send);

int optee_logbuf_setup_info_send(struct optee_rpc_param *p_rtc_set_info)
{
	struct arm_smccc_res res;

	p_rtc_set_info->a0 = OPTEE_SMC_SET_LOGBUF;

	arm_smccc_smc(p_rtc_set_info->a0,p_rtc_set_info->a1,p_rtc_set_info->a2,
				  p_rtc_set_info->a3,p_rtc_set_info->a4,p_rtc_set_info->a5,
				  p_rtc_set_info->a6,0,&res);

	if(res.a0 == 0)
		return OPTEE_SMC_RETURN_OK;
	else
		return -1;
}
EXPORT_SYMBOL(optee_logbuf_setup_info_send);

void optee_hdcp2_force_depends_on_rtktee(void)
{
	return;
}
EXPORT_SYMBOL(optee_hdcp2_force_depends_on_rtktee);
