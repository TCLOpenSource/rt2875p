#define NR_START_REGISTER_0                                                     (0xb8025000)
#define NR_END_REGISTER_0                                                       (0xb80250fc)
#define NR_REGISTER_NUM_0                                                       ((NR_END_REGISTER_0 - NR_START_REGISTER_0 + 4)/4)
#define NR_register_range_0(addr)                                               (((addr>=NR_START_REGISTER_0)&&(addr<=NR_END_REGISTER_0))?1:0)

#define NR_DDR_range_0(addr)                                                    (nr_0[(addr-NR_START_REGISTER_0)/4])

#define NR_START_REGISTER_1                                                     (0xb8025500)
#define NR_END_REGISTER_1                                                       (0xb80255fc)
#define NR_REGISTER_NUM_1                                                       ((NR_END_REGISTER_1 - NR_START_REGISTER_1 + 4)/4)
#define NR_register_range_1(addr)                                               (((addr>=NR_START_REGISTER_1)&&(addr<=NR_END_REGISTER_1))?1:0)

#define NR_DDR_range_1(addr)                                                    (nr_1[(addr-NR_START_REGISTER_1)/4])

