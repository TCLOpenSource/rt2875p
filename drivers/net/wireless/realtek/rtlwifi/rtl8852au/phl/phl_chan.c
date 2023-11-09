/******************************************************************************
 *
 * Copyright(c) 2019 Realtek Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 *****************************************************************************/
#define _PHL_CHAN_C_
#include "phl_headers.h"

const char *const _band_str[] = {
	"BAND_ON_24G",
	"BAND_ON_5G",
	"BAND_ON_6G",
	"BAND_UNKNOWN"
};
#define _get_band_str(band) (((band) >= BAND_MAX) ? _band_str[BAND_MAX] : _band_str[(band)])

const char *const _bw_str[] = {
	"BW_20M",
	"BW_40M",
	"BW_80M",
	"BW_160M",
	"BW_80_80M",
	"BW_5M",
	"BW_10M",
	"BW_UNKNOWN"
};
#define _get_bw_str(bw) (((bw) >= CHANNEL_WIDTH_MAX) ? _bw_str[CHANNEL_WIDTH_MAX] : _bw_str[((bw))])

#ifdef DBG_PHL_CHAN
void phl_chan_dump_chandef(const char *caller, const int line, bool show_caller,
				struct rtw_chan_def *chandef)
{
	if (show_caller)
		PHL_INFO("###### FUN - %s LINE - %d #######\n", caller, line);

	PHL_INFO("\t[CH] band:%s\n", _get_band_str(chandef->band));
	PHL_INFO("\t[CH] chan:%d\n", chandef->chan);
	PHL_INFO("\t[CH] center_ch:%d\n", chandef->center_ch);
	PHL_INFO("\t[CH] bw:%s\n", _get_bw_str(chandef->bw));
	PHL_INFO("\t[CH] offset:%d\n", chandef->offset);

	PHL_INFO("\t[CH] center_freq1:%d\n", chandef->center_freq1);
	PHL_INFO("\t[CH] center_freq2:%d\n", chandef->center_freq2);
	PHL_INFO("\t[CH] hw_value:%d\n", chandef->hw_value);

	if (show_caller)
		PHL_INFO("#################################\n");
}
#endif

#ifdef CONFIG_PHL_DFS
static enum rtw_phl_status
phl_radar_detect_hdl(struct phl_info_t *phl_info,
	u8 channel, enum channel_width bwmode, enum chan_offset offset)
{
	struct rtw_phl_com_t *phl_com = phl_info->phl_com;
	struct rtw_dfs_t *dfs_info = &phl_com->dfs_info;
	enum rtw_phl_status rst = RTW_PHL_STATUS_FAILURE;
	bool overlap_radar_range;

	overlap_radar_range = rtw_hal_in_radar_domain(phl_info->hal,
						channel, bwmode);
	if (overlap_radar_range)
		PHL_INFO("chan in DFS domain ch:%d,bw:%d\n", channel, bwmode);


	if (overlap_radar_range && !dfs_info->dfs_enabled) {
		/*radar_detect_enable*/
		if (rtw_hal_radar_detect_cfg(phl_info->hal, true) ==
			RTW_HAL_STATUS_SUCCESS) {
			dfs_info->dfs_enabled = true;
			PHL_INFO("[DFS] chan(%d) in radar range, enable dfs\n",
				channel);
			rst = RTW_PHL_STATUS_SUCCESS;
		}
		else {
			PHL_ERR("[DFS] chan(%d) in radar range, enable dfs failed\n",
				channel);
		}

	} else if (!overlap_radar_range && dfs_info->dfs_enabled) {
		/*radar_detect_disable*/
		if (rtw_hal_radar_detect_cfg(phl_info->hal, false) ==
			RTW_HAL_STATUS_SUCCESS) {
			dfs_info->dfs_enabled = false;
			PHL_INFO("[DFS] chan(%d) not in radar range, disable dfs\n",
				channel);
			rst = RTW_PHL_STATUS_SUCCESS;
		}
		else {
			PHL_ERR("[DFS] chan(%d) not in radar range, disable dfs failed\n",
				channel);
		}
	}
	return rst;
}
#endif /*CONFIG_PHL_DFS*/

enum rtw_phl_status
phl_set_ch_bw(struct phl_info_t *phl_info, u8 band_idx,
		  struct rtw_chan_def *chdef, bool do_rfk)
{
	enum rtw_hal_status hstatus = RTW_HAL_STATUS_FAILURE;

#ifdef CONFIG_PHL_DFS
	phl_radar_detect_hdl(phl_info, chdef->chan, chdef->bw, chdef->offset);
#endif
	hstatus = rtw_hal_set_ch_bw(phl_info->hal, band_idx,
				    chdef, do_rfk);
	if (RTW_HAL_STATUS_SUCCESS != hstatus)
		PHL_ERR("%s rtw_hal_set_ch_bw: statuts = %u\n", __func__, hstatus);

	return RTW_PHL_STATUS_SUCCESS;
}

#ifdef CONFIG_CMD_DISP

static void
_phl_chg_op_chdef_done(void *drv_priv, u8 *cmd, u32 cmd_len,
			enum rtw_phl_status status)
{
	if (cmd) {
		struct chg_opch_param *param = (struct chg_opch_param *)cmd;

		if (param->chg_opch_done) {
			enum rtw_phl_status psts = RTW_PHL_STATUS_FAILURE;

			if (RTW_PHL_STATUS_CMD_SUCCESS == status &&
			    RTW_PHL_STATUS_CMD_SUCCESS == param->cmd_start_sts)
				psts = RTW_PHL_STATUS_SUCCESS;
			param->chg_opch_done(drv_priv, param->wrole->id,
						psts);
		}
		_os_kmem_free(drv_priv, cmd, cmd_len);
		cmd = NULL;
		PHL_INFO("%s.....\n", __func__);
	}
}

static void _phl_chg_op_chdef_start_done(void *drv_priv, u8 *cmd, u32 cmd_len, enum rtw_phl_status status)
{
	if (cmd) {
		struct chg_opch_param *param = (struct chg_opch_param *)cmd;

		param->cmd_start_sts = status;
		PHL_INFO("%s.....\n", __func__);
	}
}

enum rtw_phl_status
phl_cmd_chg_op_chdef_start_hdl(struct phl_info_t *phl, u8 *param)
{
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;
	struct chg_opch_param *ch_param = (struct chg_opch_param *)param;
	void *drv = phl_to_drvpriv(phl);
	enum phl_upd_mode mode = PHL_UPD_STA_INFO_CHANGE;
	struct phl_queue *sta_queue = NULL;
	struct rtw_phl_stainfo_t *sta = NULL;
	struct rtw_chan_def chctx_result = {0};

	sta = rtw_phl_get_stainfo_self(phl, ch_param->wrole);
	if (NULL == sta) {
		PHL_TRACE(COMP_PHL_DBG, _PHL_ERR_, "%s: cannot get stainfo_self\n",
			__FUNCTION__);
		goto exit;
	}
	/* Update MR chctx */
	if (RTW_PHL_STATUS_SUCCESS != phl_mr_chandef_chg(phl, ch_param->wrole,
					&ch_param->new_chdef, &chctx_result)) {
		PHL_TRACE(COMP_PHL_DBG, _PHL_ERR_, "%s: MR chang chdef failed!\n",
			__FUNCTION__);
		goto exit;
	}
	/* Up Role chdef */
	_os_mem_cpy(drv, &ch_param->wrole->chandef, &ch_param->new_chdef,
			sizeof(struct rtw_chan_def));
	/* Update self Sta chdef */
	_os_mem_cpy(drv, &sta->chandef, &ch_param->new_chdef,
			sizeof(struct rtw_chan_def));
	/* Notify rf for the suspended channel */
	rtw_hal_disconnect_notify(phl->hal, &ch_param->ori_chdef);
	/* Switch channel */
	if (RTW_PHL_STATUS_SUCCESS != phl_set_ch_bw(phl, ch_param->wrole->hw_band,
					&chctx_result, true)) {
		PHL_TRACE(COMP_PHL_DBG, _PHL_ERR_, "%s: Switch ch failed!\n",
			__FUNCTION__);
		goto exit;
	}
	if (ch_param->wrole->mstate == MLME_LINKED) {
		/*Up STA setting(RA....) */
		sta_queue = &ch_param->wrole->assoc_sta_queue;
		_os_spinlock(drv, &sta_queue->lock, _bh, NULL);
		phl_list_for_loop(sta, struct rtw_phl_stainfo_t,
				&sta_queue->queue, list) {
			if (sta)
				phl_change_stainfo(phl, sta, mode);
		}
		_os_spinunlock(drv, &sta_queue->lock, _bh, NULL);
	}
	pstatus = RTW_PHL_STATUS_SUCCESS;
exit:
	PHL_TRACE(COMP_PHL_DBG, _PHL_INFO_, "%s: pstatus(%d)\n",
		__FUNCTION__, pstatus);
	return pstatus;
}

