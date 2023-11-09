#ifndef __RTK_ICE_DMA_DEFINE_H__
#define __RTK_ICE_DMA_DEFINE_H__

#include <rtk_kdriver/io.h>
#include "../core/rtk_ice.h"
#include "rtk_ice-dma.h"
#include <rbus/di_reg.h>
#include <rbus/iedge_smooth_reg.h>
#include <rbus/vdpq_reg.h>
#include <rbus/od_dma_reg.h>
#include <rbus/audio_reg.h>
#include <rbus/demura_reg.h>
#include <rbus/mdomain_cap_reg.h>
#include <rbus/mdomain_disp_reg.h>
#include <rbus/h3ddma_reg.h>
#include <rbus/vodma_reg.h>
#include <rbus/vodma2_reg.h>
#include <rbus/vbi_reg.h>
//#include <rbus/tve_dma_reg.h>
/*******************************ip's object************************************/
////////////////////////////////tvsb2///////////////////////////////////////////
/*diw*/
unsigned int update_tvsb2_diw_date_overflow(void){
    return (rtd_inl(DI_DDR_DMAStatus_reg) >> 15) & 0x1;
}
unsigned int update_tvsb2_diw_info_overflow (void ){
    return (rtd_inl(DI_DDR_DMAStatus_reg) >> 10) & 0x1;
}
struct CFIFODataInfo tvsb2_diw_info_overflow = {DMA_STATUS_OVERFLOW_1,0,\
    NULL,false,update_tvsb2_diw_info_overflow};
struct CFIFODataInfo tvsb2_diw_date_overflow = {DMA_STATUS_OVERFLOW_0,0,\
    &tvsb2_diw_info_overflow,false,update_tvsb2_diw_date_overflow};
/*dir*/
unsigned int update_tvsb2_dir_date_overflow(void){
    return (rtd_inl(DI_DDR_DMAStatus_reg) >> 11) & 0xf;
}
unsigned int update_tvsb2_dir_info_overflow (void ){
    return (rtd_inl(DI_DDR_DMAStatus_reg) >> 8) & 0x3;
}
static struct CFIFODataInfo tvsb2_dir_info_overflow = {DMA_STATUS_OVERFLOW_1,0,\
    NULL,false,update_tvsb2_dir_info_overflow};
static struct CFIFODataInfo tvsb2_dir_date_overflow = {DMA_STATUS_OVERFLOW_0,0,\
    &tvsb2_dir_info_overflow,false,update_tvsb2_dir_date_overflow};
/*m-cap (w)*/
unsigned int update_tvsb2_m_cap_w_data_overflow(void){
    unsigned int value = rtd_inl(MDOMAIN_CAP_Cap_DDR_FIFOStatus_reg);
    return MDOMAIN_CAP_Cap_DDR_FIFOStatus_get_in1_fifoof(value);
}
unsigned int update_tvsb2_m_cap_w_data_underflow(void){
    unsigned int value = rtd_inl(MDOMAIN_CAP_Cap_DDR_FIFOStatus_reg);
    return MDOMAIN_CAP_Cap_DDR_FIFOStatus_get_in1_fifouf(value);
}
static struct CFIFODataInfo tvsb2_m_cap_w_data_underflow = {DMA_STATUS_UNDERFLOW,0,\
    NULL,false,update_tvsb2_m_cap_w_data_underflow};
static struct CFIFODataInfo tvsb2_m_cap_w_data_overflow = {DMA_STATUS_OVERFLOW_0,0,\
    &tvsb2_m_cap_w_data_underflow,false,update_tvsb2_m_cap_w_data_overflow};
