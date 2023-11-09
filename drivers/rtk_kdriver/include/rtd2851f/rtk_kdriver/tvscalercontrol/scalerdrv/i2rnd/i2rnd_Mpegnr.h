#define MPEGNR_START_REGISTER_0                                                 (0xb8023820)
#define MPEGNR_END_REGISTER_0                                                   (0xb80238d0)
#define MPEGNR_REGISTER_NUM_0                                                   ((MPEGNR_END_REGISTER_0 - MPEGNR_START_REGISTER_0 + 4)/4)
#define MPEGNR_register_range_0(addr)                                           (((addr>=MPEGNR_START_REGISTER_0)&&(addr<=MPEGNR_END_REGISTER_0))?1:0)

#define MPEGNR_DDR_range_0(addr)                                                (Mpegnr_0[(addr-MPEGNR_START_REGISTER_0)/4])