enum rtw_phl_status
rtw_phl_cmd_chg_op_chdef(struct rtw_wifi_role_t *wrole,
	struct rtw_chan_def *new_chdef, bool cmd_wait, u32 cmd_timeout,
	void (*chg_opch_done)(void *priv, u8 ridx, enum rtw_phl_status status))
{
	enum rtw_phl_status psts = RTW_PHL_STATUS_FAILURE;
	struct phl_info_t *phl = wrole->phl_com->phl_priv;
	void *drv = phl_to_drvpriv(phl);
	u32 param_len = sizeof(struct chg_opch_param);
	struct chg_opch_param *param = _os_kmem_alloc(drv, param_len);

	if (param == NULL) {
		PHL_ERR("%s: alloc param failed!\n", __func__);
		goto _exit;
	}
	param->wrole = wrole;
	_os_mem_cpy(drv, &param->new_chdef, new_chdef,
			sizeof(struct rtw_chan_def));
	_os_mem_cpy(drv, &param->ori_chdef, &wrole->chandef,
			sizeof(struct rtw_chan_def));
	param->chg_opch_done = chg_opch_done;
	psts = phl_cmd_enqueue(phl,
	                       wrole->hw_band,
	                       MSG_EVT_CHG_OP_CH_DEF_START,
	                       (u8 *)param,
	                       param_len,
	                       _phl_chg_op_chdef_start_done,
	                       PHL_CMD_NO_WAIT,
	                       0);
	if (psts != RTW_PHL_STATUS_SUCCESS) {
		PHL_INFO("%s: Fail to issue change op chdef start!!\n",
			__func__);
		if (!is_cmd_failure(psts)) {
			/* Send cmd fail */
			_os_kmem_free(drv, param, param_len);
			psts = RTW_PHL_STATUS_FAILURE;
		}
		goto _exit;
	}
	psts = phl_cmd_enqueue(phl,
	                       wrole->hw_band,
	                       MSG_EVT_CHG_OP_CH_DEF_END,
	                       (u8 *)param,
	                       param_len,
	                       _phl_chg_op_chdef_done,
	                       cmd_wait ? PHL_CMD_WAIT : PHL_CMD_NO_WAIT,
	                       cmd_timeout);
	if (psts != RTW_PHL_STATUS_SUCCESS) {
		PHL_INFO("%s: Fail to issue change op chdef start!!\n",
			__func__);
		if (!is_cmd_failure(psts)) {
			/* Send cmd fail */
			_os_kmem_free(drv, param, param_len);
			psts = RTW_PHL_STATUS_FAILURE;
		}
		goto _exit;
	}
_exit:
	PHL_INFO("%s: Issue cmd, status(%d)\n", __func__, psts);
	return psts;
}

enum rtw_phl_status
phl_cmd_set_ch_bw_hdl(struct phl_info_t *phl_info, u8 *param)
{
	struct setch_param *ch_param = (struct setch_param *)param;

	return phl_set_ch_bw(phl_info, ch_param->wrole->hw_band,
	                         &(ch_param->chdef),
	                         ch_param->do_rfk);
}

static void _phl_set_ch_bw_done(void *drv_priv, u8 *cmd, u32 cmd_len, enum rtw_phl_status status)
{
	if (cmd) {
		_os_kmem_free(drv_priv, cmd, cmd_len);
		cmd = NULL;
		PHL_INFO("%s.....\n", __func__);
	}
}

enum rtw_phl_status
rtw_phl_cmd_set_ch_bw(struct rtw_wifi_role_t *wifi_role,
                      struct rtw_chan_def *chdef,
                      bool do_rfk,
                      enum phl_cmd_type cmd_type,
                      u32 cmd_timeout)
{
	struct phl_info_t *phl_info = wifi_role->phl_com->phl_priv;
	void *drv = wifi_role->phl_com->drv_priv;
	enum rtw_phl_status psts = RTW_PHL_STATUS_FAILURE;
	struct setch_param *param = NULL;
	u32 param_len;

	if (cmd_type == PHL_CMD_DIRECTLY) {
		psts = phl_set_ch_bw(phl_info, wifi_role->hw_band, chdef, do_rfk);
		goto _exit;
	}

	param_len = sizeof(struct setch_param);
	param = _os_kmem_alloc(drv, param_len);
	if (param == NULL) {
		PHL_ERR("%s: alloc param failed!\n", __func__);
		goto _exit;
	}
	param->wrole = wifi_role;
	_os_mem_cpy(drv, &param->chdef, chdef, sizeof(struct rtw_chan_def));
	param->do_rfk = do_rfk;

	psts = phl_cmd_enqueue(phl_info,
	                       wifi_role->hw_band,
	                       MSG_EVT_SWCH_START,
	                       (u8 *)param,
	                       param_len,
	                       _phl_set_ch_bw_done,
	                       cmd_type,
	                       cmd_timeout);

	if (is_cmd_failure(psts)) {
		/* Send cmd success, but wait cmd fail*/
		psts = RTW_PHL_STATUS_FAILURE;
	} else if (psts != RTW_PHL_STATUS_SUCCESS) {
		/* Send cmd fail */
		_os_kmem_free(drv, param, param_len);
		psts = RTW_PHL_STATUS_FAILURE;
	}
_exit:
	return psts;
}
#endif /*CONFIG_CMD_DISP*/

u8 rtw_phl_get_cur_ch(struct rtw_wifi_role_t *wifi_role)
{
	struct phl_info_t *phl_info = wifi_role->phl_com->phl_priv;

	return rtw_hal_get_cur_ch(phl_info->hal, wifi_role->hw_band);
}

enum rtw_phl_status
rtw_phl_get_cur_hal_chdef(struct rtw_wifi_role_t *wifi_role,
					struct rtw_chan_def *cur_chandef)
{
	struct phl_info_t *phl_info = wifi_role->phl_com->phl_priv;

	rtw_hal_get_cur_chdef(phl_info->hal, wifi_role->hw_band, cur_chandef);
	return RTW_PHL_STATUS_SUCCESS;
}

static enum rtw_phl_status
_dfs_hw_tx_pause(struct rtw_wifi_role_t *wifi_role, bool tx_pause)
{

	struct phl_info_t *phl_info = wifi_role->phl_com->phl_priv;
	enum rtw_hal_status hstatus = RTW_HAL_STATUS_FAILURE;

	hstatus = rtw_hal_dfs_pause_tx(phl_info->hal, wifi_role->hw_band, tx_pause);

	if (RTW_HAL_STATUS_SUCCESS == hstatus) {
		return RTW_PHL_STATUS_SUCCESS;
	} else {
		PHL_ERR("%s Failure :%u\n",__func__, hstatus);
		return RTW_PHL_STATUS_FAILURE;
	}
}

#ifdef CONFIG_CMD_DISP
struct dfs_txpause_param {
	struct rtw_wifi_role_t *wrole;
	bool pause;
};

enum rtw_phl_status
phl_cmd_dfs_tx_pause_hdl(struct phl_info_t *phl_info, u8 *param)
{
	struct dfs_txpause_param *dfs = (struct dfs_txpause_param *)param;

	PHL_INFO("%s(), dfs param, wrole = %p, pause = %d\n",
			__func__, dfs->wrole, dfs->pause);

	return _dfs_hw_tx_pause(dfs->wrole, dfs->pause);
}

static void _phl_dfs_tx_pause_done(void *drv_priv, u8 *cmd, u32 cmd_len, enum rtw_phl_status status)
{
	if (cmd) {
		_os_kmem_free(drv_priv, cmd, cmd_len);
		cmd = NULL;
		PHL_INFO("%s.....\n", __func__);
	}
}
#endif /*CONFIG_CMD_DISP*/

enum rtw_phl_status
rtw_phl_cmd_dfs_tx_pause(struct rtw_wifi_role_t *wifi_role, bool pause,
                      		enum phl_cmd_type cmd_type, u32 cmd_timeout)
{
#ifdef CONFIG_CMD_DISP
	struct phl_info_t *phl_info = wifi_role->phl_com->phl_priv;
	void *drv = wifi_role->phl_com->drv_priv;
	enum rtw_phl_status psts = RTW_PHL_STATUS_FAILURE;
	struct dfs_txpause_param *param = NULL;
	u32 param_len;

	param_len = sizeof(struct dfs_txpause_param);
	param = _os_kmem_alloc(drv, param_len);
	if (param == NULL) {
		PHL_ERR("%s: alloc param failed!\n", __func__);
		goto _exit;
	}
	param->wrole = wifi_role;
	param->pause = pause;

	psts = phl_cmd_enqueue(phl_info,
			wifi_role->hw_band,
			MSG_EVT_DFS_PAUSE_TX,
			(u8 *)param, param_len,
			_phl_dfs_tx_pause_done,
			cmd_type, cmd_timeout);

	if (is_cmd_failure(psts)) {
		/* Send cmd success, but wait cmd fail*/
		psts = RTW_PHL_STATUS_FAILURE;
	} else if (psts != RTW_PHL_STATUS_SUCCESS) {
		/* Send cmd fail */
		_os_kmem_free(drv, param, param_len);
		psts = RTW_PHL_STATUS_FAILURE;
	}

_exit:
	return psts;
#else
	PHL_ERR("%s(), CONFIG_CMD_DISP need to be enabled for MSG_EVT_DFS_PAUSE_TX \n",__func__);

	return RTW_PHL_STATUS_FAILURE;
#endif
}


#ifdef CONFIG_DBCC_SUPPORT
enum rtw_phl_status
rtw_phl_dbcc_test(void *phl, enum dbcc_test_id id, void *param)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	enum rtw_hal_status hsts = RTW_HAL_STATUS_FAILURE;

	switch (id){
	case DBCC_PRE_CFG :
	{
		bool dbcc_en = *(bool *)param;

		PHL_INFO("[DBCC] PRE_CFG :%s\n", (dbcc_en) ? "EN" : "DIS");
		hsts = rtw_hal_dbcc_pre_cfg(phl_info->hal, phl_info->phl_com, dbcc_en);
	}
	break;

	case DBCC_CFG :
	{
		bool dbcc_en = *(bool *)param;

		PHL_INFO("[DBCC] CFG :%s\n", (dbcc_en) ? "EN" : "DIS");
		hsts = rtw_hal_dbcc_cfg(phl_info->hal, phl_info->phl_com, dbcc_en);
	}
	break;
	case DBCC_CLEAN_TXQ :
		hsts = rtw_hal_clean_tx_queue(phl_info->hal);
		break;
	default :
		PHL_ERR("%s unknown DBCC Test ID:%d\n",__func__, id);
		break;
	}

	return RTW_PHL_STATUS_SUCCESS;
}
#endif

#define MAX_CHANCTX_QUEUE_NUM	2

