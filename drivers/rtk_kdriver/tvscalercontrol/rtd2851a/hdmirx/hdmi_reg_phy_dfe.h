#include <rbus/dfe_p0_reg.h>
#ifdef UT_flag
typedef unsigned int uint32_t;
#endif
#ifdef BUILD_QUICK_SHOW
#include <sysdefs.h>
#include <sed_types.h>
#endif

#define HDMI_DFE0_BASE           (0xb800DE00)
#define HDMI_DFE1_BASE           (0xb800DE50)
#define HDMI_DFE2_BASE           (0xb800DEA0)

struct hdmi_dfe_reg_st
{
    uint32_t en_l0;//DE00
    uint32_t init0_l0;//DE04
    uint32_t init1_l0;//DE08
    uint32_t dummy1;//DE0C
    uint32_t en_l1;//DE10
    uint32_t init0_l1;//DE14
    uint32_t init1_l1;//DE18
    uint32_t dummy2;//DE1C
    uint32_t en_l2;//DE20
    uint32_t init0_l2;//DE24
    uint32_t init1_l2;//DE28
    uint32_t dummy3;//DE2C
    uint32_t mode;//DE30
    uint32_t gain;//DE34
    uint32_t limit0;//DE38
    uint32_t limit1;//DE3C
    uint32_t limit2;//DE40
    uint32_t readback;//DE44
    uint32_t flag;//DE48
    uint32_t debug;//DE4C
};

extern const struct hdmi_dfe_reg_st *dfe[3];


#ifdef CONFIG_ARM64
#define DFE_REG_DFE_EN_L0_reg                  ((unsigned int)(unsigned long)(&dfe[nport]->en_l0))          // 0xB800DE00 DE50 DEA0
#define DFE_REG_DFE_INIT0_L0_reg               ((unsigned int)(unsigned long)(&dfe[nport]->init0_l0))       // 0xB800DE04 DE54 DEA4
#define DFE_REG_DFE_INIT1_L0_reg               ((unsigned int)(unsigned long)(&dfe[nport]->init1_l0))       // 0xB800DE08 DE58 DEA8
#define DFE_REG_DFE_EN_L1_reg                  ((unsigned int)(unsigned long)(&dfe[nport]->en_l1))          // 0xB800DE10 DE60 DEB0
#define DFE_REG_DFE_INIT0_L1_reg               ((unsigned int)(unsigned long)(&dfe[nport]->init0_l1))       // 0xB800DE14 DE64 DEA4
#define DFE_REG_DFE_INIT1_L1_reg               ((unsigned int)(unsigned long)(&dfe[nport]->init1_l1))       // 0xB800DE18 DE68 DEA8
#define DFE_REG_DFE_EN_L2_reg                  ((unsigned int)(unsigned long)(&dfe[nport]->en_l2 ))         // 0xB800DE20 DE70 DEB0
#define DFE_REG_DFE_INIT0_L2_reg               ((unsigned int)(unsigned long)(&dfe[nport]->init0_l2))       // 0xB800DE24 DE74 DEC4
#define DFE_REG_DFE_INIT1_L2_reg               ((unsigned int)(unsigned long)(&dfe[nport]->init1_l2))       // 0xB800DE28 DE78 DEC8

#define DFE_REG_DFE_MODE_reg                   ((unsigned int)(unsigned long)(&dfe[nport]->mode))           // 0xB800DE30 DE80 DED0
#define DFE_REG_DFE_GAIN_reg                   ((unsigned int)(unsigned long)(&dfe[nport]->gain))           // 0xB800DE34 DE84 DED4
#define DFE_REG_DFE_LIMIT0_reg                 ((unsigned int)(unsigned long)(&dfe[nport]->limit0))         // 0xB800DE38 DE88 DED8
#define DFE_REG_DFE_LIMIT1_reg                 ((unsigned int)(unsigned long)(&dfe[nport]->limit1))         // 0xB800DE3C DE8C DEDC
#define DFE_REG_DFE_LIMIT2_reg                 ((unsigned int)(unsigned long)(&dfe[nport]->limit2))         // 0xB800DE40 DE90 DEE0
#define DFE_REG_DFE_READBACK_reg               ((unsigned int)(unsigned long)(&dfe[nport]->readback))       // 0xB800DE44 DE94 DEE4
#define DFE_REG_DFE_FLAG_reg                   ((unsigned int)(unsigned long)(&dfe[nport]->flag))           // 0xB800DE48 DE98 DEE8
#define DFE_REG_DFE_DEBUG_reg                  ((unsigned int)(unsigned long)(&dfe[nport]->debug))          // 0xB800DE4C DE9C DEEC

