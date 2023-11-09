#include "CppUTest/TestHarness.h"
#include "fff.h"

extern "C"
{
    // FIXME : add necessary include over here
    #include <dprx_vfe_api.h>
    #include <dprx_platform-rtk_tv.h>
    #include <ScalerFunctionInclude.h>
    #include <ScalerDpRxInclude.h>
    #include <ScalerDpAuxRxInclude.h>
    #include <ScalerDpMacRxInclude.h>   // for fec ???
    #include <ScalerDpHdcpRxInclude.h>
    #include <ScalerDpPhyRxInclude.h>
    #include <virtual_platform.h>
    #include <virtual_main_link.h>
    #include <virtual_te.h>
}


TEST_GROUP(dprx_vfe)
{
    void setup()
    {
        virt_platform_init();
        virt_te_set_cable_connect(1);
    }
    void teardown()
    {
        virt_te_set_cable_connect(0);
        FFF_RESET;
    }
};

#define DP_DATA_PORT_ORDER_VAL    (DP1_CH_SEL(1) | DP2_CH_SEL(0))
#define DP_PORT1_CFG_VAL          (DP_PORT_VALID(1) | DP_PORT_TYPE_mDP | \
                                   DP_EDID_TYPE_INT_DDC | DP_DDC_CH_SEL_DDC0 | \
                                   DP_AUX_CH_SEL_AUX0 | DP_AUX_PN_SWAP_OFF | \
                                   DP_PHY_ID_PHY0 | DP_MAX_LANE_CNT_4 | \
                                   DP_LANE0_PHY_MAPPING_0 | DP_LANE0_PN_SWAP_OFF | \
                                   DP_LANE1_PHY_MAPPING_1 | DP_LANE1_PN_SWAP_OFF | \
                                   DP_LANE2_PHY_MAPPING_2 | DP_LANE2_PN_SWAP_OFF | \
                                   DP_LANE3_PHY_MAPPING_3 | DP_LANE3_PN_SWAP_OFF | \
                                   DP_AUX_P_GPIO_TYPE_ISO | DP_AUX_P_GPIO_ID(0x3)| \
                                   DP_AUX_N_GPIO_TYPE_ISO | DP_AUX_N_GPIO_ID(0x4)| \
                                   DP_HPD_GPIO_TYPE_ISO   | DP_HPD_GPIO_ID(0x5) )
#define DP_PORT2_CFG_VAL          (DP_PORT_VALID(0))


static PCB_ENUM_T my_fake_pcb_param[] =
{
    {DP_DATA_PORT_ORDER_VAL, "DP_PORT_ORDER"},
    {DP_PORT1_CFG_VAL,       "DP_PORT1_CFG"},
    {DP_PORT2_CFG_VAL,       "DP_PORT2_CFG"},
};


//--------------------------------------------------
// Func  : MT_TEST_DPRX_INIT_WITH_SINGLE_MDP
//
// Desc  : init DPRX vfe with single DP and do Link
//         Training test
//
// Pass  : Link Training Pass
//--------------------------------------------------
TEST(dprx_vfe, MT_TEST_DPRX_INIT_WITH_SINGLE_MDP)
{
    // Arrange
    UINT8 lane_count = 4;
    UINT8 link_rate = _DP_LINK_HBR3;
    INT8  error = 0;
    UINT8 port = 0;
    pcb_mgr_set_pcb_enum_list(my_fake_pcb_param, sizeof(my_fake_pcb_param)/sizeof(PCB_ENUM_T));

    // action
    printf("vfe_dprx_drv_init \n");

    vfe_dprx_drv_init();
    vfe_dprx_drv_set_hpd(port, 1);

    printf("start link training \n");

    if (virt_te_before_link_training(lane_count, link_rate)!=0  ||
        virt_te_clock_recovery(lane_count, link_rate) != 0 ||
        virt_te_channel_equalization(lane_count, link_rate) !=0)
    {
        //virt_te_link_training_stop(lane_count, link_rate);
        printf("step 11 : [TE] -> send idel pattern\n");
        fake_aux_tx_write_dpcd(DPCD_00102_TRAINING_PATTERN_SET, 0);
        virt_main_link_stop();
        error = -1;
    }

    virt_te_link_training_stop(lane_count, link_rate);

    vfe_dprx_drv_uninit();
    // Assert
    LONGS_EQUAL(0, error);
}