/*m-disp (r)*/
unsigned int update_tvsb2_m_disp_r_data_overflow(void){
    unsigned int value = rtd_inl(MDOMAIN_DISP_Disp_DDR_FIFOStatus_reg);
    return MDOMAIN_DISP_Disp_DDR_FIFOStatus_get_main_fifoof(value);
}
unsigned int update_tvsb2_m_disp_r_data_underflow(void){
    unsigned int value = rtd_inl(MDOMAIN_DISP_Disp_DDR_FIFOStatus_reg);
    return MDOMAIN_DISP_Disp_DDR_FIFOStatus_get_main_fifouf(value);
}
static struct CFIFODataInfo tvsb2_m_disp_r_data_underflow = {DMA_STATUS_UNDERFLOW,0,\
    NULL,false,update_tvsb2_m_disp_r_data_underflow};
static struct CFIFODataInfo tvsb2_m_disp_r_data_overflow = {DMA_STATUS_OVERFLOW_0,0,\
    &tvsb2_m_disp_r_data_underflow,false,update_tvsb2_m_disp_r_data_overflow};
/*s-cap (w)*/
unsigned int update_tvsb2_s_cap_w_data_overflow(void){
    unsigned int value = rtd_inl(MDOMAIN_CAP_Cap_DDR_FIFOStatus_reg);
    return MDOMAIN_CAP_Cap_DDR_FIFOStatus_get_in2_fifoof(value);
}
unsigned int update_tvsb2_s_cap_w_data_underflow(void){
    unsigned int value = rtd_inl(MDOMAIN_CAP_Cap_DDR_FIFOStatus_reg);
    return MDOMAIN_CAP_Cap_DDR_FIFOStatus_get_in2_fifouf(value);
}
static struct CFIFODataInfo tvsb2_s_cap_w_data_underflow = {DMA_STATUS_UNDERFLOW,0,\
    NULL,false,update_tvsb2_s_cap_w_data_underflow};
static struct CFIFODataInfo tvsb2_s_cap_w_data_overflow = {DMA_STATUS_OVERFLOW_0,0,\
    &tvsb2_s_cap_w_data_underflow,false,update_tvsb2_s_cap_w_data_overflow};
/*s-disp (r)*/
unsigned int update_tvsb2_s_disp_r_data_overflow(void){
    unsigned int value = rtd_inl(MDOMAIN_DISP_Disp_DDR_FIFOStatus_reg);
    return MDOMAIN_DISP_Disp_DDR_FIFOStatus_get_sub_fifoof(value);
}
unsigned int update_tvsb2_s_disp_r_data_underflow(void){
    unsigned int value = rtd_inl(MDOMAIN_DISP_Disp_DDR_FIFOStatus_reg);
    return MDOMAIN_DISP_Disp_DDR_FIFOStatus_get_sub_fifouf(value);
}
static struct CFIFODataInfo tvsb2_s_disp_r_data_underflow = {DMA_STATUS_UNDERFLOW,0,\
    NULL,false,update_tvsb2_s_disp_r_data_underflow};
static struct CFIFODataInfo tvsb2_s_disp_r_data_overflow = {DMA_STATUS_OVERFLOW_0,0,\
    &tvsb2_s_disp_r_data_underflow,false,update_tvsb2_s_disp_r_data_overflow};
/*vo1_y (r) ,vo1 c*/
unsigned int update_tvsb2_vo1_y_underflow(void){
    unsigned int val = rtd_inl(VODMA_VODMA_VGIP_INTPOS_reg);
    return VODMA_VODMA_VGIP_INTPOS_get_buf_underflow_inte1(val);
}
static struct CFIFODataInfo tvsb2_vo1_y_underflow = {DMA_STATUS_UNDERFLOW,0,\
    NULL,false,update_tvsb2_vo1_y_underflow};
/*vo2 y,vo2 c*/
unsigned int update_tvsb2_vo2_y_underflow(void){
    unsigned int val = rtd_inl(VODMA2_VODMA2_VGIP_INTPOS_reg);
    return VODMA2_VODMA2_VGIP_INTPOS_get_buf_underflow_inte1(val);
}
static struct CFIFODataInfo tvsb2_vo2_y_underflow = {DMA_STATUS_UNDERFLOW,0,\
    NULL,false,update_tvsb2_vo2_y_underflow};
