#define VODMA_START_REGISTER_0                                                  (0xb8005000)
#define VODMA_END_REGISTER_0                                                    (0xb80057fc)
#define VODMA_REGISTER_NUM_0                                                    ((VODMA_END_REGISTER_0 - VODMA_START_REGISTER_0 + 4)/4)
#define VODMA_register_range_0(addr)                                            (((addr>=VODMA_START_REGISTER_0)&&(addr<=VODMA_END_REGISTER_0))?1:0)

#define VODMA_DDR_range_0(addr)                                                 (VODMA_0[(addr-VODMA_START_REGISTER_0)/4])

//hdr v_top
#define MAIN_TOP_CTL_START_REGISTER_0			(0xb806b800)
#define MAIN_TOP_CTL_END_REGISTER_0				(0xb806b858)
#define MAIN_TOP_CTL_REGISTER_NUM_0			((MAIN_TOP_CTL_END_REGISTER_0 - MAIN_TOP_CTL_START_REGISTER_0 + 4)/4)
#define MAIN_TOP_CTL_register_range_0(addr)		(((addr>=MAIN_TOP_CTL_START_REGISTER_0) &&(addr <=MAIN_TOP_CTL_END_REGISTER_0)) ? 1: 0 )
#define MAIN_TOP_CTL_DDR_range_0(addr)			(HDR_VTOP_0[(addr-MAIN_TOP_CTL_START_REGISTER_0)/4])

#define MAIN_TOP_CTL_START_REGISTER_1			(0xb806b940)
#define MAIN_TOP_CTL_END_REGISTER_1				(0xb806b950)
#define MAIN_TOP_CTL_REGISTER_NUM_1			((MAIN_TOP_CTL_END_REGISTER_1 - MAIN_TOP_CTL_START_REGISTER_1 + 4)/4)
#define MAIN_TOP_CTL_register_range_1(addr)		(((addr>=MAIN_TOP_CTL_START_REGISTER_1) &&(addr <=MAIN_TOP_CTL_END_REGISTER_1)) ? 1: 0 )
#define MAIN_TOP_CTL_DDR_range_1(addr)			(HDR_VTOP_1[(addr-MAIN_TOP_CTL_START_REGISTER_1)/4])


