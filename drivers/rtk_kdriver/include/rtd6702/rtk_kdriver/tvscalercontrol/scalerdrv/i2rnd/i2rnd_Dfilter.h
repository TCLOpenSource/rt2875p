#define DFILTER_START_REGISTER_0                                                (0xb8023500)
#define DFILTER_END_REGISTER_0                                                  (0xb8023504)
#define DFILTER_REGISTER_NUM_0                                                  ((DFILTER_END_REGISTER_0 - DFILTER_START_REGISTER_0 + 4)/4)
#define DFILTER_register_range_0(addr)                                          (((addr>=DFILTER_START_REGISTER_0)&&(addr<=DFILTER_END_REGISTER_0))?1:0)

#define DFILTER_DDR_range_0(addr)                                               (Dfilter_0[(addr-DFILTER_START_REGISTER_0)/4])