static enum rtw_phl_status
_phl_cc_band_map_set(struct phl_info_t *phl_info,
		struct hw_band_ctl_t *band_ctrl, struct rtw_chan_ctx *chanctx)
{
	struct rtw_chan_def *chan_def = &chanctx->chan_def;
	static enum rtw_phl_status psts = RTW_PHL_STATUS_SUCCESS;

	if (BAND_2GHZ(chan_def->band)) {
		_os_atomic_inc(phl_to_drvpriv(phl_info), &band_ctrl->ccb_24g_rc);
		band_ctrl->chctx_band_map |= BIT(CC_BAND_24G);

	} else if (BAND_5GHZ(chan_def->band)) {
		if (CH_5GHZ_BAND1(chan_def->chan)) {
			_os_atomic_inc(phl_to_drvpriv(phl_info), &band_ctrl->ccb_5gb1_rc);
			band_ctrl->chctx_band_map |= BIT(CC_BAND_5G_B1);
		} else if (CH_5GHZ_BAND2(chan_def->chan)) {
			_os_atomic_inc(phl_to_drvpriv(phl_info), &band_ctrl->ccb_5gb2_rc);
			band_ctrl->chctx_band_map |= BIT(CC_BAND_5G_B2);
		} else if (CH_5GHZ_BAND3(chan_def->chan)) {
			_os_atomic_inc(phl_to_drvpriv(phl_info), &band_ctrl->ccb_5gb3_rc);
			band_ctrl->chctx_band_map |= BIT(CC_BAND_5G_B3);
		} else if (CH_5GHZ_BAND4(chan_def->chan)) {
			_os_atomic_inc(phl_to_drvpriv(phl_info), &band_ctrl->ccb_5gb4_rc);
			band_ctrl->chctx_band_map |= BIT(CC_BAND_5G_B4);
		} else {
			PHL_ERR("%s chctx 5G band unknow\n", __func__);
			_os_warn_on(1);
			psts = RTW_PHL_STATUS_FAILURE;
		}
	} else if (BAND_6GHZ(chan_def->band)) {
		_os_atomic_inc(phl_to_drvpriv(phl_info), &band_ctrl->ccb_6g_rc);
		band_ctrl->chctx_band_map |= BIT(CC_BAND_6G);
	}

	return psts;
}

static enum rtw_phl_status
_phl_cc_band_map_clr(struct phl_info_t *phl_info,
		struct hw_band_ctl_t *band_ctrl, struct rtw_chan_ctx *chanctx)
{
	struct rtw_chan_def *chan_def = &chanctx->chan_def;
	static enum rtw_phl_status psts = RTW_PHL_STATUS_SUCCESS;

	if (BAND_2GHZ(chan_def->band)) {
		_os_atomic_dec(phl_to_drvpriv(phl_info), &band_ctrl->ccb_24g_rc);
		if (_os_atomic_read(phl_to_drvpriv(phl_info), &band_ctrl->ccb_24g_rc) == 0)
			band_ctrl->chctx_band_map &= ~BIT(CC_BAND_24G);
	} else if (BAND_5GHZ(chan_def->band)) {
		if (CH_5GHZ_BAND1(chan_def->chan)) {
			_os_atomic_dec(phl_to_drvpriv(phl_info), &band_ctrl->ccb_5gb1_rc);
			if (_os_atomic_read(phl_to_drvpriv(phl_info), &band_ctrl->ccb_5gb1_rc) == 0)
				band_ctrl->chctx_band_map &= ~BIT(CC_BAND_5G_B1);
		} else if (CH_5GHZ_BAND2(chan_def->chan)) {
			_os_atomic_dec(phl_to_drvpriv(phl_info), &band_ctrl->ccb_5gb2_rc);
			if (_os_atomic_read(phl_to_drvpriv(phl_info), &band_ctrl->ccb_5gb2_rc) == 0)
				band_ctrl->chctx_band_map &= ~BIT(CC_BAND_5G_B2);
		} else if (CH_5GHZ_BAND3(chan_def->chan)) {
			_os_atomic_dec(phl_to_drvpriv(phl_info), &band_ctrl->ccb_5gb3_rc);
			if (_os_atomic_read(phl_to_drvpriv(phl_info), &band_ctrl->ccb_5gb3_rc) == 0)
				band_ctrl->chctx_band_map &= ~BIT(CC_BAND_5G_B3);
		} else if (CH_5GHZ_BAND4(chan_def->chan)) {
			_os_atomic_dec(phl_to_drvpriv(phl_info), &band_ctrl->ccb_5gb4_rc);
			if (_os_atomic_read(phl_to_drvpriv(phl_info), &band_ctrl->ccb_5gb4_rc) == 0)
				band_ctrl->chctx_band_map &= ~BIT(CC_BAND_5G_B4);
		} else {
			PHL_ERR("%s chctx 5G band unknow\n", __func__);
			_os_warn_on(1);
			psts = RTW_PHL_STATUS_FAILURE;
		}
	} else if (BAND_6GHZ(chan_def->band)) {
		_os_atomic_dec(phl_to_drvpriv(phl_info), &band_ctrl->ccb_6g_rc);
		if (_os_atomic_read(phl_to_drvpriv(phl_info), &band_ctrl->ccb_6g_rc) == 0)
			band_ctrl->chctx_band_map &= ~BIT(CC_BAND_6G);
	}

	return psts;
}

static enum rtw_phl_status
_phl_chanctx_add(struct phl_info_t *phl_info,
			struct hw_band_ctl_t *band_ctrl,
			struct rtw_chan_ctx *chanctx)
{
	struct phl_queue *chan_ctx_queue;
	enum rtw_phl_status psts = RTW_PHL_STATUS_FAILURE;

	if (!chanctx)
		return psts;

	chan_ctx_queue = &band_ctrl->chan_ctx_queue;

	list_add_tail(&chanctx->list, &chan_ctx_queue->queue);
	chan_ctx_queue->cnt++;
	if (chan_ctx_queue->cnt > MAX_CHANCTX_QUEUE_NUM) {
		PHL_ERR("%s chan_ctx_queue cnt(%d) > 2\n", __func__, chan_ctx_queue->cnt);
		_os_warn_on(1);
		goto _exit;
	}
	psts = _phl_cc_band_map_set(phl_info, band_ctrl, chanctx);

_exit:
	return psts;
}

static enum rtw_phl_status
_phl_chanctx_add_with_lock(struct phl_info_t *phl_info,
			struct hw_band_ctl_t *band_ctrl,
			struct rtw_chan_ctx *chanctx)
{
	void *drv = phl_to_drvpriv(phl_info);
	enum rtw_phl_status psts = RTW_PHL_STATUS_FAILURE;

	if (!chanctx)
		return psts;

	_os_spinlock(drv, &band_ctrl->chan_ctx_queue.lock, _bh, NULL);
	psts = _phl_chanctx_add(phl_info, band_ctrl, chanctx);
	_os_spinunlock(drv, &band_ctrl->chan_ctx_queue.lock, _bh, NULL);
	return psts;
}

static enum rtw_phl_status
_phl_chanctx_del(struct phl_info_t *phl_info,
			struct hw_band_ctl_t *band_ctrl,
			struct rtw_chan_ctx *chanctx)
{
	struct phl_queue *chan_ctx_queue;
	enum rtw_phl_status psts = RTW_PHL_STATUS_FAILURE;

	if (!chanctx)
		return psts;

	psts = _phl_cc_band_map_clr(phl_info,band_ctrl, chanctx);
	if (psts == RTW_PHL_STATUS_FAILURE) {
		PHL_ERR("%s _phl_cc_band_map_clr failed\n", __func__);
		_os_warn_on(1);
	}
	chan_ctx_queue = &band_ctrl->chan_ctx_queue;
	/*if (!list_empty(&chan_ctx_queue->queue)) {*/
	if (chan_ctx_queue->cnt) {
		list_del(&chanctx->list);
		chan_ctx_queue->cnt--;
		if (chan_ctx_queue->cnt < 0) {
			PHL_ERR("%s chan_ctx_queue cnt(%d) < 0\n", __func__, chan_ctx_queue->cnt);
			_os_warn_on(1);
			goto _exit;
		}
	}
	psts = RTW_PHL_STATUS_SUCCESS;
_exit:
	return psts;
}

static enum rtw_phl_status
_phl_chanctx_del_with_lock(struct phl_info_t *phl_info,
			struct hw_band_ctl_t *band_ctrl,
			struct rtw_chan_ctx *chanctx)
{
	void *drv = phl_to_drvpriv(phl_info);
	enum rtw_phl_status psts = RTW_PHL_STATUS_FAILURE;

	if (!chanctx)
		return psts;

	_os_spinlock(drv, &band_ctrl->chan_ctx_queue.lock, _bh, NULL);
	psts = _phl_chanctx_del(phl_info, band_ctrl, chanctx);
	_os_spinunlock(drv, &band_ctrl->chan_ctx_queue.lock, _bh, NULL);
	return psts;
}

static inline enum rtw_phl_status
_phl_chanctx_rmap_set(struct phl_info_t *phl_info,
			struct rtw_wifi_role_t *wifi_role,
			struct phl_queue *chan_ctx_queue,
			struct rtw_chan_ctx *chanctx)
{
	u8 ridx = wifi_role->id;

	if (!chanctx)
		return RTW_PHL_STATUS_FAILURE;

	#ifdef DBG_CHCTX_RMAP
	if (chanctx->role_map & BIT(ridx))
		PHL_ERR("wifi_role idx(%d) has in chanctx->role_map(0x%02x)\n",
				ridx, chanctx->role_map);
	#endif
	chanctx->role_map |= BIT(ridx);
	wifi_role->chanctx = chanctx;
	return RTW_PHL_STATUS_SUCCESS;
}

static enum rtw_phl_status
_phl_chanctx_rmap_set_with_lock(struct phl_info_t *phl_info,
			struct rtw_wifi_role_t *wifi_role,
			struct phl_queue *chan_ctx_queue,
			struct rtw_chan_ctx *chanctx)
{
	void *drv = phl_to_drvpriv(phl_info);

