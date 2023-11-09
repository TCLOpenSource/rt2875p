#ifndef  RTK_DC_MT_DBUS_ID_H
#define  RTK_DC_MT_DBUS_ID_H


static const MODULE_INFO module_info[] =
{

	{MODULE_TVSB2_DIW,      "TVSB2_DIW"},
	{MODULE_TVSB2_DIR,      "TVSB2_DIR"},
	{MODULE_TVSB2_M_CAP,    "TVSB2_M_CAP"},
	{MODULE_TVSB2_M_DISP,   "TVSB2_M_DISP"},
	{MODULE_TVSB2_S_CAP,    "TVSB2_S_CAP"},
	{MODULE_TVSB2_S_DISP,   "TVSB2_S_DISP"},
	{MODULE_TVSB2_VO1_Y,    "TVSB2_VO1_Y"},
	{MODULE_TVSB2_VO1_C,    "TVSB2_VO1_C"},
	{MODULE_TVSB2_VO2_Y,    "TVSB2_VO2_Y"},
	{MODULE_TVSB2_VO2_C,    "TVSB2_VO2_C"},
	{MODULE_TVSB2_I3DDMA_Y,   "TVSB2_I3DDMA"},
	{MODULE_TVSB2_DE_XC_W,  "TVSB2_DE_XC_W"},
	{MODULE_TVSB2_DE_XC_R,  "TVSB2_DE_XC_R"},
	{MODULE_TVSB2_SNR_R,    "TVSB2_SNR_R"},
	{MODULE_TVSB2_3DTABLE,  "TVSB2_3DTABLE"},
	{MODULE_TVSB2, 			 "TVSB2"},

	{MODULE_TVSB1_VD,       "TVSB1_VD"},
	{MODULE_TVSB1_AUDIO,    "TVSB1_AUDIO"},
	{MODULE_TVSB1_AUDIO_2,  "TVSB1_AUDIO_2"},
	{MODULE_TVSB1_VBI,      "TVSB1_VBI"},
	{MODULE_TVSB1,  		"TVSB1"},

	{MODULE_SB1_MD_KC,    "SB1_MD_KC"},
	{MODULE_SB1_USB,        "SB1_USB"},
	{MODULE_SB1_RBUSDMA,    "SB1_RBUSDMA"},
	{MODULE_SB1_MD_BBS,     "SB1_MD_BBS"},
	{MODULE_SB1_ETN,        "SB1_ETN"},
	{MODULE_SB1_UART_DMA,      "SB1_UART_DMA"},
	{MODULE_SB1_EMMC_KC,    "SB1_EMMC_KC"},
	{MODULE_SB1_MD_NKC_NVC,      "SB1_MD_NKC_NVC"},
	{MODULE_SB1_MD_VC,     "SB1_MD_VC"},
	{MODULE_SB1_EMMC_NKC,   "SB1_EMMC_NKC"},
	{MODULE_SB1,    "SB1"},

	{MODULE_SB2_VCPU,       "SB2_VCPU"},
	{MODULE_SB2_LZMA,       "SB2_LZMA"},
	{MODULE_SB2_UART,       "SB2_UART"},
	{MODULE_SB2_HOST_IF,    "SB2_HOST_IF"},
	{MODULE_SB2_VCPU2_2,       "SB2_VCPU2"},
	{MODULE_SB2,    		"SB2"},

	//      {MODULE_SB1_SSB1_NAG_DMA_KC,        "SB1_SSB1_NAG_DMA_KC"},

	{MODULE_SB1_SSB1_TP2_KC,        	"SB1_SSB1_TP2_KC"},
	{MODULE_SB1_SSB1_TP_KC,      		"SB1_SSB1_TP_KC"},
	//      {MODULE_SB1_SSB1_NF_KC,   "SB1_SSB1_NF_KC"},
	//      {MODULE_SB1_SSB1_NAG_DMA_SC,     "SB1_SSB1_NAG_DMA_SC"},
	{MODULE_SB1_SSB1_TP2_NKC,      		"SB1_SSB1_TP2_NKC"},
	{MODULE_SB1_SSB1_TP_NKC,    		"SB1_SSB1_TP_NKC"},
	//        {MODULE_SB1_SSB1_NF_NKC,   "SB1_SSB1_NF_NKC"},
	//        {MODULE_SB1_SSB1_NAG_DMA_PB,     "SB1_SSB1_NAG_DMA_PB"},
	//        {MODULE_SB1_SSB1_TP2_PB,      "SB1_SSB1_TP2_PB"},
	//        {MODULE_SB1_SSB1_TP_PB,      "SB1_SSB1_TP_PB"},
	{MODULE_SB1_SSB1,    "SB1_SSB1"},

	{MODULE_VE_VM1, "VE_VM1"},
	{MODULE_VE_VM2, "VE_VM2"},
	{MODULE_VE_PRD, "VE_PRD"},
	{MODULE_VE_DBK, "VE_DBK"},
	{MODULE_VE_MVD, "VE_MVD"},
	{MODULE_VE_NDBP2, 	"VE_NDBP2"},
	{MODULE_VE_R1SEQ,       "VE_R1SEQ"},
	{MODULE_VE_BS,  		"VE_BS"},
	{MODULE_VE_R1BLK,       "VE_R1BLK"},
	{MODULE_VE_VEP2COM,     "VE_VEP2COM"},
	{MODULE_VE_VECOM,       "VE_VECOM"},
	{MODULE_VE_CABP2,       "VE_CABP2"},
	{MODULE_VE_BSP2,        "VE_BSP2"},
	{MODULE_VE_LR_SAO,      "VE_LR_SAO"},
	{MODULE_VE_SEG, 	"VE_SEG"},
	{MODULE_VE_CMP, 	"VE_CMP"},


	{MODULE_TVSB3_SEQ0,     "TVSB3_SEQ0"},
	{MODULE_TVSB3_SEQ1,     "TVSB3_SEQ1"},
	{MODULE_TVSB3_CP_KC,    "TVSB3_CP_KC"},
	{MODULE_TVSB3_CP_NKC,   "TVSB3_CP_NKC"},
	{MODULE_TVSB3_DDR_TR_GEN,       "TVSB3_DDR_TR_GEN"},
	{MODULE_TVSB3_SETFBC0_R_Q0, "TVSB3_SETFBC0_R_Q0"},
	{MODULE_TVSB3_SETFBC0_R_Q1, "TVSB3_SETFBC0_R_Q1"},
	{MODULE_TVSB3_SETFBC1_R_Q0, "TVSB3_SETFBC1_R_Q0"},
	{MODULE_TVSB3_SETFBC1_R_Q1, "TVSB3_SETFBC1_R_Q1"},
	{MODULE_TVSB3,  "TVSB3"},

	{MODULE_GPU_KC,         "GPU_KCPU"},
	{MODULE_GPU_NKC_VB,     "GPU_SCPU"},
	{MODULE_GPU_10, 	"GPU_10"},
	{MODULE_GPU_11, 	"GPU_11"},
	{MODULE_GPU_NKC_VA,     "GPU_SCPU"},
	{MODULE_GPU,    	"GPU"},

	{MODULE_TVSB5_DEMOD,    "TVSB5_DEMOD"},
	{MODULE_TVSB5_DEBUGDMA, "TVSB5_DEBUGDMA"},
	{MODULE_TVSB5_DEMOD2,   "TVSB5_DEMOD2"},
	{MODULE_TVSB5_USB3,     "TVSB5_USB3"},
	{MODULE_TVSB5_DDR_TR_GEN,       "TVSB5_DDR_TR_GEN"},
	{MODULE_TVSB5,  "TVSB5"},

	{MODULE_NN_NONK,        "NN_NONK"},
	{MODULE_NN_KCPU,        "NN_KCPU"},
	{MODULE_NN,     		"NN"},

	{MODULE_VE2_VM1,         "VE2_VM1"},    
	{MODULE_VE2_VM2,         "VE2_VM2"},       
	{MODULE_VE2_PRD,         "VE2_PRD"},
	{MODULE_VE2_DBK,         "VE2_DBK"},  
	{MODULE_VE2_MVD,         "VE2_MVD"},        
	{MODULE_VE2_NDPB2,       "VE2_NDPB2"}, 	
	{MODULE_VE2_R1_SEQ  ,    "VE2_R1_SEQ"},
	{MODULE_VE2_BS   , 		 "VE2_BS"}, 	
	{MODULE_VE2_R1_BLOCK ,	 "VE2_R1_BLOCK"}, 	
	{MODULE_VE2_VEP2COM  ,   "VE2_VEP2COM"}, 	
	{MODULE_VE2_VECOM    ,   "VE2_VECOM"},
	{MODULE_VE2_CABP2    ,   "VE2_CABP2"},	
	{MODULE_VE2_BSP2     ,   "VE2_BSP2"},	
	{MODULE_VE2_LR_OR_SAO  , "VE2_LR_OR_SAO"},
	{MODULE_VE2_SEG       ,	 "VE2_SEG"},
	{MODULE_VE_CMP, 		 "VE2_CMP"},

	{MODULE_TVSB4_ODW,      		"TVSB4_ODW"},
	{MODULE_TVSB4_ODR,      		"TVSB4_ODR"},
	{MODULE_TVSB4_DC2H,     		"TVSB4_DC2H"},
	{MODULE_TVSB4_DDR_TR_GEN,       "TVSB4_DDR_TR_GEN"},
	{MODULE_TVSB4_OSD1,     		"TVSB4_OSD1"},
	{MODULE_TVSB4_OSD2,     		"TVSB4_OSD2"},
	{MODULE_TVSB4_OSD3,     		"TVSB4_OSD3"},
	{MODULE_TVSB4_DEMURA,   		"TVSB4_DEMURA"},
	{MODULE_TVSB4_DMATO3DLUT,       "TVSB4_DMATO3DLUT"},
	{MODULE_TVSB4_OSD_TFBC_1,       "TVSB4_OSD_TFBC_1"},
	{MODULE_TVSB4_OSD_TFBC_2,       "TVSB4_OSD_TFBC_2"},
	{MODULE_TVSB4_DMATOSRNN ,   	"TVSB4_DMATOSRNN"},
	{MODULE_TVSB4_D_PQMASK_D, 		"MODULE_TVSB4_D_PQMASK_D"},
	{MODULE_TVSB4_RTK_LD    ,  		"MODULE_TVSB4_RTK_LD"},
	{MODULE_TVSB4_SLD_W,			"MODULE_TVSB4_SLD_W"},
	{MODULE_TVSB4,  				"TVSB4"},

	{MODULE_MEMC_ME_W,      		"MEMC_ME_W"},
	{MODULE_MEMC_ME_R,      		"MEMC_ME_R"},
	{MODULE_MEMC_MC_LF_W,      		"MEMC_MC_LF_W"},
	{MODULE_MEMC_MC_HF_W,      		"MEMC_MC_HF_W"},
	{MODULE_MEMC_MC_LFI_R,      		"MEMC_MC_LFI_R"},
	{MODULE_MEMC_MC_HFI_R,     		"MEMC_MC_HFI_R"},
	{MODULE_MEMC_MC_LFP_R,     		"MEMC_MC_LFP_R"},
	{MODULE_MEMC_MC_HFP_R,     		"MEMC_MC_HFP_R"},
	{MODULE_MEMC_ME_SHR_W,     		"MEMC_ME_SHR_W"},
	{MODULE_MEMC_ME_SHR_R,    	 	"MEMC_ME_SHR_R"},
	{MODULE_MEMC_MVINFO_W,     		"MEMC_MVINFO_W"},
	{MODULE_MEMC_MVINFO_R,     		"MEMC_MVINFO_R"},
	{MODULE_MEMC,   		   		"MEMC"},

	{MODULE_TVSB7_ME,       		"TVSB7_ME"},
	{MODULE_TVSB7_VDE,      		"TVSB7_VDE"},
	{MODULE_TVSB7_MF2,      		"TVSB7_MF2"},
	{MODULE_TVSB7_CDF_ALF,  		"TVSB7_CDF_ALF"},
	{MODULE_TVSB7_MF1,      		"TVSB7_MF1"},
	{MODULE_TVSB7_MNHP2,     		"TVSB7_MNHP2"},
	{MODULE_TVSB7_CMP,  			"TVSB7_CMP"},
	{MODULE_TVSB7_MF0,      		"TVSB7_MF0"},
	{MODULE_TVSB7_VDE2,     		"TVSB7_VDE2 "},
	{MODULE_TVSB7_DDR_TR_GEN, 		"TVSB7_DDR_TR_GEN"},
	{MODULE_TVSB7,  				"TVSB7"},


	{MODULE_MC_PTG ,  				"MC_PTG"},
	{MODULE_VE2_MF2,                "VE2_MF2"},
	{MODULE_VE2_CDEF_OR_ATF,   		"VE2_CDEF_OR_ATF"},
	{MODULE_VE2_MF1, 				"VE2_MF1"},
	{MODULE_VE2_MNHP2,              "VE2_MNHP2"},
	{MODULE_VE2_CMP, 				"VE2_CMP"},
	{MODULE_VE2_MF0, 				"VE2_MF0"},
	{MODULE_VE2_DMVR,               "VE2_DMVR"},

	{MODULE_ANY,    				"ANY"},
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
unsigned char key_table_cpus[] = {MODULE_SB2_VCPU, MODULE_SB2_VCPU2_2};
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

