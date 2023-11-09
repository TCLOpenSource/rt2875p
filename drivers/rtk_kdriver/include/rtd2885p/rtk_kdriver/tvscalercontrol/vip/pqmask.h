#ifndef __PQMASK_H__
#define __PQMASK_H__

#if 0
/* data type */
#endif

enum {
	/* 0 */ PQMASK_DOMAIN_I = 0,
	/* 1 */ PQMASK_DOMAIN_D,
	/* 2 */ PQMASK_DOMAIN_SRNN,
	/* 3 */ PQMASK_DOMAIN_MAX,
};

enum {
	/* 0 */ PQMASK_BLKMOD_SPNR = 0,			// contains SNR, DECONT pqmask weight plane
	/* 1 */ PQMASK_BLKMOD_SHARPNESS,		// contains EDGE, TEXT pqmask weight plane
	/* 2 */ PQMASK_BLKMOD_ICM,				// contains HUE, SAT, INT pqmask weight plane
	/* 3 */ PQMASK_BLKMOD_LOCALCONTRAST,	// contains LC pqmask weight plane
	/* 4 */ PQMASK_BLKMOD_SRNN_IN,			// contains SRNN_IN0, SRNN_IN1, SRNN_IN2 pqmask weight plane
	/* 5 */ PQMASK_BLKMOD_SRNN_OUT,			// contains SRNN_OUT0 pqmask weight plane
	/* 6 */ PQMASK_BLKMOD_MAX,
};

enum {
	/* 0 */  PQMASK_I_SNR = 0,
	/* 1 */  PQMASK_I_DECONT,
	/* 2 */  PQMASK_D_SHP_EDGE,
	/* 3 */  PQMASK_D_SHP_TEXT,
	/* 4 */  PQMASK_D_ICM_0, // HUE
	/* 5 */  PQMASK_D_ICM_1, // SAT
	/* 6 */  PQMASK_D_ICM_2, // I
	/* 7 */  PQMASK_D_LC,
	/* 8 */  PQMASK_SRNN_IN_0,
	/* 9 */  PQMASK_SRNN_IN_1,
	/* 10 */ PQMASK_SRNN_IN_2,
	/* 11 */ PQMASK_SRNN_OUT_0,
	/* 12 */ PQMASK_MODULE_MAX,
	/* 13 */ PQMASK_D_DCC, // LC share pqmask with DCC
};

typedef struct PQMASK_SCALING_UNIT
{
	unsigned int InitPhase;
	unsigned int Factor;
} PQMASK_SCALING_UNIT_T;

typedef struct PQMASK_SACLING
{
	PQMASK_SCALING_UNIT_T Hor;
	PQMASK_SCALING_UNIT_T Ver;
} PQMASK_SACLING_T;

typedef struct PQMASK_IMG_SIZE
{
	unsigned int HorSta;
	unsigned int Width;
	unsigned int VerSta;
	unsigned int Height;
} PQMASK_IMG_SIZE_T;

typedef struct PQMASK_IO_SIZE
{
	PQMASK_IMG_SIZE_T Input;
	PQMASK_IMG_SIZE_T Output;
} PQMASK_IO_SIZE_T;

typedef union PQMASK_DM_WINDOW_WEIGHT
{
	unsigned int Weight[3];
	struct {
		unsigned int SNRWeight;
		unsigned int DecontourWeight;
	};
	struct {
		unsigned int EdgeWeight;
		unsigned int TextureWeight;
	};
	struct {
		unsigned int ICM_Weight0;
		unsigned int ICM_Weight1;
		unsigned int ICM_Weight2;
	};
	struct {
		unsigned int LocalContrastWeight;
	};
	struct {
		unsigned int SRNNIn_Weight0;
		unsigned int SRNNIn_Weight1;
		unsigned int SRNNIn_Weight2;
	};
	struct {
		unsigned int SRNNOut_Weight0;
	};	
} PQMASK_DM_WINDOW_WEIGHT_T;

typedef struct PQMASK_WINDOW
{
	unsigned int HorSta;
	unsigned int HorEnd;
	unsigned int VerSta;
	unsigned int VerEnd;
	PQMASK_DM_WINDOW_WEIGHT_T weight;
} PQMASK_WINDOW_T;

typedef struct PQMASK_DOMAIN_DMA_CONFIG
{
	unsigned int AddrUpLimit;
	unsigned int AddrLowLimit;
	unsigned char SwapType;
} PQMASK_DOMAIN_DMA_CONFIG_T;

typedef struct PQMASK_BLOCK_DMA_CONFIG
{
	unsigned int DataBurstLen;
	unsigned int LineStep;
} PQMASK_BLOCK_DMA_CONFIG_T;

#if 0
/* FUNCTIONS */
#endif

