#ifndef  RTK_DC_MT_DBUS_ID_H
#define  RTK_DC_MT_DBUS_ID_H


#define MODULE_STR(a) {MODULE_##a, #a}

static const MODULE_INFO module_info[] =
{

 MODULE_STR(TVSB2_DIW),
 MODULE_STR(TVSB2_DIR),
 MODULE_STR(TVSB2_M_CAP),
 MODULE_STR(TVSB2_M_DISP),
 MODULE_STR(TVSB2_S_CAP),
 MODULE_STR(TVSB2_S_DISP),
 MODULE_STR(TVSB2_VO1_Y),
 MODULE_STR(TVSB2_VO1_C),
 MODULE_STR(TVSB2_VO2_Y),
 MODULE_STR(TVSB2_VO2_C),
 MODULE_STR(TVSB2_I3DDMA_Y),
 MODULE_STR(TVSB2_DEXC_W),
 MODULE_STR(TVSB2_DEXC_R),
 MODULE_STR(TVSB2_SNR_R),
 MODULE_STR(TVSB2),



 MODULE_STR(TVSB1_VD),
 MODULE_STR(TVSB1_AUDIO),
 MODULE_STR(TVSB1_AUDIO_2),
 MODULE_STR(TVSB1_VBI),
 MODULE_STR(TVSB1),

 MODULE_STR(SB1_MD_KC),
 MODULE_STR(SB1_USB),
 MODULE_STR(SB1_RBUSDMA),
// MODULE_STR(SB1_TP2_KC),
 MODULE_STR(SB1_ETN),
 MODULE_STR(SB1_UART_DMA),
 MODULE_STR(SB1_EMMC_KC),
 MODULE_STR(SB1_MD_NKC_NVC),
 MODULE_STR(SB1_MD_VC),
 MODULE_STR(SB1_USB_OTG),
// MODULE_STR(SB1_TP),
 MODULE_STR(SB1_EMMC_NKC),
 MODULE_STR(SB1),


 MODULE_STR(SB2_VCPU),
 MODULE_STR(SB2_LZMA),
 MODULE_STR(SB2_UART),
 MODULE_STR(SB2_HOST_IF),
 MODULE_STR(SB2_VCPU2),
 MODULE_STR(SB2),



 MODULE_STR(SB1_SSB1_TP2_KC),
 MODULE_STR(SB1_SSB1_TP_KC),
 MODULE_STR(SB1_SSB1_TP2_NKC),
 MODULE_STR(SB1_SSB1_TP_NKC),
 MODULE_STR(SB1_SSB1),



 MODULE_STR(VE_VM1),
 MODULE_STR(VE_VM2),
 MODULE_STR(VE_PRD),
 MODULE_STR(VE_DBK),
 MODULE_STR(VE_MVD),
 MODULE_STR(VE_NDBP2),
 MODULE_STR(VE_R1SEQ),
 MODULE_STR(VE_BS),
 MODULE_STR(VE_R1BLK),
 MODULE_STR(VE_VEP2COM),
 MODULE_STR(VE_VECOM),
 MODULE_STR(VE_CABP2),
 MODULE_STR(VE_BSP2),
 MODULE_STR(VE_LR_SAO),
 MODULE_STR(VE_SEG),
 MODULE_STR(VE),


 MODULE_STR(TVSB3_SEQ0),
 MODULE_STR(TVSB3_SEQ1),
 MODULE_STR(TVSB3_CP_KC),
 MODULE_STR(TVSB3_CP_NKC),
 MODULE_STR(TVSB3_DDR_TR_GEN),
 MODULE_STR(TVSB3),

 MODULE_STR(GPU_NKC),
 MODULE_STR(GPU_KC),
 MODULE_STR(GPU),


 MODULE_STR(TVSB5_DEMOD),
 MODULE_STR(TVSB5_DEBUGDMA),
 MODULE_STR(TVSB5_DEMOD2),
 MODULE_STR(TVSB5_USB3),
 MODULE_STR(TVSB5_DDR_TR_GEN),
 MODULE_STR(TVSB5_PCIE),
 MODULE_STR(TVSB5),

 MODULE_STR(NN_NKC),
 MODULE_STR(NN_KC),
 MODULE_STR(NN),


 MODULE_STR(VE2_VM1),
 MODULE_STR(VE2_VM2),
 MODULE_STR(VE2_DBK),
 MODULE_STR(VE2_BS),
 MODULE_STR(VE2_IME),
 MODULE_STR(VE2_NDB),
 MODULE_STR(VE2_FME),

 MODULE_STR(TVSB4_ODW),
 MODULE_STR(TVSB4_ODR),
 MODULE_STR(TVSB4_DC2H),
 MODULE_STR(TVSB4_DDR_TR_GEN),
 MODULE_STR(TVSB4_OSD1),
 MODULE_STR(TVSB4_OSD2),
 MODULE_STR(TVSB4_OSD3),
 MODULE_STR(TVSB4_DEMURA),
 MODULE_STR(TVSB4_DMATO3DLUT),
 MODULE_STR(TVSB4_SRNN_SSB_R),
 MODULE_STR(TVSB4_D_PQMASK_D),
 MODULE_STR(TVSB4_RTK_LD_R),
 MODULE_STR(TVSB4_SLD_W),
 MODULE_STR(TVSB4),

 MODULE_STR(MEMC_ME_W),
 MODULE_STR(MEMC_ME_R),
 MODULE_STR(MEMC_MC_LF_W),
 MODULE_STR(MEMC_MC_HF_W),
 MODULE_STR(MEMC_MC_LFI_R),
 MODULE_STR(MEMC_MC_HFI_R),
 MODULE_STR(MEMC_MC_LFP_R),
 MODULE_STR(MEMC_MC_HFP_R),
 MODULE_STR(MEMC_ME_SHR_W),
 MODULE_STR(MEMC_ME_SHR_R),
 MODULE_STR(MEMC_MVINFO_W),
 MODULE_STR(MEMC_MVINFO_R),
 MODULE_STR(MEMC),


 MODULE_STR(TVSB7_ME),
 MODULE_STR(TVSB7_VDE),
 MODULE_STR(TVSB7_MF2),
 MODULE_STR(TVSB7_CDF_ALF),
 MODULE_STR(TVSB7_MF1),
 MODULE_STR(TVSB7_MNHP2),
 MODULE_STR(TVSB7_CMP),
 MODULE_STR(TVSB7_MF0),
 MODULE_STR(TVSB7_VDE2),
 MODULE_STR(TVSB7_DMVR),
 MODULE_STR(TVSB7_DDR_TR_GEN),
 MODULE_STR(TVSB7),



 MODULE_STR(SCPU_NS),
 MODULE_STR(SCPU_S),
 MODULE_STR(ACPU_NS),
 MODULE_STR(ACPU_S),

 MODULE_STR(ANY),


};







