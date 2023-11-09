#include "memc_isr/PQL/PQLPlatformDefs.h"
#include "memc_isr/PQL/PQLContext.h"
#include "memc_isr/PQL/FRC_glb_Context.h"
#include "memc_isr/PQL/PQLGlobalDefs.h"
#include <memc_isr/scalerMEMC.h>
#include "rtk_vip_logger.h"
#include "memc_reg_def.h"
#include <tvscalercontrol/panel/panelapi.h>
#include <scaler/vipRPCCommon.h>

#include <rtd_log/rtd_module_log.h>
extern unsigned int MA_print_count;
#define ROSPrintf_MEMC_IP31(fmt, args...) VIPprintf(VIP_LOGLEVEL_DEBUG,VIP_LOGMODULE_MEMC_IP31_DEBUG,MA_print_count,fmt,##args)
#define ROSPrintf_MEMC_IP31_interface1(fmt, args...) VIPprintf(VIP_LOGLEVEL_DEBUG,VIP_LOGMODULE_MEMC_IP31_interface1_DEBUG,MA_print_count,fmt,##args)//mixmode use
#define ROSPrintf_MEMC_IP31_interface2(fmt, args...) VIPprintf(VIP_LOGLEVEL_DEBUG,VIP_LOGMODULE_MEMC_IP31_interface2_DEBUG,MA_print_count,fmt,##args)//mixmode use
#define ROSPrintf_MEMC_IP31_interface3(fmt, args...) VIPprintf(VIP_LOGLEVEL_DEBUG,VIP_LOGMODULE_MEMC_IP31_interface3_DEBUG,MA_print_count,fmt,##args)//motion sequence
#define ROSPrintf_MEMC_IP31_interface4(fmt, args...) VIPprintf(VIP_LOGLEVEL_DEBUG,VIP_LOGMODULE_MEMC_IP31_interface4_DEBUG,MA_print_count,fmt,##args)
#define ROSPrintf_MEMC_IP31_interface5(fmt, args...) VIPprintf(VIP_LOGLEVEL_DEBUG,VIP_LOGMODULE_MEMC_IP31_interface5_DEBUG,MA_print_count,fmt,##args)
#define ROSPrintf_MEMC_IP31_interface6(fmt, args...) VIPprintf(VIP_LOGLEVEL_DEBUG,VIP_LOGMODULE_MEMC_IP31_interface6_DEBUG,MA_print_count,fmt,##args)
#define ROSPrintf_MEMC_IP31_interface7(fmt, args...) VIPprintf(VIP_LOGLEVEL_DEBUG,VIP_LOGMODULE_MEMC_IP31_interface7_DEBUG,MA_print_count,fmt,##args)
#define MEMC_IP31_NULL_Print_Cnt (300)

/////
extern MEMC_APP_DATA_ST		gstAppData;
extern int LowDelay_mode;
extern unsigned char g_scaler_memc_cadence[_FILM_MEMC_NUM];
unsigned char  u8_filmCad_enterCnt[_FRC_CADENCE_NUM_][_FILM_MOT_NUM];
#define	SOFTWARE1__SOFTWARE1_12_ADDR	SOFTWARE1_SOFTWARE1_12_reg
extern unsigned char u8_MEMCMode;
extern unsigned int dejudder;
extern unsigned char MEMC_Lib_get_Adaptive_Stream_Flag(VOID);
#ifdef CONFIG_MEDIA_SUPPORT
extern unsigned char vsc_get_main_win_apply_done_event_subscribe(void);
#endif
extern _RPC_system_setting_info* scaler_GetShare_Memory_RPC_system_setting_info_Struct(void);

unsigned int  u32_32322_BadEdit_case[60]=
{
	0x252A52A4, 0xA54A549, 0x14A94A92, 0x29529524, 0x12A52A49,
	0x254A5492, 0xA94A925, 0x1529524A, 0x2A52A495, 0x14A5492A,
	0x294A9254, 0x129524A9, 0x252A4952, 0xA5492A5, 0x14A9254A,
	0x29524A94, 0x12A49529, 0x25492A52, 0xA9254A5, 0x1524A94A,
	0x2A495295, 0x1492A52A, 0x29254A54, 0x124A94A9, 0x24952952,
	0x92A52A5, 0x1254A54A, 0x24A94A94, 0x9529529, 0x12A52A52,
	0x254A54A4, 0xA94A949, 0x15295292, 0x2A52A524, 0x14A54A49,
	0x294A9492, 0x12952925, 0x252A524A, 0xA54A494, 0x14A94929,
	0x29529252, 0x12A524A5, 0x254A494A, 0xA949295, 0x1529252A,
	0x2A524A54, 0x14A494A9, 0x29492952, 0x129252A5, 0x2524A54A,
	0xA494A94, 0x14929529, 0x29252A52, 0x124A54A5, 0x2494A94A,
	0x9295295, 0x1252A52A, 0x24A54A54, 0x94A94A9, 0x12952952
};
#if Pro_tv010
unsigned char u32_Special_60_cnt = 255;
unsigned int  u32_Special_60_BadEdit_case[255]=
{
	4214838360, 4134709425, 3974451555, 3653935814, 3012904332, 1730841369, 3461682738, 2628398180, 961829065, 1923658131, 
	3847316262, 3399665228, 2504363161, 713759026, 1427518053, 2855036106, 1415104916, 2830209832, 1365452369, 2730904739, 
	1166842182, 2333684365, 372401434, 744802868, 1489605737, 2979211474, 1663455652, 3326911305, 2358855314, 422743332, 
	845486665, 1690973331, 3381946662, 2468926029, 642884762, 1285769524, 2571539049, 848110802, 1696221604, 3392443209, 
	2489919122, 684870948, 1369741897, 2739483795, 1184000295, 2368000590, 441033884, 882067768, 1764135536, 3528271073, 
	2761574851, 1228182406, 2456364812, 617762329, 1235524658, 2471049317, 647131338, 1294262677, 2588525354, 882083412, 
	1764166825, 3528333651, 2761700006, 1228432716, 2456865433, 618763571, 1237527142, 2475054284, 655141273, 1310282546, 
	2620565092, 946162889, 1892325778, 3784651556, 3274335817, 2253704338, 212441381, 424882763, 849765526, 1699531052, 
	3399062105, 2503156914, 711346532, 1422693065, 2845386130, 1395804964, 2791609929, 1288252562, 2576505124, 858042953, 
	1716085907, 3432171814, 2569376332, 843785369, 1687570738, 3375141477, 2455315659, 615664022, 1231328044, 2462656088, 
	630344880, 1260689761, 2521379523, 747791750, 1495583500, 2991167001, 1687366707, 3374733414, 2454499532, 614031768, 
	1228063537, 2456127074, 617286852, 1234573705, 2469147411, 643327526, 1286655053, 2573310106, 851652916, 1703305832, 
	3406611665, 2518256034, 741544773, 1483089546, 2966179092, 1637390889, 3274781778, 2254596261, 214225227, 428450454, 
	856900908, 1713801816, 3427603633, 2560239971, 825512646, 1651025292, 3302050584, 2309133873, 323300451, 646600902, 
	1293201804, 2586403609, 877839922, 1755679845, 3511359690, 2727752084, 1160536873, 2321073746, 347180197, 694360394, 
	1388720788, 2777441577, 1259915858, 2519831717, 744696138, 1489392276, 2978784553, 1662601810, 3325203621, 2355439946, 
	415912596, 831825193, 1663650386, 3327300773, 2359634251, 424301206, 848602412, 1697204824, 3394409649, 2493852003, 
	692736710, 1385473420, 2770946840, 1246926385, 2493852771, 692738246, 1385476492, 2770952985, 1246938674, 2493877349, 
	692787402, 1385574804, 2771149609, 1247331922, 2494663845, 694360394, 1388720789, 2777441578, 1259915860, 2519831721, 
	744696147, 1489392294, 2978784588, 1662601880, 3325203761, 2355440226, 415913157, 831826314, 1663652628, 3327305257, 
	2359643218, 424319141, 848638282, 1697276564, 3394553129, 2494138962, 693310629, 1386621258, 2773242517, 1251517738, 
	2503035476, 711103657, 1422207315, 2844414630, 1393861964, 2787723929, 1280480562, 2560961125, 826954954, 1653909908, 
	3307819817, 2320672338, 346377381, 692754762, 1385509524, 2771019049, 1247070802, 2494141604, 693315913, 1386631827, 
	2773263654, 1251560012, 2503120025, 711272754, 1422545508, 2845091017, 1395214739, 2790429478, 1285891660, 2571783321, 
	848599346, 1697198693, 3394397386, 2493827476, 692687656, 1385375313, 2770750626, 1246533957, 2493067914, 691168533, 
	1382337066, 2764674132, 1234380969, 2468761938, 642556581, 
};

unsigned char u32_Special_50_cnt = 209;
unsigned int  u32_Special_50_BadEdit_case[209]=
{
	4268970637, 4242973978, 4190980661, 4086994027, 3879020758, 3463074220, 2631181145, 967394995, 1934789990, 3869579980, 
	3444192664, 2593418033, 891868771, 1783737543, 3567475086, 2839982876, 1384998457, 2769996914, 1245026533, 2490053066, 
	685138836, 1370277673, 2740555346, 1186143397, 2372286794, 449606292, 899212585, 1798425170, 3596850341, 2898733386, 
	1502499476, 3004998953, 1715030611, 3430061222, 2565155148, 835343000, 1670686001, 3341372003, 2387776710, 480586124, 
	961172249, 1922344499, 3844688998, 3394410700, 2493854105, 692740914, 1385481829, 2770963659, 1246960022, 2493920044, 
	692872793, 1385745586, 2771491172, 1248015049, 2496030098, 697092901, 1394185802, 2788371604, 1281775913, 2563551826, 
	832136357, 1664272715, 3328545430, 2362123565, 429279834, 858559668, 1717119337, 3434238674, 2573510053, 852052810, 
	1704105620, 3408211241, 2521455187, 747943078, 1495886157, 2991772314, 1688577333, 3377154667, 2459342038, 623716780, 
	1247433560, 2494867121, 694766947, 1389533894, 2779067789, 1263168283, 2526336566, 757705836, 1515411673, 3030823346, 
	1766679396, 3533358793, 2771750291, 1248533287, 2497066574, 699165852, 1398331704, 2796663409, 1298359523, 2596719046,
	898470796, 1796941593, 3593883186, 2892799077, 1490630858, 2981261716, 1667556136, 3335112273, 2375257251, 455547206, 
	911094412, 1822188825, 3644377651, 2993788006, 1692608717, 3385217434, 2475467573, 655967850, 1311935701, 2623871402, 
	952775509, 1905551018, 3811102036, 3327236777, 2359506259, 424045222, 848090444, 1696180889, 3392361779, 2489756262,
	684545228, 1369090457, 2738180914, 1181394533, 2362789066, 430610836, 861221672, 1722443345, 3444886691, 2594806086, 
	894644876, 1789289753, 3578579507, 2862191718, 1429416141, 2858832282, 1422697269, 2845394538, 1395821781, 2791643562, 
	1288319829, 2576639658, 858312021, 1716624042, 3433248085, 2571528875, 848090454, 1696180908, 3392361817, 2489756338, 
	684545381, 1369090762, 2738181524, 1181395753, 2362791507, 430615718, 861231436, 1722462873, 3444925747, 2594884198, 
	894801101, 1789602202, 3579204404, 2863441513, 1431915730, 2863831460, 1432695625, 2865391250, 1435815205, 2871630410, 
	1448293525, 2896587050, 1498206805, 2996413610, 1697859925, 3395719850, 2496472404, 697977513, 1395955027, 2791910054, 
	1288852813, 2577705626, 860443956, 1720887913, 3441775826, 2588584356, 882201417, 1764402834, 3528805669, 2762644042, 
	1230320788, 2460641577, 626315858, 1252631717, 2505263435, 715559574, 1431119148, 2862238297, 1429509299, 
};

unsigned char u32_Special_30_cnt = 223;
unsigned int  u32_Special_30_BadEdit_case[223]=
{
	3484923895, 2674880495, 1054793694, 2109587389, 4219174779, 4143382263, 3991797231, 3688627167, 3082287038, 1869606781, 
	3739213563, 3183459831, 2071952366, 4143904732, 3992842169, 3690717042, 3086466789, 1877966283, 3755932567, 3216897838, 
	2138828381, 4277656762, 4260346229, 4225725162, 4156483029, 4017998763, 3741030230, 3187093165, 2079219035, 4158438071, 
	4021908847, 3748850398, 3202733501, 2110499707, 4220999414, 4147031533, 3999095770, 3703224245, 3111481195, 1927995094, 
	3855990188, 3417013081, 2539058866, 783150437, 1566300874, 3132601749, 1970236202, 3940472405, 3585977515, 2876987734, 
	1459008173, 2918016347, 1541065398, 3082130797, 1869294299, 3738588598, 3182209901, 2069452507, 4138905014, 3982842733, 
	3670718170, 3046469045, 1797970795, 3595941591, 2896915887, 1498864478, 2997728957, 1700490619, 3400981239, 2506995183, 
	719023070, 1438046141, 2876092283, 1457217270, 2914434541, 1533901787, 3067803574, 1840639853, 3681279706, 3067592117, 
	1840216939, 3680433879, 3065900463, 1836833631, 3673667262, 3052367229, 1809767162, 3619534325, 2944101355, 1593235414, 
	3186470829, 2077974363, 4155948727, 4016930159, 3738893023, 3182818750, 2070670205, 4141340410, 3987713525, 3680459755, 
	3065952215, 1836937135, 3673874270, 3052781245, 1810595195, 3621190391, 2947413487, 1599859678, 3199719357, 2104471418, 
	2675194847, 1055422398, 2110844797, 4221689595, 4148411895, 4001856494, 3708745692, 3122524089, 1950080882, 3900161765, 
	3505356235, 2715745175, 1136523054, 2273046109, 251124922, 502249845, 1004499690, 2008999381, 4017998763, 3741030230, 
	3187093165, 2079219035, 4158438071, 4021908847, 3748850398, 3202733501, 2110499707, 4220999414, 4147031533, 3999095770, 
	3703224245, 3111481195, 1927995094, 3855990188, 3417013081, 2539058866, 783150437, 1566300874, 3132601749, 1970236202, 
	3940472405, 3585977515, 2876987734, 1459008173, 2918016347, 1541065398, 3082130797, 1869294299, 3738588598, 3182209901, 
	2069452507, 4138905014, 3982842733, 3670718170, 3046469045, 1797970795, 3595941591, 2896915887, 1498864478, 2997728957,
	1700490619, 3400981239, 2506995183, 719023070, 1438046141, 2876092283, 1457217270, 2914434541, 1533901787, 3067803574, 
	1840639853, 3681279706, 3067592117, 1840216939, 3680433879, 3065900463, 1836833631, 3673667262, 3052367229, 1809767162, 
	3619534325, 2944101355, 1593235414, 3186470829, 2077974363, 4155948727, 4016930159, 3738893023, 3182818750, 2070670205,
	4141340410, 3987713525, 3680459755, 3065952215, 1836937135, 3673874270, 3052781245, 1810595195, 3621190391, 2947413486, 
	1599859676, 3199719353, 4226793211, 4158619127, 4022270959, 3749574623, 3204181950, 2113396605, 4226793211, 4158619126, 
	4022270957, 3749574619, 4208942837, 
};
#endif

///////////////////////////////////////////////////////////////////////////////////////////
unsigned int FrameMotionTH26(const _PARAM_FilmDetectCtrl *pParam, _OUTPUT_FilmDetectCtrl *pOutput, _PQL_FILM_MOT_TYPE motion_T)
{
#if 0
	unsigned char i = 0;
	unsigned int u27_FrmMotTh = 0;

	/*
		25          20                        10                         0
		 o o o o o o o o o o o o o o o o o o o o o o o o o x
	Sort
	Tick
	*/
	// remove oldest element
	unsigned char u5_remove_index = 25;
	for (i = 0; i < 26; i++)
	{
		if (pOutput->u27_FrmMotionSortTick[i] == 25)
		{
			u5_remove_index = i;
			break;
		}
	}

	for (i = u5_remove_index; i > 0; i--)
	{
		pOutput->u27_FrmMotionSort[i] = pOutput->u27_FrmMotionSort[i-1];
		pOutput->u27_FrmMotionSortTick[i] = pOutput->u27_FrmMotionSortTick[i-1];
	}
	// FrmMotionSort[0] and FrmMotionSortTick[0] are empty now, waiting for new data

	// sorting 26 frame motion
	pOutput->u27_FrmMotionSort[0] = pOutput->u27_ipme_motionPool[motion_T][0];
	pOutput->u27_FrmMotionSortTick[0] = 0;

	// switch number and tick
	for (i = 0; i < 25; i++)
	{
		unsigned int tmp;
		if (pOutput->u27_FrmMotionSort[i] > pOutput->u27_FrmMotionSort[i+1])
		{
			tmp = pOutput->u27_FrmMotionSort[i+1];
			pOutput->u27_FrmMotionSort[i+1] = pOutput->u27_FrmMotionSort[i];
			pOutput->u27_FrmMotionSort[i] = tmp;

			tmp = pOutput->u27_FrmMotionSortTick[i+1];
			pOutput->u27_FrmMotionSortTick[i+1] = pOutput->u27_FrmMotionSortTick[i];
			pOutput->u27_FrmMotionSortTick[i] = tmp;
		}
		else
		{
			break;
		}
	}

	// all tick++
	for (i = 0; i < 26; i++)
	{
		pOutput->u27_FrmMotionSortTick[i]++;

		// remove maximum and 9 minimun frame motion
		if (i >= 9 && i < 25)
		{
			u27_FrmMotTh += pOutput->u27_FrmMotionSort[i];
		}
	}
	u27_FrmMotTh = (u27_FrmMotTh>>1);

	return u27_FrmMotTh;
#else
	return 0;
#endif
}

unsigned int FrameMotionTH8(const _PARAM_FilmDetectCtrl *pParam, _OUTPUT_FilmDetectCtrl *pOutput, _PQL_FILM_MOT_TYPE motion_T, unsigned int u27_FrmMot_sum, unsigned int u27_maxMot, unsigned int u27_secMot)
{
	// clip maxMot using secMot
	//unsigned int u27_maxMot_clip = 0x80000;
	//unsigned int u27_secFMot_clip = 0;
	//unsigned int u27_maxFMot_clip = 0;

	unsigned int u27_FrmMot_sum_t = u27_FrmMot_sum;
#if 0
	if(u27_secMot < 0x10){
		u27_maxMot_clip = _MIN_(u27_maxMot_clip, u27_maxMot); //max=960x540x255=0x7e11700
	}
	else{
		u27_maxMot_clip = _MAX_(_MIN_(u27_maxMot_clip, u27_maxMot), u27_secMot);
	}

	if(u27_secMot > 32*(1+pOutput->u27_g_FrmMot_Th[motion_T])){
		u27_secFMot_clip = _MIN_(0x80000, u27_secMot/32);		//max
		u27_maxFMot_clip = _MIN_(0x80000, u27_maxMot/32);	//sec
		u27_FrmMot_sum_t = u27_FrmMot_sum_t - u27_maxMot + u27_maxFMot_clip -u27_secMot + u27_secFMot_clip;
	}
	else if(u27_secMot > 12*(1+pOutput->u27_g_FrmMot_Th[motion_T])){
		u27_secFMot_clip = _MIN_(0x80000, u27_secMot/16);
		u27_maxFMot_clip = _MIN_(0x80000, u27_maxMot/16);
		u27_FrmMot_sum_t = u27_FrmMot_sum_t - u27_maxMot + u27_maxFMot_clip -u27_secMot + u27_secFMot_clip;
	}
	else{
		u27_FrmMot_sum_t = u27_FrmMot_sum_t - u27_maxMot + u27_maxMot_clip;
	}

	//for debug
	//if(u27_FrmMot_sum_t == u27_FrmMot_sum)
	//	WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 8, 11, 0x1);
	//else
	//	WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 8, 11, 0x0);

	return u27_FrmMot_sum;
#else
//	if(pOutput->u27_ipme_motionPool[motion_T][0]>=u27_maxMot){
//		u27_secMot = u27_maxMot;
//		u27_maxMot = pOutput->u27_ipme_motionPool[motion_T][0];
//	}

	if((u27_maxMot >= 5*u27_secMot) && (u27_maxMot >= 32*(1+pOutput->u27_g_FrmMot_Th[motion_T])) && (u27_secMot >= 200)){
		u27_FrmMot_sum_t = u27_FrmMot_sum - u27_maxMot + u27_secMot;
		//rtd_pr_memc_notice("[MEMC_FrameMotionTH8][,%d,%d,][,%d,%d,%d,]\n\r", u27_FrmMot_sum_t, u27_FrmMot_sum, u27_maxMot, u27_secMot, pOutput->u27_g_FrmMot_Th[motion_T]);
	}

	return u27_FrmMot_sum_t;
#endif

}

unsigned char g_small_motion_flag = 0;
VOID Motion_Sequence_Gen(const _PARAM_FilmDetectCtrl *pParam, _OUTPUT_FilmDetectCtrl * pOutput, _PQL_FILM_MOT_TYPE motion_T)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();

	unsigned char  u8_i = 0, u8_j = 0;
	unsigned char  u1_FrmSeq = 0,u1_FrmSeq_for_mixmode = 0, u4_Mot_diffWgt = 0, u4_FrmMot_bigCnt = 0, u8_curCad_id = 0, u1_curCad_motFlag = 0;
	unsigned int  u27_cur_FrmMot = 0, u27_FrmMot_sum = 0, u27_FrmMot_Th = 0, u27_FrmMot_Th_for_mixmode = 0, u27_FrmMot_min = 0, u27_FrmMot_big = 0, u27_FrmMot_Lmt = 150000;

	unsigned int  u27_maxMot = 0;
	unsigned int  u27_secMot = 0;
	unsigned int  u27_tmpMot = 0;
	unsigned int  MotionPool_tmp[8] = {0};
	unsigned int  log_en = 0, test_en = 0;
	unsigned int  u27_Mot_all_min_smallmot_check_en=0;
	int motion_diff=0;
	unsigned int test_en2 = 0;
	static unsigned int small_motion_cnt = 0;

	u8_curCad_id = pOutput->u8_cur_cadence_Id[motion_T];
	ReadRegister(SOFTWARE1_SOFTWARE1_60_reg, 30, 30, &log_en);
	ReadRegister(SOFTWARE1_SOFTWARE1_60_reg, 15, 15, &u27_Mot_all_min_smallmot_check_en);
	ReadRegister(SOFTWARE1_SOFTWARE1_62_reg, 0, 0, &test_en);
	ReadRegister(SOFTWARE1_SOFTWARE1_62_reg, 3, 3, &test_en2);
	if(u8_curCad_id == _CAD_32322){
		u1_curCad_motFlag  = cadence_seqFlag(frc_cadTable[u8_curCad_id], ((pOutput->u8_phase_Idx[motion_T]+6)%12));
	}else{	
		u1_curCad_motFlag  = cadence_seqFlag(frc_cadTable[u8_curCad_id], pOutput->u8_phase_Idx[motion_T]);
	}


	// distinguish information from motionType
	if(motion_T == _FILM_ALL)
	{
		u27_cur_FrmMot = s_pContext->_output_read_comreg.u27_ipme_aMot_rb;
		u4_Mot_diffWgt = pParam->u4_Mot_all_diffWgt;
		u27_FrmMot_min = pParam->u27_Mot_all_min;		
		u27_FrmMot_min = _MIN_(pOutput->u27_quit_motTh[_FILM_ALL], pParam->u27_Mot_all_min);
	}
	#if MIX_MODE_REGION_17
	else if(motion_T >=_FILM_12RGN_01)   // 17 region
	{
		u27_cur_FrmMot = s_pContext->_output_read_comreg.u27_ipme_12Mot_rb[motion_T - 1 - _FILM_5RGN_CEN];
		u4_Mot_diffWgt = pParam->u4_Mot_rgn_diffWgt;
		u27_FrmMot_min = 0;
	}
	else 
	{
		u27_cur_FrmMot = s_pContext->_output_read_comreg.u27_ipme_5Mot_rb[motion_T - 1];
		u4_Mot_diffWgt = pParam->u4_Mot_rgn_diffWgt;
		u27_FrmMot_min = 0;
	}
	#else
	else // 5 region
	{
		u27_cur_FrmMot = s_pContext->_output_read_comreg.u27_ipme_5Mot_rb[motion_T - 1];
		u4_Mot_diffWgt = pParam->u4_Mot_rgn_diffWgt;
		u27_FrmMot_min = 0;
	}
	#endif

	for (u8_i = 0; u8_i < 8; u8_i ++){
		MotionPool_tmp[u8_i] = pOutput->u27_ipme_motionPool[motion_T][u8_i];
	}

	// update motion loop.
	for (u8_i = 0; u8_i < 8; u8_i ++)
	{
		if (((pOutput->u32_Mot_sequence[motion_T] >> u8_i) & 0x01) == 1)
		{
			u4_FrmMot_bigCnt ++;
			u27_FrmMot_big += pOutput->u27_ipme_motionPool[motion_T][u8_i];
		}

		//record max & sec motion
		for (u8_j = 0; u8_j < 8-u8_i-1; u8_j++){
			if(MotionPool_tmp[u8_j] > MotionPool_tmp[u8_j+1]){
				u27_tmpMot = MotionPool_tmp[u8_j];
				MotionPool_tmp[u8_j] = MotionPool_tmp[u8_j+1];
				MotionPool_tmp[u8_j+1] = u27_tmpMot;
			}
		}
	}
	u27_FrmMot_big = u4_FrmMot_bigCnt > 0? (u27_FrmMot_big * 2 / u4_FrmMot_bigCnt) : 0x20000;
	u27_maxMot = MotionPool_tmp[7];
	u27_secMot = MotionPool_tmp[6];

	for (u8_i = 7; u8_i >0; u8_i --)
	{
		u27_FrmMot_sum                              += pOutput->u27_ipme_motionPool[motion_T][u8_i];
		pOutput->u27_ipme_motionPool[motion_T][u8_i] = pOutput->u27_ipme_motionPool[motion_T][u8_i-1];
	}
	u27_FrmMot_sum                           += pOutput->u27_ipme_motionPool[motion_T][0];
	pOutput->u27_ipme_motionPool[motion_T][0] = u27_cur_FrmMot;

	// adjust frame motion th
#if 0	
	if(motion_T == _FILM_ALL){
		unsigned int u27_Frm26Mot_sum = 0;
		u27_Frm26Mot_sum = FrameMotionTH26(pParam, pOutput, motion_T);

		//unsigned int u32_RB_val = 0;
		//ReadRegister(SOFTWARE_SOFTWARE_00_reg, 0, 31, &u32_RB_val);
		unsigned char u5_Frm26MotS_wgt = 32;// _MIN_(((u32_RB_val >> 17) & 0x3f), 32);

		u27_FrmMot_sum = ((32-u5_Frm26MotS_wgt)*u27_FrmMot_sum + u5_Frm26MotS_wgt*u27_Frm26Mot_sum)>>5;
	}
#else
	u27_FrmMot_sum = FrameMotionTH8(pParam, pOutput, motion_T, u27_FrmMot_sum, u27_maxMot, u27_secMot);
#endif		
	
	// motion limit
	pOutput->u1_Mot_selBig = 0;
	if ((motion_T == _FILM_ALL) && (u27_cur_FrmMot > u27_FrmMot_big) && (u27_cur_FrmMot > u27_FrmMot_Lmt))
	{
		//u27_cur_FrmMot         = u27_FrmMot_big;
		u27_cur_FrmMot = _MAX_(u27_FrmMot_big,u27_FrmMot_Lmt);
		pOutput->u27_ipme_motionPool[motion_T][0] = u27_cur_FrmMot;
		pOutput->u1_Mot_selBig = 1;
	}

	// motion sequence judge
	u27_FrmMot_Th = (u27_FrmMot_sum >> 3) * u4_Mot_diffWgt >> 3;
	u1_FrmSeq     = (u27_cur_FrmMot > u27_FrmMot_Th && u27_cur_FrmMot > u27_FrmMot_min)? 1 : 0;

	// motion sequence judge (mixmode)
	u27_FrmMot_Th_for_mixmode= (u27_FrmMot_sum >> 3) * 6 >> 3;
	u1_FrmSeq_for_mixmode     = (u27_cur_FrmMot > u27_FrmMot_Th_for_mixmode && u27_cur_FrmMot > u27_FrmMot_min)? 1 : 0;

	// small_motion_judge
	if(motion_T == _FILM_ALL){
		if((u27_cur_FrmMot*u27_maxMot) != 0 && (u27_maxMot <= 50*u27_cur_FrmMot) && (u27_maxMot <= 10000)){
			small_motion_cnt++;
		}
		else{
			if(small_motion_cnt > 10)
				small_motion_cnt-=10;
			else
				small_motion_cnt = 0;
		}

		if(small_motion_cnt >= 200)
			small_motion_cnt = 200;

		if(small_motion_cnt >= 30)
			g_small_motion_flag = 1;
		else
			g_small_motion_flag = 0;
	}
#if 1
	if(log_en && motion_T == _FILM_ALL){
		rtd_pr_memc_notice("[%s][%d][,%d,%d,][,%d,%d,%d,%d,][,%d,%d,]\n\r", __FUNCTION__, __LINE__, u1_FrmSeq, u1_curCad_motFlag, u27_cur_FrmMot, u27_FrmMot_Th, u27_FrmMot_min, pOutput->u27_quit_motTh[_FILM_ALL], u27_maxMot, small_motion_cnt);
	}
#endif

	pOutput->u27_g_FrmMot_Th[motion_T] = u27_FrmMot_Th;

	//for 32 10110
	if (pOutput->u8_cur_cadence_Id[motion_T] == _CAD_32 )
	{
		if ((pOutput->u8_phT_phase_Idx[motion_T] == 3) && ((pOutput->u32_Mot_sequence[motion_T]&0x1) == 1) && test_en == 0)
		{
			u1_FrmSeq = (u1_FrmSeq == 1) ? 0 : u1_FrmSeq;
			u1_FrmSeq_for_mixmode = (u1_FrmSeq_for_mixmode == 1) ? 0 : u1_FrmSeq_for_mixmode;
			pOutput->u27_ipme_motionPool[motion_T][0] = pOutput->u27_quit_smlMot[motion_T];
		}
	}
#if 0
	//for 32322 -> 32232
	if (pOutput->u8_cur_cadence_Id[motion_T] == _CAD_32322)
	{
		if ((pOutput->u8_phT_phase_Idx[motion_T] == 8) && ((pOutput->u32_Mot_sequence[motion_T]&0x1) == 1))
		{
			u1_FrmSeq = (u1_FrmSeq == 1) ? 0 : u1_FrmSeq;
			u1_FrmSeq_for_mixmode = (u1_FrmSeq_for_mixmode == 1) ? 0 : u1_FrmSeq_for_mixmode;
			pOutput->u27_ipme_motionPool[motion_T][0] = pOutput->u27_quit_smlMot[motion_T];
		}
	}
	if (pOutput->u8_cur_cadence_Id[motion_T] == _CAD_32322)
	{
		if(u1_FrmSeq == 0 && (pOutput->u27_ipme_motionPool[motion_T][0] > (u27_FrmMot_Th>>1)) && (u27_cur_FrmMot > u27_FrmMot_min)){
			u1_FrmSeq = (u1_FrmSeq == 0) ? 1 : u1_FrmSeq;
			u1_FrmSeq_for_mixmode = (u1_FrmSeq_for_mixmode == 0) ? 1 : u1_FrmSeq_for_mixmode;
		}
	}

	motion_diff = u27_cur_FrmMot - u27_FrmMot_Th;
	
	//small motion check
	if(u27_Mot_all_min_smallmot_check_en==1)
	{
		motion_diff = ((u27_cur_FrmMot) >= (u27_FrmMot_Th) ? ((u27_cur_FrmMot)-(u27_FrmMot_Th)) : ((u27_FrmMot_Th)-(u27_cur_FrmMot)) );
		if (pOutput->u8_cur_cadence_Id[motion_T] == _CAD_VIDEO )
			{
			if (u27_cur_FrmMot<1000 && motion_diff<200)
			{
				u1_FrmSeq = (u1_FrmSeq == 0) ? 1 : u1_FrmSeq ;
				u1_FrmSeq_for_mixmode = (u1_FrmSeq_for_mixmode == 0) ? 1 : u1_FrmSeq_for_mixmode;
				//pOutput->u27_ipme_motionPool[motion_T][0] = pOutput->u27_quit_smlMot[motion_T];
			}
		}	
	}

	//for 32322 #183
	if (pOutput->u8_cur_cadence_Id[motion_T] == _CAD_32322)
	{
		if(u1_FrmSeq == 0 && (pOutput->u27_ipme_motionPool[motion_T][0] > (u27_FrmMot_Th>>1)) && (u27_cur_FrmMot > u27_FrmMot_min)){
			u1_FrmSeq = (u1_FrmSeq == 0) ? 1 : u1_FrmSeq;
			u1_FrmSeq_for_mixmode = (u1_FrmSeq_for_mixmode == 0) ? 1 : u1_FrmSeq_for_mixmode;
		}
	}
#endif
	motion_diff = u27_cur_FrmMot - u27_FrmMot_Th;
	
	//small motion check
	if(u27_Mot_all_min_smallmot_check_en==1)
	{
		motion_diff = u27_cur_FrmMot >= u27_FrmMot_Th ? (u27_cur_FrmMot-u27_FrmMot_Th) : (u27_FrmMot_Th-u27_cur_FrmMot);
		if (pOutput->u8_cur_cadence_Id[motion_T] == _CAD_VIDEO )
		{
			if (u27_cur_FrmMot<1000 && motion_diff<200)
			{
				u1_FrmSeq = (u1_FrmSeq == 0) ? 1 : u1_FrmSeq;
				u1_FrmSeq_for_mixmode = (u1_FrmSeq_for_mixmode == 0) ? 1 : u1_FrmSeq_for_mixmode;
				//pOutput->u27_ipme_motionPool[motion_T][0] = pOutput->u27_quit_smlMot[motion_T];
			}
		}
	}

	// 2nd judge for seq = 0
	if(u1_FrmSeq == 0 && u27_cur_FrmMot > 0 && motion_T == _FILM_ALL){
		unsigned int motion_diff_2 = 0;
		unsigned int ratio = 0;
		motion_diff_2 = (u27_FrmMot_Th>u27_cur_FrmMot) ? u27_FrmMot_Th - u27_cur_FrmMot : u27_cur_FrmMot - u27_FrmMot_Th;
		ratio = (u27_FrmMot_Th>0) ? (motion_diff_2*100/u27_FrmMot_Th) : 0;
		if(ratio == 0){
		}
		else if(ratio<10){
			u1_FrmSeq = 1;
			if(log_en){
				rtd_pr_memc_notice("[%s][%d][%d]\n\r", __FUNCTION__, __LINE__, motion_T);
			}
		}
		else if((motion_T == _FILM_ALL && pOutput->u8_cur_cadence_Id[motion_T] == _CAD_VIDEO) || (motion_T != _FILM_ALL) || (test_en2)){
		//else if(pOutput->u8_cur_cadence_Id[motion_T] == _CAD_VIDEO){
		//else if((motion_T == _FILM_ALL && pOutput->u8_cur_cadence_Id[motion_T] == _CAD_VIDEO) || (motion_T != _FILM_ALL)){
			//if((ratio*ratio)<=(8*14*14) && u27_FrmMot_Th > 0 && u27_cur_FrmMot>u27_FrmMot_min){  //it's LG patch
			if((ratio*ratio)<=(8*12*12) && u27_FrmMot_Th > 0 && u27_cur_FrmMot>u27_FrmMot_min){    //YE Test new modify to avoid Cadence 22 detect issue
				u1_FrmSeq = 1;
				if(log_en){
					rtd_pr_memc_notice("[%s][%d][%d][,%d,%d,%d,]\n\r", __FUNCTION__, __LINE__, motion_T, motion_diff_2, u27_FrmMot_Th, u27_cur_FrmMot);
				}
			}				
		}
	}

	pOutput->u32_Mot_sequence[motion_T] = (pOutput->u32_Mot_sequence[motion_T] << 1) | u1_FrmSeq;
	pOutput->u32_Mot_sequence_for_mixmode = (pOutput->u32_Mot_sequence_for_mixmode << 1) | u1_FrmSeq_for_mixmode;

       // record small motion^M
       if(pOutput->u32_Mot_sequence[motion_T] == 0xffffffff){
               pOutput->u27_FrmMotion_S[motion_T] = pOutput->u27_ipme_motionPool[motion_T][0];
       }
       else if(pOutput->u32_Mot_sequence[motion_T] == 0x0){
               pOutput->u27_FrmMotion_S[motion_T] = pOutput->u27_ipme_motionPool[motion_T][0];
       }
       else if(u1_FrmSeq==0){
               pOutput->u27_FrmMotion_S[motion_T] = (pOutput->u27_FrmMotion_S[motion_T]+pOutput->u27_ipme_motionPool[motion_T][0])/2;
       }
       pOutput->u27_FrmMotion_S[motion_T] = _CLIP_UBOUND_(pOutput->u27_FrmMotion_S[motion_T], 0x10000);

	if (motion_T == pOutput->u8_dbg_motion_t)
	{
#if 0
		ROSPrintf_MEMC_IP31_interface3("[MEMC]%s::rbFM=%d, cFM=%d, th=%d, mseq=%d, ph=%d, qTh=%d, Cad_all=%d\n",__FUNCTION__,
					s_pContext->_output_read_comreg.u27_ipme_aMot_rb,
					u27_cur_FrmMot,u27_FrmMot_Th,(pOutput->u32_Mot_sequence[motion_T]&0xFFFF),
					pOutput->u8_phT_phase_Idx[motion_T],pOutput->u27_quit_motTh[motion_T],pOutput->u8_pre_cadence_Id[_FILM_ALL]);
#endif
	}

}

unsigned char CadenceDetect_QSwitch_Judge(const _PARAM_FilmDetectCtrl *pParam, _OUTPUT_FilmDetectCtrl * pOutput, unsigned char motion_T)
{
	// long time black bg + video switch to cadence
	const _PQLCONTEXT *s_pContext = GetPQLContext();

	unsigned char u1_func_en = pParam->u1_QuickSwitch_En;

	unsigned char u1_is_still = 0;
	unsigned char u1_is_sdtl = 0;
	unsigned char u1_is_sapl = 0;
	unsigned char u1_is_qSwitch = 0;

	static unsigned char u8_still_sapl_sdtl_bg_cnt = 0;

	//unsigned int u32_RB_val1 = 0, u32_RB_val2 = 0;
	//ReadRegister(FRC_TOP__PQL_1__pql_fb_dummy_a8_ADDR, 0, 31, &u32_RB_val1);	
	//ReadRegister(SOFTWARE_SOFTWARE_43_reg, 0, 31, &u32_RB_val2);	

	unsigned char u8_sm_th = 0x8;// (u32_RB_val1&0xff);//
	unsigned int u25_sdtl_th = 0x100000;// ((u32_RB_val2>>0)&0xffff) << 4;//
	unsigned int u26_sapl_th = 0x100000;// ((u32_RB_val2>>16)&0xffff) << 4 ;// 960x540x2 = 0xfd200

	unsigned char u8_cnt_max = 0x20;// (u32_RB_val1>>8)&0xff;//
	unsigned char u8_cnt_th = 0x8;// (u32_RB_val1>>16)&0xff;//



	if(u1_func_en == 0 || motion_T!=_FILM_ALL){
		return 0;
	}

	if(pOutput->u27_ipme_motionPool[motion_T][0] < u8_sm_th){
		u1_is_still = 1;
	}

	if(s_pContext->_output_read_comreg.u25_me_aDTL_rb < u25_sdtl_th){
		u1_is_sdtl = 1;
	}

	if((s_pContext->_output_read_comreg.u26_me_aAPLi_rb + s_pContext->_output_read_comreg.u26_me_aAPLp_rb)/2 < u26_sapl_th){
		u1_is_sapl = 1;
	}



	if(u1_is_still && u1_is_sdtl && u1_is_sapl){
		u8_still_sapl_sdtl_bg_cnt++;
	}
	else if(u8_still_sapl_sdtl_bg_cnt > 0){
		u8_still_sapl_sdtl_bg_cnt = u8_still_sapl_sdtl_bg_cnt - 1;
	}
	else{
		u8_still_sapl_sdtl_bg_cnt = 0;
	}


	u8_still_sapl_sdtl_bg_cnt = _CLIP_UBOUND_(u8_still_sapl_sdtl_bg_cnt, u8_cnt_max);

	u1_is_qSwitch = (u8_still_sapl_sdtl_bg_cnt > u8_cnt_th) ? 1 : 0;

#if 0//debug
	WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 0, 3, u1_is_qSwitch);
	WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 4, 7, u1_is_still);
	WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 8, 11, u1_is_sdtl);
	WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 12, 15, u1_is_sapl);
	WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 16, 23, u8_still_sapl_sdtl_bg_cnt);

	if ((pParam->u3_DbgPrt_Rgn == 0) && (((pOutput->u5_dbg_param1_pre & BIT_0) >> 0) == 1) && pOutput->u32_dbg_cnt_hold > 0)
    	{
		rtd_pr_memc_info("[MEMC]CadDet_QSwitch::qs=%d, still=%d, sdtl=%d, sapl=%d, cnt=%d\n",	u1_is_qSwitch,
																			u1_is_still,
																			u1_is_sdtl,
																			u1_is_sapl,
																			u8_still_sapl_sdtl_bg_cnt);
       }
#endif


	return u1_is_qSwitch;

}

unsigned char u8_enter_th_offset[_FRC_CADENCE_NUM_]={0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};


VOID CadenceDetect(const _PARAM_FilmDetectCtrl *pParam, _OUTPUT_FilmDetectCtrl *pOutput, unsigned char motion_T)
{
	unsigned char u8_i = 0, u8_j = 0;
	unsigned char  u1_match_flag = 0, u8_enter_th = 0, u8_len = 0;
	char  s8_Phase_Idx = 0;
	unsigned int u32_match_seq_sft = 0, u32_match_seq_sft2 = 0;
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	unsigned char u8_preCad_Id = 0;
	unsigned int test_en = 0, log_en = 0, log_en2 = 0, log_en3 = 0, test_32322_BadEdit_en = 0, u8_temp_enter_th_offset=0, u8_temp_enter_th_cadence=0;
	unsigned short u16_vip_source_type = 0;
	ReadRegister(SOFTWARE3_SOFTWARE3_09_reg,  9,  9, &log_en3);
	ReadRegister(SOFTWARE3_SOFTWARE3_09_reg,  8,  8, &log_en2);
	ReadRegister(SOFTWARE1_SOFTWARE1_60_reg, 31, 31, &test_en);
	ReadRegister(SOFTWARE1_SOFTWARE1_60_reg, 30, 30, &log_en);
	ReadRegister(SOFTWARE1_SOFTWARE1_60_reg, 29, 29, &test_32322_BadEdit_en);	
	ReadRegister(SOFTWARE1_SOFTWARE1_60_reg, 0, 3, &u8_temp_enter_th_cadence);	
	ReadRegister(SOFTWARE1_SOFTWARE1_60_reg, 4, 7, &u8_temp_enter_th_offset);
	
	u8_preCad_Id      = pOutput->u8_pre_cadence_Id[motion_T];
	u16_vip_source_type = s_pContext->_external_info.u16_source_type;

//	unsigned char u1_qSwitch_cond = CadenceDetect_QSwitch_Judge(pParam, pOutput, motion_T);

	pOutput->u8_det_cadence_Id[motion_T] = _CAD_VIDEO;

	for (u8_i = 1; u8_i < _FRC_CADENCE_NUM_; u8_i ++)
	{
		if(!((pParam->u32_filmDet_cadence_en >> (u8_i-1)) & 1) && u8_i!=_CAD_1225_T1 && u8_i!=_CAD_1225_T2 && u8_i!=_CAD_12214)
		{
			continue;
		}	

		u8_len = (u8_i == _CAD_22) ? frc_cadTable[u8_i].cad_len/2 :
			     (u8_i == _CAD_44) ? frc_cadTable[u8_i].cad_len/2 :
			     (u8_i == _CAD_55) ? frc_cadTable[u8_i].cad_len/2 :
			     (u8_i == _CAD_66) ? frc_cadTable[u8_i].cad_len/2 : frc_cadTable[u8_i].cad_len;
		s8_Phase_Idx = u8_len - 1;

		// if match?
		u1_match_flag = 0;
		u32_match_seq_sft = frc_cadTable[u8_i].cad_seq;
		for (u8_j = 0; u8_j < frc_cadTable[u8_i].cad_len; u8_j ++)
		{
			u32_match_seq_sft = (u32_match_seq_sft>>1) | ((u32_match_seq_sft&0x1)<<(frc_cadTable[u8_i].cad_len-1));
			s8_Phase_Idx = s8_Phase_Idx - 1;
			if(s8_Phase_Idx < 0)
			{
				s8_Phase_Idx = u8_len - 1;
			}

			if ((pOutput->u32_Mot_sequence[motion_T] & frc_cadTable[u8_i].cad_check) == u32_match_seq_sft)
			{
				u1_match_flag = 1;
				break;
			}
		}

		// statistics of enter count
		#if 0
		u8_enter_th = (u8_i == _CAD_22)? ((pParam->u4_enter_22_cntGain-1) * frc_cadTable[u8_i].cad_len + 1):
			          (u8_i == _CAD_32)? ((pParam->u4_enter_32_cntGain-1) * frc_cadTable[u8_i].cad_len + 1):
			          (u8_i == _CAD_11i23) ? frc_cadTable[u8_i].cad_len :	((pParam->u4_enter_else_cntGain-1) * frc_cadTable[u8_i].cad_len + 1) ;
		#else // K2 use.
		#if CONFIG_MC_8_BUFFER
		u8_enter_th = (u8_i == _CAD_22) ? pParam->u4_enter_22_cntGain*frc_cadTable[u8_i].cad_len+2 : // 2*4+2 = 10
					(u8_i == _CAD_32) ? (pParam->u4_enter_32_cntGain*frc_cadTable[u8_i].cad_len)-3 : // (3*5)-3=12
					(u8_i == _CAD_32322) ? (frc_cadTable[u8_i].cad_len-1) : // 12 -1 = 11
					(u8_i == _CAD_11i23) ? 2://frc_cadTable[u8_i].cad_len+4 : // 25+4=29, for "038_FastMotion_KimYuna"
					(u8_i == _CAD_1225_T1) ? 1 : //frc_cadTable[u8_i].cad_len-17 :
					(u8_i == _CAD_1225_T2) ? 1 : //frc_cadTable[u8_i].cad_len-17 :
					(u8_i == _CAD_12214) ? frc_cadTable[u8_i].cad_len : 
					(u8_i == _CAD_44 || u8_i==_CAD_55 || u8_i == _CAD_66) ? 3 * frc_cadTable[u8_i].cad_len/2 :
					(u8_i == _CAD_2224) ? (2 * frc_cadTable[u8_i].cad_len)-9 : // 18 -> 11
					(u8_i == _CAD_1112) ? frc_cadTable[u8_i].cad_len+2 :
					(u8_i == _CAD_11112) ? frc_cadTable[u8_i].cad_len : 
					pParam->u4_enter_else_cntGain*frc_cadTable[u8_i].cad_len ;
		#else
		u8_enter_th = (u8_i == _CAD_22) ? pParam->u4_enter_22_cntGain*frc_cadTable[u8_i].cad_len+2 : // 2*4+2 = 10
					(u8_i == _CAD_32) ? (pParam->u4_enter_32_cntGain*frc_cadTable[u8_i].cad_len)-3 : // (3*5)-3=12
					(u8_i == _CAD_32322) ? (frc_cadTable[u8_i].cad_len-1) : // 12 -1 = 11
					(u8_i == _CAD_11i23) ? frc_cadTable[u8_i].cad_len+4 : // 25+4=29, for "038_FastMotion_KimYuna"
					(u8_i == _CAD_1225_T1) ? 1 : //frc_cadTable[u8_i].cad_len-17 :
					(u8_i == _CAD_1225_T2) ? 1 : //frc_cadTable[u8_i].cad_len-17 :
					(u8_i == _CAD_12214) ? frc_cadTable[u8_i].cad_len : 
					(u8_i == _CAD_44 || u8_i==_CAD_55 || u8_i == _CAD_66) ? 3 * frc_cadTable[u8_i].cad_len/2 :
					(u8_i == _CAD_2224) ? (2 * frc_cadTable[u8_i].cad_len)-2 :
					(u8_i == _CAD_1112) ? frc_cadTable[u8_i].cad_len+2 : 
					(u8_i == _CAD_11112) ? frc_cadTable[u8_i].cad_len : 
					pParam->u4_enter_else_cntGain*frc_cadTable[u8_i].cad_len ;
		#endif

		if( (u16_vip_source_type >= 24) && (u16_vip_source_type<=33)){//add for DTV cadence detection th tuning
			if(u8_temp_enter_th_cadence!=0xf){
				
				if(u8_temp_enter_th_cadence==0){
					u8_temp_enter_th_offset=u8_enter_th_offset[u8_i];
				}else{//for debug and tuning!
					if(u8_i==u8_temp_enter_th_cadence){
						//u8_temp_enter_th_offset=u8_temp_enter_th_offset;
					}else{
						u8_temp_enter_th_offset=u8_enter_th_offset[u8_i];
					}
				}
				u8_enter_th = u8_enter_th+u8_temp_enter_th_offset;
			}
		}
		
		#endif

#if 0
		// dynamic control enter_th if switch from video to others
		if(pOutput->u8_pre_cadence_Id[motion_T] == _CAD_VIDEO && u1_qSwitch_cond){
			u8_enter_th =  (u8_i == _CAD_32 ) ? 1 : u8_enter_th;
			u8_enter_th =  (u8_i == _CAD_32322) ? (frc_cadTable[u8_i].cad_len - 1) : u8_enter_th;
		}

#endif
		if(test_en){
			if(u8_preCad_Id == _CAD_22){
				#if CONFIG_MC_8_BUFFER
				u8_enter_th = (u8_i == _CAD_32) ? 8 : 
					(u8_i == _CAD_22224) ? (2 * frc_cadTable[u8_i].cad_len)-4 : u8_enter_th;
				#else
				u8_enter_th = (u8_i == _CAD_32) ? 8 : u8_enter_th;
				#endif
			}
			else if(u8_preCad_Id == _CAD_32){
				#if CONFIG_MC_8_BUFFER
				u8_enter_th = (u8_i == _CAD_22224) ? (2 * frc_cadTable[u8_i].cad_len)-4 : u8_enter_th;
				#else
				#endif
			}
		}


#if 1// avoid 32322 to 32 seq easily (bad edit)   // YE Test cadence 1029
	#if 0
		if(u32_Log_en==TRUE && motion_T == _FILM_ALL) 
		rtd_pr_memc_info("[CadenceDetect]===[%d] [%d] [%x] [%x] [%x]",
		pOutput->u8_pre_cadence_Id[motion_T],
		u8_filmCad_enterCnt[_CAD_32][motion_T],
		((pOutput->u32_Mot_sequence[motion_T] ) & frc_cadTable[u8_i].cad_check),
		frc_cadTable[u8_i].cad_check,
		u32_match_seq_sft );
	#endif	
	#if 0
		if(pOutput->u8_pre_cadence_Id[_FILM_ALL] == _CAD_32322	
			&& u8_filmCad_enterCnt[_CAD_32322][_FILM_ALL] >= u8_enter_th
			//&& (pOutput->u32_Mot_sequence[motion_T] & 0x6b) == 0//identity sequence (10010100)
			&& (((pOutput->u32_Mot_sequence[_FILM_ALL] >> 1) & 0xFFF) == 0x94A)//torlerate 1 frame bad edit
			&& pOutput->u8_det_cadence_Id[_FILM_ALL] == _CAD_32)//32 sequence also greater than threshold 
		{
			u1_match_flag = 1;
		}

		if(pOutput->u8_pre_cadence_Id[motion_T] == _CAD_32	 //tv002 new issue #5#6#7
		&& u8_filmCad_enterCnt[_CAD_32][motion_T] >= u8_enter_th
		&& (((pOutput->u32_Mot_sequence[motion_T] ) & frc_cadTable[_CAD_32].cad_check) == 0x15))	
		{
			u1_match_flag = 1;
		#if 0	
			if(u32_Log_en==TRUE && motion_T == _FILM_ALL) 
			rtd_pr_memc_info("[YE]=== OK_1");
		#endif	

		if (pOutput->u8_pre_cadence_Id[motion_T] == _CAD_32	//tv002 new issue #5#6#7
		&& u8_filmCad_enterCnt[_CAD_32][motion_T] >= u8_enter_th
		&& (((pOutput->u32_Mot_sequence[motion_T] ) & 0xF) == 0xA))//torlerate 1 frame bad edit
		{
			u8_filmCad_enterCnt[_CAD_32322][motion_T] =0;
		#if 0	
			if(u32_Log_en==TRUE && motion_T == _FILM_ALL) 
			rtd_pr_memc_info("[YE]=== OK_2");
		#endif	

		}
	#endif	
	#if 1
		if( (pOutput->u8_pre_cadence_Id[motion_T] == _CAD_VIDEO) && (u8_filmCad_enterCnt[_CAD_2224][motion_T] >= u8_enter_th)
			&& (u8_filmCad_enterCnt[_CAD_22][motion_T] >= 5) && (pOutput->u8_det_cadence_Id[motion_T] == _CAD_2224) )//for 42222 quick cange to 22 just 1 frame.
		{
			u8_enter_th = u8_enter_th+1;
		#if 0	
			if(log_en==TRUE && motion_T == _FILM_ALL) 
			rtd_pr_memc_info("[YE]=== OK_2");
		#endif	
		}
		else if((pOutput->u8_pre_cadence_Id[motion_T] == _CAD_32)&&(u8_filmCad_enterCnt[_CAD_32][motion_T] >= u8_enter_th)
			&&(u8_filmCad_enterCnt[_CAD_22][motion_T] >= 1)&&(u8_filmCad_enterCnt[_CAD_32322][motion_T] >= 3)
			&&(((pOutput->u32_Mot_sequence[motion_T])& 0xFF) == 0xAB) && (pOutput->u8_det_cadence_Id[motion_T] == _CAD_32))//for 32->00->32 more smooth when badedit, delay the cadence change timing	
		{
			u1_match_flag =1;
		#if 0	
			if(log_en==TRUE && motion_T == _FILM_ALL) 
			rtd_pr_memc_info("[YE]=== OK_2");
		#endif			
		}
	#endif
#endif

		u8_filmCad_enterCnt[u8_i][motion_T] = (u1_match_flag == 1)? _CLIP_(u8_filmCad_enterCnt[u8_i][motion_T] + 1, 0, u8_enter_th + 10) : 0;

		// ex. for _CAD_32, judge cadence detect successful if motionSeq == 10010.
		//u1_match_flag_def = (pOutput->u32_Mot_sequence[motion_T] & frc_cadTable[u8_i].cad_check) == frc_cadTable[u8_i].cad_seq? 1 : 0;
		if (u8_filmCad_enterCnt[u8_i][motion_T] >= u8_enter_th)
		{
			//if(u8_i == _CAD_321)
				//pOutput->u8_det_cadence_Id[motion_T] = _CAD_22;
			//else
				pOutput->u8_det_cadence_Id[motion_T] = u8_i;
			if(log_en2) {
				rtd_printk(KERN_CRIT, TAG_NAME_MEMC, "[INFO] det:%d, Flag:%d, Cnt:%d, th:%d\n", 
				pOutput->u8_det_cadence_Id[motion_T], u1_match_flag, u8_filmCad_enterCnt[u8_i][motion_T], u8_enter_th);
			}
		}

		#if 1
		if(log_en && motion_T == _FILM_ALL){
			rtd_pr_memc_notice("[CadenceDetect][%d][cnt,%d,][th,%d,][det_cad,%d,]\n\r", u8_i, u8_filmCad_enterCnt[u8_i][motion_T], u8_enter_th, pOutput->u8_det_cadence_Id[motion_T]);
		}
		#endif
	}

	if(test_32322_BadEdit_en && motion_T == _FILM_ALL){
		pOutput->u1_32322_BadEdit_flag = 0;
		pOutput->u1_32322_BadEdit_flag2 = 0;
		pOutput->u1_Special_60_BadEdit_flag = 0;
		pOutput->u1_Special_50_BadEdit_flag = 0;
		pOutput->u1_Special_30_BadEdit_flag = 0;
		for(u8_i = 0; u8_i < 60; u8_i++){
			if ((pOutput->u32_Mot_sequence[motion_T] & 0x3fffffff) == u32_32322_BadEdit_case[u8_i])
			{
				pOutput->u1_32322_BadEdit_flag = 1;
				pOutput->u8_32322_BadEdit_idx = u8_i;
				break;
			}
		}

#if Pro_tv010
		if(s_pContext->_external_data._input_frameRate == _PQL_IN_60HZ) {
			for(u8_i = 0; u8_i < u32_Special_60_cnt; u8_i++){
				if ((pOutput->u32_Mot_sequence[motion_T] & 0xffffffff) == u32_Special_60_BadEdit_case[u8_i])
				{
					pOutput->u1_Special_60_BadEdit_flag = 1;
					pOutput->u8_Special_60_BadEdit_idx = u8_i;
					break;
				}
			}
		}
		else if(s_pContext->_external_data._input_frameRate == _PQL_IN_50HZ) {
			for(u8_i = 0; u8_i < u32_Special_50_cnt; u8_i++){
				if ((pOutput->u32_Mot_sequence[motion_T] & 0xffffffff) == u32_Special_50_BadEdit_case[u8_i])
				{
					pOutput->u1_Special_50_BadEdit_flag = 1;
					pOutput->u8_Special_50_BadEdit_idx = u8_i;
					break;
				}
			}
		}
		else if(s_pContext->_external_data._input_frameRate == _PQL_IN_30HZ) {
			for(u8_i = 0; u8_i < u32_Special_30_cnt; u8_i++){
				if ((pOutput->u32_Mot_sequence[motion_T] & 0xffffffff) == u32_Special_30_BadEdit_case[u8_i])
				{
					pOutput->u1_Special_30_BadEdit_flag = 1;
					pOutput->u8_Special_30_BadEdit_idx = u8_i;
					break;
				}
			}
		}
#endif
		u1_match_flag = 0;
		u32_match_seq_sft = 0x2295; // "423221"  0x114a; // "42322"
		u32_match_seq_sft2 = 0x2515; // "324221"  0x128a; // "32422"
		u32_match_seq_sft = (u32_match_seq_sft>>1) | ((u32_match_seq_sft&0x1)<<(14-1));
		u32_match_seq_sft2 = (u32_match_seq_sft2>>1) | ((u32_match_seq_sft2&0x1)<<(14-1));
		if(((pOutput->u32_Mot_sequence[motion_T] & 0x1fff) == u32_match_seq_sft) || ((pOutput->u32_Mot_sequence[motion_T] & 0x1fff) == u32_match_seq_sft2))
		{
			pOutput->u1_32322_BadEdit_flag2 = 1;
		}

		pOutput->u8_32322_BadEdit_cnt = (pOutput->u1_32322_BadEdit_flag == 1) ? _CLIP_(pOutput->u8_32322_BadEdit_cnt + 1, 0, 120) : 
										(pOutput->u8_32322_BadEdit_cnt > 1) ? (pOutput->u8_32322_BadEdit_cnt - 1) : 0;
		pOutput->u8_32322_BadEdit_cnt2 = (pOutput->u1_32322_BadEdit_flag2 == 1) ? _CLIP_(pOutput->u8_32322_BadEdit_cnt2 + 39, 0, 200) : 
										(pOutput->u8_32322_BadEdit_cnt2 > 1) ? (pOutput->u8_32322_BadEdit_cnt2 - 1) : 0;

		pOutput->u8_Special_60_BadEdit_cnt = (pOutput->u1_Special_60_BadEdit_flag == 1) ? _CLIP_(pOutput->u8_Special_60_BadEdit_cnt + 1, 0, 170) : 
												(pOutput->u8_Special_60_BadEdit_cnt > 1) ? (pOutput->u8_Special_60_BadEdit_cnt - 1) : 0;
		pOutput->u8_Special_50_BadEdit_cnt = (pOutput->u1_Special_50_BadEdit_flag == 1) ? _CLIP_(pOutput->u8_Special_50_BadEdit_cnt + 1, 0, 120) : 
												(pOutput->u8_Special_50_BadEdit_cnt > 1) ? (pOutput->u8_Special_50_BadEdit_cnt - 1) : 0;
		pOutput->u8_Special_30_BadEdit_cnt = (pOutput->u1_Special_30_BadEdit_flag == 1) ? ((pOutput->u8_Special_30_BadEdit_cnt >= 50) ?  _CLIP_(pOutput->u8_Special_30_BadEdit_cnt+2, 0, 102) : _CLIP_(pOutput->u8_Special_30_BadEdit_cnt+1, 0, 102) ) : 
												(pOutput->u8_Special_30_BadEdit_cnt > 1) ? (pOutput->u8_Special_30_BadEdit_cnt - 1) : 0;

		if(log_en && motion_T == _FILM_ALL){
//			rtd_pr_memc_notice("[32322_BadEdit][cnt1,%d,idx1,%d,][cnt2,%d,]\n\r", pOutput->u8_32322_BadEdit_cnt, pOutput->u8_32322_BadEdit_idx, pOutput->u8_32322_BadEdit_cnt2);
		}
		if(log_en3 && motion_T == _FILM_ALL) { //d924[9]
		rtd_pr_memc_crit("[Detect cadence] Mot:%d%d%d%d%d%d%d%d%d%d%d%d flag=[%d %d %d] cnt=[%d %d %d]\n",
			(pOutput->u32_Mot_sequence[_FILM_ALL]>>11) & 0x1,
			(pOutput->u32_Mot_sequence[_FILM_ALL]>>10) & 0x1,
			(pOutput->u32_Mot_sequence[_FILM_ALL]>>9) & 0x1,
			(pOutput->u32_Mot_sequence[_FILM_ALL]>>8) & 0x1,
			(pOutput->u32_Mot_sequence[_FILM_ALL]>>7) & 0x1,
			(pOutput->u32_Mot_sequence[_FILM_ALL]>>6) & 0x1,
			(pOutput->u32_Mot_sequence[_FILM_ALL]>>5) & 0x1,
			(pOutput->u32_Mot_sequence[_FILM_ALL]>>4) & 0x1,
			(pOutput->u32_Mot_sequence[_FILM_ALL]>>3) & 0x1,
			(pOutput->u32_Mot_sequence[_FILM_ALL]>>2) & 0x1,
			(pOutput->u32_Mot_sequence[_FILM_ALL]>>1) & 0x1,
			pOutput->u32_Mot_sequence[_FILM_ALL] & 0x1,
			pOutput->u1_Special_60_BadEdit_flag,pOutput->u1_Special_50_BadEdit_flag,pOutput->u1_Special_30_BadEdit_flag,
			pOutput->u8_Special_60_BadEdit_cnt,pOutput->u8_Special_50_BadEdit_cnt,pOutput->u8_Special_30_BadEdit_cnt);
		}
	}
	else if(test_32322_BadEdit_en == 0){
		pOutput->u1_32322_BadEdit_flag = 0;
		pOutput->u1_32322_BadEdit_flag2 = 0;
		pOutput->u8_32322_BadEdit_cnt = 0;
		pOutput->u8_32322_BadEdit_cnt2 = 0;
		pOutput->u1_Special_60_BadEdit_flag = 0;
		pOutput->u8_Special_60_BadEdit_cnt = 0;
		pOutput->u1_Special_50_BadEdit_flag = 0;
		pOutput->u8_Special_50_BadEdit_cnt = 0;
		pOutput->u1_Special_30_BadEdit_flag = 0;
		pOutput->u8_Special_30_BadEdit_cnt = 0;
	}

	if ((motion_T == pOutput->u8_dbg_motion_t) && (((pOutput->u5_dbg_param1_pre & BIT_1) >> 1) == 1) && pOutput->u32_dbg_cnt_hold > 0)
	{
		rtd_pr_memc_info("[MEMC]%s::det_cadence_Id=%d %d \n",__FUNCTION__,pOutput->u8_det_cadence_Id[motion_T],_FRC_CADENCE_NUM_);
	}

}

unsigned char Film_PreFrm_MotWeight_autoGen(unsigned int u27_PreMot, unsigned int u27_CurMot)
{
	unsigned char u5_PreWgt          = 28;

	u27_CurMot  = u27_CurMot + 1;
	u27_PreMot  = u27_PreMot + 1;

	u5_PreWgt   = u27_PreMot * 1   > u27_CurMot? 12 : \
		         (u27_PreMot * 128 < u27_CurMot? 31 : \
			   	 (u27_PreMot * 64  < u27_CurMot? 30 : \
			   	 (u27_PreMot * 32  < u27_CurMot? 29 : \
			   	 (u27_PreMot * 16  < u27_CurMot? 28 : \
			   	 (u27_PreMot * 8   < u27_CurMot? 26 : \
			   	 (u27_PreMot * 4   < u27_CurMot? 24 : \
			   	 (u27_PreMot * 2   < u27_CurMot? 20 : \
			   	 (u27_PreMot * 1   < u27_CurMot? 18 : 28))))))));

	return u5_PreWgt;
}

unsigned int Film_rgnSMot_Avg(const _PARAM_FilmDetectCtrl *pParam, _OUTPUT_FilmDetectCtrl *pOutput, unsigned char motion_T)
{
	unsigned int u27_rgnSMot_avg = pOutput->u27_quit_smlMot[motion_T];

	switch(motion_T)
	{
		case _FILM_5RGN_TOP:
			u27_rgnSMot_avg = (pOutput->u27_quit_smlMot[_FILM_5RGN_BOT] + pOutput->u27_quit_smlMot[_FILM_5RGN_CEN])/2;
			break;
		case _FILM_5RGN_BOT:
			u27_rgnSMot_avg = (pOutput->u27_quit_smlMot[_FILM_5RGN_TOP] + pOutput->u27_quit_smlMot[_FILM_5RGN_CEN])/2;
			break;
		case _FILM_5RGN_LFT:
			u27_rgnSMot_avg = (pOutput->u27_quit_smlMot[_FILM_5RGN_RHT] + pOutput->u27_quit_smlMot[_FILM_5RGN_CEN])/2;
			break;
		case _FILM_5RGN_RHT:
			u27_rgnSMot_avg = (pOutput->u27_quit_smlMot[_FILM_5RGN_LFT] + pOutput->u27_quit_smlMot[_FILM_5RGN_CEN])/2;
			break;
		default:
			break;
	}



	return u27_rgnSMot_avg;
	
}

VOID Film_motThr_calc(const _PARAM_FilmDetectCtrl *pParam, _OUTPUT_FilmDetectCtrl *pOutput, unsigned char motion_T)
{
	unsigned char  u8_preCad_id           = pOutput->u8_pre_cadence_Id[motion_T] < _FRC_CADENCE_NUM_ ? pOutput->u8_pre_cadence_Id[motion_T] : _CAD_VIDEO;
	unsigned char  u1_curCad_motFlag      = 0; //cadence_seqFlag(frc_cadTable[u8_preCad_id], pOutput->u8_phase_Idx[motion_T]);
	unsigned char  u1_curFrm_isBig        = 0; //u8_preCad_id != _CAD_VIDEO? (u1_curCad_motFlag == 1? 1 : 0) : ((pOutput->u32_Mot_sequence[motion_T] & 0x01) ==1? 1 : 0);

	unsigned char  u5_preFrm_adapt_big_motWgt = 0;
	unsigned char  u5_preFrm_adapt_sml_motWgt = 0;
	unsigned int u27_pre_bigMot         = pOutput->u27_quit_bigMot[motion_T];
	unsigned int u27_pre_smlMot         = pOutput->u27_quit_smlMot[motion_T];
	unsigned int u27_curFrm_mot         = pOutput->u27_ipme_motionPool[motion_T][0];

	unsigned short u16_SMot_Th = 256;
	unsigned char u8_SMot_ratio = 20;

	if(u8_preCad_id == _CAD_32322){
		u1_curCad_motFlag  = cadence_seqFlag(frc_cadTable[u8_preCad_id], ((pOutput->u8_phase_Idx[motion_T]+6)%12));
	}else{	
		u1_curCad_motFlag  = cadence_seqFlag(frc_cadTable[u8_preCad_id], pOutput->u8_phase_Idx[motion_T]);
	}

	u1_curFrm_isBig = u8_preCad_id != _CAD_VIDEO? (u1_curCad_motFlag == 1? 1 : 0):((pOutput->u32_Mot_sequence[motion_T] & 0x01) ==1? 1 : 0);

	// motion IIR
/*	u5_preFrm_adapt_motWgt = (pParam->u1_quit_motWgt_auto_en == 0)? 28 : \
		                      Film_PreFrm_MotWeight_autoGen(pOutput->u27_quit_motTh[motion_T], u27_curFrm_mot); // u0.5*/
	u5_preFrm_adapt_big_motWgt = (pParam->u1_quit_motWgt_auto_en == 0)? 28 : \
                 Film_PreFrm_MotWeight_autoGen(pOutput->u27_quit_bigMot[motion_T], u27_curFrm_mot); // u0.5

	u5_preFrm_adapt_sml_motWgt = (pParam->u1_quit_motWgt_auto_en == 0)? 28 : \
                 Film_PreFrm_MotWeight_autoGen(pOutput->u27_quit_smlMot[motion_T], u27_curFrm_mot); // u0.5


	if (u1_curFrm_isBig == 1)
	{
		pOutput->u27_quit_bigMot[motion_T] = _2Val_RndBlending_(u27_pre_bigMot, u27_curFrm_mot, u5_preFrm_adapt_big_motWgt, 5);
	}
	else
	{
		pOutput->u27_quit_smlMot[motion_T] = _2Val_RndBlending_(u27_pre_smlMot, u27_curFrm_mot, u5_preFrm_adapt_sml_motWgt, 5);
	}

	pOutput->u27_quit_motTh[motion_T] = (pOutput->u27_quit_bigMot[motion_T] * pParam->u4_quit_bigMot_wgt + \
										 pOutput->u27_quit_smlMot[motion_T] * pParam->u4_quit_smlMot_wgt) >> 3;

	//for mixmode@1112/11112, 1112/11112 is not easy to quit.
	if( motion_T!=_FILM_ALL && motion_T!=_FILM_5RGN_CEN 
		&& (pOutput->u8_pre_cadence_Id[_FILM_5RGN_CEN]==_CAD_1112 ||pOutput->u8_pre_cadence_Id[_FILM_5RGN_CEN]==_CAD_11112))   
	{
		unsigned int u27_rgnSmot_avg = Film_rgnSMot_Avg(pParam, pOutput, motion_T);
		unsigned int u27_bMot_mapping = (pOutput->u27_quit_bigMot[motion_T] > 8*pOutput->u27_quit_smlMot[motion_T]) ? (pOutput->u27_quit_bigMot[motion_T]/8) : 
								(pOutput->u27_quit_bigMot[motion_T] > 4*pOutput->u27_quit_smlMot[motion_T]) ? (pOutput->u27_quit_bigMot[motion_T]/4) : 
								(pOutput->u27_quit_bigMot[motion_T]/2);

		if(u27_rgnSmot_avg < u16_SMot_Th && pOutput->u27_quit_smlMot[motion_T] > u8_SMot_ratio*(1+u27_rgnSmot_avg))
		{
			pOutput->u27_quit_motTh[motion_T] = (u27_bMot_mapping * _MIN_(1, pParam->u4_quit_bigMot_wgt/2) + \
											u27_rgnSmot_avg* pParam->u4_quit_smlMot_wgt) >> 3;
		}

	}
	//--

	if ((motion_T == pOutput->u8_dbg_motion_t) && (((pOutput->u5_dbg_param1_pre & BIT_4) >> 4) == 1) && pOutput->u32_dbg_cnt_hold > 0)
	{
		rtd_pr_memc_info("[MEMC]%s::q_mTh=%d, q_bM=%d, q_sM=%d\n",__FUNCTION__,pOutput->u27_quit_motTh[motion_T],\
			pOutput->u27_quit_bigMot[motion_T], pOutput->u27_quit_smlMot[motion_T]);
	}

}

VOID Film_occasionally_fastOut(const _PARAM_FilmDetectCtrl *pParam, _OUTPUT_FilmDetectCtrl *pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	unsigned char u8_curFrm_det_cadId = pOutput->u8_det_cadence_Id[_FILM_ALL];
	unsigned char u8_preFrm_cadId     = pOutput->u8_pre_cadence_Id[_FILM_ALL];
	unsigned int test_32322_BadEdit_en = 0;
	unsigned char length = 0, u8_i = 0, debug_info[7] = {0};
	unsigned int u32_match_seq_sft = 0,  log_en = 0;
	bool u1_PreframeIsVideo = true;
	bool u1_checkCad11i23 = false;
	static bool CadVideoHist[12];
	static unsigned char u8_Cad11i23_cnt = 0;
	static unsigned char u8_Cad1225_cnt_out = 0;
	static unsigned char u8_Cad1225_cnt_in = 0;
	static unsigned char u8_Cad1225_T2_cnt_out = 0;
	static unsigned char u8_Cad1225_T2_cnt_in = 0;
	static unsigned char u8_Cnt_22in11i23 = 0;
	static unsigned char u8_Cnt_22to11i23 = 0;

	ReadRegister(SOFTWARE3_SOFTWARE3_09_reg, 10, 10, &log_en);
	ReadRegister(SOFTWARE1_SOFTWARE1_60_reg, 29, 29, &test_32322_BadEdit_en);

	if(CAD_F ==PQL_FRCCaseSupportJudge(u8_curFrm_det_cadId))//not support 
		return ;

	for(u8_i=0; u8_i<12; u8_i++){
		if(CadVideoHist[u8_i]==false) {
			u1_PreframeIsVideo = false;
			break;
		}
	}	

	length = frc_cadTable[u8_curFrm_det_cadId].cad_len;
	u32_match_seq_sft = frc_cadTable[u8_curFrm_det_cadId].cad_seq;
	pOutput->u1_cad_fastOut_true  = 0;
	#if CONFIG_MC_8_BUFFER
	// for output 50HZ case, 22 case will apply 11i23 setting
	if(u1_PreframeIsVideo==true && s_pContext->_external_data._input_frameRate == _PQL_IN_50HZ 
		&& u8_preFrm_cadId==_CAD_VIDEO && u8_curFrm_det_cadId==_CAD_22 /*&& Cad1225_cnt!=0*/)
	{
		u8_Cad11i23_cnt = 100;//150
		u1_checkCad11i23 = true;
	}
	else if(s_pContext->_external_data._input_frameRate != _PQL_IN_50HZ)
	{
		u8_Cad11i23_cnt = 0;
		u1_checkCad11i23 = false;
	}
	else if(u8_Cad11i23_cnt == 0)
	{
		u1_checkCad11i23 = false;
	}

	if(u8_preFrm_cadId == _CAD_1225_T1)
	{
		if(u8_curFrm_det_cadId==_CAD_1225_T1){
			u8_Cad1225_cnt_out=0;
		}
		else {
			u8_Cad1225_cnt_out = (u8_Cad1225_cnt_out>=255) ? 255 : u8_Cad1225_cnt_out+1;
		}
	}
	else {
		u8_Cad1225_cnt_out=0;
	}
	if(u8_preFrm_cadId == _CAD_1225_T2)
	{
		if(u8_curFrm_det_cadId==_CAD_1225_T2){
			u8_Cad1225_T2_cnt_out=0;
		}
		else {
			u8_Cad1225_T2_cnt_out = (u8_Cad1225_T2_cnt_out>=255) ? 255 : u8_Cad1225_T2_cnt_out+1;
		}
	}
	else {
		u8_Cad1225_T2_cnt_out=0;
	}
	if( u8_curFrm_det_cadId==_CAD_1225_T1 && u8_preFrm_cadId!=_CAD_1225_T1){
		u8_Cad1225_cnt_in = (u8_Cad1225_cnt_in>=255) ? 255 : u8_Cad1225_cnt_in+1;
	}
	else {
		u8_Cad1225_cnt_in = (u8_Cad1225_cnt_in==0) ? 0 : u8_Cad1225_cnt_in-1;
	}
	if( u8_curFrm_det_cadId==_CAD_1225_T2 && u8_preFrm_cadId!=_CAD_1225_T2){
		u8_Cad1225_T2_cnt_in = (u8_Cad1225_T2_cnt_out>=255) ? 255 : u8_Cad1225_T2_cnt_out+1;
	}
	else {
		u8_Cad1225_T2_cnt_in = (u8_Cad1225_T2_cnt_out==0) ? 0 : u8_Cad1225_T2_cnt_out-1;
	}

	u8_Cad11i23_cnt = (u8_Cad11i23_cnt==0) ? 0 : u8_Cad11i23_cnt-1;
	u8_Cnt_22to11i23 = (u8_Cnt_22to11i23==0) ? 0 : u8_Cnt_22to11i23-1;
	if(u8_Cad11i23_cnt==0){
		u8_Cnt_22in11i23 = 0;
	}
//	rtd_printk(KERN_NOTICE, TAG_NAME_MEMC, "[FO] pre:%d  curID:%d  cnt:%d  ch:%d cnt_O:%d cnt_I:%d\n",
//	u8_preFrm_cadId, u8_curFrm_det_cadId, u8_Cad11i23_cnt, u1_checkCad11i23, u8_Cad1225_cnt_out, u8_Cad1225_cnt_in );

	debug_info[0] = u8_preFrm_cadId;
	debug_info[1] = u8_curFrm_det_cadId;
	debug_info[2] = u8_Cad11i23_cnt;
	debug_info[3] = u1_checkCad11i23;
	debug_info[4] = u8_Cad1225_T2_cnt_out;
	debug_info[5] = u8_Cad1225_T2_cnt_in;
	debug_info[6] = 0;
	
	if(pOutput->ExchangeCad==true)
	{
		rtd_printk(KERN_WARNING, TAG_NAME_MEMC, "[fastout exchange]\n" );
	}
	else if(u1_checkCad11i23==true && u8_Cad11i23_cnt>0)
	{
		debug_info[6] = 100;
			if(u8_curFrm_det_cadId==_CAD_1225_T1){
			debug_info[6] = 108;
				pOutput->u8_cur_cadence_Id[_FILM_ALL] = _CAD_1225_T1;
				pOutput->u1_cad_fastOut_true          = 1;
			u8_Cad11i23_cnt =0;
			}
			else if(u8_curFrm_det_cadId==_CAD_1225_T2){
			debug_info[6] = 109;
				pOutput->u8_cur_cadence_Id[_FILM_ALL] = _CAD_1225_T2;
				pOutput->u1_cad_fastOut_true          = 1;
			u8_Cad11i23_cnt =0;
			}
			else if(u8_curFrm_det_cadId==_CAD_11i23){
			debug_info[6] = 101;
				pOutput->u8_cur_cadence_Id[_FILM_ALL] = _CAD_11i23;
				pOutput->u1_cad_fastOut_true          = 1;
			//u8_Cad11i23_cnt =0;
			}
		else if(u8_Cnt_22in11i23>=3){
			debug_info[6] = 103;
				pOutput->u8_cur_cadence_Id[_FILM_ALL] = _CAD_22;
				pOutput->u1_cad_fastOut_true		  = 1;
			u8_Cad11i23_cnt =0;
			u8_Cnt_22in11i23 = 0;
			}
		else if(u8_curFrm_det_cadId==_CAD_22 && u8_Cad11i23_cnt<75){
			debug_info[6] = 102;
				//pOutput->u8_cur_cadence_Id[_FILM_ALL] = _CAD_22;
				//pOutput->u1_cad_fastOut_true		  = 1;
			//u8_Cad11i23_cnt =0;
			u8_Cnt_22in11i23 = (u8_Cnt_22in11i23>=255) ? 255 : u8_Cnt_22in11i23+1;
			}
			else {
			debug_info[6] = 110;
				//pOutput->u8_cur_cadence_Id[_FILM_ALL] = _CAD_11i23;
				//pOutput->u1_cad_fastOut_true		  = 1;
			//u8_Cad11i23_cnt =0;
			}
	}
	else if(u8_preFrm_cadId != _CAD_1225_T1 && u8_curFrm_det_cadId==_CAD_1225_T1)
	{
		debug_info[6] = 2;
		pOutput->u8_cur_cadence_Id[_FILM_ALL] = _CAD_1225_T1;
		pOutput->u1_cad_fastOut_true		  = 1;
		u8_Cad11i23_cnt =0;
	}
	else if(u8_preFrm_cadId != _CAD_1225_T2 && u8_curFrm_det_cadId==_CAD_1225_T2)
	{
		debug_info[6] = 3;
		pOutput->u8_cur_cadence_Id[_FILM_ALL] = _CAD_1225_T2;
		pOutput->u1_cad_fastOut_true		  = 1;
		u8_Cad11i23_cnt =0;
	}
	else if(u8_preFrm_cadId == _CAD_1225_T1)
	{
		debug_info[6] = 4;
		if(u8_Cad1225_cnt_out>125) {
			pOutput->u8_cur_cadence_Id[_FILM_ALL] = u8_curFrm_det_cadId; 
			pOutput->u1_cad_fastOut_true		  = 1;	
			u8_Cad1225_cnt_out=0;
		}
		else {
			pOutput->u8_cur_cadence_Id[_FILM_ALL] = _CAD_1225_T1; 
	//		pOutput->u1_cad_fastOut_true		  = 1;	
		}
	}
	else if(u8_preFrm_cadId == _CAD_1225_T2)
	{
		debug_info[6] = 5;
		if(u8_Cad1225_T2_cnt_out>125) {
			pOutput->u8_cur_cadence_Id[_FILM_ALL] = u8_curFrm_det_cadId; 
			pOutput->u1_cad_fastOut_true		  = 1;	
			u8_Cad1225_T2_cnt_out=0;
		}
		else {
			pOutput->u8_cur_cadence_Id[_FILM_ALL] = _CAD_1225_T2; 
	//		pOutput->u1_cad_fastOut_true		  = 1;	
		}
	}
	else if(u8_preFrm_cadId == _CAD_11i23)
	{
		debug_info[6] = 6;
		if (u8_curFrm_det_cadId == _CAD_1225_T1)
		{
			pOutput->u8_cur_cadence_Id[_FILM_ALL] = _CAD_1225_T1; 
			pOutput->u1_cad_fastOut_true          = 1;  
		}
		else if (u8_curFrm_det_cadId == _CAD_1225_T2)
		{
			pOutput->u8_cur_cadence_Id[_FILM_ALL] = _CAD_1225_T2; 
			pOutput->u1_cad_fastOut_true          = 1;  
		}
	}
	else if(u8_Cad1225_cnt_in>25) {
		debug_info[6] = 7;
		pOutput->u8_cur_cadence_Id[_FILM_ALL] = _CAD_1225_T1; 
		pOutput->u1_cad_fastOut_true		  = 1;	
		u8_Cad1225_cnt_in = 0;
	}
	else if(u8_Cad1225_T2_cnt_in>25) {
		debug_info[6] = 8;
		pOutput->u8_cur_cadence_Id[_FILM_ALL] = _CAD_1225_T2; 
		pOutput->u1_cad_fastOut_true		  = 1;	
		u8_Cad1225_T2_cnt_in = 0;
	}
	else if (u8_preFrm_cadId == _CAD_32) // rule 1: 32->22 / 32322 / 3223
	{
		debug_info[6] = 51;
		if (/*u8_curFrm_det_cadId == _CAD_32322 ||*/ u8_curFrm_det_cadId == _CAD_3223 || u8_curFrm_det_cadId == _CAD_55)
		{
//			pOutput->u8_pre_cadence_Id[_FILM_ALL] = _CAD_VIDEO;
			pOutput->u8_cur_cadence_Id[_FILM_ALL] = _CAD_VIDEO;
			pOutput->u1_cad_fastOut_true          = 1;
		}
		else if(u8_curFrm_det_cadId == _CAD_22 && ((pOutput->u32_Mot_sequence[_FILM_ALL]&((1<<length)-1)) == u32_match_seq_sft))
		{
			pOutput->u8_cur_cadence_Id[_FILM_ALL] = _CAD_22;
			pOutput->u1_cad_fastOut_true          = 1;
		}
		else if(u8_curFrm_det_cadId == _CAD_32322 && u8_filmCad_enterCnt[_CAD_32322][_FILM_ALL]> 16
			&& ((pOutput->u32_Mot_sequence[_FILM_ALL]&((1<<length)-1)) == (((u32_match_seq_sft&((1<<1)-1))<<(length-1))|(u32_match_seq_sft>>1))))
		{
			pOutput->u8_cur_cadence_Id[_FILM_ALL] = _CAD_32322;
			pOutput->u1_cad_fastOut_true          = 1;
		}
		else if(u8_curFrm_det_cadId == _CAD_2224 /*&& u8_filmCad_enterCnt[_CAD_2224][_FILM_ALL]>= 19*/
			 && ((pOutput->u32_Mot_sequence[_FILM_ALL]&((1<<length)-1)) == (((u32_match_seq_sft&((1<<4)-1))<<(length-4))|(u32_match_seq_sft>>4))))
		{
			pOutput->u8_cur_cadence_Id[_FILM_ALL] = _CAD_2224;
			pOutput->u1_cad_fastOut_true          = 1;
		}
		else if(u8_curFrm_det_cadId == _CAD_22224 /*&& u8_filmCad_enterCnt[_CAD_2224][_FILM_ALL]>= 19*/
			 && ((pOutput->u32_Mot_sequence[_FILM_ALL]&((1<<length)-1)) == (((u32_match_seq_sft&((1<<4)-1))<<(length-4))|(u32_match_seq_sft>>4))))
		{
			pOutput->u8_cur_cadence_Id[_FILM_ALL] = _CAD_22224;
			pOutput->u1_cad_fastOut_true          = 1;
		}
		else if(u8_curFrm_det_cadId == _CAD_11112 && ((pOutput->u32_Mot_sequence[_FILM_ALL]&((1<<length)-1)) == u32_match_seq_sft))
		{
			pOutput->u8_cur_cadence_Id[_FILM_ALL] = _CAD_11112;
			pOutput->u1_cad_fastOut_true          = 1;
		}
	}
	else if (u8_preFrm_cadId == _CAD_22) // 22 -> 32 / 44 / 22224 / 2224 / 11'23
	{
		debug_info[6] = 80;
		if(s_pContext->_output_frc_sceneAnalysis.u1_Swing_true ==1 )  //  small motion
		{
			debug_info[6] = 81;
			if (u8_curFrm_det_cadId == _CAD_44  || u8_curFrm_det_cadId == _CAD_11i23)
			{
				pOutput->u8_cur_cadence_Id[_FILM_ALL] = _CAD_VIDEO; 
				pOutput->u1_cad_fastOut_true          = 1;  
				debug_info[6] = 82;
			}
		}
		else if(u8_curFrm_det_cadId == _CAD_11i23 /*&& ((pOutput->u32_Mot_sequence[_FILM_ALL]&((1<<length)-1)) == u32_match_seq_sft)*/)
		{
			debug_info[6] = 83;
			u8_Cnt_22to11i23 = (u8_Cnt_22to11i23>=253) ? 255 : u8_Cnt_22to11i23+2;
			if(u8_Cnt_22to11i23>6){
				debug_info[6] = 183;
				pOutput->u8_cur_cadence_Id[_FILM_ALL] = _CAD_11i23;
				pOutput->u1_cad_fastOut_true          = 1;
			}
		}
		else if (u8_curFrm_det_cadId == _CAD_44 || u8_curFrm_det_cadId == _CAD_66)
		{
			debug_info[6] = 84;
//			pOutput->u8_pre_cadence_Id[_FILM_ALL] = _CAD_VIDEO;
			pOutput->u8_cur_cadence_Id[_FILM_ALL] = _CAD_VIDEO;
			pOutput->u1_cad_fastOut_true          = 1;
		}
		else if(u8_curFrm_det_cadId == _CAD_321)
		{
			pOutput->u8_cur_cadence_Id[_FILM_ALL] = _CAD_321;
			pOutput->u1_cad_fastOut_true          = 1;
		}
		else if(u8_curFrm_det_cadId == _CAD_32 && ((pOutput->u32_Mot_sequence[_FILM_ALL]&((1<<length)-1)) == u32_match_seq_sft))
		{
			pOutput->u8_cur_cadence_Id[_FILM_ALL] = _CAD_32;
 			pOutput->u1_cad_fastOut_true          = 1;
 		}
		else if(u8_curFrm_det_cadId == _CAD_22224 && ((pOutput->u32_Mot_sequence[_FILM_ALL]&((1<<length)-1)) == (((u32_match_seq_sft&((1<<4)-1))<<(length-4))|(u32_match_seq_sft>>4))))
		{
			pOutput->u8_cur_cadence_Id[_FILM_ALL] = _CAD_22224;
 			pOutput->u1_cad_fastOut_true          = 1;
		}
		else if(u8_curFrm_det_cadId == _CAD_2224 && (((pOutput->u32_Mot_sequence[_FILM_ALL]&((1<<length)-1)) == (((u32_match_seq_sft&((1<<4)-1))<<(length-4))|(u32_match_seq_sft>>4))) || ( s_pContext->_output_wrt_comreg.u1_BTS_Rp_flag!=1)))
		{
			pOutput->u8_cur_cadence_Id[_FILM_ALL] = _CAD_2224;
 			pOutput->u1_cad_fastOut_true          = 1;
 		}
	}
	else if (u8_preFrm_cadId == _CAD_2224)
	{
		debug_info[6] = 52;
		if (u8_curFrm_det_cadId == _CAD_22 && ((pOutput->u32_Mot_sequence[_FILM_ALL]&((1<<length)-1)) == u32_match_seq_sft))
		{
			pOutput->u8_cur_cadence_Id[_FILM_ALL] = _CAD_22;
			pOutput->u1_cad_fastOut_true          = 1;
		}
		else if(u8_curFrm_det_cadId == _CAD_32 && ((pOutput->u32_Mot_sequence[_FILM_ALL]&((1<<length)-1)) == u32_match_seq_sft)/*&& u8_filmCad_enterCnt[_CAD_32][_FILM_ALL]> 16*/)
		{
			pOutput->u8_cur_cadence_Id[_FILM_ALL] = _CAD_32;
			pOutput->u1_cad_fastOut_true          = 1;
		}
		else if(u8_curFrm_det_cadId == _CAD_22224 && ((pOutput->u32_Mot_sequence[_FILM_ALL]&((1<<length)-1)) == (((u32_match_seq_sft&((1<<6)-1))<<(length-6))|(u32_match_seq_sft>>6))))
		{
			pOutput->u8_cur_cadence_Id[_FILM_ALL] = _CAD_22224;
 			pOutput->u1_cad_fastOut_true          = 1;
		}
		else if(u8_curFrm_det_cadId == _CAD_32322 && ((pOutput->u32_Mot_sequence[_FILM_ALL]&((1<<length)-1)) == u32_match_seq_sft)) // (((u32_match_seq_sft&((1<<1)-1))<<(length-1))|(u32_match_seq_sft>>1))
		{
			pOutput->u8_cur_cadence_Id[_FILM_ALL] = _CAD_32322;
			pOutput->u1_cad_fastOut_true          = 1;
		}
	} 
	else if (u8_preFrm_cadId == _CAD_22224)
	{
		debug_info[6] = 53;
		if (u8_curFrm_det_cadId == _CAD_22 && ((pOutput->u32_Mot_sequence[_FILM_ALL]&((1<<length)-1)) == u32_match_seq_sft))
		{
			pOutput->u8_cur_cadence_Id[_FILM_ALL] = _CAD_22;
			pOutput->u1_cad_fastOut_true          = 1;
		}
		else if(u8_curFrm_det_cadId == _CAD_32 && ((pOutput->u32_Mot_sequence[_FILM_ALL]&((1<<length)-1)) == (((u32_match_seq_sft&((1<<1)-1))<<(length-1))|(u32_match_seq_sft>>1)))/*&& u8_filmCad_enterCnt[_CAD_32][_FILM_ALL]> 16*/)
		{
			pOutput->u8_cur_cadence_Id[_FILM_ALL] = _CAD_32;
			pOutput->u1_cad_fastOut_true          = 1;
		}
		else if(u8_curFrm_det_cadId == _CAD_2224 && ((pOutput->u32_Mot_sequence[_FILM_ALL]&((1<<length)-1)) == (((u32_match_seq_sft&((1<<4)-1))<<(length-4))|(u32_match_seq_sft>>4))))
		{
			pOutput->u8_cur_cadence_Id[_FILM_ALL] = _CAD_2224;
 			pOutput->u1_cad_fastOut_true          = 1;
 		}
		else if(u8_curFrm_det_cadId == _CAD_32322 && ((pOutput->u32_Mot_sequence[_FILM_ALL]&((1<<length)-1)) == u32_match_seq_sft)) // (((u32_match_seq_sft&((1<<1)-1))<<(length-1))|(u32_match_seq_sft>>1))
		{
			pOutput->u8_cur_cadence_Id[_FILM_ALL] = _CAD_32322;
			pOutput->u1_cad_fastOut_true          = 1;
		}
	} 
	else if (u8_preFrm_cadId == _CAD_11112) // 11112 -> 22. 11i23,32322,22224, not easy to quit
	{
		debug_info[6] = 54;
		if (u8_curFrm_det_cadId == _CAD_22 || u8_curFrm_det_cadId == _CAD_11i23 || u8_curFrm_det_cadId == _CAD_32322 || \
			u8_curFrm_det_cadId == _CAD_22224 || u8_curFrm_det_cadId == _CAD_2224 || u8_curFrm_det_cadId == _CAD_55)
		{
//			pOutput->u8_pre_cadence_Id[_FILM_ALL] = _CAD_VIDEO;
			pOutput->u8_cur_cadence_Id[_FILM_ALL] = _CAD_VIDEO;
			pOutput->u1_cad_fastOut_true          = 1;
		}
	}
	else	if(u8_preFrm_cadId == _CAD_1112)// not easy to quit
	{
		if (u8_curFrm_det_cadId == _CAD_32 ||u8_curFrm_det_cadId == _CAD_3223 || u8_curFrm_det_cadId == _CAD_122 || \
			u8_curFrm_det_cadId == _CAD_55||u8_curFrm_det_cadId == _CAD_2224)
		{
//			pOutput->u8_pre_cadence_Id[_FILM_ALL] = _CAD_VIDEO;
			pOutput->u8_cur_cadence_Id[_FILM_ALL] = _CAD_VIDEO;
			pOutput->u1_cad_fastOut_true          = 1;
		}
	}
	else if(u8_preFrm_cadId == _CAD_3223)
	{
		debug_info[6] = 55;
		if (u8_curFrm_det_cadId == _CAD_55)
		{
//			pOutput->u8_pre_cadence_Id[_FILM_ALL] = _CAD_VIDEO;
			pOutput->u8_cur_cadence_Id[_FILM_ALL] = _CAD_VIDEO;
			pOutput->u1_cad_fastOut_true          = 1;
		}
	}
	else if (u8_preFrm_cadId == _CAD_122)
	{
		debug_info[6] = 56;
		if (u8_curFrm_det_cadId == _CAD_32)
		{
			pOutput->u8_cur_cadence_Id[_FILM_ALL] = _CAD_VIDEO;
			pOutput->u1_cad_fastOut_true          = 1;
		}
		//else if(u8_curFrm_det_cadId == _CAD_11i23)
		//{
		//need optimize
		//	rtd_printk(KERN_CRIT, TAG_NAME_MEMC, "[EXAM] motion:%d, Len:%d, sft:%d\n", pOutput->u32_Mot_sequence[_FILM_ALL], length, u32_match_seq_sft);
		//	pOutput->u8_cur_cadence_Id[_FILM_ALL] = _CAD_11i23;
		//	pOutput->u1_cad_fastOut_true		  = 1;
		//}
		//else if(u8_curFrm_det_cadId == _CAD_22)
		//{
		//need optimize
		//	rtd_printk(KERN_CRIT, TAG_NAME_MEMC, "[EXAM] motion:%d, Len:%d, sft:%d\n", pOutput->u32_Mot_sequence[_FILM_ALL], length, u32_match_seq_sft);
		//	pOutput->u8_cur_cadence_Id[_FILM_ALL] = _CAD_22;
		//	pOutput->u1_cad_fastOut_true		  = 1;
		//}
	}
	else if (u8_preFrm_cadId == _CAD_334)
	{
		if (u8_curFrm_det_cadId == _CAD_66)
		{
			pOutput->u8_cur_cadence_Id[_FILM_ALL] = _CAD_VIDEO;
			pOutput->u1_cad_fastOut_true          = 1;
		}
	}
	// 32322 bad-edit adjust, for quick switch 32
#if 1	
	else if(u8_preFrm_cadId == _CAD_32322)
	{
		unsigned char u8_cad_enter_cnt_th =  frc_cadTable[_CAD_32322].cad_outLen + 4 - frc_cadTable[_CAD_32].cad_outLen;
		if(test_32322_BadEdit_en){
			u8_cad_enter_cnt_th = u8_cad_enter_cnt_th + 2;
		}
		if(u8_curFrm_det_cadId == _CAD_32 && u8_filmCad_enterCnt[_CAD_32][_FILM_ALL]> 21 /*u8_cad_enter_cnt_th*/
			&& ((pOutput->u32_Mot_sequence[_FILM_ALL]&((1<<length)-1)) == (((u32_match_seq_sft&((1<<2)-1))<<(length-2))|(u32_match_seq_sft>>2))))
		{
			//pOutput->u8_pre_cadence_Id[_FILM_ALL] = _CAD_VIDEO;
			//pOutput->u8_cur_cadence_Id[_FILM_ALL] = _CAD_VIDEO;
			pOutput->u8_cur_cadence_Id[_FILM_ALL] = _CAD_32;
			pOutput->u1_cad_fastOut_true          = 1;
		}
		else if(u8_curFrm_det_cadId == _CAD_22224 && ((pOutput->u32_Mot_sequence[_FILM_ALL]&((1<<length)-1)) == (((u32_match_seq_sft&((1<<4)-1))<<(length-4))|(u32_match_seq_sft>>4))))
		{
			pOutput->u8_cur_cadence_Id[_FILM_ALL] = _CAD_22224;
 			pOutput->u1_cad_fastOut_true          = 1;
		}
		else if(u8_curFrm_det_cadId == _CAD_2224 && ((pOutput->u32_Mot_sequence[_FILM_ALL]&((1<<length)-1)) == (((u32_match_seq_sft&((1<<4)-1))<<(length-4))|(u32_match_seq_sft>>4))))
		{
			pOutput->u8_cur_cadence_Id[_FILM_ALL] = _CAD_2224;
 			pOutput->u1_cad_fastOut_true          = 1;
 		}
	}
#endif	
	else if (u8_preFrm_cadId == _CAD_321) // 321 -> 22 
	{
		if(u8_curFrm_det_cadId == _CAD_22)
		{
			pOutput->u8_cur_cadence_Id[_FILM_ALL] = _CAD_22;
			pOutput->u1_cad_fastOut_true          = 1;
		}
	}
	#else
	if (u8_preFrm_cadId == _CAD_32) // rule 1: 32->22 / 32322 / 3223
	{
		debug_info[6] = 60;
		if (/*u8_curFrm_det_cadId == _CAD_32322 ||*/ u8_curFrm_det_cadId == _CAD_3223 || u8_curFrm_det_cadId == _CAD_55)
		{
//			pOutput->u8_pre_cadence_Id[_FILM_ALL] = _CAD_VIDEO;
			pOutput->u8_cur_cadence_Id[_FILM_ALL] = _CAD_VIDEO;
			pOutput->u1_cad_fastOut_true          = 1;
		}
		else if(u8_curFrm_det_cadId == _CAD_22)
		{
			pOutput->u8_cur_cadence_Id[_FILM_ALL] = _CAD_22;
			pOutput->u1_cad_fastOut_true          = 1;
		}
		else if(u8_curFrm_det_cadId == _CAD_32322 && u8_filmCad_enterCnt[_CAD_32322][_FILM_ALL]> 16)
		{
			pOutput->u8_cur_cadence_Id[_FILM_ALL] = _CAD_32322;
			pOutput->u1_cad_fastOut_true          = 1;
		}
		else if(u8_curFrm_det_cadId == _CAD_2224 && u8_filmCad_enterCnt[_CAD_2224][_FILM_ALL]>= 19)
		{
			pOutput->u8_cur_cadence_Id[_FILM_ALL] = _CAD_2224;
			pOutput->u1_cad_fastOut_true          = 1;
		}
	}
	else if (u8_preFrm_cadId == _CAD_22) // 22 -> 32 / 44 / 22224 / 2224 / 11'23
	{
		debug_info[6]=20;
		if(s_pContext->_output_frc_sceneAnalysis.u1_Swing_true ==1 )  //  small motion
		{
			debug_info[6]=21;
			if (u8_curFrm_det_cadId == _CAD_44  || u8_curFrm_det_cadId == _CAD_11i23)
			{
				pOutput->u8_cur_cadence_Id[_FILM_ALL] = _CAD_VIDEO; 
				pOutput->u1_cad_fastOut_true          = 1;  
				debug_info[6]=22;
			}
		}
		else if (u8_curFrm_det_cadId == _CAD_44 || u8_curFrm_det_cadId == _CAD_11i23 || u8_curFrm_det_cadId == _CAD_66)
		{
//			pOutput->u8_pre_cadence_Id[_FILM_ALL] = _CAD_VIDEO;
			pOutput->u8_cur_cadence_Id[_FILM_ALL] = _CAD_VIDEO;
			pOutput->u1_cad_fastOut_true          = 1;
			debug_info[6]=23;
		}
		else if(u8_curFrm_det_cadId == _CAD_321)
		{
			pOutput->u8_cur_cadence_Id[_FILM_ALL] = _CAD_321;
			pOutput->u1_cad_fastOut_true          = 1;
		}
		else if(u8_curFrm_det_cadId == _CAD_32)
		{
			pOutput->u8_cur_cadence_Id[_FILM_ALL] = _CAD_32;
 			pOutput->u1_cad_fastOut_true          = 1;
 		}
		else if(u8_curFrm_det_cadId == _CAD_22224)
		{
			pOutput->u8_cur_cadence_Id[_FILM_ALL] = _CAD_22224;
 			pOutput->u1_cad_fastOut_true          = 1;
		}
		else if(u8_curFrm_det_cadId == _CAD_2224)
		{
			pOutput->u8_cur_cadence_Id[_FILM_ALL] = _CAD_2224;
 			pOutput->u1_cad_fastOut_true          = 1;
 		}
	}
	else if (u8_preFrm_cadId == _CAD_2224)
	{
		if (u8_curFrm_det_cadId == _CAD_22)
		{
			pOutput->u8_cur_cadence_Id[_FILM_ALL] = _CAD_22;
			pOutput->u1_cad_fastOut_true          = 1;
		}
		else if(u8_curFrm_det_cadId == _CAD_32 /*&& u8_filmCad_enterCnt[_CAD_32][_FILM_ALL]> 16*/)
		{
			pOutput->u8_cur_cadence_Id[_FILM_ALL] = _CAD_32;
			pOutput->u1_cad_fastOut_true          = 1;
		}
	} 
	else if (u8_preFrm_cadId == _CAD_11112) // 11112 -> 22. 11i23,32322,22224, not easy to quit
	{
		if (u8_curFrm_det_cadId == _CAD_22 || u8_curFrm_det_cadId == _CAD_11i23 || u8_curFrm_det_cadId == _CAD_32322 || \
			u8_curFrm_det_cadId == _CAD_22224 || u8_curFrm_det_cadId == _CAD_2224 || u8_curFrm_det_cadId == _CAD_55)
		{
//			pOutput->u8_pre_cadence_Id[_FILM_ALL] = _CAD_VIDEO;
			pOutput->u8_cur_cadence_Id[_FILM_ALL] = _CAD_VIDEO;
			pOutput->u1_cad_fastOut_true          = 1;
		}
	}
	else	if(u8_preFrm_cadId == _CAD_1112)// not easy to quit
	{
		if (u8_curFrm_det_cadId == _CAD_32 ||u8_curFrm_det_cadId == _CAD_3223 || u8_curFrm_det_cadId == _CAD_122 || \
			u8_curFrm_det_cadId == _CAD_55||u8_curFrm_det_cadId == _CAD_2224)
		{
//			pOutput->u8_pre_cadence_Id[_FILM_ALL] = _CAD_VIDEO;
			pOutput->u8_cur_cadence_Id[_FILM_ALL] = _CAD_VIDEO;
			pOutput->u1_cad_fastOut_true          = 1;
		}
	}
	else if(u8_preFrm_cadId == _CAD_3223)
	{
		if (u8_curFrm_det_cadId == _CAD_55)
		{
//			pOutput->u8_pre_cadence_Id[_FILM_ALL] = _CAD_VIDEO;
			pOutput->u8_cur_cadence_Id[_FILM_ALL] = _CAD_VIDEO;
			pOutput->u1_cad_fastOut_true          = 1;
		}
	}
	else if (u8_preFrm_cadId == _CAD_122)
	{
		if (u8_curFrm_det_cadId == _CAD_32)
		{
			pOutput->u8_cur_cadence_Id[_FILM_ALL] = _CAD_VIDEO;
			pOutput->u1_cad_fastOut_true          = 1;
		}
	}
	else if (u8_preFrm_cadId == _CAD_334)
	{
		if (u8_curFrm_det_cadId == _CAD_66)
		{
			pOutput->u8_cur_cadence_Id[_FILM_ALL] = _CAD_VIDEO;
			pOutput->u1_cad_fastOut_true          = 1;
		}
	}
	// 32322 bad-edit adjust, for quick switch 32
#if 1	
	else if(u8_preFrm_cadId == _CAD_32322)
	{
		unsigned char u8_cad_enter_cnt_th =  frc_cadTable[_CAD_32322].cad_outLen + 4 - frc_cadTable[_CAD_32].cad_outLen;
		if(test_32322_BadEdit_en){
			u8_cad_enter_cnt_th = u8_cad_enter_cnt_th + 2;
		}
		if(u8_curFrm_det_cadId == _CAD_32 && u8_filmCad_enterCnt[_CAD_32][_FILM_ALL]> 21 /*u8_cad_enter_cnt_th*/)
		{
			//pOutput->u8_pre_cadence_Id[_FILM_ALL] = _CAD_VIDEO;
			//pOutput->u8_cur_cadence_Id[_FILM_ALL] = _CAD_VIDEO;
			pOutput->u8_cur_cadence_Id[_FILM_ALL] = _CAD_32;
			pOutput->u1_cad_fastOut_true          = 1;
		}
	}
#endif	
	else if (u8_preFrm_cadId == _CAD_321) // 321 -> 22 
	{
		if(u8_curFrm_det_cadId == _CAD_22)
		{
			pOutput->u8_cur_cadence_Id[_FILM_ALL] = _CAD_22;
			pOutput->u1_cad_fastOut_true          = 1;
		}
	}
	else if (u8_preFrm_cadId == _CAD_55) // 55 -> 22 
	{
		if(u8_curFrm_det_cadId == _CAD_22)
		{
			pOutput->u8_cur_cadence_Id[_FILM_ALL] = _CAD_22;
			pOutput->u1_cad_fastOut_true          = 1;
		}
	}
	else{
		debug_info[6] = 99;
	}
	#endif

	if(log_en){ //d924[10]
	rtd_printk(KERN_CRIT, TAG_NAME_MEMC, "[FO] %d%d%d%d%d%d%d%d%d%d%d%d case:%d pre:%d  curID:%d  cnt:%d  ch:%d cnt_O:%d cnt_I:%d  curID:%d detID:%d T:%d\n",
		(pOutput->u32_Mot_sequence[_FILM_ALL]>>11) & 0x1,
		(pOutput->u32_Mot_sequence[_FILM_ALL]>>10) & 0x1,
		(pOutput->u32_Mot_sequence[_FILM_ALL]>>9) & 0x1,
		(pOutput->u32_Mot_sequence[_FILM_ALL]>>8) & 0x1,
		(pOutput->u32_Mot_sequence[_FILM_ALL]>>7) & 0x1,
		(pOutput->u32_Mot_sequence[_FILM_ALL]>>6) & 0x1,
		(pOutput->u32_Mot_sequence[_FILM_ALL]>>5) & 0x1,
		(pOutput->u32_Mot_sequence[_FILM_ALL]>>4) & 0x1,
		(pOutput->u32_Mot_sequence[_FILM_ALL]>>3) & 0x1,
		(pOutput->u32_Mot_sequence[_FILM_ALL]>>2) & 0x1,
		(pOutput->u32_Mot_sequence[_FILM_ALL]>>1) & 0x1,
		pOutput->u32_Mot_sequence[_FILM_ALL] & 0x1,
		debug_info[6], debug_info[0], debug_info[1], debug_info[2], debug_info[3], debug_info[4], debug_info[5],
		pOutput->u8_cur_cadence_Id[_FILM_ALL], u8_curFrm_det_cadId, pOutput->u1_cad_fastOut_true );
	}

	//to avoid CAD_22 be quit by CAD_32's quit cnt
	if(pOutput->u8_pre_cadence_Id[_FILM_ALL] != pOutput->u8_cur_cadence_Id[_FILM_ALL]){
		pOutput->u8_quit_cnt[_FILM_ALL] = 0;
		pOutput->u8_quit_prd[_FILM_ALL] = 0;
	}

	for(u8_i=0; u8_i<11; u8_i++){
		CadVideoHist[u8_i]=CadVideoHist[u8_i+1];
	}

	if(u8_preFrm_cadId==0&&u8_curFrm_det_cadId==0){
		//lastCadisVideo = true;
		CadVideoHist[11]=true;
	}
	else{
		//lastCadisVideo = false;
		CadVideoHist[11]=false;
	}
}

VOID FilmEnter(const _PARAM_FilmDetectCtrl *pParam, _OUTPUT_FilmDetectCtrl *pOutput, unsigned char motion_T)
{
	unsigned char mot_isSml = 0;
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	unsigned char u8_sys_N = s_pContext->_output_read_comreg.u8_sys_N_rb;
	unsigned char u8_sys_M = s_pContext->_output_read_comreg.u8_sys_M_rb;
	unsigned char det_cadence = 0, length = 0, u8_i=0;
	bool u1_match_seq=0;
	unsigned int u32_seq_cmp = 0, u32_match_tmp = 0, showlog = 0;
	unsigned int u32_match_seq_sft = 0;
	unsigned int log_en = 0, test_en = 0;

	ReadRegister(SOFTWARE1_SOFTWARE1_60_reg, 30, 30, &log_en);
	ReadRegister(SOFTWARE1_SOFTWARE1_62_reg, 0, 0, &test_en);
	ReadRegister(SOFTWARE3_SOFTWARE3_09_reg, 6, 6, &showlog);
	if(motion_T >= _FILM_MOT_NUM)
	{
		rtd_pr_memc_notice("[%s]Invalid Motion_Type:%d!!\n",__FUNCTION__,motion_T);
		motion_T = _FILM_ALL;
	}
	mot_isSml = (pOutput->u27_ipme_motionPool[motion_T][0] < pOutput->u27_quit_motTh[motion_T])? 1 : 0; 
	det_cadence = pOutput->u8_det_cadence_Id[motion_T];
	length = frc_cadTable[det_cadence].cad_len;
	u32_match_seq_sft = frc_cadTable[det_cadence].cad_seq;
	if ((pOutput->u8_det_cadence_Id[motion_T] != _CAD_VIDEO) /*&& mot_isSml*/)
	{
		//u32_match_seq_sft = (u32_match_seq_sft<<1) | ((u32_match_seq_sft<<(length-1))&0x1));

		if(((u8_sys_N == 5 && u8_sys_M == 6) || (u8_sys_N == 5 && u8_sys_M == 12))    //50 to 60  or 25 to 50
			&&((pOutput->u8_det_cadence_Id[motion_T] != _CAD_22) && (pOutput->u8_det_cadence_Id[motion_T] != _CAD_55)))
		{			
			pOutput->u8_cur_cadence_Id[motion_T] = _CAD_VIDEO;
			pOutput->u8_phase_Idx[motion_T] = 0;
			if(showlog && motion_T==_FILM_ALL)
				rtd_printk(KERN_CRIT, TAG_NAME_MEMC, "[FE] 0 %d %d %d\n", pOutput->u8_det_cadence_Id[motion_T], u8_sys_N, u8_sys_M );
		}
		else
		{
			
			#if CONFIG_MC_8_BUFFER
			if(test_en && det_cadence != _CAD_2224 && det_cadence != _CAD_22224 && det_cadence != _CAD_1112 && det_cadence != _CAD_11112
				&& (pOutput->u32_Mot_sequence[motion_T]&((1<<length)-1)) == u32_match_seq_sft)
			{
				pOutput->u8_cur_cadence_Id[motion_T] = pOutput->u8_det_cadence_Id[motion_T];
				pOutput->u8_quit_cnt[motion_T] = 0;
				if(showlog && motion_T==_FILM_ALL)
					rtd_printk(KERN_CRIT, TAG_NAME_MEMC, "[FE] 1 %d\n", pOutput->u8_det_cadence_Id[motion_T] );
			}
			else if(test_en && det_cadence == _CAD_2224
				&& (pOutput->u32_Mot_sequence[motion_T]&((1<<length)-1)) == (((u32_match_seq_sft&((1<<4)-1))<<(length-4))|(u32_match_seq_sft>>4)))
			{
				pOutput->u8_cur_cadence_Id[motion_T] = pOutput->u8_det_cadence_Id[motion_T];
				pOutput->u8_quit_cnt[motion_T] = 0;
				if(showlog && motion_T==_FILM_ALL)
					rtd_printk(KERN_CRIT, TAG_NAME_MEMC, "[FE] 2\n" );
			}
			else if(test_en && det_cadence == _CAD_22224
				&& (pOutput->u32_Mot_sequence[motion_T]&((1<<length)-1)) == (((u32_match_seq_sft&((1<<4)-1))<<(length-4))|(u32_match_seq_sft>>4)))
			{
				pOutput->u8_cur_cadence_Id[motion_T] = pOutput->u8_det_cadence_Id[motion_T];
				pOutput->u8_quit_cnt[motion_T] = 0;
				if(showlog && motion_T==_FILM_ALL)
					rtd_printk(KERN_CRIT, TAG_NAME_MEMC, "[FE] 3 %d\n", pOutput->u8_det_cadence_Id[motion_T] );
			}
#if 1
			else if(test_en && (det_cadence == _CAD_1112 || det_cadence == _CAD_11112) && motion_T==_FILM_ALL )
			{
				u1_match_seq = false;
				u32_seq_cmp = pOutput->u32_Mot_sequence[motion_T]&(((1<<(3*length))-1));
				u32_match_tmp = (u32_match_seq_sft<<(2*length)) + (u32_match_seq_sft<<length) + u32_match_seq_sft;
				for(u8_i=0;u8_i<length;u8_i++){
					if((u32_seq_cmp&((1<<(3*length))-1)) == u32_match_tmp){
						u1_match_seq=true;
						break;
					}
					u32_seq_cmp =((u32_seq_cmp&1)<<(3*length-1)) + (u32_seq_cmp>>1);
				}

				if(u1_match_seq) {
					pOutput->u8_cur_cadence_Id[motion_T] = pOutput->u8_det_cadence_Id[motion_T];
					pOutput->u8_quit_cnt[motion_T] = 0;
					if(showlog && motion_T==_FILM_ALL) {
						rtd_printk(KERN_CRIT, TAG_NAME_MEMC, "[FE] 4 det:%d Mot:%u pre:%d cmp[%d %d %d]\n",
								pOutput->u8_det_cadence_Id[motion_T], pOutput->u32_Mot_sequence[motion_T],
								pOutput->u8_pre_cadence_Id[motion_T], u32_seq_cmp, length, u32_match_tmp );
					}
			}
			else{
				pOutput->u8_cur_cadence_Id[motion_T] = pOutput->u8_pre_cadence_Id[motion_T];
					if(showlog && motion_T==_FILM_ALL) {
						rtd_printk(KERN_CRIT, TAG_NAME_MEMC, "[FE] 5 det:%d Mot:%u pre:%d cmp[%d %d %d]\n",
								pOutput->u8_det_cadence_Id[motion_T], pOutput->u32_Mot_sequence[motion_T],
								pOutput->u8_pre_cadence_Id[motion_T], u32_seq_cmp, length, u32_match_tmp );
					}
				}
			}
#endif
			else{
				if(showlog && motion_T==_FILM_ALL){
					rtd_printk(KERN_CRIT, TAG_NAME_MEMC, "[FE] 6 %u %d %d %u - %u\n",
				pOutput->u32_Mot_sequence[motion_T], det_cadence, length, u32_match_seq_sft, 
				(pOutput->u32_Mot_sequence[motion_T]&((1<<length)-1)) );
				}
				pOutput->u8_cur_cadence_Id[motion_T] = pOutput->u8_pre_cadence_Id[motion_T];
				
			}
			#else
			pOutput->u8_cur_cadence_Id[motion_T] = pOutput->u8_det_cadence_Id[motion_T];
			pOutput->u8_quit_cnt[motion_T] = 0;
			if(showlog && motion_T==_FILM_ALL)
				rtd_printk(KERN_CRIT, TAG_NAME_MEMC, "[FE] F \n" );
			#endif
			//pOutput->u8_phase_Idx[motion_T] = pOutput->u8_Calc_phase_Idx[motion_T];
		}
	}
//	if(motion_T == _FILM_ALL && log_en){
	if(0){
		rtd_pr_memc_notice("[%s][,%d,%d,][,%x,%x,][cur_cadence,%d,]\n\r", __FUNCTION__, 
			det_cadence, length, 
			u32_match_seq_sft, (pOutput->u32_Mot_sequence[motion_T]&((1<<length)-1)), 
			pOutput->u8_cur_cadence_Id[motion_T]);
	}
}

VOID FilmQuit_BadEditPatch(const _PARAM_FilmDetectCtrl *pParam, _OUTPUT_FilmDetectCtrl *pOutput)
{
	unsigned int flag = 0;
	
	unsigned char j = 0;
	unsigned char u8_checkbit = 16;
	unsigned char u8_totalcheck = 0;
	unsigned char u1_match32322_flag = 0;
	unsigned char u1_32322_BadEdit_flag = 0;
	unsigned char u1_1225_BadEdit_flag = 0;
	unsigned int u32_match_data_temp = 0;
	unsigned int test_32322_BadEdit_en = 0;

	ReadRegister(SOFTWARE1_SOFTWARE1_60_reg, 29, 29, &test_32322_BadEdit_en);
	u1_match32322_flag = 0;
	u8_totalcheck = u8_checkbit - 5;
	for(j = 0;j <= u8_totalcheck; j++)
	{
		u32_match_data_temp = ((pOutput->u32_Mot_sequence[_FILM_ALL] & ((1<<u8_checkbit)-1))>>j);// at least check 16bits for 32322
		// confirm 32322 seq to quit
		if ((u32_match_data_temp & frc_cadTable[_CAD_32].cad_check) == 0x15)	// 10101(part of 32322)
		{
			u1_match32322_flag = 1;
		}
		else if((pOutput->u32_Mot_sequence[_FILM_ALL] & (0x7fff<<j)) == 0x4a95		// 100 10 10 100 10 10 1
			|| (pOutput->u32_Mot_sequence[_FILM_ALL] & (0xfff<<j)) == 0x942		// 100 10 100 00 10
			|| (pOutput->u32_Mot_sequence[_FILM_ALL] & (0xfff<<j)) == 0x948		// 100 10 100 10 00
			|| (pOutput->u32_Mot_sequence[_FILM_ALL] & (0xffff<<j)) == 0x9409		// 100 10 100 0000 100 1
			|| (pOutput->u32_Mot_sequence[_FILM_ALL] & (0x1fff<<j)) == 0x152d		// 10 10 100 10 110 1
			|| (pOutput->u32_Mot_sequence[_FILM_ALL] & (0x1fff<<j)) == 0x1297){	// 100 10 100 10 11 1
			u1_32322_BadEdit_flag = 1;
		}
		else if((pOutput->u32_Mot_sequence[_FILM_ALL] & (0x7ffff<<j)) == 0x53329	// 10 100 1 100 1 100 10 100 1
			|| (pOutput->u32_Mot_sequence[_FILM_ALL] & (0x7fffff<<j)) == 0x5332A9	// 10 100 1 100 1 100 10 10 10 100 1
			|| (pOutput->u32_Mot_sequence[_FILM_ALL] & (0xffffff<<j)) == 0xA66545	// 10 100 1 100 1 100 10 10 1000 10	1
			|| (pOutput->u32_Mot_sequence[_FILM_ALL] & (0x7fffff<<j)) == 0x5332AD	// 10 100 1 100 1 100 10 10 10 1 10	1
			|| (pOutput->u32_Mot_sequence[_FILM_ALL] & (0x7fff<<j)) == 0x5335){// 10 100 1 100 1 10 10 1
			u1_1225_BadEdit_flag = 1;
		}
	}

	if(pOutput->u8_pre_cadence_Id[_FILM_ALL] == _CAD_32322	
		&& (u8_filmCad_enterCnt[_CAD_32322][_FILM_ALL] >= 1 || (pOutput->u32_Mot_sequence[_FILM_ALL] & 0x3f) == 0)
		&& (u8_filmCad_enterCnt[_CAD_32][_FILM_ALL] <= u8_totalcheck || u1_match32322_flag == 1) // avoid 32seq
		&& (pParam->u1_BadEditSlowOut_En == 1))
	{
		pOutput->u8_quit_cnt[_FILM_ALL] = (pOutput->u8_quit_cnt[_FILM_ALL] >= 1) ? pOutput->u8_quit_cnt[_FILM_ALL]-1 : 0;
		flag = 1;
	}
	else if((pOutput->u8_pre_cadence_Id[_FILM_ALL] == _CAD_32322) && (test_32322_BadEdit_en) && (u1_32322_BadEdit_flag)){
		pOutput->u8_quit_cnt[_FILM_ALL] = (pOutput->u8_quit_cnt[_FILM_ALL] >= 1) ? pOutput->u8_quit_cnt[_FILM_ALL]-1 : 0;
	}
	else if((pOutput->u8_pre_cadence_Id[_FILM_ALL] == _CAD_1225_T1) && (test_32322_BadEdit_en) && (u1_1225_BadEdit_flag)){
		pOutput->u8_quit_cnt[_FILM_ALL] = (pOutput->u8_quit_cnt[_FILM_ALL] >= 1) ? pOutput->u8_quit_cnt[_FILM_ALL]-1 : 0;
		//rtd_printk(KERN_CRIT, TAG_NAME_MEMC, "[CH] \n");
	}

	//WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 24, 27, flag);
	//WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 28, 31, (unsigned int)u1_match32322_flag);
	
}

unsigned int FilmQuit_QuitCntOffsetCalc(const _PARAM_FilmDetectCtrl *pParam, _OUTPUT_FilmDetectCtrl *pOutput, unsigned char motion_T)
{
	unsigned char det_cadence = 0, cur_cadence = 0;
	unsigned int quitCntOffset = 2;
	static unsigned int quitCntOffset_tmp = 0;

	det_cadence = pOutput->u8_det_cadence_Id[motion_T];
	cur_cadence = pOutput->u8_cur_cadence_Id[motion_T];

	if(cur_cadence == _CAD_32){
		if(det_cadence != cur_cadence){
			if((pOutput->u32_Mot_sequence[motion_T]&0x1f) == 0x1f){ // 1 1 1 1 1
				quitCntOffset_tmp = quitCntOffset_tmp-1;
			}
			else if(pOutput->u27_ipme_motionPool[motion_T][0] > pParam->u27_Mot_all_min){
				quitCntOffset_tmp = quitCntOffset_tmp+1;
			}
		}
		else{
			quitCntOffset_tmp = quitCntOffset_tmp-1;
		}
		quitCntOffset_tmp = (quitCntOffset_tmp<4) ? 4 : (quitCntOffset_tmp>20) ? 20 : quitCntOffset_tmp;
	}

	quitCntOffset = quitCntOffset_tmp;

	return quitCntOffset;
}

VOID FilmQuit(const _PARAM_FilmDetectCtrl *pParam, _OUTPUT_FilmDetectCtrl *pOutput, unsigned char motion_T)
{
	unsigned char  u8_preCad_id = 0,u1_curCad_motFlag = 0;
	unsigned int u27_curFrm_mot = 0,u27_quit_motTh = 0;
	unsigned char u8_pre_quit_prd = 0,u8_pre_quit_cnt = 0,u8_quit_prd_th = 0;
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	unsigned char u8_curFrm_det_cadId = pOutput->u8_det_cadence_Id[motion_T];
	unsigned char length = 0, length_PreCad = 0;
	unsigned int u32_match_seq_sft = 0, u32_match_seq_sft_PreCad = 0;

	unsigned short u8_quitCntTh = pParam->u8_quit_cnt_th;
	unsigned int u3_quitCntOffset = 3;  // Don't change again
	unsigned int u3_quitCntOffset_32322_bad_edit = 20;  // Don't change again
	unsigned int u3_quitCntOffset_Black_19YO1= 24; //YE Test for tv006 01_Black_19YO1 issue
	unsigned int log_en = 0, log_en2 = 0;
	static unsigned char u1_32322_NoSC_flag = 0;
	static unsigned char u8_32322_NoSC_frame_hold = 0;
	ReadRegister(SOFTWARE1_SOFTWARE1_60_reg, 30, 30, &log_en);
	ReadRegister(SOFTWARE3_SOFTWARE3_09_reg, 7, 7, &log_en2);

	if(motion_T >= _FILM_MOT_NUM){
		rtd_pr_memc_notice("[%s]Invalid Motion_Type:%d!!\n",__FUNCTION__,motion_T);
		motion_T = _FILM_ALL;
	}
	u8_preCad_id       = pOutput->u8_pre_cadence_Id[motion_T];
	
	if(u8_preCad_id >= _FRC_CADENCE_NUM_){
		rtd_pr_memc_notice("[%s]Invalid u8_preCad_id:%d,motion=%d.Resume!\n",\
			__FUNCTION__,u8_preCad_id,motion_T);
		u8_preCad_id = _CAD_VIDEO;
	}
	if(u8_preCad_id == _CAD_32322){
		u1_curCad_motFlag  = cadence_seqFlag(frc_cadTable[u8_preCad_id], ((pOutput->u8_phase_Idx[motion_T]+6)%12));
	}else{	
		u1_curCad_motFlag  = cadence_seqFlag(frc_cadTable[u8_preCad_id], pOutput->u8_phase_Idx[motion_T]);
	}
	length = frc_cadTable[u8_curFrm_det_cadId].cad_len;
	u32_match_seq_sft = frc_cadTable[u8_curFrm_det_cadId].cad_seq;
	length_PreCad = frc_cadTable[u8_preCad_id].cad_len;
	u32_match_seq_sft_PreCad = frc_cadTable[u8_preCad_id].cad_seq;

	u27_curFrm_mot     = pOutput->u27_ipme_motionPool[motion_T][0];
	u27_quit_motTh     = pOutput->u27_quit_motTh[motion_T];

	u8_pre_quit_prd     = pOutput->u8_quit_prd[motion_T];
	u8_pre_quit_cnt     = pOutput->u8_quit_cnt[motion_T];
	u8_quit_prd_th      = ((u8_preCad_id == _CAD_1112) || (u8_preCad_id == _CAD_11112)) ? pParam->u8_quit_prd_th/2 : pParam->u8_quit_prd_th;
	u8_quit_prd_th      = pParam->u8_quit_prd_th;
	// bigger quitCntTh if many '0' in sequence => bad edit need
	if((motion_T == _FILM_ALL) && (pOutput->u8_pre_cadence_Id[motion_T] == _CAD_32322)) 
	{
		if(s_pContext->_output_fblevelctrl.u1_SW_sc_true == 1)
			u8_quitCntTh = u8_quitCntTh + (u3_quitCntOffset*2);
		else
			u8_quitCntTh = u8_quitCntTh + (u3_quitCntOffset_32322_bad_edit);
	}
	else if((motion_T == _FILM_ALL) && (pOutput->u8_pre_cadence_Id[motion_T] == _CAD_22)) // CG_Pattern, video->22(very small motion),not easy to quit
	{
		if (s_pContext->_output_wrt_comreg.u8_01_Black_19YO1_flag==1)//YE Test for tv006 01_Black_19YO1 issue
			u8_quitCntTh = (u8_quitCntTh > 1) ? (u8_quitCntTh +u3_quitCntOffset_Black_19YO1) : 0;
		else
			u8_quitCntTh = u8_quitCntTh + u3_quitCntOffset;
			//u8_quitCntTh = (u8_quitCntTh > 1) ? (u8_quitCntTh -1) : 0;
	}
	else if((motion_T == _FILM_ALL) && (pOutput->u8_pre_cadence_Id[motion_T] == _CAD_32)) 
	{
		u3_quitCntOffset = FilmQuit_QuitCntOffsetCalc(pParam, pOutput, motion_T);
		u8_quitCntTh = u8_quitCntTh + u3_quitCntOffset;
	}
	else if((motion_T == _FILM_ALL) && (pOutput->u8_pre_cadence_Id[motion_T] == _CAD_2224)) 
	{
		u8_quitCntTh = u8_quitCntTh + u3_quitCntOffset;
	}
	else if((motion_T == _FILM_ALL) && (pOutput->u8_pre_cadence_Id[motion_T] == _CAD_55)) 
	{
		u8_quitCntTh = u8_quitCntTh + u3_quitCntOffset;
	}
	else if((motion_T == _FILM_ALL) && (pOutput->u8_pre_cadence_Id[motion_T] == _CAD_11i23)) 
	{
		u8_quitCntTh = u8_quitCntTh + u3_quitCntOffset*6;
	}
	else if((motion_T == _FILM_ALL) && (pOutput->u8_pre_cadence_Id[motion_T] == _CAD_1225_T1)) 
	{
		u8_quitCntTh = u8_quitCntTh + u3_quitCntOffset*4;//frc_cadTable[_CAD_1225_T1].cad_len*100;
	}
	else if((motion_T == _FILM_ALL) && (pOutput->u8_pre_cadence_Id[motion_T] == _CAD_1225_T2)) 
	{
		u8_quitCntTh = u8_quitCntTh + u3_quitCntOffset*4;//frc_cadTable[_CAD_1225_T2].cad_len*100;
	}
#if 1
	else if((motion_T == _FILM_ALL) && (pOutput->u8_pre_cadence_Id[motion_T] == _CAD_1112)) 
	{
		u8_quitCntTh = u8_quitCntTh + u3_quitCntOffset*5+1;
	}
	else if((motion_T == _FILM_ALL) && (pOutput->u8_pre_cadence_Id[motion_T] == _CAD_11112)) 
	{
		u8_quitCntTh = u8_quitCntTh + u3_quitCntOffset+1;
	}
#endif	

	//================================
	// quit state-machine
	if (u1_curCad_motFlag == 0)//judge by motion sequence
	{
		if (u27_curFrm_mot > u27_quit_motTh)
		{
			pOutput->u8_quit_cnt[motion_T] = _CLIP_(u8_pre_quit_cnt + 1, 0, u8_quitCntTh + 4);
			pOutput->u8_quit_prd[motion_T] = 0;
		}
		else //quit cnt judge by quit motion th, keep for a while when frm motion is small when cadence change
		{
			pOutput->u8_quit_prd[motion_T] = _MIN_(u8_pre_quit_prd + 1, u8_quit_prd_th);
			pOutput->u8_quit_cnt[motion_T] = pOutput->u8_quit_prd[motion_T] == u8_quit_prd_th ? (u8_pre_quit_cnt <= 1? 0 : u8_pre_quit_cnt-1) : u8_pre_quit_cnt;
		}
	}

	// for 32322 bad-editting 
	if(pOutput->u8_pre_cadence_Id[motion_T] == _CAD_32322 && s_pContext->_output_fblevelctrl.u1_HW_sc_true == 0 
		&& pOutput->u8_det_cadence_Id[motion_T] != _CAD_32322 && pOutput->u8_det_cadence_Id_pre[motion_T] == _CAD_32322)
	{
		u1_32322_NoSC_flag = 1;
	}
	else{
		u1_32322_NoSC_flag = 0;
	}

	if(u1_32322_NoSC_flag){
		u8_32322_NoSC_frame_hold = 40;
		u8_quitCntTh = u8_quitCntTh + u3_quitCntOffset_32322_bad_edit;
		//pOutput->u8_cur_cadence_Id[_FILM_ALL] = _CAD_32322;
	}
	else if(u8_32322_NoSC_frame_hold > 0){
		u8_32322_NoSC_frame_hold--;
		u8_quitCntTh = u8_quitCntTh + u3_quitCntOffset_32322_bad_edit;
		//pOutput->u8_cur_cadence_Id[_FILM_ALL] = _CAD_32322;
	}

	if(motion_T == _FILM_ALL){
		FilmQuit_BadEditPatch(pParam, pOutput);
		pOutput->u8_quit_cnt[motion_T] = _CLIP_UBOUND_(pOutput->u8_quit_cnt[motion_T], (u8_quitCntTh+4));
	}

	if(u8_quitCntTh == 0)
		u8_quitCntTh = pParam->u8_quit_cnt_th;

	if(u8_quitCntTh == 0)
		u8_quitCntTh = pParam->u8_quit_cnt_th;
//	if ((motion_T == pOutput->u8_dbg_motion_t) && (((pOutput->u5_dbg_param1_pre & BIT_3) >> 3) == 1) && pOutput->u32_dbg_cnt_hold > 0)
	if(log_en && motion_T == _FILM_ALL)
//	if(0)
	{
		rtd_pr_memc_info("[MEMC]%s::quit_cnt=%d, quit_prd=%d, phase_ID=%d \n",__FUNCTION__,pOutput->u8_quit_cnt[motion_T], pOutput->u8_quit_prd[motion_T], pOutput->u8_phase_Idx[motion_T]);
		rtd_pr_memc_info("[MEMC]%s::preCad_id=%d, pre_cadence_Id=%d, cur_cadence_Id=%d, det_cadence_Id = %d \n",__FUNCTION__, u8_preCad_id, pOutput->u8_pre_cadence_Id[motion_T], pOutput->u8_cur_cadence_Id[motion_T], u8_curFrm_det_cadId);
		rtd_pr_memc_info("[MEMC]%s::u8_quitCntTh=%d, u8_quitPrdTh=%d, mixMode=%d \n",__FUNCTION__, u8_quitCntTh, u8_quit_prd_th, pOutput->u1_mixMode);
		rtd_pr_memc_info("[MEMC]%s::u1_32322_NoSC_flag=%d, u8_32322_NoSC_frame_hold=%d \n",__FUNCTION__, u1_32322_NoSC_flag, u8_32322_NoSC_frame_hold);
	}

//	if(motion_T==_FILM_ALL) {
//		rtd_printk(KERN_NOTICE, TAG_NAME_MEMC, "[FQ] 0 detID:%d  curID:%d  cnt:%d  cntTH:%d\n",
//		u8_curFrm_det_cadId, pOutput->u8_cur_cadence_Id[motion_T],
//		pOutput->u8_quit_cnt[motion_T], u8_quitCntTh );
//	}

	// quit judge
	if((pOutput->u8_pre_cadence_Id[_FILM_ALL] == _CAD_22 && pOutput->u8_det_cadence_Id[_FILM_ALL] == _CAD_321) ||(pOutput->u8_pre_cadence_Id[_FILM_ALL] == _CAD_321 &&pOutput->u8_det_cadence_Id[_FILM_ALL] == _CAD_22 ))
	{
		pOutput->u8_cur_cadence_Id[motion_T] = u8_preCad_id;
		if(log_en2 && motion_T==_FILM_ALL)
			rtd_printk(KERN_CRIT, TAG_NAME_MEMC, "[FQ] 1 %d\n", u8_preCad_id );
	}
	else if (pOutput->u1_mixMode == 1 && pOutput->u8_cur_cadence_Id[motion_T]!=_CAD_1225_T1 && pOutput->u8_cur_cadence_Id[motion_T]!=_CAD_1225_T2 
		&& pOutput->u8_cur_cadence_Id[motion_T]!=_CAD_11112 && pOutput->u8_cur_cadence_Id[motion_T]!=_CAD_1112 
		&& u8_curFrm_det_cadId!=_CAD_11i23)
	{
		if(log_en && motion_T == _FILM_ALL)
			pOutput->u8_cur_cadence_Id[motion_T] = _CAD_VIDEO;
		else
			pOutput->u8_cur_cadence_Id[motion_T] = u8_curFrm_det_cadId;//_CAD_VIDEO;
		pOutput->u8_quit_cnt[motion_T]       = 0;
		pOutput->u8_quit_prd[motion_T]       = 0;
		if(log_en2 && motion_T==_FILM_ALL)
			rtd_printk(KERN_CRIT, TAG_NAME_MEMC, "[FQ] 2 %d [%d<%d]\n", u8_curFrm_det_cadId, pOutput->u8_quit_cnt[motion_T], u8_quitCntTh );
	}
	else if (pOutput->u8_quit_cnt[motion_T] >= u8_quitCntTh)
	{
		if(log_en && motion_T == _FILM_ALL){
			rtd_pr_memc_info("[MEMC]%s::Mot_sequence=%x, u32_match_seq_sft=%x\n",__FUNCTION__,(pOutput->u32_Mot_sequence[motion_T]&((1<<length)-1)), u32_match_seq_sft);
		}		
		#if CONFIG_MC_8_BUFFER
		if((pOutput->u32_Mot_sequence[motion_T]&((1<<length)-1)) == u32_match_seq_sft){
			if(log_en2 && motion_T==_FILM_ALL) {
				rtd_printk(KERN_CRIT, TAG_NAME_MEMC, 
				"[FQ] 3 seq:%u len:%d sft:%u id:%d\n",
				pOutput->u32_Mot_sequence[motion_T], length, u32_match_seq_sft,
				u8_curFrm_det_cadId );
			}
			pOutput->u8_cur_cadence_Id[motion_T] = u8_curFrm_det_cadId; // _CAD_VIDEO;
			pOutput->u8_quit_cnt[motion_T]       = 0;
			pOutput->u8_quit_prd[motion_T]       = 0;
		}
//		else if(u8_curFrm_det_cadId == _CAD_VIDEO && u8_preCad_id != _CAD_VIDEO
//			&& (pOutput->u32_Mot_sequence[motion_T]&((1<<length_PreCad)-1)) == u32_match_seq_sft_PreCad){
//			pOutput->u8_cur_cadence_Id[motion_T] = u8_curFrm_det_cadId; // _CAD_VIDEO;
//			pOutput->u8_quit_cnt[motion_T]       = 0;
//			pOutput->u8_quit_prd[motion_T]       = 0;
//		}
		else{
			pOutput->u8_cur_cadence_Id[motion_T] = u8_preCad_id;
			if(log_en2 && motion_T==_FILM_ALL) {
				rtd_printk(KERN_CRIT, TAG_NAME_MEMC, "[FQ] 4 seq:%u len:%d sft:%u prd:%d\n", 
				pOutput->u32_Mot_sequence[motion_T], length, u32_match_seq_sft, u8_preCad_id );
			}
		}
		#else
		pOutput->u8_cur_cadence_Id[motion_T] = u8_curFrm_det_cadId; // _CAD_VIDEO;
		pOutput->u8_quit_cnt[motion_T]       = 0;
		pOutput->u8_quit_prd[motion_T]       = 0;
//		if(motion_T==_FILM_ALL)
//			rtd_printk(KERN_NOTICE, TAG_NAME_MEMC, "[FQ] 5 %d\n", u8_curFrm_det_cadId );
		#endif
	}
	else
	{
		pOutput->u8_cur_cadence_Id[motion_T] = u8_preCad_id;
		if(log_en2 && motion_T==_FILM_ALL)
			rtd_printk(KERN_CRIT, TAG_NAME_MEMC, "[FQ] 6 %d\n", u8_preCad_id );
	}
}

#if 0
void Cad1225To22(_OUTPUT_FilmDetectCtrl *pOutput)
{
	unsigned int u1_test_en = 0;
	unsigned int log_en = 0;
	static unsigned char CaseID;
	static unsigned char lastID=0;

	ReadRegister(SOFTWARE3_SOFTWARE3_09_reg, 14, 14, &u1_test_en);
	ReadRegister(SOFTWARE3_SOFTWARE3_09_reg, 15, 15, &log_en);

	if(lastID==0){
		pOutput->u8_phase_Idx[_FILM_ALL] = 1;
	}
	else {
		pOutput->u8_phase_Idx[_FILM_ALL] = 0;
	}

	if((pOutput->u32_Mot_sequence[_FILM_ALL]&0xff)==0xA9){ //10101001
		if(u1_test_en==1){
			pOutput->u8_phase_Idx[_FILM_ALL] = 1;
			CaseID = 1;
		}
		else{
			pOutput->u8_phase_Idx[_FILM_ALL] = 0;
			CaseID = 0;
		}
		if(log_en)
			rtd_printk(KERN_EMERG, TAG_NAME_MEMC, "[change 1] %d", CaseID);
	}
	else if((pOutput->u32_Mot_sequence[_FILM_ALL]&0xff)==0x95){ //10010101
		if(u1_test_en==1){
			pOutput->u8_phase_Idx[_FILM_ALL] = 0;
			CaseID = 0;
		}
		else{
			pOutput->u8_phase_Idx[_FILM_ALL] = 1;
			CaseID = 1;
		}
		if(log_en)
			rtd_printk(KERN_EMERG, TAG_NAME_MEMC, "[change 2] %d", CaseID);
	}
	else{
		//rtd_printk(KERN_CRIT, TAG_NAME_MEMC, "%d\n",(pOutput->u32_Mot_sequence[_FILM_ALL]&0xff));
	}

	lastID = pOutput->u8_phase_Idx[_FILM_ALL];
}
#endif

char Film_OptimizeCad11i23(_OUTPUT_FilmDetectCtrl *pOutput, unsigned char motion_T )
{
	static unsigned char u8_CadCnt=0;
	char u8_result = -1;
	unsigned char u8_offset=14; // 0x12AAAAA->0   0x1555554->14
	unsigned int log_en = 0;
	ReadRegister(SOFTWARE3_SOFTWARE3_09_reg, 13, 13, &log_en);
	
	if(motion_T!=_FILM_ALL){
		return -1;
	}
	if(pOutput->u8_cur_cadence_Id[_FILM_ALL]!=_CAD_1225_T1&&pOutput->u8_det_cadence_Id[_FILM_ALL]!=_CAD_1225_T1&&pOutput->ExchangeCad==false){
		return -1;
	}
	
	if(u8_CadCnt>0){
		u8_CadCnt--;
	}
	else {
		pOutput->ExchangeCad=false;
	}

	if((pOutput->u32_Mot_sequence[_FILM_ALL]&0xff)==0xA9){ //10101001
		pOutput->u8_cur_cadence_Id[_FILM_ALL] = _CAD_11i23;
		pOutput->ExchangeCad=true;
		u8_CadCnt=25;
		if(log_en)
			rtd_printk(KERN_INFO, TAG_NAME_MEMC, "[Force Exchange 18]");
		u8_result = (3+u8_offset); 
	}
	else if((pOutput->u32_Mot_sequence[_FILM_ALL]&0xfff)==0x999){ //100110011001
		pOutput->u8_cur_cadence_Id[_FILM_ALL] = _CAD_11i23;
		pOutput->ExchangeCad=true;
		u8_CadCnt=25;
		if(log_en)
			rtd_printk(KERN_INFO, TAG_NAME_MEMC, "[Force Exchange 1]");
		u8_result = (11+u8_offset); 
	}
	else if((pOutput->u32_Mot_sequence[_FILM_ALL]&0xff)==0x95){ //10010101
		pOutput->u8_cur_cadence_Id[_FILM_ALL] = _CAD_11i23;
		pOutput->ExchangeCad=true;
		u8_CadCnt=25;
		if(log_en)
			rtd_printk(KERN_INFO, TAG_NAME_MEMC, "[Force Exchange 5]");
		u8_result = (15+u8_offset); 
	}
	#if 0
	else if((pOutput->u32_Mot_sequence[_FILM_ALL]&0x3fff)==0x2AAA){ //10101010101010
		pOutput->u8_cur_cadence_Id[_FILM_ALL] = _CAD_11i23;
		pOutput->ExchangeCad=true;
		u8_CadCnt=25;
		rtd_printk(KERN_CRIT, TAG_NAME_MEMC, "[Force Exchange 24]");
		u8_result = (20+u8_offset); 
	}
	#endif
	else if((pOutput->u32_Mot_sequence[_FILM_ALL]&0x3ff)==0x2D9){ //1011011001
		pOutput->u8_cur_cadence_Id[_FILM_ALL] = _CAD_11i23;
		pOutput->ExchangeCad=true;
		u8_CadCnt=25;
		if(log_en)
			rtd_printk(KERN_INFO, TAG_NAME_MEMC, "[Force Exchange 22 1011011001]");
		u8_result = (7+u8_offset); 
	}
	else if((pOutput->u32_Mot_sequence[_FILM_ALL]&0x3ff)==0x2FD){ //1011111101
		pOutput->u8_cur_cadence_Id[_FILM_ALL] = _CAD_11i23;
		pOutput->ExchangeCad=true;
		u8_CadCnt=25;
		if(log_en)
			rtd_printk(KERN_INFO, TAG_NAME_MEMC, "[Force Exchange 7 1011011001]");
		u8_result = (17+u8_offset); 
	}
	#if 0
	else if((pOutput->u32_Mot_sequence[_FILM_ALL]&0x3f)==0x24){ //100100
		pOutput->u8_cur_cadence_Id[_FILM_ALL] = _CAD_11i23;
		pOutput->ExchangeCad=true;
		u8_CadCnt=25;
		rtd_printk(KERN_CRIT, TAG_NAME_MEMC, "[Force Exchange 14 100100]");
		u8_result = (24+u8_offset); 
	}
	#endif
	else if((pOutput->u32_Mot_sequence[_FILM_ALL]&0x7f)==0x4B){ //1001011
		pOutput->u8_cur_cadence_Id[_FILM_ALL] = _CAD_11i23;
		pOutput->ExchangeCad=true;
		u8_CadCnt=25;
		if(log_en)
			rtd_printk(KERN_INFO, TAG_NAME_MEMC, "[Force Exchange 5 1001011]");
		u8_result = (15+u8_offset); 
	}
	else if((pOutput->u32_Mot_sequence[_FILM_ALL]&0xff)==0x97){ //10010111
		pOutput->u8_cur_cadence_Id[_FILM_ALL] = _CAD_11i23;
		pOutput->ExchangeCad=true;
		u8_CadCnt=25;
		if(log_en)
			rtd_printk(KERN_INFO, TAG_NAME_MEMC, "[Force Exchange 7 10010111]");
		u8_result = (17+u8_offset); 
	}
	#if 0
	else if((pOutput->u32_Mot_sequence[_FILM_ALL]&0x3f)==0x29){ //101001
		pOutput->u8_cur_cadence_Id[_FILM_ALL] = _CAD_11i23;
		pOutput->ExchangeCad=true;
		u8_CadCnt=25;
		if(log_en)
			rtd_printk(KERN_CRIT, TAG_NAME_MEMC, "[Force Exchange 18 101001]");
		return (3+u8_offset); 
	}
	#else
	else if((pOutput->u32_Mot_sequence[_FILM_ALL]&0x7f)==0x53){ //1010011
		pOutput->u8_cur_cadence_Id[_FILM_ALL] = _CAD_11i23;
		pOutput->ExchangeCad=true;
		u8_CadCnt=25;
		if(log_en)
			rtd_printk(KERN_INFO, TAG_NAME_MEMC, "[Force Exchange 19 1010011]");
		u8_result = (4+u8_offset); 
	}	
	#endif

	u8_result = (u8_result>=25) ? (u8_result-25): u8_result;
	return u8_result;
}

unsigned char g_cadence_change_flag = 0;
CAD_CHANGE_QUE_BUFFER g_cadence_change_que;
unsigned char PhaseIdx_set(const _PARAM_FilmDetectCtrl *pParam, _OUTPUT_FilmDetectCtrl *pOutput, unsigned char motion_T )
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	_PQLCONTEXT *s_pContext_m = GetPQLContext_m();
	unsigned char u8_cur_cadId = 0, u8_j = 0;
	unsigned int Film_force_cadence_en = 0;
	unsigned int Film_force_cadence_type = 0;
	unsigned int test_32322_BadEdit_en = 0;
	unsigned char u1_match_flag = 0;
	unsigned int u32_match_seq_sft = 0;
	unsigned int test_phase_idx_cal_en = 0;
	unsigned int test_en = 0, log_en = 0;
	unsigned char result = 127, debug_info = 0;
	unsigned int u1_OptCad11i23_en = 0, u1_BE_apply_smooth_phase_en = 0, u1_dynCtrlPhase_en = 0, log_en2=0;
	bool u1_dynCtrlPhase = false;
#if Pro_tv030
	char u8_Opt11i23_StartID=0;
	static unsigned char u8_Opt11i23_PhaseId=0;
	static unsigned char u8_Opt11i23_HoldCnt=0;
#endif	

	ReadRegister(SOFTWARE1_SOFTWARE1_12_reg,  0,  3, &Film_force_cadence_type);
	ReadRegister(SOFTWARE1_SOFTWARE1_12_reg,  4,  4, &Film_force_cadence_en);
	ReadRegister(SOFTWARE1_SOFTWARE1_60_reg, 30, 30, &log_en);
	ReadRegister(SOFTWARE1_SOFTWARE1_60_reg, 29, 29, &test_32322_BadEdit_en);
	ReadRegister(SOFTWARE1_SOFTWARE1_60_reg, 27, 27, &test_phase_idx_cal_en);
	ReadRegister(SOFTWARE1_SOFTWARE1_62_reg,  0,  0, &test_en);
	ReadRegister(SOFTWARE3_SOFTWARE3_09_reg, 24, 24, &u1_OptCad11i23_en);
	ReadRegister(SOFTWARE3_SOFTWARE3_09_reg,  2,  2, &log_en2);
	ReadRegister(SOFTWARE3_SOFTWARE3_09_reg, 25, 25, &u1_BE_apply_smooth_phase_en);
	ReadRegister(SOFTWARE2_SOFTWARE2_63_reg, 13, 13, &u1_dynCtrlPhase_en);

#if Pro_tv030
	if(u1_OptCad11i23_en) {
		u8_Opt11i23_StartID = Film_OptimizeCad11i23(pOutput, motion_T);
		if(u8_Opt11i23_StartID>=0) {
			u8_Opt11i23_HoldCnt = 30;
			u8_Opt11i23_PhaseId = u8_Opt11i23_StartID;
		}
	}
#endif

	if(motion_T >= _FILM_MOT_NUM){
		rtd_pr_memc_notice("[%s]Invalid Motion_Type:%d!!\n",__FUNCTION__,motion_T);
		motion_T = _FILM_ALL;
	}

	//---#201_32322_BadEdit
	if(test_32322_BadEdit_en && pOutput->u8_32322_BadEdit_cnt > 30){
		pOutput->u8_cur_cadence_Id[motion_T] = _CAD_32;
	}
	if(test_32322_BadEdit_en && pOutput->u8_Special_60_BadEdit_cnt >= 33){
		pOutput->u8_cur_cadence_Id[motion_T] = _CAD_VIDEO;
	}
	if(test_32322_BadEdit_en && pOutput->u8_Special_50_BadEdit_cnt > 33){
		pOutput->u8_cur_cadence_Id[motion_T] = _CAD_334;
	}
	if(test_32322_BadEdit_en && pOutput->u8_Special_30_BadEdit_cnt > 30 && pOutput->u8_cur_cadence_Id[_FILM_ALL]!=_CAD_11112 ){
		pOutput->u8_cur_cadence_Id[motion_T] = _CAD_VIDEO;
	}
	//---------------------
	
	u8_cur_cadId = pOutput->u8_cur_cadence_Id[motion_T];
	if(Film_force_cadence_en){ // Film_force_cadence_en
		pOutput->u8_cur_cadence_Id[motion_T] = Film_force_cadence_type; // Film_force_cadence_type
		u8_cur_cadId = pOutput->u8_cur_cadence_Id[motion_T];
	}

	if(u8_cur_cadId >= _FRC_CADENCE_NUM_){
		rtd_pr_memc_notice("[%s]Invalid u8_cur_cadId:%d,motion=%d.Resume!\n", __FUNCTION__, u8_cur_cadId, motion_T);
		u8_cur_cadId = _CAD_VIDEO;
	}
#if Pro_tv030
	if (u8_cur_cadId == _CAD_VIDEO && pOutput->u8_pre_cadence_Id[motion_T]!=_CAD_1225_T1)
	{
		debug_info = 98;
		pOutput->u8_phase_Idx[motion_T] = 0;
		if(u1_dynCtrlPhase_en && motion_T==_FILM_ALL){
			debug_info = 99;
			u1_dynCtrlPhase = true;
		}
	}
	else if(u8_Opt11i23_HoldCnt>0) {
		if(motion_T == _FILM_ALL){
			u8_Opt11i23_PhaseId=(u8_Opt11i23_PhaseId==24)? 0 : u8_Opt11i23_PhaseId+1;
			u8_Opt11i23_HoldCnt--;
			pOutput->u8_phase_Idx[motion_T] = u8_Opt11i23_PhaseId;
			if(log_en2){
			rtd_printk(KERN_INFO, TAG_NAME_MEMC, "[Exchange ID] %d %d %u%u%u%u%u%u%u%u%u%u\n", u8_Opt11i23_PhaseId, u8_Opt11i23_HoldCnt,
				(pOutput->u32_Mot_sequence[_FILM_ALL]>>9)&1,
				(pOutput->u32_Mot_sequence[_FILM_ALL]>>8)&1,
				(pOutput->u32_Mot_sequence[_FILM_ALL]>>7)&1,
				(pOutput->u32_Mot_sequence[_FILM_ALL]>>6)&1,
				(pOutput->u32_Mot_sequence[_FILM_ALL]>>5)&1,
				(pOutput->u32_Mot_sequence[_FILM_ALL]>>4)&1,
				(pOutput->u32_Mot_sequence[_FILM_ALL]>>3)&1,
				(pOutput->u32_Mot_sequence[_FILM_ALL]>>2)&1,
				(pOutput->u32_Mot_sequence[_FILM_ALL]>>1)&1,
				(pOutput->u32_Mot_sequence[_FILM_ALL]>>0)&1	);
			}
		}
	}
#else
	if (u8_cur_cadId == _CAD_VIDEO)
	{
		debug_info = 98;
		pOutput->u8_phase_Idx[motion_T] = 0;
	}
#endif
	else
	{
		if(test_phase_idx_cal_en){	
			if(pOutput->u8_cur_cadence_Id[motion_T] == pOutput->u8_det_cadence_Id[motion_T] || (test_en == 1)){
				u32_match_seq_sft = frc_cadTable[u8_cur_cadId].cad_seq;
				for (u8_j = 0; u8_j < frc_cadTable[u8_cur_cadId].cad_len; u8_j ++)
				{
					u32_match_seq_sft = (u32_match_seq_sft>>1) | ((u32_match_seq_sft&0x1)<<(frc_cadTable[u8_cur_cadId].cad_len-1));
					if ((pOutput->u32_Mot_sequence[motion_T] & frc_cadTable[u8_cur_cadId].cad_check) == u32_match_seq_sft)
					{
						u1_match_flag = 1;
						pOutput->u8_phase_Idx_tmp[motion_T] = (frc_cadTable[u8_cur_cadId].cad_len-1)-u8_j;
						result = pOutput->u8_phase_Idx_tmp[motion_T];
						break;
					}
				}

				#if 0
				if(motion_T==_FILM_ALL && u8_cur_cadId==_CAD_1225_T1 && u1_match_flag == 0){

					if((pOutput->u32_Mot_sequence[_FILM_ALL]&0x1ffffff)==0x1553329){
						changeid = 0;//start with id4
						lastid = 5;
						changecnt = 26;
					}

					if(changecnt!=0){
						pOutput->u8_phase_Idx_tmp[motion_T] = lastid;
						changecnt--;
						result = pOutput->u8_phase_Idx_tmp[motion_T]+100;
					//	rtd_printk(KERN_CRIT, TAG_NAME_MEMC, "[INFO] id:%d cnt:%d Len:%d\n", lastid, changecnt, frc_cadTable[u8_cur_cadId].cad_len );

						lastid++;
						lastid = (lastid>=frc_cadTable[u8_cur_cadId].cad_len) ? lastid-frc_cadTable[u8_cur_cadId].cad_len : lastid;

					}
				//if((pOutput->u32_Mot_sequence[_FILM_ALL]&0x1ffff) == 0x13329){ 
					//if((pOutput->u32_Mot_sequence[motion_T] & 0x7f)==0x53){
					//	pOutput->u8_phase_Idx_tmp[motion_T] = 5;
					//	rtd_printk(KERN_CRIT, TAG_NAME_MEMC, "[FIX] pahse id 5\n" );
					//	break;
					//}
				}
				#endif
				//rtd_printk(KERN_CRIT, TAG_NAME_MEMC, "[INFO] %d < %d %d >\n", u1_match_flag, 
				//(pOutput->u32_Mot_sequence[motion_T] & frc_cadTable[u8_cur_cadId].cad_check), u32_match_seq_sft);

				if(u1_match_flag == 1){
					debug_info = 2;
					if(u8_cur_cadId == _CAD_32322){
						#if CONFIG_MC_8_BUFFER
						pOutput->u8_phase_Idx[motion_T] = (pOutput->u8_phase_Idx_tmp[motion_T]+6)%frc_cadTable[u8_cur_cadId].cad_outLen;
						#else
						pOutput->u8_phase_Idx[motion_T] = (pOutput->u8_phase_Idx_tmp[motion_T]+5)%frc_cadTable[u8_cur_cadId].cad_outLen;
						#endif
						debug_info = 1;
					}
					else
						pOutput->u8_phase_Idx[motion_T] = pOutput->u8_phase_Idx_tmp[motion_T];
				}
				else{
					debug_info = 3;
					pOutput->u8_phase_Idx[motion_T] ++;

					if(u8_cur_cadId==_CAD_11112&&((pOutput->u8_phase_Idx[motion_T])%6)==0&&motion_T==_FILM_ALL){
							s_pContext_m->_external_data.u1_ReplaceCad = 1;
						s_pContext_m->_external_data.u8_ReplaceCase = 2;
						pOutput->u8_phase_Idx[motion_T] = 5;
						}
					else if(u8_cur_cadId==_CAD_1112&&((pOutput->u8_phase_Idx[motion_T])%5)==0&&motion_T==_FILM_ALL){
						s_pContext_m->_external_data.u1_ReplaceCad = 1;
						s_pContext_m->_external_data.u8_ReplaceCase = 5;
						pOutput->u8_phase_Idx[motion_T] = 4;
					}
					else if(s_pContext->_external_data.u1_ReplaceProtect==1||u1_BE_apply_smooth_phase_en==1){
						if(u8_cur_cadId==_CAD_1112&&motion_T==_FILM_ALL){
							if((pOutput->u32_Mot_sequence[motion_T]&0x1F)==0x2){
								pOutput->u8_phase_Idx[motion_T] = 0;
							}
							else if((pOutput->u32_Mot_sequence[motion_T]&0x1F)==0x1 && s_pContext->_external_data.u8_SpecialCase==1){
								pOutput->u8_phase_Idx[motion_T] = 4;
							}
						}
						else if(u8_cur_cadId==_CAD_1112&&pOutput->u8_phase_Idx[motion_T]==3&&motion_T==_FILM_ALL){
							s_pContext_m->_external_data.u1_ReplaceCad = 1;
							s_pContext_m->_external_data.u8_ReplaceCase = 1;
						}
						else if(u8_cur_cadId==_CAD_11i23&&pOutput->u8_phase_Idx[motion_T]==25&&motion_T==_FILM_ALL){
							s_pContext_m->_external_data.u1_ReplaceCad = 1;
							s_pContext_m->_external_data.u8_ReplaceCase = 4;
							pOutput->u8_phase_Idx[motion_T] = 23;//23
						}
						// 3131323
						else if(u8_cur_cadId==_CAD_1225_T1&&motion_T==_FILM_ALL){
							debug_info = 10;
							if(((pOutput->u32_Mot_sequence[_FILM_ALL]<<15)>>15)==0x13329){
								s_pContext_m->_external_data.u1_ReplaceCad = 1;
								s_pContext_m->_external_data.u8_ReplaceCase = 3;
								pOutput->u8_phase_Idx[motion_T] = 0;
								debug_info = 11;
							}
						}
						//if(log_en2 && motion_T==_FILM_ALL){
						//	rtd_printk(KERN_CRIT, TAG_NAME_MEMC, "CAD:%d ID:%d Case:%d F:%d SpecialCase:%d %u\n",
						//	u8_cur_cadId, pOutput->u8_phase_Idx[motion_T], s_pContext_m->_external_data.u8_ReplaceCase,
						//	s_pContext_m->_external_data.u1_ReplaceCad, s_pContext_m->_external_data.u8_SpecialCase, pOutput->u32_Mot_sequence[_FILM_ALL]);
						//}
						}
					else if(u1_dynCtrlPhase_en && motion_T==_FILM_ALL && (u8_cur_cadId==_CAD_32322||u8_cur_cadId==_CAD_32) ){
						debug_info = 12;
						u1_dynCtrlPhase = true;
					}
				}
			}
			else{
				debug_info=4;
				pOutput->u8_phase_Idx[motion_T] ++;
			}
		}
		else{
			debug_info=5;
			pOutput->u8_phase_Idx[motion_T] ++;
		}
		pOutput->u8_phase_Idx[motion_T] = (pOutput->u8_phase_Idx[motion_T]%frc_cadTable[u8_cur_cadId].cad_outLen);
	}

#if 1
	if(test_32322_BadEdit_en){
		if(u8_cur_cadId == _CAD_32322)
		{
			#if CONFIG_MC_8_BUFFER
			if((pOutput->u32_Mot_sequence[motion_T] & frc_cadTable[u8_cur_cadId].cad_check) == frc_cadTable[u8_cur_cadId].cad_seq && pOutput->u8_phase_Idx[motion_T] != 6)
			{
				pOutput->u1_cad_resync_true[motion_T] = 0;
				pOutput->u8_phase_Idx[motion_T] = 6; // 5;
			}
			#else
			if((pOutput->u32_Mot_sequence[motion_T] & frc_cadTable[u8_cur_cadId].cad_check) == frc_cadTable[u8_cur_cadId].cad_seq && pOutput->u8_phase_Idx[motion_T] != 5)
			{
				pOutput->u1_cad_resync_true[motion_T] = 0;
				pOutput->u8_phase_Idx[motion_T] = 5; // 5;
			}
			#endif
		}
		else if((pOutput->u32_Mot_sequence[motion_T] & frc_cadTable[u8_cur_cadId].cad_check) == frc_cadTable[u8_cur_cadId].cad_seq && pOutput->u8_phase_Idx[motion_T] != 0)
		{
			pOutput->u1_cad_resync_true[motion_T] = 0;
			pOutput->u8_phase_Idx[motion_T] = 0;
		}

		if(pOutput->u8_32322_BadEdit_cnt > 30 && pOutput->u1_32322_BadEdit_flag){
			pOutput->u8_phase_Idx[motion_T] = (pOutput->u8_32322_BadEdit_idx+1)%frc_cadTable[u8_cur_cadId].cad_outLen;
		}

		g_cadence_change_flag = _CASE_NULL;
	}
	else if ((pOutput->u32_Mot_sequence[motion_T] & frc_cadTable[u8_cur_cadId].cad_check) == frc_cadTable[u8_cur_cadId].cad_seq && pOutput->u8_phase_Idx[motion_T] != 0){
		#if 0
		if (u8_cur_cadId == _CAD_32322 && (pOutput->u8_phase_Idx[motion_T] == 7))
		{
			// 32322 <-> 32232 has no re-sync.
		}
		else if ((u8_cur_cadId == _CAD_32) && (pOutput->u8_phase_Idx[motion_T] == 2))
		{
		    //32<->23 has no resync
		}
		else
		#endif
		{
			pOutput->u1_cad_resync_true[motion_T] = 0;
			pOutput->u8_phase_Idx[motion_T] = 0;
		}
		g_cadence_change_flag = _CASE_NULL;
		pOutput->u8_32322_BadEdit_cnt = 0;
		pOutput->u1_32322_BadEdit_flag = 0;
		pOutput->u8_32322_BadEdit_idx = 0;
		pOutput->u8_Special_60_BadEdit_cnt = 0;
		pOutput->u1_Special_60_BadEdit_flag = 0;
		pOutput->u8_Special_60_BadEdit_idx = 0;
		pOutput->u8_Special_50_BadEdit_cnt = 0;
		pOutput->u1_Special_50_BadEdit_flag = 0;
		pOutput->u8_Special_50_BadEdit_idx = 0;
		pOutput->u8_Special_30_BadEdit_cnt = 0;
		pOutput->u1_Special_30_BadEdit_flag = 0;
		pOutput->u8_Special_30_BadEdit_idx = 0;
	}
	else{
		g_cadence_change_flag = _CASE_NULL;
		pOutput->u8_32322_BadEdit_cnt = 0;
		pOutput->u1_32322_BadEdit_flag = 0;
		pOutput->u8_32322_BadEdit_idx = 0;
		pOutput->u8_Special_60_BadEdit_cnt = 0;
		pOutput->u1_Special_60_BadEdit_flag = 0;
		pOutput->u8_Special_60_BadEdit_idx = 0;
		pOutput->u8_Special_50_BadEdit_cnt = 0;
		pOutput->u1_Special_50_BadEdit_flag = 0;
		pOutput->u8_Special_50_BadEdit_idx = 0;
		pOutput->u8_Special_30_BadEdit_cnt = 0;
		pOutput->u1_Special_30_BadEdit_flag = 0;
		pOutput->u8_Special_30_BadEdit_idx = 0;
	}

#else
	// resync rule
	if ((pOutput->u32_Mot_sequence[motion_T] & frc_cadTable[u8_cur_cadId].cad_check) == frc_cadTable[u8_cur_cadId].cad_seq \
		&& pOutput->u8_phase_Idx[motion_T] != 0)
	{
		#if 0
		if (u8_cur_cadId == _CAD_32322 && (pOutput->u8_phase_Idx[motion_T] == 7))
		{
			// 32322 <-> 32232 has no re-sync.
		}
		else if ((u8_cur_cadId == _CAD_32) && (pOutput->u8_phase_Idx[motion_T] == 2))
		{
		    //32<->23 has no resync
		}
		else
		#endif
		{
			pOutput->u1_cad_resync_true[motion_T] = 0;
			pOutput->u8_phase_Idx[motion_T] = 0;
		}
	}
#endif

#if 1  // 0720 kiwi mask
	pOutput->u8_phT_phase_Idx[motion_T] = pOutput->u8_phase_Idx[motion_T];
#else
    ////////////
    if (u8_cur_cadId == _CAD_VIDEO)
	{
		pOutput->u8_phT_phase_Idx[motion_T] = 0;
}
//	else if (u8_cur_cadId == _CAD_32)
//	{
//	    unsigned char  u1_curCad_motFlag      = cadence_seqFlag(frc_cadTable[u8_cur_cadId], pOutput->u8_phT_phase_Idx[motion_T]);
//	    unsigned char  u1_curFrm_isBig        = (u1_curCad_motFlag == 1? 1 : 0);
//		unsigned char  u1_seqFrm_isBig        = (pOutput->u32_Mot_sequence[motion_T] & 0x01) ==1? 1 : 0;

//		if (u1_seqFrm_isBig == u1_curFrm_isBig)
//		{
//		    pOutput->u8_phT_phase_Idx[motion_T] ++;
//		}
//		else if (pOutput->u8_phT_phase_Idx[motion_T] == 2 || pOutput->u8_phT_phase_Idx[motion_T] == 3)
//		{
//		    pOutput->u8_phT_phase_Idx[motion_T] ++;
//		}
//		else
//		{
//		    pOutput->u8_phT_phase_Idx[motion_T] = pOutput->u8_phT_phase_Idx[motion_T] + 2;
//		}


//		if (pOutput->u8_phT_phase_Idx[motion_T] >= frc_cadTable[u8_cur_cadId].cad_outLen)
//		{
//			pOutput->u8_phT_phase_Idx[motion_T] = pOutput->u8_phT_phase_Idx[motion_T] - frc_cadTable[u8_cur_cadId].cad_outLen;
//		}
//	}
	else
	{
		pOutput->u8_phT_phase_Idx[motion_T] ++;

		if (pOutput->u8_phT_phase_Idx[motion_T] == frc_cadTable[u8_cur_cadId].cad_outLen)
		{
			pOutput->u8_phT_phase_Idx[motion_T] = 0;
       }
       }



	if ((pOutput->u32_Mot_sequence[motion_T] & frc_cadTable[u8_cur_cadId].cad_check) == frc_cadTable[u8_cur_cadId].cad_seq \
	&& pOutput->u8_phT_phase_Idx[motion_T] != 0)// && pParam->u1_resync_en == 1
	{
		if (u8_cur_cadId == _CAD_32322 && (pOutput->u8_phT_phase_Idx[motion_T] == 7))
		{
			// 32322 <-> 32232 has no re-sync.
		}
		else if ((u8_cur_cadId == _CAD_32) && (pOutput->u8_phT_phase_Idx[motion_T] == 2))
		{
		    //32<->23 has no resync
		}
		else
		{
			pOutput->u8_phT_phase_Idx[motion_T] = 0;
		}
	}
#endif
	
	if(u1_dynCtrlPhase_en){ //d8fc[13]
		if(u1_dynCtrlPhase){
			WriteRegister(SOFTWARE1_SOFTWARE1_62_reg, 3, 3, 1);
		}
		else {
			WriteRegister(SOFTWARE1_SOFTWARE1_62_reg, 3, 3, 0);
		}
	}

	if(motion_T==_FILM_ALL&&log_en2) {
		rtd_printk(KERN_EMERG, TAG_NAME_MEMC, "[ID] phase id:%d cur:%d %d case:%d dyctrl:%d mot:%d %d%d%d%d%d%d%d%d%d%d%d%d\n", 
		pOutput->u8_phase_Idx[_FILM_ALL],u8_cur_cadId, pOutput->u8_cur_cadence_Id[motion_T], debug_info,
		u1_dynCtrlPhase, motion_T,
		(pOutput->u32_Mot_sequence[_FILM_ALL]>>11)&1,
		(pOutput->u32_Mot_sequence[_FILM_ALL]>>10)&1,
		(pOutput->u32_Mot_sequence[_FILM_ALL]>>9)&1,
		(pOutput->u32_Mot_sequence[_FILM_ALL]>>8)&1,
		(pOutput->u32_Mot_sequence[_FILM_ALL]>>7)&1,
		(pOutput->u32_Mot_sequence[_FILM_ALL]>>6)&1,
		(pOutput->u32_Mot_sequence[_FILM_ALL]>>5)&1,
		(pOutput->u32_Mot_sequence[_FILM_ALL]>>4)&1,
		(pOutput->u32_Mot_sequence[_FILM_ALL]>>3)&1,
		(pOutput->u32_Mot_sequence[_FILM_ALL]>>2)&1,
		(pOutput->u32_Mot_sequence[_FILM_ALL]>>1)&1,
		(pOutput->u32_Mot_sequence[_FILM_ALL]>>0)&1	);
	}	
return result;
}

#define juego_badedit_case_num	31
unsigned int u32_BadEdit_case_juego[juego_badedit_case_num] = {
	0xA1284E01, 0x84A13807, 0x1284E01D, 0x509C03B1, 0x13807621, // 5
	0x2700EC43, 0xC03B10C1, 0x03B10C11, 0x0EC43047, 0xC4304731, // 10
	0xD4B52D4F, 0xB25A96C7, 0x5A96C721, 0xD4B6390F, 0x5B1C87B1, // 15
	0xC721EC71, 0xD4B53D8F, 0x5A9EC7B1, 0xD4F63D8F, 0xC7B1ED31, // 20
	0xC7B1EC71, 0x6C7B1EC7, 0xA7B1EC7B, 0xEC7B1ED3, 0xC5315C95, // 25
	0xD4F6398F, 0xA7B1CC7B, 0xEC731EC7, 0x5314C54F, 0x8A62A7A9, // 30
	0x8A9EA639
};
VOID FilmDetect_BadEditProtect(const _PARAM_FilmDetectCtrl *pParam, _OUTPUT_FilmDetectCtrl *pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	unsigned int GoodEdit_Th = 1023; // 300;
	static unsigned int score_GoodEdit = 0, score_BadEdit_1 = 0, score_BadEdit_2 = 0;
	unsigned int u32_cur_motion = 0;
	unsigned char u8_k = 0, u1_match_flag = 0, u8_Cad_id = 0, u1_curCad_motFlag = 0, CadenceDetect_dbg_en = 0, BadEdit_en = 0;
	CadenceDetect_dbg_en = (rtd_inl(SOFTWARE1_SOFTWARE1_62_reg)>>30)&0x1;
	BadEdit_en = (rtd_inl(SOFTWARE1_SOFTWARE1_62_reg)>>3)&0x1;
	//GoodEdit_Th = (rtd_inl(SOFTWARE1_00_ADDR_SOFTWARE1_08_ADDR_reg)>>8)&0xfff; // 

	//if((Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) > 590) && (Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) < 610)){
	//if((pOutput->u32_input_frame_rate > 0x59000000) && (pOutput->u32_input_frame_rate < 0x61000000)){
	//	BadEdit_en = 0;
	//}

	if(BadEdit_en == 0){
		pOutput->u8_BadEdit_flag = 0;
		score_GoodEdit = 0;
	}else{
		#if 1
		u8_Cad_id = (pOutput->u8_cur_cadence_Id[_FILM_ALL] == _CAD_VIDEO) ? pOutput->u8_det_cadence_Id[_FILM_ALL] : pOutput->u8_cur_cadence_Id[_FILM_ALL];
		u32_cur_motion = s_pContext->_output_read_comreg.u27_ipme_aMot_rb;
		
		for(u8_k = 0; u8_k < juego_badedit_case_num; u8_k++){
			if(pOutput->u32_Mot_sequence[_FILM_ALL] == u32_BadEdit_case_juego[u8_k]){
				u1_match_flag = 1;
				break;
			}
		}

		if(u1_match_flag == 1 /*&& u32_cur_motion >= pParam->u27_Mot_all_min && u8_Cad_id != _CAD_VIDEO*/){
			if(score_BadEdit_1 < 640)
				score_BadEdit_1 = score_BadEdit_1+32;
		}
		else{
			if(score_BadEdit_1 > 0)
				score_BadEdit_1--;
		}
		#else
		pOutput->u8_cadence_sequence[0] = pOutput->u8_cur_cadence_Id[_FILM_ALL];
		for(u8_k = KEEP_FRAME_NUM-1; u8_k > 0; u8_k--)
		{
			if(pOutput->u8_cadence_sequence[u8_k] != pOutput->u8_cadence_sequence[u8_k-1]){
				score_BadEdit_1++;
			}

			if(u8_k < 9){
				if(pOutput->u8_cadence_sequence[u8_k] != pOutput->u8_cadence_sequence[u8_k-1]){
					score_BadEdit_2++;
				}
			}

			pOutput->u8_cadence_sequence[u8_k] = pOutput->u8_cadence_sequence[u8_k-1];
		}
		#endif

		//if((score_BadEdit_1 >= BadEdit_Th_1) && (score_BadEdit_2 >= BadEdit_Th_2)){
		if(score_BadEdit_1 >= 64){
			pOutput->u8_BadEdit_flag = 1;
			score_GoodEdit = 0;
		}else{
			if(score_GoodEdit < ((GoodEdit_Th*1)>>2)){
				score_GoodEdit = score_GoodEdit + 1;
			}
			else if(((GoodEdit_Th*1)>>2) <= score_GoodEdit && score_GoodEdit < ((GoodEdit_Th*2)>>2)){
				score_GoodEdit = score_GoodEdit + 2;
			}
			else if(((GoodEdit_Th*2)>>2) <= score_GoodEdit && score_GoodEdit < ((GoodEdit_Th*3)>>2)){
				score_GoodEdit = score_GoodEdit + 4;
			}
			else if(((GoodEdit_Th*3)>>2) <= score_GoodEdit && score_GoodEdit < GoodEdit_Th){
				score_GoodEdit = score_GoodEdit + 8;
			}
			else{
				score_GoodEdit++;
			}
		}

		if(score_GoodEdit >= GoodEdit_Th)
			pOutput->u8_BadEdit_flag = 0;

		if(CadenceDetect_dbg_en == 1){
			rtd_pr_memc_debug("[FilmDetect_BadEditProtect][CAD,%d,%d,][SEQ,%d,%d,][,%d,][,%d,%d,%d,][motion,%d,]\n\r", 
				pOutput->u8_det_cadence_Id[_FILM_ALL], pOutput->u8_cur_cadence_out[_FILM_ALL], 
				u1_curCad_motFlag, (pOutput->u32_Mot_sequence[_FILM_ALL]&0x1),
				pOutput->u8_BadEdit_flag, 
				score_BadEdit_1, score_BadEdit_2, score_GoodEdit, 
				s_pContext->_output_read_comreg.u27_ipme_aMot_rb);
		}
	}

	if(score_GoodEdit >= GoodEdit_Th+100)
		score_GoodEdit = GoodEdit_Th+100;

	if(pOutput->u8_BadEdit_flag == 1){
		//if(pOutput->u8_cur_cadence_Id[_FILM_ALL]!=_CAD_11112&&pOutput->u8_cur_cadence_Id[_FILM_ALL]!=_CAD_1112){
		pOutput->u8_cur_cadence_Id[_FILM_ALL] = _CAD_VIDEO; // _CAD_BAD_EDIT;
		pOutput->u8_phase_Idx[_FILM_ALL] = 0;
		//}
		
		//rtd_printk(KERN_CRIT, TAG_NAME_MEMC, "[INFO] BD %d\n", pOutput->u8_cur_cadence_Id[_FILM_ALL] );
	}
}

VOID CadenceChangeDetect(const _PARAM_FilmDetectCtrl *pParam, _OUTPUT_FilmDetectCtrl *pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	_PQLCONTEXT *s_pContext_m = GetPQLContext_m();
	PQL_INPUT_FRAME_RATE in_fmRate = 0;
	unsigned char u8_cur_cadId = 0, u8_pre_cadId = 0, u8_det_cadId = 0, u8_det_cadId_pre = 0, u8_ph_idx = 0, u8_ph_idx_pre = 0;
	unsigned int CadenceChangeDetect_en = 0, test_en = 0, log_en = 0, test_en2 = 0;
	unsigned char cur_motion_seq = 0, pre_motion_seq = 0;
	unsigned char small_motion_flag = 0, std_video_flag = 0;
	unsigned char small_motion_flag_chg = 0, std_video_flag_chg = 0, cadence_chg = 0;
	static unsigned char small_motion_flag_pre = 0, std_video_flag_pre = 0;
	unsigned int me_write = 0, me1_read_i = 0, me1_read_p = 0, me2_read_i = 0, me2_read_p = 0;
	unsigned int mc_write = 0, mc_read_i = 0, mc_read_p = 0;
	unsigned int test_32322_BadEdit_en = 0;

	#if CONFIG_MC_8_BUFFER
	unsigned char u8_i = 0;
	unsigned char Que_buffer_full_flag = 0;
	#endif
#if CONFIG_MEMC_OSD_PARAM
	unsigned char u1_adaptivestream_flag = MEMC_Lib_get_Adaptive_Stream_Flag();
#endif
	//unsigned char u1_callback_subscibe = 0;
	_RPC_system_setting_info* RPC_system_info_structure_table = NULL;
#if CONFIG_MEMC_OSD_PARAM
	int PlayRate = 0;
#endif

	u8_ph_idx = pOutput->u8_phase_Idx[_FILM_ALL];
	u8_ph_idx_pre = pOutput->u8_pre_phase_Idx[_FILM_ALL];
	in_fmRate = s_pContext->_external_data._input_frameRate;
	u8_cur_cadId = pOutput->u8_cur_cadence_Id[_FILM_ALL];
	u8_pre_cadId = pOutput->u8_pre_cadence_Id[_FILM_ALL];
	u8_det_cadId = pOutput->u8_det_cadence_Id[_FILM_ALL];
	u8_det_cadId_pre = pOutput->u8_det_cadence_Id_pre[_FILM_ALL];
	ReadRegister(SOFTWARE1_SOFTWARE1_60_reg, 31, 31, &CadenceChangeDetect_en);
	ReadRegister(SOFTWARE1_SOFTWARE1_60_reg, 30, 30, &log_en);
	ReadRegister(SOFTWARE1_SOFTWARE1_60_reg, 29, 29, &test_32322_BadEdit_en);
	ReadRegister(SOFTWARE1_SOFTWARE1_62_reg, 0, 0, &test_en);
	cur_motion_seq = pOutput->u32_Mot_sequence[_FILM_ALL]&0x1;
	pre_motion_seq = (pOutput->u32_Mot_sequence[_FILM_ALL]>>1)&0x1;
	small_motion_flag = ((pOutput->u32_Mot_sequence[_FILM_ALL]&0xffff) == 0x0) ? 1 : 0;
	std_video_flag = ((pOutput->u32_Mot_sequence[_FILM_ALL]&0xffff) == 0xffff) ? 1 : 0;
	small_motion_flag_chg = (small_motion_flag == small_motion_flag_pre) ? 0 : 1;
	std_video_flag_chg = (std_video_flag == std_video_flag_pre) ? 0 : 1;
	cadence_chg = (u8_pre_cadId == u8_cur_cadId) ? 0 : 1;
	ReadRegister(KPHASE_kphase_8C_reg, 0, 2, &me_write);
	ReadRegister(KPHASE_kphase_8C_reg, 4, 6, &me1_read_i);
	ReadRegister(KPHASE_kphase_8C_reg, 8, 10, &me1_read_p);
	ReadRegister(KPHASE_kphase_8C_reg, 20, 22, &me2_read_i);
	ReadRegister(KPHASE_kphase_8C_reg, 24, 26, &me2_read_p);
	ReadRegister(KPHASE_kphase_90_reg, 0, 2, &mc_write);
	ReadRegister(KPHASE_kphase_90_reg, 4, 6, &mc_read_i);
	ReadRegister(KPHASE_kphase_90_reg, 8, 10, &mc_read_p);
	ReadRegister(SOFTWARE1_SOFTWARE1_62_reg, 3, 3, &test_en2);

#ifdef CONFIG_MEDIA_SUPPORT
	//u1_callback_subscibe = vsc_get_main_win_apply_done_event_subscribe();
#endif
	RPC_system_info_structure_table = scaler_GetShare_Memory_RPC_system_setting_info_Struct();

	if(RPC_system_info_structure_table==NULL)
	{
		rtd_pr_memc_notice(" Mem4 = %p\n",RPC_system_info_structure_table);
		return;
	}
#if CONFIG_MEMC_OSD_PARAM
	PlayRate = RPC_system_info_structure_table->SCPU_ISRIINFO_TO_VCPU.YTPlayRate;
#endif

	if((LowDelay_mode == 1) || (s_pContext->_external_data._output_mode == _PQL_OUT_PC_MODE)
		|| (s_pContext->_external_data.u3_memcMode == MEMC_OFF)
		|| (s_pContext->_external_data.u3_memcMode == MEMC_USER && dejudder == 0)
		|| (test_32322_BadEdit_en && pOutput->u8_32322_BadEdit_cnt2 > 50)
#if CONFIG_MEMC_OSD_PARAM
		|| (u1_adaptivestream_flag /*&& u1_callback_subscibe*/ && (PlayRate != 0x10000 && PlayRate != 0x0)) // Q-16508
#endif
		|| (Get_DISPLAY_PANEL_TYPE()== P_VBY1_TO_HDMI_MEMC_FREERUN)
		|| (g_small_motion_flag && u8_cur_cadId == _CAD_VIDEO)){
		CadenceChangeDetect_en = 0;
		s_pContext_m->_external_data.u1_ReplaceCad = 0;
		s_pContext_m->_external_data.u8_ReplaceCase = 0;
		if(log_en){
			//rtd_pr_memc_notice("[PlayRate,%x,][g_small_motion_flag,%d,]\n\r", PlayRate, g_small_motion_flag);
			//pOutput->u1_mixMode = 0;
		}
	}


//if(test_en == 1){
#if CONFIG_MC_8_BUFFER
	if(CadenceChangeDetect_en == 1 && in_fmRate >= _PQL_IN_50HZ){
	if(pOutput->u1_mixMode == 0){
		if(cadence_chg){
			//clean que buffer
			for(u8_i = 0; u8_i < _CAD_CHANGE_QUE_BUF_FRM; u8_i++){
				g_cadence_change_que.state[u8_i] = 0;
				g_cadence_change_que.type[u8_i] = 0;
				g_cadence_change_que.mc_wrt_index[u8_i] = 0;
			}
			if((u8_pre_cadId == _CAD_VIDEO) && (u8_cur_cadId == _CAD_22)){
				g_cadence_change_flag = _CASE_11_TO_22_NEW;
			}
			else if((u8_pre_cadId == _CAD_VIDEO) && (u8_cur_cadId == _CAD_32)){
				g_cadence_change_flag = _CASE_11_TO_32_CHG;
			}
			else if((u8_pre_cadId == _CAD_VIDEO) && (u8_cur_cadId == _CAD_32322)){
				g_cadence_change_flag = _CASE_11_TO_32322_CHG;
			}
			else if((u8_pre_cadId == _CAD_VIDEO) && (u8_cur_cadId == _CAD_2224)){
				g_cadence_change_flag = _CASE_11_TO_2224_CHG;
			}
			else if((u8_pre_cadId == _CAD_VIDEO) && (u8_cur_cadId == _CAD_22224)){
				g_cadence_change_flag = _CASE_11_TO_22224_CHG;
			}
			else if((u8_pre_cadId == _CAD_VIDEO) && (u8_cur_cadId == _CAD_11112)){
				g_cadence_change_flag = _CASE_11_TO_11112_CHG;
			}
			else if((u8_pre_cadId == _CAD_22) && (u8_cur_cadId == _CAD_VIDEO)){
				g_cadence_change_flag = _CASE_22_TO_11_CHG;
			}
			else if((u8_pre_cadId == _CAD_22) && (u8_cur_cadId == _CAD_32)){
				g_cadence_change_flag = _CASE_22_TO_32_CHG;
			}
			else if((u8_pre_cadId == _CAD_22) && (u8_cur_cadId == _CAD_2224)){
				g_cadence_change_flag = _CASE_22_TO_2224_CHG;
			}
			else if((u8_pre_cadId == _CAD_22) && (u8_cur_cadId == _CAD_22224)){
				g_cadence_change_flag = _CASE_22_TO_2224_CHG;
			}
			else if((u8_pre_cadId == _CAD_32) && (u8_cur_cadId == _CAD_VIDEO)){
				g_cadence_change_flag = _CASE_32_TO_11_CHG;
			}
			else if((u8_pre_cadId == _CAD_32) && (u8_cur_cadId == _CAD_22)){
				g_cadence_change_flag = _CASE_32_TO_22_CHG;
			}
			else if((u8_pre_cadId == _CAD_32) && (u8_cur_cadId == _CAD_2224)){
				g_cadence_change_flag = _CASE_32_TO_2224_CHG;
			}
			else if((u8_pre_cadId == _CAD_32) && (u8_cur_cadId == _CAD_32322)){
				g_cadence_change_flag = _CASE_32_TO_32322_CHG;
			}
			else if((u8_pre_cadId == _CAD_32) && (u8_cur_cadId == _CAD_22224)){
				g_cadence_change_flag = _CASE_32_TO_22224_CHG;
			}
			else if((u8_pre_cadId == _CAD_32322) && (u8_cur_cadId == _CAD_VIDEO)){
				g_cadence_change_flag = _CASE_32322_TO_11_CHG;
			}
			else if((u8_pre_cadId == _CAD_32322) && (u8_cur_cadId == _CAD_22)){
				g_cadence_change_flag = _CASE_32322_TO_22_CHG;
			}
			else if((u8_pre_cadId == _CAD_32322) && (u8_cur_cadId == _CAD_32)){
				g_cadence_change_flag = _CASE_32322_TO_32_CHG;
			}
			else if((u8_pre_cadId == _CAD_32322) && (u8_cur_cadId == _CAD_2224)){
				g_cadence_change_flag = _CASE_32322_TO_2224_CHG;
			}
			else if((u8_pre_cadId == _CAD_32322) && (u8_cur_cadId == _CAD_22224)){
				g_cadence_change_flag = _CASE_32322_TO_22224_CHG;
			}
			else if((u8_pre_cadId == _CAD_2224) && (u8_cur_cadId == _CAD_VIDEO)){
				g_cadence_change_flag = _CASE_2224_TO_11_CHG;
			}
			else if((u8_pre_cadId == _CAD_2224) && (u8_cur_cadId == _CAD_22)){
				g_cadence_change_flag = _CASE_2224_TO_22_CHG;
			}
			else if((u8_pre_cadId == _CAD_2224) && (u8_cur_cadId == _CAD_32)){
				g_cadence_change_flag = _CASE_2224_TO_32_CHG;
			}
			else if((u8_pre_cadId == _CAD_2224) && (u8_cur_cadId == _CAD_32322)){
				g_cadence_change_flag = _CASE_2224_TO_32322_CHG;
			}
			else if((u8_pre_cadId == _CAD_22224) && (u8_cur_cadId == _CAD_VIDEO)){
				g_cadence_change_flag = _CASE_22224_TO_11_CHG;
			}
			else if((u8_pre_cadId == _CAD_22224) && (u8_cur_cadId == _CAD_22)){
				g_cadence_change_flag = _CASE_22224_TO_22_CHG;
			}
			else if((u8_pre_cadId == _CAD_22224) && (u8_cur_cadId == _CAD_32)){
				g_cadence_change_flag = _CASE_22224_TO_32_CHG;
			}
			else if((u8_pre_cadId == _CAD_22224) && (u8_cur_cadId == _CAD_32322)){
				g_cadence_change_flag = _CASE_22224_TO_32322_CHG;
			}
			else if((u8_pre_cadId == _CAD_11112) && (u8_cur_cadId == _CAD_VIDEO)){
				g_cadence_change_flag = _CASE_11112_TO_11_CHG;
			}
			else{
				g_cadence_change_flag = _CASE_NULL;
			}
		}
		else{
			g_cadence_change_flag = _CASE_NULL;
			if(u8_cur_cadId == _CAD_VIDEO /*&& small_motion_flag == 0*/){
				#if 1
				if(/*pOutput->u1_BadEdit_flag &&*/ test_en2 == 0){

				if((pOutput->u32_Mot_sequence[_FILM_ALL]&0x7f) == 0x55){ // 10 10 10 1
					if((pOutput->u32_Mot_sequence[_FILM_ALL]&0x7ff) == 0x455 // 1000 10 10 10 1
						|| (pOutput->u32_Mot_sequence[_FILM_ALL]&0x1fff) == 0x1155){ // 1000 10 10 10 10 1
						for(u8_i = 0; u8_i < _CAD_CHANGE_QUE_BUF_FRM; u8_i++){
							if(g_cadence_change_que.type[u8_i] == 0 && g_cadence_change_que.state[u8_i] == 0){
								g_cadence_change_que.type[u8_i] = 4222;
								g_cadence_change_que.mc_wrt_index[u8_i] = mc_write;
								g_cadence_change_que.state[u8_i] = 1;
								if(log_en){
								rtd_pr_memc_notice("[%s][g_cadence_change_que][%d][u8_i,%d,][mc_wrt_index,%d,][state,%d,][type,%d,]\n\r", 
									__FUNCTION__, __LINE__, u8_i, g_cadence_change_que.mc_wrt_index[u8_i], g_cadence_change_que.state[u8_i], g_cadence_change_que.type[u8_i]);
								}
								break;
							}else{
								Que_buffer_full_flag++;
							}
						}
					}
					else{
						for(u8_i = 0; u8_i < _CAD_CHANGE_QUE_BUF_FRM; u8_i++){
							if(g_cadence_change_que.type[u8_i] == 0 && g_cadence_change_que.state[u8_i] == 0){
								g_cadence_change_que.type[u8_i] = 222;
								g_cadence_change_que.mc_wrt_index[u8_i] = mc_write;
								g_cadence_change_que.state[u8_i] = 1;
								if(log_en){
								rtd_pr_memc_notice("[%s][g_cadence_change_que][%d][u8_i,%d,][mc_wrt_index,%d,][state,%d,][type,%d,]\n\r", 
									__FUNCTION__, __LINE__, u8_i, g_cadence_change_que.mc_wrt_index[u8_i], g_cadence_change_que.state[u8_i], g_cadence_change_que.type[u8_i]);
								}
								break;
							}else{
								Que_buffer_full_flag++;
							}
						}
					}
				}
/*
				else if((pOutput->u32_Mot_sequence[_FILM_ALL]&0xff) == 0xf5){ // 1 1 1 10 10 1
					for(u8_i = 0; u8_i < _CAD_CHANGE_QUE_BUF_FRM; u8_i++){
						if(g_cadence_change_que.type[u8_i] == 0 && g_cadence_change_que.state[u8_i] == 0){
							g_cadence_change_que.type[u8_i] = 12;
							g_cadence_change_que.mc_wrt_index[u8_i] = mc_write;
							g_cadence_change_que.state[u8_i] = 1;
							if(log_en){
							rtd_pr_memc_notice("[%s][g_cadence_change_que][%d][u8_i,%d,][mc_wrt_index,%d,][state,%d,][type,%d,]\n\r", 
								__FUNCTION__, __LINE__, u8_i, g_cadence_change_que.mc_wrt_index[u8_i], g_cadence_change_que.state[u8_i], g_cadence_change_que.type[u8_i]);
							}
							break;
						}else{
							Que_buffer_full_flag++;
						}
					}
				}
*/
				else if((pOutput->u32_Mot_sequence[_FILM_ALL]&0x3f) == 0x25){ // 100 10 1
					if((pOutput->u32_Mot_sequence[_FILM_ALL]&0xff) == 0xa5){ // 10 100 10 1
						for(u8_i = 0; u8_i < _CAD_CHANGE_QUE_BUF_FRM; u8_i++){
							if(g_cadence_change_que.type[u8_i] == 0 && g_cadence_change_que.state[u8_i] == 0){
								g_cadence_change_que.type[u8_i] = 232;
								g_cadence_change_que.mc_wrt_index[u8_i] = mc_write;
								g_cadence_change_que.state[u8_i] = 1;
								if(log_en){
								rtd_pr_memc_notice("[%s][g_cadence_change_que][%d][u8_i,%d,][mc_wrt_index,%d,][state,%d,][type,%d,]\n\r", 
									__FUNCTION__, __LINE__, u8_i, g_cadence_change_que.mc_wrt_index[u8_i], g_cadence_change_que.state[u8_i], g_cadence_change_que.type[u8_i]);
								}
								break;
							}else{
								Que_buffer_full_flag++;
							}
						}
					}
					else if((pOutput->u32_Mot_sequence[_FILM_ALL]&0x1ff) == 0x1e5){ // 1 1 1 100 10 1
						for(u8_i = 0; u8_i < _CAD_CHANGE_QUE_BUF_FRM; u8_i++){
							if(g_cadence_change_que.type[u8_i] == 0 && g_cadence_change_que.state[u8_i] == 0){
								g_cadence_change_que.type[u8_i] = 13;
								g_cadence_change_que.mc_wrt_index[u8_i] = mc_write;
								g_cadence_change_que.state[u8_i] = 1;
								if(log_en){
								rtd_pr_memc_notice("[%s][g_cadence_change_que][%d][u8_i,%d,][mc_wrt_index,%d,][state,%d,][type,%d,]\n\r", 
									__FUNCTION__, __LINE__, u8_i, g_cadence_change_que.mc_wrt_index[u8_i], g_cadence_change_que.state[u8_i], g_cadence_change_que.type[u8_i]);
								}
								break;
							}else{
								Que_buffer_full_flag++;
							}
						}
					}
				}
				else if((pOutput->u32_Mot_sequence[_FILM_ALL]&0x1ff) == 0x129){ // 100 10 100 1
					for(u8_i = 0; u8_i < _CAD_CHANGE_QUE_BUF_FRM; u8_i++){
						if(g_cadence_change_que.type[u8_i] == 0 && g_cadence_change_que.state[u8_i] == 0){
							g_cadence_change_que.type[u8_i] = 323;
							g_cadence_change_que.mc_wrt_index[u8_i] = mc_write;
							g_cadence_change_que.state[u8_i] = 1;
							if(log_en){
							rtd_pr_memc_notice("[%s][g_cadence_change_que][%d][u8_i,%d,][mc_wrt_index,%d,][state,%d,][type,%d,]\n\r", 
								__FUNCTION__, __LINE__, u8_i, g_cadence_change_que.mc_wrt_index[u8_i], g_cadence_change_que.state[u8_i], g_cadence_change_que.type[u8_i]);
							}
							break;
						}else{
							Que_buffer_full_flag++;
						}
					}
				}
				else if((pOutput->u32_Mot_sequence[_FILM_ALL]&0xff) == 0x95){ // 100 10 10 1
					for(u8_i = 0; u8_i < _CAD_CHANGE_QUE_BUF_FRM; u8_i++){
						if(g_cadence_change_que.type[u8_i] == 0 && g_cadence_change_que.state[u8_i] == 0){
							g_cadence_change_que.type[u8_i] = 322;
							g_cadence_change_que.mc_wrt_index[u8_i] = mc_write;
							g_cadence_change_que.state[u8_i] = 1;
							if(log_en){
							rtd_pr_memc_notice("[%s][g_cadence_change_que][%d][u8_i,%d,][mc_wrt_index,%d,][state,%d,][type,%d,]\n\r", 
								__FUNCTION__, __LINE__, u8_i, g_cadence_change_que.mc_wrt_index[u8_i], g_cadence_change_que.state[u8_i], g_cadence_change_que.type[u8_i]);
							}
							break;
						}else{
							Que_buffer_full_flag++;
						}
					}
				}
				else if((pOutput->u32_Mot_sequence[_FILM_ALL]&0x7f) == 0x4b){ // 100 10 1 1
					for(u8_i = 0; u8_i < _CAD_CHANGE_QUE_BUF_FRM; u8_i++){
						if(g_cadence_change_que.type[u8_i] == 0 && g_cadence_change_que.state[u8_i] == 0){
							g_cadence_change_que.type[u8_i] = 321;
							g_cadence_change_que.mc_wrt_index[u8_i] = mc_write;
							g_cadence_change_que.state[u8_i] = 1;
							if(log_en){
							rtd_pr_memc_notice("[%s][g_cadence_change_que][%d][u8_i,%d,][mc_wrt_index,%d,][state,%d,][type,%d,]\n\r", 
								__FUNCTION__, __LINE__, u8_i, g_cadence_change_que.mc_wrt_index[u8_i], g_cadence_change_que.state[u8_i], g_cadence_change_que.type[u8_i]);
							}
							break;
						}else{
							Que_buffer_full_flag++;
						}
					}
				}
				else if((pOutput->u32_Mot_sequence[_FILM_ALL]&0x3f) == 0x27){ // 100 1 1 1
					for(u8_i = 0; u8_i < _CAD_CHANGE_QUE_BUF_FRM; u8_i++){
						if(g_cadence_change_que.type[u8_i] == 0 && g_cadence_change_que.state[u8_i] == 0){
							g_cadence_change_que.type[u8_i] = 31;
							g_cadence_change_que.mc_wrt_index[u8_i] = mc_write;
							g_cadence_change_que.state[u8_i] = 1;
							if(log_en){
							rtd_pr_memc_notice("[%s][g_cadence_change_que][%d][u8_i,%d,][mc_wrt_index,%d,][state,%d,][type,%d,]\n\r", 
								__FUNCTION__, __LINE__, u8_i, g_cadence_change_que.mc_wrt_index[u8_i], g_cadence_change_que.state[u8_i], g_cadence_change_que.type[u8_i]);
							}
							break;
						}else{
							Que_buffer_full_flag++;
						}
					}
				}
				else if((pOutput->u32_Mot_sequence[_FILM_ALL]&0x1ff) == 0x115){ // 1000 10 10 1
					for(u8_i = 0; u8_i < _CAD_CHANGE_QUE_BUF_FRM; u8_i++){
						if(g_cadence_change_que.type[u8_i] == 0 && g_cadence_change_que.state[u8_i] == 0){
							g_cadence_change_que.type[u8_i] = 422;
							g_cadence_change_que.mc_wrt_index[u8_i] = mc_write;
							g_cadence_change_que.state[u8_i] = 1;
							if(log_en){
							rtd_pr_memc_notice("[%s][g_cadence_change_que][%d][u8_i,%d,][mc_wrt_index,%d,][state,%d,][type,%d,]\n\r", 
								__FUNCTION__, __LINE__, u8_i, g_cadence_change_que.mc_wrt_index[u8_i], g_cadence_change_que.state[u8_i], g_cadence_change_que.type[u8_i]);
							}
							break;
						}else{
							Que_buffer_full_flag++;
						}
					}
				}
				else if((pOutput->u32_Mot_sequence[_FILM_ALL]&0x1f) == 0x11){ // 1000 1
					for(u8_i = 0; u8_i < _CAD_CHANGE_QUE_BUF_FRM; u8_i++){
						if(g_cadence_change_que.type[u8_i] == 0 && g_cadence_change_que.state[u8_i] == 0){
							g_cadence_change_que.type[u8_i] = 4;
							g_cadence_change_que.mc_wrt_index[u8_i] = mc_write;
							g_cadence_change_que.state[u8_i] = 1;
							if(log_en){
							rtd_pr_memc_notice("[%s][g_cadence_change_que][%d][u8_i,%d,][mc_wrt_index,%d,][state,%d,][type,%d,]\n\r", 
								__FUNCTION__, __LINE__, u8_i, g_cadence_change_que.mc_wrt_index[u8_i], g_cadence_change_que.state[u8_i], g_cadence_change_que.type[u8_i]);
							}
							break;
						}else{
							Que_buffer_full_flag++;
						}
					}
				}
				else if((pOutput->u32_Mot_sequence[_FILM_ALL]&0xff) == 0xa9){ // 10 10 100 1
					for(u8_i = 0; u8_i < _CAD_CHANGE_QUE_BUF_FRM; u8_i++){
						if(g_cadence_change_que.type[u8_i] == 0 && g_cadence_change_que.state[u8_i] == 0){
							g_cadence_change_que.type[u8_i] = 223;
							g_cadence_change_que.mc_wrt_index[u8_i] = mc_write;
							g_cadence_change_que.state[u8_i] = 1;
							if(log_en){
							rtd_pr_memc_notice("[%s][g_cadence_change_que][%d][u8_i,%d,][mc_wrt_index,%d,][state,%d,][type,%d,]\n\r", 
								__FUNCTION__, __LINE__, u8_i, g_cadence_change_que.mc_wrt_index[u8_i], g_cadence_change_que.state[u8_i], g_cadence_change_que.type[u8_i]);
							}
							break;
						}else{
							Que_buffer_full_flag++;
						}
					}
				}
				else if((pOutput->u32_Mot_sequence[_FILM_ALL]&0x3f) == 0x2b){ // 10 10 1 1
					for(u8_i = 0; u8_i < _CAD_CHANGE_QUE_BUF_FRM; u8_i++){
						if(g_cadence_change_que.type[u8_i] == 0 && g_cadence_change_que.state[u8_i] == 0){
							g_cadence_change_que.type[u8_i] = 221;
							g_cadence_change_que.mc_wrt_index[u8_i] = mc_write;
							g_cadence_change_que.state[u8_i] = 1;
							if(log_en){
							rtd_pr_memc_notice("[%s][g_cadence_change_que][%d][u8_i,%d,][mc_wrt_index,%d,][state,%d,][type,%d,]\n\r", 
								__FUNCTION__, __LINE__, u8_i, g_cadence_change_que.mc_wrt_index[u8_i], g_cadence_change_que.state[u8_i], g_cadence_change_que.type[u8_i]);
							}
							break;
						}else{
							Que_buffer_full_flag++;
						}
					}
				}
				else if((pOutput->u32_Mot_sequence[_FILM_ALL]&0x3f) == 0x2d){ // 10 1 10 1
					for(u8_i = 0; u8_i < _CAD_CHANGE_QUE_BUF_FRM; u8_i++){
						if(g_cadence_change_que.type[u8_i] == 0 && g_cadence_change_que.state[u8_i] == 0){
							g_cadence_change_que.type[u8_i] = 212;
							g_cadence_change_que.mc_wrt_index[u8_i] = mc_write;
							g_cadence_change_que.state[u8_i] = 1;
							if(log_en){
							rtd_pr_memc_notice("[%s][g_cadence_change_que][%d][u8_i,%d,][mc_wrt_index,%d,][state,%d,][type,%d,]\n\r", 
								__FUNCTION__, __LINE__, u8_i, g_cadence_change_que.mc_wrt_index[u8_i], g_cadence_change_que.state[u8_i], g_cadence_change_que.type[u8_i]);
							}
							break;
						}else{
							Que_buffer_full_flag++;
						}
					}
				}
				else if((pOutput->u32_Mot_sequence[_FILM_ALL]&0x3f) == 0x35){ // 1 10 10 1
					for(u8_i = 0; u8_i < _CAD_CHANGE_QUE_BUF_FRM; u8_i++){
						if(g_cadence_change_que.type[u8_i] == 0 && g_cadence_change_que.state[u8_i] == 0){
							g_cadence_change_que.type[u8_i] = 122;
							g_cadence_change_que.mc_wrt_index[u8_i] = mc_write;
							g_cadence_change_que.state[u8_i] = 1;
							if(log_en){
							rtd_pr_memc_notice("[%s][g_cadence_change_que][%d][u8_i,%d,][mc_wrt_index,%d,][state,%d,][type,%d,]\n\r", 
								__FUNCTION__, __LINE__, u8_i, g_cadence_change_que.mc_wrt_index[u8_i], g_cadence_change_que.state[u8_i], g_cadence_change_que.type[u8_i]);
							}
							break;
						}else{
							Que_buffer_full_flag++;
						}
					}
				}
/*
				else if((pOutput->u32_Mot_sequence[_FILM_ALL]&0x1ffff) == 0x1f295){ // 1 1 1 1 100 10 100 10 10 1
					for(u8_i = 0; u8_i < _CAD_CHANGE_QUE_BUF_FRM; u8_i++){
						if(g_cadence_change_que.type[u8_i] == 0 && g_cadence_change_que.state[u8_i] == 0){
							g_cadence_change_que.type[u8_i] = 1322;
							g_cadence_change_que.mc_wrt_index[u8_i] = mc_write;
							g_cadence_change_que.state[u8_i] = 1;
							rtd_pr_memc_notice("[%s][g_cadence_change_que][%d][u8_i,%d,][mc_wrt_index,%d,][state,%d,][type,%d,]\n\r", 
								__FUNCTION__, __LINE__, u8_i, g_cadence_change_que.mc_wrt_index[u8_i], g_cadence_change_que.state[u8_i], g_cadence_change_que.type[u8_i]);
							break;
						}else{
							Que_buffer_full_flag++;
						}
					}
				}
*/
				}else{
					if((pOutput->u32_Mot_sequence[_FILM_ALL]&0x3) == 0x3){ // 1 1
						for(u8_i = 0; u8_i < _CAD_CHANGE_QUE_BUF_FRM; u8_i++){
							if(g_cadence_change_que.type[u8_i] == 0 && g_cadence_change_que.state[u8_i] == 0){
								g_cadence_change_que.type[u8_i] = 91;
								g_cadence_change_que.mc_wrt_index[u8_i] = mc_write;
								g_cadence_change_que.state[u8_i] = 1;
								if(log_en){
								rtd_pr_memc_notice("[%s][g_cadence_change_que][%d][u8_i,%d,][mc_wrt_index,%d,][state,%d,][type,%d,]\n\r", 
									__FUNCTION__, __LINE__, u8_i, g_cadence_change_que.mc_wrt_index[u8_i], g_cadence_change_que.state[u8_i], g_cadence_change_que.type[u8_i]);
								}
								break;
							}else{
								Que_buffer_full_flag++;
							}
						}
					}
					else if((pOutput->u32_Mot_sequence[_FILM_ALL]&0x7) == 0x5){ // 10 1
						for(u8_i = 0; u8_i < _CAD_CHANGE_QUE_BUF_FRM; u8_i++){
							if(g_cadence_change_que.type[u8_i] == 0 && g_cadence_change_que.state[u8_i] == 0){
								g_cadence_change_que.type[u8_i] = 92;
								g_cadence_change_que.mc_wrt_index[u8_i] = mc_write;
								g_cadence_change_que.state[u8_i] = 1;
								if(log_en){
								rtd_pr_memc_notice("[%s][g_cadence_change_que][%d][u8_i,%d,][mc_wrt_index,%d,][state,%d,][type,%d,]\n\r", 
									__FUNCTION__, __LINE__, u8_i, g_cadence_change_que.mc_wrt_index[u8_i], g_cadence_change_que.state[u8_i], g_cadence_change_que.type[u8_i]);
								}
								break;
							}else{
								Que_buffer_full_flag++;
							}
						}
					}
					else if((pOutput->u32_Mot_sequence[_FILM_ALL]&0xf) == 0x9){ // 100 1
						for(u8_i = 0; u8_i < _CAD_CHANGE_QUE_BUF_FRM; u8_i++){
							if(g_cadence_change_que.type[u8_i] == 0 && g_cadence_change_que.state[u8_i] == 0){
								g_cadence_change_que.type[u8_i] = 93;
								g_cadence_change_que.mc_wrt_index[u8_i] = mc_write;
								g_cadence_change_que.state[u8_i] = 1;
								if(log_en){
								rtd_pr_memc_notice("[%s][g_cadence_change_que][%d][u8_i,%d,][mc_wrt_index,%d,][state,%d,][type,%d,]\n\r", 
									__FUNCTION__, __LINE__, u8_i, g_cadence_change_que.mc_wrt_index[u8_i], g_cadence_change_que.state[u8_i], g_cadence_change_que.type[u8_i]);
								}
								break;
							}else{
								Que_buffer_full_flag++;
							}
						}
					}
					else if((pOutput->u32_Mot_sequence[_FILM_ALL]&0x1f) == 0x11){ // 1000 1
						for(u8_i = 0; u8_i < _CAD_CHANGE_QUE_BUF_FRM; u8_i++){
							if(g_cadence_change_que.type[u8_i] == 0 && g_cadence_change_que.state[u8_i] == 0){
								g_cadence_change_que.type[u8_i] = 94;
								g_cadence_change_que.mc_wrt_index[u8_i] = mc_write;
								g_cadence_change_que.state[u8_i] = 1;
								if(log_en){
								rtd_pr_memc_notice("[%s][g_cadence_change_que][%d][u8_i,%d,][mc_wrt_index,%d,][state,%d,][type,%d,]\n\r", 
									__FUNCTION__, __LINE__, u8_i, g_cadence_change_que.mc_wrt_index[u8_i], g_cadence_change_que.state[u8_i], g_cadence_change_que.type[u8_i]);
								}
								break;
							}else{
								Que_buffer_full_flag++;
							}
						}
					}
					else if((pOutput->u32_Mot_sequence[_FILM_ALL]&0x3f) == 0x21){ // 10000 1
						for(u8_i = 0; u8_i < _CAD_CHANGE_QUE_BUF_FRM; u8_i++){
							if(g_cadence_change_que.type[u8_i] == 0 && g_cadence_change_que.state[u8_i] == 0){
								g_cadence_change_que.type[u8_i] = 95;
								g_cadence_change_que.mc_wrt_index[u8_i] = mc_write;
								g_cadence_change_que.state[u8_i] = 1;
								if(log_en){
								rtd_pr_memc_notice("[%s][g_cadence_change_que][%d][u8_i,%d,][mc_wrt_index,%d,][state,%d,][type,%d,]\n\r", 
									__FUNCTION__, __LINE__, u8_i, g_cadence_change_que.mc_wrt_index[u8_i], g_cadence_change_que.state[u8_i], g_cadence_change_que.type[u8_i]);
								}
								break;
							}else{
								Que_buffer_full_flag++;
							}
						}
					}
					else{ // 10000 0
						if((pOutput->u32_Mot_sequence[_FILM_ALL]&0x3f) == 0x20){
							for(u8_i = 0; u8_i < _CAD_CHANGE_QUE_BUF_FRM; u8_i++){
								if(g_cadence_change_que.type[u8_i] == 0 && g_cadence_change_que.state[u8_i] == 0){
									g_cadence_change_que.type[u8_i] = 96;
									g_cadence_change_que.mc_wrt_index[u8_i] = mc_write;
									g_cadence_change_que.state[u8_i] = 1;
									if(log_en){
									rtd_pr_memc_notice("[%s][g_cadence_change_que][%d][u8_i,%d,][mc_wrt_index,%d,][state,%d,][type,%d,]\n\r", 
										__FUNCTION__, __LINE__, u8_i, g_cadence_change_que.mc_wrt_index[u8_i], g_cadence_change_que.state[u8_i], g_cadence_change_que.type[u8_i]);
									}
									break;
								}else{
									Que_buffer_full_flag++;
								}
							}
						}
						else if((pOutput->u32_Mot_sequence[_FILM_ALL]&0x3f) == 0x0){
							for(u8_i = 0; u8_i < _CAD_CHANGE_QUE_BUF_FRM; u8_i++){
								if(g_cadence_change_que.type[u8_i] == 0 && g_cadence_change_que.state[u8_i] == 0){
									g_cadence_change_que.type[u8_i] = 91;
									g_cadence_change_que.mc_wrt_index[u8_i] = mc_write;
									g_cadence_change_que.state[u8_i] = 1;
									if(log_en){
									rtd_pr_memc_notice("[%s][g_cadence_change_que][%d][u8_i,%d,][mc_wrt_index,%d,][state,%d,][type,%d,]\n\r", 
										__FUNCTION__, __LINE__, u8_i, g_cadence_change_que.mc_wrt_index[u8_i], g_cadence_change_que.state[u8_i], g_cadence_change_que.type[u8_i]);
									}
									break;
								}else{
									Que_buffer_full_flag++;
								}
							}
						}
					}
				}
				#else
				if(pre_motion_seq == 1 && cur_motion_seq == 0){
					for(u8_i = 0; u8_i < _CAD_CHANGE_QUE_BUF_FRM; u8_i++){
						if(g_cadence_change_que.type[u8_i] == 0){
							g_cadence_change_que.type[u8_i] = 2;
							rtd_pr_memc_notice("[%s][g_cadence_change_que][%d][u8_i,%d,][mc_wrt_index,%d,][state,%d,][type,%d,]\n\r", 
								__FUNCTION__, __LINE__, u8_i, g_cadence_change_que.mc_wrt_index[u8_i], g_cadence_change_que.state[u8_i], g_cadence_change_que.type[u8_i]);
							break;
						}else{
							Que_buffer_full_flag++;
						}
					}
				}
				else if(pre_motion_seq == 0 && cur_motion_seq == 1){
					for(u8_i = 0; u8_i < _CAD_CHANGE_QUE_BUF_FRM; u8_i++){
						if(g_cadence_change_que.type[u8_i] > 0 && g_cadence_change_que.state[u8_i] == 0){
							g_cadence_change_que.mc_wrt_index[u8_i] = mc_write;
							g_cadence_change_que.state[u8_i] = 1;
							rtd_pr_memc_notice("[%s][g_cadence_change_que][%d][u8_i,%d,][mc_wrt_index,%d,][state,%d,][type,%d,]\n\r", 
								__FUNCTION__, __LINE__, u8_i, g_cadence_change_que.mc_wrt_index[u8_i], g_cadence_change_que.state[u8_i], g_cadence_change_que.type[u8_i]);
						}
					}
				}
				else if(cur_motion_seq == 0){
					for(u8_i = 0; u8_i < _CAD_CHANGE_QUE_BUF_FRM; u8_i++){
						if(g_cadence_change_que.type[u8_i] > 0 && g_cadence_change_que.state[u8_i] == 0){
							g_cadence_change_que.type[u8_i]++;
							rtd_pr_memc_notice("[%s][g_cadence_change_que][%d][u8_i,%d,][mc_wrt_index,%d,][state,%d,][type,%d,]\n\r", 
								__FUNCTION__, __LINE__, u8_i, g_cadence_change_que.mc_wrt_index[u8_i], g_cadence_change_que.state[u8_i], g_cadence_change_que.type[u8_i]);
						}
					}
				}
				#endif
			}
			else if(u8_cur_cadId == _CAD_1225_T1){
				#if 0
				if((pOutput->u32_Mot_sequence[_FILM_ALL]&0x1ffffff) == 20097706){ //1001100101010101010101010
				rtd_printk(KERN_EMERG, TAG_NAME_MEMC, "[BE] 1225 0\n");
					for(u8_i = 0; u8_i < _CAD_CHANGE_QUE_BUF_FRM; u8_i++){
						if(g_cadence_change_que.type[u8_i] == 0 && g_cadence_change_que.state[u8_i] == 0){
							g_cadence_change_que.type[u8_i] = 1225;
							g_cadence_change_que.mc_wrt_index[u8_i] = mc_write;
							g_cadence_change_que.state[u8_i] = 1;
							if(log_en){
							rtd_pr_memc_notice("[%s][g_cadence_change_que][%d][u8_i,%d,][mc_wrt_index,%d,][state,%d,][type,%d,]\n\r", 
								__FUNCTION__, __LINE__, u8_i, g_cadence_change_que.mc_wrt_index[u8_i], g_cadence_change_que.state[u8_i], g_cadence_change_que.type[u8_i]);
							}
							rtd_printk(KERN_EMERG, TAG_NAME_MEMC, "[BE] 1225 F\n");
							break;
						}else{
							Que_buffer_full_flag++;
						}
					}
				}
				else if((pOutput->u32_Mot_sequence[_FILM_ALL]&0x1ffffff) == 5592403){ //10101010101010101010011
				rtd_printk(KERN_EMERG, TAG_NAME_MEMC, "[BE] 12251 0\n");
					for(u8_i = 0; u8_i < _CAD_CHANGE_QUE_BUF_FRM; u8_i++){
						if(g_cadence_change_que.type[u8_i] == 0 && g_cadence_change_que.state[u8_i] == 0){
							g_cadence_change_que.type[u8_i] = 12251;
							g_cadence_change_que.mc_wrt_index[u8_i] = mc_write;
							g_cadence_change_que.state[u8_i] = 1;
							if(log_en){
							rtd_pr_memc_notice("[%s][g_cadence_change_que][%d][u8_i,%d,][mc_wrt_index,%d,][state,%d,][type,%d,]\n\r", 
								__FUNCTION__, __LINE__, u8_i, g_cadence_change_que.mc_wrt_index[u8_i], g_cadence_change_que.state[u8_i], g_cadence_change_que.type[u8_i]);
							}
							rtd_printk(KERN_EMERG, TAG_NAME_MEMC, "[BE] 12251 F\n");
							break;
						}else{
							Que_buffer_full_flag++;
						}
					}
				}
				
				//rtd_printk(KERN_EMERG, TAG_NAME_MEMC, "[BE] 1225\n");
				if((pOutput->u32_Mot_sequence[_FILM_ALL]&0x1ffff) == 0x13329){ //100 1 100 1 100 10 100 1
				rtd_printk(KERN_EMERG, TAG_NAME_MEMC, "[BE] 1225 0\n");
					for(u8_i = 0; u8_i < _CAD_CHANGE_QUE_BUF_FRM; u8_i++){
						if(g_cadence_change_que.type[u8_i] == 0 && g_cadence_change_que.state[u8_i] == 0){
							g_cadence_change_que.type[u8_i] = 1225;
							g_cadence_change_que.mc_wrt_index[u8_i] = mc_write;
							g_cadence_change_que.state[u8_i] = 1;
							if(log_en){
							rtd_pr_memc_notice("[%s][g_cadence_change_que][%d][u8_i,%d,][mc_wrt_index,%d,][state,%d,][type,%d,]\n\r", 
								__FUNCTION__, __LINE__, u8_i, g_cadence_change_que.mc_wrt_index[u8_i], g_cadence_change_que.state[u8_i], g_cadence_change_que.type[u8_i]);
							}
							rtd_printk(KERN_EMERG, TAG_NAME_MEMC, "[BE] 1225 F\n");
							break;
						}else{
							Que_buffer_full_flag++;
						}
					}
				}
				//2222 313132222222
				//2222 3131112132			
				else if((pOutput->u32_Mot_sequence[_FILM_ALL]&0x1ffff) == 0x133D9){ //100 1 100 11110 1100 1
				rtd_printk(KERN_EMERG, TAG_NAME_MEMC, "[BE] 1225 1\n");
					for(u8_i = 0; u8_i < _CAD_CHANGE_QUE_BUF_FRM; u8_i++){
						if(g_cadence_change_que.type[u8_i] == 0 && g_cadence_change_que.state[u8_i] == 0){
							g_cadence_change_que.type[u8_i] = 1225;
							g_cadence_change_que.mc_wrt_index[u8_i] = mc_write;
							g_cadence_change_que.state[u8_i] = 1;
							if(log_en){
							rtd_pr_memc_notice("[%s][g_cadence_change_que][%d][u8_i,%d,][mc_wrt_index,%d,][state,%d,][type,%d,]\n\r", 
								__FUNCTION__, __LINE__, u8_i, g_cadence_change_que.mc_wrt_index[u8_i], g_cadence_change_que.state[u8_i], g_cadence_change_que.type[u8_i]);
							}
							rtd_pr_memc_notice("[BE] 1225 F\n");
							break;
						}else{
							Que_buffer_full_flag++;
						}
					}
				}
				#endif
			}
			else if(u8_cur_cadId == _CAD_22 /*&& small_motion_flag == 0*/){
				if((pOutput->u32_Mot_sequence[_FILM_ALL]&0xf) == 0xf){
					for(u8_i = 0; u8_i < _CAD_CHANGE_QUE_BUF_FRM; u8_i++){
						if(g_cadence_change_que.type[u8_i] == 0 && g_cadence_change_que.state[u8_i] == 0){
							g_cadence_change_que.type[u8_i] = 1;
							g_cadence_change_que.mc_wrt_index[u8_i] = mc_write;
							g_cadence_change_que.state[u8_i] = 1;
							if(log_en){
							rtd_pr_memc_notice("[%s][g_cadence_change_que][%d][u8_i,%d,][mc_wrt_index,%d,][state,%d,][type,%d,]\n\r", 
								__FUNCTION__, __LINE__, u8_i, g_cadence_change_que.mc_wrt_index[u8_i], g_cadence_change_que.state[u8_i], g_cadence_change_que.type[u8_i]);
							}
							break;
						}else{
							Que_buffer_full_flag++;
						}
					}
				}
				else if((pOutput->u32_Mot_sequence[_FILM_ALL]&0x7f) == 0x57){ // 10 10 1 1 1
					for(u8_i = 0; u8_i < _CAD_CHANGE_QUE_BUF_FRM; u8_i++){
						if(g_cadence_change_que.type[u8_i] == 0 && g_cadence_change_que.state[u8_i] == 0){
							g_cadence_change_que.type[u8_i] = 11;
							g_cadence_change_que.mc_wrt_index[u8_i] = mc_write;
							g_cadence_change_que.state[u8_i] = 1;
							if(log_en){
							rtd_pr_memc_notice("[%s][g_cadence_change_que][%d][u8_i,%d,][mc_wrt_index,%d,][state,%d,][type,%d,]\n\r", 
								__FUNCTION__, __LINE__, u8_i, g_cadence_change_que.mc_wrt_index[u8_i], g_cadence_change_que.state[u8_i], g_cadence_change_que.type[u8_i]);
							}
							break;
						}else{
							Que_buffer_full_flag++;
						}
					}
				}
				#if 1
				else if((pOutput->u32_Mot_sequence[_FILM_ALL]&0x3f) == 0x25){ // 100 10 1
					for(u8_i = 0; u8_i < _CAD_CHANGE_QUE_BUF_FRM; u8_i++){
						if(g_cadence_change_que.type[u8_i] == 0 && g_cadence_change_que.state[u8_i] == 0){
							g_cadence_change_que.type[u8_i] = 3;
							g_cadence_change_que.mc_wrt_index[u8_i] = mc_write;
							g_cadence_change_que.state[u8_i] = 1;
							if(log_en){
							rtd_pr_memc_notice("[%s][g_cadence_change_que][%d][u8_i,%d,][mc_wrt_index,%d,][state,%d,][type,%d,]\n\r", 
								__FUNCTION__, __LINE__, u8_i, g_cadence_change_que.mc_wrt_index[u8_i], g_cadence_change_que.state[u8_i], g_cadence_change_que.type[u8_i]);
							}
							break;
						}else{
							Que_buffer_full_flag++;
						}
					}
				}
				else if((pOutput->u32_Mot_sequence[_FILM_ALL]&0x1ff) == 0x129){ // 100 10 100 1
					for(u8_i = 0; u8_i < _CAD_CHANGE_QUE_BUF_FRM; u8_i++){
						if(g_cadence_change_que.type[u8_i] == 0 && g_cadence_change_que.state[u8_i] == 0){
							g_cadence_change_que.type[u8_i] = 32;
							g_cadence_change_que.mc_wrt_index[u8_i] = mc_write;
							g_cadence_change_que.state[u8_i] = 1;
							if(log_en){
							rtd_pr_memc_notice("[%s][g_cadence_change_que][%d][u8_i,%d,][mc_wrt_index,%d,][state,%d,][type,%d,]\n\r", 
								__FUNCTION__, __LINE__, u8_i, g_cadence_change_que.mc_wrt_index[u8_i], g_cadence_change_que.state[u8_i], g_cadence_change_que.type[u8_i]);
							}
							break;
						}else{
							Que_buffer_full_flag++;
						}
					}
				}
				else if((pOutput->u32_Mot_sequence[_FILM_ALL]&0xff) == 0x95){ // 100 10 10 1
					for(u8_i = 0; u8_i < _CAD_CHANGE_QUE_BUF_FRM; u8_i++){
						if(g_cadence_change_que.type[u8_i] == 0 && g_cadence_change_que.state[u8_i] == 0){
							g_cadence_change_que.type[u8_i] = 322;
							g_cadence_change_que.mc_wrt_index[u8_i] = mc_write;
							g_cadence_change_que.state[u8_i] = 1;
							if(log_en){
								rtd_pr_memc_notice("[%s][g_cadence_change_que][%d][u8_i,%d,][mc_wrt_index,%d,][state,%d,][type,%d,]\n\r", 
									__FUNCTION__, __LINE__, u8_i, g_cadence_change_que.mc_wrt_index[u8_i], g_cadence_change_que.state[u8_i], g_cadence_change_que.type[u8_i]);
							}
							break;
						}else{
							Que_buffer_full_flag++;
						}
					}
				}
				#else
				else if((pOutput->u32_Mot_sequence[_FILM_ALL]&0xf) == 0x9){ // 1001
					for(u8_i = 0; u8_i < _CAD_CHANGE_QUE_BUF_FRM; u8_i++){
						if(g_cadence_change_que.type[u8_i] == 0 && g_cadence_change_que.state[u8_i] == 0){
							g_cadence_change_que.type[u8_i] = 3;
							g_cadence_change_que.mc_wrt_index[u8_i] = mc_write;
							g_cadence_change_que.state[u8_i] = 1;
							if(log_en){
								rtd_pr_memc_notice("[%s][g_cadence_change_que][%d][u8_i,%d,][mc_wrt_index,%d,][state,%d,][type,%d,]\n\r", 
									__FUNCTION__, __LINE__, u8_i, g_cadence_change_que.mc_wrt_index[u8_i], g_cadence_change_que.state[u8_i], g_cadence_change_que.type[u8_i]);
							}
							break;
						}else{
							Que_buffer_full_flag++;
						}
					}
				}
				else if((pOutput->u32_Mot_sequence[_FILM_ALL]&0x3f) == 0x25){ // 100 10 1
					for(u8_i = 0; u8_i < _CAD_CHANGE_QUE_BUF_FRM; u8_i++){
						if(g_cadence_change_que.type[u8_i] == 0 && g_cadence_change_que.state[u8_i] == 0){
							g_cadence_change_que.type[u8_i] = 32;
							g_cadence_change_que.mc_wrt_index[u8_i] = mc_write;
							g_cadence_change_que.state[u8_i] = 1;
							if(log_en){
								rtd_pr_memc_notice("[%s][g_cadence_change_que][%d][u8_i,%d,][mc_wrt_index,%d,][state,%d,][type,%d,]\n\r", 
									__FUNCTION__, __LINE__, u8_i, g_cadence_change_que.mc_wrt_index[u8_i], g_cadence_change_que.state[u8_i], g_cadence_change_que.type[u8_i]);
							}
							break;
						}else{
							Que_buffer_full_flag++;
						}
					}
				}
				#endif
			}
			else if(u8_cur_cadId == _CAD_32 /*&& small_motion_flag == 0*/){
				if((pOutput->u32_Mot_sequence[_FILM_ALL]&0x3f) == 0x3f
					|| (pOutput->u32_Mot_sequence[_FILM_ALL]&0x3ff) == 0x25f // 100 10 1 1 1 1 1
					|| (pOutput->u32_Mot_sequence[_FILM_ALL]&0x1ffff) == 0x1feef){ // 111 1111 10 1 1 10 1 1 1 1 #408
					for(u8_i = 0; u8_i < _CAD_CHANGE_QUE_BUF_FRM; u8_i++){
						if(g_cadence_change_que.type[u8_i] == 0 && g_cadence_change_que.state[u8_i] == 0){
							g_cadence_change_que.type[u8_i] = 1;
							g_cadence_change_que.mc_wrt_index[u8_i] = mc_write;
							g_cadence_change_que.state[u8_i] = 1;
							if(log_en){
							rtd_pr_memc_notice("[%s][g_cadence_change_que][%d][u8_i,%d,][mc_wrt_index,%d,][state,%d,][type,%d,]\n\r", 
								__FUNCTION__, __LINE__, u8_i, g_cadence_change_que.mc_wrt_index[u8_i], g_cadence_change_que.state[u8_i], g_cadence_change_que.type[u8_i]);
							}
							break;
						}else{
							Que_buffer_full_flag++;
						}
					}
				}
				else if((pOutput->u32_Mot_sequence[_FILM_ALL]&0xff) == 0x97){ // 100 10 1 1 1
					for(u8_i = 0; u8_i < _CAD_CHANGE_QUE_BUF_FRM; u8_i++){
						if(g_cadence_change_que.type[u8_i] == 0 && g_cadence_change_que.state[u8_i] == 0){
							g_cadence_change_que.type[u8_i] = 11;
							g_cadence_change_que.mc_wrt_index[u8_i] = mc_write;
							g_cadence_change_que.state[u8_i] = 1;
							if(log_en){
							rtd_pr_memc_notice("[%s][g_cadence_change_que][%d][u8_i,%d,][mc_wrt_index,%d,][state,%d,][type,%d,]\n\r", 
								__FUNCTION__, __LINE__, u8_i, g_cadence_change_que.mc_wrt_index[u8_i], g_cadence_change_que.state[u8_i], g_cadence_change_que.type[u8_i]);
							}
							break;
						}else{
							Que_buffer_full_flag++;
						}
					}
				}
				else if((pOutput->u32_Mot_sequence[_FILM_ALL]&0x3ff) == 0xf // 00 0000 1 1 1 1
					|| (pOutput->u32_Mot_sequence[_FILM_ALL]&0xfff) == 0x2f // 00 0000 1 0 1 1 1 1
					|| (pOutput->u32_Mot_sequence[_FILM_ALL]&0x1fff) == 0x6f // 00 0000 1 1 0 1 1 1 1
					|| (pOutput->u32_Mot_sequence[_FILM_ALL]&0x7fffff) == 0x4afff7 // 100 1010 1111 1111 1111 0111 for #408
					|| (pOutput->u32_Mot_sequence[_FILM_ALL]&0xfff) == 0xff7 // 111 1111 10 1 1 1 #408
					|| (pOutput->u32_Mot_sequence[_FILM_ALL]&0xffff) == 0xff77){ // 111 1111 10 1 1 10 1 1 1 #408
					for(u8_i = 0; u8_i < _CAD_CHANGE_QUE_BUF_FRM; u8_i++){
						if(g_cadence_change_que.type[u8_i] == 0 && g_cadence_change_que.state[u8_i] == 0){
							g_cadence_change_que.type[u8_i] = 111;
							g_cadence_change_que.mc_wrt_index[u8_i] = mc_write;
							g_cadence_change_que.state[u8_i] = 1;
							if(log_en){
							rtd_pr_memc_notice("[%s][g_cadence_change_que][%d][u8_i,%d,][mc_wrt_index,%d,][state,%d,][type,%d,]\n\r", 
								__FUNCTION__, __LINE__, u8_i, g_cadence_change_que.mc_wrt_index[u8_i], g_cadence_change_que.state[u8_i], g_cadence_change_que.type[u8_i]);
							}
							break;
						}else{
							Que_buffer_full_flag++;
						}
					}
				}
				else if((pOutput->u32_Mot_sequence[_FILM_ALL]&0x3fff) == 0xd5 // 00 0000 1 10 10 10 1
					|| (pOutput->u32_Mot_sequence[_FILM_ALL]&0x7fff) == 0x1d5){ // 00 0000 1 1 10 10 10 1
					for(u8_i = 0; u8_i < _CAD_CHANGE_QUE_BUF_FRM; u8_i++){
						if(g_cadence_change_que.type[u8_i] == 0 && g_cadence_change_que.state[u8_i] == 0){
							g_cadence_change_que.type[u8_i] = 122;
							g_cadence_change_que.mc_wrt_index[u8_i] = mc_write;
							g_cadence_change_que.state[u8_i] = 1;
							if(log_en){
							rtd_pr_memc_notice("[%s][g_cadence_change_que][%d][u8_i,%d,][mc_wrt_index,%d,][state,%d,][type,%d,]\n\r", 
								__FUNCTION__, __LINE__, u8_i, g_cadence_change_que.mc_wrt_index[u8_i], g_cadence_change_que.state[u8_i], g_cadence_change_que.type[u8_i]);
							}
							break;
						}else{
							Que_buffer_full_flag++;
						}
					}
				}
				else if((pOutput->u32_Mot_sequence[_FILM_ALL]&0x3fff) == 0xf5){ // 00 0000 1 1 1 10 10 1
					for(u8_i = 0; u8_i < _CAD_CHANGE_QUE_BUF_FRM; u8_i++){
						if(g_cadence_change_que.type[u8_i] == 0 && g_cadence_change_que.state[u8_i] == 0){
							g_cadence_change_que.type[u8_i] = 1122;
							g_cadence_change_que.mc_wrt_index[u8_i] = mc_write;
							g_cadence_change_que.state[u8_i] = 1;
							if(log_en){
							rtd_pr_memc_notice("[%s][g_cadence_change_que][%d][u8_i,%d,][mc_wrt_index,%d,][state,%d,][type,%d,]\n\r", 
								__FUNCTION__, __LINE__, u8_i, g_cadence_change_que.mc_wrt_index[u8_i], g_cadence_change_que.state[u8_i], g_cadence_change_que.type[u8_i]);
							}
							break;
						}else{
							Que_buffer_full_flag++;
						}
					}
				}
				else if((pOutput->u32_Mot_sequence[_FILM_ALL]&0x3fff) == 0x3d5 // 0000 1 1 1 10 10 10 1
					|| (pOutput->u32_Mot_sequence[_FILM_ALL]&0xffff) == 0xf55 // 0000 1 1 1 10 10 10 10 1
					|| (pOutput->u32_Mot_sequence[_FILM_ALL]&0x3ffff) == 0x3d55 // 0000 1 1 1 10 10 10 10 10 1
					|| (pOutput->u32_Mot_sequence[_FILM_ALL]&0xfffff) == 0xf555 // 0000 1 1 1 10 10 10 10 10 10 1
					|| (pOutput->u32_Mot_sequence[_FILM_ALL]&0x7fff) == 0x755 // 0000 1 1 10 10 10 10 1
					|| (pOutput->u32_Mot_sequence[_FILM_ALL]&0xfff) == 0x355 // 0000 1 10 10 10 10 1
					|| (pOutput->u32_Mot_sequence[_FILM_ALL]&0x3fff) == 0xd55){ // 0000 1 10 10 10 10 10 1
					for(u8_i = 0; u8_i < _CAD_CHANGE_QUE_BUF_FRM; u8_i++){
						if(g_cadence_change_que.type[u8_i] == 0 && g_cadence_change_que.state[u8_i] == 0){
							g_cadence_change_que.type[u8_i] = 11222;
							g_cadence_change_que.mc_wrt_index[u8_i] = mc_write;
							g_cadence_change_que.state[u8_i] = 1;
							if(log_en){
							rtd_pr_memc_notice("[%s][g_cadence_change_que][%d][u8_i,%d,][mc_wrt_index,%d,][state,%d,][type,%d,]\n\r", 
								__FUNCTION__, __LINE__, u8_i, g_cadence_change_que.mc_wrt_index[u8_i], g_cadence_change_que.state[u8_i], g_cadence_change_que.type[u8_i]);
							}
							break;
						}else{
							Que_buffer_full_flag++;
						}
					}
				}
				else if((pOutput->u32_Mot_sequence[_FILM_ALL]&0x1fff) == 0x1f7d){ // 1 1 1 1 10 1 1 1 1 10 1
					for(u8_i = 0; u8_i < _CAD_CHANGE_QUE_BUF_FRM; u8_i++){
						if(g_cadence_change_que.type[u8_i] == 0 && g_cadence_change_que.state[u8_i] == 0){
							g_cadence_change_que.type[u8_i] = 11112;
							g_cadence_change_que.mc_wrt_index[u8_i] = mc_write;
							g_cadence_change_que.state[u8_i] = 1;
							if(log_en){
							rtd_pr_memc_notice("[%s][g_cadence_change_que][%d][u8_i,%d,][mc_wrt_index,%d,][state,%d,][type,%d,]\n\r", 
								__FUNCTION__, __LINE__, u8_i, g_cadence_change_que.mc_wrt_index[u8_i], g_cadence_change_que.state[u8_i], g_cadence_change_que.type[u8_i]);
							}
							break;
						}else{
							Que_buffer_full_flag++;
						}
					}
				}
				else if((pOutput->u32_Mot_sequence[_FILM_ALL]&0xff) == 0x95){ // 100 10 10 1
					if((pOutput->u32_Mot_sequence[_FILM_ALL]&0x3fff) == 0x2295){ // 1000 10 100 10 10 1
						// do nothing
					}
					else{
						for(u8_i = 0; u8_i < _CAD_CHANGE_QUE_BUF_FRM; u8_i++){
							if(g_cadence_change_que.type[u8_i] == 0 && g_cadence_change_que.state[u8_i] == 0){
								g_cadence_change_que.type[u8_i] = 2;
								g_cadence_change_que.mc_wrt_index[u8_i] = mc_write;
								g_cadence_change_que.state[u8_i] = 1;
								if(log_en){
								rtd_pr_memc_notice("[%s][g_cadence_change_que][%d][u8_i,%d,][mc_wrt_index,%d,][state,%d,][type,%d,]\n\r", 
									__FUNCTION__, __LINE__, u8_i, g_cadence_change_que.mc_wrt_index[u8_i], g_cadence_change_que.state[u8_i], g_cadence_change_que.type[u8_i]);
								}
								break;
							}else{
								Que_buffer_full_flag++;
							}
						}
					}
				}
				else if((pOutput->u32_Mot_sequence[_FILM_ALL]&0x7ff) == 0x4a9){ // 100 10 10 100 1
					for(u8_i = 0; u8_i < _CAD_CHANGE_QUE_BUF_FRM; u8_i++){
						if(g_cadence_change_que.type[u8_i] == 0 && g_cadence_change_que.state[u8_i] == 0){
							g_cadence_change_que.type[u8_i] = 23;
							g_cadence_change_que.mc_wrt_index[u8_i] = mc_write;
							g_cadence_change_que.state[u8_i] = 1;
							if(log_en){
							rtd_pr_memc_notice("[%s][g_cadence_change_que][%d][u8_i,%d,][mc_wrt_index,%d,][state,%d,][type,%d,]\n\r", 
								__FUNCTION__, __LINE__, u8_i, g_cadence_change_que.mc_wrt_index[u8_i], g_cadence_change_que.state[u8_i], g_cadence_change_que.type[u8_i]);
							}
							break;
						}else{
							Que_buffer_full_flag++;
						}
					}
				}
				else if((pOutput->u32_Mot_sequence[_FILM_ALL]&0x7f) == 0x49){ // 100 100 1
					for(u8_i = 0; u8_i < _CAD_CHANGE_QUE_BUF_FRM; u8_i++){
						if(g_cadence_change_que.type[u8_i] == 0 && g_cadence_change_que.state[u8_i] == 0){
							g_cadence_change_que.type[u8_i] = 33;
							g_cadence_change_que.mc_wrt_index[u8_i] = mc_write;
							g_cadence_change_que.state[u8_i] = 1;
							if(log_en){
							rtd_pr_memc_notice("[%s][g_cadence_change_que][%d][u8_i,%d,][mc_wrt_index,%d,][state,%d,][type,%d,]\n\r", 
								__FUNCTION__, __LINE__, u8_i, g_cadence_change_que.mc_wrt_index[u8_i], g_cadence_change_que.state[u8_i], g_cadence_change_que.type[u8_i]);
							}
							break;
						}else{
							Que_buffer_full_flag++;
						}
					}
				}
				else if((pOutput->u32_Mot_sequence[_FILM_ALL]&0x3ff) == 0x257){ // 100 10 10 1 1 1
					for(u8_i = 0; u8_i < _CAD_CHANGE_QUE_BUF_FRM; u8_i++){
						if(g_cadence_change_que.type[u8_i] == 0 && g_cadence_change_que.state[u8_i] == 0){
							g_cadence_change_que.type[u8_i] = 21;
							g_cadence_change_que.mc_wrt_index[u8_i] = mc_write;
							g_cadence_change_que.state[u8_i] = 1;
							rtd_pr_memc_notice("[%s][g_cadence_change_que][%d][u8_i,%d,][mc_wrt_index,%d,][state,%d,][type,%d,]\n\r", 
								__FUNCTION__, __LINE__, u8_i, g_cadence_change_que.mc_wrt_index[u8_i], g_cadence_change_que.state[u8_i], g_cadence_change_que.type[u8_i]);
							break;
						}else{
							Que_buffer_full_flag++;
						}
					}
				}
				else if((pOutput->u32_Mot_sequence[_FILM_ALL]&0xfff) == 0x95f){ // 100 10 10 1 1 1 1 1
					for(u8_i = 0; u8_i < _CAD_CHANGE_QUE_BUF_FRM; u8_i++){
						if(g_cadence_change_que.type[u8_i] == 0 && g_cadence_change_que.state[u8_i] == 0){
							g_cadence_change_que.type[u8_i] = 211;
							g_cadence_change_que.mc_wrt_index[u8_i] = mc_write;
							g_cadence_change_que.state[u8_i] = 1;
							rtd_pr_memc_notice("[%s][g_cadence_change_que][%d][u8_i,%d,][mc_wrt_index,%d,][state,%d,][type,%d,]\n\r", 
								__FUNCTION__, __LINE__, u8_i, g_cadence_change_que.mc_wrt_index[u8_i], g_cadence_change_que.state[u8_i], g_cadence_change_que.type[u8_i]);
							break;
						}else{
							Que_buffer_full_flag++;
						}
					}
				}
				else if((pOutput->u32_Mot_sequence[_FILM_ALL]&0x7ff) == 0x4ad){ // 100 10 10 1 10 1
					for(u8_i = 0; u8_i < _CAD_CHANGE_QUE_BUF_FRM; u8_i++){
						if(g_cadence_change_que.type[u8_i] == 0 && g_cadence_change_que.state[u8_i] == 0){
							g_cadence_change_que.type[u8_i] = 212;
							g_cadence_change_que.mc_wrt_index[u8_i] = mc_write;
							g_cadence_change_que.state[u8_i] = 1;
							if(log_en){
							rtd_pr_memc_notice("[%s][g_cadence_change_que][%d][u8_i,%d,][mc_wrt_index,%d,][state,%d,][type,%d,]\n\r", 
								__FUNCTION__, __LINE__, u8_i, g_cadence_change_que.mc_wrt_index[u8_i], g_cadence_change_que.state[u8_i], g_cadence_change_que.type[u8_i]);
							}
							break;
						}else{
							Que_buffer_full_flag++;
						}
					}
				}
				else if((pOutput->u32_Mot_sequence[_FILM_ALL]&0x3ff) == 0x255 // 100 10 10 10 1
					|| (pOutput->u32_Mot_sequence[_FILM_ALL]&0xfff) == 0x955){ // 100 10 10 10 10 1
					for(u8_i = 0; u8_i < _CAD_CHANGE_QUE_BUF_FRM; u8_i++){
						if(g_cadence_change_que.type[u8_i] == 0 && g_cadence_change_que.state[u8_i] == 0){
							g_cadence_change_que.type[u8_i] = 222; // 3222 & 32222
							g_cadence_change_que.mc_wrt_index[u8_i] = mc_write;
							g_cadence_change_que.state[u8_i] = 1;
							if(log_en){
							rtd_pr_memc_notice("[%s][g_cadence_change_que][%d][u8_i,%d,][mc_wrt_index,%d,][state,%d,][type,%d,]\n\r", 
								__FUNCTION__, __LINE__, u8_i, g_cadence_change_que.mc_wrt_index[u8_i], g_cadence_change_que.state[u8_i], g_cadence_change_que.type[u8_i]);
							}
							break;
						}else{
							Que_buffer_full_flag++;
						}
					}
				}
				else if((pOutput->u32_Mot_sequence[_FILM_ALL]&0x7ff) == 0x555){ // 10 10 10 10 10 1
					for(u8_i = 0; u8_i < _CAD_CHANGE_QUE_BUF_FRM; u8_i++){
						if(g_cadence_change_que.type[u8_i] == 0 && g_cadence_change_que.state[u8_i] == 0){
							g_cadence_change_que.type[u8_i] = 22;
							g_cadence_change_que.mc_wrt_index[u8_i] = mc_write;
							g_cadence_change_que.state[u8_i] = 1;
							if(log_en){
							rtd_pr_memc_notice("[%s][g_cadence_change_que][%d][u8_i,%d,][mc_wrt_index,%d,][state,%d,][type,%d,]\n\r", 
								__FUNCTION__, __LINE__, u8_i, g_cadence_change_que.mc_wrt_index[u8_i], g_cadence_change_que.state[u8_i], g_cadence_change_que.type[u8_i]);
							}
							break;
						}else{
							Que_buffer_full_flag++;
						}
					}
				}
				else if((pOutput->u32_Mot_sequence[_FILM_ALL]&0x3ff) == 0x2a9){ // 10 10 10 100 1
					for(u8_i = 0; u8_i < _CAD_CHANGE_QUE_BUF_FRM; u8_i++){
						if(g_cadence_change_que.type[u8_i] == 0 && g_cadence_change_que.state[u8_i] == 0){
							g_cadence_change_que.type[u8_i] = 223;
							g_cadence_change_que.mc_wrt_index[u8_i] = mc_write;
							g_cadence_change_que.state[u8_i] = 1;
							if(log_en){
							rtd_pr_memc_notice("[%s][g_cadence_change_que][%d][u8_i,%d,][mc_wrt_index,%d,][state,%d,][type,%d,]\n\r", 
								__FUNCTION__, __LINE__, u8_i, g_cadence_change_que.mc_wrt_index[u8_i], g_cadence_change_que.state[u8_i], g_cadence_change_que.type[u8_i]);
							}
							break;
						}else{
							Que_buffer_full_flag++;
						}
					}
				}
				else if((pOutput->u32_Mot_sequence[_FILM_ALL]&0x1f) == 0x11){ // 1000 1
					if((pOutput->u32_Mot_sequence[_FILM_ALL]&0x3ff) == 0x251){ // 100 10 1000 1
						for(u8_i = 0; u8_i < _CAD_CHANGE_QUE_BUF_FRM; u8_i++){
							if(g_cadence_change_que.type[u8_i] == 0 && g_cadence_change_que.state[u8_i] == 0){
								g_cadence_change_que.type[u8_i] = 324;
								g_cadence_change_que.mc_wrt_index[u8_i] = mc_write;
								g_cadence_change_que.state[u8_i] = 1;
							if(log_en){
								rtd_pr_memc_notice("[%s][g_cadence_change_que][%d][u8_i,%d,][mc_wrt_index,%d,][state,%d,][type,%d,]\n\r", 
									__FUNCTION__, __LINE__, u8_i, g_cadence_change_que.mc_wrt_index[u8_i], g_cadence_change_que.state[u8_i], g_cadence_change_que.type[u8_i]);
							}
								break;
							}else{
								Que_buffer_full_flag++;
							}
						}
					}
					else if((pOutput->u32_Mot_sequence[_FILM_ALL]&0x7ff) == 0x4b1){ // 100 10 1 1000 1
						for(u8_i = 0; u8_i < _CAD_CHANGE_QUE_BUF_FRM; u8_i++){
							if(g_cadence_change_que.type[u8_i] == 0 && g_cadence_change_que.state[u8_i] == 0){
								g_cadence_change_que.type[u8_i] = 3214;
								g_cadence_change_que.mc_wrt_index[u8_i] = mc_write;
								g_cadence_change_que.state[u8_i] = 1;
							if(log_en){
								rtd_pr_memc_notice("[%s][g_cadence_change_que][%d][u8_i,%d,][mc_wrt_index,%d,][state,%d,][type,%d,]\n\r", 
									__FUNCTION__, __LINE__, u8_i, g_cadence_change_que.mc_wrt_index[u8_i], g_cadence_change_que.state[u8_i], g_cadence_change_que.type[u8_i]);
							}
								break;
							}else{
								Que_buffer_full_flag++;
							}
						}
					}
					else{
						for(u8_i = 0; u8_i < _CAD_CHANGE_QUE_BUF_FRM; u8_i++){
							if(g_cadence_change_que.type[u8_i] == 0 && g_cadence_change_que.state[u8_i] == 0){
								g_cadence_change_que.type[u8_i] = 4;
								g_cadence_change_que.mc_wrt_index[u8_i] = mc_write;
								g_cadence_change_que.state[u8_i] = 1;
							if(log_en){
								rtd_pr_memc_notice("[%s][g_cadence_change_que][%d][u8_i,%d,][mc_wrt_index,%d,][state,%d,][type,%d,]\n\r", 
									__FUNCTION__, __LINE__, u8_i, g_cadence_change_que.mc_wrt_index[u8_i], g_cadence_change_que.state[u8_i], g_cadence_change_que.type[u8_i]);
							}
								break;
							}else{
								Que_buffer_full_flag++;
							}
						}
					}
				}
				else if((pOutput->u32_Mot_sequence[_FILM_ALL]&0x7f) == 0x45 // 1000 10 1
					|| (pOutput->u32_Mot_sequence[_FILM_ALL]&0x1ff) == 0x115 // 1000 10 10 1
					|| (pOutput->u32_Mot_sequence[_FILM_ALL]&0x7ff) == 0x455 // 1000 10 10 10 1
					|| (pOutput->u32_Mot_sequence[_FILM_ALL]&0x1fff) == 0x1155){ // 1000 10 10 10 10 1
					for(u8_i = 0; u8_i < _CAD_CHANGE_QUE_BUF_FRM; u8_i++){
						if(g_cadence_change_que.type[u8_i] == 0 && g_cadence_change_que.state[u8_i] == 0){
							g_cadence_change_que.type[u8_i] = 42; // 42 & 422 & 4222 & 42222
							g_cadence_change_que.mc_wrt_index[u8_i] = mc_write;
							g_cadence_change_que.state[u8_i] = 1;
							if(log_en){
							rtd_pr_memc_notice("[%s][g_cadence_change_que][%d][u8_i,%d,][mc_wrt_index,%d,][state,%d,][type,%d,]\n\r", 
								__FUNCTION__, __LINE__, u8_i, g_cadence_change_que.mc_wrt_index[u8_i], g_cadence_change_que.state[u8_i], g_cadence_change_que.type[u8_i]);
							}
							break;
						}else{
							Que_buffer_full_flag++;
						}
					}
				}
				else if((pOutput->u32_Mot_sequence[_FILM_ALL]&0x3ff) == 0x229){ // 1000 10 100 1
					for(u8_i = 0; u8_i < _CAD_CHANGE_QUE_BUF_FRM; u8_i++){
						if(g_cadence_change_que.type[u8_i] == 0 && g_cadence_change_que.state[u8_i] == 0){
							g_cadence_change_que.type[u8_i] = 423;
							g_cadence_change_que.mc_wrt_index[u8_i] = mc_write;
							g_cadence_change_que.state[u8_i] = 1;
							if(log_en){
							rtd_pr_memc_notice("[%s][g_cadence_change_que][%d][u8_i,%d,][mc_wrt_index,%d,][state,%d,][type,%d,]\n\r", 
								__FUNCTION__, __LINE__, u8_i, g_cadence_change_que.mc_wrt_index[u8_i], g_cadence_change_que.state[u8_i], g_cadence_change_que.type[u8_i]);
							}
							break;
						}else{
							Que_buffer_full_flag++;
						}
					}
				}
				else if((pOutput->u32_Mot_sequence[_FILM_ALL]&0x3ff) == 0x253){ // 100 10 100 1 1
					for(u8_i = 0; u8_i < _CAD_CHANGE_QUE_BUF_FRM; u8_i++){
						if(g_cadence_change_que.type[u8_i] == 0 && g_cadence_change_que.state[u8_i] == 0){
							g_cadence_change_que.type[u8_i] = 31;
							g_cadence_change_que.mc_wrt_index[u8_i] = mc_write;
							g_cadence_change_que.state[u8_i] = 1;
							if(log_en){
							rtd_pr_memc_notice("[%s][g_cadence_change_que][%d][u8_i,%d,][mc_wrt_index,%d,][state,%d,][type,%d,]\n\r", 
								__FUNCTION__, __LINE__, u8_i, g_cadence_change_que.mc_wrt_index[u8_i], g_cadence_change_que.state[u8_i], g_cadence_change_que.type[u8_i]);
							}
							break;
						}else{
							Que_buffer_full_flag++;
						}
					}
				}
				else if((pOutput->u32_Mot_sequence[_FILM_ALL]&0x7ff) == 0x4a7 // 100 10 100 1 1 1
					|| (pOutput->u32_Mot_sequence[_FILM_ALL]&0xfff) == 0x94f){  // 100 10 100 1 1 1 1
					for(u8_i = 0; u8_i < _CAD_CHANGE_QUE_BUF_FRM; u8_i++){
						if(g_cadence_change_que.type[u8_i] == 0 && g_cadence_change_que.state[u8_i] == 0){
							g_cadence_change_que.type[u8_i] = 311;
							g_cadence_change_que.mc_wrt_index[u8_i] = mc_write;
							g_cadence_change_que.state[u8_i] = 1;
							if(log_en){
							rtd_pr_memc_notice("[%s][g_cadence_change_que][%d][u8_i,%d,][mc_wrt_index,%d,][state,%d,][type,%d,]\n\r", 
								__FUNCTION__, __LINE__, u8_i, g_cadence_change_que.mc_wrt_index[u8_i], g_cadence_change_que.state[u8_i], g_cadence_change_que.type[u8_i]);
							}
							break;
						}else{
							Que_buffer_full_flag++;
						}
					}
				}
				else if((pOutput->u32_Mot_sequence[_FILM_ALL]&0x1fff) == 0x129f){ // 100 10 100 1 1 1 1 1
					for(u8_i = 0; u8_i < _CAD_CHANGE_QUE_BUF_FRM; u8_i++){
						if(g_cadence_change_que.type[u8_i] == 0 && g_cadence_change_que.state[u8_i] == 0){
							g_cadence_change_que.type[u8_i] = 3111;
							g_cadence_change_que.mc_wrt_index[u8_i] = mc_write;
							g_cadence_change_que.state[u8_i] = 1;
							if(log_en){
							rtd_pr_memc_notice("[%s][g_cadence_change_que][%d][u8_i,%d,][mc_wrt_index,%d,][state,%d,][type,%d,]\n\r", 
								__FUNCTION__, __LINE__, u8_i, g_cadence_change_que.mc_wrt_index[u8_i], g_cadence_change_que.state[u8_i], g_cadence_change_que.type[u8_i]);
							}
							break;
						}else{
							Que_buffer_full_flag++;
						}
					}
				}
				else if((pOutput->u32_Mot_sequence[_FILM_ALL]&0x1fff) == 0x1299 // 100 10 100 1 100 1
					|| (pOutput->u32_Mot_sequence[_FILM_ALL]&0x3fff) == 0x2539){ // 100 10 100 1 1 100 1
					for(u8_i = 0; u8_i < _CAD_CHANGE_QUE_BUF_FRM; u8_i++){
						if(g_cadence_change_que.type[u8_i] == 0 && g_cadence_change_que.state[u8_i] == 0){
							g_cadence_change_que.type[u8_i] = 313;
							g_cadence_change_que.mc_wrt_index[u8_i] = mc_write;
							g_cadence_change_que.state[u8_i] = 1;
							if(log_en){
							rtd_pr_memc_notice("[%s][g_cadence_change_que][%d][u8_i,%d,][mc_wrt_index,%d,][state,%d,][type,%d,]\n\r", 
								__FUNCTION__, __LINE__, u8_i, g_cadence_change_que.mc_wrt_index[u8_i], g_cadence_change_que.state[u8_i], g_cadence_change_que.type[u8_i]);
							}
							break;
						}else{
							Que_buffer_full_flag++;
						}
					}
				}
				else if((pOutput->u32_Mot_sequence[_FILM_ALL]&0x1ff) == 0x127){ // 100 100 1 1 1
					for(u8_i = 0; u8_i < _CAD_CHANGE_QUE_BUF_FRM; u8_i++){
						if(g_cadence_change_que.type[u8_i] == 0 && g_cadence_change_que.state[u8_i] == 0){
							g_cadence_change_que.type[u8_i] = 3311;
							g_cadence_change_que.mc_wrt_index[u8_i] = mc_write;
							g_cadence_change_que.state[u8_i] = 1;
							if(log_en){
							rtd_pr_memc_notice("[%s][g_cadence_change_que][%d][u8_i,%d,][mc_wrt_index,%d,][state,%d,][type,%d,]\n\r", 
								__FUNCTION__, __LINE__, u8_i, g_cadence_change_que.mc_wrt_index[u8_i], g_cadence_change_que.state[u8_i], g_cadence_change_que.type[u8_i]);
							}
							break;
						}else{
							Que_buffer_full_flag++;
						}
					}
				}
			}
			else if(u8_cur_cadId == _CAD_32322){
				if((pOutput->u32_Mot_sequence[_FILM_ALL]&0x1f) == 0x1f){
					for(u8_i = 0; u8_i < _CAD_CHANGE_QUE_BUF_FRM; u8_i++){
						if(g_cadence_change_que.type[u8_i] == 0 && g_cadence_change_que.state[u8_i] == 0){
							g_cadence_change_que.type[u8_i] = 1;
							g_cadence_change_que.mc_wrt_index[u8_i] = mc_write;
							g_cadence_change_que.state[u8_i] = 1;
							if(log_en){
							rtd_pr_memc_notice("[%s][g_cadence_change_que][%d][u8_i,%d,][mc_wrt_index,%d,][state,%d,][type,%d,]\n\r", 
								__FUNCTION__, __LINE__, u8_i, g_cadence_change_que.mc_wrt_index[u8_i], g_cadence_change_que.state[u8_i], g_cadence_change_que.type[u8_i]);
							}
							break;
						}else{
							Que_buffer_full_flag++;
						}
					}
				}
				else if((pOutput->u32_Mot_sequence[_FILM_ALL]&0xffff) == 0x94af){ // 100 10 100 10 10 1 1 1 1
					for(u8_i = 0; u8_i < _CAD_CHANGE_QUE_BUF_FRM; u8_i++){
						if(g_cadence_change_que.type[u8_i] == 0 && g_cadence_change_que.state[u8_i] == 0){
							g_cadence_change_que.type[u8_i] = 11;
							g_cadence_change_que.mc_wrt_index[u8_i] = mc_write;
							g_cadence_change_que.state[u8_i] = 1;
							if(log_en){
							rtd_pr_memc_notice("[%s][g_cadence_change_que][%d][u8_i,%d,][mc_wrt_index,%d,][state,%d,][type,%d,]\n\r", 
								__FUNCTION__, __LINE__, u8_i, g_cadence_change_que.mc_wrt_index[u8_i], g_cadence_change_que.state[u8_i], g_cadence_change_que.type[u8_i]);
							}
							break;
						}else{
							Que_buffer_full_flag++;
						}
					}
				}
				else if((pOutput->u32_Mot_sequence[_FILM_ALL]&0x7f) == 0x55){ // 10 10 10 1
					for(u8_i = 0; u8_i < _CAD_CHANGE_QUE_BUF_FRM; u8_i++){
						if(g_cadence_change_que.type[u8_i] == 0 && g_cadence_change_que.state[u8_i] == 0){
							g_cadence_change_que.type[u8_i] = 2;
							g_cadence_change_que.mc_wrt_index[u8_i] = mc_write;
							g_cadence_change_que.state[u8_i] = 1;
							if(log_en){
							rtd_pr_memc_notice("[%s][g_cadence_change_que][%d][u8_i,%d,][mc_wrt_index,%d,][state,%d,][type,%d,]\n\r", 
								__FUNCTION__, __LINE__, u8_i, g_cadence_change_que.mc_wrt_index[u8_i], g_cadence_change_que.state[u8_i], g_cadence_change_que.type[u8_i]);
							}
							break;
						}else{
							Que_buffer_full_flag++;
						}
					}
				}
				else if((pOutput->u32_Mot_sequence[_FILM_ALL]&0x1f) == 0x11){ // 1000 1
					for(u8_i = 0; u8_i < _CAD_CHANGE_QUE_BUF_FRM; u8_i++){
						if(g_cadence_change_que.type[u8_i] == 0 && g_cadence_change_que.state[u8_i] == 0){
							g_cadence_change_que.type[u8_i] = 4;
							g_cadence_change_que.mc_wrt_index[u8_i] = mc_write;
							g_cadence_change_que.state[u8_i] = 1;
							if(log_en){
							rtd_pr_memc_notice("[%s][g_cadence_change_que][%d][u8_i,%d,][mc_wrt_index,%d,][state,%d,][type,%d,]\n\r", 
								__FUNCTION__, __LINE__, u8_i, g_cadence_change_que.mc_wrt_index[u8_i], g_cadence_change_que.state[u8_i], g_cadence_change_que.type[u8_i]);
							}
							break;
						}else{
							Que_buffer_full_flag++;
						}
					}
				}
				else if((pOutput->u32_Mot_sequence[_FILM_ALL]&0x7f) == 0x45){ // 1000 10 1
					for(u8_i = 0; u8_i < _CAD_CHANGE_QUE_BUF_FRM; u8_i++){
						if(g_cadence_change_que.type[u8_i] == 0 && g_cadence_change_que.state[u8_i] == 0){
							g_cadence_change_que.type[u8_i] = 42;
							g_cadence_change_que.mc_wrt_index[u8_i] = mc_write;
							g_cadence_change_que.state[u8_i] = 1;
							if(log_en){
							rtd_pr_memc_notice("[%s][g_cadence_change_que][%d][u8_i,%d,][mc_wrt_index,%d,][state,%d,][type,%d,]\n\r", 
								__FUNCTION__, __LINE__, u8_i, g_cadence_change_que.mc_wrt_index[u8_i], g_cadence_change_que.state[u8_i], g_cadence_change_que.type[u8_i]);
							}
							break;
						}else{
							Que_buffer_full_flag++;
						}
					}
				}
				else if((pOutput->u32_Mot_sequence[_FILM_ALL]&0x1ff) == 0x115){ // 1000 10 10 1
					for(u8_i = 0; u8_i < _CAD_CHANGE_QUE_BUF_FRM; u8_i++){
						if(g_cadence_change_que.type[u8_i] == 0 && g_cadence_change_que.state[u8_i] == 0){
							g_cadence_change_que.type[u8_i] = 422;
							g_cadence_change_que.mc_wrt_index[u8_i] = mc_write;
							g_cadence_change_que.state[u8_i] = 1;
							if(log_en){
							rtd_pr_memc_notice("[%s][g_cadence_change_que][%d][u8_i,%d,][mc_wrt_index,%d,][state,%d,][type,%d,]\n\r", 
								__FUNCTION__, __LINE__, u8_i, g_cadence_change_que.mc_wrt_index[u8_i], g_cadence_change_que.state[u8_i], g_cadence_change_que.type[u8_i]);
							}
							break;
						}else{
							Que_buffer_full_flag++;
						}
					}
				}
			}
			else if(u8_cur_cadId == _CAD_2224){
				if((pOutput->u32_Mot_sequence[_FILM_ALL]&0x3f) == 0x3f){ // 1 1 1 1 1 1 
					if((pOutput->u32_Mot_sequence[_FILM_ALL]&0xfff) == 0xa3f // 10 1000 1 1 1 1 1 1
						|| (pOutput->u32_Mot_sequence[_FILM_ALL]&0x1fff) == 0x147f){ // 10 1000 1 1 1 1 1 1 1
						// don't do anything
					}
					else{
						for(u8_i = 0; u8_i < _CAD_CHANGE_QUE_BUF_FRM; u8_i++){
							if(g_cadence_change_que.type[u8_i] == 0 && g_cadence_change_que.state[u8_i] == 0){
								g_cadence_change_que.type[u8_i] = 1;
								g_cadence_change_que.mc_wrt_index[u8_i] = mc_write;
								g_cadence_change_que.state[u8_i] = 1;
							if(log_en){
								rtd_pr_memc_notice("[%s][g_cadence_change_que][%d][u8_i,%d,][mc_wrt_index,%d,][state,%d,][type,%d,]\n\r", 
									__FUNCTION__, __LINE__, u8_i, g_cadence_change_que.mc_wrt_index[u8_i], g_cadence_change_que.state[u8_i], g_cadence_change_que.type[u8_i]);
							}
								break;
							}else{
								Que_buffer_full_flag++;
							}
						}
					}
				}
				else if((pOutput->u32_Mot_sequence[_FILM_ALL]&0x7fff) == 0x455f // 1000 10 10 10 1 1 1 1 1
					|| (pOutput->u32_Mot_sequence[_FILM_ALL]&0x1ffff) == 0x1155f){ // 1000 10 10 10 10 1 1 1 1 1
					for(u8_i = 0; u8_i < _CAD_CHANGE_QUE_BUF_FRM; u8_i++){
						if(g_cadence_change_que.type[u8_i] == 0 && g_cadence_change_que.state[u8_i] == 0){
							g_cadence_change_que.type[u8_i] = 11;
							g_cadence_change_que.mc_wrt_index[u8_i] = mc_write;
							g_cadence_change_que.state[u8_i] = 1;
							if(log_en){
							rtd_pr_memc_notice("[%s][g_cadence_change_que][%d][u8_i,%d,][mc_wrt_index,%d,][state,%d,][type,%d,]\n\r", 
								__FUNCTION__, __LINE__, u8_i, g_cadence_change_que.mc_wrt_index[u8_i], g_cadence_change_que.state[u8_i], g_cadence_change_que.type[u8_i]);
							}
							break;
						}else{
							Que_buffer_full_flag++;
						}
					}
				}
				else if((pOutput->u32_Mot_sequence[_FILM_ALL]&0xf) == 0x9){ // 100 1
					if((pOutput->u32_Mot_sequence[_FILM_ALL]&0x3fff) == 0x22a9){ // 1000 10 10 10 100 1
						for(u8_i = 0; u8_i < _CAD_CHANGE_QUE_BUF_FRM; u8_i++){
							if(g_cadence_change_que.type[u8_i] == 0 && g_cadence_change_que.state[u8_i] == 0){
								g_cadence_change_que.type[u8_i] = 423;
								g_cadence_change_que.mc_wrt_index[u8_i] = mc_write;
								g_cadence_change_que.state[u8_i] = 1;
							if(log_en){
								rtd_pr_memc_notice("[%s][g_cadence_change_que][%d][u8_i,%d,][mc_wrt_index,%d,][state,%d,][type,%d,]\n\r", 
									__FUNCTION__, __LINE__, u8_i, g_cadence_change_que.mc_wrt_index[u8_i], g_cadence_change_que.state[u8_i], g_cadence_change_que.type[u8_i]);
							}
								break;
							}else{
								Que_buffer_full_flag++;
							}
						}
					}
					else{
						for(u8_i = 0; u8_i < _CAD_CHANGE_QUE_BUF_FRM; u8_i++){
							if(g_cadence_change_que.type[u8_i] == 0 && g_cadence_change_que.state[u8_i] == 0){
								g_cadence_change_que.type[u8_i] = 3;
								g_cadence_change_que.mc_wrt_index[u8_i] = mc_write;
								g_cadence_change_que.state[u8_i] = 1;
							if(log_en){
								rtd_pr_memc_notice("[%s][g_cadence_change_que][%d][u8_i,%d,][mc_wrt_index,%d,][state,%d,][type,%d,]\n\r", 
									__FUNCTION__, __LINE__, u8_i, g_cadence_change_que.mc_wrt_index[u8_i], g_cadence_change_que.state[u8_i], g_cadence_change_que.type[u8_i]);
							}
								break;
							}else{
								Que_buffer_full_flag++;
							}
						}
					}
				}
				else if((pOutput->u32_Mot_sequence[_FILM_ALL]&0x3f) == 0x25){ // 100 10 1
					if((pOutput->u32_Mot_sequence[_FILM_ALL]&0xffff) == 0x8aa5){ // 1000 10 10 10 100 10 1
						for(u8_i = 0; u8_i < _CAD_CHANGE_QUE_BUF_FRM; u8_i++){
							if(g_cadence_change_que.type[u8_i] == 0 && g_cadence_change_que.state[u8_i] == 0){
								g_cadence_change_que.type[u8_i] = 4232;
								g_cadence_change_que.mc_wrt_index[u8_i] = mc_write;
								g_cadence_change_que.state[u8_i] = 1;
							if(log_en){
								rtd_pr_memc_notice("[%s][g_cadence_change_que][%d][u8_i,%d,][mc_wrt_index,%d,][state,%d,][type,%d,]\n\r", 
									__FUNCTION__, __LINE__, u8_i, g_cadence_change_que.mc_wrt_index[u8_i], g_cadence_change_que.state[u8_i], g_cadence_change_que.type[u8_i]);
							}
								break;
							}else{
								Que_buffer_full_flag++;
							}
						}
					}
					else{
						for(u8_i = 0; u8_i < _CAD_CHANGE_QUE_BUF_FRM; u8_i++){
							if(g_cadence_change_que.type[u8_i] == 0 && g_cadence_change_que.state[u8_i] == 0){
								g_cadence_change_que.type[u8_i] = 32;
								g_cadence_change_que.mc_wrt_index[u8_i] = mc_write;
								g_cadence_change_que.state[u8_i] = 1;
								if(log_en){
								rtd_pr_memc_notice("[%s][g_cadence_change_que][%d][u8_i,%d,][mc_wrt_index,%d,][state,%d,][type,%d,]\n\r", 
									__FUNCTION__, __LINE__, u8_i, g_cadence_change_que.mc_wrt_index[u8_i], g_cadence_change_que.state[u8_i], g_cadence_change_que.type[u8_i]);
								}
								break;
							}else{
								Que_buffer_full_flag++;
							}
						}
					}
				}
				else if((pOutput->u32_Mot_sequence[_FILM_ALL]&0xff) == 0x95){ // 100 10 10 1
					for(u8_i = 0; u8_i < _CAD_CHANGE_QUE_BUF_FRM; u8_i++){
						if(g_cadence_change_que.type[u8_i] == 0 && g_cadence_change_que.state[u8_i] == 0){
							g_cadence_change_que.type[u8_i] = 322;
							g_cadence_change_que.mc_wrt_index[u8_i] = mc_write;
							g_cadence_change_que.state[u8_i] = 1;
							if(log_en){
							rtd_pr_memc_notice("[%s][g_cadence_change_que][%d][u8_i,%d,][mc_wrt_index,%d,][state,%d,][type,%d,]\n\r", 
								__FUNCTION__, __LINE__, u8_i, g_cadence_change_que.mc_wrt_index[u8_i], g_cadence_change_que.state[u8_i], g_cadence_change_que.type[u8_i]);
							}
							break;
						}else{
							Que_buffer_full_flag++;
						}
					}
				}
				else if((pOutput->u32_Mot_sequence[_FILM_ALL]&0x1fff) == 0x1155){ // 1000 10 10 10 10 1
					for(u8_i = 0; u8_i < _CAD_CHANGE_QUE_BUF_FRM; u8_i++){
						if(g_cadence_change_que.type[u8_i] == 0 && g_cadence_change_que.state[u8_i] == 0){
							g_cadence_change_que.type[u8_i] = 422;
							g_cadence_change_que.mc_wrt_index[u8_i] = mc_write;
							g_cadence_change_que.state[u8_i] = 1;
							if(log_en){
							rtd_pr_memc_notice("[%s][g_cadence_change_que][%d][u8_i,%d,][mc_wrt_index,%d,][state,%d,][type,%d,]\n\r", 
								__FUNCTION__, __LINE__, u8_i, g_cadence_change_que.mc_wrt_index[u8_i], g_cadence_change_que.state[u8_i], g_cadence_change_que.type[u8_i]);
							}
							break;
						}else{
							Que_buffer_full_flag++;
						}
					}
				}
				else if((pOutput->u32_Mot_sequence[_FILM_ALL]&0x1fffff) == 0x115451 // 1000 10 10 10 1000 10 1000 1
					|| (pOutput->u32_Mot_sequence[_FILM_ALL]&0x7ffffff) == 0x4551451){ // 1000 10 10 10 1000 10 1000 10 1000 1
					for(u8_i = 0; u8_i < _CAD_CHANGE_QUE_BUF_FRM; u8_i++){
						if(g_cadence_change_que.type[u8_i] == 0 && g_cadence_change_que.state[u8_i] == 0){
							g_cadence_change_que.type[u8_i] = 4242;
							g_cadence_change_que.mc_wrt_index[u8_i] = mc_write;
							g_cadence_change_que.state[u8_i] = 1;
							if(log_en){
							rtd_pr_memc_notice("[%s][g_cadence_change_que][%d][u8_i,%d,][mc_wrt_index,%d,][state,%d,][type,%d,]\n\r", 
								__FUNCTION__, __LINE__, u8_i, g_cadence_change_que.mc_wrt_index[u8_i], g_cadence_change_que.state[u8_i], g_cadence_change_que.type[u8_i]);
							}
							break;
						}else{
							Que_buffer_full_flag++;
						}
					}
				}
				else if((pOutput->u32_Mot_sequence[_FILM_ALL]&0xfff) == 0x8d5){ // 1000 1 10 10 10 1
					for(u8_i = 0; u8_i < _CAD_CHANGE_QUE_BUF_FRM; u8_i++){
						if(g_cadence_change_que.type[u8_i] == 0 && g_cadence_change_que.state[u8_i] == 0){
							g_cadence_change_que.type[u8_i] = 41222;
							g_cadence_change_que.mc_wrt_index[u8_i] = mc_write;
							g_cadence_change_que.state[u8_i] = 1;
							if(log_en){
							rtd_pr_memc_notice("[%s][g_cadence_change_que][%d][u8_i,%d,][mc_wrt_index,%d,][state,%d,][type,%d,]\n\r", 
								__FUNCTION__, __LINE__, u8_i, g_cadence_change_que.mc_wrt_index[u8_i], g_cadence_change_que.state[u8_i], g_cadence_change_que.type[u8_i]);
							}
							break;
						}else{
							Que_buffer_full_flag++;
						}
					}
				}
			}
			else if(u8_cur_cadId == _CAD_22224){
				if((pOutput->u32_Mot_sequence[_FILM_ALL]&0xffff) == 0x8aaf){ // 1000 10 10 10 10 1 1 1 1 
					for(u8_i = 0; u8_i < _CAD_CHANGE_QUE_BUF_FRM; u8_i++){
						if(g_cadence_change_que.type[u8_i] == 0 && g_cadence_change_que.state[u8_i] == 0){
							g_cadence_change_que.type[u8_i] = 11;
							g_cadence_change_que.mc_wrt_index[u8_i] = mc_write;
							g_cadence_change_que.state[u8_i] = 1;
							if(log_en){
							rtd_pr_memc_notice("[%s][g_cadence_change_que][%d][u8_i,%d,][mc_wrt_index,%d,][state,%d,][type,%d,]\n\r", 
								__FUNCTION__, __LINE__, u8_i, g_cadence_change_que.mc_wrt_index[u8_i], g_cadence_change_que.state[u8_i], g_cadence_change_que.type[u8_i]);
							}
							break;
						}else{
							Que_buffer_full_flag++;
						}
					}
				}
				else if((pOutput->u32_Mot_sequence[_FILM_ALL]&0xf) == 0x9){ // 100 1
					if((pOutput->u32_Mot_sequence[_FILM_ALL]&0xffff) == 0x8aa9){ // 1000 10 10 10 10 100 1
						for(u8_i = 0; u8_i < _CAD_CHANGE_QUE_BUF_FRM; u8_i++){
							if(g_cadence_change_que.type[u8_i] == 0 && g_cadence_change_que.state[u8_i] == 0){
								g_cadence_change_que.type[u8_i] = 423;
								g_cadence_change_que.mc_wrt_index[u8_i] = mc_write;
								g_cadence_change_que.state[u8_i] = 1;
							if(log_en){
								rtd_pr_memc_notice("[%s][g_cadence_change_que][%d][u8_i,%d,][mc_wrt_index,%d,][state,%d,][type,%d,]\n\r", 
									__FUNCTION__, __LINE__, u8_i, g_cadence_change_que.mc_wrt_index[u8_i], g_cadence_change_que.state[u8_i], g_cadence_change_que.type[u8_i]);
							}
								break;
							}else{
								Que_buffer_full_flag++;
							}
						}
					}
					else{
						for(u8_i = 0; u8_i < _CAD_CHANGE_QUE_BUF_FRM; u8_i++){
							if(g_cadence_change_que.type[u8_i] == 0 && g_cadence_change_que.state[u8_i] == 0){
								g_cadence_change_que.type[u8_i] = 3;
								g_cadence_change_que.mc_wrt_index[u8_i] = mc_write;
								g_cadence_change_que.state[u8_i] = 1;
							if(log_en){
								rtd_pr_memc_notice("[%s][g_cadence_change_que][%d][u8_i,%d,][mc_wrt_index,%d,][state,%d,][type,%d,]\n\r", 
									__FUNCTION__, __LINE__, u8_i, g_cadence_change_que.mc_wrt_index[u8_i], g_cadence_change_que.state[u8_i], g_cadence_change_que.type[u8_i]);
							}
								break;
							}else{
								Que_buffer_full_flag++;
							}
						}
					}
				}
				else if((pOutput->u32_Mot_sequence[_FILM_ALL]&0x3f) == 0x25){ // 100 10 1
					if((pOutput->u32_Mot_sequence[_FILM_ALL]&0x3ffff) == 0x22aa5){ // 1000 10 10 10 10 100 10 1
						for(u8_i = 0; u8_i < _CAD_CHANGE_QUE_BUF_FRM; u8_i++){
							if(g_cadence_change_que.type[u8_i] == 0 && g_cadence_change_que.state[u8_i] == 0){
								g_cadence_change_que.type[u8_i] = 4232;
								g_cadence_change_que.mc_wrt_index[u8_i] = mc_write;
								g_cadence_change_que.state[u8_i] = 1;
							if(log_en){
								rtd_pr_memc_notice("[%s][g_cadence_change_que][%d][u8_i,%d,][mc_wrt_index,%d,][state,%d,][type,%d,]\n\r", 
									__FUNCTION__, __LINE__, u8_i, g_cadence_change_que.mc_wrt_index[u8_i], g_cadence_change_que.state[u8_i], g_cadence_change_que.type[u8_i]);
							}
								break;
							}else{
								Que_buffer_full_flag++;
							}
						}
					}
					else{
					for(u8_i = 0; u8_i < _CAD_CHANGE_QUE_BUF_FRM; u8_i++){
						if(g_cadence_change_que.type[u8_i] == 0 && g_cadence_change_que.state[u8_i] == 0){
							g_cadence_change_que.type[u8_i] = 32;
							g_cadence_change_que.mc_wrt_index[u8_i] = mc_write;
							g_cadence_change_que.state[u8_i] = 1;
							if(log_en){
							rtd_pr_memc_notice("[%s][g_cadence_change_que][%d][u8_i,%d,][mc_wrt_index,%d,][state,%d,][type,%d,]\n\r", 
								__FUNCTION__, __LINE__, u8_i, g_cadence_change_que.mc_wrt_index[u8_i], g_cadence_change_que.state[u8_i], g_cadence_change_que.type[u8_i]);
							}
							break;
						}else{
							Que_buffer_full_flag++;
						}

						}
					}
				}
				else if((pOutput->u32_Mot_sequence[_FILM_ALL]&0xff) == 0x95){ // 100 10 10 1
					for(u8_i = 0; u8_i < _CAD_CHANGE_QUE_BUF_FRM; u8_i++){
						if(g_cadence_change_que.type[u8_i] == 0 && g_cadence_change_que.state[u8_i] == 0){
							g_cadence_change_que.type[u8_i] = 322;
							g_cadence_change_que.mc_wrt_index[u8_i] = mc_write;
							g_cadence_change_que.state[u8_i] = 1;
							if(log_en){
							rtd_pr_memc_notice("[%s][g_cadence_change_que][%d][u8_i,%d,][mc_wrt_index,%d,][state,%d,][type,%d,]\n\r", 
								__FUNCTION__, __LINE__, u8_i, g_cadence_change_que.mc_wrt_index[u8_i], g_cadence_change_que.state[u8_i], g_cadence_change_que.type[u8_i]);
							}
							break;
						}else{
							Que_buffer_full_flag++;
						}
					}
				}
			}

			if((small_motion_flag_chg == 1 && small_motion_flag == 1) 
				|| (std_video_flag_chg == 1 && std_video_flag == 1 && u8_cur_cadId == _CAD_VIDEO)
				|| (Que_buffer_full_flag == 8)){
				if(log_en){
				rtd_pr_memc_notice("[%s][Clean que buffer !!!][%d,%d,%d]\n\r", __FUNCTION__, 
					(small_motion_flag_chg == 1 && small_motion_flag == 1),
					(std_video_flag_chg == 1 && std_video_flag == 1), 
					(Que_buffer_full_flag == 8));
				}
				//clean que buffer one times
				for(u8_i = 0; u8_i < _CAD_CHANGE_QUE_BUF_FRM; u8_i++){
					g_cadence_change_que.state[u8_i] = 0;
					g_cadence_change_que.type[u8_i] = 0;
					g_cadence_change_que.mc_wrt_index[u8_i] = 0;
				}
			}

//			rtd_pr_memc_notice("[8B_test][motion,%d,][m_th,%d,][seq,%x,]\n\r",
//				s_pContext->_output_read_comreg.u27_ipme_aMot_rb, pOutput->u27_g_FrmMot_Th[_FILM_ALL], pOutput->u32_Mot_sequence[_FILM_ALL]);
		}
	}
	else if(pOutput->u1_mixMode_chg){
		if(pOutput->u1_mixMode == 1){
			if(u8_pre_cadId == _CAD_22){
				g_cadence_change_flag = _CASE_22_TO_MIX_NEW; // _CASE_NULL;
			}
			else if(u8_pre_cadId == _CAD_32){
				g_cadence_change_flag = _CASE_32_TO_MIX_NEW; // _CASE_NULL;
			}
			else if(u8_pre_cadId == _CAD_2224){
				g_cadence_change_flag = _CASE_2224_TO_MIX_NEW; // _CASE_NULL;
			}
			else if(u8_pre_cadId == _CAD_11112){
				g_cadence_change_flag = _CASE_11112_TO_MIX_NEW; // _CASE_NULL;
			}
		}
	}

	
	}
	else{
		//clean que buffer
		for(u8_i = 0; u8_i < _CAD_CHANGE_QUE_BUF_FRM; u8_i++){
			g_cadence_change_que.state[u8_i] = 0;
			g_cadence_change_que.type[u8_i] = 0;
			g_cadence_change_que.mc_wrt_index[u8_i] = 0;
		}
		g_cadence_change_flag = _CASE_NULL;
	}

	small_motion_flag_pre = small_motion_flag;
	std_video_flag_pre = std_video_flag_chg;
//}else{
#else
	if(CadenceChangeDetect_en == 1 && in_fmRate >= _PQL_IN_50HZ){
		if((pOutput->u1_mixMode == 0) && (u8_cur_cadId == _CAD_32) && (u8_pre_cadId == _CAD_22)){
			g_cadence_change_flag = _CASE_22_TO_32;
		}
		else if((pOutput->u1_mixMode == 0) && (u8_cur_cadId == _CAD_22) && (u8_ph_idx == 0) && (u8_pre_cadId == _CAD_32) && (u8_ph_idx_pre == 2)){
			g_cadence_change_flag = _CASE_32_TO_22;
		}
		else if((pOutput->u1_mixMode == 0) && (u8_cur_cadId == _CAD_22) && (u8_pre_cadId == _CAD_VIDEO) && (u8_ph_idx == 1)){
			g_cadence_change_flag = _CASE_11_TO_22_TYPE1;
		}
		else if((pOutput->u1_mixMode == 0) && (u8_cur_cadId == _CAD_22) && (u8_pre_cadId == _CAD_VIDEO) && (u8_ph_idx == 0)){
			g_cadence_change_flag = _CASE_11_TO_22_TYPE2;
		}
		else if((pOutput->u1_mixMode == 0) && (u8_cur_cadId == _CAD_22) && (u8_pre_cadId == _CAD_VIDEO) && (u8_ph_idx == 0)){
				g_cadence_change_flag = _CASE_11_TO_22_TYPE2;
		}
		else if((pOutput->u1_mixMode == 0) && (u8_cur_cadId == _CAD_VIDEO) && (u8_pre_cadId == _CAD_22)){
			g_cadence_change_flag = _CASE_22_TO_11;
		}
		else if((pOutput->u1_mixMode == 0) && (u8_cur_cadId == _CAD_32) && (u8_pre_cadId == _CAD_VIDEO) && (u8_ph_idx == 1)){
			g_cadence_change_flag = _CASE_11_TO_32_TYPE1;
		}
		else if((pOutput->u1_mixMode == 0) && (u8_cur_cadId == _CAD_32) && (u8_pre_cadId == _CAD_VIDEO) && (u8_ph_idx == 0)){
			g_cadence_change_flag = _CASE_11_TO_32_TYPE5;
		}
		else if((pOutput->u1_mixMode == 0) && (u8_cur_cadId == _CAD_VIDEO) && (u8_pre_cadId == _CAD_32) && (u8_ph_idx_pre == 1)){
			g_cadence_change_flag = _CASE_32_TO_11_TYPE1;
		}
		else if((pOutput->u1_mixMode == 0) && (u8_cur_cadId == _CAD_VIDEO) && (u8_pre_cadId == _CAD_32) && (u8_ph_idx_pre == 2)){
			g_cadence_change_flag = _CASE_32_TO_11_TYPE2;
		}
		else if((pOutput->u1_mixMode == 0) && (u8_cur_cadId == _CAD_VIDEO) && (u8_pre_cadId == _CAD_32) && (u8_ph_idx_pre == 4)){
			g_cadence_change_flag = _CASE_32_TO_11_TYPE4;
		}
		else if((pOutput->u1_mixMode == 0) && (u8_cur_cadId == _CAD_2224) && (u8_pre_cadId == _CAD_VIDEO) && (u8_ph_idx == 1)){
			g_cadence_change_flag = _CASE_11_TO_2224_TYPE1;
		}
		else if((pOutput->u1_mixMode == 0) && (u8_cur_cadId == _CAD_2224) && (u8_pre_cadId == _CAD_VIDEO) && (u8_ph_idx == 2)){
			g_cadence_change_flag = _CASE_11_TO_2224_TYPE2; // _CASE_NULL; // 
		}
		else if((pOutput->u1_mixMode == 0) && (u8_cur_cadId == _CAD_2224) && (u8_pre_cadId == _CAD_VIDEO) && (u8_ph_idx == 3)){
			g_cadence_change_flag = _CASE_11_TO_2224_TYPE3;
		}
		else if((pOutput->u1_mixMode == 0) && (u8_cur_cadId == _CAD_2224) && (u8_pre_cadId == _CAD_VIDEO) && (u8_ph_idx == 4)){
			g_cadence_change_flag = _CASE_11_TO_2224_TYPE4;
		}
		else if((pOutput->u1_mixMode == 0) && (u8_cur_cadId == _CAD_2224) && (u8_pre_cadId == _CAD_VIDEO) && (u8_ph_idx == 5)){
			g_cadence_change_flag = _CASE_11_TO_2224_TYPE5; // _CASE_NULL; // 
		}
		else if((pOutput->u1_mixMode == 0) && (u8_cur_cadId == _CAD_VIDEO) && (u8_pre_cadId == _CAD_2224) && (u8_ph_idx_pre == 1)){
			g_cadence_change_flag = _CASE_2224_TO_11_TYPE1; // _CASE_NULL; // 
		}
		else if((pOutput->u1_mixMode == 0) && (u8_cur_cadId == _CAD_VIDEO) && (u8_pre_cadId == _CAD_2224) && (u8_ph_idx_pre == 3)){
			g_cadence_change_flag = _CASE_2224_TO_11_TYPE3;
		}
		else if((pOutput->u1_mixMode == 0) && (u8_cur_cadId == _CAD_VIDEO) && (u8_pre_cadId == _CAD_2224) && (u8_ph_idx_pre == 5)){
			g_cadence_change_flag = _CASE_2224_TO_11_TYPE5;
		}
		else if((pOutput->u1_mixMode == 0) && (u8_cur_cadId == _CAD_VIDEO) && (u8_pre_cadId == _CAD_2224) && (u8_ph_idx_pre == 7)){
			g_cadence_change_flag = _CASE_2224_TO_11_TYPE7;
		}
		else if((pOutput->u1_mixMode == 0) && (u8_cur_cadId == _CAD_VIDEO) && (u8_pre_cadId == _CAD_2224) && (u8_ph_idx_pre == 8)){
			g_cadence_change_flag = _CASE_2224_TO_11_TYPE8;
		}
		else if((pOutput->u1_mixMode == 0) && (u8_cur_cadId == _CAD_VIDEO) && (u8_pre_cadId == _CAD_2224) && (u8_ph_idx_pre == 9)){
			g_cadence_change_flag = _CASE_2224_TO_11_TYPE9;
		}
		else if((pOutput->u1_mixMode == 0) && (u8_cur_cadId == _CAD_2224) && (u8_ph_idx == 6) && (u8_pre_cadId == _CAD_22) && (u8_ph_idx_pre == 1)){
			g_cadence_change_flag = _CASE_22_TYPE1_TO_2224_TYPE6;
		}
		else if((pOutput->u1_mixMode == 0) && (u8_cur_cadId == _CAD_22) && (u8_ph_idx == 0) && (u8_pre_cadId == _CAD_2224) && (u8_ph_idx_pre == 1)){
			g_cadence_change_flag = _CASE_2224_TYPE1_TO_22_TYPE0;
		}
		else if((pOutput->u1_mixMode == 0) && (u8_cur_cadId == _CAD_2224) && (u8_ph_idx == 1) && (u8_pre_cadId == _CAD_32) && (u8_ph_idx_pre == 4)){
			g_cadence_change_flag = _CASE_32_TYPE4_TO_2224_TYPE1;
		}
		else if((pOutput->u1_mixMode == 0) && (u8_cur_cadId == _CAD_32) && (u8_ph_idx == 3) && (u8_pre_cadId == _CAD_2224) && (u8_ph_idx_pre == 8)){
			g_cadence_change_flag = _CASE_2224_TYPE8_TO_32_TYPE3;
		}
		else if(pOutput->u1_mixMode_chg){
			if(pOutput->u1_mixMode == 1){
				if(u8_pre_cadId == _CAD_22 && (u8_ph_idx_pre == 1)){
					g_cadence_change_flag = _CASE_22_TO_MIX; // _CASE_NULL;
				}
				else if(u8_pre_cadId == _CAD_32 && (u8_ph_idx_pre == 1)){
					g_cadence_change_flag = _CASE_32_TO_MIX; // _CASE_NULL;
				}
				else if(u8_pre_cadId == _CAD_32 && (u8_ph_idx_pre == 2)){
					g_cadence_change_flag = _CASE_32_TO_11_TYPE2; // _CASE_NULL;
				}
				else if(u8_pre_cadId == _CAD_32 && (u8_ph_idx_pre == 4)){
					g_cadence_change_flag = _CASE_32_TO_11_TYPE4; // _CASE_NULL;
				}
				else if(u8_pre_cadId == _CAD_2224 && (u8_ph_idx_pre == 1)){
					g_cadence_change_flag = _CASE_2224_TO_11_TYPE1; // _CASE_NULL; // 
				}
				else if(u8_pre_cadId == _CAD_2224 && (u8_ph_idx_pre == 3)){
					g_cadence_change_flag = _CASE_2224_TO_11_TYPE3; // _CASE_NULL; // 
				}
				else if(u8_pre_cadId == _CAD_2224 && (u8_ph_idx_pre == 5)){
					g_cadence_change_flag = _CASE_2224_TO_11_TYPE5; // _CASE_NULL; // 
				}
				else if(u8_pre_cadId == _CAD_2224 && (u8_ph_idx_pre == 7)){
					g_cadence_change_flag = _CASE_2224_TO_11_TYPE7; // _CASE_NULL; // 
				}
				else if(u8_pre_cadId == _CAD_2224 && (u8_ph_idx_pre == 8)){
					g_cadence_change_flag = _CASE_2224_TO_11_TYPE8; // _CASE_NULL; // 
				}
				else if(u8_pre_cadId == _CAD_2224 && (u8_ph_idx_pre == 9)){
					g_cadence_change_flag = _CASE_2224_TO_11_TYPE9; // _CASE_NULL; // 
				}
			}
			else{ //if(pOutput->u1_mixMode == 0)
				if(u8_det_cadId == _CAD_22 && (u8_ph_idx == 1)){
					g_cadence_change_flag = _CASE_11_TO_22_TYPE1; // _CASE_NULL;
				}
				else if(u8_det_cadId == _CAD_22 && (u8_ph_idx == 0)){
					g_cadence_change_flag = _CASE_11_TO_22_TYPE2;
				}
				else if(u8_det_cadId == _CAD_32 && (u8_ph_idx == 1)){
					g_cadence_change_flag = _CASE_11_TO_32_TYPE1; // _CASE_NULL;
				}
				else if(u8_det_cadId == _CAD_32 && (u8_ph_idx == 0)){
					g_cadence_change_flag = _CASE_11_TO_32_TYPE5;
				}
				else if(u8_det_cadId == _CAD_2224 && (u8_ph_idx == 1)){
					g_cadence_change_flag = _CASE_11_TO_2224_TYPE1; // _CASE_NULL; // 
				}
				else if(u8_det_cadId == _CAD_2224 && (u8_ph_idx == 2)){
					g_cadence_change_flag = _CASE_11_TO_2224_TYPE2; // _CASE_NULL; // 
				}
				else if(u8_det_cadId == _CAD_2224 && (u8_ph_idx == 3)){
					g_cadence_change_flag = _CASE_11_TO_2224_TYPE3; // _CASE_NULL; // 
				}
				else if(u8_det_cadId == _CAD_2224 && (u8_ph_idx == 4)){
					g_cadence_change_flag = _CASE_11_TO_2224_TYPE4; // _CASE_NULL; // 
				}
				else if(u8_det_cadId == _CAD_2224 && (u8_ph_idx == 5)){
					g_cadence_change_flag = _CASE_11_TO_2224_TYPE5; // _CASE_NULL; // 
				}
			}
		}
		else{
			g_cadence_change_flag = _CASE_NULL;
		}
	}
//}
#endif
}

VOID MixModeQuit(const _PARAM_FilmDetectCtrl *pParam, _OUTPUT_FilmDetectCtrl *pOutput)
{
       unsigned char u1_quit_cond1 = 0;
       unsigned char u1_quit_cond1_dly = 0;
       unsigned int u27_smot_th_l = _MIN_(pParam->u27_mixMode_rgnMove_minMot, 0x200);

       unsigned int u27_smot_th_h = 0x3072;//(u32_RB_val&0x7fffffff);

	unsigned char u5_quit_cond2 = 0;
	unsigned char i = 0;
	// rgn0=rgn1=rgn2=rgn3=rgn4
       u1_quit_cond1 = ((pOutput->u8_pre_cadence_Id[_FILM_5RGN_LFT]==pOutput->u8_pre_cadence_Id[_FILM_5RGN_CEN])
                               && (pOutput->u8_pre_cadence_Id[_FILM_5RGN_RHT]==pOutput->u8_pre_cadence_Id[_FILM_5RGN_CEN])
                               && (pOutput->u8_pre_cadence_Id[_FILM_5RGN_TOP]==pOutput->u8_pre_cadence_Id[_FILM_5RGN_CEN])
                               && (pOutput->u8_pre_cadence_Id[_FILM_5RGN_BOT]==pOutput->u8_pre_cadence_Id[_FILM_5RGN_CEN])
                               && (pOutput->u8_pre_cadence_Id[_FILM_5RGN_CEN]!=_CAD_VIDEO)) ? 1 : u1_quit_cond1;

       //avoid: 1. center area smot < th, bmot > N*th
       //           2. t/b/l/r area smot > M*th
       u1_quit_cond1_dly = ((pOutput->u27_quit_smlMot[_FILM_5RGN_CEN] < u27_smot_th_l) && (pOutput->u27_quit_bigMot[_FILM_5RGN_CEN] > 8*u27_smot_th_l))
                                       && ((_MIN_(pOutput->u27_quit_smlMot[_FILM_5RGN_LFT], pOutput->u27_quit_bigMot[_FILM_5RGN_LFT]) > 4*u27_smot_th_l)
                                               ||(_MIN_(pOutput->u27_quit_smlMot[_FILM_5RGN_RHT], pOutput->u27_quit_bigMot[_FILM_5RGN_RHT]) > 4*u27_smot_th_l)
                                               ||(_MIN_(pOutput->u27_quit_smlMot[_FILM_5RGN_TOP], pOutput->u27_quit_bigMot[_FILM_5RGN_TOP]) > 4*u27_smot_th_l)

                                               ||(_MIN_(pOutput->u27_quit_smlMot[_FILM_5RGN_BOT], pOutput->u27_quit_bigMot[_FILM_5RGN_BOT]) > 4*u27_smot_th_l));

       if(u1_quit_cond1 && (!u1_quit_cond1_dly)){
               pOutput->u8_mixMode_cnt = (pOutput->u8_mixMode_cnt>2)? (pOutput->u8_mixMode_cnt-2) : 0;
       }

       // 5 region motion big => depend on full region detect
       for(i = _FILM_5RGN_TOP; i <= _FILM_5RGN_CEN; i++){
               if(pOutput->u27_FrmMotion_S[i]> u27_smot_th_h) // th = 0x3072
                       u5_quit_cond2 = u5_quit_cond2 + (1<<i);
       }

       if(u5_quit_cond2 == 0x1f /*&& pParam->u1_MixModeQuitBySmv_En*/){
               pOutput->u8_mixMode_cnt = (pOutput->u8_mixMode_cnt>1)? (pOutput->u8_mixMode_cnt-1) : 0;
       }

}
unsigned char CadenceComparison(_OUTPUT_FilmDetectCtrl *pOutput,int sta,int end)
{
	int nIndx,nIndy;
	unsigned char nNotMatch =FALSE;
	for(nIndx =sta; nIndx<end -1 ;nIndx ++ )
		for(nIndy =nIndx +1; nIndy<end  ;nIndy ++ )
		{
			if(pOutput->u8_pre_cadence_Id[nIndx] !=  _CAD_VIDEO && pOutput->u8_pre_cadence_Id[nIndy] !=  _CAD_VIDEO && pOutput->u8_pre_cadence_Id[nIndx] !=   pOutput->u8_pre_cadence_Id[nIndy]  )
			{
				nNotMatch =TRUE;
				break;
			}
		}

	if(nNotMatch ==FALSE && pOutput->u8_pre_cadence_Id[sta] ==  _CAD_VIDEO)  // sta ~end all VIDEO
		nNotMatch =TRUE;

		
	return nNotMatch;
		
}
extern unsigned char fwif_color_get_cinema_mode_en(void);
VOID MixMode_detect(const _PARAM_FilmDetectCtrl *pParam, _OUTPUT_FilmDetectCtrl *pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	unsigned char u1_mixMode_status = 0, u1_badMot_status  = 0, u1_badMot_flag = 0, u1_badCad_flag = 0,check_flag = 0;
	unsigned char u8_mixMode_cnt_p1 = 0, u8_mixMode_cnt_n1 = 0;
	unsigned char u8_k = 0,u8_preCad_Id = 0,u1_preCad_motFlag = 0,u1_ipmeMot_flag = 0;
	unsigned int u17_dbg_1 = 0, u17_dbg_2 = 0, u17_dbg_3 = 0;
	#if MIX_MODE_REGION_17
	unsigned char u1_v1Cad_cond, u1_v2Cad_cond, u1_h1Cad_cond, u1_h2Cad_cond, u1_h3Cad_cond;
	#endif
	unsigned char u1_badCad_cond1, u1_badCad_cond2;
	static unsigned char u8_maxPossible_CadId = 0,u1_mixMode_pre=0, mix_cnt_hold_flag = 0, u1_VScrollingtext_flag = 0;
	unsigned char u4_rgnMove_motWgt = pParam->u4_mixMode_rgnMove_motWgt;
	unsigned int u27_smlMot=0,u28_smlMot_xGain=0,u28_smlMot_badTh=0,u28_badMot_Th=0,u8_mixMode_cnt_max=0;
	unsigned int badCad_cond1_weighting[_FILM_MOT_NUM] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,};
	unsigned char badCad_cond1_record[_FILM_MOT_NUM] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,};
	unsigned char mixmode_cnt_stride = 0;
	unsigned int rmv_temp = 0, logo_cnt_temp = 0, logo_cnt_top = 0;
	unsigned int rmv_top = 0, rmv_bottom = 0;//, rmv_left = 0, rmv_right = 0;
	unsigned int debug_flag1 = 0,debug_flag2 = 0;
	unsigned char det_cad = 0, length = 0;
	unsigned int u32_match_seq_sft = 0;
	unsigned int test_en2 = 0;
#if 1//weighting of small region of entering mixmode count
	ReadRegister(SOFTWARE1_SOFTWARE1_41_reg, 0, 1, &badCad_cond1_weighting[0]);//all
	ReadRegister(SOFTWARE1_SOFTWARE1_41_reg, 2, 3, &badCad_cond1_weighting[1]);//top
	ReadRegister(SOFTWARE1_SOFTWARE1_41_reg, 4, 5, &badCad_cond1_weighting[2]);//bottom
	ReadRegister(SOFTWARE1_SOFTWARE1_41_reg, 6, 7, &badCad_cond1_weighting[3]);//left
	ReadRegister(SOFTWARE1_SOFTWARE1_41_reg, 8, 9,  &badCad_cond1_weighting[4]);//right
	ReadRegister(SOFTWARE1_SOFTWARE1_41_reg, 10, 11 , &badCad_cond1_weighting[5]);//center
	ReadRegister(SOFTWARE1_SOFTWARE1_41_reg, 12, 13, &badCad_cond1_weighting[6]);//region1_of_12regions
	ReadRegister(SOFTWARE1_SOFTWARE1_41_reg, 14, 15, &badCad_cond1_weighting[7]);//region2_of_12regions
	ReadRegister(SOFTWARE1_SOFTWARE1_41_reg, 16, 17 , &badCad_cond1_weighting[8]);//region3_of_12regions
	ReadRegister(SOFTWARE1_SOFTWARE1_41_reg, 18, 19 , &badCad_cond1_weighting[9]);//region4_of_12regions
	ReadRegister(SOFTWARE1_SOFTWARE1_41_reg, 20, 21, &badCad_cond1_weighting[10]);//region5_of_12regions
	ReadRegister(SOFTWARE1_SOFTWARE1_41_reg, 22, 23, &badCad_cond1_weighting[11]);//region6_of_12regions
	ReadRegister(SOFTWARE1_SOFTWARE1_41_reg, 24, 25,  &badCad_cond1_weighting[12]);//region7_of_12regions
	ReadRegister(SOFTWARE1_SOFTWARE1_41_reg, 26, 27, &badCad_cond1_weighting[13]);//region8_of_12regions
	ReadRegister(SOFTWARE1_SOFTWARE1_41_reg, 28, 29, &badCad_cond1_weighting[14]);//region9_of_12regions
	ReadRegister(SOFTWARE1_SOFTWARE1_41_reg, 30, 31, &badCad_cond1_weighting[15]);//region10_of_12regions
	ReadRegister(SOFTWARE1_SOFTWARE1_42_reg, 0,  1, &badCad_cond1_weighting[16]);//region11_of_12regions
	ReadRegister(SOFTWARE1_SOFTWARE1_42_reg, 2,  3, &badCad_cond1_weighting[17]);//region12_of_12regions
#endif
	ReadRegister(SOFTWARE1_SOFTWARE1_42_reg, 4,  4, &debug_flag1);//debug_1
	ReadRegister(SOFTWARE1_SOFTWARE1_42_reg, 5,  5, &debug_flag2);//debug_2
#if 1//RMV info: get regional horizonal RMV info of top region and bottom region
	//top
	ReadRegister(KME_ME1_TOP4_KME_ME1_TOP4_00_reg,0,10,&rmv_temp);
	rmv_temp = ((rmv_temp >> 10) &1) == 0? (rmv_temp & 0x7ff) : -((rmv_temp & 0x7ff) - (1<<11));
	rmv_top = rmv_temp > rmv_top ? rmv_temp : rmv_top;
	ReadRegister(KME_ME1_TOP4_KME_ME1_TOP4_08_reg,0,10,&rmv_temp);
	rmv_temp = ((rmv_temp >> 10) &1) == 0? (rmv_temp & 0x7ff) : -((rmv_temp & 0x7ff) - (1<<11));
	rmv_top = rmv_temp > rmv_top ? rmv_temp : rmv_top;
	ReadRegister(KME_ME1_TOP4_KME_ME1_TOP4_10_reg,0,10,&rmv_temp);
	rmv_temp = ((rmv_temp >> 10) &1) == 0? (rmv_temp & 0x7ff) : -((rmv_temp & 0x7ff) - (1<<11));
	rmv_top = rmv_temp > rmv_top ? rmv_temp : rmv_top;
	ReadRegister(KME_ME1_TOP4_KME_ME1_TOP4_18_reg,0,10,&rmv_temp);
	rmv_temp = ((rmv_temp >> 10) &1) == 0? (rmv_temp & 0x7ff) : -((rmv_temp & 0x7ff) - (1<<11));
	rmv_top = rmv_temp > rmv_top ? rmv_temp : rmv_top;
	ReadRegister(KME_ME1_TOP4_KME_ME1_TOP4_20_reg,0,10,&rmv_temp);
	rmv_temp = ((rmv_temp >> 10) &1) == 0? (rmv_temp & 0x7ff) : -((rmv_temp & 0x7ff) - (1<<11));
	rmv_top = rmv_temp > rmv_top ? rmv_temp : rmv_top;
	ReadRegister(KME_ME1_TOP4_KME_ME1_TOP4_28_reg,0,10,&rmv_temp);
	rmv_temp = ((rmv_temp >> 10) &1) == 0? (rmv_temp & 0x7ff) : -((rmv_temp & 0x7ff) - (1<<11));
	rmv_top = rmv_temp > rmv_top ? rmv_temp : rmv_top;
	ReadRegister(KME_ME1_TOP4_KME_ME1_TOP4_30_reg,0,10,&rmv_temp);
	rmv_temp = ((rmv_temp >> 10) &1) == 0? (rmv_temp & 0x7ff) : -((rmv_temp & 0x7ff) - (1<<11));
	rmv_top = rmv_temp > rmv_top ? rmv_temp : rmv_top;
	ReadRegister(KME_ME1_TOP4_KME_ME1_TOP4_38_reg,0,10,&rmv_temp);
	rmv_temp = ((rmv_temp >> 10) &1) == 0? (rmv_temp & 0x7ff) : -((rmv_temp & 0x7ff) - (1<<11));
	rmv_top = rmv_temp > rmv_top ? rmv_temp : rmv_top;

	//bottom
	ReadRegister(KME_ME1_TOP4_KME_ME1_TOP4_C0_reg,0,10,&rmv_temp);
	rmv_temp = ((rmv_temp >> 10) &1) == 0? (rmv_temp & 0x7ff) : -((rmv_temp & 0x7ff) - (1<<11));
	rmv_bottom = rmv_temp > rmv_bottom ? rmv_temp : rmv_bottom;
	ReadRegister(KME_ME1_TOP4_KME_ME1_TOP4_C8_reg,0,10,&rmv_temp);
	rmv_temp = ((rmv_temp >> 10) &1) == 0? (rmv_temp & 0x7ff) : -((rmv_temp & 0x7ff) - (1<<11));
	rmv_bottom = rmv_temp > rmv_bottom ? rmv_temp : rmv_bottom;
	ReadRegister(KME_ME1_TOP4_KME_ME1_TOP4_D0_reg,0,10,&rmv_temp);
	rmv_temp = ((rmv_temp >> 10) &1) == 0? (rmv_temp & 0x7ff) : -((rmv_temp & 0x7ff) - (1<<11));
	rmv_bottom = rmv_temp > rmv_bottom ? rmv_temp : rmv_bottom;
	ReadRegister(KME_ME1_TOP4_KME_ME1_TOP4_D8_reg,0,10,&rmv_temp);
	rmv_temp = ((rmv_temp >> 10) &1) == 0? (rmv_temp & 0x7ff) : -((rmv_temp & 0x7ff) - (1<<11));
	rmv_bottom = rmv_temp > rmv_bottom ? rmv_temp : rmv_bottom;
	ReadRegister(KME_ME1_TOP4_KME_ME1_TOP4_E0_reg,0,10,&rmv_temp);
	rmv_temp = ((rmv_temp >> 10) &1) == 0? (rmv_temp & 0x7ff) : -((rmv_temp & 0x7ff) - (1<<11));
	rmv_bottom = rmv_temp > rmv_bottom ? rmv_temp : rmv_bottom;
	ReadRegister(KME_ME1_TOP4_KME_ME1_TOP4_E8_reg,0,10,&rmv_temp);
	rmv_temp = ((rmv_temp >> 10) &1) == 0? (rmv_temp & 0x7ff) : -((rmv_temp & 0x7ff) - (1<<11));
	rmv_bottom = rmv_temp > rmv_bottom ? rmv_temp : rmv_bottom;
	ReadRegister(KME_ME1_TOP4_KME_ME1_TOP4_F0_reg,0,10,&rmv_temp);
	rmv_temp = ((rmv_temp >> 10) &1) == 0? (rmv_temp & 0x7ff) : -((rmv_temp & 0x7ff) - (1<<11));
	rmv_bottom = rmv_temp > rmv_bottom ? rmv_temp : rmv_bottom;
	ReadRegister(KME_ME1_TOP4_KME_ME1_TOP4_F8_reg,0,10,&rmv_temp);
	rmv_temp = ((rmv_temp >> 10) &1) == 0? (rmv_temp & 0x7ff) : -((rmv_temp & 0x7ff) - (1<<11));
	rmv_bottom = rmv_temp > rmv_bottom ? rmv_temp : rmv_bottom;
#endif
#if 0
	//left
	ReadRegister(KME_ME1_TOP4_KME_ME1_TOP4_00_reg,11,20,&rmv_temp);
	rmv_left = (((rmv_temp >> 10) &1) == 0? (rmv_temp & 0x7ff) : -((rmv_temp & 0x7ff) - (1<<11)) > 2) ? 1: rmv_left;
	ReadRegister(KME_ME1_TOP4_KME_ME1_TOP4_40_reg,11,20,&rmv_temp);
	rmv_left = (((rmv_temp >> 10) &1) == 0? (rmv_temp & 0x7ff) : -((rmv_temp & 0x7ff) - (1<<11)) > 2) ? 1: rmv_left;
	ReadRegister(KME_ME1_TOP4_KME_ME1_TOP4_80_reg,11,20,&rmv_temp);
	rmv_left = (((rmv_temp >> 10) &1) == 0? (rmv_temp & 0x7ff) : -((rmv_temp & 0x7ff) - (1<<11)) > 2) ? 1: rmv_left;
	ReadRegister(KME_ME1_TOP4_KME_ME1_TOP4_C0_reg,11,20,&rmv_temp);
	rmv_left = (((rmv_temp >> 10) &1) == 0? (rmv_temp & 0x7ff) : -((rmv_temp & 0x7ff) - (1<<11)) > 2) ? 1: rmv_left;
	
	//right
	ReadRegister(KME_ME1_TOP4_KME_ME1_TOP4_38_reg,11,20,&rmv_temp);
	rmv_right = (((rmv_temp >> 10) &1) == 0? (rmv_temp & 0x7ff) : -((rmv_temp & 0x7ff) - (1<<11)) > 2) ? 1: rmv_right;
	ReadRegister(KME_ME1_TOP4_KME_ME1_TOP4_78_reg,11,20,&rmv_temp);
	rmv_right = (((rmv_temp >> 10) &1) == 0? (rmv_temp & 0x7ff) : -((rmv_temp & 0x7ff) - (1<<11)) > 2) ? 1: rmv_right;
	ReadRegister(KME_ME1_TOP4_KME_ME1_TOP4_B8_reg,11,20,&rmv_temp);
	rmv_right = (((rmv_temp >> 10) &1) == 0? (rmv_temp & 0x7ff) : -((rmv_temp & 0x7ff) - (1<<11)) > 2) ? 1: rmv_right;
	ReadRegister(KME_ME1_TOP4_KME_ME1_TOP4_F8_reg,11,20,&rmv_temp);
	rmv_right = (((rmv_temp >> 10) &1) == 0? (rmv_temp & 0x7ff) : -((rmv_temp & 0x7ff) - (1<<11)) > 2) ? 1: rmv_right;
#endif
#if 1//logo info: if top region has to many logo detect, top region doesnt count --> this is to fix little shaking of logo on top region
	ReadRegister(KME_LOGO2_KME_LOGO2_80_reg,20,29,&logo_cnt_temp);
	logo_cnt_top += logo_cnt_temp;
	ReadRegister(KME_LOGO2_KME_LOGO2_84_reg,20,29,&logo_cnt_temp);
	logo_cnt_top += logo_cnt_temp;
	ReadRegister(KME_LOGO2_KME_LOGO2_88_reg,20,29,&logo_cnt_temp);
	logo_cnt_top += logo_cnt_temp;
	ReadRegister(KME_LOGO2_KME_LOGO2_8C_reg,20,29,&logo_cnt_temp);
	logo_cnt_top += logo_cnt_temp;
	ReadRegister(KME_LOGO2_KME_LOGO2_90_reg,20,29,&logo_cnt_temp);
	logo_cnt_top += logo_cnt_temp;
	ReadRegister(KME_LOGO2_KME_LOGO2_94_reg,20,29,&logo_cnt_temp);
	logo_cnt_top += logo_cnt_temp;
	ReadRegister(KME_LOGO2_KME_LOGO2_98_reg,20,29,&logo_cnt_temp);
	logo_cnt_top += logo_cnt_temp;
	ReadRegister(KME_LOGO2_KME_LOGO2_9C_reg,20,29,&logo_cnt_temp);
	logo_cnt_top += logo_cnt_temp;
#endif
	ReadRegister(SOFTWARE1_SOFTWARE1_62_reg, 3, 3, &test_en2);

	u8_preCad_Id      = pOutput->u8_pre_cadence_Id[_FILM_ALL];
	if(u8_preCad_Id >= _FRC_CADENCE_NUM_){
		rtd_pr_memc_notice("[%s]Invalid u8_preCad_Id:%d,motion=%d.Resume!\n",\
			__FUNCTION__,u8_preCad_Id,_FILM_ALL);
		u8_preCad_Id = _CAD_VIDEO;
	}
	if(u8_preCad_Id == _CAD_32322){
		u1_preCad_motFlag  = cadence_seqFlag(frc_cadTable[u8_preCad_Id], ((pOutput->u8_phase_Idx[_FILM_ALL]+6)%12));
	}else{	
		u1_preCad_motFlag = cadence_seqFlag(frc_cadTable[u8_preCad_Id], pOutput->u8_phase_Idx[_FILM_ALL]);
	}
	u1_ipmeMot_flag  = pOutput->u32_Mot_sequence_for_mixmode & 0x01;
	det_cad = pOutput->u8_det_cadence_Id[_FILM_ALL];
	length = frc_cadTable[det_cad].cad_len;
	u32_match_seq_sft = frc_cadTable[det_cad].cad_seq;
	{
		int s11_me_GMV_1st_vx_rb,s10_me_GMV_1st_vy_rb,u17_me_GMV_1st_cnt_rb,u32_RB_val;
		ReadRegister(KME_ME1_TOP2_KME_ME1_TOP2_B0_reg, 0, 31, &u32_RB_val);
		s11_me_GMV_1st_vx_rb = ((u32_RB_val >> 10) &1) == 0? (u32_RB_val         & 0x7ff) : ((u32_RB_val         & 0x7ff)  - (1<<11));
		s10_me_GMV_1st_vy_rb = ((u32_RB_val >> 20) &1) == 0? ((u32_RB_val >> 11) & 0x3ff) : (((u32_RB_val >> 11) & 0x3ff)  - (1<<10));
		ReadRegister(KME_ME1_TOP2_KME_ME1_TOP2_C0_reg, 0, 16, &u32_RB_val);
		u17_me_GMV_1st_cnt_rb = u32_RB_val;
		if(pOutput->u1_mixMode && det_cad == _CAD_2224)
		{
			if(_ABS_(s10_me_GMV_1st_vy_rb) > 5 && _ABS_(s11_me_GMV_1st_vx_rb) < 2)// vertical panning for this case (400_Mixed_16)
				mix_cnt_hold_flag = 32;//becasue non key frame would get small gmv
			else
				mix_cnt_hold_flag>0?mix_cnt_hold_flag--:0;
		}
		ROSPrintf_MEMC_IP31_interface4("[Mixmode_4]%d,%d,%d, %d,%d,%d,%d\n",
		s11_me_GMV_1st_vx_rb,s10_me_GMV_1st_vy_rb,u17_me_GMV_1st_cnt_rb,mix_cnt_hold_flag,
		pOutput->u1_mixMode,pOutput->u8_mixMode_cnt,det_cad);
	}

	if ((u8_preCad_Id != _CAD_VIDEO && u1_preCad_motFlag == 0   ) || (u8_preCad_Id == _CAD_VIDEO && u1_ipmeMot_flag == 0))
	{
		u8_mixMode_cnt_p1 = 0;
		u8_mixMode_cnt_n1 = 0;
		check_flag = 1;
		#if MIX_MODE_REGION_17
		for (u8_k = _FILM_5RGN_TOP; u8_k <= _FILM_12RGN_12; u8_k ++)
		#else
		for (u8_k = _FILM_5RGN_TOP; u8_k <= _FILM_5RGN_CEN; u8_k ++)
		#endif
		{
			unsigned char  u8_preCad_Id_rgn  = pOutput->u8_pre_cadence_Id[u8_k];

			//unsigned int u28_smlMot_xGain  = (pOutput->u27_quit_smlMot[u8_k] * pParam->u4_mixMode_rgnMove_motWgt >> 3);
			#if MIX_MODE_REGION_17
			//if(u8_k <= _FILM_5RGN_CEN){
			u27_smlMot = (pOutput->u27_quit_smlMot[_FILM_12RGN_01]+pOutput->u27_quit_smlMot[_FILM_12RGN_02]+pOutput->u27_quit_smlMot[_FILM_12RGN_03]+pOutput->u27_quit_smlMot[_FILM_12RGN_04]+
					 pOutput->u27_quit_smlMot[_FILM_12RGN_05]+pOutput->u27_quit_smlMot[_FILM_12RGN_06]+pOutput->u27_quit_smlMot[_FILM_12RGN_07]+pOutput->u27_quit_smlMot[_FILM_12RGN_08]+
					 pOutput->u27_quit_smlMot[_FILM_12RGN_09]+pOutput->u27_quit_smlMot[_FILM_12RGN_10]+pOutput->u27_quit_smlMot[_FILM_12RGN_11]+pOutput->u27_quit_smlMot[_FILM_12RGN_12]);
			
			u28_smlMot_xGain  = (u27_smlMot * u4_rgnMove_motWgt >> 3);   // u4_mixMode_rgnMove_motWgt =10
			u28_smlMot_badTh  = u28_smlMot_xGain;
			//}else{
			//	u27_smlMot = pOutput->u27_quit_smlMot[_FILM_5RGN_CEN] + pOutput->u27_quit_smlMot[_FILM_5RGN_LFT] + pOutput->u27_quit_smlMot[_FILM_5RGN_RHT];
			//	u28_smlMot_xGain  = (u27_smlMot >> 4);	 // u4_mixMode_rgnMove_motWgt =10
			//	u28_smlMot_badTh  = u28_smlMot_xGain;
			//}
			#else
			u27_smlMot = (u8_k==_FILM_5RGN_CEN) ? pOutput->u27_quit_smlMot[_FILM_ALL] : pOutput->u27_quit_smlMot[_FILM_5RGN_CEN];
			u28_smlMot_xGain  = (u27_smlMot * u4_rgnMove_motWgt >> 3);   // u4_mixMode_rgnMove_motWgt =10
			u28_smlMot_badTh  = (u8_k == _FILM_5RGN_CEN)? _MIN_(u28_smlMot_xGain, 0x3000/*0x5000*/) : u28_smlMot_xGain;
			#endif
			u28_badMot_Th     = _MAX_(u28_smlMot_badTh, pParam->u27_mixMode_rgnMove_minMot);   //  u27_mixMode_rgnMove_minMot = 544
			u1_badMot_flag    = pOutput->u27_ipme_motionPool[u8_k][0] > u28_badMot_Th?   1 : 0;//not still = 1; still = 0

			//top region has to be horizonal and directional motion; top region has too many logo detect
			if((u8_k == _FILM_5RGN_TOP && logo_cnt_top > 2500)|| (u8_k == _FILM_5RGN_TOP && rmv_top < 1))//(u8_k == _FILM_5RGN_LFT&& rmv_left == 0) || (u8_k == _FILM_5RGN_RHT&& rmv_right == 0)
				u1_badMot_flag = 0;

			if(test_en2){
				u1_badCad_cond1 = (pOutput->u1_mixMode == 0 && (u8_preCad_Id_rgn != pOutput->u8_pre_cadence_Id[_FILM_5RGN_CEN]));//check if theres different between all and small regions
			}
			else{
				u1_badCad_cond1 = (pOutput->u1_mixMode == 0 && (u8_preCad_Id == _CAD_VIDEO || u8_preCad_Id_rgn != u8_preCad_Id));//check if theres different between all and small regions
			}
			badCad_cond1_record[u8_k] = u1_badCad_cond1;
			#if MIX_MODE_REGION_17
			u1_v1Cad_cond =  pOutput->u8_pre_cadence_Id[_FILM_5RGN_CEN]+pOutput->u8_pre_cadence_Id[_FILM_12RGN_02]+pOutput->u8_pre_cadence_Id[_FILM_12RGN_06]+pOutput->u8_pre_cadence_Id[_FILM_12RGN_10];
			u1_v2Cad_cond =  pOutput->u8_pre_cadence_Id[_FILM_5RGN_CEN]+pOutput->u8_pre_cadence_Id[_FILM_12RGN_03]+pOutput->u8_pre_cadence_Id[_FILM_12RGN_07]+pOutput->u8_pre_cadence_Id[_FILM_12RGN_11];
			u1_h1Cad_cond =  pOutput->u8_pre_cadence_Id[_FILM_12RGN_01]+pOutput->u8_pre_cadence_Id[_FILM_12RGN_02]+pOutput->u8_pre_cadence_Id[_FILM_12RGN_03]+pOutput->u8_pre_cadence_Id[_FILM_12RGN_04];
			u1_h2Cad_cond =  pOutput->u8_pre_cadence_Id[_FILM_12RGN_05]+pOutput->u8_pre_cadence_Id[_FILM_12RGN_06]+pOutput->u8_pre_cadence_Id[_FILM_12RGN_07]+pOutput->u8_pre_cadence_Id[_FILM_12RGN_08];
			u1_h3Cad_cond =  pOutput->u8_pre_cadence_Id[_FILM_12RGN_09]+pOutput->u8_pre_cadence_Id[_FILM_12RGN_10]+pOutput->u8_pre_cadence_Id[_FILM_12RGN_11]+pOutput->u8_pre_cadence_Id[_FILM_12RGN_12];
			
			u1_badCad_cond2 = (pOutput->u1_mixMode == 1 &&(( u8_k<_FILM_5RGN_CEN &&( u8_preCad_Id_rgn*4 !=u1_v1Cad_cond || u8_preCad_Id_rgn*4 !=u1_v2Cad_cond || u8_preCad_Id_rgn*4 !=u1_h1Cad_cond || u8_preCad_Id_rgn*4 !=u1_h2Cad_cond || u8_preCad_Id_rgn*4 !=u1_h3Cad_cond))
                                                       ||( u8_k>=_FILM_5RGN_CEN && CadenceComparison(pOutput,_FILM_5RGN_CEN,_FILM_12RGN_12) )));
			#else
			u1_badCad_cond2 = (pOutput->u1_mixMode == 1 &&((u8_preCad_Id_rgn != pOutput->u8_pre_cadence_Id[_FILM_5RGN_CEN] && u8_k!=_FILM_5RGN_CEN)
                                                                                                                       ||(u8_preCad_Id_rgn == _CAD_VIDEO && u8_k==_FILM_5RGN_CEN)));
			#endif
                     u1_badCad_flag    = (u1_badCad_cond1 || u1_badCad_cond2)? 1 : 0;
			// when cadence == 22, its more acceptable to not fall into mixmode, when rmv_bot_x is small, not let in (for JP medicine ad record)
			if(u8_k == _FILM_5RGN_BOT && rmv_bottom < 7 && u8_preCad_Id == _CAD_22)
				u1_badCad_flag = 0;

			u17_dbg_1 = (u17_dbg_1<<1) |u1_badMot_flag;
			u17_dbg_2 = (u17_dbg_2<<1) |u1_badCad_cond1;
			u17_dbg_3 = (u17_dbg_3<<1) |u1_badCad_cond2;

			//u1_badMot_status  = u1_badMot_status | u1_badMot_flag;
			u1_badMot_status  = u1_badMot_status | (u1_badMot_flag && pParam->u1_mixMode_rgnEn[u8_k-1]);
			u1_mixMode_status = u1_mixMode_status | (u1_badMot_flag && u1_badCad_flag && pParam->u1_mixMode_rgnEn[u8_k-1]);

			if((((pOutput->u5_dbg_param1_pre & BIT_2) >> 2) == 1) && pOutput->u32_dbg_cnt_hold > 0 &&  pOutput->u1_mixMode ==1)
			{
				rtd_pr_memc_info("(%d)(%d,%d)(%d,%d)(%d,%d,%d)\n", u8_k ,u1_badMot_flag,u1_badCad_flag,
					u28_badMot_Th,pOutput->u27_ipme_motionPool[u8_k][0],
					u28_smlMot_badTh,u28_smlMot_xGain,u27_smlMot);
			}
		}
		//find max of cnt stride
#if MIX_MODE_REGION_17
		for (u8_k = _FILM_5RGN_TOP; u8_k <= _FILM_12RGN_12; u8_k ++)
#else
		for (u8_k = _FILM_5RGN_TOP; u8_k <= _FILM_5RGN_CEN; u8_k ++)
#endif
		{
			//mixmode_cnt_stride = ((badCad_cond1_record[u8_k]*badCad_cond1_weighting[u8_k]) > mixmode_cnt_stride)? (badCad_cond1_record[u8_k]*badCad_cond1_weighting[u8_k]):mixmode_cnt_stride;
			mixmode_cnt_stride = ((badCad_cond1_record[u8_k]*badCad_cond1_weighting[u8_k]) > mixmode_cnt_stride)? (badCad_cond1_record[u8_k]*badCad_cond1_weighting[u8_k]):(mixmode_cnt_stride<1?1:mixmode_cnt_stride);
		}
#if 1 //vertical scrolling text in the middle
				if(s_pContext->_external_data._input_frameRate == _PQL_IN_60HZ && gstAppData.stMEMCData.uiDejudderLevel == 0 &&
					pOutput->u27_ipme_motionPool[_FILM_5RGN_LFT][0] < 250 && pOutput->u27_ipme_motionPool[_FILM_5RGN_RHT][0] < 150 && pOutput->u8_pre_cadence_Id[_FILM_5RGN_LFT] == _CAD_32 && /*pOutput->u8_pre_cadence_Id[_FILM_5RGN_RHT] == _CAD_32 &&*/
					pOutput->u27_ipme_motionPool[_FILM_5RGN_CEN][0] > 15000 && pOutput->u27_ipme_motionPool[_FILM_12RGN_02][0] > 1500 && pOutput->u27_ipme_motionPool[_FILM_12RGN_03][0] > 1500 && pOutput->u27_ipme_motionPool[_FILM_12RGN_06][0] > 1500 && pOutput->u27_ipme_motionPool[_FILM_12RGN_07][0] > 1500 && pOutput->u27_ipme_motionPool[_FILM_12RGN_10][0] > 1500 && pOutput->u27_ipme_motionPool[_FILM_12RGN_11][0] > 1500)
				{
					u1_mixMode_status = 1;
					u1_VScrollingtext_flag = 3;
				}
#endif
		u8_mixMode_cnt_n1 = pOutput->u8_mixMode_cnt > 0? (pOutput->u8_mixMode_cnt - 1) : 0;
		u8_mixMode_cnt_p1 = pOutput->u8_mixMode_cnt >= pParam->u8_mixMode_cnt_max? pOutput->u8_mixMode_cnt : (pOutput->u8_mixMode_cnt + mixmode_cnt_stride);
		pOutput->u8_mixMode_cnt =  u1_mixMode_status == 1?  u8_mixMode_cnt_p1 : ((mix_cnt_hold_flag ||u1_badMot_status)?  pOutput->u8_mixMode_cnt : u8_mixMode_cnt_n1);
		ROSPrintf_MEMC_IP31_interface5("[MixMode] (%d) %d, %d, %d, %d, %d\n",u1_VScrollingtext_flag, pOutput->u8_mixMode_cnt,u8_mixMode_cnt_p1,u8_mixMode_cnt_n1,u1_mixMode_status,gstAppData.stMEMCData.uiDejudderLevel);
	}

	u8_maxPossible_CadId = (pOutput->u1_mixMode==0) ? u8_preCad_Id : u8_maxPossible_CadId;
	//for mixmode@1112/11112, 1112/11112 is not easy to enter, thresold of entering will be reduced, so the maximum cnt will be reduced as well to avoid exit mixmode late
	u8_mixMode_cnt_max = (u8_maxPossible_CadId==_CAD_1112 || u8_maxPossible_CadId==_CAD_11112/*||u8_maxPossible_CadId==_CAD_2224*/) ? pParam->u8_mixMode_cnt_max/2 : pParam->u8_mixMode_cnt_max;
	pOutput->u8_mixMode_cnt = _CLIP_(pOutput->u8_mixMode_cnt, 0, u8_mixMode_cnt_max);
		
	if(mix_cnt_hold_flag == 0 && u1_VScrollingtext_flag == 0)	
	MixModeQuit(pParam, pOutput);

	u1_VScrollingtext_flag>0?u1_VScrollingtext_flag--:u1_VScrollingtext_flag;
	#if CONFIG_MC_8_BUFFER
		#if 1
		if(pOutput->u8_pre_cadence_Id[_FILM_ALL] != _CAD_VIDEO && pOutput->u1_mixMode==0
			&& ((pOutput->u32_Mot_sequence[_FILM_ALL]&((1<<length)-1)) == u32_match_seq_sft)) // for first time Film -> Mix 
		{
			unsigned char u8_mxMode_enterCnt_th =  (u8_maxPossible_CadId==_CAD_1112 || u8_maxPossible_CadId==_CAD_11112 /*||u8_maxPossible_CadId==_CAD_2224*/) ?
										(pParam->u8_mixMode_enter_cntTh/2) : pParam->u8_mixMode_enter_cntTh;//for mixmode@1112/11112, 1112/11112 is not easy to enter
			#if 1//seperate entering and exiting threshold
			if(pOutput->u8_mixMode_cnt >= u8_mxMode_enterCnt_th)
				pOutput->u1_mixMode = 1;
			if(pOutput->u8_mixMode_cnt < u8_mxMode_enterCnt_th - 2)
				pOutput->u1_mixMode = 0;
			#endif
		}
		else if(pOutput->u1_mixMode == 1){
			unsigned char u8_mxMode_enterCnt_th =  (u8_maxPossible_CadId==_CAD_1112 || u8_maxPossible_CadId==_CAD_11112 /*||u8_maxPossible_CadId==_CAD_2224*/) ?
										(pParam->u8_mixMode_enter_cntTh/2) : pParam->u8_mixMode_enter_cntTh;//for mixmode@1112/11112, 1112/11112 is not easy to enter
			if(pOutput->u8_mixMode_cnt >= u8_mxMode_enterCnt_th)
				pOutput->u1_mixMode = 1;
			if(pOutput->u8_mixMode_cnt < u8_mxMode_enterCnt_th - 2)
				pOutput->u1_mixMode = 0;
		}
		else if(pOutput->u8_pre_cadence_Id[_FILM_ALL] == _CAD_VIDEO /*&& pOutput->u1_mixMode==0*/){
			if(pOutput->u8_mixMode_cnt >= u8_mixMode_cnt_max-1)
				pOutput->u1_mixMode = 1;
			else
				pOutput->u1_mixMode = 0;
		}
		#else
		if((!(pOutput->u8_pre_cadence_Id[_FILM_ALL]==_CAD_VIDEO && pOutput->u1_mixMode==0)) 
			&& ((pOutput->u32_Mot_sequence[_FILM_ALL]&((1<<length)-1)) == u32_match_seq_sft)) // for first time Film -> Mix 
		{
			unsigned char u8_mxMode_enterCnt_th =  (u8_maxPossible_CadId==_CAD_1112 || u8_maxPossible_CadId==_CAD_11112 /*||u8_maxPossible_CadId==_CAD_2224*/) ?
										(pParam->u8_mixMode_enter_cntTh/2) : pParam->u8_mixMode_enter_cntTh;//for mixmode@1112/11112, 1112/11112 is not easy to enter
			#if 1//seperate entering and exiting threshold
			if(pOutput->u8_mixMode_cnt >= u8_mxMode_enterCnt_th)
				pOutput->u1_mixMode = 1;
			if(pOutput->u8_mixMode_cnt < u8_mxMode_enterCnt_th - 2)
				pOutput->u1_mixMode = 0;
			#endif
		}
		else if(pOutput->u8_pre_cadence_Id[_FILM_ALL] == _CAD_VIDEO /*&& pOutput->u1_mixMode==0*/){
			if(pOutput->u8_mixMode_cnt >= u8_mixMode_cnt_max-1)
				pOutput->u1_mixMode = 1;
			else
				pOutput->u1_mixMode = 0;
		}
		#endif
	#else
	//if(!(pOutput->u8_pre_cadence_Id[_FILM_ALL]==_CAD_VIDEO && pOutput->u1_mixMode==0)) // when (pOutput->u8_pre_cadence_Id[_FILM_ALL]!=_CAD_VIDEO || pOutput->u1_mixMode==1)
	{
		unsigned char u8_mxMode_enterCnt_th =  (u8_maxPossible_CadId==_CAD_1112 || u8_maxPossible_CadId==_CAD_11112||u8_maxPossible_CadId==_CAD_2224) ?
									(pParam->u8_mixMode_enter_cntTh/2) : pParam->u8_mixMode_enter_cntTh;//for mixmode@1112/11112, 1112/11112 is not easy to enter
		#if 1//seperate entering and exiting threshold
		if(pOutput->u8_mixMode_cnt >= u8_mxMode_enterCnt_th)
			pOutput->u1_mixMode = 1;
		if(pOutput->u8_mixMode_cnt < u8_mxMode_enterCnt_th - 2)
			pOutput->u1_mixMode = 0;
		#endif
	}
	#endif

	pOutput->u1_mixMode_chg = (u1_mixMode_pre == pOutput->u1_mixMode) ? 0 : 1;

#if 0	//debug
	ROSPrintf_MEMC_IP31_interface2("[MixMode](,%d,%d,%d,%d,%d,)(,%d,%d,%d,%d,)(,%d,%d,%d,%d,)(,%d,%d,%d,%d,)(,%d,%d,%d,%d)",
		pOutput->u27_ipme_motionPool[_FILM_5RGN_TOP][0],pOutput->u27_ipme_motionPool[_FILM_5RGN_BOT][0],pOutput->u27_ipme_motionPool[_FILM_5RGN_LFT][0],pOutput->u27_ipme_motionPool[_FILM_5RGN_RHT][0],pOutput->u27_ipme_motionPool[_FILM_5RGN_CEN][0],
		pOutput->u27_ipme_motionPool[_FILM_12RGN_01][0],pOutput->u27_ipme_motionPool[_FILM_12RGN_02][0],pOutput->u27_ipme_motionPool[_FILM_12RGN_03][0],pOutput->u27_ipme_motionPool[_FILM_12RGN_04][0],
		pOutput->u27_ipme_motionPool[_FILM_12RGN_05][0],pOutput->u27_ipme_motionPool[_FILM_12RGN_06][0],pOutput->u27_ipme_motionPool[_FILM_12RGN_07][0],pOutput->u27_ipme_motionPool[_FILM_12RGN_08][0],
		pOutput->u27_ipme_motionPool[_FILM_12RGN_09][0],pOutput->u27_ipme_motionPool[_FILM_12RGN_10][0],pOutput->u27_ipme_motionPool[_FILM_12RGN_11][0],pOutput->u27_ipme_motionPool[_FILM_12RGN_12][0],
		u28_badMot_Th, u1_preCad_motFlag, u1_ipmeMot_flag, u8_preCad_Id);
#endif
#if 0   //debug
	ROSPrintf_MEMC_IP31_interface1("[MixMode](,%d,%d,%d,%d,%d,)(,%d,%d,%d,%d,)(,%d,%d,%d,%d,)(,%d,%d,%d,%d,)(,%d,%d,%d,%d,%d,)(,%d,)(,%d,%d,%d)(,%d,%d,%d,%d,%d,%d,)(,%d,%d,%d,%d,)(,%d,%d,%d,)\n", 
					pOutput->u8_pre_cadence_Id[_FILM_5RGN_TOP],pOutput->u8_pre_cadence_Id[_FILM_5RGN_BOT],pOutput->u8_pre_cadence_Id[_FILM_5RGN_LFT],pOutput->u8_pre_cadence_Id[_FILM_5RGN_RHT],pOutput->u8_pre_cadence_Id[_FILM_5RGN_CEN],
					pOutput->u8_pre_cadence_Id[_FILM_12RGN_01],pOutput->u8_pre_cadence_Id[_FILM_12RGN_02],pOutput->u8_pre_cadence_Id[_FILM_12RGN_03],pOutput->u8_pre_cadence_Id[_FILM_12RGN_04],
					pOutput->u8_pre_cadence_Id[_FILM_12RGN_05],pOutput->u8_pre_cadence_Id[_FILM_12RGN_06],pOutput->u8_pre_cadence_Id[_FILM_12RGN_07],pOutput->u8_pre_cadence_Id[_FILM_12RGN_08],
					pOutput->u8_pre_cadence_Id[_FILM_12RGN_09],pOutput->u8_pre_cadence_Id[_FILM_12RGN_10],pOutput->u8_pre_cadence_Id[_FILM_12RGN_11],pOutput->u8_pre_cadence_Id[_FILM_12RGN_12],
					u1_mixMode_status, u1_badMot_status, pOutput->u8_mixMode_cnt,pParam->u8_mixMode_enter_cntTh,pOutput->u1_mixMode, 
					u8_preCad_Id,
					u1_badMot_flag, u1_badCad_flag, mix_cnt_hold_flag,
					u17_dbg_1, u17_dbg_2,u17_dbg_3,u1_preCad_motFlag,u27_smlMot, pOutput->u27_ipme_motionPool[_FILM_ALL][0],
					u8_mixMode_cnt_p1,u8_mixMode_cnt_n1,pOutput->u32_Mot_sequence[_FILM_ALL],check_flag,
					rmv_top,rmv_bottom,logo_cnt_top);
#endif

	if((((pOutput->u5_dbg_param1_pre & BIT_2) >> 2) == 1) && pOutput->u32_dbg_cnt_hold > 0 )
	{
		rtd_pr_memc_info("(%d,%d,%d,%d,%d)(%d,%d,%d,%d,%d)(%d,%d,%d,%d)(%d,%d,%d,%d)(%d,%d,%d,%d)(%d,%d,%d)(%d)\n",  u17_dbg_1, u17_dbg_2,u17_dbg_3,u1_preCad_motFlag,u27_smlMot,
					pOutput->u8_pre_cadence_Id[_FILM_5RGN_TOP],pOutput->u8_pre_cadence_Id[_FILM_5RGN_BOT],
					pOutput->u8_pre_cadence_Id[_FILM_5RGN_LFT],pOutput->u8_pre_cadence_Id[_FILM_5RGN_RHT],pOutput->u8_pre_cadence_Id[_FILM_5RGN_CEN],
					pOutput->u8_pre_cadence_Id[_FILM_12RGN_01],pOutput->u8_pre_cadence_Id[_FILM_12RGN_02],pOutput->u8_pre_cadence_Id[_FILM_12RGN_03],pOutput->u8_pre_cadence_Id[_FILM_12RGN_04],
					pOutput->u8_pre_cadence_Id[_FILM_12RGN_05],pOutput->u8_pre_cadence_Id[_FILM_12RGN_06],pOutput->u8_pre_cadence_Id[_FILM_12RGN_07],pOutput->u8_pre_cadence_Id[_FILM_12RGN_08],
					pOutput->u8_pre_cadence_Id[_FILM_12RGN_09],pOutput->u8_pre_cadence_Id[_FILM_12RGN_10],pOutput->u8_pre_cadence_Id[_FILM_12RGN_11],pOutput->u8_pre_cadence_Id[_FILM_12RGN_12],
					pOutput->u8_mixMode_cnt,pParam->u8_mixMode_enter_cntTh,pOutput->u1_mixMode, u8_preCad_Id);
		#if 0
		rtd_pr_memc_info("(%d,%d,%d,%d,%d)(%d,%d,%d,%d)(%d,%d,%d,%d)(%d,%d,%d,%d)\n", 
					pOutput->u27_ipme_motionPool[_FILM_5RGN_TOP][0],pOutput->u27_ipme_motionPool[_FILM_5RGN_BOT][0],
					pOutput->u27_ipme_motionPool[_FILM_5RGN_LFT][0],pOutput->u27_ipme_motionPool[_FILM_5RGN_RHT][0],pOutput->u27_ipme_motionPool[_FILM_5RGN_CEN][0],
					pOutput->u27_ipme_motionPool[_FILM_12RGN_01][0],pOutput->u27_ipme_motionPool[_FILM_12RGN_02][0],pOutput->u27_ipme_motionPool[_FILM_12RGN_03][0],pOutput->u27_ipme_motionPool[_FILM_12RGN_04][0],
					pOutput->u27_ipme_motionPool[_FILM_12RGN_05][0],pOutput->u27_ipme_motionPool[_FILM_12RGN_06][0],pOutput->u27_ipme_motionPool[_FILM_12RGN_07][0],pOutput->u27_ipme_motionPool[_FILM_12RGN_08][0],
					pOutput->u27_ipme_motionPool[_FILM_12RGN_09][0],pOutput->u27_ipme_motionPool[_FILM_12RGN_10][0],pOutput->u27_ipme_motionPool[_FILM_12RGN_11][0],pOutput->u27_ipme_motionPool[_FILM_12RGN_12][0]
					);
		rtd_pr_memc_info("(%d,%d,%d,%d,%d,%d)(%d,%d,%d,%d)(%d,%d,%d,%d)(%d,%d,%d,%d)\n",pOutput->u27_quit_smlMot[_FILM_ALL] ,
					pOutput->u27_quit_smlMot[_FILM_5RGN_TOP],pOutput->u27_quit_smlMot[_FILM_5RGN_BOT],
					pOutput->u27_quit_smlMot[_FILM_5RGN_LFT],pOutput->u27_quit_smlMot[_FILM_5RGN_RHT],pOutput->u27_quit_smlMot[_FILM_5RGN_CEN],
					pOutput->u27_quit_smlMot[_FILM_12RGN_01],pOutput->u27_quit_smlMot[_FILM_12RGN_02],pOutput->u27_quit_smlMot[_FILM_12RGN_03],pOutput->u27_quit_smlMot[_FILM_12RGN_04],
					pOutput->u27_quit_smlMot[_FILM_12RGN_05],pOutput->u27_quit_smlMot[_FILM_12RGN_06],pOutput->u27_quit_smlMot[_FILM_12RGN_07],pOutput->u27_quit_smlMot[_FILM_12RGN_08],
					pOutput->u27_quit_smlMot[_FILM_12RGN_09],pOutput->u27_quit_smlMot[_FILM_12RGN_10],pOutput->u27_quit_smlMot[_FILM_12RGN_11],pOutput->u27_quit_smlMot[_FILM_12RGN_12]
					);
		#endif

	}
	u1_mixMode_pre = pOutput->u1_mixMode;
}

VOID MidLayer_Film_phTable_proc(VOID)
{

}

extern unsigned char MEMC_Lib_get_memc_Freeze_flag(VOID);
extern unsigned char lowdelay_state_chg_flg;
VOID FilmDetect_UpdateOutput(_OUTPUT_FilmDetectCtrl *pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	unsigned char u8_k = 0;
	unsigned char mPullDownMode;
	unsigned int log_en = 0;
	unsigned int test_32322_BadEdit_en = 0;
	static unsigned char lowdelay_cnt = 0; // K-22968
	
	ReadRegister(SOFTWARE1_SOFTWARE1_60_reg, 30, 30, &log_en);
	ReadRegister(SOFTWARE1_SOFTWARE1_60_reg, 29, 29, &test_32322_BadEdit_en);
	if(LowDelay_mode == 1){
		lowdelay_cnt = 2;
	}

	for(u8_k = _FILM_ALL; u8_k < _FILM_MOT_NUM; u8_k++)
	{
		pOutput->u8_cur_cadence_out[u8_k] = pOutput->u8_cur_cadence_Id[u8_k];
		pOutput->u8_phT_phase_Idx_out[u8_k] = pOutput->u8_phT_phase_Idx[u8_k];

		if(CAD_T != PQL_FRCCaseSupportJudge(pOutput->u8_cur_cadence_Id[u8_k]))//not support & need to fallback
		{
			pOutput->u8_cur_cadence_out[u8_k] = _CAD_VIDEO;
			pOutput->u8_phT_phase_Idx_out[u8_k] = 0;
			g_cadence_change_flag = _CASE_NULL;
			if(log_en){
				rtd_pr_memc_notice("[%s][%d]\n\r", __FUNCTION__, __LINE__);
			}
		}
		
		// special case for real cinema on/off at cadence 4222
		// when 60(4222)in and 60out must keep 4222 sequence when truMotion off
		// when 60(32)in and 60out not need to do anything(default already 32 out)
		//if (s_pContext->_external_data._input_frameRate == _PQL_IN_60HZ && s_pContext->_external_data._output_frameRate == _PQL_OUT_60HZ &&
		//	s_pContext->_external_data.u3_memcMode == MEMC_OFF/*MEMC_OFF*/ && pOutput->u8_cur_cadence_Id[_FILM_ALL] == _CAD_2224)
		if(s_pContext->_external_data._input_frameRate == _PQL_IN_60HZ && s_pContext->_external_data.u3_memcMode == MEMC_OFF/*MEMC_OFF*/ && 
		  	 (pOutput->u8_cur_cadence_Id[_FILM_ALL] == _CAD_2224 || pOutput->u8_cur_cadence_Id[_FILM_ALL] == _CAD_22224 || pOutput->u8_cur_cadence_Id[_FILM_ALL] == _CAD_32322))
		{
			pOutput->u8_cur_cadence_out[u8_k] = _CAD_VIDEO;
			pOutput->u8_phT_phase_Idx_out[u8_k] = 0;
			g_cadence_change_flag = _CASE_NULL;
			if(log_en){
				rtd_pr_memc_notice("[%s][%d]\n\r", __FUNCTION__, __LINE__);
			}
		}

		mPullDownMode = s_pContext->_external_data.u8_pulldown_mode;
		if(pOutput->u8_cur_cadence_Id[u8_k] != _CAD_VIDEO && (s_pContext->_external_data.u3_memcMode == MEMC_OFF || s_pContext->_external_data.u3_memcMode == MEMC_MODE_MAX) &&
			mPullDownMode != _PQL_IN60HZ_TRUMOTOFF_CINEMAON)
		{
			pOutput->u8_cur_cadence_out[u8_k] = _CAD_VIDEO;
			pOutput->u8_phT_phase_Idx_out[u8_k] = 0;
			g_cadence_change_flag = _CASE_NULL;
			if(log_en){
				rtd_pr_memc_notice("[%s][%d]\n\r", __FUNCTION__, __LINE__);
			}
		}

		if(pOutput->u8_cur_cadence_Id[u8_k] != _CAD_VIDEO && ((lowdelay_cnt > 0) || s_pContext->_external_data._output_mode == _PQL_OUT_PC_MODE))
		{
			pOutput->u8_cur_cadence_out[u8_k] = _CAD_VIDEO;
			pOutput->u8_phT_phase_Idx_out[u8_k] = 0;
			g_cadence_change_flag = _CASE_NULL;
			if(log_en){
				rtd_pr_memc_notice("[%s][%d]\n\r", __FUNCTION__, __LINE__);
			}
		}
		if(pOutput->u8_cur_cadence_Id[u8_k] != _CAD_VIDEO && (MEMC_Lib_get_memc_PowerSaving_Mode()==2 || Get_DISPLAY_PANEL_TYPE()== P_VBY1_TO_HDMI_MEMC_FREERUN)){
			pOutput->u8_cur_cadence_out[u8_k] = _CAD_VIDEO;
			pOutput->u8_phT_phase_Idx_out[u8_k] = 0;
			g_cadence_change_flag = _CASE_NULL;
			if(log_en){
				rtd_pr_memc_notice("[%s][%d]\n\r", __FUNCTION__, __LINE__);
			}
		}

		if(pOutput->u8_cur_cadence_Id[u8_k] != _CAD_VIDEO && MEMC_Lib_get_memc_Freeze_flag()){
			pOutput->u8_cur_cadence_out[u8_k] = _CAD_VIDEO;
			pOutput->u8_phT_phase_Idx_out[u8_k] = 0;
			g_cadence_change_flag = _CASE_NULL;
			if(log_en){
				rtd_pr_memc_notice("[%s][%d]\n\r", __FUNCTION__, __LINE__);
			}
		}

		if((test_32322_BadEdit_en && pOutput->u8_32322_BadEdit_cnt2 > 50)){
			pOutput->u8_cur_cadence_out[u8_k] = _CAD_VIDEO;
			pOutput->u8_phT_phase_Idx_out[u8_k] = 0;
			g_cadence_change_flag = _CASE_NULL;
			if(log_en){
				rtd_pr_memc_notice("[%s][%d]\n\r", __FUNCTION__, __LINE__);
			}
		}

#if 0 //Special_60_BadEdit
		if((test_32322_BadEdit_en && pOutput->u8_Special_60_BadEdit_cnt > 50)){
			pOutput->u8_cur_cadence_out[u8_k] = _CAD_VIDEO;
			pOutput->u8_phT_phase_Idx_out[u8_k] = 0;
			g_cadence_change_flag = _CASE_NULL;
			if(log_en){
				rtd_pr_memc_notice("[%s][%d]\n\r", __FUNCTION__, __LINE__);
			}
		}
#endif
		if(pOutput->u8_cur_cadence_Id[u8_k] != _CAD_VIDEO && lowdelay_state_chg_flg == 1){
			pOutput->u8_cur_cadence_out[u8_k] = _CAD_VIDEO;
			pOutput->u8_phT_phase_Idx_out[u8_k] = 0;
			g_cadence_change_flag = _CASE_NULL;
			if(log_en){
				rtd_pr_memc_notice("[%s][%d]\n\r", __FUNCTION__, __LINE__);
			}
		}
	}

	//no support & need fallback
	if(CAD_F ==PQL_FRCCaseSupportJudge(pOutput->u8_cur_cadence_Id[_FILM_ALL]))
		pOutput->u8_flbk_lvl_cad_cnt = 4;
	else if(pOutput->u8_flbk_lvl_cad_cnt > 0)
		pOutput->u8_flbk_lvl_cad_cnt = pOutput->u8_flbk_lvl_cad_cnt-1;
	else
		pOutput->u8_flbk_lvl_cad_cnt = 0;

	if(lowdelay_cnt > 0){ // K-22968
		lowdelay_cnt--;
	}
}

extern VOID MEMC_Lib_set_memc_do_power_saving_flag(unsigned char u1_enable);
VOID FilmDetect_PowerSavingJudge(const _PARAM_FilmDetectCtrl *pParam, _OUTPUT_FilmDetectCtrl *pOutput)
{
	unsigned char u8_k = 0;
	unsigned int test_val = 0;
	unsigned int PowerSavingJudge_en = 0;
	unsigned int PowerSavingJudge_th = 0; //PowerSavingJudge_Th
	unsigned int PowerSavingJudge_log_en = 0;
	unsigned int FrmMotion_all = 0;
	unsigned int small_motion_cnt = 0;
	ReadRegister(SOFTWARE1_SOFTWARE1_12_reg, 5, 12, &test_val);
	ReadRegister(SOFTWARE1_SOFTWARE1_12_reg, 14, 14, &PowerSavingJudge_log_en);
	ReadRegister(SOFTWARE1_SOFTWARE1_12_reg, 15, 15, &PowerSavingJudge_en);
	ReadRegister(SOFTWARE1_SOFTWARE1_12_reg, 16, 31, &PowerSavingJudge_th);
	ReadRegister(KME_IPME_KME_IPME_A8_reg, 0, 26, &FrmMotion_all);

	if(PowerSavingJudge_en){
		pOutput->u27_FrameMotionPool[0] = FrmMotion_all;
		for(u8_k = 60-1; u8_k > 0; u8_k--)
		{
			if(pOutput->u27_FrameMotionPool[u8_k] < pParam->u27_Mot_all_min){
				small_motion_cnt++;
			}
			pOutput->u27_FrameMotionPool[u8_k] = pOutput->u27_FrameMotionPool[u8_k-1];
		}

		for(u8_k = _FILM_ALL; u8_k < _FILM_MOT_NUM; u8_k++)
		{
			//if(pOutput->u8_cur_cadence_out[u8_k] != _CAD_VIDEO){
			if(pOutput->u8_det_cadence_Id[u8_k] != _CAD_VIDEO){
				pOutput->u32_FilmFrame_cnt[u8_k] = _MIN_(pOutput->u32_FilmFrame_cnt[u8_k] + (((PowerSavingJudge_th - pOutput->u32_FilmFrame_cnt[u8_k])/test_val) + 1), PowerSavingJudge_th);
			}else if(small_motion_cnt == 60){
				pOutput->u32_FilmFrame_cnt[u8_k] = (pOutput->u32_FilmFrame_cnt[u8_k] <= 0) ? pOutput->u32_FilmFrame_cnt[u8_k] : pOutput->u32_FilmFrame_cnt[u8_k]-1;
			}else{
				pOutput->u32_FilmFrame_cnt[u8_k] = (pOutput->u32_FilmFrame_cnt[u8_k] <= 0) ? pOutput->u32_FilmFrame_cnt[u8_k] : pOutput->u32_FilmFrame_cnt[u8_k]-1; //(pOutput->u1_mixMode == 1) ? pOutput->u32_FilmFrame_cnt[u8_k] :
			}
		}

#if 1
		if((pOutput->u32_FilmFrame_cnt[_FILM_ALL] <= 0))
#else
		if((pOutput->u32_FilmFrame_cnt[_FILM_ALL] <= 0) && (pOutput->u32_FilmFrame_cnt[_FILM_5RGN_TOP] <= 0) && (pOutput->u32_FilmFrame_cnt[_FILM_5RGN_BOT] <= 0)
            && (pOutput->u32_FilmFrame_cnt[_FILM_5RGN_LFT] <= 0) && (pOutput->u32_FilmFrame_cnt[_FILM_5RGN_RHT] <= 0) && (pOutput->u32_FilmFrame_cnt[_FILM_5RGN_CEN] <= 0)
            && (pOutput->u32_FilmFrame_cnt[_FILM_12RGN_01] <= 0) && (pOutput->u32_FilmFrame_cnt[_FILM_12RGN_02] <= 0) && (pOutput->u32_FilmFrame_cnt[_FILM_12RGN_03] <= 0)
            && (pOutput->u32_FilmFrame_cnt[_FILM_12RGN_04] <= 0) && (pOutput->u32_FilmFrame_cnt[_FILM_12RGN_05] <= 0) && (pOutput->u32_FilmFrame_cnt[_FILM_12RGN_06] <= 0)
            && (pOutput->u32_FilmFrame_cnt[_FILM_12RGN_07] <= 0) && (pOutput->u32_FilmFrame_cnt[_FILM_12RGN_08] <= 0) && (pOutput->u32_FilmFrame_cnt[_FILM_12RGN_09] <= 0)
            && (pOutput->u32_FilmFrame_cnt[_FILM_12RGN_10] <= 0) && (pOutput->u32_FilmFrame_cnt[_FILM_12RGN_11] <= 0) && (pOutput->u32_FilmFrame_cnt[_FILM_12RGN_12] <= 0))
#endif
		{
			MEMC_Lib_set_memc_do_power_saving_flag(1);
		}else{
			MEMC_Lib_set_memc_do_power_saving_flag(0);
		}

		if((pOutput->u32_FilmFrame_cnt[_FILM_ALL] < PowerSavingJudge_th) && PowerSavingJudge_log_en){
			rtd_pr_memc_crit("[%s][,%d,%d,%d,%d,%d,%d,]\n\r", __FUNCTION__,
				pOutput->u32_FilmFrame_cnt[_FILM_ALL], pOutput->u32_FilmFrame_cnt[_FILM_5RGN_TOP], pOutput->u32_FilmFrame_cnt[_FILM_5RGN_BOT],
				pOutput->u32_FilmFrame_cnt[_FILM_5RGN_LFT], pOutput->u32_FilmFrame_cnt[_FILM_5RGN_RHT], pOutput->u32_FilmFrame_cnt[_FILM_5RGN_CEN]);
		}
	}
	else{
		for(u8_k = _FILM_ALL; u8_k < _FILM_MOT_NUM; u8_k++)
		{
			pOutput->u32_FilmFrame_cnt[u8_k] = 0;
		}
		for(u8_k = 0; u8_k < 60; u8_k++)
		{
  			pOutput->u27_FrameMotionPool[u8_k] = 0;
		}
		MEMC_Lib_set_memc_do_power_saving_flag(0);
	}
}

/////////////////// External functions  ////////////////////////////////////////////////////////////
VOID FilmDetect_Init(_OUTPUT_FilmDetectCtrl *pOutput)
{
	unsigned char u8_k, u8_i;

	for (u8_k = 0; u8_k < _FILM_MOT_NUM; u8_k ++)
	{
		for (u8_i = 0; u8_i < _FRC_CADENCE_NUM_; u8_i ++)
		{
			u8_filmCad_enterCnt[u8_i][u8_k] = 0;
		}

		for (u8_i = 0; u8_i < 8; u8_i ++)
		{
			pOutput->u27_ipme_motionPool[u8_k][u8_i] = 0;
		}

		pOutput->u32_Mot_sequence[u8_k]  = 0;

		pOutput->u8_det_cadence_Id_pre[u8_k] = _CAD_VIDEO;
		pOutput->u8_det_cadence_Id[u8_k] = _CAD_VIDEO;
		pOutput->u8_cur_cadence_Id[u8_k] = _CAD_VIDEO;
		pOutput->u8_pre_cadence_Id[u8_k] = _CAD_VIDEO;
		pOutput->u8_phase_Idx[u8_k]      = 0;
		pOutput->u8_phase_Idx_tmp[u8_k]      = 0;
		pOutput->u8_pre_phase_Idx[u8_k]      = 0;

		pOutput->u27_quit_bigMot[u8_k] = 0x1FF;
		pOutput->u27_quit_smlMot[u8_k] = 0;
		pOutput->u27_quit_motTh[u8_k]  = 0;
		pOutput->u8_quit_cnt[u8_k]     = 0;
		pOutput->u8_quit_prd[u8_k]     = 0;

		pOutput->u8_phT_phase_Idx[u8_k] = 0;

		pOutput->u27_FrmMotion_S[u8_k] = 0;

		pOutput->u8_cur_cadence_out[u8_k] = _CAD_VIDEO;
		pOutput->u8_phT_phase_Idx_out[u8_k] = 0;

		pOutput->u27_g_FrmMot_Th[u8_k] = 0;
		
	}

	for(u8_k = 0; u8_k < 26; u8_k ++){
		pOutput->u27_FrmMotionSort[u8_k] = 0;
		pOutput->u27_FrmMotionSortTick[u8_k] = 0;
	}

	pOutput->u5_dbg_param1_pre     = 0;
	pOutput->u32_dbg_param2_pre    = 0;
	pOutput->u32_dbg_cnt_hold      = 0;
	pOutput->u8_dbg_motion_t = _FILM_ALL;

	pOutput->u8_flbk_lvl_cad_cnt = 0;
	
	pOutput->u1_mixMode = 0;
	pOutput->u8_mixMode_cnt = 0;
	pOutput->u1_mixMode_chg = 0;

	pOutput->u1_32322_BadEdit_flag = 0;
	pOutput->u8_32322_BadEdit_cnt = 0;
	pOutput->u8_32322_BadEdit_idx = 0;

	pOutput->u1_32322_BadEdit_flag2 = 0;
	pOutput->u8_32322_BadEdit_cnt2 = 0;
	pOutput->u8_32322_BadEdit_idx2 = 0;
	
	pOutput->u1_Special_60_BadEdit_flag = 0;
	pOutput->u8_Special_60_BadEdit_cnt = 0;
	pOutput->u8_Special_60_BadEdit_idx = 0;

	pOutput->u1_Special_50_BadEdit_flag = 0;
	pOutput->u8_Special_50_BadEdit_cnt = 0;
	pOutput->u8_Special_50_BadEdit_idx = 0;

	pOutput->u1_Special_30_BadEdit_flag = 0;
	pOutput->u8_Special_30_BadEdit_cnt = 0;
	pOutput->u8_Special_30_BadEdit_idx = 0;
	
	pOutput->u8_BadEdit_flag = 0;
	for(u8_k = 0; u8_k < KEEP_FRAME_NUM; u8_k ++){
		pOutput->u8_cadence_sequence[u8_k] = 0;
	}

	for(u8_k = 0; u8_k < _CAD_CHANGE_QUE_BUF_FRM; u8_k++){
		g_cadence_change_que.state[u8_k] = 0;
		g_cadence_change_que.type[u8_k] = 0;
		g_cadence_change_que.mc_wrt_index[u8_k] = 0;
	}
}


//Input signal and parameters are connected locally
VOID FilmDetectCtrl_Proc(const _PARAM_FilmDetectCtrl *pParam, _OUTPUT_FilmDetectCtrl *pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	unsigned char mPullDownMode;
	unsigned char u8_k = 0;
	unsigned char debug_info[12];
	unsigned int log_motion, log_en;
	ReadRegister(SOFTWARE3_SOFTWARE3_09_reg, 0, 0, &log_en);
	ReadRegister(SOFTWARE3_SOFTWARE3_09_reg, 1, 1, &log_motion);

	if(log_motion)//bit1
	{
	   rtd_printk(KERN_CRIT, TAG_NAME_MEMC, "[MOT] %d\n", pOutput->u32_Mot_sequence[_FILM_ALL] & 0x1 );
    }

	pOutput->u8_dbg_motion_t = _CLIP_(pParam->u3_DbgPrt_Rgn, _FILM_ALL, _FILM_5RGN_CEN);

	if (pParam->u1_filmDet_bypass == 1)
	{
		pOutput->u8_det_cadence_Id[_FILM_ALL] = _CAD_VIDEO;
		pOutput->u8_phase_Idx[_FILM_ALL]      = 0;

		pOutput->u8_cur_cadence_Id[_FILM_ALL] = (FRC_CADENCE_ID)(s_pContext->_output_read_comreg.u3_ipme_filmMode_rb>>1);
	}
	else if ((s_pContext->_external_data._input_3dFormat != _PQL_IN_LR) ||\
		((s_pContext->_external_data._input_3dFormat == _PQL_IN_LR) && (s_pContext->_output_read_comreg.u1_kphase_inLR == _PQL_3D_FRAME_L)))
	{
        pOutput->u32_dbg_cnt_hold = (pOutput->u5_dbg_param1_pre != pParam->u5_dbg_param1 || pOutput->u32_dbg_param2_pre!= pParam->u32_dbg_param2)?\
			                         pParam->u32_dbg_cnt : (pOutput->u32_dbg_cnt_hold > 0 ? (pOutput->u32_dbg_cnt_hold - 1) : 0);

		Motion_Sequence_Gen(pParam, pOutput, _FILM_ALL);
		CadenceDetect(pParam,pOutput, _FILM_ALL);
		Film_motThr_calc(pParam,pOutput, _FILM_ALL);
		//Film_occasionally_fastOut(pParam,pOutput);
		debug_info[0] = pOutput->u8_cur_cadence_Id[_FILM_ALL];

		if((pParam->u1_mixMode_det_en == 1) && (pParam->u1_mixMode_en == 1)&&(s_pContext->_output_wrt_comreg.u1_FILM_force_mix_true ==1))
			pOutput->u1_mixMode =1;
		else if ((pParam->u1_mixMode_det_en == 1) && (pParam->u1_mixMode_en == 1)&&(s_pContext->_output_me_sceneAnalysis.u8_EPG_apply== 0))
		{
			#if MIX_MODE_REGION_17
			for (u8_k = _FILM_5RGN_TOP; u8_k <= _FILM_12RGN_12; u8_k ++)
			#else
			for (u8_k = _FILM_5RGN_TOP; u8_k <= _FILM_5RGN_CEN; u8_k ++)
			#endif
			{
				Motion_Sequence_Gen(pParam, pOutput, (_PQL_FILM_MOT_TYPE)u8_k);
				CadenceDetect(pParam, pOutput, (_PQL_FILM_MOT_TYPE)u8_k);
				Film_motThr_calc(pParam, pOutput, (_PQL_FILM_MOT_TYPE)u8_k);

				if (pOutput->u8_pre_cadence_Id[u8_k] == _CAD_VIDEO)
				{
					FilmEnter(pParam, pOutput, (_PQL_FILM_MOT_TYPE)u8_k);
				}
				else
				{
					FilmQuit(pParam, pOutput, (_PQL_FILM_MOT_TYPE)u8_k);
				}

				//cadence+timing support or not
//				if(CAD_N == PQL_FRCCaseSupportJudge(pOutput->u8_cur_cadence_Id[u8_k]))
//				{
//					pOutput->u8_cur_cadence_Id[u8_k] = _CAD_VIDEO;
//				}

				PhaseIdx_set(pParam, pOutput, (_PQL_FILM_MOT_TYPE)u8_k);

				pOutput->u8_pre_cadence_Id[u8_k] = pOutput->u8_cur_cadence_Id[u8_k];
			}

			MixMode_detect(pParam, pOutput);
		}
		else
		{
			pOutput->u1_mixMode     = 0;
			pOutput->u8_mixMode_cnt = 0;
		}
		debug_info[1] = pOutput->u8_cur_cadence_Id[_FILM_ALL];
		debug_info[7] = pOutput->u8_det_cadence_Id[_FILM_ALL];
		//if (pOutput->u8_pre_cadence_Id[_FILM_ALL] == _CAD_VIDEO && pOutput->u1_mixMode == 0)
		//		&& (pOutput->u8_det_cadence_Id[_FILM_ALL] != _CAD_1112
		//if (pOutput->u8_pre_cadence_Id[_FILM_ALL] == _CAD_VIDEO /*&& pOutput->u1_mixMode == 0*/)
		if (pOutput->u8_pre_cadence_Id[_FILM_ALL] == _CAD_VIDEO && ( pOutput->u1_mixMode == 0 || pOutput->u8_det_cadence_Id[_FILM_ALL] == _CAD_1112 || pOutput->u8_det_cadence_Id[_FILM_ALL] == _CAD_11112 ) )
		{
			FilmEnter(pParam, pOutput, _FILM_ALL);
			debug_info[2] = 100+pOutput->u8_cur_cadence_Id[_FILM_ALL];
		}
		else
		{
			FilmQuit(pParam, pOutput, _FILM_ALL);
			debug_info[2] = pOutput->u8_cur_cadence_Id[_FILM_ALL];
		}

		//pull down mode
		//input 25hz/30hz/50hz/60hz, TruMotion off, Cinema off => clear cadence to avoid incorrect pulldown
		mPullDownMode = s_pContext->_external_data.u8_pulldown_mode;
		if(pOutput->u8_cur_cadence_Id[_FILM_ALL] != _CAD_VIDEO && s_pContext->_external_data._input_frameRate == _PQL_IN_24HZ &&
			/*mPullDownMode >= _PQL_IN24HZ_TRUMOTOFF_CINEMAON &&*/ mPullDownMode <= _PQL_IN24HZ_TRUMOTOFF_CINEMAOFF)
		{
			pOutput->u8_cur_cadence_Id[_FILM_ALL] = _CAD_VIDEO;  
		}
		debug_info[3] = pOutput->u8_cur_cadence_Id[_FILM_ALL];

		// FF or FB force to video
		if((s_pContext->_external_info.s16_speed != 256) && (s_pContext->_external_info.s16_speed != -256))
		{
			pOutput->u8_cur_cadence_Id[_FILM_ALL] = _CAD_VIDEO;   
		}
		debug_info[4] = pOutput->u8_cur_cadence_Id[_FILM_ALL];

		//cadence+timing support or not
		if(CAD_N == PQL_FRCCaseSupportJudge( pOutput->u8_cur_cadence_Id[_FILM_ALL] ))
		{
			pOutput->u8_cur_cadence_Id[_FILM_ALL] = _CAD_VIDEO;    
		}
		debug_info[5] = pOutput->u8_cur_cadence_Id[_FILM_ALL];

		Film_occasionally_fastOut(pParam,pOutput);   

		debug_info[6] = pOutput->u8_cur_cadence_Id[_FILM_ALL];

		debug_info[8] = PhaseIdx_set(pParam, pOutput, _FILM_ALL);

		debug_info[9] = pOutput->u8_cur_cadence_Id[_FILM_ALL];

		FilmDetect_BadEditProtect(pParam, pOutput);

		debug_info[10] = pOutput->u8_cur_cadence_Id[_FILM_ALL];

		CadenceChangeDetect(pParam, pOutput);//for cadence smooth change

		debug_info[11] = pOutput->u8_cur_cadence_Id[_FILM_ALL];

		pOutput->u8_det_cadence_Id_pre[_FILM_ALL] = pOutput->u8_det_cadence_Id[_FILM_ALL];
		pOutput->u8_pre_cadence_Id[_FILM_ALL] = pOutput->u8_cur_cadence_Id[_FILM_ALL];
		pOutput->u8_pre_phase_Idx[_FILM_ALL] = pOutput->u8_phase_Idx[_FILM_ALL];
	
		if(0) //((((pOutput->u5_dbg_param1_pre & BIT_2) >> 2) == 1) && pOutput->u32_dbg_cnt_hold > 0)
	        {
	    	   rtd_pr_memc_info("[MEMC]%s::cur_cadence_Id=%d, phase_ID=%d, phT_phase_ID=%d \n",__FUNCTION__,pOutput->u8_cur_cadence_Id[_FILM_ALL],pOutput->u8_phase_Idx[_FILM_ALL],pOutput->u8_phT_phase_Idx[_FILM_ALL]);
	        }
	
		pOutput->u5_dbg_param1_pre  = pParam->u5_dbg_param1;
		pOutput->u32_dbg_param2_pre = pParam->u32_dbg_param2;
	}

	//update output to other module
	//pOutput->u8_cur_cadence_out[u8_k]
	//pOutput->u8_phT_phase_Idx_out[u8_k]
	#if (!Pro_tv002 & Pro_tv030)
	//  mc bld
	if( pOutput->u8_cur_cadence_Id[_FILM_ALL]==_CAD_55)  // low frame rate
		WriteRegister(MC_MC_28_reg,11, 13, 0x4);
	else
		WriteRegister(MC_MC_28_reg,11, 13, 0x3);
	#endif
	FilmDetect_UpdateOutput(pOutput);
	FilmDetect_PowerSavingJudge(pParam, pOutput);

	// Pass out the film information
	g_scaler_memc_cadence[_FILM_MEMC_ALL] = pOutput->u8_cur_cadence_Id[_FILM_ALL];
	g_scaler_memc_cadence[_FILM_MEMC_TOP] = ( pParam->u1_mixMode_det_en == 1 )? pOutput->u8_cur_cadence_Id[_FILM_5RGN_TOP] : 255;
	g_scaler_memc_cadence[_FILM_MEMC_BOT] = ( pParam->u1_mixMode_det_en == 1 )? pOutput->u8_cur_cadence_Id[_FILM_5RGN_BOT] : 255;
	g_scaler_memc_cadence[_FILM_MEMC_LFT] = ( pParam->u1_mixMode_det_en == 1 )? pOutput->u8_cur_cadence_Id[_FILM_5RGN_LFT] : 255;
	g_scaler_memc_cadence[_FILM_MEMC_RHT] = ( pParam->u1_mixMode_det_en == 1 )? pOutput->u8_cur_cadence_Id[_FILM_5RGN_RHT] : 255;
	g_scaler_memc_cadence[_FILM_MEMC_CEN] = ( pParam->u1_mixMode_det_en == 1 )? pOutput->u8_cur_cadence_Id[_FILM_5RGN_CEN] : 255;
	g_scaler_memc_cadence[_FILM_MEMC_OUT_ALL] = pOutput->u8_cur_cadence_out[_FILM_ALL];

	if(log_en) {
		rtd_printk(KERN_CRIT, TAG_NAME_MEMC, "[FD] %d%d%d%d%d%d%d%d%d%d%d%d E:%d id:%d %d %d %d %d %d %d %d %d %d	Ph_id:%d	Mot:%u	det:%d  Cad:%d, Out:%d\n", 		
		(pOutput->u32_Mot_sequence[_FILM_ALL]>>11) & 0x1,
		(pOutput->u32_Mot_sequence[_FILM_ALL]>>10) & 0x1,
		(pOutput->u32_Mot_sequence[_FILM_ALL]>>9) & 0x1,
		(pOutput->u32_Mot_sequence[_FILM_ALL]>>8) & 0x1,
		(pOutput->u32_Mot_sequence[_FILM_ALL]>>7) & 0x1,
		(pOutput->u32_Mot_sequence[_FILM_ALL]>>6) & 0x1,
		(pOutput->u32_Mot_sequence[_FILM_ALL]>>5) & 0x1,
		(pOutput->u32_Mot_sequence[_FILM_ALL]>>4) & 0x1,
		(pOutput->u32_Mot_sequence[_FILM_ALL]>>3) & 0x1,
		(pOutput->u32_Mot_sequence[_FILM_ALL]>>2) & 0x1,
		(pOutput->u32_Mot_sequence[_FILM_ALL]>>1) & 0x1,
		pOutput->u32_Mot_sequence[_FILM_ALL] & 0x1,
			pOutput->ExchangeCad, debug_info[0], debug_info[1], debug_info[2], debug_info[3], debug_info[4], debug_info[5], debug_info[6], debug_info[9], debug_info[10], debug_info[11],
			debug_info[8], pOutput->u32_Mot_sequence[_FILM_ALL], debug_info[7], g_scaler_memc_cadence[_FILM_MEMC_ALL], g_scaler_memc_cadence[_FILM_MEMC_OUT_ALL] );
	}
}
