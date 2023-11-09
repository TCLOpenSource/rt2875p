#define IEDGE_SMOOTH_START_REGISTER_0                                           (0xb8025100)
#define IEDGE_SMOOTH_END_REGISTER_0                                             (0xb80251ec)
#define IEDGE_SMOOTH_REGISTER_NUM_0                                             ((IEDGE_SMOOTH_END_REGISTER_0 - IEDGE_SMOOTH_START_REGISTER_0 + 4)/4)
#define IEDGE_SMOOTH_register_range_0(addr)                                     (((addr>=IEDGE_SMOOTH_START_REGISTER_0)&&(addr<=IEDGE_SMOOTH_END_REGISTER_0))?1:0)

#define IEDGE_SMOOTH_DDR_range_0(addr)                                          (iedge_smooth_0[(addr-IEDGE_SMOOTH_START_REGISTER_0)/4])