	if (!chanctx)
		return RTW_PHL_STATUS_FAILURE;

	_os_spinlock(drv, &chan_ctx_queue->lock, _bh, NULL);
	_phl_chanctx_rmap_set(phl_info, wifi_role, chan_ctx_queue, chanctx);
	_os_spinunlock(drv, &chan_ctx_queue->lock, _bh, NULL);
	return RTW_PHL_STATUS_SUCCESS;
}

static inline enum rtw_phl_status
_phl_chanctx_rmap_clr(struct phl_info_t *phl_info,
			struct rtw_wifi_role_t *wifi_role,
			struct phl_queue *chan_ctx_queue,
			struct rtw_chan_ctx *chanctx)
{
	u8 ridx = wifi_role->id;

	if (!chanctx)
		return RTW_PHL_STATUS_FAILURE;

	#ifdef DBG_CHCTX_RMAP
	if (!(chanctx->role_map & BIT(ridx)))
		PHL_ERR("ridx(%d) hasn't in chanctx->role_map(0x%02x)\n", ridx, chanctx->role_map);
	#endif
	wifi_role->chanctx = NULL;
	chanctx->role_map &= ~BIT(ridx);

	return RTW_PHL_STATUS_SUCCESS;
}

static enum rtw_phl_status
_phl_chanctx_rmap_clr_with_lock(struct phl_info_t *phl_info,
			struct rtw_wifi_role_t *wifi_role,
			struct phl_queue *chan_ctx_queue,
			struct rtw_chan_ctx *chanctx)
{
	void *drv = phl_to_drvpriv(phl_info);

	if (!chanctx)
		return RTW_PHL_STATUS_FAILURE;

	_os_spinlock(drv, &chan_ctx_queue->lock, _bh, NULL);
	_phl_chanctx_rmap_clr(phl_info, wifi_role, chan_ctx_queue, chanctx);
	_os_spinunlock(drv, &chan_ctx_queue->lock, _bh, NULL);
	return RTW_PHL_STATUS_SUCCESS;
}

u8 phl_chanctx_get_rnum(struct phl_info_t *phl_info,
					struct phl_queue *chan_ctx_queue,
					struct rtw_chan_ctx *chanctx)
{
	u8 i;
	u8 role_num = 0;

	for (i = 0; i < MAX_WIFI_ROLE_NUMBER; i++)
		if (chanctx->role_map & BIT(i))
			role_num++;
	return role_num;
}

u8 phl_chanctx_get_rnum_with_lock(struct phl_info_t *phl_info,
			struct phl_queue *chan_ctx_queue,
			struct rtw_chan_ctx *chanctx)
{
	void *drv = phl_to_drvpriv(phl_info);
	u8 role_num = 0;

	if (!chanctx)
		return role_num;

	_os_spinlock(drv, &chan_ctx_queue->lock, _bh, NULL);
	role_num = phl_chanctx_get_rnum(phl_info, chan_ctx_queue, chanctx);
	_os_spinunlock(drv, &chan_ctx_queue->lock, _bh, NULL);
	return role_num;
}

/**
 * _phl_is_chbw_grouped - test if the two ch settings can be grouped together
 * @chandef_a : chandef of set a
 * @chandef_b : chandef of set b
 */
static bool _phl_is_chbw_grouped(struct rtw_chan_def *chandef_a,
	struct rtw_chan_def *chandef_b)
{
	bool is_grouped = false;

	if (chandef_a->band != chandef_b->band)
		goto exit;

	if (chandef_a->chan != chandef_b->chan) {
		/* ch is different */
		goto exit;
	} else if ((chandef_a->bw == CHANNEL_WIDTH_40 || chandef_a->bw == CHANNEL_WIDTH_80)
		   && (chandef_b->bw == CHANNEL_WIDTH_40 || chandef_b->bw == CHANNEL_WIDTH_80)
		  ) {
		if (chandef_a->offset != chandef_b->offset)
			goto exit;
	}

	is_grouped = true;

exit:
	return is_grouped;
}


static inline bool
_phl_feature_check(struct rtw_phl_com_t *phl_com, u8 flg)
{
	return (phl_com->dev_cap.hw_sup_flags & flg) ? true : false;
}

static u8 _phl_get_offset_by_chbw(u8 ch, enum channel_width bw, enum chan_offset *r_offset)
{
	u8 valid = 1;
	enum chan_offset offset = CHAN_OFFSET_NO_EXT;

	if (bw == CHANNEL_WIDTH_20)
		goto exit;

	if (bw >= CHANNEL_WIDTH_80 && ch <= 14) {
		valid = 0;
		goto exit;
	}

	if (ch >= 1 && ch <= 4)
		offset = CHAN_OFFSET_UPPER;
	else if (ch >= 5 && ch <= 9) {
		if (*r_offset == CHAN_OFFSET_UPPER || *r_offset == CHAN_OFFSET_LOWER)
			offset = *r_offset; /* both lower and upper is valid, obey input value */
		else
			offset = CHAN_OFFSET_LOWER; /* default use upper */
	} else if (ch >= 10 && ch <= 13)
		offset = CHAN_OFFSET_LOWER;
	else if (ch == 14) {
		valid = 0; /* ch14 doesn't support 40MHz bandwidth */
		goto exit;
	} else if (ch >= 36 && ch <= 177) {
		switch (ch) {
		case 36:
		case 44:
		case 52:
		case 60:
		case 100:
		case 108:
		case 116:
		case 124:
		case 132:
		case 140:
		case 149:
		case 157:
		case 165:
		case 173:
			offset = CHAN_OFFSET_UPPER;
			break;
		case 40:
		case 48:
		case 56:
		case 64:
		case 104:
		case 112:
		case 120:
		case 128:
		case 136:
		case 144:
		case 153:
		case 161:
		case 169:
		case 177:
			offset = CHAN_OFFSET_LOWER;
			break;
		default:
			valid = 0;
			break;
		}
	} else
		valid = 0;

exit:
	if (valid && r_offset)
		*r_offset = offset;
	return valid;
}

/**
 * _phl_adjust_chandef - obey g_ch, adjust g_bw, g_offset, bw, offset
 * @req_ch: pointer of the request ch, may be modified further
 * @req_bw: pointer of the request bw, may be modified further
 * @req_offset: pointer of the request offset, may be modified further
 * @g_ch: pointer of the ongoing group ch
 * @g_bw: pointer of the ongoing group bw, may be modified further
 * @g_offset: pointer of the ongoing group offset, may be modified further
 */
static void _phl_adjust_chandef(u8 *req_ch, enum channel_width *req_bw, enum chan_offset *req_offset,
		   u8 *g_ch, enum channel_width *g_bw, enum chan_offset *g_offset)
{

	*req_ch = *g_ch;

	if (*req_bw == CHANNEL_WIDTH_80 && *g_ch <= 14) {
		/*2.4G ch, downgrade to 40Mhz */
		*req_bw = CHANNEL_WIDTH_40;
	}

	switch (*req_bw) {
	case CHANNEL_WIDTH_80:
		if (*g_bw == CHANNEL_WIDTH_40 || *g_bw == CHANNEL_WIDTH_80)
			*req_offset = *g_offset;
		else if (*g_bw == CHANNEL_WIDTH_20)
			_phl_get_offset_by_chbw(*req_ch, *req_bw, req_offset);

		if (*req_offset == CHAN_OFFSET_NO_EXT) {
			PHL_ERR("%s req 80MHz BW without offset, down to 20MHz\n", __func__);
			_os_warn_on(1);
			*req_bw = CHANNEL_WIDTH_20;
		}
		break;
	case CHANNEL_WIDTH_40:
		if (*g_bw == CHANNEL_WIDTH_40 || *g_bw == CHANNEL_WIDTH_80)
			*req_offset = *g_offset;
		else if (*g_bw == CHANNEL_WIDTH_20)
			_phl_get_offset_by_chbw(*req_ch, *req_bw, req_offset);

		if (*req_offset == CHAN_OFFSET_NO_EXT) {
			PHL_ERR("%s req 40MHz BW without offset, down to 20MHz\n", __func__);
			_os_warn_on(1);
			*req_bw = CHANNEL_WIDTH_20;
		}
		break;
	case CHANNEL_WIDTH_20:
		*req_offset = CHAN_OFFSET_NO_EXT;
		break;
	default:
		PHL_ERR("%s req unsupported BW:%u\n", __func__, *req_bw);
		_os_warn_on(1);
	}

	if (*req_bw > *g_bw) {
		*g_bw = *req_bw;
		*g_offset = *req_offset;
	}
}

static enum rtw_phl_status
_phl_chanctx_create(struct phl_info_t *phl_info,
		struct rtw_wifi_role_t *wifi_role,
		struct rtw_chan_def *new_chdef)
{
	enum rtw_phl_status phl_sts = RTW_PHL_STATUS_FAILURE;
	void *drv = phl_to_drvpriv(phl_info);
	struct rtw_chan_ctx *chanctx = NULL;
	struct mr_ctl_t *mr_ctl = phlcom_to_mr_ctrl(phl_info->phl_com);
	struct hw_band_ctl_t *band_ctrl = &(mr_ctl->band_ctrl[wifi_role->hw_band]);

	chanctx = _os_kmem_alloc(drv, sizeof(struct rtw_chan_ctx));
	if (chanctx == NULL) {
		PHL_ERR("alloc chanctx failed\n");
		goto _exit;
	}

	_os_mem_cpy(drv, &chanctx->chan_def, new_chdef, sizeof(struct rtw_chan_def));
	phl_sts = _phl_chanctx_add_with_lock(phl_info, band_ctrl, chanctx);

	if (phl_sts == RTW_PHL_STATUS_SUCCESS)
		_phl_chanctx_rmap_set_with_lock(phl_info, wifi_role,
					&band_ctrl->chan_ctx_queue, chanctx);
_exit:
	return phl_sts;
}

