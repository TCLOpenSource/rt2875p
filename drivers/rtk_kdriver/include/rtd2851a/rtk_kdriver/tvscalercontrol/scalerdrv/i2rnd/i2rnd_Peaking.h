#define PEAKING_START_REGISTER_0                                                (0xb8023170)
#define PEAKING_END_REGISTER_0                                                  (0xb8023180)
#define PEAKING_REGISTER_NUM_0                                                  0//((PEAKING_END_REGISTER_0 - PEAKING_START_REGISTER_0 + 4)/4)
#define PEAKING_register_range_0(addr)                                          0//(((addr>=PEAKING_START_REGISTER_0)&&(addr<=PEAKING_END_REGISTER_0))?1:0)

#define PEAKING_DDR_range_0(addr)                                               0//(Peaking_0[(addr-PEAKING_START_REGISTER_0)/4])