#else
#define DFE_REG_DFE_EN_L0_reg                  ((unsigned int)(&dfe[nport]->en_l0))             //  0xB800DE00 DE50 DEA0
#define DFE_REG_DFE_INIT0_L0_reg               ((unsigned int)(&dfe[nport]->init0_l0))          // 0xB800DE04 DE54 DEA4
#define DFE_REG_DFE_INIT1_L0_reg               ((unsigned int)(&dfe[nport]->init1_l0))          // 0xB800DE08 DE58 DEA8
#define DFE_REG_DFE_EN_L1_reg                  ((unsigned int)(&dfe[nport]->en_l1))             //  0xB800DE10 DE60 DEB0
#define DFE_REG_DFE_INIT0_L1_reg               ((unsigned int)(&dfe[nport]->init0_l1))          //  0xB800DE14 DE64 DEA4
#define DFE_REG_DFE_INIT1_L1_reg               ((unsigned int)(&dfe[nport]->init1_l1))          // 0xB800DE18 DE68 DEA8
#define DFE_REG_DFE_EN_L2_reg                  ((unsigned int)(&dfe[nport]->en_l2 ))            // 0xB800DE20 DE70 DEB0
#define DFE_REG_DFE_INIT0_L2_reg               ((unsigned int)(&dfe[nport]->init0_l2))          // 0 0xB800DE24 DE74 DEC4
#define DFE_REG_DFE_INIT1_L2_reg               ((unsigned int)(&dfe[nport]->init1_l2))          //  0xB800DE24 DE74 DEC4

#define DFE_REG_DFE_MODE_reg                   ((unsigned int)(&dfe[nport]->mode))              // 0xB800DE30 DE80 DEC0
#define DFE_REG_DFE_GAIN_reg                   ((unsigned int)(&dfe[nport]->gain))              // 0xB800DE34 DE84 DEC4
#define DFE_REG_DFE_LIMIT0_reg                 ((unsigned int)(&dfe[nport]->limit0))            // 0xB800DE38 DE88 DEC8
#define DFE_REG_DFE_LIMIT1_reg                 ((unsigned int)(&dfe[nport]->limit1))            // 0xB800DE3C DE8C DEDC
#define DFE_REG_DFE_LIMIT2_reg                 ((unsigned int)(&dfe[nport]->limit2))            // 0xB800DE40 DE90 DEE0
#define DFE_REG_DFE_READBACK_reg               ((unsigned int)(&dfe[nport]->readback))          // 0xB800DE44 DE94 DEE4
#define DFE_REG_DFE_FLAG_reg                   ((unsigned int)(&dfe[nport]->flag))              // 0xB800DE48 DE98 DEE8
#define DFE_REG_DFE_DEBUG_reg                  ((unsigned int)(&dfe[nport]->debug))             // 0xB800DE4C DE9C DEEC
#endif




