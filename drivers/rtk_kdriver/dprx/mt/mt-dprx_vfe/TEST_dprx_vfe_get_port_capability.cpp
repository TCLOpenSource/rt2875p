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
    #include <dprx_pcb_config_fff.h>
}


TEST_GROUP(dprx_vfe_get_port_capability)
{
    void setup()
    {
        virt_platform_init();
    }
    void teardown()
    {
        FFF_RESET;
    }
};


//--------------------------------------------------
// Func  : DDprxVfeGetPortCapability
//
// Desc  : Check dprx_drv_source_check with idel pattern
//
// Pass  :
//--------------------------------------------------
TEST(dprx_vfe_get_port_capability, DDprxVfeGetPortCapabilityMdp1Lane)
{
    // Arrange
    DP_PORT_CAPABILITY cap;
    INT8  ret = 0;
    UINT8 port = 0;
    init_pcb_parameter(PCB_PARAM_DP1_MDP_1_LANE);

    // action
    vfe_dprx_drv_init();

    ret = vfe_dprx_drv_get_rx_port_capability(port, &cap);

    vfe_dprx_drv_uninit();

    // Assert
    LONGS_EQUAL(DPRX_DRV_NO_ERR, ret);
    LONGS_EQUAL(DP_TYPE_mDP, cap.type);
    LONGS_EQUAL(1, cap.lane_cnt);
    LONGS_EQUAL(DP_LINK_RATE_RBR_MASK | DP_LINK_RATE_HBR1_MASK | DP_LINK_RATE_HBR2_MASK | DP_LINK_RATE_HBR3_MASK, cap.link_rate_mask);
    //LONGS_EQUAL(1, cap.capability.fec_support);
    LONGS_EQUAL(0, cap.capability.dsc_support);
}



//--------------------------------------------------
// Func  : DDprxVfeGetPortCapabilityMdp2Lane
//
// Desc  :
//
// Pass  :
//--------------------------------------------------
TEST(dprx_vfe_get_port_capability, DDprxVfeGetPortCapabilityMdp2Lane)
{
    // Arrange
    DP_PORT_CAPABILITY cap;
    INT8  ret = 0;
    UINT8 port = 0;
    init_pcb_parameter(PCB_PARAM_DP1_MDP_2_LANE);

    // action
    vfe_dprx_drv_init();

    ret = vfe_dprx_drv_get_rx_port_capability(port, &cap);

    vfe_dprx_drv_uninit();

    // Assert
    LONGS_EQUAL(DPRX_DRV_NO_ERR, ret);
    LONGS_EQUAL(DP_TYPE_mDP, cap.type);
    LONGS_EQUAL(2, cap.lane_cnt);
    LONGS_EQUAL(DP_LINK_RATE_RBR_MASK | DP_LINK_RATE_HBR1_MASK | DP_LINK_RATE_HBR2_MASK | DP_LINK_RATE_HBR3_MASK, cap.link_rate_mask);
    //LONGS_EQUAL(1, cap.capability.fec_support);
    LONGS_EQUAL(0, cap.capability.dsc_support);
}


//--------------------------------------------------
// Func  : DDprxVfeGetPortCapabilityMdp4Lane
//
// Desc  :
//
// Pass  :
//--------------------------------------------------
TEST(dprx_vfe_get_port_capability, DDprxVfeGetPortCapabilityMdp4Lane)
{
    // Arrange
    DP_PORT_CAPABILITY cap;
    INT8  ret = 0;
    UINT8 port = 0;
    init_pcb_parameter(PCB_PARAM_DP1_MDP_4_LANE);

    // action
    vfe_dprx_drv_init();

    ret = vfe_dprx_drv_get_rx_port_capability(port, &cap);

    vfe_dprx_drv_uninit();

    // Assert
    LONGS_EQUAL(DPRX_DRV_NO_ERR, ret);
    LONGS_EQUAL(DP_TYPE_mDP, cap.type);
    LONGS_EQUAL(4, cap.lane_cnt);
    LONGS_EQUAL(DP_LINK_RATE_RBR_MASK | DP_LINK_RATE_HBR1_MASK | DP_LINK_RATE_HBR2_MASK | DP_LINK_RATE_HBR3_MASK, cap.link_rate_mask);    
    //LONGS_EQUAL(1, cap.capability.fec_support);
    LONGS_EQUAL(0, cap.capability.dsc_support);
}


