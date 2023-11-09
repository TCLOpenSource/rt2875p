/**
* @file Merlin6 SW DEF
* RBus systemc program.
*
* @author RT_MM2_SD
* @email mm_mm2_rd_sw_kernel_driver@realtek.com
* @ingroup model_rbus
 * @version { 1.0 }
 **
 */

#ifndef _SW_DEF_REG_H_
#define _SW_DEF_REG_H_



//  SW DEF Register Address
#if 1
#define SW_DEF_BASE_ADDR        0xb8016000

#define REG_DDR_OC_CONTROL      0xb80c9028
#define REG_DDR_OC_DONE         0xb80c9028
#define REG_DDR_SSC_CONTROL     0xb80c901c
#define REG_DDR_NCODE_SSC       0xb80c9028
#define REG_DDR_FCODE_SSC       0xb80c9024
#define REG_DDR_GRAN_EST        0xb80c9020

#define REG_DDR2_OC_CONTROL 0xb80ca028
#define REG_DDR2_OC_DONE 0xb80ca028
#define REG_DDR2_SSC_CONTROL 0xb80ca01c
#define REG_DDR2_NCODE_SSC 0xb80ca028
#define REG_DDR2_FCODE_SSC 0xb80ca024
#define REG_DDR2_GRAN_EST 0xb80ca020

#define RTD_SHARE_MEM_BASE      0xb8060500
#define RTD_SHARE_MEM_LEN       32

#define REG_ETN_WOL_STD_REG1    0xb8060500
#define REG_ETN_WOL_STD_REG     0xb806050c
#define REG_DTMBIP_START_ADDR   0xb8170000


/* write/read MMIO register */
#define RTL_W8(reg, val8)       rtd_outb(ioaddr + (reg), (val8))
#define RTL_W16(reg, val16)     rtd_outw(ioaddr + (reg), (val16))
#define RTL_W32(reg, val32)     rtd_outl(ioaddr + (reg), (val32))
#define RTL_R8(reg)             rtd_inb(ioaddr + (reg))
#define RTL_R16(reg)            rtd_inw(ioaddr + (reg))
#define RTL_R32(reg)            rtd_inl(ioaddr + (reg))

enum rtl_tx_desc_version {
	RTL_TD_0	= 0,
	RTL_TD_1	= 1,
};

#define JUMBO_1K	ETH_DATA_LEN
#define JUMBO_4K	(4*1024 - ETH_HLEN - 2)
#define JUMBO_6K	(6*1024 - ETH_HLEN - 2)
#define JUMBO_7K	(7*1024 - ETH_HLEN - 2)
#define JUMBO_9K	(9*1024 - ETH_HLEN - 2)

enum cfg_version {
	RTL_CFG_0 = 0x00,
	RTL_CFG_1,
};

#if 0
static int rx_buf_sz = 16383;
static int use_dac;
static struct {
	u32 msg_enable;
} debug = { -1 };
#endif

enum rtl_registers {
	MAC0		= 0,	/* Ethernet hardware address. */
	MAC4		= 4,
	MAR0		= 8,	/* Multicast filter. */
	CounterAddrLow		= 0x10,
	CounterAddrHigh		= 0x14,
	CustomLED	= 0x18,
	TxDescStartAddrLow	= 0x20,
	TxDescStartAddrHigh	= 0x24,
	TxHDescStartAddrLow	= 0x28,
	TxHDescStartAddrHigh	= 0x2c,
	FLASH		= 0x30,
	ERSR		= 0x36,
	ChipCmd		= 0x37,
	TxPoll		= 0x38,
	IntrMask	= 0x3c,
	IntrStatus	= 0x3e,

	TxConfig	= 0x40,
#define	TXCFG_AUTO_FIFO         (1 << 7)	/* 8111e-vl */
#define	TXCFG_EMPTY             (1 << 11)	/* 8111e-vl */

	RxConfig	= 0x44,
#define	RX128_INT_EN            (1 << 15)	/* 8111c and later */
#define	RX_MULTI_EN             (1 << 14)	/* 8111c only */
#define	RXCFG_FIFO_SHIFT		13
	/* No threshold before first PCI xfer */
#define	RX_FIFO_THRESH          (7 << RXCFG_FIFO_SHIFT)
#define	RX_EARLY_OFF            (1 << 11)
#define	RXCFG_DMA_SHIFT			8
	/* Unlimited maximum PCI burst. */
#define	RX_DMA_BURST            (3 << RXCFG_DMA_SHIFT)

