#include "CppUTest/TestHarness.h"
#include "fff.h"

extern "C"
{
    // FIXME : add necessary include over here
    #include <dprx_vfe_api.h>
    #include <dprx_platform-rtk_tv.h>
    #include <dprx_pcb_config_fff.h>
}



#define NORMAL_MDP_PORT(phy, max_lane)      (DP_PORT_VALID(1) | DP_PORT_TYPE_mDP | \
                                             DP_EDID_TYPE_INT_DDC | DP_DDC_CH_SEL_DDC3 | \
                                             DP_AUX_CH_SEL_AUX0 | DP_AUX_PN_SWAP_OFF | \
                                             phy | max_lane | \
                                             DP_LANE0_PHY_MAPPING_0 | DP_LANE0_PN_SWAP_OFF | \
                                             DP_LANE1_PHY_MAPPING_1 | DP_LANE1_PN_SWAP_OFF | \
                                             DP_LANE2_PHY_MAPPING_2 | DP_LANE2_PN_SWAP_OFF | \
                                             DP_LANE3_PHY_MAPPING_3 | DP_LANE3_PN_SWAP_OFF | \
                                             DP_DET_GPIO_TYPE_ISO   | DP_DET_GPIO_ID(0x2)  | \
                                             DP_AUX_P_GPIO_TYPE_ISO | DP_AUX_P_GPIO_ID(0x3)| \
                                             DP_AUX_N_GPIO_TYPE_ISO | DP_AUX_N_GPIO_ID(0x4)| \
                                             DP_HPD_GPIO_TYPE_ISO   | DP_HPD_GPIO_ID(0x5) )

#define TYPEC_DP_PORT(phy, max_lane, pd_idx)(DP_PORT_VALID(1) | DP_PORT_TYPE_TYPE_C | \
                                             DP_EDID_TYPE_INT_DDC | DP_DDC_CH_SEL_DDC3 | \
                                             DP_AUX_CH_SEL_AUX0 | DP_AUX_PN_SWAP_OFF | \
                                             phy | max_lane | \
                                             DP_LANE0_PHY_MAPPING_0 | DP_LANE0_PN_SWAP_OFF | \
                                             DP_LANE1_PHY_MAPPING_1 | DP_LANE1_PN_SWAP_OFF | \
                                             DP_LANE2_PHY_MAPPING_2 | DP_LANE2_PN_SWAP_OFF | \
                                             DP_LANE3_PHY_MAPPING_3 | DP_LANE3_PN_SWAP_OFF | \
                                             DP_PORT_TYPEC_DP_SEL(pd_idx))

PCB_ENUM_T my_fake_pcb_param[3] =
{
    {0, "DP_PORT_ORDER"},
    {0, "DP_PORT1_CFG"},
    {0, "DP_PORT2_CFG"},
};

