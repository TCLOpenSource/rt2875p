/******************************************************************************
 *
 * Copyright(c) 2019 - 2021 Realtek Corporation.
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
#ifndef _RTW_TRX_H_
#define _RTW_TRX_H_

struct dvobj_priv;
struct rtw_intf_ops {
	/*** xmit section ***/
	s32(*init_xmit_priv)(_adapter *adapter);
	void(*free_xmit_priv)(_adapter *adapter);

	s32(*data_xmit)(_adapter *adapter, struct xmit_frame *pxmitframe);
	s32(*xmitframe_enqueue)(_adapter *adapter, struct xmit_frame *pxmitframe);

	/*** recv section ***/
	s32(*init_recv_priv)(struct dvobj_priv *dvobj);
	void(*free_recv_priv)(struct dvobj_priv *dvobj);

	#if defined(CONFIG_PCI_HCI)
	u32(*trxbd_init)(_adapter *adapter);
	u32(*trxbd_deinit)(_adapter *adapter);
	void(*trxbd_reset)(_adapter *adapter);
	s32(*interrupt_handler)(_adapter *adapter);
	#endif

	#if defined(CONFIG_USB_HCI)
	#ifdef CONFIG_SUPPORT_USB_INT
	void(*interrupt_handler)(_adapter *adapter, u16 pkt_len, u8 *pbuf);
	#endif
	#endif

	#ifdef CONFIG_HOSTAPD_MLME
	s32(*hostap_mgnt_xmit_entry)(_adapter *adapter, struct sk_buff *pkt);
	#endif
};

s32 rtw_mgnt_xmit(_adapter *adapter, struct xmit_frame *pmgntframe);


#endif /* _RTW_TRX_H_ */