enum rtw_phl_status
phl_chanctx_free(struct phl_info_t *phl_info, struct hw_band_ctl_t *band_ctl)
{
	int chanctx_num = 0;
	struct rtw_chan_ctx *chanctx = NULL;
	struct phl_queue *chan_ctx_queue = &band_ctl->chan_ctx_queue;
	void *drv = phl_to_drvpriv(phl_info);

	chanctx_num = phl_mr_get_chanctx_num(phl_info, band_ctl);
	if (chanctx_num == 0)
		return RTW_PHL_STATUS_SUCCESS;

	PHL_INFO("%s band_idx:%d chctx_num:%d\n", __func__, band_ctl->id, chanctx_num);
	do {
		_os_spinlock(drv, &band_ctl->chan_ctx_queue.lock, _bh, NULL);
		if (list_empty(&chan_ctx_queue->queue)) {
			chanctx = NULL;
		} else {
			chanctx = list_first_entry(&chan_ctx_queue->queue,
						struct rtw_chan_ctx, list);
			list_del(&chanctx->list);
			chan_ctx_queue->cnt--;
		}
		_os_spinunlock(drv, &band_ctl->chan_ctx_queue.lock, _bh, NULL);

		if (chanctx) {
			_os_kmem_free(drv, chanctx, sizeof(struct rtw_chan_ctx));
		}
	} while (chanctx != NULL);
	return RTW_PHL_STATUS_SUCCESS;
}

/* used for get all role under band_idx */
u8 phl_get_chanctx_rolemap(struct phl_info_t *phl_info, u8 band_idx)
{
	void *drv = phl_to_drvpriv(phl_info);
	struct hw_band_ctl_t *band_ctrl = get_band_ctrl(phl_info, band_idx);
	_os_list *chan_ctx_list = &band_ctrl->chan_ctx_queue.queue;
	struct rtw_chan_ctx *chanctx = NULL;
	u8 role_map =0;

	_os_spinlock(drv, &band_ctrl->chan_ctx_queue.lock, _bh, NULL);
	phl_list_for_loop(chanctx, struct rtw_chan_ctx, chan_ctx_list, list) {
		role_map |= chanctx->role_map;
	}
	_os_spinunlock(drv, &band_ctrl->chan_ctx_queue.lock, _bh, NULL);

	return role_map;
}

static bool
is_ch_in_same_band(struct rtw_chan_def *chdef_a, struct rtw_chan_def *chdef_b)
{
	bool ch_in_same_band = false;

	if (chdef_a->band == chdef_b->band) {
		if (BAND_2GHZ(chdef_a->band)) {
			ch_in_same_band = true;
		} else if (BAND_5GHZ(chdef_b->band)) {
			if (CH_IN_SAME_5G_BAND(chdef_a->chan, chdef_b->chan))
				ch_in_same_band = true;
		} else if (BAND_6GHZ(chdef_b->band)) {
				ch_in_same_band = true;
		}
	}
	return ch_in_same_band;
}

static void _phl_dump_mr_cc_info(struct phl_info_t *phl_info,
					struct rtw_mr_chctx_info *mr_cc_info)
{
	PHL_INFO("[CC-INFO] is single band:%s\n", (mr_cc_info->is_sb) ? "Y" : "N");
	PHL_INFO("[CC-INFO] cur_band_idx:%d\n", mr_cc_info->cur_band_idx);
	PHL_INFO("[CC-INFO] role_num:%d\n", mr_cc_info->role_num);
	PHL_INFO("[CC-INFO] chctx_num:%d\n", mr_cc_info->chctx_num);

	PHL_INFO("[CC-INFO] chctx_band_map:0x%04x\n", mr_cc_info->chctx_band_map);
	phl_dump_cc_band_map(phl_info, mr_cc_info->chctx_band_map);

	if (mr_cc_info->sugg_opmode == MR_OP_SWR)
		PHL_INFO("[CC-INFO] suggest opmode:%s\n", "SINGLE ROLE");
	else if (mr_cc_info->sugg_opmode == MR_OP_SCC)
		PHL_INFO("[CC-INFO] suggest opmode:%s\n", "SCC");
	else if (mr_cc_info->sugg_opmode == MR_OP_MCC)
		PHL_INFO("[CC-INFO] suggest opmode:%s\n", "MCC");
	else if (mr_cc_info->sugg_opmode == MR_OP_DBCC)
		PHL_INFO("[CC-INFO] suggest opmode:%s\n", "DBCC");
	else
		PHL_INFO("[CC-INFO] suggest opmode:%s\n", "NON");
}

bool rtw_phl_chanctx_chk(void *phl, struct rtw_wifi_role_t *wifi_role,
		struct rtw_chan_def *new_chdef,
		struct rtw_mr_chctx_info *mr_cc_info)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct rtw_phl_com_t *phl_com = phl_info->phl_com;
	void *drv = phl_to_drvpriv(phl_info);
	struct mr_ctl_t *mr_ctl = phlcom_to_mr_ctrl(phl_com);
	u8 band_idx = wifi_role->hw_band;
	bool is_ch_group = false;
	struct hw_band_ctl_t *band_ctrl = &(mr_ctl->band_ctrl[band_idx]);
	int chanctx_num = 0;
	struct rtw_chan_ctx *chanctx = NULL;
	struct rtw_chan_def *chandef = NULL;
	enum mr_op_mode sop_mode = MR_OP_NONE;
	u8 band_role_num = 0;

	if (new_chdef && new_chdef->chan == 0) {
		PHL_ERR("%s req chan = 0 \n", __func__);
		_os_warn_on(1);
		goto _exit;
	}

	#if 0 /*status check for debug*/
	if (mr_ctl->is_sb == false) { /*in DBCC*/
		struct hw_band_ctl_t *tmp_band_ctrl = NULL;
		u8 band_idx;

		for (band_idx = 0; band_idx < HW_BAND_MAX; band_idx++) {
			tmp_band_ctrl = &mr_ctl->band_ctrl[band_idx];
			band_role_num = phl_mr_get_role_num(phl_info, tmp_band_ctrl);
			chanctx_num = phl_mr_get_chanctx_num(phl_info, tmp_band_ctrl);
			PHL_INFO("BAND_%d : role num:%d, chctx num:%d\n",
				  band_idx, band_role_num, chanctx_num);
		}
	}
	#endif

	band_role_num = phl_mr_get_role_num(phl_info, band_ctrl);

	chanctx_num = phl_mr_get_chanctx_num(phl_info, band_ctrl);

	if (new_chdef) {
		if (chanctx_num == 0) {
			is_ch_group = true;
			sop_mode = MR_OP_SWR;
			goto _exit;
		}

		_os_spinlock(drv, &band_ctrl->chan_ctx_queue.lock, _bh, NULL);
		phl_list_for_loop(chanctx, struct rtw_chan_ctx, &band_ctrl->chan_ctx_queue.queue, list) {
			chandef = &chanctx->chan_def;
			is_ch_group = _phl_is_chbw_grouped(chandef, new_chdef);
			if (is_ch_group)
				break;
		}
		_os_spinunlock(drv, &band_ctrl->chan_ctx_queue.lock, _bh, NULL);

		if (is_ch_group == true) {/* SCC or MCC */
			/*current chanctx_num*/
			if (chanctx_num == 1) {
				sop_mode = MR_OP_SCC;
			}
			else if (chanctx_num == 2) {
				#ifdef CONFIG_MCC_SUPPORT
				if (phl_com->dev_cap.mcc_sup == true) {
					sop_mode = MR_OP_MCC;
				} else
				#endif
				{
					PHL_ERR("Don't support MCC, but cur chanctx num = 2 ??\n");
					_os_warn_on(1);
				}
			}
			else {
				PHL_ERR("%s Unknow state - chan can grouped, ch-ctx num:%d\n",
							__func__, chanctx_num);
				_os_warn_on(1);
			}
			goto _exit;
		}

		if (chanctx_num >= 2) {
			PHL_ERR("current chan_ctx num(%d) >= 2, not support over 3xchan-set on one band\n",
									chanctx_num);
			#ifdef CONFIG_DBCC_SUPPORT
			/*TODO : Consider scenario - [MCC] 2G-CH6+5G-CH149, New ch is 2G-CH11*/
			if (mr_cc_info->mcmb_skip_dbcc == false && phl_com->dev_cap.dbcc_sup == true)
				sop_mode = MR_OP_DBCC;
			#endif
			goto _exit;
		}

		/* MCC,condition- chanctx_num == 1 */
		if (is_ch_in_same_band(new_chdef, chandef)) {/* MC-SB */
			PHL_INFO("%s - case MC-SB\n", __func__);

			#ifdef CONFIG_MCC_SUPPORT
			if (phl_com->dev_cap.mcc_sup == true) {
				is_ch_group = true;
				sop_mode = MR_OP_MCC;
			} else
			#endif
			{
				PHL_ERR("%s don't support MCC\n", __func__);
			}
			goto _exit;
		}

		/* MC-MB */
		/* DBCC */
		#ifdef CONFIG_DBCC_SUPPORT
		if (mr_cc_info->mcmb_skip_dbcc == false && phl_com->dev_cap.dbcc_sup == true) {
			PHL_INFO("%s support DBCC\n", __func__);
			/* suggest DBCC */
			sop_mode = MR_OP_DBCC;
			goto _exit;
		}
		#endif

		/* MCC */
		#ifdef CONFIG_MCC_SUPPORT
		if (phl_com->dev_cap.mcc_sup == true) {
			PHL_INFO("%s support MCC\n", __func__);
			is_ch_group = true;
			sop_mode = MR_OP_MCC;
		}
		#endif
	} else {
		if (mr_ctl->is_sb == true) { /*stay in single band*/
			sop_mode = band_ctrl->op_mode;
		} else {/*stay in dual band*/
			#ifdef CONFIG_DBCC_SUPPORT
			if (band_ctrl->op_mode == MR_OP_NONE)
				sop_mode = MR_OP_DBCC;
			#endif
		}
	}