void init_pcb_parameter(PCB_PARAM_IDX mode)
{

    unsigned long long port_order = 0;
    unsigned long long port1_cfg  = 0;
    unsigned long long port2_cfg  = 0;

    switch(mode)
    {
    case PCB_PARAM_DP1_MDP_1_LANE_DP2_NONE:
        port_order = DP1_CH_SEL(1);
        port1_cfg  = NORMAL_MDP_PORT(DP_PHY_ID_PHY0, DP_MAX_LANE_CNT_1);
        break;

    case PCB_PARAM_DP1_MDP_1_LANE_DP2_MDP_1_LANE:
        port_order = DP1_CH_SEL(1) | DP2_CH_SEL(2);
        port1_cfg  = NORMAL_MDP_PORT(DP_PHY_ID_PHY0, DP_MAX_LANE_CNT_1);
        port2_cfg  = NORMAL_MDP_PORT(DP_PHY_ID_PHY1, DP_MAX_LANE_CNT_1);
        break;

    case PCB_PARAM_DP1_MDP_1_LANE_DP2_MDP_2_LANE:
        port_order = DP1_CH_SEL(1) | DP2_CH_SEL(2);
        port1_cfg  = NORMAL_MDP_PORT(DP_PHY_ID_PHY0, DP_MAX_LANE_CNT_1);
        port2_cfg  = NORMAL_MDP_PORT(DP_PHY_ID_PHY1, DP_MAX_LANE_CNT_2);
        break;

    case PCB_PARAM_DP1_MDP_1_LANE_DP2_MDP_4_LANE:
        port_order = DP1_CH_SEL(1) | DP2_CH_SEL(2);
        port1_cfg  = NORMAL_MDP_PORT(DP_PHY_ID_PHY0, DP_MAX_LANE_CNT_1);
        port2_cfg  = NORMAL_MDP_PORT(DP_PHY_ID_PHY1, DP_MAX_LANE_CNT_4);
        break;

    case PCB_PARAM_DP1_MDP_2_LANE_DP2_NONE:
        port_order = DP1_CH_SEL(1);
        port1_cfg  = NORMAL_MDP_PORT(DP_PHY_ID_PHY0, DP_MAX_LANE_CNT_2);
        break;

    case PCB_PARAM_DP1_MDP_2_LANE_DP2_MDP_1_LANE:
        port_order = DP1_CH_SEL(1) | DP2_CH_SEL(2);
        port1_cfg  = NORMAL_MDP_PORT(DP_PHY_ID_PHY0, DP_MAX_LANE_CNT_2);
        port2_cfg  = NORMAL_MDP_PORT(DP_PHY_ID_PHY1, DP_MAX_LANE_CNT_1);
        break;

    case PCB_PARAM_DP1_MDP_2_LANE_DP2_MDP_2_LANE:
        port_order = DP1_CH_SEL(1) | DP2_CH_SEL(2);
        port1_cfg  = NORMAL_MDP_PORT(DP_PHY_ID_PHY0, DP_MAX_LANE_CNT_2);
        port2_cfg  = NORMAL_MDP_PORT(DP_PHY_ID_PHY1, DP_MAX_LANE_CNT_2);
        break;

    case PCB_PARAM_DP1_MDP_2_LANE_DP2_MDP_4_LANE:
        port_order = DP1_CH_SEL(1) | DP2_CH_SEL(2);
        port1_cfg  = NORMAL_MDP_PORT(DP_PHY_ID_PHY0, DP_MAX_LANE_CNT_2);
        port2_cfg  = NORMAL_MDP_PORT(DP_PHY_ID_PHY1, DP_MAX_LANE_CNT_4);
        break;

    case PCB_PARAM_DP1_MDP_4_LANE_DP2_NONE:
        port_order = DP1_CH_SEL(1);
        port1_cfg  = NORMAL_MDP_PORT(DP_PHY_ID_PHY0, DP_MAX_LANE_CNT_4);
        break;

    case PCB_PARAM_DP1_MDP_4_LANE_DP2_MDP_1_LANE:
        port_order = DP1_CH_SEL(1) | DP2_CH_SEL(2);
        port1_cfg  = NORMAL_MDP_PORT(DP_PHY_ID_PHY0, DP_MAX_LANE_CNT_4);
        port2_cfg  = NORMAL_MDP_PORT(DP_PHY_ID_PHY1, DP_MAX_LANE_CNT_1);
        break;

    case PCB_PARAM_DP1_MDP_4_LANE_DP2_MDP_2_LANE:
        port_order = DP1_CH_SEL(1) | DP2_CH_SEL(2);
        port1_cfg  = NORMAL_MDP_PORT(DP_PHY_ID_PHY0, DP_MAX_LANE_CNT_4);
        port2_cfg  = NORMAL_MDP_PORT(DP_PHY_ID_PHY1, DP_MAX_LANE_CNT_2);
        break;

    case PCB_PARAM_DP1_MDP_4_LANE_DP2_MDP_4_LANE:
        port_order = DP1_CH_SEL(1) | DP2_CH_SEL(2);
        port1_cfg  = NORMAL_MDP_PORT(DP_PHY_ID_PHY0, DP_MAX_LANE_CNT_4);
        port2_cfg  = NORMAL_MDP_PORT(DP_PHY_ID_PHY1, DP_MAX_LANE_CNT_4);
        break;

    // config for RTD2851C
    case PCB_PARAM_DP1_TYPEC_4_LANE_DP2_NONE:

        // RTD2851C (TV)    HDMI1 + TypeC DP (P0) + HDMI2
        port_order = DP1_CH_SEL(1);
        port1_cfg  = DP_PORT_VALID(1);
        port1_cfg |= DP_PORT_TYPE_TYPE_C;
        port1_cfg |= DP_EDID_TYPE_INT_DDC | DP_DDC_CH_SEL_DDC3;
        port1_cfg |= DP_AUX_CH_SEL_AUX0 | DP_AUX_PN_SWAP_OFF;
        port1_cfg |= DP_PHY_ID(0);
        port1_cfg |= DP_MAX_LANE_CNT_4;
        port1_cfg |= DP_LANE0_PHY_MAPPING_0 | DP_LANE0_PN_SWAP_OFF;
        port1_cfg |= DP_LANE1_PHY_MAPPING_1 | DP_LANE1_PN_SWAP_OFF;
        port1_cfg |= DP_LANE2_PHY_MAPPING_2 | DP_LANE2_PN_SWAP_OFF;
        port1_cfg |= DP_LANE3_PHY_MAPPING_3 | DP_LANE3_PN_SWAP_OFF;
        port1_cfg |= DP_PORT_TYPEC_DP_SEL(0);
        break;

    case PCB_PARAM_DP1_TYPEC_4_LANE_DP2_MDP_4_LANE:

        // RTD2851C (DIAS)  HDMI1 + TypeC DP (P0) + DP (P1)
        port_order = DP1_CH_SEL(1) | DP2_CH_SEL(2);

        // DP1 : TYPEC
        port1_cfg  = DP_PORT_VALID(1);
        port1_cfg |= DP_PORT_TYPE_TYPE_C;
        port1_cfg |= DP_EDID_TYPE_INT_DDC | DP_DDC_CH_SEL_DDC3;
        port1_cfg |= DP_AUX_CH_SEL_AUX0 | DP_AUX_PN_SWAP_OFF;
        port1_cfg |= DP_PHY_ID(0);
        port1_cfg |= DP_MAX_LANE_CNT_4;
        port1_cfg |= DP_LANE0_PHY_MAPPING_0 | DP_LANE0_PN_SWAP_OFF;
        port1_cfg |= DP_LANE1_PHY_MAPPING_1 | DP_LANE1_PN_SWAP_OFF;
        port1_cfg |= DP_LANE2_PHY_MAPPING_2 | DP_LANE2_PN_SWAP_OFF;
        port1_cfg |= DP_LANE3_PHY_MAPPING_3 | DP_LANE3_PN_SWAP_OFF;
        port1_cfg |= DP_PORT_TYPEC_DP_SEL(0);

        // DP2 : MDP
        port2_cfg  = DP_PORT_VALID(1);
        port2_cfg |= DP_PORT_TYPE_mDP;
        port2_cfg |= DP_EDID_TYPE_INT_DDC | DP_DDC_CH_SEL_DDC3;
        port2_cfg |= DP_AUX_CH_SEL_AUX0 | DP_AUX_PN_SWAP_OFF;
        port2_cfg |= DP_PHY_ID(1);
        port2_cfg |= DP_MAX_LANE_CNT_4;
        port2_cfg |= DP_LANE0_PHY_MAPPING_0 | DP_LANE0_PN_SWAP_OFF;
        port2_cfg |= DP_LANE1_PHY_MAPPING_1 | DP_LANE1_PN_SWAP_OFF;
        port2_cfg |= DP_LANE2_PHY_MAPPING_2 | DP_LANE2_PN_SWAP_OFF;
        port2_cfg |= DP_LANE3_PHY_MAPPING_3 | DP_LANE3_PN_SWAP_OFF;
        port2_cfg |= DP_DET_GPIO_TYPE_ISO   | DP_DET_GPIO_ID(0x5);    // ST GPIO 5 (IN)
        port2_cfg |= DP_AUX_P_GPIO_TYPE_ISO | DP_AUX_P_GPIO_ID(0x7);  // ST GPIO 7 (IN)
        port2_cfg |= DP_AUX_N_GPIO_TYPE_ISO | DP_AUX_N_GPIO_ID(0x8);  // ST GPIO 8 (IN)
        port2_cfg |= DP_HPD_GPIO_TYPE_ISO   | DP_HPD_GPIO_ID(0x6);    // ST GPIO 6 (OUT)
        break;

    default:
    case PCB_PARAM_DP1_NONE:
        break;
    }

    my_fake_pcb_param[0].info = port_order;
    my_fake_pcb_param[1].info = port1_cfg;
    my_fake_pcb_param[2].info = port2_cfg;

    pcb_mgr_set_pcb_enum_list(my_fake_pcb_param, sizeof(my_fake_pcb_param)/sizeof(PCB_ENUM_T));
}


