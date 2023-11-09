#define COLOR_MB_PEAKING_START_REGISTER_0                                       (0xb8023c00)
#define COLOR_MB_PEAKING_END_REGISTER_0                                         (0xb8023c40)
#define COLOR_MB_PEAKING_REGISTER_NUM_0                                         ((COLOR_MB_PEAKING_END_REGISTER_0 - COLOR_MB_PEAKING_START_REGISTER_0 + 4)/4)
#define COLOR_MB_PEAKING_register_range_0(addr)                                 (((addr>=COLOR_MB_PEAKING_START_REGISTER_0)&&(addr<=COLOR_MB_PEAKING_END_REGISTER_0))?1:0)

#define COLOR_MB_PEAKING_DDR_range_0(addr)                                      (color_mb_peaking_0[(addr-COLOR_MB_PEAKING_START_REGISTER_0)/4])

