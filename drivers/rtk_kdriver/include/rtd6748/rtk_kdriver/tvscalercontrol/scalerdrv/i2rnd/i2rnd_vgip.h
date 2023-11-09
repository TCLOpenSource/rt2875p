#define VGIP_START_REGISTER_0                                                   (0xb8022200)
#define VGIP_END_REGISTER_0                                                     (0xb8022280)
#define VGIP_REGISTER_NUM_0                                                     ((VGIP_END_REGISTER_0 - VGIP_START_REGISTER_0 + 4)/4)
#define VGIP_register_range_0(addr)                                             (((addr>=VGIP_START_REGISTER_0)&&(addr<=VGIP_END_REGISTER_0))?1:0)

#define VGIP_DDR_range_0(addr)                                                  (vgip_0[(addr-VGIP_START_REGISTER_0)/4])

