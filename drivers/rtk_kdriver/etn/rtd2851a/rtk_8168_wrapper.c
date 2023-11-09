
static void rtl8168_phy_int_disable(struct rtl8168_private *tp)
{
        u32 data;

        data = r8168_mac_ocp_read(tp, 0xfc1e);
        data &= ~(BIT(1) | BIT(11) | BIT(12));
        r8168_mac_ocp_write(tp, 0xfc1e, data);
}

static void rtl8168_phy_int_enable(struct rtl8168_private *tp)
{
        u32 data;

        data = r8168_mac_ocp_read(tp, 0xfc1e);
        data |= (BIT(1) | BIT(11) | BIT(12));
        r8168_mac_ocp_write(tp, 0xfc1e, data);
}

#ifdef ENABLE_EEEP
/*
 * Mac EEEPlus patch from CN
 */
static void rtl8168_mac_eeep_patch_disable(struct rtl8168_private *tp)
{
        u32 ioaddr = tp->base_addr;

        RTL_W32(OCPDR,0xFE140000);
        RTL_W32(OCPDR,0xFE150000);
        RTL_W32(OCPDR,0xFE160000);
        RTL_W32(OCPDR,0xFE170000);
        RTL_W32(OCPDR,0xFE180000);
        RTL_W32(OCPDR,0xFE190000);
        RTL_W32(OCPDR,0xFE1A0000);
        RTL_W32(OCPDR,0xFE1B0000);
        mdelay(3);
        RTL_W32(OCPDR,0xFE130000);
}

static void rtl8168_mac_eeep_enable(struct rtl8168_private *tp)
{
        /* EEEPlus switch (ERI channel 0x1D0 bit[1]=1) */
        rtl_w1w0_eri(tp, 0x1d0, ERIAR_MASK_0001, 0x02, 0x00, ERIAR_EXGMAC);
        //eth_err("ERI value = %08x\n", RTL_R32(ERIDR));

        /* EEE on */
        rtl_writephy(tp, 0x1f, 0x0a40);
        rtl_writephy(tp, 0x09, 0x0000);

        rtl_writephy(tp, 0x1f, 0x0a43);
        rtl_writephy(tp, 0x19, 0x0073);
        rtl_writephy(tp, 0x1f, 0x0a5d);
        rtl_writephy(tp, 0x10, 0x0006);

        rtl_writephy(tp, 0x1f, 0x0a44);
        rtl_writephy(tp, 0x11, 0x0cbc);
        rtl_writephy(tp, 0x1f, 0x0a43);
        rtl_writephy(tp, 0x10, 0x2199);
}

