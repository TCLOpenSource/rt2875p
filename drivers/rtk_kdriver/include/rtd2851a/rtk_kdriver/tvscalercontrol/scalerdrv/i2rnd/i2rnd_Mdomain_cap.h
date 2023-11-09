#define MDOMAIN_CAP_START_REGISTER_0                                            (0xb8027200)
#define MDOMAIN_CAP_END_REGISTER_0                                              (0xb80272f4)
#define MDOMAIN_CAP_REGISTER_NUM_0                                              ((MDOMAIN_CAP_END_REGISTER_0 - MDOMAIN_CAP_START_REGISTER_0 + 4)/4)
#define MDOMAIN_CAP_register_range_0(addr)                                      (((addr>=MDOMAIN_CAP_START_REGISTER_0)&&(addr<=MDOMAIN_CAP_END_REGISTER_0))?1:0)

#define MDOMAIN_CAP_DDR_range_0(addr)                                           (Mdomain_cap_0[(addr-MDOMAIN_CAP_START_REGISTER_0)/4])

#define MDOMAIN_CAP_START_REGISTER_1                                            (0xb8027800)
#define MDOMAIN_CAP_END_REGISTER_1                                              (0xb802786c)
#define MDOMAIN_CAP_REGISTER_NUM_1                                              ((MDOMAIN_CAP_END_REGISTER_1 - MDOMAIN_CAP_START_REGISTER_1 + 4)/4)
#define MDOMAIN_CAP_register_range_1(addr)                                      (((addr>=MDOMAIN_CAP_START_REGISTER_1)&&(addr<=MDOMAIN_CAP_END_REGISTER_1))?1:0)

#define MDOMAIN_CAP_DDR_range_1(addr)                                           (Mdomain_cap_1[(addr-MDOMAIN_CAP_START_REGISTER_1)/4])

#define MDOMAIN_CAP_START_REGISTER_2                                            (0xb8027700)
#define MDOMAIN_CAP_END_REGISTER_2                                              (0xb8027710)
#define MDOMAIN_CAP_REGISTER_NUM_2                                              ((MDOMAIN_CAP_END_REGISTER_2 - MDOMAIN_CAP_START_REGISTER_2 + 4)/4)
#define MDOMAIN_CAP_register_range_2(addr)                                      (((addr>=MDOMAIN_CAP_START_REGISTER_2)&&(addr<=MDOMAIN_CAP_END_REGISTER_2))?1:0)

#define MDOMAIN_CAP_DDR_range_2(addr)                                           (Mdomain_cap_2[(addr-MDOMAIN_CAP_START_REGISTER_2)/4])