unsigned char key_table_audio[] = {MODULE_TVSB1_AUDIO,MODULE_TVSB1_AUDIO_2};
unsigned char key_table_se[] = {MODULE_TVSB3_SEQ0,MODULE_TVSB3_SEQ1};
//unsigned char key_table_ve[] = {MODULE_VE};
unsigned char key_table_dispi[] = {MODULE_TVSB2_DIW,MODULE_TVSB2_DIR,MODULE_TVSB2_VO1_Y,MODULE_TVSB2_VO1_C,
                                                MODULE_TVSB2_I3DDMA_Y,
                                                MODULE_TVSB2_SNR_R};
unsigned char key_table_dispm[] = {MODULE_TVSB2_M_CAP,MODULE_TVSB2_M_DISP};
unsigned char key_table_dispd[] = {MODULE_TVSB4_ODW,MODULE_TVSB4_ODR,MODULE_TVSB4_DEMURA};
unsigned char key_table_gpu[] = {MODULE_GPU};
unsigned char key_table_memc[] = {MODULE_MEMC};
unsigned char key_table_osd[] = {MODULE_TVSB4_OSD1,MODULE_TVSB4_OSD2,MODULE_TVSB4_OSD3};
//unsigned char key_table_tve[] = {MODULE_TVSB1_TVE_VD, MODULE_TVSB1_TVE_VBI};
//unsigned char key_table_aee[] = {MODULE_SB3_AEE};
//unsigned char key_table_ade[] = {MODULE_SB3_ADE};
unsigned char key_table_vd[] = {MODULE_TVSB1_VD};
unsigned char key_table_vde[] = {MODULE_TVSB7_VDE};
//unsigned char key_table_cp[] = {MODULE_SB1_CP_KC,MODULE_SB1_CP_NKC};
unsigned char key_table_usb[] = {MODULE_SB1_USB,MODULE_TVSB5_USB3};
unsigned char key_table_tp[] = {MODULE_SB1_SSB1_TP_NKC,MODULE_SB1_SSB1_TP_KC,MODULE_SB1_SSB1_TP2_KC,MODULE_SB1_SSB1_TP2_NKC};