/*i3ddma*/
unsigned int update_tvsb2_i3ddma_overflow(void){
    unsigned int value = rtd_inl(H3DDMA_CAP0_Cap_Status_reg);
    return H3DDMA_CAP0_Cap_Status_get_cap0_cap_fifo_overflow(value);
}
static struct CFIFODataInfo tvsb2_i3ddma_overflow = {DMA_STATUS_OVERFLOW_0,0,\
    NULL,false,update_tvsb2_i3ddma_overflow};
/*snr(r)*/
unsigned int update_tvsb2_snr_r_underflow(void){
    unsigned int value = rtd_inl(IEDGE_SMOOTH_MVF_DMA_Err_Status_reg);
    return IEDGE_SMOOTH_MVF_DMA_Err_Status_get_dma_fifo_unfl(value);
}
static struct CFIFODataInfo tvsb2_snr_r_underflow = {DMA_STATUS_UNDERFLOW,0,\
    NULL,false,update_tvsb2_snr_r_underflow};
////////////////////////////////tvsb1///////////////////////////////////////////
/*vd*/
unsigned int update_tvsb1_vd_over_under_flow(void){
    return (rtd_inl(VDPQ_DMA_CTRL5_reg) >>3) & 0x1f;
}
static struct CFIFODataInfo tvsb1_vd_over_under_flow = {DMA_STATUS_OVER_UNDER_FLOW,0,\
    NULL,false,update_tvsb1_vd_over_under_flow};
/*audio*/
unsigned int update_tvsb1_audio_overflow(void){
	unsigned int value = rtd_inl(AUDIO_ADMA_CSR_pbw_reg);
    return AUDIO_ADMA_CSR_pbw_get_overflow_dma_status(value);
}
static struct CFIFODataInfo tvsb1_audio_overflow = {DMA_STATUS_OVERFLOW_0,0,\
    NULL,false,update_tvsb1_audio_overflow};
/*audio2*/
unsigned int update_tvsb1_audio2_overflow_1(void){
	unsigned int value = rtd_inl(AUDIO_ADMA_CSR_arcw_reg);
    return AUDIO_ADMA_CSR_arcw_get_overflow_dma_status(value);
}
static struct CFIFODataInfo tvsb1_audio2_overflow_1 = {DMA_STATUS_OVERFLOW_1,0,\
    NULL,false,update_tvsb1_audio2_overflow_1};
/*VBI*/
unsigned int update_tvsb1_vbi_over_under_flow(void){
    return ((rtd_inl(VBI_VBI_DMA_CTRL_reg) >> 3) & 0x1);
}
static struct CFIFODataInfo tvsb1_vbi_over_unnder_flow = {DMA_STATUS_OVER_UNDER_FLOW,0,\
    NULL,false,update_tvsb1_vbi_over_under_flow};
#if 0
/*TVE-VD*/
unsigned int update_tvsb1_tve_vd_overflow(void){
    return ((rtd_inl(TVE_DMA_TVEDMA_FIFOSTATUS_reg) >> 1) & 0x1);
}
static struct CFIFODataInfo tvsb1_tve_vd_overflow = {DMA_STATUS_OVERFLOW_0,0,\
    NULL,false,update_tvsb1_tve_vd_overflow};
/*TVE-VBI*/
unsigned int update_tvsb1_tve_vbi_overflow(void){
    return ((rtd_inl(TVE_DMA_TVEDMA_FIFOSTATUS_reg) >> 5) & 0x1);
}
static struct CFIFODataInfo tvsb1_tve_vbi_overflow = {DMA_STATUS_OVERFLOW_0,0,\
    NULL,false,update_tvsb1_tve_vbi_overflow};
#endif
////////////////////////////////sb1///////////////////////////////////////////
/*EMMC*/
unsigned int update_sb1_emmc_overflow(void){
    return ((rtd_inl(0xb8010948) >>2) & 0x3);
}
static struct CFIFODataInfo sb1_emmc_over_under_flow = {DMA_STATUS_OVER_UNDER_FLOW,0,\
    NULL,false,update_sb1_emmc_overflow};