_exit:
	if (mr_cc_info) {
		mr_cc_info->is_sb = mr_ctl->is_sb;
		mr_cc_info->cur_band_idx = band_idx;
		mr_cc_info->role_num = band_role_num;
		mr_cc_info->chctx_num = (u8)chanctx_num;
		mr_cc_info->chctx_band_map = band_ctrl->chctx_band_map;
		mr_cc_info->sugg_opmode = sop_mode;
		_phl_dump_mr_cc_info(phl_info, mr_cc_info);
	}

	PHL_DUMP_MR_EX(phl_info);
	return is_ch_group;
}

static bool phl_chanctx_add(struct phl_info_t *phl_info,
		struct rtw_wifi_role_t *wifi_role,
		struct rtw_chan_def *new_chdef,
		struct rtw_mr_chctx_info *mr_cc_info)
{
	struct rtw_phl_com_t *phl_com = phl_info->phl_com;
	enum rtw_phl_status phl_sts = RTW_PHL_STATUS_FAILURE;
	void *drv = phl_to_drvpriv(phl_info);
	struct mr_ctl_t *mr_ctl = phlcom_to_mr_ctrl(phl_com);
	u8 band_idx = wifi_role->hw_band;
	struct hw_band_ctl_t *band_ctrl = &(mr_ctl->band_ctrl[band_idx]);
	struct rtw_chan_ctx *chanctx = NULL;
	struct rtw_chan_def *chandef = NULL;
	_os_list *chan_ctx_list = &band_ctrl->chan_ctx_queue.queue;
	bool is_ch_grouped = false;
	enum mr_op_mode sop_mode = MR_OP_NONE;
	u8 band_role_num = 0;
	int chanctx_num = 0;

	if (wifi_role == NULL) {
		PHL_ERR("%s wrole == NULL\n", __func__);
		goto _exit;
	}

	PHL_INFO("%s new chan_def - hw_band_idx:%d, band:%u, chan:%d, bw:%d, offset:%d\n",
		__func__, wifi_role->hw_band, new_chdef->band,
		new_chdef->chan, new_chdef->bw, new_chdef->offset);

	band_ctrl->op_mode = sop_mode;
	band_role_num = phl_mr_get_role_num(phl_info, band_ctrl);

	chanctx_num = phl_mr_get_chanctx_num(phl_info, band_ctrl);
	if (chanctx_num == 0) {
		phl_sts = _phl_chanctx_create(phl_info, wifi_role, new_chdef);
		if (phl_sts == RTW_PHL_STATUS_SUCCESS) {
			is_ch_grouped = true;
			sop_mode = MR_OP_SWR;
			band_ctrl->op_mode = sop_mode;
		}
		else {
			PHL_ERR("%s %d create chanctx failed\n", __FUNCTION__, __LINE__);
			_os_warn_on(1);
		}
		goto _exit;
	}

	_os_spinlock(drv, &band_ctrl->chan_ctx_queue.lock, _bh, NULL);
	phl_list_for_loop(chanctx, struct rtw_chan_ctx, chan_ctx_list, list) {
		chandef = &chanctx->chan_def;
		is_ch_grouped = _phl_is_chbw_grouped(chandef, new_chdef);
		if (is_ch_grouped) {
			_phl_adjust_chandef(&new_chdef->chan, &new_chdef->bw, &new_chdef->offset,
				&chandef->chan, &chandef->bw, &chandef->offset);
			new_chdef->chan = chandef->chan;
			new_chdef->bw = chandef->bw;
			new_chdef->offset = chandef->offset;
			_phl_chanctx_rmap_set(phl_info, wifi_role,
				&band_ctrl->chan_ctx_queue, chanctx);
			break;
		}
	}
	_os_spinunlock(drv, &band_ctrl->chan_ctx_queue.lock, _bh, NULL);

	if (is_ch_grouped == true) {/****** SCC or MCC ******/
		PHL_INFO("%s grouped chan_def - hw_band_idx:%d, chan:%d, bw:%d, offset:%d\n",
				__func__, wifi_role->hw_band, new_chdef->chan, new_chdef->bw, new_chdef->offset);
		/*current chanctx_num*/
		if (chanctx_num == 1) {
			sop_mode = MR_OP_SCC;
			band_ctrl->op_mode = sop_mode;
		}
		else if (chanctx_num == 2) {
			#ifdef CONFIG_MCC_SUPPORT
			if (phl_com->dev_cap.mcc_sup == true) {
				sop_mode = MR_OP_MCC;
				band_ctrl->op_mode = sop_mode;
			} else
			#endif
			{
				PHL_ERR("Don't support MCC, but cur chanctx num = 2 ??\n");
				_os_warn_on(1);
			}
		}
		else {
			PHL_ERR("%s Unknow state - chan can grouped, ch-ctx num:%d\n",
						__func__, chanctx_num);
			_os_warn_on(1);
		}
		goto _exit;
	}

	if (chanctx_num >= 2) {
		PHL_ERR("current chan_ctx num(%d) >= 2, not support over 3xchan-set on one band\n",
								chanctx_num);
		#ifdef CONFIG_DBCC_SUPPORT
		/*TODO : Consider scenario - [MCC] 2G-CH6+5G-CH149, New ch is 2G-CH11*/
		if (mr_cc_info->mcmb_skip_dbcc == false && phl_com->dev_cap.dbcc_sup == true)
			sop_mode = MR_OP_DBCC;
		#endif
		goto _exit;
	}

	/* MCC - chanctx_num == 1 */
	if (is_ch_in_same_band(new_chdef, chandef)) {/* MC-SB */
		PHL_INFO("%s - case MC-SB\n", __func__);
		#ifdef CONFIG_MCC_SUPPORT
		if (phl_com->dev_cap.mcc_sup == true) {
			phl_sts = _phl_chanctx_create(phl_info, wifi_role,
						new_chdef);
			if (phl_sts == RTW_PHL_STATUS_SUCCESS) {
				is_ch_grouped = true;
				PHL_INFO("%s chan:%d, bw:%d, offset:%d could be created - MCC\n",
				__func__, new_chdef->chan, new_chdef->bw, new_chdef->offset);
				sop_mode = MR_OP_MCC;
				band_ctrl->op_mode = sop_mode;
			} else {
				PHL_ERR("%s %d create chanctx failed\n", __FUNCTION__, __LINE__);
				_os_warn_on(1);
			}
		} else
		#endif
		{
			PHL_ERR("%s don't support MCC\n", __func__);
		}
		goto _exit;
	}

	/* MC-MB */
	/* DBCC */
	#ifdef CONFIG_DBCC_SUPPORT
	if (mr_cc_info->mcmb_skip_dbcc == false && phl_com->dev_cap.dbcc_sup == true) {
		PHL_INFO("%s support DBCC\n", __func__);
		sop_mode = MR_OP_DBCC;
		goto _exit;
	}
	#endif

	/* MCC */
	#ifdef CONFIG_MCC_SUPPORT
	if (phl_com->dev_cap.mcc_sup == true) {
		phl_sts = _phl_chanctx_create(phl_info, wifi_role, new_chdef);
		if (phl_sts == RTW_PHL_STATUS_SUCCESS) {
			is_ch_grouped = true;
			PHL_INFO("%s chan:%d, bw:%d, offset:%d could be created - MCC\n",
				__func__, new_chdef->chan, new_chdef->bw, new_chdef->offset);
			sop_mode = MR_OP_MCC;
			band_ctrl->op_mode = sop_mode;
		} else {
			PHL_ERR("%s %d create chanctx failed\n", __FUNCTION__, __LINE__);
			_os_warn_on(1);
		}
	}
	#endif

_exit:
	if (mr_cc_info) {
		mr_cc_info->is_sb = mr_ctl->is_sb;
		mr_cc_info->cur_band_idx = band_idx;
		mr_cc_info->role_num = band_role_num;
		mr_cc_info->chctx_num = (u8)chanctx_num;
		mr_cc_info->chctx_band_map = band_ctrl->chctx_band_map;
		mr_cc_info->sugg_opmode = sop_mode;
		_phl_dump_mr_cc_info(phl_info, mr_cc_info);
	}
	PHL_DUMP_MR_EX(phl_info);

	return is_ch_grouped;
}


/*
 * Add new operating chdef to MR.
 * @new_chan: Input: new chdef; Output: the final operating ch ctx.
 * ex: In the scc case, it will be the group chdef.
 */
bool rtw_phl_chanctx_add(void *phl, struct rtw_wifi_role_t *wifi_role,
		struct rtw_chan_def *new_chdef,
		struct rtw_mr_chctx_info *mr_cc_info)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct rtw_phl_com_t *phl_com = phl_info->phl_com;
	struct mr_ctl_t *mr_ctl = phlcom_to_mr_ctrl(phl_com);
	u8 band_idx = wifi_role->hw_band;
	bool is_ch_grouped = false;

	if(new_chdef == NULL)
		goto _exit;

	if (new_chdef->chan == 0) {
		PHL_ERR("%s req chan = 0 \n", __func__);
		goto _exit;
	}

	/*status check*/
	if (mr_ctl->is_sb) {
		if (band_idx == 1) {
			PHL_ERR("wrole:%d in band_idx:%d\n", wifi_role->id, band_idx);
			goto _exit;
		}
	}

	is_ch_grouped = phl_chanctx_add(phl_info, wifi_role, new_chdef, mr_cc_info);

_exit:
	PHL_DUMP_MR_EX(phl_info);
	return is_ch_grouped;
}

