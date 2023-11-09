
/* ----- ----- ----- ----- ----- HEADER ----- ----- ----- ----- ----- */
#ifndef BUILD_QUICK_SHOW
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/string.h>
#include <linux/moduleparam.h>
#include <linux/types.h>
#include <linux/export.h>
#include <linux/semaphore.h>
#include <mach/system.h>
#else
#include <no_os/printk.h>
#include <no_os/slab.h>
#include <include/string.h>
#endif
#include <rbus/iedge_smooth_reg.h>
#include <rtk_kdriver/io.h>
#include <tvscalercontrol/vip/cbrev.h>
/* ----- ----- ----- ----- ----- VARIABLES ----- ----- ----- ----- ----- */

enum
{
	eCBREV_LEVEL_off,
	eCBREV_LEVEL_weak,
	eCBREV_LEVEL_standard,
	eCBREV_LEVEL_flat_3_3_0_0,
	eCBREV_LEVEL_flat_3_3_1_1,
	eCBREV_LEVEL__count
};

VIP_CBREV_LEVEL_TABLE gVipCbrevTable[eCBREV_LEVEL__count]={
	{
		{ 0 , 1 , 6 , 4 , 4 , 4 , 1 , 0 , 0 },
		{ 500 , 200 , 1023 , 0 },
		{ 0 , 2 , 10 , 1 , 4 , 0 },
	},
	{
		{ 1 , 1 , 6 , 4 , 4 , 4 , 1 , 0 , 8 },
		{ 500 , 200 , 1023 , 0 },
		{ 0 , 2 , 10 , 1 , 4 , 0 },
	},
	{
		{ 1 , 1 , 6 , 4 , 4 , 4 , 1 , 0 , 0 },
		{ 500 , 200 , 1023 , 0 },
		{ 0 , 2 , 10 , 1 , 4 , 0 },
	},
	{
		{ 1 , 1 , 6 , 4 , 4 , 4 , 1 , 0 , 0 },
		{ 500 , 200 , 1023 , 0 },
		{ 1 , 2 , 10 , 1 , 4 , 0 },
	},
	{
		{ 1 , 1 , 6 , 4 , 4 , 4 , 1 , 0 , 0 },
		{ 500 , 200 , 1023 , 0 },
		{ 2 , 2 , 10 , 1 , 4 , 0 },
	},
};

/* ----- ----- ----- ----- ----- FUNCTIONS ----- ----- ----- ----- ----- */

void VPQ_Cbrev_RegCtrl_SetCbrev_reg_db_apply(void)
{
	unsigned int z_RBUS_reg;
	z_RBUS_reg = rtd_inl( IEDGE_SMOOTH_EDSM_DB_CTRL_reg );
	z_RBUS_reg = ( z_RBUS_reg & ~IEDGE_SMOOTH_EDSM_DB_CTRL_edsm_db_apply_mask) | (IEDGE_SMOOTH_EDSM_DB_CTRL_edsm_db_apply( 1 ) );
	rtd_outl( IEDGE_SMOOTH_EDSM_DB_CTRL_reg , z_RBUS_reg );
}

