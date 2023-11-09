#define HSD_DITHER_START_REGISTER_0                                             (0xb8023600)
#define HSD_DITHER_END_REGISTER_0                                               (0xb80236b8)
#define HSD_DITHER_REGISTER_NUM_0                                               ((HSD_DITHER_END_REGISTER_0 - HSD_DITHER_START_REGISTER_0 + 4)/4)
#define HSD_DITHER_register_range_0(addr)                                       (((addr>=HSD_DITHER_START_REGISTER_0)&&(addr<=HSD_DITHER_END_REGISTER_0))?1:0)

#define HSD_DITHER_DDR_range_0(addr)                                            (Hsd_dither_0[(addr-HSD_DITHER_START_REGISTER_0)/4])