#define LOAD_IN_INIT_LE     (_BIT31)
#define LOAD_IN_INIT_VTH    (_BIT30)
#define LOAD_IN_INIT_SERVO  (_BIT29)
#define LOAD_IN_INIT_TAP4   (_BIT28)
#define LOAD_IN_INIT_TAP3   (_BIT27)
#define LOAD_IN_INIT_TAP2   (_BIT26)
#define LOAD_IN_INIT_TAP1   (_BIT25)
#define LOAD_IN_INIT_TAP0   (_BIT24)
#define LOAD_IN_INIT_ALL    (LOAD_IN_INIT_LE|LOAD_IN_INIT_VTH|LOAD_IN_INIT_SERVO|LOAD_IN_INIT_TAP4|LOAD_IN_INIT_TAP3|LOAD_IN_INIT_TAP2|LOAD_IN_INIT_TAP1|LOAD_IN_INIT_TAP0)


#define REG_dfe_adapt_en_lane0_TAP0                                        _BIT16
#define REG_dfe_adapt_en_lane0_TAP1                                        _BIT17
#define REG_dfe_adapt_en_lane0_TAP2                                        _BIT18
#define REG_dfe_adapt_en_lane0_TAP3                                        _BIT19
#define REG_dfe_adapt_en_lane0_TAP4                                        _BIT20
#define REG_dfe_adapt_en_lane0_SERVO                                       _BIT21
#define REG_dfe_adapt_en_lane0_Vth                                         _BIT22
#define REG_dfe_adapt_en_lane0_LE                                          _BIT23
#define REG_dfe_adapt_en_lane1_TAP0                                        _BIT16
#define REG_dfe_adapt_en_lane1_TAP1                                        _BIT17
#define REG_dfe_adapt_en_lane1_TAP2                                        _BIT18
#define REG_dfe_adapt_en_lane1_TAP3                                        _BIT19
#define REG_dfe_adapt_en_lane1_TAP4                                        _BIT20
#define REG_dfe_adapt_en_lane1_SERVO                                       _BIT21
#define REG_dfe_adapt_en_lane1_Vth                                         _BIT22
#define REG_dfe_adapt_en_lane1_LE                                          _BIT23
#define REG_dfe_adapt_en_lane2_TAP0                                        _BIT16
#define REG_dfe_adapt_en_lane2_TAP1                                        _BIT17
#define REG_dfe_adapt_en_lane2_TAP2                                        _BIT18
#define REG_dfe_adapt_en_lane2_TAP3                                        _BIT19
#define REG_dfe_adapt_en_lane2_TAP4                                        _BIT20
#define REG_dfe_adapt_en_lane2_SERVO                                       _BIT21
#define REG_dfe_adapt_en_lane2_Vth                                         _BIT22
#define REG_dfe_adapt_en_lane2_LE                                          _BIT23
#define REG_dfe_adapt_en_lane3_TAP0                                        _BIT16
#define REG_dfe_adapt_en_lane3_TAP1                                        _BIT17
#define REG_dfe_adapt_en_lane3_TAP2                                        _BIT18
#define REG_dfe_adapt_en_lane3_TAP3                                        _BIT19
#define REG_dfe_adapt_en_lane3_TAP4                                        _BIT20
#define REG_dfe_adapt_en_lane3_SERVO                                       _BIT21
#define REG_dfe_adapt_en_lane3_Vth                                         _BIT22
#define REG_dfe_adapt_en_lane3_LE                                          _BIT23


#define LIMIT_TAP0_MAX          0
#define LIMIT_TAP0_MIN          1
#define LIMIT_TAP1_MAX          2
#define LIMIT_TAP1_MIN          3
#define LIMIT_LE_MAX            4
#define LIMIT_LE_MIN            5

#define COEF_VTH                0
#define COEF_TAP0               1
#define COEF_TAP1               2
#define COEF_TAP2               3
#define COEF_TAP3               4
#define COEF_TAP4               5
#define COEF_SERVO              6
#define COEF_LE                 7


#define get_TAP2_coef_sign(data)                    (0x20&(data))
#define get_TAP2_coef(data)                         (0x1F&(data))
#define get_TAP3_coef_sign(data)                    (0x20&(data))
#define get_TAP3_coef(data)                         (0x1F&(data))
#define get_TAP4_coef_sign(data)                    (0x20&(data))
#define get_TAP4_coef(data)                         (0x1F&(data))