	RxMissed	= 0x4c,
	Cfg9346		= 0x50,
	Config0		= 0x51,
	Config1		= 0x52,
	Config2		= 0x53,
#define PME_SIGNAL              (1 << 5)	/* 8168c and later */

	Config3		= 0x54,
	Config4		= 0x55,
	Config5		= 0x56,
	MultiIntr	= 0x5c,
	PHYAR		= 0x60,
	PHYstatus	= 0x6c,
	RxMaxSize	= 0xda,
	CPlusCmd	= 0xe0,
	IntrMitigate	= 0xe2,
	RxDescAddrLow	= 0xe4,
	RxDescAddrHigh	= 0xe8,
	EarlyTxThres	= 0xec,	/* 8168. Unit of 32 bytes. */

#define NoEarlyTx               0x3f	/* Max value : no early transmit. */

	MaxTxPacketSize	= 0xec,	/* 8101/8168. Unit of 128 bytes. */

#define TxPacketMax             (8064 >> 7) /* 63 = 0x3f means no early transmit */
#define EarlySize               0x27

	FuncEvent	= 0xf0,
	FuncEventMask	= 0xf4,
	FuncPresetState	= 0xf8,
	FuncForceEvent	= 0xfc,
};

enum rtl8110_registers {
	TBICSR			= 0x64,
	TBI_ANAR		= 0x68,
	TBI_LPAR		= 0x6a,
};

enum rtl8168_8101_registers {
	CSIDR			= 0x64,
	CSIAR			= 0x68,
#define	CSIAR_FLAG              0x80000000
#define	CSIAR_WRITE_CMD         0x80000000
#define	CSIAR_BYTE_ENABLE       0x0f
#define	CSIAR_BYTE_ENABLE_SHIFT 12
#define	CSIAR_ADDR_MASK         0x0fff
#define CSIAR_FUNC_CARD         0x00000000
#define CSIAR_FUNC_SDIO         0x00010000
#define CSIAR_FUNC_NIC          0x00020000
	PMCH			= 0x6f,
	EPHYAR			= 0x80,
#define	EPHYAR_FLAG             0x80000000
#define	EPHYAR_WRITE_CMD        0x80000000
#define	EPHYAR_REG_MASK         0x1f
#define	EPHYAR_REG_SHIFT        16
#define	EPHYAR_DATA_MASK        0xffff
	DLLPR			= 0xd0,
#define	PFM_EN                  (1 << 6)
	DBG_REG			= 0xd1,
#define	FIX_NAK_1               (1 << 4)
#define	FIX_NAK_2               (1 << 3)
	TWSI			= 0xd2,
	MCU			= 0xd3,
#define	NOW_IS_OOB              (1 << 7)
#define	TX_EMPTY                (1 << 5)
#define	RX_EMPTY                (1 << 4)
#define	RXTX_EMPTY              (TX_EMPTY | RX_EMPTY)
#define	EN_NDP                  (1 << 3)
#define	EN_OOB_RESET            (1 << 2)
#define	LINK_LIST_RDY           (1 << 1)
#define DIS_MCU_CLROOB          BIT(0)
	EFUSEAR			= 0xdc,
#define	EFUSEAR_FLAG            0x80000000
#define	EFUSEAR_WRITE_CMD       0x80000000
#define	EFUSEAR_READ_CMD        0x00000000
#define	EFUSEAR_REG_MASK        0x03ff
#define	EFUSEAR_REG_SHIFT       8
#define	EFUSEAR_DATA_MASK       0xff
};