//--------------------------------------------------
// Func  : DPRX_GET_PORT_CAPABILITY_DP1_MDP__LANE_DP2_MDP_1_LANE
//
// Desc  : Check dprx_drv_source_check with idel pattern
//
// Pass  :
//--------------------------------------------------
TEST(dprx_vfe_get_port_capability, DPRX_GET_PORT_CAPABILITY_DP1_MDP_1_LANE_DP2_MDP_1_LANE)
{
    // Arrange
    DP_PORT_CAPABILITY cap[2];
    INT8  ret[2] = {0xF, 0xF};
    init_pcb_parameter(PCB_PARAM_DP1_MDP_1_LANE_DP2_MDP_1_LANE);

    // action
    vfe_dprx_drv_init();

    ret[0] = vfe_dprx_drv_get_rx_port_capability(0, &cap[0]);
    ret[1] = vfe_dprx_drv_get_rx_port_capability(1, &cap[1]);

    vfe_dprx_drv_uninit();

    // Assert
    LONGS_EQUAL(DPRX_DRV_NO_ERR, ret[0]);
    LONGS_EQUAL(DP_TYPE_mDP,     cap[0].type);
    LONGS_EQUAL(1,               cap[0].lane_cnt);
    LONGS_EQUAL(DP_LINK_RATE_RBR_MASK | DP_LINK_RATE_HBR1_MASK | DP_LINK_RATE_HBR2_MASK | DP_LINK_RATE_HBR3_MASK, cap[0].link_rate_mask);
    //LONGS_EQUAL(1, cap.capability.fec_support);
    LONGS_EQUAL(0, cap[0].capability.dsc_support);

    LONGS_EQUAL(DPRX_DRV_NO_ERR, ret[1]);
    LONGS_EQUAL(DP_TYPE_mDP,     cap[1].type);
    LONGS_EQUAL(1,               cap[1].lane_cnt);
    LONGS_EQUAL(0,               cap[1].capability.dsc_support);
    LONGS_EQUAL(DP_LINK_RATE_RBR_MASK | DP_LINK_RATE_HBR1_MASK | DP_LINK_RATE_HBR2_MASK | DP_LINK_RATE_HBR3_MASK, cap[1].link_rate_mask);
    //LONGS_EQUAL(1, cap.capability.fec_support);
}


//--------------------------------------------------
// Func  : DPRX_GET_PORT_CAPABILITY_DP1_MDP_2_LANE_DP2_MDP_2_LANE
//
// Desc  : Check dprx_drv_source_check with idel pattern
//
// Pass  :
//--------------------------------------------------
TEST(dprx_vfe_get_port_capability, DPRX_GET_PORT_CAPABILITY_DP1_MDP_1_LANE_DP2_MDP_2_LANE)
{
    // Arrange
    DP_PORT_CAPABILITY cap[2];
    INT8  ret[2] = {0xF, 0xF};
    init_pcb_parameter(PCB_PARAM_DP1_MDP_1_LANE_DP2_MDP_2_LANE);

    // action
    vfe_dprx_drv_init();

    ret[0] = vfe_dprx_drv_get_rx_port_capability(0, &cap[0]);
    ret[1] = vfe_dprx_drv_get_rx_port_capability(1, &cap[1]);

    vfe_dprx_drv_uninit();

    // Assert
    LONGS_EQUAL(DPRX_DRV_NO_ERR, ret[0]);
    LONGS_EQUAL(DP_TYPE_mDP,     cap[0].type);
    LONGS_EQUAL(1,               cap[0].lane_cnt);
    LONGS_EQUAL(DP_LINK_RATE_RBR_MASK | DP_LINK_RATE_HBR1_MASK | DP_LINK_RATE_HBR2_MASK | DP_LINK_RATE_HBR3_MASK, cap[0].link_rate_mask);
    //LONGS_EQUAL(1, cap.capability.fec_support);
    LONGS_EQUAL(0, cap[0].capability.dsc_support);

    LONGS_EQUAL(DPRX_DRV_NO_ERR, ret[1]);
    LONGS_EQUAL(DP_TYPE_mDP,     cap[1].type);
    LONGS_EQUAL(2,               cap[1].lane_cnt);
    LONGS_EQUAL(0,               cap[1].capability.dsc_support);
    LONGS_EQUAL(DP_LINK_RATE_RBR_MASK | DP_LINK_RATE_HBR1_MASK | DP_LINK_RATE_HBR2_MASK | DP_LINK_RATE_HBR3_MASK, cap[1].link_rate_mask);
    //LONGS_EQUAL(1, cap.capability.fec_support);
}



