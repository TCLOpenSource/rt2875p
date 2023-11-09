#define IPQ_DECONTOUR_START_REGISTER_0                                          (0xb8025700)
#define IPQ_DECONTOUR_END_REGISTER_0                                            (0xb8025710)
#define IPQ_DECONTOUR_REGISTER_NUM_0                                            ((IPQ_DECONTOUR_END_REGISTER_0 - IPQ_DECONTOUR_START_REGISTER_0 + 4)/4)
#define IPQ_DECONTOUR_register_range_0(addr)                                    (((addr>=IPQ_DECONTOUR_START_REGISTER_0)&&(addr<=IPQ_DECONTOUR_END_REGISTER_0))?1:0)

#define IPQ_DECONTOUR_DDR_range_0(addr)                                         (ipq_decontour_0[(addr-IPQ_DECONTOUR_START_REGISTER_0)/4])