void VPQ_Cbrev_RegCtrl_SetCbrev_reg_table(VIP_CBREV_LEVEL_TABLE* x)
{
	// RTK2851F add
	#if defined(CONFIG_ARCH_RTK2851F)
	unsigned int z_RBUS_reg;
	
	if( !x )
	{
		return;
	}
	
	z_RBUS_reg = rtd_inl( IEDGE_SMOOTH_CBCR_REV_CTRL0_reg );
	z_RBUS_reg = ( z_RBUS_reg & ~IEDGE_SMOOTH_CBCR_REV_CTRL0_cbrev_enable_mask) | (IEDGE_SMOOTH_CBCR_REV_CTRL0_cbrev_enable( x->row0.cbrev_enable ) );
	z_RBUS_reg = ( z_RBUS_reg & ~IEDGE_SMOOTH_CBCR_REV_CTRL0_cbrev_lr_biu_y_mask) | (IEDGE_SMOOTH_CBCR_REV_CTRL0_cbrev_lr_biu_y( x->row0.cbrev_lr_biu_y ) );
	z_RBUS_reg = ( z_RBUS_reg & ~IEDGE_SMOOTH_CBCR_REV_CTRL0_cbrev_mode_mask) | (IEDGE_SMOOTH_CBCR_REV_CTRL0_cbrev_mode( x->row0.cbrev_mode ) );
	z_RBUS_reg = ( z_RBUS_reg & ~IEDGE_SMOOTH_CBCR_REV_CTRL0_cbrev_lr_bit_y_mask) | (IEDGE_SMOOTH_CBCR_REV_CTRL0_cbrev_lr_bit_y( x->row0.cbrev_lr_bit_y ) );
	z_RBUS_reg = ( z_RBUS_reg & ~IEDGE_SMOOTH_CBCR_REV_CTRL0_cbrev_cccoring_mask) | (IEDGE_SMOOTH_CBCR_REV_CTRL0_cbrev_cccoring( x->row0.cbrev_cccoring ) );
	z_RBUS_reg = ( z_RBUS_reg & ~IEDGE_SMOOTH_CBCR_REV_CTRL0_cbrev_gain_mask) | (IEDGE_SMOOTH_CBCR_REV_CTRL0_cbrev_gain( x->row0.cbrev_gain ) );
	z_RBUS_reg = ( z_RBUS_reg & ~IEDGE_SMOOTH_CBCR_REV_CTRL0_cbrev_localmax_method_mask) | (IEDGE_SMOOTH_CBCR_REV_CTRL0_cbrev_localmax_method( x->row0.cbrev_localmax_method ) );
	z_RBUS_reg = ( z_RBUS_reg & ~IEDGE_SMOOTH_CBCR_REV_CTRL0_cbrev_shape_adj_f5_gain_mask) | (IEDGE_SMOOTH_CBCR_REV_CTRL0_cbrev_shape_adj_f5_gain( x->row0.cbrev_shape_adj_F5_gain ) );
	z_RBUS_reg = ( z_RBUS_reg & ~IEDGE_SMOOTH_CBCR_REV_CTRL0_cbrev_gain_neg_mask) | (IEDGE_SMOOTH_CBCR_REV_CTRL0_cbrev_gain_neg( x->row0.cbrev_gain_neg ) );
	z_RBUS_reg = ( z_RBUS_reg & ~IEDGE_SMOOTH_CBCR_REV_CTRL0_cbrev_weak_mask) | (IEDGE_SMOOTH_CBCR_REV_CTRL0_cbrev_weak( x->row0.cbrev_weak ) );
	rtd_outl( IEDGE_SMOOTH_CBCR_REV_CTRL0_reg , z_RBUS_reg );

	z_RBUS_reg = rtd_inl( IEDGE_SMOOTH_CBCR_REV_CTRL1_reg );
	z_RBUS_reg = ( z_RBUS_reg & ~IEDGE_SMOOTH_CBCR_REV_CTRL1_cbrev_force_max_y_ub_mask) | (IEDGE_SMOOTH_CBCR_REV_CTRL1_cbrev_force_max_y_ub( x->row1.cbrev_force_max_y_ub ) );
	z_RBUS_reg = ( z_RBUS_reg & ~IEDGE_SMOOTH_CBCR_REV_CTRL1_cbrev_force_max_y_lb_mask) | (IEDGE_SMOOTH_CBCR_REV_CTRL1_cbrev_force_max_y_lb( x->row1.cbrev_force_max_y_lb ) );
	rtd_outl( IEDGE_SMOOTH_CBCR_REV_CTRL1_reg , z_RBUS_reg );

	z_RBUS_reg = rtd_inl( IEDGE_SMOOTH_CBCR_REV_CTRL2_reg );
	z_RBUS_reg = ( z_RBUS_reg & ~IEDGE_SMOOTH_CBCR_REV_CTRL2_cbrev_adapt_y_ub_mask) | (IEDGE_SMOOTH_CBCR_REV_CTRL2_cbrev_adapt_y_ub( x->row1.cbrev_adapt_y_ub ) );
	z_RBUS_reg = ( z_RBUS_reg & ~IEDGE_SMOOTH_CBCR_REV_CTRL2_cbrev_adapt_y_lb_mask) | (IEDGE_SMOOTH_CBCR_REV_CTRL2_cbrev_adapt_y_lb( x->row1.cbrev_adapt_y_lb ) );
	rtd_outl( IEDGE_SMOOTH_CBCR_REV_CTRL2_reg , z_RBUS_reg );
	#endif
}

void VPQ_Cbrev_Lib_SetCbrev_PqLevel(unsigned char gain)
{
	if( gain>=eCBREV_LEVEL__count )
	{
		return;
	}
	VPQ_Cbrev_RegCtrl_SetCbrev_reg_table( gVipCbrevTable+gain );
}

void VPQ_Cbrev_Lib_SetCbrev_Default(void)
{
	VPQ_Cbrev_Lib_SetCbrev_PqLevel( eCBREV_LEVEL_standard );
}