//--------------------------------------------------
// Func  : DPRX_GET_PORT_CAPABILITY_DP1_MDP_2_LANE_DP2_MDP_4_LANE
//
// Desc  : Check dprx_drv_source_check with idel pattern
//
// Pass  :
//--------------------------------------------------
TEST(dprx_vfe_get_port_capability, DPRX_GET_PORT_CAPABILITY_DP1_MDP_1_LANE_DP2_MDP_4_LANE)
{
    // Arrange
    DP_PORT_CAPABILITY cap[2];
    INT8  ret[2] = {0xF, 0xF};
    init_pcb_parameter(PCB_PARAM_DP1_MDP_1_LANE_DP2_MDP_4_LANE);

    // action
    vfe_dprx_drv_init();

    ret[0] = vfe_dprx_drv_get_rx_port_capability(0, &cap[0]);
    ret[1] = vfe_dprx_drv_get_rx_port_capability(1, &cap[1]);

    vfe_dprx_drv_uninit();

    // Assert
    LONGS_EQUAL(DPRX_DRV_NO_ERR, ret[0]);
    LONGS_EQUAL(DP_TYPE_mDP,     cap[0].type);
    LONGS_EQUAL(1,               cap[0].lane_cnt);
    LONGS_EQUAL(DP_LINK_RATE_RBR_MASK | DP_LINK_RATE_HBR1_MASK | DP_LINK_RATE_HBR2_MASK | DP_LINK_RATE_HBR3_MASK, cap[0].link_rate_mask);
    //LONGS_EQUAL(1, cap.capability.fec_support);
    LONGS_EQUAL(0, cap[0].capability.dsc_support);

    LONGS_EQUAL(DPRX_DRV_NO_ERR, ret[1]);
    LONGS_EQUAL(DP_TYPE_mDP,     cap[1].type);
    LONGS_EQUAL(4,               cap[1].lane_cnt);
    LONGS_EQUAL(0,               cap[1].capability.dsc_support);
    LONGS_EQUAL(DP_LINK_RATE_RBR_MASK | DP_LINK_RATE_HBR1_MASK | DP_LINK_RATE_HBR2_MASK | DP_LINK_RATE_HBR3_MASK, cap[1].link_rate_mask);
    //LONGS_EQUAL(1, cap.capability.fec_support);
}



//--------------------------------------------------
// Func  : DPRX_GET_PORT_CAPABILITY_DP1_MDP__LANE_DP2_MDP_1_LANE
//
// Desc  : Check dprx_drv_source_check with idel pattern
//
// Pass  :
//--------------------------------------------------
TEST(dprx_vfe_get_port_capability, DPRX_GET_PORT_CAPABILITY_DP1_MDP_2_LANE_DP2_MDP_1_LANE)
{
    // Arrange
    DP_PORT_CAPABILITY cap[2];
    INT8  ret[2] = {0xF, 0xF};
    init_pcb_parameter(PCB_PARAM_DP1_MDP_2_LANE_DP2_MDP_1_LANE);

    // action
    vfe_dprx_drv_init();

    ret[0] = vfe_dprx_drv_get_rx_port_capability(0, &cap[0]);
    ret[1] = vfe_dprx_drv_get_rx_port_capability(1, &cap[1]);

    vfe_dprx_drv_uninit();

    // Assert
    LONGS_EQUAL(DPRX_DRV_NO_ERR, ret[0]);
    LONGS_EQUAL(DP_TYPE_mDP,     cap[0].type);
    LONGS_EQUAL(2,               cap[0].lane_cnt);
    LONGS_EQUAL(DP_LINK_RATE_RBR_MASK | DP_LINK_RATE_HBR1_MASK | DP_LINK_RATE_HBR2_MASK | DP_LINK_RATE_HBR3_MASK, cap[0].link_rate_mask);
    //LONGS_EQUAL(1, cap.capability.fec_support);
    LONGS_EQUAL(0, cap[0].capability.dsc_support);

    LONGS_EQUAL(DPRX_DRV_NO_ERR, ret[1]);
    LONGS_EQUAL(DP_TYPE_mDP,     cap[1].type);
    LONGS_EQUAL(1,               cap[1].lane_cnt);
    LONGS_EQUAL(0,               cap[1].capability.dsc_support);
    LONGS_EQUAL(DP_LINK_RATE_RBR_MASK | DP_LINK_RATE_HBR1_MASK | DP_LINK_RATE_HBR2_MASK | DP_LINK_RATE_HBR3_MASK, cap[1].link_rate_mask);
    //LONGS_EQUAL(1, cap.capability.fec_support);
}