/*DOUBLE BUFFER CONTROL*/
void drvif_module_set_pqmask_domain_db_enable(unsigned char bEnable, unsigned char DomainItem);
void drvif_module_set_pqmask_domain_db_apply(unsigned char DomainItem);
int drvif_module_wait_pqmask_domain_db_apply_done(unsigned int WaitCnt, unsigned char DomainItem);
/*SET FUNCTION*/
/*DOMAIN DMA*/
void drvif_color_set_pqmask_domain_DMA_enable(unsigned char bEnable,  unsigned char DomainItem);
void drvif_color_set_pqmask_domain_DMA_config(PQMASK_DOMAIN_DMA_CONFIG_T *pDmaConfig, unsigned char DomainItem);
/*BLOCK DMA*/
void drvif_color_set_pqmask_block_DMA_config(PQMASK_BLOCK_DMA_CONFIG_T *pDmaConfig, unsigned char BlockItem);
/*BLOCK CTRL*/
void drvif_color_set_pqmask_block_scaling_enable(unsigned char bHorEnable, unsigned char bVerEnable, unsigned char BlockItem);
void drvif_color_set_pqmask_block_scaling(PQMASK_SACLING_T *ptr, unsigned char BlockItem);
void drvif_color_set_pqmask_block_size(PQMASK_IO_SIZE_T *ptr, unsigned char BlockItem);
void drvif_color_set_pqmask_block_dm_window_enable(unsigned char bEnable, unsigned char BlockItem);
void drvif_color_set_pqmask_block_dm_window(PQMASK_WINDOW_T *ptr, unsigned char BlockItem);
/*MODULE*/
void drvif_color_set_pqmask_module_DMA_Addr(unsigned int Addr, unsigned char ModuleItem);
void drvif_color_set_pqmask_module_enable(unsigned char bEnablem, unsigned char ModuleItem);
/*GET FUNCTION*/
/*DOMAIN*/
unsigned char drvif_color_get_pqmask_domain_DMA_enable(unsigned char DomainItem);
/*BLOCK*/

/*MODULE*/
unsigned char drvif_color_get_pqmask_module_enable(unsigned char ModuleItem);
unsigned int drvif_color_get_pqmask_module_DMA_Addr(unsigned char ModuleItem);


/* Merlin7 */
/*I DOMAIN*/
void drvif_color_get_pqmask_idomain_DMA_config(PQMASK_DOMAIN_DMA_CONFIG_T *pDmaConfig, PQMASK_BLOCK_DMA_CONFIG_T *pModuleConfig);
void drvif_color_get_pqmask_idomain_scaling_enable(unsigned char *bHorEnable, unsigned char *bVerEnable);
void drvif_color_get_pqmask_idomain_scaling(PQMASK_SACLING_T *ptr);
void drvif_color_get_pqmask_idomain_size(PQMASK_IO_SIZE_T *ptr);
void drvif_color_get_pqmask_idomain_size_rd_status(PQMASK_IMG_SIZE_T *ptr);
void drvif_color_get_pqmask_idomain_dm_window_enable(unsigned char *bEnable);
void drvif_color_get_pqmask_idomain_dm_window(PQMASK_WINDOW_T *ptr);
/*D DOMAIN DMA*/
void drvif_color_get_pqmask_ddomain_DMA_enable(unsigned char *bEnable);
void drvif_color_get_pqmask_ddomain_DMA_config(PQMASK_DOMAIN_DMA_CONFIG_T *pDmaConfig);
/*D DOMAIN SHARPNESS DMA*/
void drvif_color_get_pqmask_shp_DMA_config(PQMASK_BLOCK_DMA_CONFIG_T *pModuleConfig);
/*D DOMAIN SHARPNESS*/
void drvif_color_get_pqmask_shp_scaling_enable(unsigned char *bHorEnable, unsigned char *bVerEnable);
void drvif_color_get_pqmask_shp_scaling(PQMASK_SACLING_T *ptr);
void drvif_color_get_pqmask_shp_size(PQMASK_IO_SIZE_T *ptr);
void drvif_color_get_pqmask_shp_size_rd_status(PQMASK_IMG_SIZE_T *ptr);
void drvif_color_get_pqmask_shp_dm_window_enable(unsigned char *bEnable);
void drvif_color_get_pqmask_shp_dm_window(PQMASK_WINDOW_T *ptr);
/*D DOMAIN ICM DMA*/
void drvif_color_get_pqmask_icm_DMA_config(PQMASK_BLOCK_DMA_CONFIG_T *pModuleConfig);
/*D DOMAIN ICM*/
void drvif_color_get_pqmask_icm_scaling_enable(unsigned char *bHorEnable, unsigned char *bVerEnable);
void drvif_color_get_pqmask_icm_scaling(PQMASK_SACLING_T *ptr);
void drvif_color_get_pqmask_icm_size(PQMASK_IO_SIZE_T *ptr);
void drvif_color_get_pqmask_icm_size_rd_status(PQMASK_IMG_SIZE_T *ptr);
void drvif_color_get_pqmask_icm_dm_window_enable(unsigned char *bEnable);
void drvif_color_get_pqmask_icm_dm_window(PQMASK_WINDOW_T *ptr);
/*D DOMAIN LOCALCONTRAST DMA*/
void drvif_color_get_pqmask_local_contrast_DMA_config(PQMASK_BLOCK_DMA_CONFIG_T *pModuleConfig);
/*D DOMAIN LOCALCONTRAST*/
void drvif_color_get_pqmask_local_contrast_scaling_enable(unsigned char *bHorEnable, unsigned char *bVerEnable);
void drvif_color_get_pqmask_local_contrast_scaling(PQMASK_SACLING_T *ptr);
void drvif_color_get_pqmask_local_contrast_size(PQMASK_IO_SIZE_T *ptr);
void drvif_color_get_pqmask_local_contrast_size_rd_status(PQMASK_IMG_SIZE_T *ptr);
void drvif_color_get_pqmask_local_contrast_dm_window_enable(unsigned char *bEnable);
void drvif_color_get_pqmask_local_contrast_dm_window(PQMASK_WINDOW_T *ptr);

#endif

