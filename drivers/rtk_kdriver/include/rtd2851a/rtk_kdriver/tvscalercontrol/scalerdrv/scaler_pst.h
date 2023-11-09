#ifndef _SCALER_PST_H_
#define _SCALER_PST_H_

#ifdef __cplusplus
extern "C" {
#endif

//#define PST_CMD_NUMBER 9//hw setting + 1
//#define PST_CMD_NUMBER 2//hw setting + 1

typedef struct _PST_STEP_SIZE {
    unsigned int	i_m_blk_size;
    unsigned int	i_s_blk_size;
    unsigned int	dispm_blk_size;
    unsigned int	disps_blk_size;
    unsigned int	gdma_blk_size;
} PST_STEP_SIZE;

typedef struct _PST_CMD_BUF {
    unsigned int	i_m_cur_cmd;
    unsigned int	i_s_cur_cmd;
    unsigned int	dispm_cur_cmd;
    unsigned int	disps_cur_cmd;
    unsigned int	gdma_cur_cmd;
} PST_CMD_BUF;

typedef enum{
	_DEFAULT_OFF = 0,
	_MAIN_MD,
	_MAIN_VO_IMD,
	_SUB_MD,
	_SUB_VO_IMD,
	_PST_MODE_MAX,
}_PST_MODE_T;

typedef struct _PST_DISPM_SCPU_VIR_START_ADDR {
	unsigned long    s0_mdisp_st_vir_addr; //0xb8005000 - 0xb8005288
	unsigned long	s0_uzu_st_vir_addr; //0xb806b900 -  0xb806b9fc
	unsigned long	s0_dtg_st_vir_addr; //0xb806b900 -  0xb806b9fc
	unsigned long	pst_enable_vir_addr;//pst enable flag
	unsigned long	pst_dispm_write_cmd_vir_addr;
	unsigned long	pst_dispm_read_cmd_vir_addr;
	//sub pst
	unsigned long	s1_mdisp_st_vir_addr; //0xb8005000 - 0xb8005288
	unsigned long	s1_uzu_st_vir_addr; //0xb806b900 -  0xb806b9fc
	unsigned long	s1_dtg_st_vir_addr; //0xb806b900 -  0xb806b9fc
	unsigned long	pst_disps_enable_vir_addr;//pst enable flag
	unsigned long	pst_disps_write_cmd_vir_addr;
	unsigned long	pst_disps_read_cmd_vir_addr;
}PST_DISPM_SCPU_VIR_START_ADDR;

typedef struct _PST_IMD_SCPU_VIR_START_ADDR {
	/*i domain part*/
	unsigned long    s0_vo_st_vir_addr; //0xb8005000 - 0xb8005288
	unsigned long    s0_vgip_st_vir_addr;//b8028022200 - b80222e0
	unsigned long	s0_color_mb_peaking_st_vir_addr;
	unsigned long	s0_dexcxl_st_vir_addr; //0xb8023900 - 0xb80239e4
	unsigned long	s0_dfilter_st_vir_addr;
	unsigned long	s0_di0_st_vir_addr;//b8024000-b8024bf4
	unsigned long	s0_di1_st_vir_addr;//b8024000-b8024bf4
	unsigned long	s0_di2_st_vir_addr;//b8024000-b8024bf4
	unsigned long	s0_di3_st_vir_addr;//b8024000-b8024bf4
	unsigned long	s0_di4_st_vir_addr;//b8024000-b8024bf4
	unsigned long	s0_histogram_st_vir_addr;//0xb802_3200  ~ 0xb802_3338
	unsigned long	s0_hsddither_st_vir_addr;//b8022600 -b8022668
	unsigned long	s0_idcti_st_vir_addr;//0xb80231d4- 0xb80231f8
	unsigned long	s0_iedgesmooth_st_vir_addr;//b8025100-b80251ec
	unsigned long	s0_ipq_decontour_st_vir_addr;//0xb8025700 - 0xb802575c
	unsigned long	s0_main_dither_st_vir_addr;
	unsigned long	s0_mpegnr_st_vir_addr;//0xb8023820 -0xb80238fc
	unsigned long	s0_nr0_st_vir_addr;//b8025000-b80250ff
	unsigned long	s0_nr1_st_vir_addr;//b8025500-b80255ff
	unsigned long	s0_peaking_st_vir_addr;
	unsigned long	s0_profile0_st_vir_addr;//0xb802_3400  ~ 0xb802_34FF
	unsigned long	s0_profile1_st_vir_addr;//b8024000-b8024bf4
	unsigned long	s0_rgb2yuv0_st_vir_addr;// 0xb8023000, b8023008~b802301c,0xb8023038
	unsigned long	s0_rgb2yuv1_st_vir_addr;
	unsigned long	s0_ringfilter_st_vir_addr;//-0xb8023580 - 0xb8023590
	unsigned long    s0_uzd0_st_vir_addr;//b8025204-b802537c
	unsigned long    s0_uzd1_st_vir_addr;
	unsigned long    s0_mcap0_st_vir_addr;//0xb8027200 - 0xb802722c;b802726c~b8027280;b80272c8~b80272cc;b80272e0~b80272ec;
	unsigned long    s0_mcap1_st_vir_addr;
	unsigned long    s0_mcap2_st_vir_addr;
	unsigned long	idom_pst_enable_vir_addr;//idomain pst enable flag
	unsigned long	pst_idom_write_cmd_vir_addr;
	unsigned long	pst_idom_read_cmd_vir_addr;
	/*d domain part*/
	unsigned long    s0_mdisp_st_vir_addr; //0xb8005000 - 0xb8005288
	unsigned long	s0_uzu_st_vir_addr; //0xb806b900 -  0xb806b9fc
	unsigned long	s0_dtg_st_vir_addr; //0xb806b900 -  0xb806b9fc
	unsigned long	pst_enable_vir_addr;//ddomain pst enable flag
	unsigned long	pst_dispm_write_cmd_vir_addr;
	unsigned long	pst_dispm_read_cmd_vir_addr;

	/*sub pst*/
	unsigned long    s1_vo_st_vir_addr; //0xb8005000 - 0xb8005288
	unsigned long    s1_vgip_st_vir_addr;//b8028022200 - b80222e0
	unsigned long	s1_color_mb_peaking_st_vir_addr;
	unsigned long	s1_dexcxl_st_vir_addr; //0xb8023900 - 0xb80239e4
	unsigned long	s1_dfilter_st_vir_addr;
	unsigned long	s1_di0_st_vir_addr;//b8024000-b8024bf4
	unsigned long	s1_di1_st_vir_addr;//b8024000-b8024bf4
	unsigned long	s1_di2_st_vir_addr;//b8024000-b8024bf4
	unsigned long	s1_di3_st_vir_addr;//b8024000-b8024bf4
	unsigned long	s1_di4_st_vir_addr;//b8024000-b8024bf4
	unsigned long	s1_histogram_st_vir_addr;//0xb802_3200  ~ 0xb802_3338
	unsigned long	s1_hsddither_st_vir_addr;//b8022600 -b8022668
	unsigned long	s1_idcti_st_vir_addr;//0xb80231d4- 0xb80231f8
	unsigned long	s1_iedgesmooth_st_vir_addr;//b8025100-b80251ec
	unsigned long	s1_ipq_decontour_st_vir_addr;//0xb8025700 - 0xb802575c
	unsigned long	s1_main_dither_st_vir_addr;
	unsigned long	s1_mpegnr_st_vir_addr;//0xb8023820 -0xb80238fc
	unsigned long	s1_nr0_st_vir_addr;//b8025000-b80250ff
	unsigned long	s1_nr1_st_vir_addr;//b8025500-b80255ff
	unsigned long	s1_peaking_st_vir_addr;
	unsigned long	s1_profile0_st_vir_addr;//0xb802_3400  ~ 0xb802_34FF
	unsigned long	s1_profile1_st_vir_addr;//b8024000-b8024bf4
	unsigned long	s1_rgb2yuv0_st_vir_addr;// 0xb8023000, b8023008~b802301c,0xb8023038
	unsigned long	s1_rgb2yuv1_st_vir_addr;
	unsigned long	s1_ringfilter_st_vir_addr;//-0xb8023580 - 0xb8023590
	unsigned long    s1_uzd0_st_vir_addr;//b8025204-b802537c
	unsigned long    s1_uzd1_st_vir_addr;
	unsigned long    s1_mcap0_st_vir_addr;//0xb8027200 - 0xb802722c;b802726c~b8027280;b80272c8~b80272cc;b80272e0~b80272ec;
	unsigned long    s1_mcap1_st_vir_addr;
	unsigned long    s1_mcap2_st_vir_addr;
	unsigned long	idos_pst_enable_vir_addr;//idomain pst enable flag
	unsigned long	pst_idos_write_cmd_vir_addr;
	unsigned long	pst_idos_read_cmd_vir_addr;
	/*d domain part*/
	unsigned long    s1_mdisp_st_vir_addr; //0xb8005000 - 0xb8005288
	unsigned long	s1_uzu_st_vir_addr; //0xb806b900 -  0xb806b9fc
	unsigned long	s1_dtg_st_vir_addr; //0xb806b900 -  0xb806b9fc
	unsigned long	pst_sub_enable_vir_addr;//ddomain pst enable flag
	unsigned long	pst_disps_write_cmd_vir_addr;
	unsigned long	pst_disps_read_cmd_vir_addr;
}PST_IMD_SCPU_VIR_START_ADDR;


typedef struct {
	UINT32 PST_I2RND_TOP_CTRL_S_R;
	UINT32 PST_I2RND_DMA_CTRL_S_R;
	UINT32 PST_I2RND_DMA_NUM0_S_R;
	UINT32 PST_I2RND_DMA_NUM1_S_R;
	UINT32 PST_I2RND_DMA_NUM2_S_R;
	UINT32 PST_I2RND_DMA_NUM3_S_R;
	UINT32 PST_I2RND_DMA_NUM4_S_R;
	UINT32 PST_I2RND_DMA_ADDR0_S_R;
	UINT32 PST_I2RND_DMA_ADDR2_S_R;
	UINT32 PST_I2RND_DMA_ADDR4_S_R;
	UINT32 PST_I2RND_DMA_ADDR5_S_R;
	UINT32 PST_I2RND_DMA_ADDR6_S_R;
	UINT32 PST_I2RND_DMA_ADDR7_S_R;
	UINT32 PST_I2RND_DMA_ADDR8_S_R;
	UINT32 PST_I2RND_DMA_STEP0_S_R;
	UINT32 PST_I2RND_DMA_STEP2_S_R;
	UINT32 PST_I2RND_DMA_STEP4_S_R;
	UINT32 PST_I2RND_DMA_STEP5_S_R;
	UINT32 PST_I2RND_DMA_STEP6_S_R;
	UINT32 PST_I2RND_DMA_STEP7_S_R;
	UINT32 PST_I2RND_DMA_STEP8_S_R;
	UINT32 PST_I2RND_DMA_BUF_NUM_S_R;
	UINT32 PST_I2RND_DMA_WRT_PNT0_S_R;
	UINT32 PST_I2RND_DMA_WRT_PNT1_S_R;
	UINT32 PST_I2RND_DMA_WRT_PNT2_S_R;
	UINT32 PST_I2RND_DMA_READ_PNT0_S_R;
	UINT32 PST_I2RND_DMA_READ_PNT1_S_R;
	UINT32 PST_I2RND_DMA_READ_PNT2_S_R;
	UINT32 PST_I2RND_DEBUG0_S_R;
	UINT32 PST_I2RND_DEBUG1_S_R;
	UINT32 PST_I2RND_DISPM_REPEAT_CTRL_S_R;
	UINT32 PST_I2RND_DISPM_REPEAT_DEBUG_S_R;
	UINT32 PST_I2RND_PST_MBIST_MODE_S_R;
	UINT32 PST_I2RND_PST_MBIST_DONE_S_R;
	UINT32 PST_I2RND_PST_MBIST_FAIL_S_R;
	UINT32 PST_I2RND_PST_DRF_MODE_S_R;
	UINT32 PST_I2RND_PST_DRF_RESUME_S_R;
	UINT32 PST_I2RND_PST_DRF_DONE_S_R;
	UINT32 PST_I2RND_PST_DRF_PAUSE_S_R;
	UINT32 PST_I2RND_PST_MBIST_DRF_FAIL_S_R;
	UINT32 PST_I2RND_PST_MBIST_LS_S_R;
	UINT32 PST_I2RND_PST_MBIST_RM_S_R;
	UINT32 PST_I2RND_PST_MBIST_RME_S_R;
}SCALER_PST_SUSPEND_RESUME_T;

unsigned char Scaler_main_md_pst_get_enable(void);

void Scaler_main_md_pst_set_enable(unsigned char enable);
void drvif_scaler_pst_initial(_PST_MODE_T mode);
void drvif_scaler_pst_uninitial(_PST_MODE_T mode);
void drvif_scaler_pst_dma_setting(unsigned int cmd_num, unsigned int blk_size, unsigned int idom_blk_size,_PST_MODE_T mode);
void drvif_scaler_pst_top_ctrl(bool enable, _PST_MODE_T mode);
void drvif_scaler_pst_set_mode(_PST_MODE_T  mode);
unsigned char  drvif_scaler_pst_get_mode_enable(_PST_MODE_T  mode);
unsigned int Scaler_PST_IP_register_get_cur_write_addr(unsigned int addr,unsigned char display);
unsigned char	Scaler_pst_setbuffer(_PST_MODE_T  mode);
void Scaler_pst_set_write_point(_PST_MODE_T  mode);
unsigned char Scaler_pst_get_write_point(_PST_MODE_T  mode);
unsigned char Scaler_pst_get_read_point(_PST_MODE_T mode);
void Scaler_pst_buffer_copy(_PST_MODE_T mode);
void Scaler_pst_main_md_set_write_cmd_num(void);
void Scaler_pst_main_md_write_default_cmd_num(void);
void Scaler_pst_main_md_set_read_cmd_num(unsigned char enable);
void Scaler_main_imd_pst_set_enable(unsigned char enable);

unsigned int Scaler_PST_IP_register_get_cur_read_addr(unsigned int addr,unsigned char display);
void Scaler_pst_buffer_initial(_PST_MODE_T mode);
unsigned char Scaler_sub_md_pst_get_enable(void);
void Scaler_sub_md_pst_set_enable(unsigned char enable);
unsigned char Scaler_pst_sub_md_get_read_cmd_num(void);
void Scaler_pst_sub_md_set_read_cmd_num(unsigned char enable);
unsigned char Scaler_pst_sub_md_get_write_cmd_num(void);
void Scaler_pst_sub_md_set_write_cmd_num(void);
void Scaler_pst_sub_md_write_default_cmd_num(void);
unsigned int Scaler_s0_pst_cmd_buffer_write_offset_addr(unsigned int addr);
unsigned int Scaler_s0_pst_cmd_buffer_read_offset_addr(unsigned int addr);
unsigned int Scaler_s1_pst_cmd_buffer_write_offset_addr(unsigned int addr);
unsigned int Scaler_s1_pst_cmd_buffer_read_offset_addr(unsigned int addr);
void scaler_imd_pst_enable_interrupt(void);
void scaler_pst_suspend(void);
void scaler_pst_resume(void);
unsigned char check_pst_active_state(unsigned char display);
void reset_pst_sharemem_buffer(unsigned char display);
void set_pst_active_state(unsigned char display, unsigned short state);

#ifdef __cplusplus
}
#endif

#endif // #ifndef _SCALER_H_
/*======================== End of File =======================================*/
/**
*
* @}
*/
