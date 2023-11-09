#ifndef __RTK_8168_DEF_H__
#define __RTK_8168_DEF_H__
#include <rbus/sw_def_reg.h>

#define RTL_NETIF_FEATURES    (NETIF_F_RXCSUM | NETIF_F_HW_VLAN_CTAG_TX | NETIF_F_HW_VLAN_CTAG_RX | NETIF_F_TSO | NETIF_F_SG |NETIF_F_IP_CSUM)
#define RTL_TP_FEATURE            4
#define RTL_USE_NEW_CRC_WAKE_UP_METHOD

#endif