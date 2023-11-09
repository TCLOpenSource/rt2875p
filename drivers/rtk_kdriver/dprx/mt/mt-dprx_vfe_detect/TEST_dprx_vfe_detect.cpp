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

#define POLL_TIMING_TRY_COUNT           10

TEST_GROUP(dprx_vfe_detect)
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


void dump_timing_info(DPRX_TIMING_INFO_T *p_timing_info)
{
    printf("=========== Timing ==========\n");
    printf("color_space : %d             \n", p_timing_info->color_space);
    printf("color_depth : %d             \n", p_timing_info->color_depth);
    printf("interlace : %d               \n", p_timing_info->is_interlace);
    printf("dsc : %d, allm=%d            \n", p_timing_info->is_dsc, p_timing_info->isALLM);
    printf("drr_mode : %d, curr_vfreq_hz_x100=%d \n", p_timing_info->drr_mode , p_timing_info->curr_vfreq_hz_x100);
    printf("-----------------------------\n");
    printf("        | H       | V        \n");
    printf("--------+---------+----------\n");
    printf(" total  | %5d     | %5d      \n", p_timing_info->htotal, p_timing_info->vtotal);
    printf(" active | %5d     | %5d      \n", p_timing_info->hact, p_timing_info->vact);
    printf(" porch  | %5d     | %5d      \n", p_timing_info->hporch, p_timing_info->vporch);
    printf(" start  | %5d     | %5d      \n", p_timing_info->hstart, p_timing_info->vstart);
    printf(" sync   | %5d     | %5d      \n", p_timing_info->hsync, p_timing_info->vsync);
    printf(" freq   | %5d     | %5d      \n", p_timing_info->hfreq_hz, p_timing_info->vfreq_hz_x100);
    printf("-----------------------------\n");
}


/*------------------------------------------------
 * Func : poll_get_port_timing
 *
 * Desc : check source detect
 *
 * Para : port : DPX port
 *        try_count : timeout
 *
 * Retn : 1 : successed, 0 : failed
 *-----------------------------------------------*/
UINT8 poll_get_port_timing(
    UINT8           port,
    DPRX_TIMING_INFO_T* p_timing,
    UINT16          try_cnt
    )
{
    while(try_cnt--)
    {
        INT8 ret = vfe_dprx_drv_source_check(port);

        printf("vfe_dprx_drv_source_check=%d (%d)\n", ret, try_cnt);

        if (ret==DPRX_DRV_NO_ERR)
        {
            vfe_dprx_drv_get_port_timing_info(port, p_timing);
            return 1;
        }

        usleep(1000);  // polling for each 1 ms
    }

    return 0;
}


//--------------------------------------------------
// Func  : DDprxVfeGetPortTimingWithIdelPattern
//
// Desc  : Check dprx_drv_source_check with idel pattern
//
// Pass  : Link Training Pass
//         Timing detect fail
//--------------------------------------------------
TEST(dprx_vfe_detect, DDprxVfeGetPortTimingWithIdelPattern)
{
    // Arrange
    DPRX_TIMING_INFO_T timing_info;
    INT8  link_train_pass = 0;
    UINT8 timing_detected = 0;

    // action
    link_train_pass = virt_te_do_link_training();

    if (link_train_pass)
    {
        virt_te_set_video_timing(STREAM_TIMING_NONE);
        virt_te_set_color_format(STREAM_COLOR_FORMAT_RGB_8_BIT);
        virt_te_set_video_output(0);
        timing_detected = poll_get_port_timing(port, &timing_info, POLL_TIMING_TRY_COUNT);

        dump_timing_info(&timing_info);
    }

    // Assert
    LONGS_EQUAL(1, link_train_pass);
    LONGS_EQUAL(0, timing_detected);
}


//--------------------------------------------------
// Func  : DprxVfeGetPortTimingWith4K2K60Video
//
// Desc  :  make sure DPRX VFE unabled to detect signal
//         if TX send idel pattern only
//
// Pass  : Link Training Pass
//         Timing detect Pass
//         Timing correct
//--------------------------------------------------
TEST(dprx_vfe_detect, DprxVfeGetPortTimingWith4K2K60Video)
{
    // Arrange
    DPRX_TIMING_INFO_T timing_info;
    STREAM_TIMING_ID timing_id = STREAM_TIMING_CTA_861_VIC_97_4K_2K_60P;
    STREAM_TIMING* p_expected_timing = get_timing_info(timing_id);
    INT8  link_train_pass = 0;
    UINT8 timing_detected = 0;

    // action
    link_train_pass = virt_te_do_link_training();

    if (link_train_pass)
    {
        virt_te_set_video_timing(timing_id);
        virt_te_set_color_format(STREAM_COLOR_FORMAT_RGB_8_BIT);
        virt_te_set_video_output(1);
        timing_detected = poll_get_port_timing(port, &timing_info, POLL_TIMING_TRY_COUNT);

        dump_timing_info(&timing_info);
    }

    // Assert
    LONGS_EQUAL(1, link_train_pass);
    LONGS_EQUAL(1, timing_detected);
    LONGS_EQUAL(p_expected_timing->h_total, timing_info.htotal);
    LONGS_EQUAL(p_expected_timing->v_total, timing_info.vtotal);
    LONGS_EQUAL(p_expected_timing->h_act, timing_info.hact);
    LONGS_EQUAL(p_expected_timing->v_act, timing_info.vact);
}
