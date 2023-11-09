#define MAIN_DITHER_START_REGISTER_0                                            (0xb8022600)
#define MAIN_DITHER_END_REGISTER_0                                              (0xb8022650)
#define MAIN_DITHER_REGISTER_NUM_0                                              ((MAIN_DITHER_END_REGISTER_0 - MAIN_DITHER_START_REGISTER_0 + 4)/4)
#define MAIN_DITHER_register_range_0(addr)                                      (((addr>=MAIN_DITHER_START_REGISTER_0)&&(addr<=MAIN_DITHER_END_REGISTER_0))?1:0)

#define MAIN_DITHER_DDR_range_0(addr)                                           (Main_Dither_0[(addr-MAIN_DITHER_START_REGISTER_0)/4])