enum rtl8168_registers {
	LED_FREQ		= 0x1a,
	EEE_LED			= 0x1b,
	ERIDR			= 0x70,
	ERIAR			= 0x74,
#define ERIAR_FLAG              0x80000000
#define ERIAR_WRITE_CMD         0x80000000
#define ERIAR_READ_CMD          0x00000000
#define ERIAR_ADDR_BYTE_ALIGN   4
#define ERIAR_TYPE_SHIFT        16
#define ERIAR_EXGMAC            (0x00 << ERIAR_TYPE_SHIFT)
#define ERIAR_MSIX              (0x01 << ERIAR_TYPE_SHIFT)
#define ERIAR_ASF               (0x02 << ERIAR_TYPE_SHIFT)
#define ERIAR_MASK_SHIFT        12
#define ERIAR_MASK_0001         (0x1 << ERIAR_MASK_SHIFT)
#define ERIAR_MASK_0011         (0x3 << ERIAR_MASK_SHIFT)
#define ERIAR_MASK_0101         (0x5 << ERIAR_MASK_SHIFT)
#define ERIAR_MASK_1111         (0xf << ERIAR_MASK_SHIFT)
	EPHY_RXER_NUM		= 0x7c,
	OCPDR			= 0xb0,	/* OCP GPHY access */
#define OCPDR_WRITE_CMD         0x80000000
#define OCPDR_READ_CMD          0x00000000
#define OCPDR_REG_MASK          0x7f
#define OCPDR_GPHY_REG_SHIFT    16
#define OCPDR_DATA_MASK         0xffff
	OCPAR			= 0xb4,
#define OCPAR_FLAG              0x80000000
#define OCPAR_GPHY_WRITE_CMD    0x8000f060
#define OCPAR_GPHY_READ_CMD     0x0000f060
	GPHY_OCP		= 0xb8,
	RDSAR1			= 0xd0,	/* 8168c only. Undocumented on 8168dp */
	MISC			= 0xf0,	/* 8168e only. */
#define TXPLA_RST               (1 << 29)
#define DISABLE_LAN_EN          (1 << 23) /* Enable GPIO pin */
#define PWM_EN                  (1 << 22)
#define RXDV_GATED_EN           (1 << 19)
#define EARLY_TALLY_EN          (1 << 16)
};

enum rtl_register_content {
	/* InterruptStatusBits */
	SYSErr		= 0x8000,
	PCSTimeout	= 0x4000,
	SWInt		= 0x0100,
	TxDescUnavail	= 0x0080,
	RxFIFOOver	= 0x0040,
	LinkChg		= 0x0020,
	RxOverflow	= 0x0010,
	TxErr		= 0x0008,
	TxOK		= 0x0004,
	RxErr		= 0x0002,
	RxOK		= 0x0001,

	/* RxStatusDesc */
	RxBOVF	= (1 << 24),
	RxFOVF	= (1 << 23),
	RxRWT	= (1 << 22),
	RxRES	= (1 << 21),
	RxRUNT	= (1 << 20),
	RxCRC	= (1 << 19),

	/* ChipCmdBits */
	StopReq		= 0x80,
	CmdReset	= 0x10,
	CmdRxEnb	= 0x08,
	CmdTxEnb	= 0x04,
	RxBufEmpty	= 0x01,

	/* TXPoll register p.5 */
	HPQ		= 0x80,		/* Poll cmd on the high prio queue */
	NPQ		= 0x40,		/* Poll cmd on the low prio queue */
	FSWInt		= 0x01,		/* Forced software interrupt */

	/* Cfg9346Bits */
	Cfg9346_Lock	= 0x00,
	Cfg9346_Unlock	= 0xc0,

	/* rx_mode_bits */
	AcceptErr	= 0x20,
	AcceptRunt	= 0x10,
	AcceptBroadcast	= 0x08,
	AcceptMulticast	= 0x04,
	AcceptMyPhys	= 0x02,
	AcceptAllPhys	= 0x01,
#define RX_CONFIG_ACCEPT_MASK		0x3f

	/* TxConfigBits */
	TxInterFrameGapShift = 24,
	TxDMAShift = 8,	/* DMA burst value (0-7) is shift this many bits */

	/* Config1 register p.24 */
	LEDS1		= (1 << 7),
	LEDS0		= (1 << 6),
	Speed_down	= (1 << 4),
	MEMMAP		= (1 << 3),
	IOMAP		= (1 << 2),
	VPD		= (1 << 1),
	PMEnable	= (1 << 0),	/* Power Management Enable */

	/* Config2 register p. 25 */
	ClkReqEn	= (1 << 7),	/* Clock Request Enable */
	MSIEnable	= (1 << 5),	/* 8169 only. Reserved in the 8168. */
	PCI_Clock_66MHz = 0x01,
	PCI_Clock_33MHz = 0x00,

	/* Config3 register p.25 */
	MagicPacket	= (1 << 5),	/* Wake up when receives a Magic Packet */
	LinkUp		= (1 << 4),	/* Wake up when the cable connection is re-established */
	Jumbo_En0	= (1 << 2),	/* 8168 only. Reserved in the 8168b */
	Beacon_en	= (1 << 0),	/* 8168 only. Reserved in the 8168b */

	/* Config4 register */
	Jumbo_En1	= (1 << 1),	/* 8168 only. Reserved in the 8168b */

	/* Config5 register p.27 */
	BWF		= (1 << 6),	/* Accept Broadcast wakeup frame */
	MWF		= (1 << 5),	/* Accept Multicast wakeup frame */
	UWF		= (1 << 4),	/* Accept Unicast wakeup frame */
	Spi_en		= (1 << 3),
	LanWake		= (1 << 1),	/* LanWake enable/disable */
	PMEStatus	= (1 << 0),	/* PME status can be reset by PCI RST# */
	ASPM_en		= (1 << 0),	/* ASPM enable */

