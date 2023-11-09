#define PROFILE_START_REGISTER_0                                                (0xb8023400)
#define PROFILE_END_REGISTER_0                                                  (0xb80234c4)
#define PROFILE_REGISTER_NUM_0                                                  ((PROFILE_END_REGISTER_0 - PROFILE_START_REGISTER_0 + 4)/4)
#define PROFILE_register_range_0(addr)                                          (((addr>=PROFILE_START_REGISTER_0)&&(addr<=PROFILE_END_REGISTER_0))?1:0)

#define PROFILE_DDR_range_0(addr)                                               (Profile_0[(addr-PROFILE_START_REGISTER_0)/4])

#define PROFILE_START_REGISTER_1                                                (0xb8023700)
#define PROFILE_END_REGISTER_1                                                  (0xb80237d4)
#define PROFILE_REGISTER_NUM_1                                                  ((PROFILE_END_REGISTER_1 - PROFILE_START_REGISTER_1 + 4)/4)
#define PROFILE_register_range_1(addr)                                          (((addr>=PROFILE_START_REGISTER_1)&&(addr<=PROFILE_END_REGISTER_1))?1:0)

#define PROFILE_DDR_range_1(addr)                                               (Profile_1[(addr-PROFILE_START_REGISTER_1)/4])