//--------------------------------------------------
// Func  : DPRX_GET_PORT_CAPABILITY_DP1_MDP_2_LANE_DP2_MDP_2_LANE
//
// Desc  : Check dprx_drv_source_check with idel pattern
//
// Pass  :
//--------------------------------------------------
TEST(dprx_vfe_get_port_capability, DPRX_GET_PORT_CAPABILITY_DP1_MDP_2_LANE_DP2_MDP_2_LANE)
{
    // Arrange
    DP_PORT_CAPABILITY cap[2];
    INT8  ret[2] = {0xF, 0xF};
    init_pcb_parameter(PCB_PARAM_DP1_MDP_2_LANE_DP2_MDP_2_LANE);

    // action
    vfe_dprx_drv_init();

    ret[0] = vfe_dprx_drv_get_rx_port_capability(0, &cap[0]);
    ret[1] = vfe_dprx_drv_get_rx_port_capability(1, &cap[1]);

    vfe_dprx_drv_uninit();

    // Assert
    LONGS_EQUAL(DPRX_DRV_NO_ERR, ret[0]);
    LONGS_EQUAL(DP_TYPE_mDP,     cap[0].type);
    LONGS_EQUAL(2,               cap[0].lane_cnt);
    LONGS_EQUAL(DP_LINK_RATE_RBR_MASK | DP_LINK_RATE_HBR1_MASK | DP_LINK_RATE_HBR2_MASK | DP_LINK_RATE_HBR3_MASK, cap[0].link_rate_mask);
    //LONGS_EQUAL(1, cap.capability.fec_support);
    LONGS_EQUAL(0, cap[0].capability.dsc_support);

    LONGS_EQUAL(DPRX_DRV_NO_ERR, ret[1]);
    LONGS_EQUAL(DP_TYPE_mDP,     cap[1].type);
    LONGS_EQUAL(2,               cap[1].lane_cnt);
    LONGS_EQUAL(0,               cap[1].capability.dsc_support);
    LONGS_EQUAL(DP_LINK_RATE_RBR_MASK | DP_LINK_RATE_HBR1_MASK | DP_LINK_RATE_HBR2_MASK | DP_LINK_RATE_HBR3_MASK, cap[1].link_rate_mask);
    //LONGS_EQUAL(1, cap.capability.fec_support);
}