enum rtw_phl_status
rtw_phl_chanctx_del_no_self(void *phl, struct rtw_wifi_role_t *wifi_role)
{
	enum rtw_phl_status phl_sts = RTW_PHL_STATUS_FAILURE;
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct rtw_phl_com_t *phl_com = phl_info->phl_com;
	void *drv = phl_to_drvpriv(phl_info);
	struct mr_ctl_t *mr_ctl = phlcom_to_mr_ctrl(phl_com);
	struct hw_band_ctl_t *band_ctrl = &(mr_ctl->band_ctrl[wifi_role->hw_band]);
	struct rtw_chan_ctx *chanctx = NULL;
	int chctx_num = 0;

	chctx_num = phl_mr_get_chanctx_num(phl_info, band_ctrl);
	if (chctx_num > 2) {
		PHL_ERR("%s ERR - chanctx_num(%d) > 2\n", __func__, chctx_num);
		_os_warn_on(1);
		goto _exit;
	}

	if (chctx_num == 0) {
		phl_sts = RTW_PHL_STATUS_SUCCESS;
		PHL_INFO("%s - chctx_num = 0\n", __func__);
		goto _exit;
	}
	else if (chctx_num == 1) { /*SCC*/
		_os_spinlock(drv, &band_ctrl->chan_ctx_queue.lock, _bh, NULL);
		if (!list_empty(&band_ctrl->chan_ctx_queue.queue)) {
			chanctx = list_first_entry(&band_ctrl->chan_ctx_queue.queue,
							struct rtw_chan_ctx, list);
			phl_sts = _phl_chanctx_del(phl_info, band_ctrl, chanctx);
			if (phl_sts != RTW_PHL_STATUS_SUCCESS) {
				PHL_ERR("%s _phl_chanctx_del failed\n", __func__);
				_os_warn_on(1);
			}
		}
		_os_spinunlock(drv, &band_ctrl->chan_ctx_queue.lock, _bh, NULL);
		_os_kmem_free(drv, chanctx, sizeof(struct rtw_chan_ctx));
	}
	else if (chctx_num == 2) { /*MCC*/
	}

_exit:
	PHL_DUMP_MR_EX(phl_info);
	return phl_sts;
}

int rtw_phl_chanctx_del(void *phl, struct rtw_wifi_role_t *wifi_role,
						struct rtw_chan_def *chan_def)
{
	enum rtw_phl_status phl_sts = RTW_PHL_STATUS_FAILURE;
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct rtw_phl_com_t *phl_com = phl_info->phl_com;
	void *drv = phl_to_drvpriv(phl_info);
	struct mr_ctl_t *mr_ctl = phlcom_to_mr_ctrl(phl_com);
	u8 band_idx = wifi_role->hw_band;
	struct hw_band_ctl_t *band_ctrl = &(mr_ctl->band_ctrl[band_idx]);
	struct phl_queue *chan_ctx_queue = &band_ctrl->chan_ctx_queue;
	struct rtw_chan_ctx *target_chanctx = NULL;
	struct rtw_chan_ctx *chanctx = NULL;
	int chctx_num = 0;
	u8 chctx_role_num = 0;
	u8 band_role_num = 0;

	if (wifi_role == NULL) {
		PHL_ERR("%s wifi_role == NULL!!\n", __func__);
		/*_os_warn_on(1);*/
		goto _exit;
	}

	target_chanctx = wifi_role->chanctx;
	if (target_chanctx == NULL) {
		PHL_ERR("%s wifi_role->chanctx == NULL\n", __func__);
		/*_os_warn_on(1);*/
		goto _exit;
	}
	/*init chan_def*/
	if (chan_def)
		chan_def->chan = 0;

	chctx_num = phl_mr_get_chanctx_num(phl_info, band_ctrl);
	band_role_num = phl_mr_get_role_num(phl_info, band_ctrl);

	chctx_role_num = phl_chanctx_get_rnum_with_lock(phl_info, chan_ctx_queue, target_chanctx);

	if (chctx_num == 0 || chctx_role_num == 0) {
		PHL_ERR("%s ERR - chanctx_num(%d), role_num(%d)\n", __func__, chctx_num, chctx_role_num);
		_os_warn_on(1);
		goto _exit;
	}
	if (chctx_num > 2) {
		PHL_ERR("%s ERR - chanctx_num(%d) > 2\n", __func__, chctx_num);
		_os_warn_on(1);
		goto _exit;
	}

	if (chctx_role_num == 1) { /*single role on this chctx*/
		_os_spinlock(drv, &chan_ctx_queue->lock, _bh, NULL);
		phl_sts = _phl_chanctx_rmap_clr(phl_info, wifi_role,
						chan_ctx_queue, target_chanctx);
		if (phl_sts != RTW_PHL_STATUS_SUCCESS)
			PHL_ERR("_phl_chanctx_rmap_clr failed\n");

		phl_sts = _phl_chanctx_del(phl_info, band_ctrl, target_chanctx);
		if (phl_sts != RTW_PHL_STATUS_SUCCESS)
			PHL_ERR("_phl_chanctx_del failed\n");
		_os_spinunlock(drv, &chan_ctx_queue->lock, _bh, NULL);

		_os_kmem_free(drv, target_chanctx, sizeof(struct rtw_chan_ctx));

	} else { /*multi roles on this chctx*/
		phl_sts = _phl_chanctx_rmap_clr_with_lock(phl_info, wifi_role,
						chan_ctx_queue, target_chanctx);
		if (phl_sts != RTW_PHL_STATUS_SUCCESS)
			PHL_ERR("_phl_chanctx_rmap_clr_with_lock failed\n");

		phl_sts = phl_mr_chandef_upt(phl_info, band_ctrl, target_chanctx);
		if (phl_sts != RTW_PHL_STATUS_SUCCESS) {
			PHL_ERR("phl_mr_chandef_upt failed\n");
			_os_warn_on(1);
			goto _exit;
		}
	}

	chctx_num = phl_mr_get_chanctx_num(phl_info, band_ctrl);
	if (chctx_num == 0) {
		band_ctrl->op_mode = MR_OP_NONE;
	}
	else if (chctx_num == 1) {
		_os_spinlock(drv, &chan_ctx_queue->lock, _bh, NULL);
		chanctx = list_first_entry(&chan_ctx_queue->queue,
						struct rtw_chan_ctx, list);
		chctx_role_num = phl_chanctx_get_rnum(phl_info, chan_ctx_queue, chanctx);
		if (chan_def)
			_os_mem_cpy(drv, chan_def, &chanctx->chan_def, sizeof(struct rtw_chan_def));
		_os_spinunlock(drv, &chan_ctx_queue->lock, _bh, NULL);

		#ifdef DBG_PHL_MR
		if (chctx_role_num == 0) {
			PHL_ERR("chctx_num=1, chctx_role_num=0\n");
			_os_warn_on(1);
		}
		#endif
		band_ctrl->op_mode = (chctx_role_num == 1) ? MR_OP_SWR : MR_OP_SCC;
	} else if (chctx_num == 2) {
		if (chan_def)
			_os_mem_cpy(drv, chan_def, &target_chanctx->chan_def, sizeof(struct rtw_chan_def));
		band_ctrl->op_mode = MR_OP_MCC;
	}

	phl_sts = RTW_PHL_STATUS_SUCCESS;
	PHL_INFO("%s - Bidx(%d) - Total role_num:%d, chctx_num:%d, target-chctx rnum:%d, op_mode:%d\n",
		__func__, band_ctrl->id, band_role_num, chctx_num, chctx_role_num, band_ctrl->op_mode);

_exit:
	PHL_DUMP_MR_EX(phl_info);
	return chctx_num;
}

#ifdef	PHL_MR_PROC_CMD
bool rtw_phl_chanctx_test(void *phl,
				struct rtw_wifi_role_t *wifi_role,
				u8 test_mode,/*0-chk, 1-add, 2-del*/
				struct rtw_chan_def *new_chdef,
				struct rtw_mr_chctx_info *mr_cc_info)
{
	bool rst = true;
	int chanctx_num = 0;
	struct rtw_chan_def chan_def = {0};

	if (test_mode == 1) {
		rst = rtw_phl_chanctx_add(phl, wifi_role, new_chdef, mr_cc_info);
	}
	else if (test_mode == 2){
		chanctx_num = rtw_phl_chanctx_del(phl, wifi_role, &chan_def);
		PHL_ERR("%s chctx_num = %d\n", __func__, chanctx_num);
		PHL_DUMP_CHAN_DEF(&chan_def);
	}
	else if (test_mode == 0){
		rst = rtw_phl_chanctx_chk(phl, wifi_role, new_chdef, mr_cc_info);
	}
	return rst;
}
#endif
enum band_type rtw_phl_get_band_type(u8 chan)
{
	/*TODO - BAND_ON_6G*/
	return (chan > 14) ? BAND_ON_5G : BAND_ON_24G;
}
u8 rtw_phl_get_center_ch(struct rtw_chan_def *chan_def)
{
	u8 ch = chan_def->chan;
	u8 i, start, end, cent_ch;

	if (chan_def->bw == CHANNEL_WIDTH_160) {
		if (chan_def->band == BAND_ON_6G) {
			start = 1;
			end = 29;
			cent_ch = 15;

			for (i = 0; i < 8; i++){
				if (ch >= (start + (i * 32))
				     && ch <= (end + (i * 32))) {
					     ch = (cent_ch + (i * 32));
				}
			}
		} else {
			if (ch % 4 == 0) {
				if (ch >= 36 && ch <= 64)
					ch = 50;
				else if (ch >= 100 && ch <= 128)
					ch = 114;
			} else if (ch % 4 == 1) {
				if (ch >= 149 && ch <= 177)
					ch = 163;
			}
		}
	} else if (chan_def->bw == CHANNEL_WIDTH_80) {
		if (chan_def->band == BAND_ON_6G) {
			start = 1;
			end = 13;
			cent_ch = 7;

			for (i = 0; i < 16; i++){
				if (ch >= (start + (i * 16))
				     && ch <= (end + (i * 16))) {
					     ch = (cent_ch + (i * 16));
				}
			}
		} else {
			if (ch <= 14)
				ch = 7; /* special case for 2.4G */
			else if (ch % 4 == 0) {
				if (ch >= 36 && ch <= 48)
					ch = 42;
				else if (ch >= 52 && ch <= 64)
					ch = 58;
				else if (ch >= 100 && ch <= 112)
					ch = 106;
				else if (ch >= 116 && ch <= 128)
					ch = 122;
				else if (ch >= 132 && ch <= 144)
					ch = 138;
			} else if (ch % 4 == 1) {
				if (ch >= 149 && ch <= 161)
					ch = 155;
				else if (ch >= 165 && ch <= 177)
					ch = 171;
			}
		}
	} else if (chan_def->bw == CHANNEL_WIDTH_40) {
		if (chan_def->offset == CHAN_OFFSET_UPPER)
			ch += 2;
		else if (chan_def->offset == CHAN_OFFSET_LOWER)
			ch -= 2;

	} else if (chan_def->bw == CHANNEL_WIDTH_20
		|| chan_def->bw == CHANNEL_WIDTH_10
		|| chan_def->bw == CHANNEL_WIDTH_5) {
		; /* the same as ch */
	}
	else {
		PHL_ERR("%s failed\n", __func__);
	}
	return ch;
}

