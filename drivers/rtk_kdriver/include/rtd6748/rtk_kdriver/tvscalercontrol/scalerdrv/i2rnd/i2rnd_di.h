#define DI_START_REGISTER_0                                                     (0xb8024000)
#define DI_END_REGISTER_0                                                       (0xb80240c0)
#define DI_REGISTER_NUM_0                                                       ((DI_END_REGISTER_0 - DI_START_REGISTER_0 + 4)/4)
#define DI_register_range_0(addr)                                               (((addr>=DI_START_REGISTER_0)&&(addr<=DI_END_REGISTER_0))?1:0)

#define DI_DDR_range_0(addr)                                                    (di_0[(addr-DI_START_REGISTER_0)/4])

#define DI_START_REGISTER_1                                                     (0xb802419c)
#define DI_END_REGISTER_1                                                       (0xb8024724)
#define DI_REGISTER_NUM_1                                                       ((DI_END_REGISTER_1 - DI_START_REGISTER_1 + 4)/4)
#define DI_register_range_1(addr)                                               (((addr>=DI_START_REGISTER_1)&&(addr<=DI_END_REGISTER_1))?1:0)

#define DI_DDR_range_1(addr)                                                    (di_1[(addr-DI_START_REGISTER_1)/4])

#define DI_START_REGISTER_2                                                     (0xb8024800)
#define DI_END_REGISTER_2                                                       (0xb80249ac)
#define DI_REGISTER_NUM_2                                                       ((DI_END_REGISTER_2 - DI_START_REGISTER_2 + 4)/4)
#define DI_register_range_2(addr)                                               (((addr>=DI_START_REGISTER_2)&&(addr<=DI_END_REGISTER_2))?1:0)

#define DI_DDR_range_2(addr)                                                    (di_2[(addr-DI_START_REGISTER_2)/4])

#define DI_START_REGISTER_3                                                     (0xb8024a10)
#define DI_END_REGISTER_3                                                       (0xb8024ac4)
#define DI_REGISTER_NUM_3                                                       ((DI_END_REGISTER_3 - DI_START_REGISTER_3 + 4)/4)
#define DI_register_range_3(addr)                                               (((addr>=DI_START_REGISTER_3)&&(addr<=DI_END_REGISTER_3))?1:0)

#define DI_DDR_range_3(addr)                                                    (di_3[(addr-DI_START_REGISTER_3)/4])

#define DI_START_REGISTER_4                                                     (0xb8024b04)
#define DI_END_REGISTER_4                                                       (0xb8024bd8)
#define DI_REGISTER_NUM_4                                                       ((DI_END_REGISTER_4 - DI_START_REGISTER_4 + 4)/4)
#define DI_register_range_4(addr)                                               (((addr>=DI_START_REGISTER_4)&&(addr<=DI_END_REGISTER_4))?1:0)

#define DI_DDR_range_4(addr)                                                    (di_4[(addr-DI_START_REGISTER_4)/4])

//DI PQC
#define DI_START_REGISTER_5                                                     (0xb8024c00)
#define DI_END_REGISTER_5                                                       (0xb8024c20)
#define DI_REGISTER_NUM_5                                                       ((DI_END_REGISTER_5 - DI_START_REGISTER_5 + 4)/4)
#define DI_register_range_5(addr)                                               (((addr>=DI_START_REGISTER_5)&&(addr<=DI_END_REGISTER_5))?1:0)
//num==,0->not I2run reg,1->I2run reg,2->I2Run reg,data port reg,->Group3 register
#define DI_DDR_range_5(addr)                                                    (di_5[(addr-DI_START_REGISTER_5)/4])

//DI PQDC
#define DI_START_REGISTER_6                                                     (0xb8024c80)
#define DI_END_REGISTER_6                                                       (0xb8024c88)
#define DI_REGISTER_NUM_6                                                       ((DI_END_REGISTER_6 - DI_START_REGISTER_6 + 4)/4)
#define DI_register_range_6(addr)                                               (((addr>=DI_START_REGISTER_6)&&(addr<=DI_END_REGISTER_6))?1:0)
//num==,0->not I2run reg,1->I2run reg,2->I2Run reg,data port reg,->Group3 register
#define DI_DDR_range_6(addr)                                                    (di_6[(addr-DI_START_REGISTER_6)/4])