//--------------------------------------------------
// Func  : DPRX_GET_PORT_CAPABILITY_DP1_MDP_2_LANE_DP2_MDP_4_LANE
//
// Desc  : Check dprx_drv_source_check with idel pattern
//
// Pass  :
//--------------------------------------------------
TEST(dprx_vfe_get_port_capability, DPRX_GET_PORT_CAPABILITY_DP1_MDP_2_LANE_DP2_MDP_4_LANE)
{
    // Arrange
    DP_PORT_CAPABILITY cap[2];
    INT8  ret[2] = {0xF, 0xF};
    init_pcb_parameter(PCB_PARAM_DP1_MDP_2_LANE_DP2_MDP_4_LANE);

    // action
    vfe_dprx_drv_init();

    ret[0] = vfe_dprx_drv_get_rx_port_capability(0, &cap[0]);
    ret[1] = vfe_dprx_drv_get_rx_port_capability(1, &cap[1]);

    vfe_dprx_drv_uninit();

    // Assert
    LONGS_EQUAL(DPRX_DRV_NO_ERR, ret[0]);
    LONGS_EQUAL(DP_TYPE_mDP,     cap[0].type);
    LONGS_EQUAL(2,               cap[0].lane_cnt);
    LONGS_EQUAL(DP_LINK_RATE_RBR_MASK | DP_LINK_RATE_HBR1_MASK | DP_LINK_RATE_HBR2_MASK | DP_LINK_RATE_HBR3_MASK, cap[0].link_rate_mask);
    //LONGS_EQUAL(1, cap.capability.fec_support);
    LONGS_EQUAL(0, cap[0].capability.dsc_support);

    LONGS_EQUAL(DPRX_DRV_NO_ERR, ret[1]);
    LONGS_EQUAL(DP_TYPE_mDP,     cap[1].type);
    LONGS_EQUAL(4,               cap[1].lane_cnt);
    LONGS_EQUAL(0,               cap[1].capability.dsc_support);
    LONGS_EQUAL(DP_LINK_RATE_RBR_MASK | DP_LINK_RATE_HBR1_MASK | DP_LINK_RATE_HBR2_MASK | DP_LINK_RATE_HBR3_MASK, cap[1].link_rate_mask);
    //LONGS_EQUAL(1, cap.capability.fec_support);
}


//--------------------------------------------------
// Func  : DPRX_GET_PORT_CAPABILITY_DP1_MDP_4_LANE_DP2_MDP_1_LANE
//
// Desc  : Check dprx_drv_source_check with idel pattern
//
// Pass  :
//--------------------------------------------------
TEST(dprx_vfe_get_port_capability, DPRX_GET_PORT_CAPABILITY_DP1_MDP_4_LANE_DP2_MDP_1_LANE)
{
    // Arrange
    DP_PORT_CAPABILITY cap[2];
    INT8  ret[2] = {0xF, 0xF};
    init_pcb_parameter(PCB_PARAM_DP1_MDP_4_LANE_DP2_MDP_1_LANE);

    // action
    vfe_dprx_drv_init();

    ret[0] = vfe_dprx_drv_get_rx_port_capability(0, &cap[0]);
    ret[1] = vfe_dprx_drv_get_rx_port_capability(1, &cap[1]);

    vfe_dprx_drv_uninit();

    // Assert
    LONGS_EQUAL(DPRX_DRV_NO_ERR, ret[0]);
    LONGS_EQUAL(DP_TYPE_mDP,     cap[0].type);
    LONGS_EQUAL(4,               cap[0].lane_cnt);
    LONGS_EQUAL(DP_LINK_RATE_RBR_MASK | DP_LINK_RATE_HBR1_MASK | DP_LINK_RATE_HBR2_MASK | DP_LINK_RATE_HBR3_MASK, cap[0].link_rate_mask);
    //LONGS_EQUAL(1, cap.capability.fec_support);
    LONGS_EQUAL(0, cap[0].capability.dsc_support);

    LONGS_EQUAL(DPRX_DRV_NO_ERR, ret[1]);
    LONGS_EQUAL(DP_TYPE_mDP,     cap[1].type);
    LONGS_EQUAL(1,               cap[1].lane_cnt);
    LONGS_EQUAL(0,               cap[1].capability.dsc_support);
    LONGS_EQUAL(DP_LINK_RATE_RBR_MASK | DP_LINK_RATE_HBR1_MASK | DP_LINK_RATE_HBR2_MASK | DP_LINK_RATE_HBR3_MASK, cap[1].link_rate_mask);
    //LONGS_EQUAL(1, cap.capability.fec_support);
}


