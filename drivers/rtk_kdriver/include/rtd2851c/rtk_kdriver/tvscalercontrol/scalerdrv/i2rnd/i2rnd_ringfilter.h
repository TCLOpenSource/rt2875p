#define RINGFILTER_START_REGISTER_0                                             (0xb8023580)
#define RINGFILTER_END_REGISTER_0                                               (0xb8023590)
#define RINGFILTER_REGISTER_NUM_0                                               ((RINGFILTER_END_REGISTER_0 - RINGFILTER_START_REGISTER_0 + 4)/4)
#define RINGFILTER_register_range_0(addr)                                       (((addr>=RINGFILTER_START_REGISTER_0)&&(addr<=RINGFILTER_END_REGISTER_0))?1:0)

#define RINGFILTER_DDR_range_0(addr)                                            (ringfilter_0[(addr-RINGFILTER_START_REGISTER_0)/4])

