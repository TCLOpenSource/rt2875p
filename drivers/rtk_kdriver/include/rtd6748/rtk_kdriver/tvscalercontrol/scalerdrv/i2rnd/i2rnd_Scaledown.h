#define SCALEDOWN_START_REGISTER_0                                              (0xb8025204)
#define SCALEDOWN_END_REGISTER_0                                                (0xb80252f4)
#define SCALEDOWN_REGISTER_NUM_0                                                ((SCALEDOWN_END_REGISTER_0 - SCALEDOWN_START_REGISTER_0 + 4)/4)
#define SCALEDOWN_register_range_0(addr)                                        (((addr>=SCALEDOWN_START_REGISTER_0)&&(addr<=SCALEDOWN_END_REGISTER_0))?1:0)

#define SCALEDOWN_DDR_range_0(addr)                                             (Scaledown_0[(addr-SCALEDOWN_START_REGISTER_0)/4])

#define SCALEDOWN_START_REGISTER_1                                              (0xb8025300)
#define SCALEDOWN_END_REGISTER_1                                                (0xb802537c)
#define SCALEDOWN_REGISTER_NUM_1                                                ((SCALEDOWN_END_REGISTER_1 - SCALEDOWN_START_REGISTER_1 + 4)/4)
#define SCALEDOWN_register_range_1(addr)                                        (((addr>=SCALEDOWN_START_REGISTER_1)&&(addr<=SCALEDOWN_END_REGISTER_1))?1:0)

#define SCALEDOWN_DDR_range_1(addr)                                             (Scaledown_1[(addr-SCALEDOWN_START_REGISTER_1)/4])

