#define HISTOGRAM_START_REGISTER_0                                              (0xb8023200)
#define HISTOGRAM_END_REGISTER_0                                                (0xb80232a8)
#define HISTOGRAM_REGISTER_NUM_0                                                ((HISTOGRAM_END_REGISTER_0 - HISTOGRAM_START_REGISTER_0 + 4)/4)
#define HISTOGRAM_register_range_0(addr)                                        (((addr>=HISTOGRAM_START_REGISTER_0)&&(addr<=HISTOGRAM_END_REGISTER_0))?1:0)

#define HISTOGRAM_DDR_range_0(addr)                                             (Histogram_0[(addr-HISTOGRAM_START_REGISTER_0)/4])

