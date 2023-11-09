
static void rtl8168_phy_int_disable(struct rtl8168_private *tp)
{
        u32 data = 0;
        data = r8168_mac_ocp_read(tp, 0xE404);
        data = (data & ~BIT(11)) | BIT(12);
        r8168_mac_ocp_write(tp, 0xE404, data);

        data = r8168_mac_ocp_read(tp, 0xFC1E);
        data &= ~BIT(1);
        r8168_mac_ocp_write(tp, 0xFC1E, data);
}

static void rtl8168_phy_int_enable(struct rtl8168_private *tp)
{
        u32 data = 0;
        data = r8168_mac_ocp_read(tp, 0xE404);
        data |= BIT(12)|BIT(11);
        r8168_mac_ocp_write(tp, 0xE404, data);

        data = r8168_mac_ocp_read(tp, 0xFC1E);
        data |= BIT(1);
        r8168_mac_ocp_write(tp, 0xFC1E, data);
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
/**
* EEEP flow setting from HWSD jerome_zou@realtek.com
*/
static void rtl8168_mac_eeep_enable(struct rtl8168_private *tp)
{
        /* EEEPlus switch (ERI channel 0x1D0 bit[1]=1) */
        rtl_w1w0_eri(tp, 0x1d0, ERIAR_MASK_0001, 0x02, 0x00, ERIAR_EXGMAC);
        //eth_err("ERI value = %08x\n", RTL_R32(ERIDR));

        /* EEE on */
        rtl_writephy(tp, 0x1f, 0x0a43);
        rtl_writephy(tp, 0x19, 0x0013);// eee10_en, bit[4]=1
        rtl_writephy(tp, 0x1f, 0x0a5d);
        rtl_writephy(tp, 0x10, 0x0006);// my100_eee, bit[1]=1, Enable EEE Nway ability

        rtl_writephy(tp, 0x1f, 0x0a44);
        rtl_writephy(tp, 0x11, 0x0480);// en_10m_bgoff, bit[7]=1; eee_dly1s_en, bit[10]=1, def=0x400 -->0x480
        rtl_writephy(tp, 0x1f, 0x0a43);
        rtl_writephy(tp, 0x18, 0x2199);// enable 10M pll off bit[0]=1,def=0x2198 -->0x2199
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
	r8168_mac_ocp_write(tp,0xEA34,0x0002);
}


static void rtl8168_swing(struct rtl8168_private *tp)
{
	unsigned int count = 0;
	unsigned int val = 0;
	// for common mode voltage
	rtl8168_xmii_set_page(tp, 0x0bc0);// change page bc0
	// write reg.17 bit[11:4]=0xbc 
	rtl_writephy(tp, 0x11, 0x0bc0);
	rtl_writephy(tp, 0x17, 0x2900);//  write reg.23 bit[10:9]=0x0,LD_COMP; bit[13:12]=0x2,LD_CMFB=2'b10
	rtl_writephy(tp, 0x13, 0xa1c0);//  write reg.19 bit[15:13],LDVDC=3'b101=0x5

	rtl8168_xmii_set_page(tp, 0x0a46);// change page a46
	rtl_writephy(tp, 0x15, 0x0302);// write reg.21 bit[1]=1 

        
	rtl8168_xmii_set_page(tp, 0x0a60);// change page a60
	// read reg.16 bit[7:0]=0x1
	while(1) {
		val = rtl_readphy(tp, 0x10);
		if((val & 0xFF) == 0x1)
			break;
		udelay(1);
		count++;
		if(count > 10000)
			goto EXIT;
	}

	//cal _itune_giga_L
	rtl8168_xmii_set_page(tp, 0x0bcb);//change page bcb
	//write reg.16[3:2]=0x1
	rtl_writephy(tp, 0x10, 0x4004);

	rtl8168_xmii_set_page(tp, 0x0a43);// change page a43
	// Change green table default LDVBIAS to 0x66
	rtl_writephy(tp, 0x1b, 0x8049);// write reg.27 bit[15:0]=0x8049 
	rtl_writephy(tp, 0x1C, 0x66d7);

	// Change green table 10M LDVBIAS to 0x66
	rtl_writephy(tp, 0x1b, 0x8050);// write reg.27 bit[15:0]=0x8050 
	rtl_writephy(tp, 0x1C, 0x6617);

	// Change green table 100M short LDVBIAS to 0x66
	rtl_writephy(tp, 0x1b, 0x8057);// write reg.27 bit[15:0]=0x8057 
	rtl_writephy(tp, 0x1C, 0x66d7);

	// Change green table 100M long LDVBIAS to 0x66
	rtl_writephy(tp, 0x1b, 0x805e);// write reg.27 bit[15:0]=0x805e
	rtl_writephy(tp, 0x1C, 0x66d7);

	// Change green table GIGA short LDVBIAS to 0x66
	rtl_writephy(tp, 0x1b, 0x8065);// write reg.27 bit[15:0]=0x8065
	rtl_writephy(tp, 0x1C, 0x66d7);

	// Change green table GIGA middle LDVBIAS to 0x66
	rtl_writephy(tp, 0x1b, 0x806c);// write reg.27 bit[15:0]=0x806c
	rtl_writephy(tp, 0x1C, 0x66d7);

	// Change green table GIGA long LDVBIAS to 0x66
	rtl_writephy(tp, 0x1b, 0x8073);// write reg.27 bit[15:0]=0x8073
	rtl_writephy(tp, 0x1C, 0x66d7);

EXIT:
	// release lock main
	rtl8168_xmii_set_page(tp, 0x0a46);// change page a46
	// write reg.21 bit[1]=0
	rtl_writephy(tp, 0x15, 0x0300);

	rtl8168_xmii_set_page(tp, 0x0000);
        
}
#if IS_ENABLED(CONFIG_RTK_KDRV_OTP)
static void rtl8168_write_otp(struct rtl8168_private *tp)
{
	unsigned int done = 0;
	unsigned int R_CAL_otp  = 0;
	unsigned int R_CAL  = 0;
	unsigned int RC_CAL_otp  = 0;
	unsigned int RC_CAL  = 0;
	unsigned int AMP_CAL_otp  = 0;
	unsigned int AMP_CAL  = 0;


	// hw_eth_r_cal
	if (rtk_otp_field_read_by_name("hw_eth_r_cal", (unsigned char *)&R_CAL_otp, sizeof(R_CAL_otp))) {
		if (rtk_otp_field_read_by_name("hw_eth_r_cal_done", (unsigned char *)&done, sizeof(done))) {
                		if(done) {
                			R_CAL = R_CAL_otp + (R_CAL_otp << 8);
                			//1. tapbin_p0: page.BCF reg19[3:0]
                			 //2. tapbin_p1: page.BCF reg19[11:8]
                			rtl8168_xmii_set_page(tp, 0x0bcf);
                			rtl_writephy(tp, 0x13, R_CAL);
                                        
                			//3. tapbin_p2: page.BCF reg18[3:0]
                			//4. tapbin_p3: page.BCF reg18[11:8]
                			rtl_writephy(tp, 0x12, R_CAL);

                			//5. tapbin_pm_p0: page.BCF reg21[3:0]
                			//6. tapbin_pm_p1: page.BCF reg21[11:8]
                			rtl_writephy(tp, 0x15, R_CAL);

                			//7. tapbin_pm_p2: page.BCF reg20[3:0]
                			//8. tapbin_pm_p3: page.BCF reg20[11:8]|
                			rtl_writephy(tp, 0x14, R_CAL);
                			rtl8168_xmii_set_page(tp, 0x0000);
                		}
		}
	}

	// hw_eth_rc_cal
	if (rtk_otp_field_read_by_name("hw_eth_rc_cal", (unsigned char *)&RC_CAL_otp, sizeof(RC_CAL_otp))) {
		if (rtk_otp_field_read_by_name("hw_eth_rc_cal_done", (unsigned char *)&done, sizeof(done))) {
                		if(done) {
                			RC_CAL = RC_CAL_otp;
                			//page.BCD reg22[15:0]
                			rtl8168_xmii_set_page(tp, 0x0bcd);
                			rtl_writephy(tp, 0x16, RC_CAL);
                			//mdelay(10);
                			//page.BCD reg23[15:0]
                			rtl_writephy(tp, 0x17, RC_CAL);
                			//mdelay(10);
                			rtl8168_xmii_set_page(tp, 0x0000);
                		}
		}
	}

	// hw_eth_amp_cal
	if (rtk_otp_field_read_by_name("hw_eth_amp_cal", (unsigned char *)&AMP_CAL_otp, sizeof(AMP_CAL_otp))) {
		if (rtk_otp_field_read_by_name("hw_eth_amp_cal_done", (unsigned char *)&done, sizeof(done))) {
			if(done) {
				AMP_CAL = AMP_CAL_otp;
				//page.BCA reg22[15:0]|
				rtl8168_xmii_set_page(tp, 0x0bca);
				rtl_writephy(tp, 0x16, AMP_CAL);
				//mdelay(10);
				rtl8168_xmii_set_page(tp, 0x0000);
			}
		}
	}
}
#endif

static void rtl8168_pre_phy_init(struct rtl8168_private *tp)
{
	rtl8168_phy_int_disable(tp);
	rtl8168_swing(tp);
#if IS_ENABLED(CONFIG_RTK_KDRV_OTP)
	rtl8168_write_otp(tp);
#endif
	rtl8168_phy_int_enable(tp);  	
}


static void rtl8168_set_tx_tdfnr(struct rtl8168_private *tp)
{
	u32 ioaddr = tp->base_addr;
	RTL_W8(TDFNR, 0x4);
}
