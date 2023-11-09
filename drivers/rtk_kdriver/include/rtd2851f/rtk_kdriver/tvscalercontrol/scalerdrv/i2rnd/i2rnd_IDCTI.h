#define IDCTI_START_REGISTER_0                                                  (0xb80231d4)
#define IDCTI_END_REGISTER_0                                                    (0xb80231ec)
#define IDCTI_REGISTER_NUM_0                                                    ((IDCTI_END_REGISTER_0 - IDCTI_START_REGISTER_0 + 4)/4)
#define IDCTI_register_range_0(addr)                                            (((addr>=IDCTI_START_REGISTER_0)&&(addr<=IDCTI_END_REGISTER_0))?1:0)

#define IDCTI_DDR_range_0(addr)                                                 (IDCTI_0[(addr-IDCTI_START_REGISTER_0)/4])