////////////////////////////////sb3///////////////////////////////////////////

////////////////////////////////tvsb3///////////////////////////////////////////

////////////////////////////////tvsb5///////////////////////////////////////////
/*PST*/
unsigned int update_tvsb5_pst_underflow(void){
    return ((rtd_inl(0xb80b10a0) >>8) & 0x1);
}
static struct CFIFODataInfo tvsb5_pst_underflow = {DMA_STATUS_UNDERFLOW,0,\
    NULL,false,update_tvsb5_pst_underflow};
////////////////////////////////tvsb4///////////////////////////////////////////
/*OD(W)*/
unsigned int update_tvsb4_odw_overflow_0(void){
    return ((rtd_inl(OD_DMA_ODDMA_WR_Ctrl_reg)>>2) & 0x1);
}
unsigned int update_tvsb4_odw_overflow_1(void){
    unsigned int value = rtd_inl(OD_DMA_ODDMA_WR_status_reg);
    return OD_DMA_ODDMA_WR_status_get_cap_data_fifo_underflow_err(value);
}
static struct CFIFODataInfo tvsb4_odw_overflow_1 = {DMA_STATUS_OVERFLOW_1,0,\
    NULL,false,update_tvsb4_odw_overflow_1};
static struct CFIFODataInfo tvsb4_odw_overflow_0 = {DMA_STATUS_OVERFLOW_0,0,\
    &tvsb4_odw_overflow_1,false,update_tvsb4_odw_overflow_0};
/*OD(R)*/
unsigned int update_tvsb4_odr_underflow_0(void){
    return ((rtd_inl(OD_DMA_ODDMA_RD_num_bl_wrap_st2_reg)>>2) & 0x1);
}
unsigned int update_tvsb4_odr_underflow_1(void)
{
    return ((rtd_inl(0xb802cef0) >>2) & 0x1);
}
static struct CFIFODataInfo tvsb4_odr_underflow_1 = {DMA_STATUS_UNDERFLOW_1,0,\
    NULL,false,update_tvsb4_odr_underflow_1};
static struct CFIFODataInfo tvsb4_odr_underflow_0 = {DMA_STATUS_UNDERFLOW,0,\
    &tvsb4_odr_underflow_1,false,update_tvsb4_odr_underflow_0};
/*DC2H*/
unsigned int update_tvsb4_dc2h_over_under_flow(void)
{
    return ((rtd_inl(0xb8029d24) >>4) & 0x7);
}
static struct CFIFODataInfo  tvsb4_dc2h_over_underflow= {DMA_STATUS_OVER_UNDER_FLOW,0,\
    NULL,false,update_tvsb4_dc2h_over_under_flow};
/*subtitle*/
unsigned int update_tvsb4_subtitle_underflow(void)
{
    return ((rtd_inl(0xb802f0d8) >>14) & 0x1);
}
static struct CFIFODataInfo tvsb4_subtitle_underflow = {DMA_STATUS_UNDERFLOW,0,\
    NULL,false,update_tvsb4_subtitle_underflow};
/*osd1-5*/
unsigned int update_tvsb4_osd_1_3_underflow(void){
    return ((rtd_inl(0xb802fc10) >>1) & 0x3f);
}
static struct CFIFODataInfo tvsb4_osd_1_3_underflow = {DMA_STATUS_UNDERFLOW,0,\
    NULL,false,update_tvsb4_osd_1_3_underflow};
/*demura*/
unsigned int update_tvsb4_demura_underflow(void){
    unsigned int value = rtd_inl(DEMURA_DEMURA_DMA_WRAP_WORD_reg);
    return value & 0x1;
}
static struct CFIFODataInfo tvsb4_demura_underflow = {DMA_STATUS_UNDERFLOW,0,\
    NULL,false,update_tvsb4_demura_underflow};
#endif
