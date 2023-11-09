#define DE_XCXL_START_REGISTER_0                                                (0xb8023900)
#define DE_XCXL_END_REGISTER_0                                                  (0xb80239b8)
#define DE_XCXL_REGISTER_NUM_0                                                  ((DE_XCXL_END_REGISTER_0 - DE_XCXL_START_REGISTER_0 + 4)/4)
#define DE_XCXL_register_range_0(addr)                                          (((addr>=DE_XCXL_START_REGISTER_0)&&(addr<=DE_XCXL_END_REGISTER_0))?1:0)

#define DE_XCXL_DDR_range_0(addr)                                               (De_xcxl_0[(addr-DE_XCXL_START_REGISTER_0)/4])