	/* TBICSR p.28 */
	TBIReset	= 0x80000000,
	TBILoopback	= 0x40000000,
	TBINwEnable	= 0x20000000,
	TBINwRestart	= 0x10000000,
	TBILinkOk	= 0x02000000,
	TBINwComplete	= 0x01000000,

	/* CPlusCmd p.31 */
	EnableBist	= (1 << 15),	/* 8168 8101 */
	Mac_dbgo_oe	= (1 << 14),	/* 8168 8101 */
	Normal_mode	= (1 << 13),	/* unused */
	Force_half_dup	= (1 << 12),	/* 8168 8101 */
	Force_rxflow_en	= (1 << 11),	/* 8168 8101 */
	Force_txflow_en	= (1 << 10),	/* 8168 8101 */
	Cxpl_dbg_sel	= (1 << 9),	/* 8168 8101 */
	ASF		= (1 << 8),	/* 8168 8101 */
	PktCntrDisable	= (1 << 7),	/* 8168 8101 */
	Mac_dbgo_sel	= 0x001c,	/* 8168 */
	RxVlan		= (1 << 6),
	RxChkSum	= (1 << 5),
	PCIDAC		= (1 << 4),
	PCIMulRW	= (1 << 3),
	INTT_0		= 0x0000,	/* 8168 */
	INTT_1		= 0x0001,	/* 8168 */
	INTT_2		= 0x0002,	/* 8168 */
	INTT_3		= 0x0003,	/* 8168 */

	/* rtl8168_PHYstatus */
	PWR_SAVE	= 0x80,
	TxFlowCtrl	= 0x40,
	RxFlowCtrl	= 0x20,
	_1000bpsF	= 0x10,
	_100bps		= 0x08,
	_10bps		= 0x04,
	LinkStatus	= 0x02,
	FullDup		= 0x01,

	/* _TBICSRBit */
	TBILinkOK	= 0x02000000,

	/* DumpCounterCommand */
	CounterDump	= 0x8,
};

enum rtl_desc_bit {
	/* First doubleword. */
	DescOwn		= (1 << 31), /* Descriptor is owned by NIC */
	RingEnd		= (1 << 30), /* End of descriptor ring */
	FirstFrag	= (1 << 29), /* First segment of a packet */
	LastFrag	= (1 << 28), /* Final segment of a packet */
};

/* Generic case. */
enum rtl_tx_desc_bit {
	/* First doubleword. */
	TD_LSO		= (1 << 27),		/* Large Send Offload */
#define TD_MSS_MAX              0x07ffu	/* MSS value */

	/* Second doubleword. */
	TxVlanTag	= (1 << 17),		/* Add VLAN tag */
};

/* 8102e, 8168c and beyond. */
enum rtl_tx_desc_bit_1 {
	/* Second doubleword. */
#define TD1_MSS_SHIFT           18	/* MSS position (11 bits) */
	TD1_IP_CS	= (1 << 29),		/* Calculate IP checksum */
	TD1_TCP_CS	= (1 << 30),		/* Calculate TCP/IP checksum */
	TD1_UDP_CS	= (1 << 31),		/* Calculate UDP/IP checksum */
};

static const struct rtl_tx_desc_info {
	struct {
		u32 udp;
		u32 tcp;
	} checksum;
	u16 mss_shift;
	u16 opts_offset;
} tx_desc_info = {
	.checksum = {
		.udp	= TD1_IP_CS | TD1_UDP_CS,
		.tcp	= TD1_IP_CS | TD1_TCP_CS
	},
	.mss_shift	= TD1_MSS_SHIFT,
	.opts_offset	= 1
};

enum rtl_rx_desc_bit {
	/* Rx private */
	PID1		= (1 << 18), /* Protocol ID bit 1/2 */
	PID0		= (1 << 17), /* Protocol ID bit 2/2 */

#define RxProtoUDP              (PID1)
#define RxProtoTCP              (PID0)
#define RxProtoIP               (PID1 | PID0)
#define RxProtoMask             RxProtoIP

	IPFail		= (1 << 16), /* IP checksum failed */
	UDPFail		= (1 << 15), /* UDP/IP checksum failed */
	TCPFail		= (1 << 14), /* TCP/IP checksum failed */
	RxVlanTag	= (1 << 16), /* VLAN tag available */
};

#define RsvdMask                0x3fffc000
#endif


#endif 
