#define RGB2YUV_START_REGISTER_0                                                (0xb8023000)
#define RGB2YUV_END_REGISTER_0                                                  (0xb8023000)
#define RGB2YUV_REGISTER_NUM_0                                                  ((RGB2YUV_END_REGISTER_0 - RGB2YUV_START_REGISTER_0 + 4)/4)
#define RGB2YUV_register_range_0(addr)                                          (((addr>=RGB2YUV_START_REGISTER_0)&&(addr<=RGB2YUV_END_REGISTER_0))?1:0)

#define RGB2YUV_DDR_range_0(addr)                                               (rgb2yuv_0[(addr-RGB2YUV_START_REGISTER_0)/4])

#define RGB2YUV_START_REGISTER_1                                                (0xb8023008)
#define RGB2YUV_END_REGISTER_1                                                  (0xb8023038)
#define RGB2YUV_REGISTER_NUM_1                                                  ((RGB2YUV_END_REGISTER_1 - RGB2YUV_START_REGISTER_1 + 4)/4)
#define RGB2YUV_register_range_1(addr)                                          (((addr>=RGB2YUV_START_REGISTER_1)&&(addr<=RGB2YUV_END_REGISTER_1))?1:0)

#define RGB2YUV_DDR_range_1(addr)                                               (rgb2yuv_1[(addr-RGB2YUV_START_REGISTER_1)/4])

