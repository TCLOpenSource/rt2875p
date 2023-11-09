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


TEST_GROUP(vfe_dprx_drv_get_stream_type)
{
    UINT8 port = 0;

    void setup()
    {
        // Init Virtual Platform
        virt_platform_init();
        init_pcb_parameter(PCB_PARAM_DP1_MDP_4_LANE);
        virt_te_set_cable_connect(1);

        // Init VFE
        vfe_dprx_drv_init();
        vfe_dprx_drv_open();
        vfe_dprx_drv_set_hpd(port, 1);  // enable HPD
        vfe_dprx_drv_connect(port);
    }
    void teardown()
    {
        // UnInit VFE
        vfe_dprx_drv_uninit();

        // UnInit Virtual Platform
        virt_te_set_cable_connect(0);
        FFF_RESET;
    }
};


//--------------------------------------------------
// Func  : DprxVfeGetStreamTypeNone
//
// Desc  : Check dprx_drv_source_check with idel pattern
//
// Pass  :
//--------------------------------------------------
TEST(vfe_dprx_drv_get_stream_type, DprxVfeGetStreamTypeNone)
{
    // Arrange
    DP_STREAM_TYPE_E type = DPRX_STREAM_TYPE_MAX;
    INT8 link_train_pass = 0;

    // action
    vfe_dprx_drv_set_hpd(port, 1);  // enable HPD

    link_train_pass = virt_te_do_link_training();

    if (link_train_pass)
    {
        virt_te_set_video_timing(STREAM_TIMING_NONE);
        virt_te_set_color_format(STREAM_COLOR_FORMAT_RGB_8_BIT);
        virt_te_set_video_output(0);
        vfe_dprx_drv_get_stream_type(port, &type);
    }

    // Assert
    LONGS_EQUAL(1, link_train_pass);
    LONGS_EQUAL(DPRX_STREAM_TYPE_UNKNOWN, type);
}


//--------------------------------------------------
// Func  : DprxVfeGetStreamTypeNone
//
// Desc  : Check dprx_drv_source_check with idel pattern
//
// Pass  :
//--------------------------------------------------
TEST(vfe_dprx_drv_get_stream_type, DprxVfeGetStreamTypeVideoOnly)
{
    // Arrange
    DP_STREAM_TYPE_E type = DPRX_STREAM_TYPE_MAX;
    INT8 link_train_pass = 0;

    // action
    link_train_pass = virt_te_do_link_training();

    if (link_train_pass)
    {
        virt_te_set_video_timing(STREAM_TIMING_CTA_861_VIC_97_4K_2K_60P);
        virt_te_set_color_format(STREAM_COLOR_FORMAT_RGB_8_BIT);
        virt_te_set_video_output(1);
        vfe_dprx_drv_get_stream_type(port, &type);
    }

    // Assert
    LONGS_EQUAL(1, link_train_pass);
    LONGS_EQUAL(DPRX_STREAM_TYPE_VIDEO_ONLY, type);
}