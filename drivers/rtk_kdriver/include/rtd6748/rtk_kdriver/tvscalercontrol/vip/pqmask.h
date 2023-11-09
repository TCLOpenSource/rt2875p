#ifndef __PQMASK_H__
#define __PQMASK_H__

#if 0
/* data type */
#endif

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
		unsigned int HueWeight;
		unsigned int SaturationWeight;
		unsigned int IntensityWeight;
	};
	struct {
		unsigned int LocalContrastWeight;
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

typedef enum PQMASK_DMA_SWAP
{
	PQMASK_DMA_NONE_SWP = 0,
	PQMASK_DMA_1BYTE_SWP,
	PQMASK_DMA_2BYTE_SWP,
	PQMASK_DMA_4BYTE_SWP,
	PQMASK_DMA_8BYTE_SWP,
	PQMASK_DMA_BYTE_SWP_MAX
} PQMASK_DMA_SWAP_T;

typedef struct PQMASK_DMA_CONFIG
{
	unsigned int AddrUpLimit;
	unsigned int AddrLowLimit;
	PQMASK_DMA_SWAP_T SwapType;
} PQMASK_DMA_CONFIG_T;

typedef struct PQMASK_MODULE_DMA_CONFIG
{
	unsigned int DataBurstLen;
	unsigned int LineStep;
} PQMASK_MODULE_DMA_CONFIG_T;

#if 0
/* FUNCTIONS */
#endif

/*DOUBLE BUFFER CONTROL*/
void drvif_module_set_pqmask_idomain_db_enable(unsigned char bEnable);
void drvif_module_set_pqmask_idomain_db_apply(void);
int drvif_module_wait_pqmask_idomain_db_apply_done(unsigned int WaitCnt);
void drvif_module_set_pqmask_ddomain_db_enable(unsigned char bEnable);
void drvif_module_set_pqmask_ddomain_db_apply(void);
int drvif_module_wait_pqmask_ddomain_db_apply_done(unsigned int WaitCnt);

/*I DOMAIN DMA*/
void drvif_color_set_pqmask_idomain_DMA_enable(unsigned char bEnable);
void drvif_color_set_pqmask_idomain_DMA_config(PQMASK_DMA_CONFIG_T *pDmaConfig, PQMASK_MODULE_DMA_CONFIG_T *pModuleConfig);
void drvif_color_set_pqmask_idomain_DMA_SNR_Addr(unsigned int Addr);
void drvif_color_set_pqmask_idomain_DMA_Decontour_Addr(unsigned int Addr);
void drvif_color_set_pqmask_idomain_SNR_enable(unsigned char bEnable);
void drvif_color_set_pqmask_idomain_Decontour_enable(unsigned char bEnable);

void drvif_color_get_pqmask_idomain_DMA_enable(unsigned char *bEnable);
void drvif_color_get_pqmask_idomain_DMA_config(PQMASK_DMA_CONFIG_T *pDmaConfig, PQMASK_MODULE_DMA_CONFIG_T *pModuleConfig);
void drvif_color_get_pqmask_idomain_DMA_SNR_Addr(unsigned int *Addr);
void drvif_color_get_pqmask_idomain_DMA_Decontour_Addr(unsigned int *Addr);
void drvif_color_get_pqmask_idomain_SNR_enable(unsigned char *bEnable);
void drvif_color_get_pqmask_idomain_Decontour_enable(unsigned char *bEnable);

/*I DOMAIN*/
void drvif_color_set_pqmask_idomain_scaling_enable(unsigned char bHorEnable, unsigned char bVerEnable);
void drvif_color_set_pqmask_idomain_scaling(PQMASK_SACLING_T *ptr);
void drvif_color_set_pqmask_idomain_size(PQMASK_IO_SIZE_T *ptr);
void drvif_color_set_pqmask_idomain_dm_window_enable(unsigned char bEnable);
void drvif_color_set_pqmask_idomain_dm_window(PQMASK_WINDOW_T *ptr);

void drvif_color_get_pqmask_idomain_scaling_enable(unsigned char *bHorEnable, unsigned char *bVerEnable);
void drvif_color_get_pqmask_idomain_scaling(PQMASK_SACLING_T *ptr);
void drvif_color_get_pqmask_idomain_size(PQMASK_IO_SIZE_T *ptr);
void drvif_color_get_pqmask_idomain_size_rd_status(PQMASK_IMG_SIZE_T *ptr);
void drvif_color_get_pqmask_idomain_dm_window_enable(unsigned char *bEnable);
void drvif_color_get_pqmask_idomain_dm_window(PQMASK_WINDOW_T *ptr);

/*D DOMAIN DMA*/
void drvif_color_set_pqmask_ddomain_DMA_enable(unsigned char bEnable);
void drvif_color_set_pqmask_ddomain_DMA_config(PQMASK_DMA_CONFIG_T *pDmaConfig);

void drvif_color_get_pqmask_ddomain_DMA_enable(unsigned char *bEnable);
void drvif_color_get_pqmask_ddomain_DMA_config(PQMASK_DMA_CONFIG_T *pDmaConfig);

/*D DOMAIN SHARPNESS DMA*/
void drvif_color_set_pqmask_shp_DMA_config(PQMASK_MODULE_DMA_CONFIG_T *pModuleConfig);
void drvif_color_set_pqmask_shp_Text_Addr(unsigned int Addr);
void drvif_color_set_pqmask_shp_Edge_Addr(unsigned int Addr);
void drvif_color_set_pqmask_shp_enable(unsigned char bTextureEnable, unsigned char bEdgeEnable);

void drvif_color_get_pqmask_shp_DMA_config(PQMASK_MODULE_DMA_CONFIG_T *pModuleConfig);
void drvif_color_get_pqmask_shp_Text_Addr(unsigned int *Addr);
void drvif_color_get_pqmask_shp_Edge_Addr(unsigned int *Addr);
void drvif_color_get_pqmask_shp_enable(unsigned char *bTextureEnable, unsigned char *bEdgeEnable);

/*D DOMAIN SHARPNESS*/
void drvif_color_set_pqmask_shp_scaling_enable(unsigned char bHorEnable, unsigned char bVerEnable);
void drvif_color_set_pqmask_shp_scaling(PQMASK_SACLING_T *ptr);
void drvif_color_set_pqmask_shp_size(PQMASK_IO_SIZE_T *ptr);
void drvif_color_set_pqmask_shp_dm_window_enable(unsigned char bEnable);
void drvif_color_set_pqmask_shp_dm_window(PQMASK_WINDOW_T *ptr);

void drvif_color_get_pqmask_shp_scaling_enable(unsigned char *bHorEnable, unsigned char *bVerEnable);
void drvif_color_get_pqmask_shp_scaling(PQMASK_SACLING_T *ptr);
void drvif_color_get_pqmask_shp_size(PQMASK_IO_SIZE_T *ptr);
void drvif_color_get_pqmask_shp_size_rd_status(PQMASK_IMG_SIZE_T *ptr);
void drvif_color_get_pqmask_shp_dm_window_enable(unsigned char *bEnable);
void drvif_color_get_pqmask_shp_dm_window(PQMASK_WINDOW_T *ptr);

/*D DOMAIN ICM DMA*/
void drvif_color_set_pqmask_icm_DMA_config(PQMASK_MODULE_DMA_CONFIG_T *pModuleConfig);
void drvif_color_set_pqmask_icm_HUE_Addr(unsigned int Addr);
void drvif_color_set_pqmask_icm_SAT_Addr(unsigned int Addr);
void drvif_color_set_pqmask_icm_INT_Addr(unsigned int Addr);
void drvif_color_set_pqmask_icm_enable(unsigned char bHueEnable, unsigned char bSaturationEnable, unsigned char bIntensityEnable);

void drvif_color_get_pqmask_icm_DMA_config(PQMASK_MODULE_DMA_CONFIG_T *pModuleConfig);
void drvif_color_get_pqmask_icm_HUE_Addr(unsigned int *Addr);
void drvif_color_get_pqmask_icm_SAT_Addr(unsigned int *Addr);
void drvif_color_get_pqmask_icm_INT_Addr(unsigned int *Addr);
void drvif_color_get_pqmask_icm_enable(unsigned char *bHueEnable, unsigned char *bSaturationEnable, unsigned char *bIntensityEnable);

/*D DOMAIN ICM*/
void drvif_color_set_pqmask_icm_scaling_enable(unsigned char bHorEnable, unsigned char bVerEnable);
void drvif_color_set_pqmask_icm_scaling(PQMASK_SACLING_T *ptr);
void drvif_color_set_pqmask_icm_size(PQMASK_IO_SIZE_T *ptr);
void drvif_color_set_pqmask_icm_dm_window_enable(unsigned char bEnable);
void drvif_color_set_pqmask_icm_dm_window(PQMASK_WINDOW_T *ptr);

void drvif_color_get_pqmask_icm_scaling_enable(unsigned char *bHorEnable, unsigned char *bVerEnable);
void drvif_color_get_pqmask_icm_scaling(PQMASK_SACLING_T *ptr);
void drvif_color_get_pqmask_icm_size(PQMASK_IO_SIZE_T *ptr);
void drvif_color_get_pqmask_icm_size_rd_status(PQMASK_IMG_SIZE_T *ptr);
void drvif_color_get_pqmask_icm_dm_window_enable(unsigned char *bEnable);
void drvif_color_get_pqmask_icm_dm_window(PQMASK_WINDOW_T *ptr);

/*D DOMAIN LOCALCONTRAST DMA*/
void drvif_color_set_pqmask_local_contrast_DMA_config(PQMASK_MODULE_DMA_CONFIG_T *pModuleConfig);
void drvif_color_set_pqmask_local_contrast_Addr(unsigned int LocalContrastAddr);
void drvif_color_set_pqmask_local_contrast_enable(unsigned char bLocalContrastEnable);

void drvif_color_get_pqmask_local_contrast_DMA_config(PQMASK_MODULE_DMA_CONFIG_T *pModuleConfig);
void drvif_color_get_pqmask_local_contrast_Addr(unsigned int *LocalContrastAddr);
void drvif_color_get_pqmask_local_contrast_enable(unsigned char *bLocalContrastEnable);

/*D DOMAIN LOCALCONTRAST*/
void drvif_color_set_pqmask_local_contrast_scaling_enable(unsigned char bHorEnable, unsigned char bVerEnable);
void drvif_color_set_pqmask_local_contrast_scaling(PQMASK_SACLING_T *ptr);
void drvif_color_set_pqmask_local_contrast_size(PQMASK_IO_SIZE_T *ptr);
void drvif_color_set_pqmask_local_contrast_dm_window_enable(unsigned char bEnable);
void drvif_color_set_pqmask_local_contrast_dm_window(PQMASK_WINDOW_T *ptr);

void drvif_color_get_pqmask_local_contrast_scaling_enable(unsigned char *bHorEnable, unsigned char *bVerEnable);
void drvif_color_get_pqmask_local_contrast_scaling(PQMASK_SACLING_T *ptr);
void drvif_color_get_pqmask_local_contrast_size(PQMASK_IO_SIZE_T *ptr);
void drvif_color_get_pqmask_local_contrast_size_rd_status(PQMASK_IMG_SIZE_T *ptr);
void drvif_color_get_pqmask_local_contrast_dm_window_enable(unsigned char *bEnable);
void drvif_color_get_pqmask_local_contrast_dm_window(PQMASK_WINDOW_T *ptr);

#endif