//--------------------------------------------------
// Func  : DPRX_GET_PORT_CAPABILITY_DP1_MDP_4_LANE_DP2_MDP_2_LANE
//
// Desc  : Check dprx_drv_source_check with idel pattern
//
// Pass  :
//--------------------------------------------------
TEST(dprx_vfe_get_port_capability, DPRX_GET_PORT_CAPABILITY_DP1_MDP_4_LANE_DP2_MDP_2_LANE)
{
    // Arrange
    DP_PORT_CAPABILITY cap[2];
    INT8  ret[2] = {0xF, 0xF};
    init_pcb_parameter(PCB_PARAM_DP1_MDP_4_LANE_DP2_MDP_2_LANE);

    // action
    vfe_dprx_drv_init();

    ret[0] = vfe_dprx_drv_get_rx_port_capability(0, &cap[0]);
    ret[1] = vfe_dprx_drv_get_rx_port_capability(1, &cap[1]);

    vfe_dprx_drv_uninit();

    // Assert
    LONGS_EQUAL(DPRX_DRV_NO_ERR, ret[0]);
    LONGS_EQUAL(DP_TYPE_mDP,     cap[0].type);
    LONGS_EQUAL(4,               cap[0].lane_cnt);
    LONGS_EQUAL(DP_LINK_RATE_RBR_MASK | DP_LINK_RATE_HBR1_MASK | DP_LINK_RATE_HBR2_MASK | DP_LINK_RATE_HBR3_MASK, cap[0].link_rate_mask);
    //LONGS_EQUAL(1, cap.capability.fec_support);
    LONGS_EQUAL(0, cap[0].capability.dsc_support);

    LONGS_EQUAL(DPRX_DRV_NO_ERR, ret[1]);
    LONGS_EQUAL(DP_TYPE_mDP,     cap[1].type);
    LONGS_EQUAL(2,               cap[1].lane_cnt);
    LONGS_EQUAL(0,               cap[1].capability.dsc_support);
    LONGS_EQUAL(DP_LINK_RATE_RBR_MASK | DP_LINK_RATE_HBR1_MASK | DP_LINK_RATE_HBR2_MASK | DP_LINK_RATE_HBR3_MASK, cap[1].link_rate_mask);
    //LONGS_EQUAL(1, cap.capability.fec_support);
}



//--------------------------------------------------
// Func  : DPRX_GET_PORT_CAPABILITY_DP1_MDP_4_LANE_DP2_MDP_4_LANE
//
// Desc  : Check dprx_drv_source_check with idel pattern
//
// Pass  :
//--------------------------------------------------
TEST(dprx_vfe_get_port_capability, DPRX_GET_PORT_CAPABILITY_DP1_MDP_4_LANE_DP2_MDP_4_LANE)
{
    // Arrange
    DP_PORT_CAPABILITY cap[2];
    INT8  ret[2] = {0xF, 0xF};
    init_pcb_parameter(PCB_PARAM_DP1_MDP_4_LANE_DP2_MDP_4_LANE);

    // action
    vfe_dprx_drv_init();

    ret[0] = vfe_dprx_drv_get_rx_port_capability(0, &cap[0]);
    ret[1] = vfe_dprx_drv_get_rx_port_capability(1, &cap[1]);

    vfe_dprx_drv_uninit();

    // Assert
    LONGS_EQUAL(DPRX_DRV_NO_ERR, ret[0]);
    LONGS_EQUAL(DP_TYPE_mDP,     cap[0].type);
    LONGS_EQUAL(4,               cap[0].lane_cnt);
    LONGS_EQUAL(DP_LINK_RATE_RBR_MASK | DP_LINK_RATE_HBR1_MASK | DP_LINK_RATE_HBR2_MASK | DP_LINK_RATE_HBR3_MASK, cap[0].link_rate_mask);
    //LONGS_EQUAL(1, cap.capability.fec_support);
    LONGS_EQUAL(0, cap[0].capability.dsc_support);

    LONGS_EQUAL(DPRX_DRV_NO_ERR, ret[1]);
    LONGS_EQUAL(DP_TYPE_mDP,     cap[1].type);
    LONGS_EQUAL(4,               cap[1].lane_cnt);
    LONGS_EQUAL(0,               cap[1].capability.dsc_support);
    LONGS_EQUAL(DP_LINK_RATE_RBR_MASK | DP_LINK_RATE_HBR1_MASK | DP_LINK_RATE_HBR2_MASK | DP_LINK_RATE_HBR3_MASK, cap[1].link_rate_mask);
    //LONGS_EQUAL(1, cap.capability.fec_support);
}
