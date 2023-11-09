#define VODMA2_START_REGISTER_0                                                 (0xb8005800)
#define VODMA2_END_REGISTER_0                                                   (0xb8005ffc)
#define VODMA2_REGISTER_NUM_0                                                   ((VODMA2_END_REGISTER_0 - VODMA2_START_REGISTER_0 + 4)/4)
#define VODMA2_register_range_0(addr)                                           (((addr>=VODMA2_START_REGISTER_0)&&(addr<=VODMA2_END_REGISTER_0))?1:0)

#define VODMA2_DDR_range_0(addr)                                                (VODMA2_0[(addr-VODMA2_START_REGISTER_0)/4])