//unsigned char key_table_uart[] = {MODULE_SB2_UART,MODULE_SB3_UART2};
unsigned char key_table_uart[] = {MODULE_SB2_UART};

unsigned char key_table_md[] = {MODULE_SB1_MD_KC,MODULE_SB1_MD_NKC_NVC,MODULE_SB1_MD_VC};
unsigned char key_table_etn[] = {MODULE_SB1_ETN};
unsigned char key_table_cpus[] = {MODULE_SB2_VCPU};
unsigned char key_table_emmc[] = {MODULE_SB1_EMMC_KC,MODULE_SB1_EMMC_NKC};
unsigned char key_table_demod[] = {MODULE_TVSB5_DEMOD};
unsigned char key_table_vbi[] = {MODULE_TVSB1_VBI};
//unsigned char key_table_cr[] = {MODULE_SB1_CR};
unsigned char key_table_hostif[] = {MODULE_SB2_HOST_IF};
unsigned char key_table_me[] = {MODULE_TVSB7_ME};
//unsigned char key_table_subtitle[] = {MODULE_TVSB4_SUBT};
unsigned char key_table_dma[] = {MODULE_TVSB5_DEBUGDMA,MODULE_TVSB4_DMATO3DLUT};

const MODULE_SEARCH_TABLE key_table[] =
{
        {"AUDIO", key_table_audio, sizeof(key_table_audio)},
        {"SE", key_table_se, sizeof(key_table_se)},
        //{"VE", key_table_ve, sizeof(key_table_ve)},
        {"DISP_I", key_table_dispi, sizeof(key_table_dispi)},
        {"DISP_M", key_table_dispm, sizeof(key_table_dispm)},
        {"DISP_D", key_table_dispd, sizeof(key_table_dispd)},
        {"GPU", key_table_gpu, sizeof(key_table_gpu)},
        //{"MEMC", key_table_memc, sizeof(key_table_memc)},
        {"OSD", key_table_osd, sizeof(key_table_osd)},
        //{"TVE", key_table_tve, sizeof(key_table_tve)},
        //{"AEE", key_table_aee, sizeof(key_table_aee)},
        //{"ADE", key_table_ade, sizeof(key_table_ade)},
        {"VD", key_table_vd, sizeof(key_table_vd)},
        {"VDE", key_table_vde, sizeof(key_table_vde)},
        //{"CP", key_table_cp, sizeof(key_table_cp)},
        {"USB", key_table_usb, sizeof(key_table_usb)},
        {"TP", key_table_tp, sizeof(key_table_tp)},
        {"UART", key_table_uart, sizeof(key_table_uart)},
        {"MD", key_table_md, sizeof(key_table_md)},
        {"ETN", key_table_etn, sizeof(key_table_etn)},
        {"CPUS", key_table_cpus, sizeof(key_table_cpus)},
        {"EMMC", key_table_emmc, sizeof(key_table_emmc)},
        {"DEMOD", key_table_demod, sizeof(key_table_demod)},
        {"VBI", key_table_vbi, sizeof(key_table_vbi)},
        //{"CR", key_table_cr, sizeof(key_table_cr)},
        {"HOST_IF", key_table_hostif, sizeof(key_table_hostif)},
        {"ME", key_table_me, sizeof(key_table_me)},
        //{"SUBTITLE", key_table_subtitle, sizeof(key_table_subtitle)},
        {"DMA", key_table_dma, sizeof(key_table_dma)},
};

