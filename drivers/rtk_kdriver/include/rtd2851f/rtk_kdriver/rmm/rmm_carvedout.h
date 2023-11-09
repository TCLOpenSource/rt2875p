#ifndef _RMM_CARVEDOUT_H_
#define _RMM_CARVEDOUT_H_

/*
 *  before all carvedout migrate to devicetree
 *  both mm & dts only refer devicetree for reserved-memory if it's declared here.
 *
 *  native -> rtk_platform.h -> rmm.h (rmm_carvedout.h, this)
 *  rmm & dts -> rmm.h (rmm_carvedout.h, this)
 *
 *  TODO: it could be phased-out once all carvedout migrated to devcietree and GKI compliant.
 */

#ifndef BUILD_QUICK_SHOW
#define RTK_MEM_LAYOUT_DEVICETREE // layout on devicetree or legacy builtin
#endif


/*
 *  fixed index defines of carvedout_buf[] origin from rmm.h enum
 *  referred by both kernel driver and devicetree sources
 *
 */

    /* address 512MB before */
#define CARVEDOUT_BOOTCODE          0
#define CARVEDOUT_DEMOD             1
#define CARVEDOUT_AV_DMEM           2
#define CARVEDOUT_VDEC_COMEM        CARVEDOUT_AV_DMEM

#define CARVEDOUT_AUDIO             3

#define CARVEDOUT_K_BOOT            4
#define CARVEDOUT_K_OS              5

#define CARVEDOUT_MAP_GIC           6
#define CARVEDOUT_MAP_RBUS          7
#define CARVEDOUT_GPU_FW            8   //deprecated?

#define CARVEDOUT_V_OS              9
#define CARVEDOUT_MAP_RPC           10

#define CARVEDOUT_A_OS              11
#define CARVEDOUT_LOGBUF            12
#define CARVEDOUT_ROMCODE           13
#define CARVEDOUT_RAMOOPS           14

#define CARVEDOUT_IR_TABLE          15
#define CARVEDOUT_DDR_BOUNDARY      16 //deprecated
#define CARVEDOUT_DDR_BOUNDARY_2    17 //deprecated

    /* device */
#define CARVEDOUT_VDEC_RINGBUF      18

#define CARVEDOUT_GAL				19
#define CARVEDOUT_SNAPSHOT			20

#define CARVEDOUT_SCALER            21 //deprecated
#define CARVEDOUT_SCALER_MEMC       22
#define CARVEDOUT_SCALER_MDOMAIN    23
#define CARVEDOUT_SCALER_DI_NR      24
#define CARVEDOUT_SCALER_NN         25
#define CARVEDOUT_SCALER_VIP        26
#define CARVEDOUT_SCALER_OD         27
#define CARVEDOUT_VDEC_VBM          28 //deprecated
#define CARVEDOUT_TP                29

    /* for desired cma size calculation */
#define CARVEDOUT_CMA_LOW           30 //deprecated
#define CARVEDOUT_CMA_HIGH          31 //deprecated
#define CARVEDOUT_CMA_GPU_4K        32 //if UI_RESOLUTION_4K
#define CARVEDOUT_CMA_3             33 //deprecated
#define CARVEDOUT_CMA_LOW_LIMIT     34 //deprecated
#define CARVEDOUT_CMA_BW            35 //deprecated

#define CARVEDOUT_CMA_VBM           36

#define CARVEDOUT_GPU_RESERVED      37

    /* array end number */
#define CARVEDOUT_NUM               38


    /* for reserved size in highmem */
    //CARVEDOUT_HIGH_START = CARVEDOUT_SCALER
    //CARVEDOUT_HIGH_END = CARVEDOUT_TP


#endif // _RMM_CARVEDOUT_H_