/*
 * Refer to 80211 spec Annex E Table E-4 Global operating classes
 * Handle 2.4G/5G/6G Bandwidth 20/40/80/160
 * 80+ not support.
 */
u8
rtw_phl_get_operating_class(
	struct rtw_chan_def chan_def
)
{
	u8 operating_class = 0;

	if(chan_def.bw == CHANNEL_WIDTH_20){
		if(chan_def.band == BAND_ON_6G)
			operating_class = 131;
		else if(chan_def.chan <= 13)
			operating_class = 81;
		else if(chan_def.chan ==14)
			operating_class = 82;
		else if(chan_def.chan >= 36 && chan_def.chan <= 48)
			operating_class = 115;
		else if(chan_def.chan >= 52 && chan_def.chan <= 64)
			operating_class = 118;
		else if(chan_def.chan >= 100 && chan_def.chan <= 144)
			operating_class = 121;
		else if(chan_def.chan >= 149 && chan_def.chan <= 177)
			operating_class = 125;
		else
			PHL_WARN("%s: Undefined channel (%d)\n", __FUNCTION__, chan_def.chan);
	} else if(chan_def.bw == CHANNEL_WIDTH_40) {
		if(chan_def.band == BAND_ON_6G)
			operating_class = 132;
		else if(chan_def.offset == CHAN_OFFSET_UPPER) {
			if(chan_def.chan >= 1 && chan_def.chan <= 9)
				operating_class = 83;
			else if(chan_def.chan == 36 || chan_def.chan == 44)
				operating_class = 116;
			else if(chan_def.chan == 52 || chan_def.chan == 60)
				operating_class = 119;
			else if(chan_def.chan == 100 || chan_def.chan == 108 ||
				chan_def.chan == 116 || chan_def.chan == 124 ||
				chan_def.chan == 132 || chan_def.chan == 140)
				operating_class = 122;
			else if(chan_def.chan == 149 || chan_def.chan == 157 ||
				chan_def.chan == 165 || chan_def.chan == 173)
				operating_class = 126;
			else
				PHL_WARN("%s: Undefined channel (%d)\n", __FUNCTION__, chan_def.chan);
		} else if(chan_def.offset == CHAN_OFFSET_LOWER) {
			if(chan_def.chan >= 5 && chan_def.chan <= 13)
				operating_class = 84;
			else if(chan_def.chan == 40 || chan_def.chan == 48)
				operating_class = 117;
			else if(chan_def.chan == 56 || chan_def.chan == 64)
				operating_class = 120;
			else if(chan_def.chan == 104 || chan_def.chan == 112 ||
				chan_def.chan == 120 || chan_def.chan == 128 ||
				chan_def.chan == 136 || chan_def.chan == 144)
				operating_class = 123;
			else if(chan_def.chan == 153 || chan_def.chan == 161 ||
				chan_def.chan == 169 || chan_def.chan == 177)
				operating_class = 127;
			else
				PHL_WARN("%s: Undefined channel (%d)\n", __FUNCTION__, chan_def.chan);
		} else {
			PHL_WARN("%s: Invalid offset(%d)\n",
				 __FUNCTION__, chan_def.offset);
		}
	} else if(chan_def.bw == CHANNEL_WIDTH_80) {
		if(chan_def.band == BAND_ON_5G)
			operating_class = 128;
		else if(chan_def.band == BAND_ON_6G)
			operating_class = 133;
		else
			PHL_WARN("%s: Undefined channel (%d)\n", __FUNCTION__, chan_def.center_ch);
	} else if(chan_def.bw == CHANNEL_WIDTH_160) {
		if(chan_def.band == BAND_ON_5G)
			operating_class = 129;
		else if(chan_def.band == BAND_ON_6G)
			operating_class = 134;
		else
			PHL_WARN("%s: Undefined channel (%d)\n", __FUNCTION__, chan_def.center_ch);
	} else {
		PHL_WARN("%s: Not handle bandwidth (%d)\n", __FUNCTION__, chan_def.bw);
	}

	return operating_class;
}

bool
rtw_phl_get_chandef_from_operating_class(
	u8 channel,
	u8 operating_class,
	struct rtw_chan_def *chan_def
)
{
	bool ret = true;

	if(operating_class == 81 || operating_class == 82) {
		/* 2.4G 20MHz */
		chan_def->chan = channel;
		chan_def->bw = CHANNEL_WIDTH_20;
		chan_def->offset = CHAN_OFFSET_NO_EXT;
		chan_def->band = BAND_ON_24G;
		chan_def->center_ch = rtw_phl_get_center_ch(chan_def);
	} else if(operating_class == 115 || operating_class == 118 ||
		  operating_class == 121 || operating_class == 124 ||
		  operating_class == 125) {
		/* 5G 20MHz */
		chan_def->chan = channel;
		chan_def->bw = CHANNEL_WIDTH_20;
		chan_def->offset = CHAN_OFFSET_NO_EXT;
		chan_def->band = BAND_ON_5G;
		chan_def->center_ch = rtw_phl_get_center_ch(chan_def);
	} else if(operating_class == 83) {
		/* 2.4G 40MHz Upper */
		chan_def->chan = channel;
		chan_def->bw = CHANNEL_WIDTH_40;
		chan_def->offset = CHAN_OFFSET_UPPER;
		chan_def->band = BAND_ON_24G;
		chan_def->center_ch = rtw_phl_get_center_ch(chan_def);
	} else if(operating_class == 116 || operating_class == 119 ||
		  operating_class == 122 || operating_class == 126) {
		/* 5G 40MHz Upper */
		chan_def->chan = channel;
		chan_def->bw = CHANNEL_WIDTH_40;
		chan_def->offset = CHAN_OFFSET_UPPER;
		chan_def->band = BAND_ON_5G;
		chan_def->center_ch = rtw_phl_get_center_ch(chan_def);
	} else if(operating_class == 84) {
		/* 2.4G 40MHz Lower */
		chan_def->chan = channel;
		chan_def->bw = CHANNEL_WIDTH_40;
		chan_def->offset = CHAN_OFFSET_LOWER;
		chan_def->band = BAND_ON_24G;
		chan_def->center_ch = rtw_phl_get_center_ch(chan_def);
	} else if(operating_class == 117 || operating_class == 120 ||
		  operating_class == 123 || operating_class == 127) {
		/* 5G 40MHz Lower */
		chan_def->chan = channel;
		chan_def->bw = CHANNEL_WIDTH_40;
		chan_def->offset = CHAN_OFFSET_LOWER;
		chan_def->band = BAND_ON_5G;
		chan_def->center_ch = rtw_phl_get_center_ch(chan_def);
	} else if(operating_class == 128){
		/* 5G 80MHz */
		chan_def->chan = channel;
		chan_def->bw = CHANNEL_WIDTH_80;
		chan_def->offset = CHAN_OFFSET_NO_DEF;
		chan_def->band = BAND_ON_5G;
		chan_def->center_ch = rtw_phl_get_center_ch(chan_def);
	} else if(operating_class == 129){
		/* 5G 160MHz */
		chan_def->chan = channel;
		chan_def->bw = CHANNEL_WIDTH_160;
		chan_def->offset = CHAN_OFFSET_NO_DEF;
		chan_def->band = BAND_ON_5G;
		chan_def->center_ch = rtw_phl_get_center_ch(chan_def);
	} else if(operating_class == 131) {
		/* 6G 20MHz */
		chan_def->chan = channel;
		chan_def->bw = CHANNEL_WIDTH_20;
		chan_def->offset = CHAN_OFFSET_NO_DEF;
		chan_def->center_ch = channel;
		chan_def->band = BAND_ON_6G;
	} else if(operating_class == 132) {
		/* 6G 40MHz */
		chan_def->chan = channel;
		chan_def->bw = CHANNEL_WIDTH_40;
		chan_def->offset = CHAN_OFFSET_NO_DEF;
		chan_def->center_ch = channel;
		chan_def->band = BAND_ON_6G;
	} else if(operating_class == 133 || operating_class == 135) {
		/* 6G 80MHz */
		chan_def->chan = channel;
		chan_def->bw = CHANNEL_WIDTH_80;
		chan_def->offset = CHAN_OFFSET_NO_DEF;
		chan_def->center_ch = channel;
		chan_def->band = BAND_ON_6G;
	} else if(operating_class == 134) {
		/* 6G 160MHz */
		chan_def->chan = channel;
		chan_def->bw = CHANNEL_WIDTH_160;
		chan_def->offset = CHAN_OFFSET_NO_DEF;
		chan_def->center_ch = channel;
		chan_def->band = BAND_ON_6G;
	} else {
		PHL_ERR("%s: Unknown operating class (%d)\n", __FUNCTION__, operating_class);
		ret = false;
	}

	return ret;
}