//unsigned char trap_table_ve[] = {MODULE_VE};
//unsigned char trap_table_ve2[] = {MODULE_VE2};
//unsigned char trap_table_aee[] = {MODULE_SB3_AEE};
//unsigned char trap_table_aee2[] = {MODULE_SB3_AEE2};
//unsigned char trap_table_ade[] = {MODULE_SB3_ADE};
//unsigned char trap_table_ade2[] = {MODULE_SB3_ADE2};
unsigned char trap_table_vde[] = {MODULE_TVSB7_VDE};
//unsigned char trap_table_vde2[] = {MODULE_SB3_VDE2};
unsigned char trap_table_kcpucp[] = {MODULE_TVSB3_CP_KC};
unsigned char trap_table_scpucp[] = {MODULE_TVSB3_CP_NKC};
unsigned char trap_table_kcputp[] = {MODULE_SB1_SSB1_TP_KC,};
unsigned char trap_table_scputp[] = {MODULE_SB1_SSB1_TP_NKC};
unsigned char trap_table_kcpumd[] = {MODULE_SB1_MD_KC};
unsigned char trap_table_scpumd[] = {MODULE_SB1_MD_NKC_NVC};
unsigned char trap_table_vcpumd[] = {MODULE_SB1_MD_VC};
unsigned char trap_table_vcpu[] = {MODULE_SB2_VCPU};
unsigned char trap_table_kcpuemmc[] = {MODULE_SB1_EMMC_KC};
unsigned char trap_table_scpuemmc[] = {MODULE_SB1_EMMC_NKC};

const MODULE_SEARCH_TABLE trap_table[] =
{
        //{"VE", trap_table_ve, sizeof(trap_table_ve)},
        //{"VE2", trap_table_ve2, sizeof(trap_table_ve2)},
        //{"AEE", trap_table_aee, sizeof(trap_table_aee)},
        //{"AEE2", trap_table_aee2, sizeof(trap_table_aee2)},
        //{"ADE", trap_table_ade, sizeof(trap_table_ade)},
        //{"ADE2", trap_table_ade2, sizeof(trap_table_ade2)},
        {"VDE", trap_table_vde, sizeof(trap_table_vde)},
        //{"VDE2", trap_table_vde2, sizeof(trap_table_vde2)},
        {"KCPU_CP", trap_table_kcpucp, sizeof(trap_table_kcpucp)},
        {"SCPU_CP", trap_table_scpucp, sizeof(trap_table_scpucp)},
        {"KCPU_TP", trap_table_kcputp, sizeof(trap_table_kcputp)},
        {"SCPU_TP", trap_table_scputp, sizeof(trap_table_scputp)},
        {"KCPU_MD", trap_table_kcpumd, sizeof(trap_table_kcpumd)},
        {"SCPU_MD", trap_table_scpumd, sizeof(trap_table_scpumd)},
        {"VCPU_MD", trap_table_vcpumd, sizeof(trap_table_vcpumd)},
        {"VCPU", trap_table_vcpu, sizeof(trap_table_vcpu)},
        {"KCPU_EMMC", trap_table_kcpuemmc, sizeof(trap_table_kcpuemmc)},
        {"SCPU_EMMC", trap_table_scpuemmc, sizeof(trap_table_scpuemmc)},
};




#endif