static void rtl8168_mac_eeep_patch(struct rtl8168_private *tp)
{
        u32 ioaddr = tp->base_addr;

        RTL_W32(OCPDR,0xFC00E008);
        RTL_W32(OCPDR,0xFC01E012);
        RTL_W32(OCPDR,0xFC02E044);
        RTL_W32(OCPDR,0xFC03E046);
        RTL_W32(OCPDR,0xFC04E048);
        RTL_W32(OCPDR,0xFC05E04A);
        RTL_W32(OCPDR,0xFC06E04C);
        RTL_W32(OCPDR,0xFC07E04E);
        RTL_W32(OCPDR,0xFC0844E3);
        RTL_W32(OCPDR,0xFC09C708);
        RTL_W32(OCPDR,0xFC0A75E0);
        RTL_W32(OCPDR,0xFC0B485D);
        RTL_W32(OCPDR,0xFC0C9DE0);
        RTL_W32(OCPDR,0xFC0DC705);
        RTL_W32(OCPDR,0xFC0EC502);
        RTL_W32(OCPDR,0xFC0FBD00);
        RTL_W32(OCPDR,0xFC1001A4);
        RTL_W32(OCPDR,0xFC11E85A);
        RTL_W32(OCPDR,0xFC12E000);
        RTL_W32(OCPDR,0xFC13C72D);
        RTL_W32(OCPDR,0xFC1476E0);
        RTL_W32(OCPDR,0xFC1549ED);
        RTL_W32(OCPDR,0xFC16F026);
        RTL_W32(OCPDR,0xFC17C02A);
        RTL_W32(OCPDR,0xFC187400);
        RTL_W32(OCPDR,0xFC19C526);
        RTL_W32(OCPDR,0xFC1AC228);
        RTL_W32(OCPDR,0xFC1B9AA0);
        RTL_W32(OCPDR,0xFC1C73A2);
        RTL_W32(OCPDR,0xFC1D49BE);
        RTL_W32(OCPDR,0xFC1EF11E);
        RTL_W32(OCPDR,0xFC1FC324);
        RTL_W32(OCPDR,0xFC209BA2);
        RTL_W32(OCPDR,0xFC2173A2);
        RTL_W32(OCPDR,0xFC2249BE);
        RTL_W32(OCPDR,0xFC23F0FE);
        RTL_W32(OCPDR,0xFC2473A2);
        RTL_W32(OCPDR,0xFC2549BE);
        RTL_W32(OCPDR,0xFC26F1FE);
        RTL_W32(OCPDR,0xFC271A02);
        RTL_W32(OCPDR,0xFC2849C9);
        RTL_W32(OCPDR,0xFC29F003);
        RTL_W32(OCPDR,0xFC2A4821);
        RTL_W32(OCPDR,0xFC2BE002);
        RTL_W32(OCPDR,0xFC2C48A1);
        RTL_W32(OCPDR,0xFC2D73A2);
        RTL_W32(OCPDR,0xFC2E49BE);
        RTL_W32(OCPDR,0xFC2FF10D);
        RTL_W32(OCPDR,0xFC30C313);
        RTL_W32(OCPDR,0xFC319AA0);
        RTL_W32(OCPDR,0xFC32C312);
        RTL_W32(OCPDR,0xFC339BA2);
        RTL_W32(OCPDR,0xFC3473A2);
        RTL_W32(OCPDR,0xFC3549BE);
        RTL_W32(OCPDR,0xFC36F0FE);
        RTL_W32(OCPDR,0xFC3773A2);
        RTL_W32(OCPDR,0xFC3849BE);
        RTL_W32(OCPDR,0xFC39F1FE);
        RTL_W32(OCPDR,0xFC3A48ED);
        RTL_W32(OCPDR,0xFC3B9EE0);
        RTL_W32(OCPDR,0xFC3CC602);
        RTL_W32(OCPDR,0xFC3DBE00);
        RTL_W32(OCPDR,0xFC3E04E6);
        RTL_W32(OCPDR,0xFC3FDE00);
        RTL_W32(OCPDR,0xFC40E85A);
        RTL_W32(OCPDR,0xFC41E086);
        RTL_W32(OCPDR,0xFC420A44);
        RTL_W32(OCPDR,0xFC43801F);
        RTL_W32(OCPDR,0xFC448015);
        RTL_W32(OCPDR,0xFC450015);
        RTL_W32(OCPDR,0xFC46C602);
        RTL_W32(OCPDR,0xFC47BE00);
        RTL_W32(OCPDR,0xFC480000);
        RTL_W32(OCPDR,0xFC49C602);
        RTL_W32(OCPDR,0xFC4ABE00);
        RTL_W32(OCPDR,0xFC4B0000);
        RTL_W32(OCPDR,0xFC4CC602);
        RTL_W32(OCPDR,0xFC4DBE00);
        RTL_W32(OCPDR,0xFC4E0000);
        RTL_W32(OCPDR,0xFC4FC602);
        RTL_W32(OCPDR,0xFC50BE00);
        RTL_W32(OCPDR,0xFC510000);
        RTL_W32(OCPDR,0xFC52C602);
        RTL_W32(OCPDR,0xFC53BE00);
        RTL_W32(OCPDR,0xFC540000);
        RTL_W32(OCPDR,0xFC55C602);
        RTL_W32(OCPDR,0xFC56BE00);
        RTL_W32(OCPDR,0xFC570000);

        RTL_W32(OCPDR,0xFE138000);
        RTL_W32(OCPDR,0xFE1401A3);
        RTL_W32(OCPDR,0xFE1504E5);
}
#endif


static void rtl8168_enable_mac_phy_connect(struct rtl8168_private *tp)
{
}

static void rtl8168_pre_phy_init(struct rtl8168_private *tp)
{
}

static void rtl8168_set_tx_tdfnr(struct rtl8168_private *tp)
{
}