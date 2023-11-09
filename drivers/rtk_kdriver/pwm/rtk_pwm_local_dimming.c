#include <rtk_kdriver/rtk_pwm.h>
#include "rtk_pwm_local_dimming.h"
#include "rtk_pwm_func.h"
#include <rbus/ldspi_reg.h>
#include <rbus/pinmux_lvdsphy_reg.h>
#include <rbus/ppoverlay_reg.h>
#include <tvscalercontrol/panel/panelapi.h>
#include <mach/rtk_platform.h>

//unsigned int ld_time_tick=0;

void rtk_local_dimming_enable(void)
{
    //extern webos_info_t  webos_tooloption;
    rtd_pr_pwm_ld_warn("+++ %s.\n", __FUNCTION__ );
    rtd_part_outl(LDSPI_LD_CTRL_reg,28,28,0x1);                        //Ld_spi enable
    rtd_part_outl(LDSPI_LD_CTRL_reg,31,30,0x1);                        //LGD mode(default)
    rtd_part_outl(LDSPI_LD_CTRL_reg,4,4,0x1);                          //LD_SPI1
    //rtd_part_outl(0xb802C900,4,4,0x0);                               //LD_SPI0
    rtd_part_outl(LDSPI_LD_CTRL_reg,3,3,0x0);                          //one SPI local dimming block
    //rtd_part_outl(0xb802C900,3,3,0x1);                               //two SPI local dimming block
    rtd_part_outl(LDSPI_Out_data_CTRL_reg,31,31,0x1);                  //First received bit or transmitted bit is in the MSB of SRAM or register
    rtd_part_outl(LDSPI_Out_data_CTRL_reg,1,0,0x0);                    //Output_Data_format:8 bit
    //rtd_part_outl(0xb802C904,1,0,0x1);                               //Output_Data_format:8 bit
    rtd_part_outl(LDSPI_Send_delay_reg,31,8,0x4a9);                    //Data_send_Delay   T1=(data_send_delay+CS_hold+SCKL+10)*4 = 10us
    rtd_part_outl(LDSPI_sck_hold_time_reg,31,8,0x4a9);                 //Sck hold time
    rtd_part_outl(LDSPI_Cs_hold_time_reg,31,8,0x4a9);                  //CS_hold_time
    rtd_part_outl(LDSPI_Timing_CTRL_reg,23,16,0x68);                   //SCK_H             T2=(SCK_H+SCK_L+2)*4= 0.84us
    rtd_part_outl(LDSPI_Timing_CTRL_reg,7,0,0x68);                     //SCK_L             T3=T2/2 = 0.42us
    rtd_part_outl(LDSPI_Data_unit_delay_reg,31,8,0x10b);               //Each_unit_delay   T4=(Each_unit_delay+SCK_H+SCK_L)*4 = 1.9us
    rtd_part_outl(LDSPI_V_sync_duty_reg,31,24,0xaf);                   //Vsync_d           T5=(Vsync_d+1)*128*4 = 30.2us
    rtd_part_outl(LDSPI_Data_h_time_reg,24,16,0x68);                   //Data_hold_time, must to set equal SCK_H!!!
    //rtd_pr_pwm_ld_warn("[pyToolOpt]BKlight:%d,local_dim_block%d,localDimming:%d\n",webos_tooloption.eBackLight, webos_tooloption.eLEDBarType,webos_tooloption.bLocalDimming);

      /*if ((webos_tooloption.eBackLight == 0) && (webos_tooloption.eLEDBarType == 2) && (webos_tooloption.bLocalDimming == 1)) {
            rtd_part_outl(LDSPI_Out_data_CTRL_reg,16,4,0x24);          //Output Unit:8 unit(= set+1)
            rtd_part_outl(LDSPI_LD_CTRL_reg,0,0,0x0);                  //Ld_spi disable
            rtd_part_outl(LDSPI_Outindex_Addr_CTRL_reg,26,16,0x0);     //SRAM index
            rtd_part_outl(LDSPI_Outindex_Rwport_reg,11,0,0x80);
            rtd_part_outl(LDSPI_Outindex_Rwport_reg,11,0,0x81);
            rtd_part_outl(LDSPI_Outindex_Rwport_reg,11,0,0x82);
            rtd_part_outl(LDSPI_Outindex_Rwport_reg,11,0,0x83);
            rtd_part_outl(LDSPI_Outindex_Rwport_reg,11,0,0x84);
            rtd_part_outl(LDSPI_Outindex_Rwport_reg,11,0,0x0);
            rtd_part_outl(LDSPI_Outindex_Rwport_reg,11,0,0x1);
            rtd_part_outl(LDSPI_Outindex_Rwport_reg,11,0,0x2);
            rtd_part_outl(LDSPI_Outindex_Rwport_reg,11,0,0x3);
            rtd_part_outl(LDSPI_Outindex_Rwport_reg,11,0,0x4);
            rtd_part_outl(LDSPI_Outindex_Rwport_reg,11,0,0x5);
            rtd_part_outl(LDSPI_Outindex_Rwport_reg,11,0,0x6);
            rtd_part_outl(LDSPI_Outindex_Rwport_reg,11,0,0x7);
            rtd_part_outl(LDSPI_Outindex_Rwport_reg,11,0,0x8);
            rtd_part_outl(LDSPI_Outindex_Rwport_reg,11,0,0x9);
            rtd_part_outl(LDSPI_Outindex_Rwport_reg,11,0,0xA);
            rtd_part_outl(LDSPI_Outindex_Rwport_reg,11,0,0xB);
            rtd_part_outl(LDSPI_Outindex_Rwport_reg,11,0,0xC);
            rtd_part_outl(LDSPI_Outindex_Rwport_reg,11,0,0xD);
            rtd_part_outl(LDSPI_Outindex_Rwport_reg,11,0,0xE);
            rtd_part_outl(LDSPI_Outindex_Rwport_reg,11,0,0xF);
            rtd_part_outl(LDSPI_Outindex_Rwport_reg,11,0,0x10);
            rtd_part_outl(LDSPI_Outindex_Rwport_reg,11,0,0x11);
            rtd_part_outl(LDSPI_Outindex_Rwport_reg,11,0,0x12);
            rtd_part_outl(LDSPI_Outindex_Rwport_reg,11,0,0x13);
            rtd_part_outl(LDSPI_Outindex_Rwport_reg,11,0,0x14);
            rtd_part_outl(LDSPI_Outindex_Rwport_reg,11,0,0x15);
            rtd_part_outl(LDSPI_Outindex_Rwport_reg,11,0,0x16);
            rtd_part_outl(LDSPI_Outindex_Rwport_reg,11,0,0x17);
            rtd_part_outl(LDSPI_Outindex_Rwport_reg,11,0,0x18);
            rtd_part_outl(LDSPI_Outindex_Rwport_reg,11,0,0x19);
            rtd_part_outl(LDSPI_Outindex_Rwport_reg,11,0,0x1A);
            rtd_part_outl(LDSPI_Outindex_Rwport_reg,11,0,0x1B);
            rtd_part_outl(LDSPI_Outindex_Rwport_reg,11,0,0x1C);
            rtd_part_outl(LDSPI_Outindex_Rwport_reg,11,0,0x1D);
            rtd_part_outl(LDSPI_Outindex_Rwport_reg,11,0,0x1E);
            rtd_part_outl(LDSPI_Outindex_Rwport_reg,11,0,0x1F);

            ////backlight control
            rtd_part_outl(LDSPI_Data_Addr_CTRL_reg,27,16,0x0); //Data index
            rtd_part_outl(LDSPI_Data_Rwport_reg,11,0,0x99);
            rtd_part_outl(LDSPI_Data_Rwport_reg,11,0,0x99);
            rtd_part_outl(LDSPI_Data_Rwport_reg,11,0,0x99);
            rtd_part_outl(LDSPI_Data_Rwport_reg,11,0,0x99);
            rtd_part_outl(LDSPI_Data_Rwport_reg,11,0,0x99);
            rtd_part_outl(LDSPI_Data_Rwport_reg,11,0,0x99);
            rtd_part_outl(LDSPI_Data_Rwport_reg,11,0,0x99);
            rtd_part_outl(LDSPI_Data_Rwport_reg,11,0,0x99);
            rtd_part_outl(LDSPI_Data_Rwport_reg,11,0,0x99);
            rtd_part_outl(LDSPI_Data_Rwport_reg,11,0,0x99);
            rtd_part_outl(LDSPI_Data_Rwport_reg,11,0,0x99);
            rtd_part_outl(LDSPI_Data_Rwport_reg,11,0,0x99);
            rtd_part_outl(LDSPI_Data_Rwport_reg,11,0,0x99);
            rtd_part_outl(LDSPI_Data_Rwport_reg,11,0,0x99);
            rtd_part_outl(LDSPI_Data_Rwport_reg,11,0,0x99);
            rtd_part_outl(LDSPI_Data_Rwport_reg,11,0,0x99);
            rtd_part_outl(LDSPI_Data_Rwport_reg,11,0,0x99);
            rtd_part_outl(LDSPI_Data_Rwport_reg,11,0,0x99);
            rtd_part_outl(LDSPI_Data_Rwport_reg,11,0,0x99);
            rtd_part_outl(LDSPI_Data_Rwport_reg,11,0,0x99);
            rtd_part_outl(LDSPI_Data_Rwport_reg,11,0,0x99);
            rtd_part_outl(LDSPI_Data_Rwport_reg,11,0,0x99);
            rtd_part_outl(LDSPI_Data_Rwport_reg,11,0,0x99);
            rtd_part_outl(LDSPI_Data_Rwport_reg,11,0,0x99);
            rtd_part_outl(LDSPI_Data_Rwport_reg,11,0,0x99);
            rtd_part_outl(LDSPI_Data_Rwport_reg,11,0,0x99);
            rtd_part_outl(LDSPI_Data_Rwport_reg,11,0,0x99);
            rtd_part_outl(LDSPI_Data_Rwport_reg,11,0,0x99);
            rtd_part_outl(LDSPI_Data_Rwport_reg,11,0,0x99);
            rtd_part_outl(LDSPI_Data_Rwport_reg,11,0,0x99);
            rtd_part_outl(LDSPI_Data_Rwport_reg,11,0,0x99);
            rtd_part_outl(LDSPI_Data_Rwport_reg,11,0,0x99);
            rtd_part_outl(LDSPI_Data_Addr_CTRL_reg,27,16,0x80);     //Data : index
            rtd_part_outl(LDSPI_Data_Rwport_reg,11,0,0xAA);         //Data : ID
            rtd_part_outl(LDSPI_Data_Rwport_reg,11,0,0x01);         //Data : cmd
            rtd_part_outl(LDSPI_Data_Rwport_reg,11,0,0x00);
            rtd_part_outl(LDSPI_Data_Rwport_reg,11,0,0x64);
            rtd_part_outl(LDSPI_Data_Rwport_reg,11,0,0xFF);
       } else {*/
          rtd_part_outl(LDSPI_Out_data_CTRL_reg,16,4,0x7);          //Output Unit:8 unit(= set+1)
          rtd_part_outl(LDSPI_LD_CTRL_reg,0,0,0x0);                 //Ld_spi disable
          rtd_part_outl(LDSPI_Outindex_Addr_CTRL_reg,26,16,0x0);    //SRAM index
          rtd_part_outl(LDSPI_Outindex_Rwport_reg,11,0,0x0);
          rtd_part_outl(LDSPI_Outindex_Rwport_reg,11,0,0x1);
          rtd_part_outl(LDSPI_Outindex_Rwport_reg,11,0,0x2);
          rtd_part_outl(LDSPI_Outindex_Rwport_reg,11,0,0x3);
          rtd_part_outl(LDSPI_Outindex_Rwport_reg,11,0,0x4);
          rtd_part_outl(LDSPI_Outindex_Rwport_reg,11,0,0x5);
          rtd_part_outl(LDSPI_Outindex_Rwport_reg,11,0,0x6);
          rtd_part_outl(LDSPI_Outindex_Rwport_reg,11,0,0x7);

          //backlight control
          rtd_part_outl(LDSPI_Data_Addr_CTRL_reg,27,16,0x0);       //Data index
          rtd_part_outl(LDSPI_Data_Rwport_reg,11,0,0xAA);          //data : ID
          rtd_part_outl(LDSPI_Data_Rwport_reg,11,0,0x01);          //data : cmd
          rtd_part_outl(LDSPI_Data_Rwport_reg,11,0,0x99);
          rtd_part_outl(LDSPI_Data_Rwport_reg,11,0,0x99);
          rtd_part_outl(LDSPI_Data_Rwport_reg,11,0,0x99);
          rtd_part_outl(LDSPI_Data_Rwport_reg,11,0,0x99);
          rtd_part_outl(LDSPI_Data_Rwport_reg,11,0,0x99);
          rtd_part_outl(LDSPI_Data_Rwport_reg,11,0,0x99);
          //checksum = 1E
       //}
#if 1
    rtd_inl(LDSPI_Data_Addr_CTRL_reg);
    rtd_inl(LDSPI_Data_Rwport_reg);
    rtd_inl(LDSPI_Data_Rwport_reg);
    rtd_inl(LDSPI_Data_Rwport_reg);
    rtd_inl(LDSPI_Data_Rwport_reg);
    rtd_inl(LDSPI_Data_Rwport_reg);
    rtd_inl(LDSPI_Data_Rwport_reg);
    rtd_inl(LDSPI_Data_Rwport_reg);
    rtd_inl(LDSPI_Data_Rwport_reg);
#endif
    rtd_part_outl(LDSPI_LD_CTRL_reg,2,2,0x1);         //Send_follow_Vsync
    //rtd_part_outl(0xb802C900,1,1,0x1);              //Send_trigger
    rtd_part_outl(LDSPI_LD_CTRL_reg,0,0,0x1);         //Ld_spi start enable

#if IS_ENABLED(CONFIG_SUPPORT_SCALER)
    if(pwm_get_disp_refresh_rate() == 60){
        ppoverlay_dv_total_RBUS ppoverlay_dv_total_reg;
        ppoverlay_dv_total_reg.regValue = rtd_inl(PPOVERLAY_DV_total_reg);
        rtd_part_outl(LDSPI_ld_spi_h_total_reg,31,19, pwm_get_disp_htotal());                     //ld_spi_dh_total ,reproduce hsync signal	 0x112f 4k
        if(ppoverlay_dv_total_reg.dv_total != 0){
            rtd_maskl(LDSPI_ld_spi_v_total_reg, 0x7ffff, ((ppoverlay_dv_total_reg.dv_total/2)-1)<<19);  // repeat local dimming vsync
        }else{
            rtd_maskl(LDSPI_ld_spi_v_total_reg, 0x7ffff, ((pwm_get_disp_vtotal()/2)-1)<<19);        // repeat local dimming vsync
        }
        PWM_INFO("vertical_total =%x, 0xb802c96c = %x\n", ppoverlay_dv_total_reg.dv_total, rtd_inl(LDSPI_ld_spi_v_total_reg));
        rtd_part_outl(LDSPI_ld_spi_reproduce_mode_reg,2,0,0x1);                                         //reproduce mode
    }
#endif
    //ld_time_tick = rtd_inl(SCPU_CLK90K_LO_reg);
    rtd_pr_pwm_ld_warn("--- %s.\n", __FUNCTION__ );
}


