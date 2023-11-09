#include <generated/autoconf.h>
#include <linux/compat.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/string.h>
#include <linux/kernel.h>   /* DBG_PRINT()    */
#include <linux/slab.h>     /* kmalloc()      */
#include <linux/fs.h>       /* everything...  */
#include <linux/errno.h>    /* error codes    */
#include <linux/types.h>    /* size_t         */
#include <linux/cdev.h>
#include <linux/version.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0))
#include <linux/dma-map-ops.h>
#else
#include <linux/dma-mapping.h>
#endif

#include <linux/uaccess.h>    /* copy_*_user    */
#include <linux/jiffies.h>
#include <linux/interrupt.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/pageremap.h>
#include <linux/random.h>
#include <linux/pageremap.h>
#include <linux/proc_fs.h>
#include <linux/kthread.h>  /* for threads */
#include <linux/time.h>   /* for using jiffies */
#include <linux/version.h>
#include <asm/barrier.h> /*dsb()*/
#include <asm/cacheflush.h>

/* [WOSQRTK-6662]fix logo dealy time.*/
#include "tvscalercontrol/panel/panelapi.h"

#include <linux/of_irq.h>
#include <mach/platform.h>
#include <mach/system.h>
#include <mach/rtk_platform.h>

#include <rbus/sb2_reg.h>

#include <rbus/sfg_reg.h>
#include <rbus/ppoverlay_reg.h>
#include <rbus/sys_reg_reg.h>
#include <rbus/pll27x_reg_reg.h>
#include <rbus/pll_reg_reg.h>
#include <rbus/stb_reg.h>
#include <rbus/sys_reg_reg.h> /* rtk interrupt*/
#include <rbus/timer_reg.h>

#include <rtk_kdriver/io.h>

#include <rtk_crt.h>
#include <panelConfigParameter.h>

#include <rbus/gdma_reg.h>
#include <gal/rtk_gdma_driver.h>
#include <gal/rtk_gdma_scale_coef.h>
#include <gal/rtk_osdcomp_driver.h>
#include <tvscalercontrol/panel/panelapi.h>
#include <gal/rtk_gdma_split.h>
#include <gal/rtk_kadp_gdma_hal_gal_driver.h>
#include <linux/version.h>
#include <linux/spinlock.h>
#include <linux/file.h>
#include <linux/fs.h>
#include <linux/sync_file.h>
#include <linux/dma-fence.h>
#include "osd_dump_agent.h"
/*need sema.*/
//static int osd_dump_state = 0;

#define  GDMA_GAMMA 0
#include "rtk_kadp_se.h"

#define DRV_NAME	"gdma"

#include <rtd_log/rtd_module_log.h>
#define TAG_NAME "GDMA"



  // [FIXME]
#define ENABLE_BRINGUP_WORKAROUND
#if defined(CONFIG_RTK_KDRV_GDMA_MODULE)
extern int (*ptr_GDMA_Check_Dvr_Reclaim)(void);

#endif


/*
0: disable
1: fatal logs
..
10: print all logs

*/
unsigned int gDebugGDMA_loglevel = 0;
unsigned int gdma_suspend_disable_osd=0;
unsigned int gDebugGDMA_dump_count = 0;
char gDebugGDMA_dump_name[128] = {0};

EXPORT_SYMBOL(gDebugGDMA_loglevel);



int gDebugFence_loglevel = 0 ;
int ui_re=UI_RESOLUTION_2K;
GDMA_MONITOR_CTL  gGDMA_Monitor_ctl;


#ifdef CONFIG_RTK_KDRV_GDMA_MODULE
   void rtk_parse_osd_split_mode(void);
#endif

#ifdef ENABLE_VSYNC_NOTIFY
static DECLARE_COMPLETION( gdma_vsync_completion );

static DEFINE_SPINLOCK(vsync_completion_lock);

atomic64_t gGDMA_VSYNC_USER_TIME = ATOMIC64_INIT(0);

#endif//

static DEFINE_SPINLOCK(dma_buf_gdma_fence_lock);

struct gdma_fence {
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 10, 0))
	struct fence base;
#else
	struct dma_fence base;
#endif
    int gdma_fence_fd;
};

#define GDMA_FENCE_ARRAY_SIZE    4096
struct gdma_fence *gdma_fence_array = NULL;
unsigned int gdma_fence_index = 0;

static int GDMA_SignalFence(struct gdma_fence *gf);

unsigned int gFence_OSD_Context = 0;
/* The dma-buf fence sequence number for this atom. This is
 * increased every time this katom uses dma-buf fence.
 */
atomic_t gFence_OSD_Seqno;

static bool timeline_fence_enable_signaling(struct dma_fence *fence)
{
    return test_and_set_bit(DMA_FENCE_FLAG_ENABLE_SIGNAL_BIT, &fence->flags);
}
static const char *timeline_fence_get_timeline_name(struct dma_fence *fence)
{
    return "T";
}
static const char *timeline_fence_get_driver_name(struct dma_fence *fence)
{
	return "gdma";
}
static bool gdma_fence_signaled(struct dma_fence *fence)
{
    return test_bit(DMA_FENCE_FLAG_SIGNALED_BIT, &fence->flags);
}
static void gdma_fence_fence_value_str(struct dma_fence *fence, char *str, int size)
{
#if (KERNEL_VERSION(5, 1, 0) > LINUX_VERSION_CODE)
	snprintf(str, size, "%u", fence->seqno);
#else
	snprintf(str, size, "%llu", fence->seqno);
#endif
}
static void gdma_fence_timeline_value_str(struct dma_fence *fence, char *str, int size)
{
    snprintf(str, size, "%d", atomic_read(&gFence_OSD_Seqno));
}

void gdma_fence_free(struct dma_fence *fence)
{
    //printk(KERN_EMERG"gdma fence (%llx) to free \n", fence);
}

static const struct dma_fence_ops timeline_fence_ops = {
	.get_driver_name = timeline_fence_get_driver_name,
	.get_timeline_name = timeline_fence_get_timeline_name,
	.enable_signaling = timeline_fence_enable_signaling,
    .signaled = gdma_fence_signaled,
	.wait = dma_fence_default_wait,
	.release = gdma_fence_free,
	.fence_value_str = gdma_fence_fence_value_str,
    .timeline_value_str = gdma_fence_timeline_value_str,
};

static int GAL_Runtime_Suspend_status = 0;
static  int GAL_Suspend_status = 0;
gdma_dev *gdma_devices; /* allocated in gdma_init_module */

static struct gdma_fence* alloc_gdma_fence(void)
{
    struct gdma_fence *gf = 0;
    int fd = 0;
    struct sync_file *local_sync_file = 0;
    gdma_dev *gdma = &gdma_devices[0];

    down(&gdma->sem_get_fence);
    //gf = kzalloc(sizeof(*gf), GFP_KERNEL);
    if (gdma_fence_array) {
        gf = gdma_fence_array + gdma_fence_index;
        gdma_fence_index = (gdma_fence_index+1) % GDMA_FENCE_ARRAY_SIZE;
    }
    if (!gf) {
        up(&gdma->sem_get_fence);
        return 0;
    }

    dma_fence_init(&gf->base,
                   &timeline_fence_ops,
                   &dma_buf_gdma_fence_lock,
                   gFence_OSD_Context,
                   atomic_inc_return(&gFence_OSD_Seqno) );

    fd = get_unused_fd_flags(O_CLOEXEC);
    if (fd < 0) {
        kfree(gf);
        up(&gdma->sem_get_fence);
        return 0;
    }

    gf->gdma_fence_fd = fd;
    local_sync_file = sync_file_create(&gf->base);  /* BUG :scheduling while atomic: */

    if (!local_sync_file) {
        put_unused_fd(fd);
        up(&gdma->sem_get_fence);
        return 0;
    }

    fd_install(fd, local_sync_file->file);
    up(&gdma->sem_get_fence);

    return gf;
}

extern u32 gic_irq_find_mapping(u32 hwirq);

static int gdma_probe(struct platform_device* dev);
static int gdma_remove(struct platform_device* dev);

#ifdef CONFIG_PM
static int gdma_suspend(struct device *p_dev);
static int gdma_resume(struct device *p_dev);
//add by fox for gfx std
//
#ifdef CONFIG_HIBERNATION
static int gdma_suspend_std(struct device *p_dev);
static int gdma_resume_std(struct device *p_dev);
static int gdma_resume_std_thraw(struct device *p_dev);

#endif
#endif

/*will be updated in gdma_init func */
static unsigned long gdma_total_ddr_size = 0x60000000;

int gdma_major = GDMA_MAJOR;
int gdma_minor = 0;
int gdma_nr_devs = GDMA_NR_DEVS;

#ifdef CONFIG_LG_SNAPSHOT_BOOT
extern bool reserve_boot_memory;
#endif
#if (defined(CONFIG_ARCH_RTK288O) || defined(CONFIG_ARCH_RTK2875) || defined(CONFIG_ARCH_RTK2885M) || defined(CONFIG_ARCH_RTK6748)) && defined(THRESHOLD_DETECT)
#define ALPHA_THRESHOLD_NUM 8
unsigned int alpha_osd_detect_cnt[ALPHA_THRESHOLD_NUM];
CONFIG_OSD_GAIN_INFO osd_gain_info;
#define GDMA_PM_ALPHA_ALL_ZERO_NUM (3840*2160)
#endif
int flag_gdma_resume_std=0;
static PICTURE_LAYERS_OBJECT glastinfo;
unsigned int  buf_phy_addr[BUF_PHY_NUM];


#if USE_RLINK_INTERFACE
/*
    RLNK_GDMA_OSD_VBY1_ENABLE = 0x100,
    RLNK_GDMA_OSD_VBY1_DISABLE,
    RLNK_GDMA_FUNC_1,
    RLNK_GDMA_FUNC_2,
    RLNK_GDMA_ESTICKER_512 = 0x104,
    RLNK_GDMA_ESTICKER_1080 = 0x105,
	RLNK_GDMA_OSD_DISABLE = 0x106,
	RLNK_GDMA_OSD_ENABLE = 0x107,
	RLNK_GDMA_OSD_X_P = 0x108,
	RLNK_GDMA_OSD_Y_P = 0x109,
 */
struct rtk_gdma_interface_rlink {
    int (*func)(int);
};
struct rtk_gdma_interface {
    struct rtk_gdma_interface_rlink rlink;

    /* add your own interface below*/
};
static struct rtk_gdma_interface g_iface;
static int rlink_interface_cmd=0;
static int rlink_interface_ready=0;
static int rlink_interface_open=0;
void GDMA_rlink_interface_register_rlink(struct rtk_gdma_interface *iface);
void GDMA_rlink_interface_remove_rlink(void);
void GDMA_rlink_interface_init(void);
void GDMA_rlink_interface_call(int num);	
void GDMA_rlink_split(unsigned int data);

void GDMA_rlink_interface_remove_rlink()
{
	g_iface.rlink.func = NULL; 
	rlink_interface_ready = 0;
	return ;
}
void GDMA_rlink_interface_register_rlink(struct rtk_gdma_interface *iface)
{
	g_iface.rlink.func = iface->rlink.func; 
	rlink_interface_ready = 1;
	return ;
}
void GDMA_rlink_interface_init()
{
	g_iface.rlink.func = NULL; 
	return ;
}
void GDMA_rlink_interface_call(int num)
{
	printk("func=%s line=%d num=%d \n",__FUNCTION__,__LINE__,num);
	if( rlink_interface_ready & (g_iface.rlink.func != NULL) ){
		g_iface.rlink.func(num); 
	printk("func=%s line=%d num=%d \n",__FUNCTION__,__LINE__,num);
	}
	return ;
}
void GDMA_rlink_split(unsigned int data)
{
	if(rlink_interface_open == 0){
		GDMA_rlink_interface_call(1);// 1 -- > open session
		rlink_interface_open = 1;
	}
	if(data){
		//GDMA_rlink_interface_call(258);  // 258 --> RLNK_GDMA_FUNC_1 , black
		GDMA_split_mode(1);
		GDMA_ReceivePicture(&glastinfo);
		GDMA_rlink_interface_call(256); // 256 -- > RLNK_GDMA_OSD_VBY1_ENABLE
		gdma_usleep(16*1000);
		//gdma_usleep(100*1000);
		//GDMA_rlink_interface_call(259);  // 259 --> RLNK_GDMA_FUNC_2 , remove black
	}else{
		//GDMA_rlink_interface_call(258);  // 258 --> RLNK_GDMA_FUNC_1 , black
		GDMA_rlink_interface_call(257); // 257 -- > RLNK_GDMA_OSD_VBY1_DISABLE
		gdma_usleep(16*1000);
		GDMA_split_mode(0);
		GDMA_ReceivePicture(&glastinfo);
		//gdma_usleep(100*1000);
		//GDMA_rlink_interface_call(259);  // 259 --> RLNK_GDMA_FUNC_2 , remove black
	}
}
#endif
/* from offline process */
extern volatile unsigned int gActiveOffPlane;
#if defined(CONFIG_CUSTOMER_TV006) && defined(CONFIG_SUPPORT_SCALER)
extern void vbe_disp_early_resume(void);
#endif

//RTK mark for netfix
int enable_osd1osd3mixerorder = 1;

//RTK mark for stop update gdma
int enable_stop_update = 0;

//RTK mark for change OSD mode
//0 --> original mode
//1 --> osd split mode mixer mode 
//2 --> GDMA_SPLIT_MODE_2 osd no sr 2k1k split mode
//3 --> GDMA_SPLIT_MODE_3 osd no sr 1k2k split mode
#if defined(CONFIG_RTK_KDRV_GDMA_SPLIT_2)
int change_OSD_mode = GDMA_SPLIT_MODE_2;
#elif defined(CONFIG_RTK_KDRV_GDMA_SPLIT_3)
int change_OSD_mode = GDMA_SPLIT_MODE_3;
#else
int change_OSD_mode = 0;
#endif
#define ES_OSD_ADDR_NUM 3
#define ES_OSD_MAX_H 280
unsigned int es_osd_addr[ES_OSD_ADDR_NUM];
int es_osd_addr_count=0;
unsigned int cp_src=0;
int GDMA_ES_OSD_Data_test(unsigned int cp_src);
int GDMA_ES_OSD_Data(int s_w,int s_h,
					 int d_w,int d_h,int num,
					 unsigned int *cp_dst,
					 unsigned int cp_src) ;
int gContext = -1;
static struct class *gdma_class;
#if defined(CONFIG_ARM64)
unsigned long gdma_dev_addr = 0;
#else
unsigned int gdma_dev_addr = 0;
#endif
short GDMA_coef_4t8p_ratio_swap_[GDMA_NUM_COEFF_RATIOS][4][4];

#ifdef TRIPLE_BUFFER_SEMAPHORE
int gSEMAPHORE_MAX = 1;
int gSemaphore[GDMA_PLANE_MAXNUM];
#endif

CONFIG_OSD_ESTICKER_INFO g_esticker_info;
void GDMA_ConfigEstickerPosition(int eightk);

#if USE_RLINK_INTERFACE
int GDMA_estiker_osd_enable(unsigned int enable)
{
	
	if(change_OSD_mode != GDMA_SPLIT_MODE_2)
		return GDMA_SUCCESS;
	if(rlink_interface_open == 0){
		GDMA_rlink_interface_call(1);// 1 -- > open session
		rlink_interface_open = 1;
	}
	if(enable){
		GDMA_rlink_interface_call(0x107);//RLNK_GDMA_OSD_ENABLE
	}else{
		GDMA_rlink_interface_call(0x106);//RLNK_GDMA_OSD_DISABLE
	}
	return GDMA_SUCCESS;
}

int GDMA_estiker_enable(unsigned int enable)
{

	if(change_OSD_mode != GDMA_SPLIT_MODE_2)
		return GDMA_SUCCESS;
	if(rlink_interface_open == 0){
		GDMA_rlink_interface_call(1);// 1 -- > open session
		rlink_interface_open = 1;
	}
	if(enable){
		GDMA_rlink_interface_call(0x104);//RLNK_GDMA_ESTICKER_512
	}else{
		GDMA_rlink_interface_call(0x105);//RLNK_GDMA_ESTICKER_512
	}
	return GDMA_SUCCESS;
}

int GDMA_estiker_osd_x_p(unsigned int x_p)
{
	
	if(change_OSD_mode != GDMA_SPLIT_MODE_2)
		return GDMA_SUCCESS;
	if(rlink_interface_open == 0){
		GDMA_rlink_interface_call(1);// 1 -- > open session
		rlink_interface_open = 1;
	}

	GDMA_rlink_interface_call(0x108);//RLNK_GDMA_OSD_X_P
	GDMA_rlink_interface_call(x_p);//x_p

	return GDMA_SUCCESS;
}
int GDMA_estiker_osd_y_p(unsigned int y_p)
{
	
	if(change_OSD_mode != GDMA_SPLIT_MODE_2)
		return GDMA_SUCCESS;
	if(rlink_interface_open == 0){
		GDMA_rlink_interface_call(1);// 1 -- > open session
		rlink_interface_open = 1;
	}
	GDMA_rlink_interface_call(0x109);//RLNK_GDMA_OSD_Y_P
	GDMA_rlink_interface_call(y_p);//y_p

	return GDMA_SUCCESS;
}
#endif
static const char  drv_name[] = DRV_NAME;

#define FBC_ON      1
#define FBC_OFF     0

#define MAX_AI_BLOCK 6

#define AI_WIDTH 480   //480
#define AI_HEIGH 270

#define AI_WIDTH_1920 1920 //960
#define AI_HEIGH_1080 1080
#define AI_BLOCK_NUM 1
#define AI_DRAW_BY_BLOCK		1
#define AI_DRAW_BY_BLOCK_AFBC	2
#define AI_DRAW_BY_BUFFER		3
KGAL_RECT_T ai_block[MAX_AI_BLOCK];
PICTURE_LAYERS_OBJECT patlayerAI;

unsigned char *dst_AI=0;
unsigned char *dst_AI_2=0;
unsigned int dst_order = 0;
unsigned int ai_color = 0x00000000;
int ai_num = 0;
unsigned int AI_Enable=0;
unsigned char ai_draw_by=0;
int GDMA_AI_SE_draw_block(int s_w,int s_h,int num,unsigned int *color,KGAL_RECT_T *ai_block, unsigned char draw_scene) ;
int GDMA_AI_SE_draw_block_afbc(int s_w,int s_h,int num,unsigned int *color,KGAL_RECT_T *ai_block, unsigned char draw_scene) ;
int GDMA_AI_SE_draw_buffer(int s_w,int s_h,KGAL_PIXEL_FORMAT_T format,unsigned char *buffer);
int GDMA_AI_Get_UI_size(int *ui_size_w,int *ui_size_h);

int GDMA_CaptureFramebuffer(KGAL_SURFACE_INFO_T *pDstSurfaceInfo);

#if defined(CONFIG_RTK_KDRV_GAL)
//add by fox for gfx std
extern int GAL_MEM_ADDR_START;
extern int GAL_MEM_SIZE;
extern int fbcOnOff;

unsigned int gal_std_save_dvr_phy = 0;
#if defined(CONFIG_ARM64)
unsigned long gal_std_save_dvr_ptr = 0;
#else
unsigned int gal_std_save_dvr_ptr = 0;
#endif

//add by fox for gfx std
#define GFX_SPECIAL_SIZE 256*256
#define GFX_NUM_MAX 200
#define GFX_CURSOR_SIZE 4*1024*1024
typedef struct {
	unsigned int width;
	unsigned int height;
	unsigned int phyAddr;
#if defined(CONFIG_ARM64)
	unsigned long virtAddr;
#else
	unsigned int virtAddr;
#endif
	unsigned int to_phyAddr;
#if defined(CONFIG_ARM64)
	unsigned long to_virtAddr;
#else
	unsigned int to_virtAddr;
#endif
	unsigned int size;
} RtgfxRecord;

extern RtgfxRecord gfx_data[];

extern unsigned int total_size;
extern unsigned int total_index;

/*For free logo flow, add those extern func*/


bool bstoplogo =false;
static int logo_dalay_init_bool_1=0;
static int logo_do_dvr_reclaim=0;
#define INNOLUX_LOGO_TIME_DELAY_VALUE (1*HZ)
#define INNOLUX_LOGO_TIME_DELAY_VALUE_1 (HZ/10*4)
#else
extern phys_addr_t __initdata reserved_dvr_start;
extern phys_addr_t __initdata reserved_dvr_size;
extern phys_addr_t __initdata reserved_last_image_start;
extern phys_addr_t __initdata reserved_last_image_size;

int  fbcOnOff = FBC_OFF;
#endif

#if OSD_DETECTION_SUPPORT
struct timer_list GDMA_timer;
GDMA_OSD_DETECTION_PARAMETER_T gDetectParameter;
int gDetectionStatus = 0;
static int g_jiffies_counter; //ms
#endif

#if IS_ENABLED(CONFIG_RTK_KDRV_PM)
extern void do_dvr_reclaim(int cmd);
#endif

#ifdef CONFIG_PM
//static struct platform_device *gdma_devs;

static const struct dev_pm_ops gdma_pm_ops = {
	.suspend    = gdma_suspend,
	.resume     = gdma_resume,
#ifdef CONFIG_HIBERNATION
	.freeze     = gdma_suspend_std,
	.thaw       = gdma_resume_std_thraw,  //in_supsend == 1
	.poweroff   = gdma_suspend_std,
	.restore    = gdma_resume_std,        //in_supsend == 0
#endif
};

static const struct of_device_id rtk_gdma_match[] = {
	{
		.compatible = "realtek,gdma",
		.type = "osd",
	},
    { /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, rtk_gdma_match);


static struct platform_driver gdma_driver = {
	.probe = gdma_probe,
	.remove = gdma_remove,
	.driver = {
		.name           = (char *)drv_name,
		.of_match_table = of_match_ptr(rtk_gdma_match),
		.bus            = &platform_bus_type,
#ifdef CONFIG_PM
		.pm             = &gdma_pm_ops,
#endif
	},
};
#endif /* CONFIG_PM */

#if IS_ENABLED(CONFIG_REALTEK_PCBMGR)
extern platform_info_t platform_info;
#endif

/* for OSD shift function */
CONFIG_OSDSHIFT_STRUCT g_osdshift_ctrl;
CONFIG_OSDSHIFT_UPSCALE_STRUCT g_osdshift_upscale_ctrl;
CONFIG_OSDSHIFT_UPSCALE_PARA  g_osdshift_upscale_para;
struct semaphore sem_gdma_def;
//static int gdma_def_func(void*);
struct proc_dir_entry *rtkgdma_proc_dir=0;
struct proc_dir_entry *rtkgdma_proc_entry=0;
#define RTKGDMA_PROC_DIR "rtkgdma"
#define RTKGDMA_PROC_ENTRY "dbg"

#define RTKGDMA_MAX_CMD_LENGTH (256)
#define PRINT_LIMIT_ACC  4
// check OSD DMA INT status
unsigned int g_osd_acc_sync_num[GDMA_PLANE_MAXNUM] = {0};
unsigned int g_osd_acc_sync_print_num = 0 ;
unsigned int g_osd_udfl_num[GDMA_PLANE_MAXNUM] = {0};
unsigned int g_sr_udfl_num[OSD_SR_MAXNUM] = {0};

bool GDMA_init_debug_proc(void);
void GDMA_uninit_debug_proc(void);
static inline bool rtkgdma_dbg_parse_value(char *cmd_pointer, long long *parsed_data);
static inline void rtkgdma_dbg_EXECUTE(const int cmd_index, char **cmd_pointer);
GDMA_WIN winfo_osd3;
GDMA_WIN winfo_osd2;
GDMA_WIN winfo_osd1;
static int osd_sync_en = 1;

static const char *rtkgdma_cmd_str[] = {
	"h_shift=",		/* GDMA_DBG_CMD_SET_H_SHIFT */
	"v_shift=",		/* GDMA_DBG_CMD_SET_V_SHIFT */
	"osd_dp=",		/* GDMA_DBG_CMD_OSD_DUMP    */
	"osd_dp_ex=",		/* GDMA_DBG_CMD_OSD_DUMP_EX    */
	"osd_drop=",
	"osd_pattern=",
	"osd_vflip=",
	"osd_ctrl=",
	"osd_swap=",
	"osd_cap2mem=",
	"osd_info=",
	"osd_get_info=",
	"osd_ctrl_en=",
	"osd_int_status=",
	"osd_clone_en=",
	"osd_bw_gen_en=",
	"osd_sync_en=",
	"osd_stop_update=",
	"osd_split_mode=",
	"osd_rlink=",
	"osd_es_addr=",
	"osd_ai_block=",
	"osd_get_global_alpha=",
	"osd_set_osd_gain=",
	"osd_cap_fb=",
	"osd_ai_block_afbc=",
	"osd_set_alpha_detect=",
	"osd_get_alpha_ratio=",
	"osd_log=",
	"osd_monitor_ctl=",
	"osd_runtime_ctl=",
	"osd_continue_dump",
};

int GDMA_Check_Dvr_Reclaim(void)
{
	unsigned int osdwi = (unsigned int)rtd_inl(GDMA_OSD1_WI_reg);


	printk(KERN_EMERG"[GDMA] [%s %d] osdwi=%x",__FUNCTION__,__LINE__,osdwi);

	//GDMA_ConfigOSDxEnable(GDMA_PLANE_OSD1,false);

	if(SYS_REG_SYS_DISPCLKSEL_get_clken_disp_gdma(rtd_inl(SYS_REG_SYS_DISPCLKSEL_reg))) {
		if(osdwi && gdma_dev_addr && (osdwi < gdma_dev_addr)) {
			//rtd_pr_gdma_info("[GDMA]Skip %s, used=%08x, gdma will free later..\n",__func__,osdwi);
			printk(KERN_EMERG"[GDMA]Skip %s, used=%08x, gdma will free later..\n",__func__,osdwi);
			return 1;
		}
	}
	printk(KERN_EMERG"[GDMA] [%s ok  %d]",__FUNCTION__,__LINE__);

    return 0;
}

#if IS_ENABLED(CONFIG_RTK_KDRV_DC_MEMORY_TRASH_DETCTER)
#include <rtk_dc_mt.h>
#include <rbus/dc_sys_reg.h>
static void osd_debug_info(void);
DCMT_DEBUG_INFO_DECLARE(osd_mdscpu,osd_debug_info);
//for ANDROIDTV-305 test
static void osd_debug_info(void)
{
	dcmt_trap_info trap_info;
	unsigned int osd_rbase = GDMA_OSD1_CTRL_reg;
	char osd_msg[256] = {0};
#if defined(CONFIG_ARM64)
	volatile unsigned long wi_addr;
#else
	volatile unsigned int wi_addr;
#endif

	if(isDcmtTrap("TVSB4_OSD1")) {
		osd_rbase = GDMA_OSD1_CTRL_reg;
		snprintf(osd_msg, 256,"[OSD1]Trash:");
        rtd_outl(GDMA_DBG_WI_INFO_reg, 0x1);
	}else if(isDcmtTrap("TVSB4_OSD2")) {
		osd_rbase = GDMA_OSD2_CTRL_reg;
		snprintf(osd_msg, 256,"[OSD2]Trash:");
        rtd_outl(GDMA_DBG_WI_INFO_reg, 0x2);
	}else if(isDcmtTrap("TVSB4_OSD3")) {
		osd_rbase = GDMA_OSD3_CTRL_reg;
		snprintf(osd_msg, 256,"[OSD3]Trash:");
        rtd_outl(GDMA_DBG_WI_INFO_reg, 0x3);
	}else{
		/*do nothing, shall we need printk ?*/
		printk(KERN_ERR"NOT OSDx module.\n");
		return;
	}
	/*get dcmt trap information (include trash_addr,module_id,rw_type)*/
	get_dcmt_trap_info(&trap_info);
	printk(KERN_ERR"=============%s=============\n",osd_msg);
	printk(KERN_ERR"\t%s=%08x\n",GDMA_UPDATE_reg_addr,
		rtd_inl(GDMA_UPDATE_reg));
	printk(KERN_ERR"\t%s=%08x\n",GDMA_CTRL_reg_addr,
		rtd_inl(GDMA_CTRL_reg));
	printk(KERN_ERR"\t======================================\n");
	printk(KERN_ERR"\t%08x=%08x\n",osd_rbase, rtd_inl(osd_rbase));
	printk(KERN_ERR"\t%08x=%08x\n",osd_rbase+0x4, rtd_inl(osd_rbase+0x4));
	printk(KERN_ERR"\t%08x=%08x\n",osd_rbase+0x8, rtd_inl(osd_rbase+0x8));
	printk(KERN_ERR"\t%08x=%08x\n",osd_rbase+0xc, rtd_inl(osd_rbase+0xc));
	printk(KERN_ERR"\t%08x=%08x\n",osd_rbase+0x10, rtd_inl(osd_rbase+0x10));

	if(rtd_inl(osd_rbase+0x10)) {
#if defined(CONFIG_ARM64)
		wi_addr = (volatile unsigned long)phys_to_virt(rtd_inl(osd_rbase+0x10));
		printk(KERN_ERR"\tsurface_addr=%lx\n",*((volatile unsigned long*)(wi_addr)+6));
#else
		wi_addr = (volatile unsigned int)phys_to_virt(rtd_inl(osd_rbase+0x10));
		printk(KERN_ERR"\tsurface_addr=%08x\n",*((volatile unsigned int*)(wi_addr)+6));
   		printk(KERN_ERR"\tnext_addr=%08x\n",*((volatile unsigned int*)(wi_addr)));
   		printk(KERN_ERR"\tbot addr=%08x\n",*((volatile unsigned int*)(wi_addr)+7) );
        printk(KERN_ERR"\tpitch=%08x\n",*((volatile unsigned int*)(wi_addr)+8) );
        printk(KERN_ERR"\t Y/X start=%08x\n",*((volatile unsigned int*)(wi_addr)+9) );
#endif
	}
	printk(KERN_ERR"======================================\n");
    printk(KERN_ERR"w/h: [%d %d]\n", 
            GDMA_DBG_OSD_WI_0_get_width(rtd_inl(GDMA_DBG_OSD_WI_0_reg)),
            GDMA_DBG_OSD_WI_0_get_height(rtd_inl(GDMA_DBG_OSD_WI_0_reg)) );
    printk(KERN_ERR"OSD_WI_1:0x%08x\n", rtd_inl(GDMA_DBG_OSD_WI_1_reg) );
    printk(KERN_ERR"\ttop_addr=%08x\n",rtd_inl(GDMA_DBG_OSD_WI_2_reg));
    printk(KERN_ERR"\tpitch=%08x\n",rtd_inl(GDMA_DBG_OSD_WI_4_reg));


        return;
}
#endif
/*
 * 1. save current OSD buffer
 * 2. GDMA display this buffer and reject all update from AP
 *
 * st[3:0]: 0:disable,  1:enable
 * st[7:4]: 1:OSD1, 2:OSD2, 3:OSD3
 *
 * gdrop_counter is for drop GDMA_ReceivePicture
 *
 */
static int gdrop_counter =0;
static unsigned int gdrop_counter_phy =0;
static unsigned int* gdrop_counter_vir =0;

static void OSD_Freeze(unsigned int st) {
#if IS_ENABLED(CONFIG_RTK_KDRV_SE)
	KGAL_SURFACE_INFO_T src;
	KGAL_SURFACE_INFO_T dst;
	KGAL_RECT_T rect;
	KGAL_BLIT_FLAGS_T flags = KGAL_BLIT_NOFX;
	KGAL_BLIT_SETTINGS_T settings;
#if defined(CONFIG_ARM64)
	volatile unsigned long wi_addr;
#else
	volatile unsigned int wi_addr;
#endif
	unsigned int osd_rbase = 0;


	settings.srcBlend = KGAL_BLEND_ONE;
	settings.dstBlend = KGAL_BLEND_ZERO;

	if((st>>4 &0xf) == 0x1){
		osd_rbase = GDMA_OSD1_CTRL_reg+0x10;
	}else if((st>>4 &0xf) == 0x2){
		osd_rbase = GDMA_OSD2_CTRL_reg+0x10;
	}else if((st>>4 &0xf) == 0x3){
		osd_rbase = GDMA_OSD3_CTRL_reg+0x10;
	}else if((st&0xf) == 0xf) {
		gdrop_counter = (int)((st>>4)&0xfffffff);
		printk(KERN_ERR"[%s]OSD%d drop cnt %d\n",__func__, (st>>4 &0xf), gdrop_counter);
	}else{
		return;
	}

	if((st&0xf) == 1) {
		gdrop_counter = -1;
		gdma_usleep(16000);
		if(rtd_inl(osd_rbase)) {
#if defined(CONFIG_ARM64)
			wi_addr = (volatile unsigned long)phys_to_virt(rtd_inl(osd_rbase));
			src.physicalAddress = (unsigned int)*((volatile unsigned long*)(wi_addr)+6);
#else
			wi_addr = (volatile unsigned int)phys_to_virt(rtd_inl(osd_rbase));
			src.physicalAddress = *((volatile unsigned int*)(wi_addr)+6);
#endif
			src.pitch = dst.pitch = 1920*4;
			src.bpp = dst.bpp = 32;
			src.width = dst.width = 1920;
			src.height = dst.height = 1090;
			src.pixelFormat = dst.pixelFormat = KGAL_PIXEL_FORMAT_ARGB;
			rect.x = 0;
			rect.y = 0;
			rect.w = 1920;
			rect.h = 1090;
		}else{
			gdrop_counter = 0;
			return;
		}
		if(gdrop_counter_vir == NULL) {
			gdrop_counter_vir = (unsigned int*)dvr_malloc_specific(1920*1090*4, GFP_DCU2_FIRST);
			if(gdrop_counter_vir == NULL) {
				printk(KERN_ERR"%s,fail! get memory fail.\n",__func__);
				gdrop_counter = 0;
				return;
			}
			gdrop_counter_phy = dvr_to_phys(gdrop_counter_vir);
		}
		dst.physicalAddress = gdrop_counter_phy;

		KGAL_Blit(&src, &rect, &dst, 0,0, &flags, &settings);
		printk(KERN_ERR"[%s]OSD%d Freeze\n",__func__, (st>>4 &0xf));
		gdma_usleep(5000);
		/*debug, don't care finish*/
		*((volatile unsigned int*)(wi_addr)+6) = gdrop_counter_phy;
		return;
	}else if((st&0xf) == 0) {
		gdrop_counter = 0;
		printk(KERN_ERR"[%s]OSD%d unFreeze\n",__func__, (st>>4 &0xf));
		/*if this func is not for debug,  add sleep here*/
		if(gdrop_counter_vir) {
			dvr_free((void*)(gdrop_counter_vir));
			gdrop_counter_vir = NULL;
			gdrop_counter_phy = 0;
		}
	}
#endif
}
/*
 * OSD pattern
 * we have to save the info from GDAM_receivePicture,
 * if disable osd, we have update the last info.
 * 1. create 1290*720 surface,
 * 2. GAL draw color
 * 3. display
 */

static int gosdpattern =0;
static unsigned int gosdpattern_phy =0;
static unsigned int* gosdpattern_vir =0;
static int used_glastinfo = 0;
static PICTURE_LAYERS_OBJECT glastinfo;

static void OSD_Pattern(KADP_HAL_GAL_CTRL_T* pat) {
#if IS_ENABLED(CONFIG_RTK_KDRV_SE)
	KGAL_SURFACE_INFO_T src;
	KGAL_RECT_T rect;
	KGAL_DRAW_FLAGS_T flags = KGAL_DRAW_NOFX;
	KGAL_DRAW_SETTINGS_T settings;
	PICTURE_LAYERS_OBJECT *patlayer;
	GDMA_PICTURE_OBJECT *picObj;
	osd_planealpha_ar osdcmp_plane_ar;
	osd_planealpha_gb osdcmp_plane_gb;
	gdma_dev *gdma = &gdma_devices[0];
	VO_RECTANGLE disp_res;
	osdcmp_plane_ar.plane_alpha_r = 0xff;
	osdcmp_plane_ar.plane_alpha_a = 0xff;
	osdcmp_plane_gb.plane_alpha_b = 0xff;
	osdcmp_plane_gb.plane_alpha_g = 0xff;
	getDispSize(&disp_res);
	settings.srcBlend = KGAL_BLEND_ONE;
	settings.dstBlend = KGAL_BLEND_ZERO;

	if(pat->en == 0) {
		/*update last*/
		printk(KERN_ERR"%s %d,%d,%08x\n",
				__func__, pat->en, pat->osdn, pat->clr);
		gosdpattern = 0;
		GDMA_ReceivePicture(&glastinfo);
		gdma_usleep(16000);
		if(gosdpattern_vir) {
			dvr_free((void*)(gosdpattern_vir));
			gosdpattern_vir = NULL;
			gosdpattern_phy = 0;
		}
	}else if(pat->en == 1) {
		if(gosdpattern_vir == NULL) {
			gosdpattern_vir = (unsigned int*)dvr_malloc_specific(1280*724, GFP_DCU1_FIRST);
			if(gosdpattern_vir == NULL) {
				printk(KERN_ERR"%s,fail! get memory fail.\n",__func__);
				return;
			}
			gosdpattern_phy = dvr_to_phys(gosdpattern_vir);
		}


		printk(KERN_ERR"%s get addr=%08x  \n", __func__, gosdpattern_phy);
		gosdpattern = 1;

		src.physicalAddress = gosdpattern_phy;
		src.pitch = 1280*4;
		src.bpp = 32;
		src.width = 1280;
		src.height = 724/4;
		src.pixelFormat = KGAL_PIXEL_FORMAT_ARGB;
		rect.x = 0;
		rect.y = 0;
		rect.w = 1280;
		rect.h = 724/4;
		KGAL_FillRectangle(&src, &rect, 0, &flags, &settings);
		gdma_usleep(2000);

		patlayer =(PICTURE_LAYERS_OBJECT*)(gosdpattern_vir + 1280*720/4);
		//ARGB -> ABGR
		*(gdma->OSD_CLUT[0]) = (pat->clr&0xff000000) |
			(pat->clr&0xff0000) >>16 |
			(pat->clr&0xff00)|
			(pat->clr&0xff) << 16 ;
		printk(KERN_ERR"LUT8 addr = %p\n",gdma->OSD_CLUT[0]);
		picObj = &patlayer->layer[0].normal[0];
		picObj->format = VO_OSD_COLOR_FORMAT_8BIT;
		picObj->plane = (VO_GRAPHIC_PLANE)(pat->osdn&0x3?pat->osdn&0x3:1);
		patlayer->layer[0].onlineOrder = C0;
		patlayer->layer_num = 1;
		patlayer->layer[0].normal_num = 1;
		picObj->src_type = SRC_NORMAL;
		picObj->alpha = (pat->clr >> 24) &0xff;
		picObj->width = 1280;
		picObj->height = 720;
		picObj->address = gosdpattern_phy;
		picObj->pitch = 1280;
		picObj->layer_used = 1;

		picObj->plane_ar = osdcmp_plane_ar;
		picObj->plane_gb = osdcmp_plane_gb;
		picObj->show = 1;
		picObj->paletteformat = 1; /* rgba format */
		picObj->syncInfo.syncstamp = 0;
		picObj->scale_factor = 1;

		picObj->dst_width = disp_res.width;
		picObj->dst_height = disp_res.height;
		GDMA_ReceivePicture(patlayer);

	}
#endif
}

static int OSD_debug_vflip(unsigned int en) {
	gdma_dev *gdma = &gdma_devices[0];
	int osdn=1;
	unsigned int offset = 0x100;
	unsigned int regbase = GDMA_OSD1_CTRL_reg;
	GDMA_WIN* winfo = NULL;

	//GDMA_WIN* winfo_rotate = NULL;
	//GDMA_PIC_DATA *pic = NULL;

	if(gdma->ctrl.enableVFlip == en){
		//printk(KERN_ERR"[OSD]VFlip = %s\n",en?"Enable":"Disable");
		return 0;
	}
	gdma->ctrl.enableVFlip = en;
	printk(KERN_ERR"[OSD]change VFlip = %s\n",en?"Enable":"Disable");
	if(en)
		rtd_setbits(SFG_SFG_CTRL_0_reg,SFG_SFG_CTRL_0_h_flip_en_mask);
	else
		rtd_clearbits(SFG_SFG_CTRL_0_reg,SFG_SFG_CTRL_0_h_flip_en_mask);

	/* progdone all OSDs that has win info */
	for(osdn=1;osdn < GDMA_PLANE_MAXNUM;osdn++) {
		if(rtd_inl(regbase)&GDMA_OSD1_CTRL_osd1_en_mask &&
			rtd_inl(regbase+0x10)) {
			if(en) {
				rtd_setbits(regbase,GDMA_OSD1_CTRL_rotate_mask |
					GDMA_OSD1_CTRL_osd1_en_mask |
					GDMA_OSD1_CTRL_write_data_mask);
			}else {

				rtd_outl(regbase,GDMA_OSD1_CTRL_rotate_mask);
			}

			winfo = (GDMA_WIN*)phys_to_virt(rtd_inl(regbase+0x10));
			if(winfo->attr.IMGcompress) {
				//FBDC...
			}else{
				//pic = gdma->pic[osdn-1] + ((gdma->picQwr[osdn-1] -1) & (GDMA_MAX_PIC_Q_SIZE - 1));
				//winfo_rotate = &pic->OSDwin;
				//memcpy(winfo_rotate, winfo,sizeof(GDMA_WIN));

				//printk(KERN_ERR"old winfo = %p new is %p\n",winfo,winfo_rotate );
				if(en) {
					//winfo_rotate->top_addr +=winfo->pitch*winfo->winWH.height;
					winfo->top_addr +=winfo->pitch*winfo->winWH.height;
				}else {
					//winfo_rotate->top_addr -=winfo->pitch*winfo->winWH.height;
					winfo->top_addr -=winfo->pitch*winfo->winWH.height;
				}

				GDMA_Update(gdma, 1);
				//if(winfo_rotate->top_addr == winfo->top_addr)
				//	rtd_outl(regbase+0x10,GDMA_OSD1_WI_addr(virt_to_phys(winfo)));
				//else
				//	rtd_outl(regbase+0x10,GDMA_OSD1_WI_addr(virt_to_phys(winfo_rotate)));
				//gdma->picQwr[osdn-1] +=1;
			}
		}
		regbase +=offset;
	}
	rtd_outl(GDMA_CTRL_reg, 0xf);
	return 0;
}

/*============================================================
 *	inline func
*/

static void gdma_to_dump(void) {
#if 0
	unsigned int offset = 0;
	int ret=-1;
	if(osd_dump_state == -1 || (GDMA_DISPLAY_PLANE)osd_dump_state >= GDMA_PLANE_MAXNUM ){
		return;
	}

	if ((GDMA_DISPLAY_PLANE)osd_dump_state == GDMA_PLANE_OSD2)
		offset = 0x100;
	else if ((GDMA_DISPLAY_PLANE)osd_dump_state == GDMA_PLANE_OSD3)
		offset = 0x200;


	if(osd_dump_state) {
		volatile unsigned int *ptr;
		volatile unsigned int addr_tmp;
		osd_dmem_info pinfo;
		if(rtd_inl(GDMA_OSD1_WI_reg+offset)) {
			addr_tmp = (unsigned int)phys_to_virt(rtd_inl(GDMA_OSD1_WI_reg+offset));
			ptr = (unsigned int *)addr_tmp;
			ptr += 6;
			pinfo.src_phyaddr = *ptr;
			pinfo.pitch = 1280*4;
			ret = osd_dump_agent_add_task_to_queue(&pinfo);
			if(ret)
				osd_dump_state = -1;
		}
	}
#endif
}

static void gdma_config_fbc(void) {
#if 0
	online_RBUS online_reg;
	dma_swap_RBUS dma_swap_reg;

	/* balance with tv_sb3 and tv_sb4 */
	/* 1st: FBDC DMA for OSD1/4, but OSD2. OSD2 just for mouse */
	online_reg.regValue = rtd_inl(GDMA_ONLINE_reg);
	online_reg.set_osd1_fbdc1_use_2nd_dma = 0;  /*  tv_sb4 */
	online_reg.set_osd1_fbdc2_use_2nd_dma = 0;  /*  tv_sb4 */
	online_reg.set_osd1_fbdc3_use_2nd_dma = 0;  /*  tv_sb4 */
	online_reg.set_osd2_fbdc1_use_2nd_dma = 0;  /*  tv_sb4 */
	online_reg.set_osd2_fbdc2_use_2nd_dma = 0;  /*  tv_sb4 */
	online_reg.set_osd2_fbdc3_use_2nd_dma = 0;  /*  tv_sb4 */
	online_reg.set_osd3_fbdc1_use_2nd_dma = 1;  /*  tv_sb3 */
	online_reg.set_osd3_fbdc2_use_2nd_dma = 1;  /*  tv_sb3 */
	online_reg.set_osd3_fbdc3_use_2nd_dma = 1;  /*  tv_sb3 */
	/* 2nd: online/offline DMA */
	/*  online use offline DMA
	online_reg.set_osd4_online_use_offline_dma = 1; */
	rtd_outl(GDMA_ONLINE_reg, online_reg.regValue);

	/* for FBDC dma swap */
	dma_swap_reg.regValue = rtd_inl(GDMA_DMA_SWAP_reg);
	dma_swap_reg.fbdc1_1b = 1;
	dma_swap_reg.fbdc1_2b = 1;
	dma_swap_reg.fbdc1_4b = 1;
	dma_swap_reg.fbdc1_8b = 1;
	dma_swap_reg.fbdc2_1b = 1;
	dma_swap_reg.fbdc2_2b = 1;
	dma_swap_reg.fbdc2_4b = 1;
	dma_swap_reg.fbdc2_8b = 1;
	rtd_outl(GDMA_DMA_SWAP_reg, dma_swap_reg.regValue);
#else
	gdma_dma_swap_RBUS dma_swap_reg;
	dma_swap_reg.regValue = rtd_inl(GDMA_DMA_SWAP_reg);
	dma_swap_reg.tfbc1_1b = 1;
	dma_swap_reg.tfbc1_2b = 1;
	dma_swap_reg.tfbc1_4b = 1;
	dma_swap_reg.tfbc1_8b = 1;
	dma_swap_reg.tfbc2_1b = 1;
	dma_swap_reg.tfbc2_2b = 1;
	dma_swap_reg.tfbc2_4b = 1;
	dma_swap_reg.tfbc2_8b = 1;
	
	rtd_outl(GDMA_DMA_SWAP_reg, dma_swap_reg.regValue);
#endif
}

static int GDMA_ConfigOSD1OSD4MixerOrder(bool flag)
{
    //RTK mark for netfix
	enable_osd1osd3mixerorder = flag;
	return 0;
}

static void gdma_config_line_buffer(void) {
	gdma_line_buffer_sta_RBUS line_buffer_sta_reg;
	gdma_line_buffer_size_RBUS line_buffer_size_reg;
	gdma_line_buffer_end_RBUS line_buffer_end_reg;

	ui_re = get_ui_resolution();

	/* OSD_SR switch */
	line_buffer_end_reg.regValue = rtd_inl(GDMA_line_buffer_end_reg);
#if defined(CONFIG_ARCH_RTK288O) || defined(CONFIG_ARCH_RTK2875) || defined(CONFIG_ARCH_RTK2885M) || defined(CONFIG_ARCH_RTK6748)
#else
	line_buffer_end_reg.osdsr1_switch_to = GDMA_PLANE_OSD1;
	line_buffer_end_reg.osdsr2_switch_to = GDMA_PLANE_OSD3;
	line_buffer_end_reg.osdsr3_switch_to = GDMA_PLANE_OSD2; /*maxinum 256*256*/
#endif
	rtd_outl(GDMA_line_buffer_end_reg, line_buffer_end_reg.regValue);

	/* line buffer setting */
	line_buffer_sta_reg.regValue = rtd_inl(GDMA_line_buffer_sta_reg);
	line_buffer_size_reg.regValue = rtd_inl(GDMA_line_buffer_size_reg);
#if 0
	if(fbcOnOff == FBC_ON){
		line_buffer_sta_reg.l1 = 0;
		line_buffer_sta_reg.l4 = 2;
		line_buffer_sta_reg.l5 = 2;
		line_buffer_size_reg.l1 = 2;
		line_buffer_size_reg.l4 = 0;
		line_buffer_size_reg.l5 = 2;		
	}else{
		line_buffer_sta_reg.l1 = 0;
		line_buffer_sta_reg.l4 = 0;
	        line_buffer_sta_reg.l5 = 2;
                line_buffer_size_reg.l1 = 0;
	        line_buffer_size_reg.l4 = 2;
	        line_buffer_size_reg.l5 = 2;
	}
#else
	if(ui_re == UI_RESOLUTION_4K)
	{
		line_buffer_sta_reg.l1 = 0;
		line_buffer_sta_reg.l2 = 4;
	}else{
        line_buffer_sta_reg.l1 = 0;
	line_buffer_sta_reg.l2 = 2;
	}
	line_buffer_sta_reg.l4 = 4;
	line_buffer_sta_reg.l5 = 5;

  	if(ui_re == UI_RESOLUTION_4K)
  	{
	  	line_buffer_size_reg.l1 = 4;
	  	line_buffer_size_reg.l2 = 0;
  	}else{
	line_buffer_size_reg.l1 = 2;
	line_buffer_size_reg.l2 = 2;
  	}
	line_buffer_size_reg.l4 = 1;
	line_buffer_size_reg.l5 = 1;
#endif
	rtd_outl(GDMA_line_buffer_sta_reg, line_buffer_sta_reg.regValue);
	rtd_outl(GDMA_line_buffer_size_reg, line_buffer_size_reg.regValue);
}

static void gdma_config_line_buffer_afbc(int fbcOnOff) {

	gdma_line_buffer_sta_RBUS line_buffer_sta_reg;
	gdma_line_buffer_size_RBUS line_buffer_size_reg;
	ui_re = get_ui_resolution();


	/* line buffer setting */
	line_buffer_sta_reg.regValue = rtd_inl(GDMA_line_buffer_sta_reg);
	line_buffer_size_reg.regValue = rtd_inl(GDMA_line_buffer_size_reg);
#if 0
	if(fbcOnOff == FBC_ON){
		line_buffer_sta_reg.l1 = 0;
		line_buffer_sta_reg.l4 = 2;
		line_buffer_sta_reg.l5 = 3;
		line_buffer_size_reg.l1 = 2;
		line_buffer_size_reg.l4 = 1;
		line_buffer_size_reg.l5 = 1;		
	}else{
		line_buffer_sta_reg.l1 = 0;
		line_buffer_sta_reg.l4 = 0;
		line_buffer_sta_reg.l5 = 2;
		line_buffer_size_reg.l1 = 0;
		line_buffer_size_reg.l4 = 2;
		line_buffer_size_reg.l5 = 2;
    }
#else
		if(ui_re == UI_RESOLUTION_4K)
		{
			line_buffer_sta_reg.l1 = 0;
			line_buffer_sta_reg.l2 = 4;
			line_buffer_sta_reg.l4 = 4;
			line_buffer_sta_reg.l5 = 5;
			line_buffer_size_reg.l1 = 4;
			line_buffer_size_reg.l2 = 0;
			line_buffer_size_reg.l4 = 1;
			line_buffer_size_reg.l5 = 1;
		}else{
        line_buffer_sta_reg.l1 = 0;
        line_buffer_sta_reg.l2 = 2;
        line_buffer_sta_reg.l4 = 4;
        line_buffer_sta_reg.l5 = 5;
        line_buffer_size_reg.l1 = 2;
        line_buffer_size_reg.l2 = 2;
        line_buffer_size_reg.l4 = 1;
        line_buffer_size_reg.l5 = 1;
		}
#endif

	rtd_outl(GDMA_line_buffer_sta_reg, line_buffer_sta_reg.regValue);
	rtd_outl(GDMA_line_buffer_size_reg, line_buffer_size_reg.regValue);
}

static void gdma_config_mid_blend(void) {
#if defined(CONFIG_ARCH_RTK287O) || defined(CONFIG_ARCH_RTK288O) || defined(CONFIG_ARCH_RTK2875) || defined(CONFIG_ARCH_RTK2885M) || defined(CONFIG_ARCH_RTK6748)
#else
	gdma_mid_blend_RBUS mid_blend_reg;
#endif
	osdovl_mixer_b1_RBUS mixer_bx_reg;
	osdovl_osd_db_ctrl_RBUS osd_db_ctrl_reg;

#if defined(CONFIG_ARCH_RTK287O) || defined(CONFIG_ARCH_RTK288O) || defined(CONFIG_ARCH_RTK2875) || defined(CONFIG_ARCH_RTK2885M) || defined(CONFIG_ARCH_RTK6748)
#else
	/* mid-blend order */
	mid_blend_reg.regValue = rtd_inl(GDMA_mid_blend_reg);
	mid_blend_reg.c0_sel = GDMA_PLANE_OSD1;
	mid_blend_reg.c1_sel = GDMA_PLANE_OSD2;
	mid_blend_reg.c2_sel = GDMA_PLANE_OSD3;
	rtd_outl(GDMA_mid_blend_reg, mid_blend_reg.regValue);
#endif

	/* mixer blend factor & mid-blend blend factor & offline blend factor */
	mixer_bx_reg.k1 = 0xc;
	mixer_bx_reg.k2 = 0x3;
	mixer_bx_reg.k3 = 0x2d;
	mixer_bx_reg.k4 = 0x3;
	rtd_outl(OSDOVL_Mixer_b1_reg, mixer_bx_reg.regValue);
	rtd_outl(OSDOVL_Mixer_b2_reg, mixer_bx_reg.regValue);
	rtd_outl(OSDOVL_Mixer_b3_reg, mixer_bx_reg.regValue);

#if defined(CONFIG_ARCH_RTK287O) || defined(CONFIG_ARCH_RTK288O) || defined(CONFIG_ARCH_RTK2875) || defined(CONFIG_ARCH_RTK2885M) || defined(CONFIG_ARCH_RTK6748)
#else
	rtd_outl(GDMA_mid_blend_b1_reg, mixer_bx_reg.regValue);
	rtd_outl(GDMA_mid_blend_b2_reg, mixer_bx_reg.regValue);
#endif

	mixer_bx_reg.k1 = 0x1;
	mixer_bx_reg.k2 = 0x4;
	mixer_bx_reg.k3 = 0x1;
	mixer_bx_reg.k4 = 0x4;
	rtd_outl(OSDOVL_Mixer_b3_reg, mixer_bx_reg.regValue);

	/* mixer double buffer */
	osd_db_ctrl_reg.regValue = rtd_inl(OSDOVL_OSD_Db_Ctrl_reg);
	osd_db_ctrl_reg.db_read = 1;	/* 1: see rbus setting temporarily, 0: see HW latch now  */
	osd_db_ctrl_reg.db_en = 1;
	osd_db_ctrl_reg.db_load = 1;
	rtd_outl(OSDOVL_OSD_Db_Ctrl_reg, osd_db_ctrl_reg.regValue);

}


static bool osd_shift_proc(void) {
	/*apply current OSD display*/
	GDMA_WIN *win;
	gdma_dev *gdma = &gdma_devices[0];

	if (g_osdshift_ctrl.plane == VO_GRAPHIC_OSD1)
		win = (GDMA_WIN *)phys_to_virt(rtd_inl(GDMA_OSD1_WI_reg));
	else if (g_osdshift_ctrl.plane == VO_GRAPHIC_OSD2)
		win = (GDMA_WIN *)phys_to_virt(rtd_inl(GDMA_OSD2_WI_reg));
	else if (g_osdshift_ctrl.plane == VO_GRAPHIC_OSD3)
		win = (GDMA_WIN *)phys_to_virt(rtd_inl(GDMA_OSD3_WI_reg));
	else {
		/*add printk*/
		memset(&g_osdshift_ctrl, 0x00, sizeof(CONFIG_OSDSHIFT_STRUCT));
		return false;
	}

	win = (GDMA_WIN *)phys_to_virt(rtd_inl(GDMA_OSD1_WI_reg));

	if (win == 0 || gdma == 0) {
		/*add printk*/
		return false;
	}

	down(&gdma->sem_receive);

	if (win->attr.IMGcompress == 0) {
		if (g_osdshift_ctrl.shift_h_enable) {
			win->winWH.width -= abs(g_osdshift_ctrl.h_shift_pixel);
			if (g_osdshift_ctrl.h_shift_pixel > 0)	// shift right
				win->winXY.x += g_osdshift_ctrl.h_shift_pixel;
			else	// shift left
				win->objOffset.objXoffset += abs(g_osdshift_ctrl.h_shift_pixel);
		}
		if (g_osdshift_ctrl.shift_v_enable) {
			win->winWH.height -= abs(g_osdshift_ctrl.v_shift_pixel);
			if (g_osdshift_ctrl.v_shift_pixel > 0)	// shift down
				win->winXY.y += g_osdshift_ctrl.v_shift_pixel;
			else	// shift up
				win->objOffset.objYoffset += abs(g_osdshift_ctrl.v_shift_pixel);
		}
	} else {
#if defined(CONFIG_ARCH_RTK287O) || defined(CONFIG_ARCH_RTK288O) || defined(CONFIG_ARCH_RTK2875) || defined(CONFIG_ARCH_RTK2885M) || defined(CONFIG_ARCH_RTK6748)
#else
		if (g_osdshift_ctrl.shift_h_enable) {
			if (win->fbdc_attr.fbdc_1st_en) {
				win->fbdc1_xy_pixel.x_pic_pixel -= abs(g_osdshift_ctrl.h_shift_pixel);
				if (g_osdshift_ctrl.h_shift_pixel > 0)	// shift right
					win->fbdc1_preblend_xy_sta.x_sta += g_osdshift_ctrl.h_shift_pixel;
				else	// shift left
					win->fbdc1_xy_sta.x_sta += abs(g_osdshift_ctrl.h_shift_pixel);
			}
			if (win->fbdc_attr.fbdc_2nd_en) {
				win->fbdc2_xy_pixel.x_pic_pixel -= abs(g_osdshift_ctrl.h_shift_pixel);
				if (g_osdshift_ctrl.h_shift_pixel > 0)	// shift right
					win->fbdc2_preblend_xy_sta.x_sta += g_osdshift_ctrl.h_shift_pixel;
				else	// shift left
					win->fbdc2_xy_sta.x_sta += abs(g_osdshift_ctrl.h_shift_pixel);
			}
			if (win->fbdc_attr.fbdc_3rd_en) {
				win->fbdc3_xy_pixel.x_pic_pixel -= abs(g_osdshift_ctrl.h_shift_pixel);
				if (g_osdshift_ctrl.h_shift_pixel > 0)	// shift right
					win->fbdc3_preblend_xy_sta.x_sta += g_osdshift_ctrl.h_shift_pixel;
				else	// shift left
					win->fbdc3_xy_sta.x_sta += abs(g_osdshift_ctrl.h_shift_pixel);
			}
		}
		if (g_osdshift_ctrl.shift_v_enable) {
			if (win->fbdc_attr.fbdc_1st_en) {
				win->fbdc1_xy_pixel.y_pic_pixel -= abs(g_osdshift_ctrl.v_shift_pixel);
				if (g_osdshift_ctrl.v_shift_pixel > 0)	// shift down
					win->fbdc1_preblend_xy_sta.y_sta += g_osdshift_ctrl.v_shift_pixel;
				else	// shift up
					win->fbdc1_xy_sta.y_sta += abs(g_osdshift_ctrl.v_shift_pixel);
			}
			if (win->fbdc_attr.fbdc_2nd_en) {
				win->fbdc2_xy_pixel.y_pic_pixel -= abs(g_osdshift_ctrl.v_shift_pixel);
				if (g_osdshift_ctrl.v_shift_pixel > 0)	// shift down
					win->fbdc2_preblend_xy_sta.y_sta += g_osdshift_ctrl.v_shift_pixel;
				else	// shift up
					win->fbdc2_xy_sta.y_sta += abs(g_osdshift_ctrl.v_shift_pixel);
			}
			if (win->fbdc_attr.fbdc_3rd_en) {
				win->fbdc3_xy_pixel.y_pic_pixel -= abs(g_osdshift_ctrl.v_shift_pixel);
				if (g_osdshift_ctrl.v_shift_pixel > 0)	// shift down
					win->fbdc3_preblend_xy_sta.y_sta += g_osdshift_ctrl.v_shift_pixel;
				else	// shift up
					win->fbdc3_xy_sta.y_sta += abs(g_osdshift_ctrl.v_shift_pixel);
			}
		}
#endif
	}

#if !defined(CONFIG_ARM64)
	flush_cache_all();
#endif
	up(&gdma->sem_receive);
	GDMA_OSD1OnTop(true);

	return true;
}

/*
 * a.k.a  OSD dump
 * if type == 0, dump one frame to file
 * if type == 1, stretch dump 25 frames to file .
 */
int gdma_osd_writedump(unsigned int type, unsigned int osdn, osd_dmem_info* pinfo,osd_dmem_info* pinfo_stch) {
	//gdma_dev *gdma = &gdma_devices[0];
	unsigned int offset = 0x100;
	unsigned int regbase = GDMA_OSD1_CTRL_reg;
	GDMA_WIN* winfo = NULL;
	int cntw = 0;
	int cnth = 0;
	osd_dmem_info dinfo;
	unsigned int mixercrc = 0;
	unsigned int timeoutcnt=0;

	regbase += (osdn-1)*offset;

	if(rtd_inl(regbase)&GDMA_OSD1_CTRL_osd1_en_mask &&
		rtd_inl(regbase+0x10)) {
		winfo = (GDMA_WIN*)phys_to_virt(rtd_inl(regbase+0x10));
	}else{
		printk(KERN_ERR"OSD-%d is empty\n",osdn);
		return -1;
	}
#if 0
	/*need to handle compression data?*/
	if(winfo->attr.type != 0x7){
		printk(KERN_ERR"OSD-%d isn't ARGB8888\n",osdn);
		return -1;
	}
#endif


	pinfo->src_phyaddr = winfo->top_addr;

	pinfo->x = 0;
	pinfo->y = 0;
	pinfo->w = winfo->winWH.width;
	pinfo->h = winfo->winWH.height;
	pinfo->pitch = winfo->pitch;

	if (winfo->attr.IMGcompress)
		pinfo->fmt = KGAL_PIXEL_FORMAT_FBC;
	else
		pinfo->fmt = KGAL_PIXEL_FORMAT_ARGB;

	if(pinfo->dst_phyaddr == 0){
		pinfo->viraddr = (unsigned char*)dvr_malloc_specific(winfo->pitch*winfo->winWH.height+0x36 , GFP_DCU1_FIRST);
		if(pinfo->viraddr == 0) {
			printk(KERN_ERR"%s,fail! get memory fail.\n",__func__);
			return -1;
		}else{
			pinfo->dst_phyaddr = dvr_to_phys(pinfo->viraddr);
		}
	}

	if(type == 0) {
		osd_deme_cpy(pinfo);
	}else{
		dinfo.x = 0;
		dinfo.y = 0;
		dinfo.w = winfo->winWH.width/5;
		dinfo.h = winfo->winWH.height/5;
		dinfo.pitch = winfo->pitch;
		rtd_outl(OSDOVL_Mixer_CRC_Ctrl_reg, OSDOVL_Mixer_CRC_Ctrl_crc_start(1) | OSDOVL_Mixer_CRC_Ctrl_crc_conti(1) | OSDOVL_Mixer_CRC_Ctrl_mixer_crc_sel(osdn));
		mixercrc = rtd_inl(OSDOVL_Mixer_CRC_Result_reg);

		for(cnth=0;cnth <5;cnth++) {
			dinfo.y = (winfo->winWH.height/5)*cnth;
			for(cntw=0;cntw <5;cntw++) {
				dinfo.x = (winfo->winWH.width/5)*cntw;
				while(rtd_inl(OSDOVL_Mixer_CRC_Result_reg) == mixercrc) {
					gdma_usleep(1000);
					timeoutcnt++;
					if(timeoutcnt >=16) {
						timeoutcnt=0;
						break;
					}
				}
				winfo = (GDMA_WIN*)phys_to_virt(rtd_inl(regbase+0x10));
				pinfo->src_phyaddr = winfo->top_addr;
				osd_deme_stch_cpy(pinfo,&dinfo);
				mixercrc = rtd_inl(OSDOVL_Mixer_CRC_Result_reg);
				printk(KERN_ERR"CRC change..%08x\n",mixercrc);
			}

		}
	}
	osd_deme_write_file(pinfo);
	printk(KERN_INFO"%s , dump osd=%d, path = %s\n",
		__func__, osdn, pinfo->fname);
	if(pinfo->viraddr)
		dvr_free((void*)(pinfo->viraddr));
	pinfo->viraddr = NULL;
	return 0;
}

/*
 * a.k.a  OSD dump to mem
 */
int gdma_osd_dmp2mem(KADP_HAL_GAL_CTRL_T *data) {
#if IS_ENABLED(CONFIG_RTK_KDRV_SE)
	KGAL_SURFACE_INFO_T src_sf;
	KGAL_RECT_T src_rect;
	KGAL_SURFACE_INFO_T dst_sf;
	KGAL_RECT_T dst_rect;
	KGAL_BLIT_FLAGS_T flags = KGAL_BLIT_NOFX;
	KGAL_BLIT_SETTINGS_T settings;
	unsigned int regbase = GDMA_OSD1_CTRL_reg;
	GDMA_WIN* winfo = NULL;
	regbase += ((data->osdn-1)*0x100);

	memset(&src_sf, 0, sizeof(KGAL_SURFACE_INFO_T));
	memset(&dst_sf, 0, sizeof(KGAL_SURFACE_INFO_T));
	memset(&src_rect, 0, sizeof(KGAL_RECT_T));
	memset(&dst_rect, 0, sizeof(KGAL_RECT_T));

	settings.srcBlend = KGAL_BLEND_ONE;
	settings.dstBlend = KGAL_BLEND_ZERO;

	if(rtd_inl(regbase)&GDMA_OSD1_CTRL_osd1_en_mask &&
		rtd_inl(regbase+0x10)) {
		winfo = (GDMA_WIN*)phys_to_virt(rtd_inl(regbase+0x10));
	}else{
		printk(KERN_ERR"OSD-%d is empty\n",data->osdn);
		return -1;
	}
	if(winfo->attr.type != 0x7){
		printk(KERN_ERR"OSD-%d isn't ARGB8888\n",data->osdn);
		return -1;
	}
	if((data->capinfo.x + data->capinfo.w >winfo->winWH.width) ||
		(data->capinfo.y + data->capinfo.h >winfo->winWH.height)){

		printk(KERN_ERR"[%s]cap out of range ,surface=[%d,%d], cap=[%d,%d,%d,%d]\n",
			__func__,winfo->winWH.width ,winfo->winWH.height,
			data->capinfo.x, data->capinfo.y ,
			data->capinfo.w, data->capinfo.h);
		return -1;
	}

	src_sf.physicalAddress = winfo->top_addr;
	src_sf.pitch = winfo->pitch;
	src_sf.bpp = 32;
	src_sf.width = winfo->winWH.width;
	src_sf.height = winfo->winWH.height;
	src_sf.pixelFormat = KGAL_PIXEL_FORMAT_ARGB;


	dst_sf.physicalAddress = data->stchinfo.phyaddr;
	dst_sf.pitch = data->stchinfo.pitch;
	dst_sf.width = data->stchinfo.w;
	dst_sf.height = data->stchinfo.h;
	if(data->stchinfo.fmt == KGAL_PIXEL_FORMAT_ARGB) {
		dst_sf.bpp = 32;
		dst_sf.pixelFormat = KGAL_PIXEL_FORMAT_ARGB;
	}else if(data->stchinfo.fmt == KGAL_PIXEL_FORMAT_RGB888){
		dst_sf.bpp = 24;
		dst_sf.pixelFormat = KGAL_PIXEL_FORMAT_RGB888;
	}else{
		printk(KERN_ERR"[%s]error!, unsupport FMT = %d\n",__func__, data->stchinfo.fmt);
		return -1;
	}


	src_rect.x=data->capinfo.x;
	src_rect.y=data->capinfo.y;
	src_rect.w=data->capinfo.w;
	src_rect.h=data->capinfo.h;

	dst_rect.x=data->stchinfo.x;
	dst_rect.y=data->stchinfo.y;
	dst_rect.w=data->stchinfo.w;
	dst_rect.h=data->stchinfo.h;

	KGAL_StretchBlit(&src_sf,&src_rect,&dst_sf,&dst_rect,
		&flags,&settings);

#endif
	return 0;
}

static int gdma_osd_winfo_dump(void) {
	unsigned int offset = 0x100;
	unsigned int regbase = GDMA_OSD1_CTRL_reg;
	unsigned int regvalue = 0;
	int osdn = 1;
	GDMA_WIN* winfo = NULL;
	VO_RECTANGLE res;
	getDispSize(&res);
	printk(KERN_ERR"====panel info : [%d*%d]\n",res.width,res.height);

	for(osdn=1;osdn<GDMA_PLANE_MAXNUM;osdn++) {
		winfo = NULL;
		regbase += (osdn-1)*offset;
        regvalue = rtd_inl(regbase+0x10);
		if(regvalue)
			winfo = (GDMA_WIN*)phys_to_virt(regvalue);
		if(winfo == NULL) {
			printk(KERN_ERR"\n====OSD-%d is None\n\n",osdn);
			continue;
		}
		regvalue =rtd_inl(regbase);
		printk(KERN_ERR"====OSD-%d info===\n",osdn);
		printk(KERN_ERR"Enable = %d, VFlip = %d\n",
			GDMA_OSD1_CTRL_get_osd1_en(regvalue)?1:0,
			GDMA_OSD1_CTRL_get_rotate(regvalue)?1:0);
		printk(KERN_ERR"winfo addr= (%p)\n", winfo);
		printk(KERN_ERR"winfo canvas= (%d,%d,%d,%d), offsetXY=(%d,%d)\n",
			winfo->winXY.x,winfo->winXY.y,
			winfo->winWH.width,winfo->winWH.height,
			winfo->objOffset.objXoffset,winfo->objOffset.objYoffset);
		printk(KERN_ERR"\twinfo pitch= (%d) addr=%08x\n",
			winfo->pitch,winfo->top_addr);
		printk(KERN_ERR"\tFormat=%x Compress = %d\n",
			winfo->attr.type,
			winfo->attr.IMGcompress?1:0);
	}


	if(OSD_SR_OSD_SR_1_Scaleup_Ctrl1_get_h_zoom_en(rtd_inl(OSD_SR_OSD_SR_1_Scaleup_Ctrl1_reg))) {
		printk(KERN_ERR"\n====OSDSR-1 : X=%d, W=%d\n",
		OSD_SR_OSD_SR_1_H_Location_Ctrl_get_osd_h_sta(rtd_inl(OSD_SR_OSD_SR_1_H_Location_Ctrl_reg)),
		OSD_SR_OSD_SR_1_H_Location_Ctrl_get_osd_width(rtd_inl(OSD_SR_OSD_SR_1_H_Location_Ctrl_reg)));
	}

	if(OSD_SR_OSD_SR_1_Scaleup_Ctrl0_get_v_zoom_en(rtd_inl(OSD_SR_OSD_SR_1_Scaleup_Ctrl0_reg))) {
		printk(KERN_ERR"\n====OSDSR-1 : Y=%d, H=%d\n",
		OSD_SR_OSD_SR_1_V_Location_Ctrl_get_osd_v_sta(rtd_inl(OSD_SR_OSD_SR_1_V_Location_Ctrl_reg)),
		OSD_SR_OSD_SR_1_V_Location_Ctrl_get_osd_height(rtd_inl(OSD_SR_OSD_SR_1_V_Location_Ctrl_reg)));
	}

	if(OSD_SR_OSD_SR_2_Scaleup_Ctrl1_get_h_zoom_en(rtd_inl(OSD_SR_OSD_SR_2_Scaleup_Ctrl1_reg))) {
		printk(KERN_ERR"\n====OSDSR-2 : X=%d, W=%d\n",
		OSD_SR_OSD_SR_2_H_Location_Ctrl_get_osd_h_sta(rtd_inl(OSD_SR_OSD_SR_2_H_Location_Ctrl_reg)),
		OSD_SR_OSD_SR_2_H_Location_Ctrl_get_osd_width(rtd_inl(OSD_SR_OSD_SR_2_H_Location_Ctrl_reg)));
	}

	if(OSD_SR_OSD_SR_2_Scaleup_Ctrl0_get_v_zoom_en(rtd_inl(OSD_SR_OSD_SR_2_Scaleup_Ctrl0_reg))) {
		printk(KERN_ERR"\n====OSDSR-2 : Y=%d, H=%d\n",
		OSD_SR_OSD_SR_2_V_Location_Ctrl_get_osd_v_sta(rtd_inl(OSD_SR_OSD_SR_2_V_Location_Ctrl_reg)),
		OSD_SR_OSD_SR_2_V_Location_Ctrl_get_osd_height(rtd_inl(OSD_SR_OSD_SR_2_V_Location_Ctrl_reg)));
	}

	return 0;
}

static int gdma_osd_winfo_read(KADP_HAL_GAL_CTRL_T *data) {
	unsigned int offset = 0x100;
	unsigned int regbase = GDMA_OSD1_CTRL_reg;
	unsigned int regvalue = 0;
	GDMA_WIN* winfo = NULL;
	regbase += (data->osdn-1)*offset;
	regvalue = rtd_inl(regbase+0x10);

	data->capinfo.x = 0;
	data->capinfo.y = 0;
	data->capinfo.w = 0;
	data->capinfo.h = 0;

	data->stchinfo.x = 0;
	data->stchinfo.y = 0;
	data->stchinfo.w = 0;
	data->stchinfo.h = 0;

	if(regvalue)
		winfo = (GDMA_WIN*)phys_to_virt(regvalue);
	if(winfo == NULL)
		return 0;

	data->capinfo.x = winfo->winXY.x;
	data->capinfo.y = winfo->winXY.y;
	data->capinfo.w = winfo->winWH.width;
	data->capinfo.h = winfo->winWH.height;

	/*ToDo: need to check the pair of OSD and SR*/

	if(OSD_SR_OSD_SR_1_Scaleup_Ctrl1_get_h_zoom_en(rtd_inl(OSD_SR_OSD_SR_1_Scaleup_Ctrl1_reg))) {
		data->stchinfo.x = OSD_SR_OSD_SR_1_H_Location_Ctrl_get_osd_h_sta(rtd_inl(OSD_SR_OSD_SR_1_H_Location_Ctrl_reg));
		data->stchinfo.w = OSD_SR_OSD_SR_1_H_Location_Ctrl_get_osd_width(rtd_inl(OSD_SR_OSD_SR_1_H_Location_Ctrl_reg));
	}

	if(OSD_SR_OSD_SR_1_Scaleup_Ctrl0_get_v_zoom_en(rtd_inl(OSD_SR_OSD_SR_1_Scaleup_Ctrl0_reg))) {
		data->stchinfo.y = OSD_SR_OSD_SR_1_V_Location_Ctrl_get_osd_v_sta(rtd_inl(OSD_SR_OSD_SR_1_V_Location_Ctrl_reg));
		data->stchinfo.h = OSD_SR_OSD_SR_1_V_Location_Ctrl_get_osd_height(rtd_inl(OSD_SR_OSD_SR_1_V_Location_Ctrl_reg));
	}

	return 0;
}

static int gdma_osd_int_status_dump(void)
{
	int i;
	
	printk(KERN_ERR"==== OSD INT status ===\n");
	for (i = 1; i < GDMA_PLANE_MAXNUM; i++)
		printk(KERN_ERR"OSD%d accident sync num = %d\n", i, g_osd_acc_sync_num[i]);
	for (i = 1; i < GDMA_PLANE_MAXNUM; i++)
		printk(KERN_ERR"OSD%d under flow num = %d\n", i, g_osd_udfl_num[i]);

	printk(KERN_ERR"\n==== OSD-SR INT status ===\n");
	
	for (i = 1; i < OSD_SR_MAXNUM; i++)
		printk(KERN_ERR"OSD-SR%d under flow num = %d\n", i, g_sr_udfl_num[i]);

	return 0;
}

#ifdef CONFIG_COMPAT
#if defined(CONFIG_ARM64)
long compat_GDMA_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);
#else
int compat_GDMA_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);
#endif
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0))
struct proc_ops gdma_proc_fops
    = {
    .proc_lseek   =    GDMA_llseek,
    .proc_read     =    GDMA_read,
    .proc_write    =    GDMA_write,
    .proc_ioctl    =    GDMA_ioctl,
    .proc_open     =    GDMA_open,
    .proc_release  =    GDMA_release,
#ifdef CONFIG_COMPAT
    .proc_compat_ioctl   = compat_GDMA_ioctl,
#endif
};

#endif

struct file_operations gdma_fops
	= {
	.owner    =    THIS_MODULE,
	.llseek   =    GDMA_llseek,
	.read     =    GDMA_read,
	.write    =    GDMA_write,
	.unlocked_ioctl    =    GDMA_ioctl,
	.open     =    GDMA_open,
	.release  =    GDMA_release,
#ifdef CONFIG_COMPAT
	.compat_ioctl   = compat_GDMA_ioctl,
#endif
};

/* Osd sync work queue for hwc sync */
#define SYN_TRY_MAX 2
struct workqueue_struct *psOsdSyncWorkQueue;
struct work_struct sOsdSyncWork;

//#define USR_WORKQUEUE_UPDATE_BUFFER

#ifdef USR_WORKQUEUE_UPDATE_BUFFER
#define USE_TRIPLE_BUFFER_NUM   1
#define TRIPLE_BUFFER_TRY_NUM   20
#define TRIPLE_BUFFER_TRY_SLEEP 2

struct workqueue_struct *psOsd1RecWorkQueue;
struct gdma_receive_work sOSD1ReceiveWork[USE_TRIPLE_BUFFER_NUM];

#endif

/* gdma workqueue  */
struct gdma_receive_work sGdmaReceiveWork[GDMA_MAX_PIC_Q_SIZE];

/* use in GDMA_Update */
volatile int osdSyncStamp[GDMA_PLANE_MAXNUM] = {0};	/*  keep the setting which picture has syncstamp */
volatile int osdSyncStampTimeout[GDMA_PLANE_MAXNUM] = {0};


/* GDMA_CTRL_reg   <-- OSD apply */
/* GDMA_OSD1_CTRL_reg <-- OSD1 enabled */
unsigned int osd_ctrl_data[GDMA_PLANE_MAXNUM];

/* buffer address for online write-back */
unsigned int gOnlineWbaddr = 0;

/* start check for interrupt routing to SCPU */
unsigned int g_StartCheckFrameCnt = 100;

#ifdef CONFIG_PM
unsigned int GDMA_PM_REGISTER_DATA[][2]
= {
	{ OSDOVL_Mixer_CTRL2_reg, 0},
	{ OSDOVL_Mixer_layer_sel_reg, 0},
	{ GDMA_line_buffer_sta_reg, 0 },
	{ GDMA_line_buffer_size_reg, 0 },
	{ GDMA_line_buffer_end_reg, 0 },
	{ GDMA_OSD1_reg, 0 },
	{ GDMA_OSD1_START_reg, 0 },
	{ GDMA_OSD1_WI_reg, 0 },
	{ GDMA_OSD1_WI_3D_reg, 0 },
	{ GDMA_OSD1_SIZE_reg, 0 },
#if 0
	{ GDMA_OSD2_reg, 0 },
	{ GDMA_OSD2_START_reg, 0 },
	{ GDMA_OSD2_WI_reg, 0 },
	{ GDMA_OSD2_WI_3D_reg, 0 },
	{ GDMA_OSD2_SIZE_reg, 0 },
#endif
	{ GDMA_OSD3_reg, 0 },
	{ GDMA_OSD3_START_reg, 0 },
	{ GDMA_OSD3_WI_reg, 0 },
	{ GDMA_OSD3_WI_3D_reg, 0 },
	{ GDMA_OSD3_SIZE_reg, 0 },
	{ GDMA_TFBC1_MAX_reg, 0 },
	{ GDMA_TFBC1_MIN_reg, 0 },
	{ OSDOVL_alpha_osd_detect_0_reg, 0 },
	{ OSDOVL_alpha_osd_detect_1_reg, 0 },
	{ OSD_SR_OSD_SR_1_Scaleup_Ctrl0_reg, 0 },
	{ OSD_SR_OSD_SR_1_Scaleup_Ctrl1_reg, 0 },
	{ OSD_SR_OSD_SR_1_H_Location_Ctrl_reg, 0 },
	{ OSD_SR_OSD_SR_1_V_Location_Ctrl_reg, 0 },
	{ OSD_SR_OSD_SR_1_Ctrl_reg, 0},
	{ OSD_SR_OSD_SR_2_Scaleup_Ctrl0_reg, 0 },
	{ OSD_SR_OSD_SR_2_Scaleup_Ctrl1_reg, 0 },
	{ OSD_SR_OSD_SR_2_H_Location_Ctrl_reg, 0 },
	{ OSD_SR_OSD_SR_2_V_Location_Ctrl_reg, 0 },
};
#endif


void GDMA_CopyLayerInfo(GDMA_PICTURE_OBJECT *dest, GDMA_LAYER_OBJECT *src);

void getDispSize(VO_RECTANGLE *disp_rec)
{
	ppoverlay_memcdtg_dh_den_start_end_RBUS dtg_dh;
	ppoverlay_memcdtg_dv_den_start_end_RBUS dtg_dv;
	unsigned int actvie_h = 0;
	unsigned int actvie_v = 0;

	dtg_dh.regValue = rtd_inl(PPOVERLAY_memcdtg_DH_DEN_Start_End_reg);
	dtg_dv.regValue = rtd_inl(PPOVERLAY_memcdtg_DV_DEN_Start_End_reg);
	memset(disp_rec, 0, sizeof(VO_RECTANGLE));

	actvie_h = dtg_dh.memcdtg_dh_den_end-dtg_dh.memcdtg_dh_den_sta;
	actvie_v = dtg_dv.memcdtg_dv_den_end-dtg_dv.memcdtg_dv_den_sta;

	if ((actvie_h > 3800) && (actvie_h <= 3850))
		actvie_h = 3840;
	else if ((actvie_h > 1900) && (actvie_h <= 1930))
		actvie_h = 1920;

	if ((actvie_v > 2100) && (actvie_v <= 2170))
		actvie_v = 2160;
	else if ((actvie_v > 1000) && (actvie_v <= 1090))
		actvie_v = 1080;

	if(actvie_h == 0 || actvie_v ==0) {
		actvie_h = 1920;
		actvie_v = 1080;
		//printk(KERN_INFO"[GDMA] forced to set Disp 2k1k.\n");
	}

	disp_rec->width = actvie_h;
	disp_rec->height = actvie_v;

	return;

}

#ifdef USR_WORKQUEUE_UPDATE_BUFFER
void osd1_send_hw(struct work_struct *psWork)
{
    struct gdma_receive_work *d_work;
    gdma_dev *gdma = &gdma_devices[0];

    d_work = (struct gdma_receive_work *)psWork;
    GDMA_ReceivePicture(d_work->data);

	down(&gdma->sem_work);
	d_work->used = 0;
    up(&gdma->sem_work);
}
#endif

void osd_sync_hwc(struct work_struct *psWork)
{
	gdma_dev *gdma = &gdma_devices[0];
	int ret;

	/* pr_debug(KERN_EMERG"func=%s line=%d sem->count=%d\n",__FUNCTION__,__LINE__,gdma->vsync_sem.count); */
	/* if (gdma->vsync_sem.count < 1) */
	up(&gdma->vsync_sem);
	/* msleep(1); */
	ret = down_trylock(&gdma->vsync_sem);
	/* down_interruptible(&gdma->vsync_sem); */
	/* mutex_unlock(&gdma_vsync_lock); */
	/* pr_debug(KERN_EMERG"func=%s line=%d sem->count=%d\n",__FUNCTION__,__LINE__,gdma->vsync_sem.count); */

}

void GDMA_ENTER_CRITICAL (void)
{
	gdma_dev *gdma = &gdma_devices[0];
	/* disable GDMA interrupt */
	if (gdma->ctrl.enable3D)
		rtd_outl(GDMA_OSD_INTEN_3D_reg, GDMA_OSD_INTEN_3D_write_data(0) | GDMA_OSD_INTEN_3D_osd1_vact_end(1));
	else
#ifndef USING_GDMA_VSYNC
#ifdef TRIPLE_BUFFER_SEMAPHORE
		rtd_outl(GDMA_OSD_INTEN_reg, GDMA_OSD_INTEN_write_data(0) | GDMA_OSD_INTEN_osd1_vact_end(1) | GDMA_OSD_INTEN_osd1_vsync(1));
#else
		rtd_outl(GDMA_OSD_INTEN_reg, GDMA_OSD_INTEN_write_data(0) | GDMA_OSD_INTEN_osd1_vact_end(1));
#endif
#else
		rtd_outl(GDMA_OSD_INTEN_reg, GDMA_OSD_INTEN_write_data(0) | GDMA_OSD_INTEN_osd1_vsync(1));
#endif
	return;
}

void GDMA_EXIT_CRITICAL (void)
{
	gdma_dev *gdma = &gdma_devices[0];
	if(GAL_Runtime_Suspend_status)
	    return;

	/* enable GDMA interrupt */
	if (gdma->ctrl.enable3D)
		rtd_outl(GDMA_OSD_INTEN_3D_reg, GDMA_OSD_INTEN_3D_write_data(1) | GDMA_OSD_INTEN_3D_osd1_vact_end(1));
	else
#ifndef USING_GDMA_VSYNC
#ifdef TRIPLE_BUFFER_SEMAPHORE
		rtd_outl(GDMA_OSD_INTEN_reg, GDMA_OSD_INTEN_write_data(1) | GDMA_OSD_INTEN_osd1_vact_end(1) | GDMA_OSD_INTEN_osd1_vsync(1));
#else
		rtd_outl(GDMA_OSD_INTEN_reg, GDMA_OSD_INTEN_write_data(1) | GDMA_OSD_INTEN_osd1_vact_end(1));
#endif
#else
		rtd_outl(GDMA_OSD_INTEN_reg, GDMA_OSD_INTEN_write_data(1) | GDMA_OSD_INTEN_osd1_vsync(1));
#endif

	return;
}



void GDMA_UpdateGraphicwindow (int bBlocking, VO_GRAPHIC_PLANE plane)
{
	gdma_dev   *gdma     = &gdma_devices[0];
	GDMA_PLANE *osdPlane = &gdma->osdPlane[plane];
	GDMA_WIN   *win = 0;
	unsigned int i, osdEn;
	GDMA_PICTURE_OBJECT *picObj;
	static PICTURE_LAYERS_OBJECT *picLayerObj;
	int onlinePlane[GDMA_PLANE_MAXNUM], onlineMaxNum = 0, onlineIdx = 0;
	VO_GRAPHIC_PLANE disPlane;
	onlinePlane[onlineMaxNum++] = VO_GRAPHIC_OSD1;
	onlinePlane[onlineMaxNum++] = VO_GRAPHIC_OSD2;
	onlinePlane[onlineMaxNum++] = VO_GRAPHIC_OSD3;

	if (picLayerObj == NULL) {
		picLayerObj = (PICTURE_LAYERS_OBJECT *)kmalloc(sizeof(PICTURE_LAYERS_OBJECT), GFP_KERNEL);
		if (picLayerObj == NULL) {
			pr_err(KERN_EMERG"%s, kmalloc failed \n", __FUNCTION__);
			return;
		}
	}

	memset(picLayerObj, 0, sizeof(PICTURE_LAYERS_OBJECT));

	if (osdPlane->OSDwin[osdPlane->OSD_topid].attr.compress)	{
		picObj = &picLayerObj->layer[0].fbdc[OSD1_1];	
	} else {
		picObj = &picLayerObj->layer[0].normal[OSD1_1];
	}
	
	down(&gdma->sem);

	GDMA_ENTER_CRITICAL ();

	osdEn = osdPlane->OSD_ctrl.en;

	GDMA_PRINT(7, "osdEn:%d turnOff:%d\n", osdEn, osdPlane->OSD_ctrl.hwTurnOff);
	
	if ((osdEn && !osdPlane->OSD_ctrl.hwTurnOff)) {
		if (osdPlane->OSDwin[osdPlane->OSD_topid].attr.compress) {
			picLayerObj->layer_num = 1;
			picLayerObj->layer[0].fbdc_num = 1;
		} else {
			picLayerObj->layer_num = 1;
			picLayerObj->layer[0].normal_num = 1;
		}
		
		memcpy (osdPlane->OSDwinBuf[osdPlane->OSD_ctrl.index], osdPlane->OSDwin, sizeof(osdPlane->OSDwin));

		/* for (i=0, win = &osdPlane->OSDwinBuf[osdPlane->OSD_ctrl.index][osdPlane->OSD_topid]; i < osdPlane->numOSDwins; i++) */
		for (i = 0, win = &osdPlane->OSDwinBuf[osdPlane->OSD_ctrl.index][osdPlane->OSD_topid]; i < 1; i++) {
			if (win->ctrl.drawn) {
				picObj->alpha = (win->attr.alphaEn == 0) ? 0 : win->attr.alpha;
				picObj->rgb_order = win->attr.rgbOrder;
				picObj->clear_x.value = 0;
				picObj->clear_y.value = 0;
				picObj->colorkey = (win->colorKey.keyEn == 0) ? -1 : win->colorKey.key;

				picObj->compressed = win->attr.compress;

				/*picObj->context = src->context; */
				/*picObj->decompress = src->decompress; */
				picObj->dst_height = osdPlane->OSD_canvas.dispWin.height;
				picObj->dst_width = osdPlane->OSD_canvas.dispWin.width;
				picObj->dst_x = osdPlane->OSD_canvas.dispWin.x;
				picObj->dst_y = osdPlane->OSD_canvas.dispWin.y;
				/*picObj->fbdc_format = src->format; */
				if (win->attr.compress){
					picObj->fbdc_format = FBDC_COLOR_FORMAT_U8U8U8U8;
					//picObj->compressed = 0;
				} else {
					picObj->format = (win->attr.type) | (win->attr.littleEndian<<5);
				}
				picObj->height = win->winWH.height;
				picObj->key_en = win->colorKey.keyEn;
				picObj->layer_used = 1;
				/*picObj->paletteIndex = src->paletteIndex; */
				/*picObj->picLayout = src->picLayout; */
				picObj->pitch = win->pitch;
				/*
				* workaround for system AP call that uses the old definition VO_GRAPHIC_OSD1 or VO_GRAPHIC_OSD is zero
				* force the zero of plane value to one of VO_GRAPHIC_OSD1
				*/
#if defined(CONFIG_ARCH_RTK287X) || defined(CONFIG_ARCH_RTK287O)
			#if defined(CONFIG_ARCH_RTK6748)
			    picObj->plane = (plane == 0) ? VO_GRAPHIC_OSD1 : VO_GRAPHIC_OSD2;//plane;
			#else
				picObj->plane = (plane == 0) ? VO_GRAPHIC_OSD1 : VO_GRAPHIC_OSD3;//plane;
			#endif
#else
				picObj->plane = (plane == 0) ? VO_GRAPHIC_OSD1 : plane;
#endif
				picObj->plane_ar.plane_alpha_r = 0xff;
				picObj->plane_ar.plane_alpha_a = 0xff;
				picObj->plane_gb.plane_alpha_g = 0xff;
				picObj->plane_gb.plane_alpha_b = 0xff;
				/*picObj->seg_addr */
				/*picObj->seg_num */
				picObj->show = 1;       /*  always show the picture by normal update pic flow */
				if (win->attr.compress) {
					picObj->src_type = SRC_FBDC;
				} else {
					picObj->src_type = SRC_NORMAL;
				}
				picObj->syncInfo.syncstamp = 0;
				picObj->scale_factor = 1;
				if (picObj->compressed) {
					picObj->wbType = RTKCOMPRESS;
					/*
					picObj->wbType = ARGB8888;
					picObj->wbType = RBG888;
					*/
				}
				picObj->width = win->winWH.width;
				picObj->x = win->winXY.x;
				picObj->y = win->winXY.y;
				picObj->address = win->top_addr+(win->objOffset.objYoffset * win->pitch);
				picObj->offlineOrder = C0;

				for (onlineIdx = 0; onlineIdx < onlineMaxNum; onlineIdx++) { /* online dispaly check & set blend order */
					disPlane = onlinePlane[onlineIdx];

					if (picObj->plane == disPlane) {
						picLayerObj->layer[0].onlineOrder = (E_BLEND_ORDER)onlineIdx;
						break;
					}
				}

				if (onlineIdx == onlineMaxNum) {	/* plane does NOT indicate the online path of OSD */
					if (picObj->compressed == 0) {
						GDMA_PRINT(7, "%s OSD%d are offline, Does NOT display with this plane if plane doesn't compress \n", __FUNCTION__, picObj->plane);
						continue;
					}
				}
				
				GDMA_ReceivePicture(picLayerObj);
			}
		}
	}
	GDMA_EXIT_CRITICAL();
	up(&gdma->sem);
}

int GDMA_ConfigureGraphicCanvas (VIDEO_RPC_VOUT_CONFIG_GRAPHIC_CANVAS *cmd)
{

	DBG_PRINT(KERN_EMERG"GDMA: GDMA_CONFIGURE_GRPAHIC_CANVAS,plane %d\n", cmd->plane);
	DBG_PRINT(KERN_EMERG"GDMA: src x %d y %d w %d h %d, disp x %d y %d w %d h %d\n", cmd->srcWin.x, cmd->srcWin.y, cmd->srcWin.width, cmd->srcWin.height, cmd->dispWin.x, cmd->dispWin.y, cmd->dispWin.width, cmd->dispWin.height);
#if 0
	gdma_dev   *gdma     = &gdma_devices[0];
	GDMA_PLANE *osdPlane = &gdma->osdPlane[cmd->plane];
	osdPlane->OSD_canvas.en = 1;
	osdPlane->OSD_canvas.srcWin  = *(VO_RECTANGLE *)&cmd->srcWin;
	osdPlane->OSD_canvas.dispWin = *(VO_RECTANGLE *)&cmd->dispWin;

	if (cmd->go)
		GDMA_UpdateGraphicwindow (1, cmd->plane);
#endif
	return 0;
}
int GDMA_CreateGraphicWindow (VIDEO_RPC_VOUT_CREATE_GRAPHIC_WIN *cmd)
{
	gdma_dev   *gdma     = &gdma_devices[0];
	GDMA_PLANE *osdPlane = &gdma->osdPlane[cmd->plane];
	GDMA_WIN *win , *curWin, *preWin = 0;
	int i;

	if (osdPlane->OSD_ctrl.dispOn)
		return -EFAULT;

	if (osdPlane->numOSDwins >= GDMA_MAX_NUM_OSD_WIN)
		return -EFAULT;

	for (win = osdPlane->OSDwin;; win++)
		if (!win->ctrl.occupied)
			break;

	win->ctrl.occupied  = 1;
	win->ctrl.previd    =
		win->ctrl.nextid    = 0xFF;
	win->attr.alphaEn   = (win->attr.alpha = cmd->alpha) >  0 ? 1 : 0;
	win->colorKey.keyEn = (cmd->colorKey != -1);
	win->colorKey.key   = cmd->colorKey;
	win->attr.type      = cmd->colorFmt & 0x1f;
	win->attr.littleEndian = cmd->colorFmt >> 5;
	win->attr.compress = (cmd->compressed > 0 ? 1 : 0);
	win->attr.rgbOrder  = cmd->rgbOrder;
	win->winXY.x        = cmd->winPos.x;
	win->winXY.y        = cmd->winPos.y;
	win->winWH.width    = cmd->winPos.width;
	win->winWH.height   = cmd->winPos.height;

	for (i = 0, curWin = &osdPlane->OSDwin[osdPlane->OSD_topid]; i < osdPlane->numOSDwins; i++) {
		if (gdma->ctrl.enableVFlip ? win->winXY.y >= curWin->winXY.y : win->winXY.y < curWin->winXY.y)
			break;
		else
			preWin = curWin;
		curWin = &osdPlane->OSDwin[preWin->ctrl.nextid];
	}
	if (preWin) {
		win->ctrl.nextid = preWin->ctrl.nextid;
		if ((win->ctrl.nextid) != 0xFF)
			osdPlane->OSDwin[win->ctrl.nextid].ctrl.previd = win->ctrl.id;
		win->ctrl.previd    = preWin->ctrl.id;
		preWin->ctrl.nextid = win->ctrl.id;
	} else {
		if (osdPlane->numOSDwins) {
			win->ctrl.nextid     = osdPlane->OSD_topid;
			osdPlane->OSDwin[osdPlane->OSD_topid].ctrl.previd = win->ctrl.id;
		}
		osdPlane->OSD_topid = win->ctrl.id;
	}

	osdPlane->numOSDwins++;

	return win->ctrl.id;
}

int GDMA_DrawGraphicWindow (VIDEO_RPC_VOUT_DRAW_GRAPHIC_WIN *cmd)
{
	gdma_dev   *gdma     = &gdma_devices[0];
	GDMA_PLANE *osdPlane = &gdma->osdPlane[cmd->plane];
	GDMA_WIN *win, *thisWin;
	int i;
	
	DBG_PRINT(KERN_EMERG"GDMA: GDMA_DRAW_GRPAHIC_WINDOW, plane %d, winID %d\n", cmd->plane, cmd->winID);
	DBG_PRINT(KERN_EMERG"GDMA: addr 0x%x, p %d, x %d, y %d\n", (unsigned int)cmd->pImage[0], cmd->imgPitch[0], cmd->startX[0], cmd->startY[0]);
	if ( osdPlane->OSD_ctrl.wn >= GDMA_MAX_NUM_OSD_WIN) {
		GDMA_PRINT(8 , "%s crtlWin  fail:%d\n", __FUNCTION__, osdPlane->OSD_ctrl.wn);
		return -EFAULT;
	}

	thisWin = &osdPlane->OSDwin[cmd->winID];
	if (cmd->winID >= GDMA_MAX_NUM_OSD_WIN || !(thisWin->ctrl.occupied))  {
		GDMA_PRINT(8 , "%s win fail :%d %d\n", __FUNCTION__, cmd->winID, thisWin->ctrl.occupied );		
		return -EFAULT;
	}


	/* check if the OSD window to draw horizontally overlaps any
	existing OSD window on screen */

	for (i = 0, win = &osdPlane->OSDwin[osdPlane->OSD_topid]; i < osdPlane->numOSDwins; i++) {
		if ( win->ctrl.drawn
			&& thisWin->winXY.y + thisWin->winWH.height > win->winXY.y
			&& thisWin->winXY.y < win->winXY.y + win->winWH.height) {

				GDMA_PRINT(8 , "%s winSize fail idx:%d num:%d %d [%d %d %d %d]\n", __FUNCTION__, i, osdPlane->numOSDwins, 
					win->ctrl.drawn, thisWin->winXY.x, thisWin->winXY.y, thisWin->winWH.width, thisWin->winWH.height );

				return -EFAULT;
			}
	}

	thisWin->pitch = cmd->imgPitch[0];

	if (thisWin->attr.type < 3)
		thisWin->CLUT_addr = virt_to_phys(gdma->OSD_CLUT[cmd->paletteIndex]);

	thisWin->attr.compress = cmd->compressed;
	thisWin->objOffset.objXoffset = cmd->startX[0];
	/*thisWin->objOffset.objYoffset = gdma->ctrl.enableVFlip ? 0 : cmd->startY[0];*/
	thisWin->objOffset.objYoffset = cmd->startY[0];

	for (i = 0; i < 2; i++) {
		thisWin->srcPitch[i] = cmd->imgPitch[i];
		thisWin->srcImg[i] = cmd->pImage[i];
	}

	/* mapping to hardware's location */
	{
		thisWin->top_addr     = cmd->pImage[0];
		thisWin->bot_addr     = cmd->pImage[1];
	}

	thisWin->ctrl.drawn = 1;

	osdPlane->OSD_ctrl.en = 1;
	osdPlane->OSD_ctrl.wn++;


#if defined(CONFIG_ARCH_RTK6748)
	GDMA_UpdateGraphicwindow (1, cmd->plane);
#else
	if (cmd->go)
		GDMA_UpdateGraphicwindow (1, cmd->plane);
#endif

	

	return 0;
}

int GDMA_ModifyGraphicWindow (VIDEO_RPC_VOUT_MODIFY_GRAPHIC_WIN *cmd)
{
	gdma_dev   *gdma     = &gdma_devices[0];
	GDMA_PLANE *osdPlane = &gdma->osdPlane[cmd->plane];
	GDMA_WIN   *win , *curWin, *preWin = 0;
	int i;
	DBG_PRINT(KERN_EMERG"GDMA: VIDEO_RPC_VOUT_MODIFY_GRAPHIC_WIN, plane %d, winID %d, mask 0x%x\n", cmd->plane, cmd->winID, cmd->reqMask);
	DBG_PRINT(KERN_EMERG"GDMA: fmt %d, rgbOrder %d, key %d, alpha %d\n", cmd->colorFmt, cmd->rgbOrder, cmd->colorKey, cmd->alpha);
	DBG_PRINT(KERN_EMERG"GDMA: pos x %d y %d w %d h %d\n", cmd->winPos.x, cmd->winPos.y, cmd->winPos.width, cmd->winPos.height);
	DBG_PRINT(KERN_EMERG"GDMA: addr 0x%x, p %d, x %d, y %d\n", (unsigned int)cmd->pImage[0], cmd->imgPitch[0], cmd->startX[0], cmd->startY[0]);

	win = &osdPlane->OSDwin[cmd->winID];
	if (cmd->winID >= GDMA_MAX_NUM_OSD_WIN || !(win->ctrl.occupied))
		return -EFAULT;

	if (cmd->reqMask & GDMA_MODIFY_POSITION) {
		if (win->ctrl.drawn) {
			for (i = 0, curWin = &osdPlane->OSDwin[osdPlane->OSD_topid]; i < osdPlane->numOSDwins; i++) {
				if (curWin->ctrl.drawn && curWin->ctrl.id != win->ctrl.id
					&& cmd->winPos.y + cmd->winPos.height > curWin->winXY.y
					&& cmd->winPos.y < curWin->winXY.y + curWin->winWH.height)
					return -EFAULT;

				if (curWin->ctrl.nextid != 0xFF)
					curWin = &osdPlane->OSDwin[curWin->ctrl.nextid];
				else
					break;

			}
		}

		/* Delete */
		if (win->ctrl.previd != 0xFF)
			osdPlane->OSDwin[win->ctrl.previd].ctrl.nextid = win->ctrl.nextid;
		else
			osdPlane->OSD_topid = (win->ctrl.nextid != 0xFF) ? win->ctrl.nextid : win->ctrl.id;

		if (win->ctrl.nextid != 0xFF)
			osdPlane->OSDwin[win->ctrl.nextid].ctrl.previd = win->ctrl.previd;
		win->ctrl.previd =
			win->ctrl.nextid = 0xFF;
		osdPlane->numOSDwins--;

		/* Insert */
		for (i = 0, curWin = &osdPlane->OSDwin[osdPlane->OSD_topid]; i < osdPlane->numOSDwins; i++) {
			if (gdma->ctrl.enableVFlip ? win->winXY.y >= curWin->winXY.y : win->winXY.y < curWin->winXY.y)
				break;
			else
				preWin = curWin;
			curWin = &osdPlane->OSDwin[preWin->ctrl.nextid];
		}
		if (preWin) {
			win->ctrl.nextid = preWin->ctrl.nextid;
			if ((win->ctrl.nextid) != 0xFF)
				osdPlane->OSDwin[win->ctrl.nextid].ctrl.previd = win->ctrl.id;
			win->ctrl.previd    = preWin->ctrl.id;
			preWin->ctrl.nextid = win->ctrl.id;
		} else {
			if (osdPlane->numOSDwins) {
				win->ctrl.nextid = osdPlane->OSD_topid;
				osdPlane->OSDwin[osdPlane->OSD_topid].ctrl.previd = win->ctrl.id;
			}
			osdPlane->OSD_topid = win->ctrl.id;
		}

		osdPlane->numOSDwins++;

		win->winXY.x            = cmd->winPos.x;
		win->winXY.y            = cmd->winPos.y;
		win->winWH.width        = cmd->winPos.width;
		win->winWH.height       = cmd->winPos.height;
	}

	if (cmd->reqMask & GDMA_MODIFY_COLORFMT) {
		win->attr.type      = cmd->colorFmt & 0x1f;
		win->attr.littleEndian = cmd->colorFmt >> 5;
		win->attr.rgbOrder  = cmd->rgbOrder;
	}

	if (cmd->reqMask & GDMA_MODIFY_COLORKEY) {
		win->colorKey.keyEn = (cmd->colorKey != -1);
		win->colorKey.key   = cmd->colorKey;
	}

	if (cmd->reqMask & GDMA_MODIFY_ALPHA)
		win->attr.alphaEn   = (win->attr.alpha = cmd->alpha) >  0 ? 1 : 0;

	if (cmd->reqMask & GDMA_MODIFY_STORAGE_TYPE) {
		win->attr.compress = cmd->compressed;
		if (win->attr.type < 3)
			win->CLUT_addr       = virt_to_phys(gdma->OSD_CLUT[cmd->paletteIndex]);
	}

	if (cmd->reqMask & GDMA_MODIFY_SRC_LOCATION) {
		win->objOffset.objXoffset = cmd->startX[0];
		/*win->objOffset.objYoffset = gdma->ctrl.enableVFlip ? 0 : cmd->startY[0];*/
		win->objOffset.objYoffset = cmd->startY[0];
		win->pitch = cmd->imgPitch[0];
		for (i = 0; i < 2; i++) {
			win->srcPitch[i] = cmd->imgPitch[i];
			win->srcImg[i] = cmd->pImage[i];
		}

		/* mapping to hardware's location */
		{
			win->top_addr     = cmd->pImage[0];
			win->bot_addr     = cmd->pImage[1];
		}
	}

	if (cmd->go)
		GDMA_UpdateGraphicwindow (1, cmd->plane);

	return 0;
}

int GDMA_HideGraphicWindow (unsigned char winID, VO_GRAPHIC_PLANE plane, unsigned char go)
{

	gdma_dev   *gdma     = &gdma_devices[0];
	GDMA_PLANE *osdPlane;
	GDMA_WIN *win;

	if((GDMA_DISPLAY_PLANE)plane >= GDMA_PLANE_MAXNUM)
		return -EFAULT;
	osdPlane = &gdma->osdPlane[plane];
	win = &osdPlane->OSDwin[winID];
	if (winID >= GDMA_MAX_NUM_OSD_WIN || !(win->ctrl.occupied))
		return -EFAULT;
	if (win->ctrl.drawn) {
		win->ctrl.drawn = 0;
		osdPlane->OSD_ctrl.wn--;

		if (!osdPlane->OSD_ctrl.wn)
			osdPlane->OSD_ctrl.en = 0;

		if (go)
			GDMA_UpdateGraphicwindow (1, plane);
	}

	return 0;
}

int GDMA_DeleteGraphicWindow (unsigned char winID, VO_GRAPHIC_PLANE plane, unsigned char go)
{
	gdma_dev   *gdma     = &gdma_devices[0];
	GDMA_PLANE *osdPlane = &gdma->osdPlane[plane];
	GDMA_WIN *win;

	win = &osdPlane->OSDwin[winID];
	if (winID >= GDMA_MAX_NUM_OSD_WIN || !(win->ctrl.occupied))
		return -EFAULT;

	if (win->ctrl.drawn)
		GDMA_HideGraphicWindow (winID, plane, 0);
	if (win->ctrl.previd != 0xFF)
		osdPlane->OSDwin[win->ctrl.previd].ctrl.nextid = win->ctrl.nextid;
	else
		osdPlane->OSD_topid = (win->ctrl.nextid != 0xFF) ? win->ctrl.nextid : 0;

	if (win->ctrl.nextid != 0xFF)
		osdPlane->OSDwin[win->ctrl.nextid].ctrl.previd = win->ctrl.previd;

	win->ctrl.occupied = 0;
	osdPlane->numOSDwins--;

	if (go)
		GDMA_UpdateGraphicwindow (1, plane);

	return 0;
}

int GDMA_GetOSDPalette (unsigned int index, unsigned int *palette, unsigned int *len)
{
	gdma_dev   *gdma = &gdma_devices[0];
	if (index < GDMA_MAX_NUM_CLUT && len != NULL) {
		memcpy (palette, gdma->OSD_CLUT[index], sizeof(unsigned int) * gdma->OSD_CLUT_LEN[index]);
		*len = gdma->OSD_CLUT_LEN[index];
		return GDMA_SUCCESS;
	}

	return GDMA_FAIL;
}
int GDMA_SetOSDPalette (unsigned int index, unsigned int *palette, unsigned int len)
{
	dma_addr_t addr;
	gdma_dev   *gdma = &gdma_devices[0];
	if (index < GDMA_MAX_NUM_CLUT && len <= 256) {
		memcpy (gdma->OSD_CLUT[index], (void *)(palette), sizeof(unsigned int) * len);
		gdma->OSD_CLUT_LEN[index] = len;
		addr = dma_map_single(gdma->dev, (void *)gdma->OSD_CLUT[index], 256*sizeof(unsigned int), DMA_TO_DEVICE);
		dma_unmap_single(gdma->dev, addr, 256*sizeof(unsigned int), DMA_TO_DEVICE);
		gdma->f_palette_update = TRUE;
		return GDMA_SUCCESS;
	}
	return GDMA_FAIL;
}



int GDMA_ConfigureOSDPalette (VIDEO_RPC_VOUT_CONFIG_OSD_PALETTE *cmd)
{
	gdma_dev   *gdma = &gdma_devices[0];
	if (cmd->paletteIndex < GDMA_MAX_NUM_CLUT)
		memcpy (gdma->OSD_CLUT[cmd->paletteIndex], (void *)(cmd->pPalette | 0xa0000000), sizeof(unsigned int) * 256);
	return 0;
}

int GDMA_DisplayGraphic (VIDEO_RPC_VOUT_DISPLAY_GRAPHIC *cmd)
{
	gdma_dev   *gdma     = &gdma_devices[0];
	GDMA_PLANE *osdPlane = &gdma->osdPlane[cmd->plane];
	int i , id;

	/* Reset */
	osdPlane->OSD_topid   = 0;
	osdPlane->numOSDwins  = 0;
	osdPlane->OSD_ctrl.en = 0;
	osdPlane->OSD_ctrl.wn = 0;
	for (i = 0; i < GDMA_MAX_NUM_OSD_WIN; i++) {
		osdPlane->OSDwin[i].ctrl.id     = i;
		osdPlane->OSDwin[i].ctrl.previd =
			osdPlane->OSDwin[i].ctrl.nextid = 0xff;
	}
	osdPlane->OSD_ctrl.dispOn = 0;


	if (cmd->osdNum) {
		VIDEO_RPC_VOUT_CREATE_GRAPHIC_WIN   create_osd;
		VIDEO_RPC_VOUT_DRAW_GRAPHIC_WIN     draw_osd;
		VIDEO_RPC_VOUT_DISPLAY_GRAPHIC_WIN *winInfo  = (VIDEO_RPC_VOUT_DISPLAY_GRAPHIC_WIN *)(cmd->pGraphic | 0xa0000000);

		for (i = 0; i < cmd->osdNum; i++, winInfo++) {
			create_osd.plane         = cmd->plane;
			create_osd.winPos.x      = winInfo->x;
			create_osd.winPos.y      = winInfo->y;
			create_osd.winPos.width  = winInfo->width;
			create_osd.winPos.height = winInfo->height;
			create_osd.colorFmt      = winInfo->colorFmt;
			create_osd.rgbOrder      = winInfo->rgbOrder;
			create_osd.colorKey      = winInfo->colorKey;
			create_osd.alpha         = winInfo->alpha;
			create_osd.compressed = winInfo->compressed;
			id = GDMA_CreateGraphicWindow (&create_osd);
			if (id >= 0) {
				draw_osd.plane           = cmd->plane;
				draw_osd.winID           = id;
				draw_osd.paletteIndex    = winInfo->paletteIndex;
				draw_osd.go              = 0;
				draw_osd.compressed      = winInfo->compressed;
				for (i = 0; i < 4; i++) {
					draw_osd.startX[i]     = winInfo->startX[i];
					draw_osd.startY[i]     = winInfo->startY[i];
					draw_osd.imgPitch[i]   = winInfo->imgPitch[i];
					draw_osd.pImage[i]     = winInfo->pImage[i];
				}
				GDMA_DrawGraphicWindow   (&draw_osd);
			}
		}
	}
	osdPlane->OSD_ctrl.dispOn = (cmd->osdNum > 0);

	GDMA_UpdateGraphicwindow (1, cmd->plane);

	return 0;
}
#if OSD_DETECTION_ALPHA_SUPPORT
int GDMA_GetAlphaOsdDetectionParameter(int *data)
{
	unsigned int reg=0;
	reg = OSDOVL_alpha_osd_detect_0_get_thd1(rtd_inl(OSDOVL_alpha_osd_detect_0_reg));
	*data = reg;

     return 1;
}

int GDMA_SetAlphaOsdDetectionParameter(int data)
{
	unsigned int reg=0;
	osdovl_osd_db_ctrl_RBUS osd_db_ctrl_reg;
	
	reg = rtd_inl(OSDOVL_alpha_osd_detect_0_reg) & ~(OSDOVL_alpha_osd_detect_0_thd1_mask|OSDOVL_alpha_osd_detect_0_thd2_mask); 
	reg |= OSDOVL_alpha_osd_detect_0_thd1(data) | OSDOVL_alpha_osd_detect_0_thd2(0xff);
	rtd_outl(OSDOVL_alpha_osd_detect_0_reg,reg);
	
	reg = OSDOVL_Mixer_layer_sel_a_det_osd3_en(1)|OSDOVL_Mixer_layer_sel_a_det_osd2_en(1)|OSDOVL_Mixer_layer_sel_a_det_osd1_en(1);
	rtd_outl(OSDOVL_Mixer_layer_sel_reg,rtd_inl(OSDOVL_Mixer_layer_sel_reg) | reg);

	/* mixer double buffer */
	osd_db_ctrl_reg.regValue = rtd_inl(OSDOVL_OSD_Db_Ctrl_reg);
	osd_db_ctrl_reg.db_read = 1;	/* 1: see rbus setting temporarily, 0: see HW latch now  */
	osd_db_ctrl_reg.db_en = 1;
	osd_db_ctrl_reg.db_load = 1;
	rtd_outl(OSDOVL_OSD_Db_Ctrl_reg, osd_db_ctrl_reg.regValue);
	
	return GDMA_SUCCESS;
}

EXPORT_SYMBOL(GDMA_SetAlphaOsdDetectionParameter);

int GDMA_GetAlphaOsdDetectionParametertable(GDMA_ALPHA_TABLE *rlt_table)
{
	if(rlt_table != NULL){
		rlt_table->alpha_table[0] = rtd_inl(OSDOVL_alpha_osd_detect_0_ro_reg);
		rlt_table->alpha_table[1] = rtd_inl(OSDOVL_alpha_osd_detect_1_ro_reg);
		rlt_table->alpha_table[2] = rtd_inl(OSDOVL_alpha_osd_detect_2_ro_reg);
		rlt_table->alpha_table[3] = rtd_inl(OSDOVL_alpha_osd_detect_3_ro_reg);
		rlt_table->alpha_table[4] = rtd_inl(OSDOVL_alpha_osd_detect_4_ro_reg);
		rlt_table->alpha_table[5] = rtd_inl(OSDOVL_alpha_osd_detect_5_ro_reg);
		rlt_table->alpha_table[6] = rtd_inl(OSDOVL_alpha_osd_detect_6_ro_reg);
		rlt_table->alpha_table[7] = rtd_inl(OSDOVL_alpha_osd_detect_7_ro_reg);
		return GDMA_SUCCESS;
	}
	return GDMA_FAIL;
}

int GDMA_GetAlphaOsdDetectionratio(int *ratio)
{
    GDMA_ALPHA_TABLE alpha_data;
	unsigned int sum=0,rlt=0;
	
	if(ratio == NULL)
		return GDMA_FAIL;
	if(GDMA_GetAlphaOsdDetectionParametertable(&alpha_data) != GDMA_SUCCESS)
		return GDMA_FAIL;

	sum = alpha_data.alpha_table[0] + alpha_data.alpha_table[1] + alpha_data.alpha_table[2];

	rlt = ( alpha_data.alpha_table[2] << 8 ) / sum;

	*ratio = rlt;
	return GDMA_SUCCESS;
}
#endif
#if OSD_DETECTION_SUPPORT
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0)) 
void GDMA_OsdDetection(struct timer_list *t);
#else
void GDMA_OsdDetection(void);
#endif

int GDMA_SetOsdDetectionParameter(GDMA_OSD_DETECTION_PARAMETER_T *pDetectionParameter)
{
	if (pDetectionParameter == NULL) {
		pr_debug("%s, get parameter failed.\n", __FUNCTION__);
		return GDMA_FAIL;
	}
	
	memcpy(&gDetectParameter, pDetectionParameter, sizeof(GDMA_OSD_DETECTION_PARAMETER_T));
	
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0)) 
	GDMA_OsdDetection(&GDMA_timer);
#else
	GDMA_OsdDetection();
#endif

	return GDMA_SUCCESS;
}

int GDMA_GetOsdDetectionStatus(int *pStatus)
{	
	*pStatus = gDetectionStatus;
	
	return GDMA_SUCCESS;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0)) 
void GDMA_OsdDetection(struct timer_list *t)
#else
void GDMA_OsdDetection(void)
#endif
{
	unsigned int sta_x = 0, sta_y = 0, end_x = 0, end_y = 0;

	sta_x = OSDOVL_measure_osd1_sta_get_x(rtd_inl(OSDOVL_measure_osd1_sta_reg));
	sta_y = OSDOVL_measure_osd1_sta_get_y(rtd_inl(OSDOVL_measure_osd1_sta_reg));
	end_x = OSDOVL_measure_osd1_end_get_x(rtd_inl(OSDOVL_measure_osd1_end_reg));
	end_y = OSDOVL_measure_osd1_end_get_y(rtd_inl(OSDOVL_measure_osd1_end_reg));

	if (sta_x == 0x1FFF && sta_y == 0x1FFF)
		sta_x = sta_y = 0x0;
	
	if (sta_x <= gDetectParameter.sta_x_th && sta_y <= gDetectParameter.sta_y_th && 
		gDetectParameter.end_x_th <= end_x && gDetectParameter.end_y_th <= end_y ) {
		gDetectionStatus = 1; // OSD minimal rectangle is more than threshold
	} else {
		gDetectionStatus = 0;
	}
    //printk(KERN_EMERG"[GDMA] %s : status = %d\n", __func__, gDetectionStatus);

	if (gDetectParameter.frame_interval == 0) {
		GDMA_timer.expires = jiffies + msecs_to_jiffies(g_jiffies_counter);
	} else {
		GDMA_timer.expires = jiffies + msecs_to_jiffies(16 * gDetectParameter.frame_interval);
	}

	mod_timer(&GDMA_timer,GDMA_timer.expires);
	//add_timer(&GDMA_timer);
}

void GDMA_timer_init(void)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
    timer_setup(&GDMA_timer, GDMA_OsdDetection, 0);
#else
	init_timer(&GDMA_timer);
	GDMA_timer.function = (void *)GDMA_OsdDetection;
	GDMA_timer.data = ((unsigned long) 0);
#endif
	
	GDMA_timer.expires = jiffies + msecs_to_jiffies(g_jiffies_counter);
	add_timer(&GDMA_timer);
}
#endif

loff_t GDMA_llseek(struct file *filp, loff_t off, int whence)
{
	return -EINVAL;
}
ssize_t GDMA_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
	return -EFAULT;
}
ssize_t GDMA_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
	char str[RTKGDMA_MAX_CMD_LENGTH] = {0};
	int cmd_index = 0, cmd_length = 0;
	char *cmd_pointer = NULL;

	if (buf == NULL) {
		return -EFAULT;
	}
	if (count > RTKGDMA_MAX_CMD_LENGTH - 1)
		count = RTKGDMA_MAX_CMD_LENGTH - 1;
	if (copy_from_user(str, buf, count)) {
		printk(KERN_ERR "rtkgdma_debug:%d copy_from_user failed! (buf=%p, count=%d)\n",
			__LINE__, buf, (unsigned int)count);
		return -EFAULT;
	}

	if (count > 0)
		str[count-1] = '\0';

	for (cmd_index = 0; cmd_index < GDMA_DBG_CMD_NUMBER; cmd_index++) {
		if (strncmp(str, rtkgdma_cmd_str[cmd_index], strlen(rtkgdma_cmd_str[cmd_index])) == 0)
			break;
	}

	if (cmd_index < GDMA_DBG_CMD_NUMBER) {
		//cmd_pointer = &str[strlen(rtkgdma_cmd_str[cmd_index])];
		cmd_length = strlen(rtkgdma_cmd_str[cmd_index]);
		if (cmd_length <= RTKGDMA_MAX_CMD_LENGTH - 1)
			cmd_pointer = &str[cmd_length];
		else {
			printk(KERN_ERR"rtkgdma_debug:%d array index:%d invalid!\n", __LINE__, cmd_length);
			return -EFAULT;
		}
	}

	rtkgdma_dbg_EXECUTE(cmd_index, &cmd_pointer);

	return count;
}
int GDMA_open(struct inode *inode, struct file *filp)
{
	return 0;
}

int GDMA_release(struct inode *inode, struct file *filp)
{
	return 0;
}

void Graphic_PatternColor(BOOLEAN enable, UINT32 patterncolor)
{
	KADP_HAL_GAL_CTRL_T st;

	st.en = enable;
	st.osdn = 1;
	st.clr = patterncolor;

	//printk(KERN_EMERG"\n  *******G*********** enable = %d ", enable);
	//printk(KERN_EMERG"\n  *******G*********** patterncolor = %x ", patterncolor);

	OSD_Pattern(&st);
	if (enable == 1)
		GDMA_ConfigOSDxEnable(GDMA_PLANE_OSD1, 1);
	else
		GDMA_ConfigOSDxEnable(GDMA_PLANE_OSD1, 0);
}

void Set_GDMA_ConfigOSDxEnable(BOOLEAN enable)
{
	GDMA_ConfigOSDxEnable(GDMA_PLANE_OSD1, enable);
}


#ifdef CONFIG_COMPAT
#if defined(CONFIG_ARM64)
long compat_GDMA_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
#else
int compat_GDMA_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
#endif
{
	return filp->f_op->unlocked_ioctl(filp, cmd,  (unsigned long)compat_ptr(arg));
}

#endif// CONFIG_COMPAT

/* int GDMA_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg) */
long GDMA_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	int ret =  -ENOTTY;

	DBG_PRINT(KERN_EMERG"GDMA: GDMA_ioctl, %d\n", _IOC_NR(cmd));
	if (_IOC_TYPE(cmd) != GDMA_IOC_MAGIC || _IOC_NR(cmd) > GDMA_IOC_MAXNR)
		return -ENOTTY;

	switch (cmd) {
	case GDMA_CONFIGURE_GRPAHIC_CANVAS:
		{
			VIDEO_RPC_VOUT_CONFIG_GRAPHIC_CANVAS data;
			if (copy_from_user((void *)&data, (const void __user *)arg, sizeof(VIDEO_RPC_VOUT_CONFIG_GRAPHIC_CANVAS))) {
				ret = -EFAULT;
				DBG_PRINT(KERN_EMERG"GDMA: ioctl code = %d failed!!!!!!!!!!!!!!!1\n", GDMA_CONFIGURE_GRPAHIC_CANVAS);
			} else
				ret = GDMA_ConfigureGraphicCanvas    ((VIDEO_RPC_VOUT_CONFIG_GRAPHIC_CANVAS *)&data);

			break;
		}
	case GDMA_CREATE_GRPAHIC_WINDOW:
		{
			VIDEO_RPC_VOUT_CREATE_GRAPHIC_WIN data;
			if (copy_from_user((void *)&data, (const void __user *)arg, sizeof(VIDEO_RPC_VOUT_CREATE_GRAPHIC_WIN))) {
				ret = -EFAULT;
				DBG_PRINT(KERN_EMERG"GDMA: ioctl code = %d failed!!!!!!!!!!!!!!!1\n", GDMA_CREATE_GRPAHIC_WINDOW);
			} else
				ret = GDMA_CreateGraphicWindow ((VIDEO_RPC_VOUT_CREATE_GRAPHIC_WIN    *)&data);

			DBG_PRINT(KERN_EMERG"GDMA: ioctl GDMA_CREATE_GRPAHIC_WINDOW, plane %d, winID %d\n", data.plane, ret);
			break;
		}
	case GDMA_DRAW_GRPAHIC_WINDOW:
		{
			VIDEO_RPC_VOUT_DRAW_GRAPHIC_WIN data;
			if (copy_from_user((void *)&data, (const void __user *)arg, sizeof(VIDEO_RPC_VOUT_DRAW_GRAPHIC_WIN))) {
				ret = -EFAULT;
				DBG_PRINT(KERN_EMERG"GDMA: ioctl code = %d failed!!!!!!!!!!!!!!!1\n", GDMA_DRAW_GRPAHIC_WINDOW);
			} else
				ret = GDMA_DrawGraphicWindow   ((VIDEO_RPC_VOUT_DRAW_GRAPHIC_WIN      *)&data);
			break;
		}
	case GDMA_MODIFY_GRPAHIC_WINDOW:
		{
			VIDEO_RPC_VOUT_MODIFY_GRAPHIC_WIN data;
			if (copy_from_user((void *)&data, (const void __user *)arg, sizeof(VIDEO_RPC_VOUT_MODIFY_GRAPHIC_WIN))) {
				ret = -EFAULT;
				DBG_PRINT(KERN_EMERG"GDMA: ioctl code = %d failed!!!!!!!!!!!!!!!1\n", GDMA_MODIFY_GRPAHIC_WINDOW);
			} else
				ret = GDMA_ModifyGraphicWindow ((VIDEO_RPC_VOUT_MODIFY_GRAPHIC_WIN    *)&data);
			break;
		}
	case GDMA_HIDE_GRPAHIC_WINDOW:
		{
			VIDEO_RPC_VOUT_HIDE_GRAPHIC_WIN data;
			if (copy_from_user((void *)&data, (const void __user *)arg, sizeof(VIDEO_RPC_VOUT_HIDE_GRAPHIC_WIN))) {
				ret = -EFAULT;
				DBG_PRINT(KERN_EMERG"GDMA: ioctl code = %d failed!!!!!!!!!!!!!!!1\n", GDMA_HIDE_GRPAHIC_WINDOW);
			} else
				ret = GDMA_HideGraphicWindow  (data.winID, data.plane, data.go);
			DBG_PRINT(KERN_EMERG"GDMA: ioctl GDMA_HIDE_GRPAHIC_WINDOW, plane %d, winID %d\n", data.plane, data.winID);
			break;
		}
	case GDMA_DELETE_GRPAHIC_WINDOW:
		{
			VIDEO_RPC_VOUT_DELETE_GRAPHIC_WIN data;
			if (copy_from_user((void *)&data, (const void __user *)arg, sizeof(VIDEO_RPC_VOUT_DELETE_GRAPHIC_WIN))) {
				ret = -EFAULT;
				DBG_PRINT(KERN_EMERG"GDMA: ioctl code = %d failed!!!!!!!!!!!!!!!1\n", GDMA_DELETE_GRPAHIC_WINDOW);
			} else
				ret = GDMA_DeleteGraphicWindow (data.winID, data.plane, data.go);
			DBG_PRINT(KERN_EMERG"GDMA: ioctl GDMA_DELETE_GRPAHIC_WINDOW, plane %d, winID %d\n", data.plane, data.winID);
			break;
		}
	case GDMA_CONFIGURE_OSD_PALETTE:
		{
			VIDEO_RPC_VOUT_CONFIG_OSD_PALETTE data;
			DBG_PRINT(KERN_EMERG"GDMA: ioctl GDMA_CONFIGURE_OSD_PALETTE\n");
			if (copy_from_user((void *)&data, (const void __user *)arg, sizeof(VIDEO_RPC_VOUT_CONFIG_OSD_PALETTE))) {
				ret = -EFAULT;
				DBG_PRINT(KERN_EMERG"GDMA: ioctl code = %d failed!!!!!!!!!!!!!!!1\n", GDMA_CONFIGURE_OSD_PALETTE);
			} else
				ret = GDMA_ConfigureOSDPalette ((VIDEO_RPC_VOUT_CONFIG_OSD_PALETTE    *)&data);
			break;
		}
	case GDMA_DISPLAY_GRPAHIC:
		{
			VIDEO_RPC_VOUT_DISPLAY_GRAPHIC data;
			DBG_PRINT(KERN_EMERG"GDMA: ioctl GDMA_DISPLAY_GRPAHIC\n");
			if (copy_from_user((void *)&data, (const void __user *)arg, sizeof(VIDEO_RPC_VOUT_DISPLAY_GRAPHIC))) {
				ret = -EFAULT;
				DBG_PRINT(KERN_EMERG"GDMA: ioctl code = %d failed!!!!!!!!!!!!!!!1\n", GDMA_DISPLAY_GRPAHIC);
			} else
				ret = GDMA_DisplayGraphic      ((VIDEO_RPC_VOUT_DISPLAY_GRAPHIC       *)&data);
			break;
		}
	case GDMA_QUERY_STATUS:
		ret = GDMA_OSD_INTST_get_osd1_vact_end(rtd_inl(GDMA_OSD_INTST_reg));


	/* below for 3D Grpahic */
	case GDMA_PLANE_CONFIG_DISPLAY_WIN:
		{
			GDMA_CONFIG_DISP_WIN data;
			if (copy_from_user((void *)&data, (const void __user *)arg, sizeof(GDMA_CONFIG_DISP_WIN))) {
				ret = -EFAULT;
				DBG_PRINT(KERN_EMERG"GDMA: ioctl code = %d failed!!!!!!!!!!!!!!!1\n", GDMA_PLANE_CONFIG_DISPLAY_WIN);
			} else
				ret = GDMA_ConfigureDisplayWindow ((GDMA_CONFIG_DISP_WIN *)&data);
			DBG_PRINT(KERN_EMERG"GDMA: ioctl GDMA_PLANE_CONFIG_DISPLAY_WIN plane %d, x %d, y %d w %d h %d\n", data.plane, data.dispWin.x, data.dispWin.y, data.dispWin.width, data.dispWin.height);
			break;
		}

	case GDMA_PLANE_DISPLAY:
		{
			GDMA_DISPLAY data;
			if (copy_from_user((void *)&data, (const void __user *)arg, sizeof(GDMA_DISPLAY))) {
				ret = -EFAULT;
				DBG_PRINT(KERN_EMERG"GDMA: ioctl code = %d failed!!!!!!!!!!!!!!!1\n", GDMA_PLANE_DISPLAY);
			} else
				ret = GDMA_Display      ((GDMA_DISPLAY *)&data);
			DBG_PRINT(KERN_EMERG"GDMA: ioctl GDMA_PLANE_DISPLAY, plane %d, zb %d\n", data.plane, data.zeroBuffer);
			break;
		}

	case GDMA_OSD1_ON_TOP:
		{
			bool data;
			if (copy_from_user((void *)&data, (const void __user *)arg, sizeof(bool))) {
				ret = -EFAULT;
				DBG_PRINT(KERN_EMERG"GDMA: ioctl code = %d failed!!!!!!!!!!!!!!!1\n", GDMA_OSD1_ON_TOP);
			} else
				ret = GDMA_OSD1OnTop      (data);
			DBG_PRINT(KERN_EMERG"GDMA: ioctl GDMA_OSD1_ON_TOP %d\n", data);
			break;
		}

	case GDMA_CONFIG_3D:
		{
			bool data;
			if (copy_from_user((void *)&data, (const void __user *)arg, sizeof(bool))) {
				ret = -EFAULT;
				DBG_PRINT(KERN_EMERG"GDMA: ioctl code = %d failed!!!!!!!!!!!!!!!1\n", GDMA_CONFIG_3D);
			} else
				ret = GDMA_Config3D      (data);
			DBG_PRINT(KERN_EMERG"GDMA: ioctl GDMA_CONFIG_3D, enable %d\n", data);
			break;
		}
	case GDMA_CONFIG_INPUTSRC_4K2K:
		{
			bool data;
			if (copy_from_user((void *)&data, (const void __user *)arg, sizeof(bool))) {
				ret = -EFAULT;
				DBG_PRINT(KERN_EMERG"GDMA: ioctl code = %d failed!!!!!!!!!!!!!!!1\n", GDMA_CONFIG_INPUTSRC_4K2K);
			} else
				ret = GDMA_ConfigInputSrc_4k2k      (data);
			DBG_PRINT(KERN_EMERG"GDMA: ioctl GDMA_CONFIG_SRC_4K2K, enable %d\n", data);
			break;
		}

	case GDMA_CONFIG_3D_UI:
		{
			bool data;
			if (copy_from_user((void *)&data, (const void __user *)arg, sizeof(bool))) {
				ret = -EFAULT;
				DBG_PRINT(KERN_EMERG"GDMA: ioctl GDMA_CONFIG_3D_UI failed !!\n");
			} else
				ret = GDMA_Config3dUi(data);
			break;
		}

	case GDMA_CONFIG_BOX_SRC_SIZE:
		{
			int data;
			if (copy_from_user((void *)&data, (const void __user *)arg, sizeof(int))) {
				ret = -EFAULT;
				DBG_PRINT(KERN_EMERG"GDMA: ioctl GDMA_CONFIG_BOX_SRC_SIZE failed !!\n");
			} else
				ret = GDMA_ConfigBoxSrcSize(data);
			break;
		}
	case GDMA_CONFIG_BOX_DST_SIZE:
		{
			int data;
			if (copy_from_user((void *)&data, (const void __user *)arg, sizeof(int))) {
				ret = -EFAULT;
				DBG_PRINT(KERN_EMERG"GDMA: ioctl GDMA_CONFIG_BOX_DST_SIZE failed !!\n");
			} else
				ret = GDMA_ConfigBoxDstSize(data);
			break;
		}
	case GDMA_CONFIG_ANDROID_LAYERS:
		{
			GRAPHIC_LAYERS_OBJECT data;
			if (copy_from_user((void *)&data, (const void __user *)arg, sizeof(GRAPHIC_LAYERS_OBJECT))) {
				ret = -EFAULT;
				DBG_PRINT(KERN_EMERG"GDMA: ioctl GDMA_CONFIG_ANDROID_LAYERS failed !!\n");
			} else
				ret = GDMA_ReceiveGraphicLayers(&data);
			break;
		}
//RTK mark for netfix
	case GDMA_CONFIG_OSD1_MIXER_ENABLE:
		{
			bool data;
			if (copy_from_user((void *)&data, (const void __user *)arg, sizeof(bool))) {
				ret = -EFAULT;
				DBG_PRINT(KERN_EMERG"GDMA: ioctl code = %d failed!!!!!!!!!!!!!!!1\n", GDMA_CONFIG_OSD_ENABLE);
			} else
				ret = GDMA_ConfigOSDxMixerEnableNoDelay(GDMA_PLANE_OSD1,data);
			DBG_PRINT(KERN_EMERG"GDMA: ioctl GDMA_CONFIG_SRC_4K2K, enable %d\n", data);
			break;
		}
	case GDMA_CONFIG_OSD4_MIXER_ENABLE:
		{
			bool data;
			if (copy_from_user((void *)&data, (const void __user *)arg, sizeof(bool))) {
				ret = -EFAULT;
				DBG_PRINT(KERN_EMERG"GDMA: ioctl code = %d failed!!!!!!!!!!!!!!!1\n", GDMA_CONFIG_OSD_ENABLE);
			} else
				ret = GDMA_ConfigOSDxMixerEnableNoDelay(GDMA_PLANE_OSD3,data);
			DBG_PRINT(KERN_EMERG"GDMA: ioctl GDMA_CONFIG_SRC_4K2K, enable %d\n", data);
			break;
		}
	case GDMA_CONFIG_OSD1OSD4_MIXER_ORDER:
		{
			bool data;
			if (copy_from_user((void *)&data, (const void __user *)arg, sizeof(bool))) {
				ret = -EFAULT;
				DBG_PRINT(KERN_EMERG"GDMA: ioctl code = %d failed!!!!!!!!!!!!!!!1\n", GDMA_CONFIG_OSD_ENABLE);
			} else
				ret = GDMA_ConfigOSD1OSD4MixerOrder(data);
			DBG_PRINT(KERN_EMERG"GDMA: ioctl GDMA_CONFIG_SRC_4K2K, enable %d\n", data);
			break;
		}
	case GDMA_CONFIG_OSD_ENABLE:
		{
			bool data;
			if (copy_from_user((void *)&data, (const void __user *)arg, sizeof(bool))) {
				ret = -EFAULT;
				DBG_PRINT(KERN_EMERG"GDMA: ioctl code = %d failed!!!!!!!!!!!!!!!1\n", GDMA_CONFIG_OSD_ENABLE);
			} else
				ret = GDMA_ConfigOSDEnable      (data);
			DBG_PRINT(KERN_EMERG"GDMA: ioctl GDMA_CONFIG_SRC_4K2K, enable %d\n", data);
			break;
		}
	case GDMA_CONFIG_OSD1_ENABLE:
		{
			bool data;
			if (copy_from_user((void *)&data, (const void __user *)arg, sizeof(bool))) {
				ret = -EFAULT;
				DBG_PRINT(KERN_EMERG"GDMA: ioctl GDMA_CONFIG_OSD1_ENABLE failed !!\n");
			} else
				ret = GDMA_ConfigOSDxEnable(GDMA_PLANE_OSD1, data);

			break;
		}
	case GDMA_CONFIG_OSD2_ENABLE:
		{
			bool data;
			if (copy_from_user((void *)&data, (const void __user *)arg, sizeof(bool))) {
				ret = -EFAULT;
				DBG_PRINT(KERN_EMERG"GDMA: ioctl GDMA_CONFIG_OSD2_ENABLE failed !!\n");
			} else
#if defined(CONFIG_ARCH_RTK287X)
				ret = GDMA_ConfigOSDxEnable(GDMA_PLANE_OSD3, data);
#else
				//ret = GDMA_ConfigOSDxEnable(GDMA_PLANE_OSD2, data);
#endif
			break;
		}
	case GDMA_CONFIG_OSD4_ENABLE:
		{
			ret = -EFAULT;
			break;
		}

	case GDMA_LOCK_VSYNC_SEM:
		{
			#ifdef ENABLE_VSYNC_NOTIFY
			/* fox modify use cookie to sync GDMA */
			u64 nsecs = 0 ;
			ret = 0;
			if ( GDMA_Lock_Vsync_Sem( &nsecs) != 0 )
			{
				pr_err(" GDMA_LOCK_VSYNC_SEM cp fail: %lld\n", nsecs);
				ret = -ERANGE;	
			}
			
			if( copy_to_user((void *)arg, &nsecs, sizeof(nsecs)) != 0 ) {
				pr_err(" GDMA_LOCK_VSYNC_SEM cp fail: %lld\n", nsecs);
				ret = -EFAULT;
			}
			
			#else
				ret = -EFAULT;
			
			#endif//ENABLE_VSYNC_NOTIFY
			
			break;
		}
	case GDMA_READ_Millet3:
		{
			ret = -EFAULT;
			break;
		}
	case GDMA_READ_PANEL_RES:
		{
			VO_RECTANGLE res;
			getDispSize(&res);
			if (copy_to_user((void __user *)arg, &res, sizeof(VO_RECTANGLE))) {
				pr_debug(" GDMA_READ_PANEL_RES ERROR *******\n");
				ret = -EFAULT;
			}
			DBG_PRINT(KERN_EMERG"GDMA: ioctl GDMA_READ_PANEL_RES, res.width=%d, res.height=%d\n", res.width, res.height);
			break;
		}
	case GDMA_GET_OSD1_BUFFS:
	{

		if (copy_to_user((void __user *)arg, &buf_phy_addr, sizeof(unsigned int)*BUF_PHY_NUM)) {
			pr_debug(" GDMA_GET_OSD1_BUFFS ERROR *******\n");
			ret = -EFAULT;
		}else
			ret = GDMA_SUCCESS;
		break;
	}

	case GDMA_GET_OSD1_PHY:
	{
        unsigned int phy_addr;
        phy_addr = rtd_inl(GDMA_DBG_OSD_WI_2_reg);

		if (copy_to_user((void __user *)arg, &phy_addr, sizeof(unsigned int))) {
			pr_debug(" GDMA_GET_OSD1_PHY ERROR *******\n");
			ret = -EFAULT;
		}else
			ret = GDMA_SUCCESS;
		break;
	}
	case GDMA_GET_OSD1_SIZE:
	{
		unsigned int phy_size;

		phy_size = rtd_inl(GDMA_DBG_OSD_WI_4_reg)*GDMA_DBG_OSD_WI_0_get_height(rtd_inl(GDMA_DBG_OSD_WI_0_reg));
			
		if (copy_to_user((void __user *)arg, &phy_size, sizeof(unsigned int))) {
			pr_debug(" GDMA_GET_OSD1_SIZE ERROR *******\n");
			ret = -EFAULT;
		}else
			ret = GDMA_SUCCESS;
		break;
	}
	case GDMA_GET_OSD1_WIDTH:
	{
		unsigned int phy_width;

		phy_width = GDMA_DBG_OSD_WI_0_get_width(rtd_inl(GDMA_DBG_OSD_WI_0_reg));

		if (copy_to_user((void __user *)arg, &phy_width, sizeof(unsigned int))) {
			pr_debug(" GDMA_GET_OSD1_WIDTH ERROR *******\n");
			ret = -EFAULT;
		}else
			ret = GDMA_SUCCESS;
		break;
	}
	case GDMA_GET_OSD1_HEIGHT:
	{
		unsigned int phy_height;

		phy_height = GDMA_DBG_OSD_WI_0_get_height(rtd_inl(GDMA_DBG_OSD_WI_0_reg));
		if (copy_to_user((void __user *)arg, &phy_height, sizeof(unsigned int))) {
			pr_debug(" GDMA_GET_OSD1_HEIGHT ERROR *******\n");
			ret = -EFAULT;
		} else
			ret = GDMA_SUCCESS;
		break;
	}

    case GDMA_GET_FENCE_INFO:
        {
            GDMA_FENCE_INFO data = { .fence_fd = -1, .fence_handle = 0 };
            ret = 0;
            if(GDMA_GetFenceInfo(&data) == GDMA_SUCCESS){
                if (copy_to_user((void __user *)arg, &data, sizeof(GDMA_FENCE_INFO))) {
                        GDMA_PRINT(2, " GDMA_GET_GDMA_FEND_INFO ERROR *******\n");
                        ret = -EFAULT;
                }
            }else{
                ret = -EFAULT;
            }
        }
	break;
	case GDMA_READ_Millet3_INPUTSRC_4K2K:
		{
			ret = -EFAULT;
			break;
		}
	case GDMA_READ_PANEL_SETTING:
		{
			int  type;
			type = GDMA_GetPanelPixelMode();
			if (copy_to_user((void __user *)arg, &type, sizeof(type))) {
				pr_debug(" GDMA_READ_PANEL_SETTING ERROR *******\n");
				ret = -EFAULT;
			}
			break;
		}
	case GDMA_DEBUG_CONTROL:
		{
			KADP_HAL_GAL_CTRL_T data;
			if (copy_from_user((void *)&data, (const void __user *)arg, sizeof(KADP_HAL_GAL_CTRL_T)))
				ret = -EFAULT;
			else
				ret = GDMA_Debug((KADP_HAL_GAL_CTRL_T *)&data);

			if(data.type == GDMA_DBG_CMD_OSD_GET_INFO){
				if(copy_to_user((void __user *)arg, &data, sizeof(KADP_HAL_GAL_CTRL_T)))
					ret = -EFAULT;
			}
			break;
		}
	case GDMA_CONFIG_3D_OFFSET:
		{
			CONFIG_3D_OFFSET_STRUCT data;
			if (copy_from_user((void *)&data, (const void __user *)arg, sizeof(CONFIG_3D_OFFSET_STRUCT))) {
				ret = -EFAULT;
				DBG_PRINT(KERN_EMERG"GDMA: ioctl code = %d failed!!!!!!!!!!!!!!!1\n", GDMA_CONFIG_3D_OFFSET);
			} else
				ret = GDMA_Config3DOffset ((CONFIG_3D_OFFSET_STRUCT *)&data);
			DBG_PRINT(KERN_EMERG"GDMA: ioctl GDMA_CONFIG_3D_OFFSET, plane %d, dir %d, offset %d\n", data.plane, data.closeToViewer, data.offset);
			break;
		}

	case GDMA_CONFIG_BLENDING:
		{
			CONFIG_BLENDING_STRUCT data;
			if (copy_from_user((void *)&data, (const void __user *)arg, sizeof(CONFIG_BLENDING_STRUCT))) {
				ret = -EFAULT;
				DBG_PRINT(KERN_EMERG"GDMA: ioctl code = %d failed!!!!!!!!!!!!!!!1\n", GDMA_CONFIG_BLENDING);
			} else
				ret = GDMA_ConfigBlending ((CONFIG_BLENDING_STRUCT *)&data);
			DBG_PRINT(KERN_EMERG"GDMA: ioctl GDMA_CONFIG_BLENDING, %d, %d, %d\n", data.blendingOSD1En, data.blendingOSD2En, data.blendingCursorEn);
			break;
		}

	case GDMA_CONFIG_VFLIP:
		{
			bool data;
			if (copy_from_user((void *)&data, (const void __user *)arg, sizeof(bool))) {
				ret = -EFAULT;
				DBG_PRINT(KERN_EMERG"GDMA: ioctl code = %d failed!!!!!!!!!!!!!!!1\n", GDMA_CONFIG_VFLIP);
			} else
				ret = GDMA_ConfigVFlip(data);
			DBG_PRINT(KERN_EMERG"GDMA: ioctl GDMA_CONFIG_VFLIP, enable %d\n", data);
			break;
		}

	case GDMA_CONFIG_NONBLOCKING:
		{
			bool data;
			if (copy_from_user((void *)&data, (const void __user *)arg, sizeof(bool))) {
				ret = -EFAULT;
				DBG_PRINT(KERN_EMERG"GDMA: ioctl code = %d failed!!!!!!!!!!!!!!!1\n", GDMA_CONFIG_NONBLOCKING);
			} else
				ret = GDMA_ConfigNonBlocking(data);
			DBG_PRINT(KERN_EMERG"GDMA: ioctl GDMA_CONFIG_NONBLOCKING, enable %d\n", data);
			break;
		}

	case GDMA_CONFIG_OSDCOMP_WRITEBACK_OFFLINE:
		{
			ret = -EFAULT;
			break;
		}

	case GDMA_CONFIG_OSDCOMP_WRITEBACK_ONLINE:
		{
			ret = -EFAULT;
			break;
		}

	case GDMA_CONFIG_QUEUEFLUSH:
		{
			GDMA_QueueFlush();
			break;
		}
	case GDMA_CONFIG_OSDSPLIT:
		{
			unsigned int data=0;
			if(change_OSD_mode == GDMA_SPLIT_MODE_1
			  || change_OSD_mode == GDMA_SPLIT_MODE_0)
			{		
				if (copy_from_user((void *)&data, (const void __user *)arg, sizeof(unsigned int))) {
					ret = -EFAULT;
					DBG_PRINT(KERN_EMERG"GDMA: ioctl code = %d failed!!!!!!!!!!!!!!!1\n", GDMA_CONFIG_NONBLOCKING);
				} else{
					change_OSD_mode = data;

	#if USE_RLINK_INTERFACE
					GDMA_rlink_split(data);
	#endif
					ret = 0;
				}
				DBG_PRINT(KERN_EMERG"GDMA: ioctl GDMA_CONFIG_OSDSPLIT, enable %d\n", data);
			}
			break;
		}
	case GDMA_CONFIG_GETOSDSPLIT:
		{
			unsigned int data=0;
			data = change_OSD_mode;			

			if(copy_to_user((void __user *)arg, &data, sizeof(unsigned int)))
				ret = -EFAULT;

			break;			
		}
		case GDMA_CONFIG_SEND_ES_OSD:
		{
			OSD_ES_SEND_DATA data;
			unsigned int data_size;
			int s_h;
			
			if (copy_from_user((void *)&data, (const void __user *)arg, sizeof(OSD_ES_SEND_DATA))) {
				ret = -EFAULT;
				DBG_PRINT(KERN_EMERG"GDMA: ioctl OSD_ES_SEND_DATA failed !!\n");
				break;
			}
			data_size = data.size;
			s_h = (data.size/(TWOK_W*4))+1;
			if(s_h > ES_OSD_MAX_H)
				s_h = ES_OSD_MAX_H;
			GDMA_ES_OSD_Data(TWOK_W,s_h,
							 TWOK_W,s_h,es_osd_addr_count,es_osd_addr,data.phy_addr); 			

			break;
		}

	case GDMA_CONFIG_OSD_SMOOTH_TOGGLE:
		{

			ret = -EFAULT;
			GDMA_OSDSmoothToggle();
			break;
		}

	case GDMA_CONFIG_OSD_SHIFT: //for OSD shift
		{
			if (copy_from_user((void *)&g_osdshift_ctrl, (const void __user *)arg, sizeof(CONFIG_OSDSHIFT_STRUCT))) {
				ret = -EFAULT;
				memset(&g_osdshift_ctrl, 0x00, sizeof(CONFIG_OSDSHIFT_STRUCT));
				DBG_PRINT(KERN_EMERG"GDMA: ioctl CONFIG_OSDSHIFT_STRUCT failed !!\n");
				break;
			}
			if(osd_shift_proc() == false) {
				ret = -EFAULT;
			}
			break;
		}
#if GDMA_GAMMA
	case GDMA_CONFIG_OSD_GAMMA_POSITION:
	{
		unsigned int data;
		if (copy_from_user((void *)&data, (const void __user *)arg, sizeof(unsigned int))) {
			ret = -EFAULT;
			DBG_PRINT(KERN_EMERG"GDMA: ioctl code = %d failed!!!!!!!!!!!!!!!1\n", GDMA_CONFIG_OSD_GAMMA_POSITION);
		} else
			ret = GDMA_ConfigGammaPosition(data);
		DBG_PRINT(KERN_EMERG"GDMA: ioctl GDMA_CONFIG_OSD_GAMMA_POSITION, enable %d\n", data);
		break;
	}
	case GDMA_CONFIG_OSD_GAMMA_COLOR_MAPPING:
	{
		GAMMA_COLOR_MAPPING_DATA data;
		if (copy_from_user((void *)&data, (const void __user *)arg, sizeof(GAMMA_COLOR_MAPPING_DATA))) {
			ret = -EFAULT;
			DBG_PRINT(KERN_EMERG"GDMA: ioctl code = %d failed!!!!!!!!!!!!!!!1\n", GDMA_CONFIG_OSD_GAMMA_POSITION);
		} else
			ret = GDMA_ConfigGammaColorMapping(&data);
		DBG_PRINT(KERN_EMERG"GDMA: ioctl GDMA_CONFIG_OSD_GAMMA_POSITION, enable %d\n", data);
		break;
	}
	case GDMA_CONFIG_OSD_GAMMA_OP:
	{
		GDMA_GAMMA_OP data;
		if (copy_from_user((void *)&data, (const void __user *)arg, sizeof(GAMMA_COLOR_MAPPING_DATA))) {
			ret = -EFAULT;
			DBG_PRINT(KERN_EMERG"GDMA: ioctl code = %d failed!!!!!!!!!!!!!!!1\n", GDMA_CONFIG_OSD_GAMMA_POSITION);
		} else
			ret = GDMA_GammaOperation(&data);
		DBG_PRINT(KERN_EMERG"GDMA: ioctl GDMA_CONFIG_OSD_GAMMA_OP, enable %d\n", data);
		break;
	}
#endif
	case GDMA_GET_OSD_STATUS:
		{
			unsigned int st = 0;
			st |= GDMA_OSD1_CTRL_get_osd1_en(rtd_inl(GDMA_OSD1_CTRL_reg))?1:0;
			st |= GDMA_OSD2_CTRL_get_osd2_en(rtd_inl(GDMA_OSD2_CTRL_reg))?2:0;
			st |= GDMA_OSD3_CTRL_get_osd3_en(rtd_inl(GDMA_OSD3_CTRL_reg))?4:0;
			if (copy_to_user((void __user *)arg, &st, sizeof(st))) {
				ret = -EFAULT;
			}
			ret = 0;
			break;
		}
#if OSD_DETECTION_ALPHA_SUPPORT
		case GDMA_CONFIG_OSD_GET_GOLBAL_ALPHA_VALUE:
		{
			unsigned int data;
			if (GDMA_GetAlphaOsdDetectionParameter(&data) == GDMA_SUCCESS) {
				if (copy_to_user((void __user *)arg, &data, sizeof(int))) {
					ret = -EFAULT;
					DBG_PRINT(KERN_EMERG"GDMA: ioctl code = %d failed!!!!!!!!!!!!!!!1\n", GDMA_CONFIG_OSD_GET_GOLBAL_ALPHA_VALUE);
				} else
					ret = GDMA_SUCCESS;
			}
			break;
		}
		case GDMA_CONFIG_OSD_SET_GOLBAL_ALPHA: 
		{
			unsigned int data;
			if (copy_from_user((void *)&data, (const void __user *)arg, sizeof(int))) {
				ret = -EFAULT;
				DBG_PRINT(KERN_EMERG"GDMA: ioctl code = %d failed!!!!!!!!!!!!!!!1\n", GDMA_CONFIG_OSD_SET_GOLBAL_ALPHA);
			} else {
				if (GDMA_SetAlphaOsdDetectionParameter(data) == GDMA_SUCCESS) {
					ret = GDMA_SUCCESS;
				}
			}
			break;
		}
		case GDMA_CONFIG_OSD_GET_GOLBAL_ALPHA: 
		{
			GDMA_ALPHA_TABLE table;
			if (GDMA_GetAlphaOsdDetectionParametertable(&table) == GDMA_SUCCESS) {
				if (copy_to_user((void __user *)arg, &table, sizeof(GDMA_ALPHA_TABLE))) {
					ret = -EFAULT;
					DBG_PRINT(KERN_EMERG"GDMA: ioctl code = %d failed!!!!!!!!!!!!!!!1\n", GDMA_GET_OSD_DETECTION_STATUS);
				} else
					ret = GDMA_SUCCESS;
			}
			break;
		}
#endif
#if OSD_DETECTION_SUPPORT
	case GDMA_GET_OSD_DETECTION_STATUS: 
		{
			int status;
			if (GDMA_GetOsdDetectionStatus(&status) == GDMA_SUCCESS) {
				if (copy_to_user((void __user *)arg, &status, sizeof(int))) {
					ret = -EFAULT;
					DBG_PRINT(KERN_EMERG"GDMA: ioctl code = %d failed!!!!!!!!!!!!!!!1\n", GDMA_GET_OSD_DETECTION_STATUS);
				} else
					ret = GDMA_SUCCESS;
			}
			break;
		}
	case GDMA_SET_OSD_DETECTION_PARAMETER: 
		{
			GDMA_OSD_DETECTION_PARAMETER_T detectionParameter;
			if (copy_from_user((void *)&detectionParameter, (const void __user *)arg, sizeof(GDMA_OSD_DETECTION_PARAMETER_T))) {
				ret = -EFAULT;
				DBG_PRINT(KERN_EMERG"GDMA: ioctl code = %d failed!!!!!!!!!!!!!!!1\n", GDMA_SET_OSD_DETECTION_PARAMETER);
			} else {
				if (GDMA_SetOsdDetectionParameter(&detectionParameter) == GDMA_SUCCESS) {
					ret = GDMA_SUCCESS;
				}
			}
			break;
		}
#endif
#if USE_RLINK_INTERFACE		
	case GDMA_CONFIG_OSD_2893_EANBLE: 
	{
		unsigned int data;
		if (copy_from_user((void *)&data, (const void __user *)arg, sizeof(unsigned int))) {
			ret = -EFAULT;
			DBG_PRINT(KERN_EMERG"GDMA: ioctl code = %d failed!!!!!!!!!!!!!!!1\n", GDMA_CONFIG_OSD_2893_EANBLE);
		} else
			ret = GDMA_estiker_osd_enable(data);
		DBG_PRINT(KERN_EMERG"GDMA: ioctl GDMA_CONFIG_OSD_2893_EANBLE, enable %d\n", data);
		break;
	}
	case GDMA_CONFIG_OSD_ESTICKER_EANBLE: 
	{
		unsigned int data;
		if (copy_from_user((void *)&data, (const void __user *)arg, sizeof(unsigned int))) {
			ret = -EFAULT;
			DBG_PRINT(KERN_EMERG"GDMA: ioctl code = %d failed!!!!!!!!!!!!!!!1\n", GDMA_CONFIG_OSD_ESTICKER_EANBLE);
		} else
			ret = GDMA_estiker_enable(data);
		DBG_PRINT(KERN_EMERG"GDMA: ioctl GDMA_CONFIG_OSD_ESTICKER_EANBLE, enable %d\n", data);
		break;
	}
	case GDMA_CONFIG_OSD_ESTICKER_X_P:
	{
		unsigned int data;
		if (copy_from_user((void *)&data, (const void __user *)arg, sizeof(unsigned int))) {
			ret = -EFAULT;
			DBG_PRINT(KERN_EMERG"GDMA: ioctl code = %d failed!!!!!!!!!!!!!!!1\n", GDMA_CONFIG_OSD_ESTICKER_EANBLE);
		} else
			ret = GDMA_estiker_osd_x_p(data);
		DBG_PRINT(KERN_EMERG"GDMA: ioctl GDMA_CONFIG_OSD_ESTICKER_EANBLE, enable %d\n", data);
		break;
	}
	case GDMA_CONFIG_OSD_ESTICKER_Y_P:
	{
		unsigned int data;
		if (copy_from_user((void *)&data, (const void __user *)arg, sizeof(unsigned int))) {
			ret = -EFAULT;
			DBG_PRINT(KERN_EMERG"GDMA: ioctl code = %d failed!!!!!!!!!!!!!!!1\n", GDMA_CONFIG_OSD_ESTICKER_EANBLE);
		} else
			ret = GDMA_estiker_osd_y_p(data);
		DBG_PRINT(KERN_EMERG"GDMA: ioctl GDMA_CONFIG_OSD_ESTICKER_EANBLE, enable %d\n", data);
		break;
	}
#endif
	case GDMA_CAPTURE_FRAMEFUBBER:
	{
		//KADP_HAL_GAL_SURFACE_INFO_T dSurface;
		KGAL_SURFACE_INFO_T dSurface;

		if (copy_from_user((void *)&dSurface, (const void __user *)arg, sizeof(KGAL_SURFACE_INFO_T))) {
			ret = -EFAULT;
			DBG_PRINT(KERN_EMERG"GDMA: ioctl code = %d failed!!!!!!!!!!!!!!!1\n", GDMA_CAPTURE_FRAMEFUBBER);
		} else {
			if (GDMA_CaptureFramebuffer(&dSurface) == GDMA_SUCCESS) {
				ret = GDMA_SUCCESS;
			}
		}
		break;
	}
	case GDMA_READ_UI_RESOLUTION:
	{
		int ui_resolution=3840;
		int ui_re=UI_RESOLUTION_4K;
		//if (GDMA_GetOsdDetectionStatus(&status) == GDMA_SUCCESS) {
		ui_re = get_ui_resolution();
		//printk(KERN_EMERG"func=%s line=%d ui_re=%d\n",__FUNCTION__,__LINE__,ui_re);
		if((ui_re == UI_RESOLUTION_4K) || (ui_re ==UI_RESOLUTION_OTHER))
			ui_resolution=3840;
		else if(ui_re == UI_RESOLUTION_2K)
			ui_resolution=1920;
		if (copy_to_user((void __user *)arg, &ui_resolution, sizeof(int))) {
			ret = -EFAULT;
			DBG_PRINT(KERN_EMERG"GDMA: ioctl code = %d failed!!!!!!!!!!!!!!!1\n", GDMA_READ_UI_RESOLUTION);
		} else
			ret = GDMA_SUCCESS;
	
		break;
	}	
	case GDMA_CONFIG_OSD_STOP_UPDATE:
	{
		unsigned int data;
		if (copy_from_user((void *)&data, (const void __user *)arg, sizeof(unsigned int))) {
			ret = -EFAULT;
			DBG_PRINT(KERN_EMERG"GDMA: ioctl code = %d failed!!!!!!!!!!!!!!!1\n", GDMA_CONFIG_OSD_STOP_UPDATE);
		} else{
			enable_stop_update = data;
			ret = GDMA_SUCCESS;
		}
		DBG_PRINT(KERN_EMERG"GDMA: ioctl GDMA_CONFIG_OSD_STOP_UPDATE, enable %d\n", data);
		break;
	}
	case GDMA_CONFIG_OSD_MONITOR_WIN: 
	{
		OSD_MONITOR_WIN_CTL data;
		if (copy_from_user((void *)&data, (const void __user *)arg, sizeof(OSD_MONITOR_WIN_CTL))) {
			ret = -EFAULT;
			DBG_PRINT(KERN_EMERG"GDMA: ioctl code = %d failed!!!!!!!!!!!!!!!1\n", GDMA_CONFIG_OSD_MONITOR_WIN);
		} else {
			if(data.enable == 2){
				GDMA_ReceivePicture(&glastinfo);
			}else{
				gGDMA_Monitor_ctl.input_x = data.input_x;
				gGDMA_Monitor_ctl.input_y = data.input_y;
				gGDMA_Monitor_ctl.input_w = data.input_w;
				gGDMA_Monitor_ctl.input_h = data.input_h;
				gGDMA_Monitor_ctl.output_x = data.output_x;
	            gGDMA_Monitor_ctl.output_y = data.output_y;			
	 			if((data.enable == 1) && (gGDMA_Monitor_ctl.input_w > 1920)){
					printk(KERN_ERR" ERROR !! input_w > 1920. \n");
					gGDMA_Monitor_ctl.enable = 0;
				}else{
					gGDMA_Monitor_ctl.enable = data.enable;
					if(gGDMA_Monitor_ctl.enable == 1){
						GDMA_ReceivePicture(&glastinfo);
					}
				}
			}
		}
		break;
	}

	default:
		pr_debug(KERN_EMERG"GDMA: ioctl code = %d is invalid!!!!!!!!!!!!!!!1\n", cmd);
		break;
	}

	return ret;
}


static void GDMA_setup_cdev(gdma_dev *dev, int index)
{
	int err, devno = MKDEV(gdma_major, gdma_minor + index);

	cdev_init(&dev->cdev, &gdma_fops);
	dev->cdev.owner = THIS_MODULE;
	dev->cdev.ops   = &gdma_fops;
	err = cdev_add (&dev->cdev, devno, 1);

	if (err)
		DBG_PRINT(KERN_EMERG "Error %d adding se%d", err, index);

	device_create(gdma_class, NULL, MKDEV(gdma_major, index), NULL, "gdma%d", index);
}

#if defined(CONFIG_ARM64)
static char *GDMA_devnode(struct device *dev, umode_t *mode) {
#else
static char *GDMA_devnode(struct device *dev, mode_t *mode) {
#endif
	return NULL;
}
/*
long long GetSystemClock(void)
{
	unsigned int hi, low;

	do {
		hi  = rtd_inl(MIS_SCPU_CLK90K_HI_reg);
		low = rtd_inl(MIS_SCPU_CLK90K_LO_reg);
	} while (hi != rtd_inl(MIS_SCPU_CLK90K_HI_reg));

	return ((long long)hi << 32) | low;
}
*/

#if IS_ENABLED(CONFIG_RTK_KDRIVER_SUPPORT)
extern int pll_info_bush(void);
extern int pll_info_disp(void);

#else
/*
*   sel: 0: return BusH clock
*        1: return Ddomain clock
*/
int GDMA_BusH_DClk_info(int sel)
{
	unsigned int ret_clk = 27;
	unsigned int value_m = 0, value_n = 0, value_o = 0, value_f = 0;
	unsigned int tmp = 0;
	unsigned int pll_pdiv=1;
	unsigned int pll_div=1;

	if (sel == 0) {
		/*for K4 PLL
		BUSH = 27/(PDIV+1)*(Ncode+3+Fcode/2048)/(DIV+1)
		*/

		if ((rtd_inl(STB_ST_BUSCLK_reg) & STB_ST_BUSCLK_bus_clksel_mask) == 0) {
			tmp = rtd_inl(PLL_REG_SYS_PLL_BUSH1_reg);
			pll_pdiv = PLL_REG_SYS_PLL_BUSH1_get_pllbus_pdiv_h(tmp);
			pll_div = PLL_REG_SYS_PLL_BUSH1_get_pllbus_div_h(tmp);

			tmp = rtd_inl(PLL_REG_SYS_PLL_BUSH2_reg);
			value_f = PLL_REG_SYS_PLL_BUSH2_get_pllbus_fcode_h(tmp);
			value_n = PLL_REG_SYS_PLL_BUSH2_get_pllbus_ncode_h(tmp);

			/*baker, for more precise, add multiple 2 at clk and
			 divide 2 in the end*/

			ret_clk = 27*2/(pll_pdiv+1)*(value_n + 3 + (value_f >> 11))/(pll_div+1)/2;
		} else {
			ret_clk = 27;
		}
		DBG_PRINT("BusH clock: %d Mhz \n", ret_clk);
	} else if (sel == 1) {
		tmp = rtd_inl(PLL27X_REG_SYS_PLL_DISP1_reg);
		value_m = PLL27X_REG_SYS_PLL_DISP1_get_dpll_m(tmp);
		value_n = PLL27X_REG_SYS_PLL_DISP1_get_dpll_n(tmp);
		tmp = rtd_inl(PLL27X_REG_PLL_SSC0_reg);
		value_f = PLL27X_REG_PLL_SSC0_fcode_t_ssc(tmp);
		tmp = rtd_inl(PLL27X_REG_SYS_PLL_DISP3_reg);
		value_o = PLL27X_REG_SYS_PLL_DISP3_get_dpll_o(tmp);

		ret_clk = 27*(value_m+3+(value_f >> 11))/(value_n+2)/(1<<value_o);
		DBG_PRINT("D-domain clock: %d Mhz \n", ret_clk);
	}

	return ret_clk;
}
#endif

irqreturn_t GDMA_irq_handler(int irq, void *dev_id)
{
	/*volatile decomp_inten_RBUS decomp_inten_reg;*/
	volatile gdma_dma_intst_RBUS dma_intst_reg;
	gdma_dev *gdma = dev_id;
#if (defined(CONFIG_ARCH_RTK288O) || defined(CONFIG_ARCH_RTK6748)) && defined(THRESHOLD_DETECT)
	unsigned int i;
#endif
	volatile int osd1_vend = 0, osd1_vend_3D = 0, osd_vend = 0;/* , i; */
	volatile int osd1_finish = 0;
	volatile int osd1_sync = 0;
	volatile int osd1_acc_under = 0;

#if !defined(CONFIG_ARCH_RTK2875) && !defined(CONFIG_ARCH_RTK2885M) && !defined(CONFIG_ARCH_RTK6748)
	volatile ppoverlay_lflag_gen_RBUS lflag_gen_reg;
#endif    
	/*int lineCnt = (*(volatile u32 *)GET_MAPPED_RBUS_ADDR(0xB8028248)) & 0xfff; */
	volatile unsigned int osd_intst_reg = rtd_inl(GDMA_OSD_INTST_reg);
	volatile unsigned int osd_intst_3d_reg = rtd_inl(GDMA_OSD_INTST_3D_reg);
	volatile unsigned int osd_inten_reg = rtd_inl(GDMA_OSD_INTEN_reg);
	volatile unsigned int osd_inten_3d_reg = rtd_inl(GDMA_OSD_INTEN_3D_reg);

#if defined(_TEST_CODE_GDMA_) || defined(_TEST_CODE_OSDCOMP_)
#if !defined(CONFIG_ARM64)
	flush_cache_all();
#endif
#endif
	if (GDMA_OSD_INTEN_get_osd1_fin(osd_inten_reg))
		osd1_finish = GDMA_OSD_INTST_get_osd1_fin(osd_intst_reg);
	
	if (GDMA_OSD_INTEN_get_osd1_vsync(osd_inten_reg))
		osd1_sync = GDMA_OSD_INTST_get_osd1_vsync(osd_intst_reg);

#ifndef USING_GDMA_VSYNC
	if (GDMA_OSD_INTEN_get_osd1_vact_end(osd_inten_reg))
		osd1_vend     = GDMA_OSD_INTST_get_osd1_vact_end(osd_intst_reg);
#else
	if (GDMA_OSD_INTEN_get_osd1_vsync(osd_inten_reg))
		osd1_vend     = GDMA_OSD_INTST_get_osd1_vsync(osd_intst_reg);
#endif
	if (GDMA_OSD_INTEN_3D_get_osd1_vact_end(osd_inten_3d_reg))
		osd1_vend_3D  = GDMA_OSD_INTST_3D_get_osd1_vact_end(osd_intst_3d_reg);
	
	osd_vend      = gdma->ctrl.enable3D ? (osd1_vend_3D || (osd1_vend && gdma->ctrl.forceUpdate)) : osd1_vend;

#ifdef USING_GDMA_VSYNC
	if (GDMA_CTRL_get_osd1_prog_done(rtd_inl(GDMA_CTRL_reg)) == 0)
#endif
	{
		if (osd_vend) {
#ifdef TRIPLE_BUFFER_SEMAPHORE
			int lineCnt = rtd_inl(PPOVERLAY_new_meas0_linecnt_real_reg) & 0x1fff;
			if (lineCnt < (rtd_inl(PPOVERLAY_DV_DEN_Start_End_reg) & 0x1fff))
				;/*DBG_PRINT(KERN_EMERG"gdma:vend isr @ %d\n", lineCnt);*/ /*  check interrupt timing in front-porch time? */
#endif
#if !defined(CONFIG_ARCH_RTK2875) && !defined(CONFIG_ARCH_RTK2885M) && !defined(CONFIG_ARCH_RTK6748)
			if (gdma->ctrl.updateOSD1Offset) {
				rtd_outl(PPOVERLAY_OSD1_3D_OFFSET_reg, *(volatile u32 *)&gdma->osd3Doffset[0]);
				gdma->ctrl.updateOSD1Offset = 0;
				*(volatile u32 *)&lflag_gen_reg = rtd_inl(PPOVERLAY_LFLAG_GEN_reg);
				if (gdma->osd3Doffset[0].left_offset == 0 && gdma->osd3Doffset[0].right_offset == 0) {
					lflag_gen_reg.gdma_osd_line_alt = 0;
					rtd_outl(PPOVERLAY_LFLAG_GEN_reg, lflag_gen_reg.regValue);
				} else {
					lflag_gen_reg.gdma_osd_line_alt = 1;
					rtd_outl(PPOVERLAY_LFLAG_GEN_reg, lflag_gen_reg.regValue);
				}
			}
			if (gdma->ctrl.updateOSD2Offset) {
				rtd_outl(PPOVERLAY_OSD2_3D_OFFSET_reg, *(volatile u32 *)&gdma->osd3Doffset[1]);
				gdma->ctrl.updateOSD2Offset = 0;
				*(volatile u32 *)&lflag_gen_reg = rtd_inl(PPOVERLAY_LFLAG_GEN_reg);
				if (gdma->osd3Doffset[1].left_offset == 0 && gdma->osd3Doffset[1].right_offset == 0) {
					lflag_gen_reg.gdma_osd_line_alt = 0;
					rtd_outl(PPOVERLAY_LFLAG_GEN_reg, lflag_gen_reg.regValue);
				} else {
					lflag_gen_reg.gdma_osd_line_alt = 1;
					rtd_outl(PPOVERLAY_LFLAG_GEN_reg, lflag_gen_reg.regValue);
				}
			}
			if (gdma->ctrl.updateOSD3Offset) {
				rtd_outl(PPOVERLAY_OSD3_3D_OFFSET_reg, *(volatile u32 *)&gdma->osd3Doffset[2]);
				gdma->ctrl.updateOSD3Offset = 0;
				*(volatile u32 *)&lflag_gen_reg = rtd_inl(PPOVERLAY_LFLAG_GEN_reg);
				if (gdma->osd3Doffset[2].left_offset == 0 && gdma->osd3Doffset[2].right_offset == 0) {
					lflag_gen_reg.gdma_osd_line_alt = 0;
					rtd_outl(PPOVERLAY_LFLAG_GEN_reg, lflag_gen_reg.regValue);
				} else {
					lflag_gen_reg.gdma_osd_line_alt = 1;
					rtd_outl(PPOVERLAY_LFLAG_GEN_reg, lflag_gen_reg.regValue);
				}
			}
#endif
#if 0
			decomp_inten_reg.regValue = rtd_inl(GDMA_DECOMP_INTEN_reg);
			decomp_inten_reg.head_error_clr = 1;
			decomp_inten_reg.pix_fifo_underflow_clr = 1;
			rtd_outl(GDMA_DECOMP_INTEN_reg, decomp_inten_reg.regValue);
			decomp_inten_reg.head_error_clr = 0;
			decomp_inten_reg.pix_fifo_underflow_clr = 0;
			rtd_outl(GDMA_DECOMP_INTEN_reg, decomp_inten_reg.regValue);
#endif
			//GDMA_Update (gdma, 1);	// move to main process

			if (osd1_vend_3D)
				rtd_outl(GDMA_OSD_INTST_3D_reg, GDMA_OSD_INTST_3D_write_data(0) | GDMA_OSD_INTST_3D_osd1_vact_end(1));
			else
#ifndef USING_GDMA_VSYNC
				rtd_outl(GDMA_OSD_INTST_reg, GDMA_OSD_INTST_write_data(0) | GDMA_OSD_INTST_osd1_vact_end(1));
#else
				rtd_outl(GDMA_OSD_INTST_reg, GDMA_OSD_INTST_write_data(0) | GDMA_OSD_INTST_osd1_vsync(1));
#endif

			gdma->ctrl.forceUpdate = 0;
		}
#ifdef TRIPLE_BUFFER_SEMAPHORE
		/*else if (osd1_sync) {*/
		if (osd1_sync) {

			#ifdef ENABLE_VSYNC_NOTIFY
				atomic64_set(&gGDMA_VSYNC_USER_TIME, ktime_get());
			#endif
	
			if (gSemaphore[GDMA_PLANE_OSD1] > 0 && gdma->pendingRPC[GDMA_PLANE_OSD1] && (GDMA_CTRL_get_osd1_prog_done(rtd_inl(GDMA_CTRL_reg)) == 0)) {
				gdma->pendingRPC[GDMA_PLANE_OSD1] = 0;
				up(&gdma->updateSem[GDMA_PLANE_OSD1]);
			}
			if (gSemaphore[GDMA_PLANE_OSD2] > 0 && gdma->pendingRPC[GDMA_PLANE_OSD2] && (GDMA_CTRL_get_osd2_prog_done(rtd_inl(GDMA_CTRL_reg)) == 0)) {
				gdma->pendingRPC[GDMA_PLANE_OSD2] = 0;
				up(&gdma->updateSem[GDMA_PLANE_OSD2]);
			}
			if (gSemaphore[GDMA_PLANE_OSD3] > 0 && gdma->pendingRPC[GDMA_PLANE_OSD3] && (GDMA_CTRL_get_osd3_prog_done(rtd_inl(GDMA_CTRL_reg)) == 0)) {
				gdma->pendingRPC[GDMA_PLANE_OSD3] = 0;
				up(&gdma->updateSem[GDMA_PLANE_OSD3]);
			}

			rtd_outl(GDMA_OSD_INTST_reg, GDMA_OSD_INTST_write_data(0) | GDMA_OSD_INTST_osd1_vsync(1));
#if (defined(CONFIG_ARCH_RTK288O) || defined(CONFIG_ARCH_RTK6748)) && defined(THRESHOLD_DETECT)
			for(i=0;i<ALPHA_THRESHOLD_NUM;i++)
			{
				alpha_osd_detect_cnt[i] = rtd_inl((unsigned int)(OSDOVL_alpha_osd_detect_0_ro_reg+(i*4)));
			}
#endif
			#if ENABLE_VSYNC_NOTIFY
			{
				unsigned long vsync_lock_flags;
				spin_lock_irqsave(&vsync_completion_lock, vsync_lock_flags);

				complete( &gdma_vsync_completion );

				spin_unlock_irqrestore(&vsync_completion_lock, vsync_lock_flags);
			}
			#endif//ENABLE_VSYNC_NOTIFY
		}
#endif
		else {
			if (osd1_vend) {
#ifndef USING_GDMA_VSYNC
				rtd_outl(GDMA_OSD_INTST_reg, GDMA_OSD_INTST_write_data(0) | GDMA_OSD_INTST_osd1_vact_end(1));
#else
				rtd_outl(GDMA_OSD_INTST_reg, GDMA_OSD_INTST_write_data(0) | GDMA_OSD_INTST_osd1_vsync(1));
#endif
				gdma->ctrl.forceUpdate = 1;
			}
		}
	}
#ifdef USING_GDMA_VSYNC
	else {
		pr_debug(KERN_EMERG"OSD1 not prog_done\n");
	}
#endif

	if (osd_vend) {
		/* if (gdma->vsync_enable) */
		{
			//queue_work(psOsdSyncWorkQueue, &sOsdSyncWork);
		}
		if (gdma->GDMA_CallBack)
			gdma->GDMA_CallBack (dev_id, osd1_vend_3D);
#if IS_ENABLED(CONFIG_DRM_REALTEK)
		if(gdma->GDMA_IrqCallBack)
			gdma->GDMA_IrqCallBack(gdma->IrqCallBackData);
#endif
	}


	if (OSD_SR_OSD_SR_1_UDFLOW_get_osd_sr_in_udflow(rtd_inl(OSD_SR_OSD_SR_1_UDFLOW_reg))) {
		osd1_acc_under = 1;
		g_sr_udfl_num[OSD_SR_1]++;
#if IS_ENABLED(CONFIG_RTK_KDRIVER_SUPPORT)
		pr_debug(KERN_EMERG"GDMA: osd1_sr underflow, source GDMA_osd1, busH=%d Mhz, dclk=%d Mhz, num=%d\n",
			pll_info_bush(), pll_info_disp(), g_sr_udfl_num[OSD_SR_1]);
#else
		pr_debug(KERN_EMERG"GDMA: osd1_sr underflow, source GDMA_osd1, busH=%d Mhz, dclk=%d Mhz, num=%d\n",
			GDMA_BusH_DClk_info(0), GDMA_BusH_DClk_info(1), g_sr_udfl_num[OSD_SR_1]);
#endif
		rtd_outl(OSD_SR_OSD_SR_1_UDFLOW_reg, OSD_SR_OSD_SR_1_UDFLOW_osd_sr_in_udflow(1));
	}

	if (OSD_SR_OSD_SR_2_UDFLOW_get_osd_sr_in_udflow(rtd_inl(OSD_SR_OSD_SR_2_UDFLOW_reg))) {
		osd1_acc_under = 1;
		g_sr_udfl_num[OSD_SR_2]++;
#if IS_ENABLED(CONFIG_RTK_KDRIVER_SUPPORT)
		pr_debug(KERN_EMERG"GDMA: osd2_sr underflow, source GDMA_osd2, busH=%d Mhz, dclk=%d Mhz, num=%d\n",
			pll_info_bush(), pll_info_disp(), g_sr_udfl_num[OSD_SR_2]);
#else
		pr_debug(KERN_EMERG"GDMA: osd2_sr underflow, source GDMA_osd2, busH=%d Mhz, dclk=%d Mhz, num=%d\n",
			GDMA_BusH_DClk_info(0), GDMA_BusH_DClk_info(1), g_sr_udfl_num[OSD_SR_2]);
#endif

		rtd_outl(OSD_SR_OSD_SR_2_UDFLOW_reg, OSD_SR_OSD_SR_2_UDFLOW_osd_sr_in_udflow(1));
	}

	dma_intst_reg.regValue = rtd_inl(GDMA_DMA_INTST_reg);
	if (dma_intst_reg.osd1_acc_sync) {
		osd1_acc_under = 1;
		g_osd_acc_sync_num[GDMA_PLANE_OSD1]++;
		if (g_osd_acc_sync_print_num < PRINT_LIMIT_ACC){
			g_osd_acc_sync_print_num++;
			//if(Get_DISPLAY_PANEL_OLED_TYPE() == FALSE)
			printk(KERN_ERR"[GDMA][DMA.ERR] OSD-%d accident sync [%d]  num = %d!!\n", GDMA_PLANE_OSD1,g_osd_acc_sync_print_num,g_osd_acc_sync_num[GDMA_PLANE_OSD1]);
		}
		pr_debug(KERN_EMERG"GDMA: OSD1 accident sync! num = %d\n", g_osd_acc_sync_num[GDMA_PLANE_OSD1]);
	}else{
		g_osd_acc_sync_print_num = 0;
	}

	if (dma_intst_reg.osd1_acc_sync || dma_intst_reg.tfbc1_acc_sync) {
					//if(Get_DISPLAY_PANEL_OLED_TYPE() == FALSE)
	
		unsigned int memcdvscnt = rtd_inl(0xB80282EC);
		rtd_pr_gdma_err("%s %d(%d)Hz\n", __func__, 270000000/memcdvscnt, memcdvscnt);
		rtd_pr_gdma_err("%s 0xb8028618=%x\n", __func__, rtd_inl(0xb8028618));
		rtd_pr_gdma_err("%s 0xb802861C=%x\n", __func__, rtd_inl(0xb802861C));
		rtd_pr_gdma_err("%s 0xb8028620=%x\n", __func__, rtd_inl(0xb8028620));
		rtd_pr_gdma_err("%s 0xb8028624=%x\n", __func__, rtd_inl(0xb8028624));
		rtd_pr_gdma_err("%s 0xb8028644=%x\n", __func__, rtd_inl(0xb8028644));
		rtd_pr_gdma_err("%s 0xB80282EC=%x\n", __func__, rtd_inl(0xb80282EC));
		rtd_pr_gdma_err("%s 0xb8028608=%x\n", __func__, rtd_inl(0xb8028608));
		rtd_pr_gdma_err("%s 0xb8028508=%x\n", __func__, rtd_inl(0xb8028508));
		rtd_pr_gdma_err("%s 0xb802800C=%x\n", __func__, rtd_inl(0xb802800C));

		rtd_pr_gdma_err("%s 0xb8028600=%x\n", __func__, rtd_inl(0xb8028600));
		rtd_pr_gdma_err("%s 0xb8028604=%x\n", __func__, rtd_inl(0xb8028604));

		rtd_pr_gdma_err("%s vtotal 0xb8028014=%x\n", __func__, rtd_inl(0xb8028014));
		rtd_pr_gdma_err("%s vtotal 0xb8028504=%x\n", __func__, rtd_inl(0xb8028504));

		rtd_pr_gdma_err("%s shift_h_enable=%d\n", __func__, g_osdshift_upscale_ctrl.shift_h_enable);
		rtd_pr_gdma_err("%s shift_v_enable=%x\n", __func__, g_osdshift_upscale_ctrl.shift_v_enable);

		rtd_pr_gdma_err("%s SYS_REG_SYS_DISPCLKSEL_reg=%x\n", __func__, rtd_inl(SYS_REG_SYS_DISPCLKSEL_reg));

		rtd_pr_gdma_err("%s GDMA_OSD1_CTRL_reg=%x\n", __func__, rtd_inl(GDMA_OSD1_CTRL_reg));
		rtd_pr_gdma_err("%s GDMA_OSD1_reg=%x\n", __func__, rtd_inl(GDMA_OSD1_reg));
		rtd_pr_gdma_err("%s GDMA_OSD1_START_reg=%x\n", __func__, rtd_inl(GDMA_OSD1_START_reg));
		rtd_pr_gdma_err("%s GDMA_OSD1_WI_reg=%x\n", __func__, rtd_inl(GDMA_OSD1_WI_reg));
		rtd_pr_gdma_err("%s GDMA_OSD1_SIZE_reg=%x\n", __func__, rtd_inl(GDMA_OSD1_SIZE_reg));

		rtd_pr_gdma_err("%s GDMA_TFBC1_CODE_ID0_reg=%x\n", __func__, rtd_inl(GDMA_TFBC1_CODE_ID0_reg));
		rtd_pr_gdma_err("%s GDMA_TFBC1_CODE_ID1_reg=%x\n", __func__, rtd_inl(GDMA_TFBC1_CODE_ID1_reg));
		rtd_pr_gdma_err("%s GDMA_TFBC1_CODE_ID2_reg=%x\n", __func__, rtd_inl(GDMA_TFBC1_CODE_ID2_reg));
		rtd_pr_gdma_err("%s GDMA_TFBC1_CODE_IP_reg=%x\n", __func__, rtd_inl(GDMA_TFBC1_CODE_IP_reg));
		rtd_pr_gdma_err("%s GDMA_TFBC1_DBG_HEAD0_reg=%x\n", __func__, rtd_inl(GDMA_TFBC1_DBG_HEAD0_reg));
		rtd_pr_gdma_err("%s GDMA_TFBC1_DBG_HEAD1_reg=%x\n", __func__, rtd_inl(GDMA_TFBC1_DBG_HEAD1_reg));
		rtd_pr_gdma_err("%s GDMA_TFBC1_DBG_HEAD2_reg=%x\n", __func__, rtd_inl(GDMA_TFBC1_DBG_HEAD2_reg));
		rtd_pr_gdma_err("%s GDMA_TFBC1_DBG_HEAD3_reg=%x\n", __func__, rtd_inl(GDMA_TFBC1_DBG_HEAD3_reg));

		rtd_pr_gdma_err("%s GDMA_DEBUG IONFO [%x][%x] [%x] [%x] [%x] [%x] [%x] [%x] [%x] [%x]\n", __func__, rtd_inl(
		GDMA_DBG_WI_INFO_reg),rtd_inl(GDMA_DBG_OSD_WI_0_reg),rtd_inl(GDMA_DBG_OSD_WI_1_reg),rtd_inl(GDMA_DBG_OSD_WI_2_reg),
		rtd_inl(GDMA_DBG_OSD_WI_3_reg),rtd_inl(GDMA_DBG_OSD_WI_4_reg),rtd_inl(GDMA_DBG_OSD_WI_5_reg),rtd_inl(	
		GDMA_DBG_OSD_WI_6_reg),rtd_inl(GDMA_DBG_OSD_WI_7_reg),rtd_inl(GDMA_DBG_OSD_WI_8_reg));
		
		
				
	}

	if (dma_intst_reg.osd2_acc_sync) {
		osd1_acc_under = 1;
		g_osd_acc_sync_num[GDMA_PLANE_OSD2]++;
		pr_debug(KERN_EMERG"GDMA: OSD2 accident sync! num = %d\n", g_osd_acc_sync_num[GDMA_PLANE_OSD2]);
	}
	if (dma_intst_reg.osd3_acc_sync) {
		osd1_acc_under = 1;
		g_osd_acc_sync_num[GDMA_PLANE_OSD3]++;
		pr_debug(KERN_EMERG"GDMA: OSD3 accident sync! num = %d\n", g_osd_acc_sync_num[GDMA_PLANE_OSD3]);
	}
	if (dma_intst_reg.osd1_udfl) {
		osd1_acc_under = 1;
		g_osd_udfl_num[GDMA_PLANE_OSD1]++;
		if (g_osd_acc_sync_print_num < PRINT_LIMIT_ACC){
			g_osd_acc_sync_print_num++;
			//if(Get_DISPLAY_PANEL_OLED_TYPE() == FALSE)
			printk(KERN_ERR"GDMA: OSD1 under flow! num = %d\n", g_osd_udfl_num[GDMA_PLANE_OSD1]);
		}
		pr_debug(KERN_EMERG"GDMA: OSD1 under flow! num = %d\n", g_osd_udfl_num[GDMA_PLANE_OSD1]);
	}
	if (dma_intst_reg.osd2_udfl) {
		osd1_acc_under = 1;

		g_osd_udfl_num[GDMA_PLANE_OSD2]++;
		pr_debug(KERN_EMERG"GDMA: OSD2 under flow! num = %d\n", g_osd_udfl_num[GDMA_PLANE_OSD2]);
	}
	if (dma_intst_reg.osd3_udfl) {
		osd1_acc_under = 1;
		g_osd_udfl_num[GDMA_PLANE_OSD3]++;
		pr_debug(KERN_EMERG"GDMA: OSD3 under flow! num = %d\n", g_osd_udfl_num[GDMA_PLANE_OSD3]);
	}
	
	dma_intst_reg.write_data = 0;
	rtd_outl(GDMA_DMA_INTST_reg, dma_intst_reg.regValue);
	/*  conflict condition check */
	/*if (rtd_inl(GDMA_blend_setting_conflict_reg))*/
	/*	Warning("GDMA: Conflict condition happened, [0x%x] %x", GDMA_blend_setting_conflict_reg, rtd_inl(GDMA_blend_setting_conflict_reg));*/

#if 0   /*  frame-rate calculation by Vsync */
	static unsigned int gdmaINT_cnt;
	static unsigned int spentTime, sta;
	if (gdmaINT_cnt == 0 && sta == 0) {
		sta = rtd_inl(SCPU_CLK90K_LO_reg);
		/* Warning("[GDMA] Starting counting for FPS, sta=%d \n", sta); */
	} else {
		if (rtd_inl(SCPU_CLK90K_LO_reg) > sta) {
			spentTime = (rtd_inl(SCPU_CLK90K_LO_reg) - sta);
			gdmaINT_cnt++;
		} else if (gdmaINT_cnt < 1000) {
			if (gdmaINT_cnt) {
				Warning(KERN_EMERG"[GDMA]FPS=%d, spend time %d, income time %d \n", (gdmaINT_cnt / (spentTime/90000)), spentTime, gdmaINT_cnt);
			}
		}

		if (gdmaINT_cnt >= 1000) {
			Warning(KERN_EMERG"[GDMA]FPS=%d, spend time %d, income time %d \n", (gdmaINT_cnt / (spentTime/90000)), spentTime, gdmaINT_cnt);
			sta = spentTime = gdmaINT_cnt = 0;
		}

		/* Warning("[GDMA] Counting , gdmaINT_cnt=%d \n", gdmaINT_cnt); */
	}
#endif
	if (!(osd1_vend || osd1_vend_3D || osd1_finish || osd1_sync || osd1_acc_under)) {
		return IRQ_NONE;
	}

	return IRQ_HANDLED;
}

void GDMA_ISR_export(void)
{
	return;
}

static int gdma_remove(struct platform_device *pdev)
{
	int i;
	int irq = 0;
	gdma_dev *gdma = NULL;

	dev_t devno = MKDEV(gdma_major, gdma_minor);

	DBG_PRINT(KERN_EMERG "gdma clean module gdma_major = %d\n", gdma_major);

#if IS_ENABLED(CONFIG_RTK_KDRV_DC_MEMORY_TRASH_DETCTER)
	DCMT_DEBUG_INFO_UNREGISTER(osd_mdscpu, osd_debug_info);
#endif

	GDMA_uninit_debug_proc();

#if OSD_DETECTION_SUPPORT
	del_timer_sync(&GDMA_timer);
#endif

	irq = platform_get_irq(pdev, 0);

	if (irq >= 0) {
		for (i = 0; i < gdma_nr_devs; i++) {
			gdma = &gdma_devices[i];
			free_irq(gdma->irq_no, &gdma_devices[i]);
		}
	}

	if (gdma_devices) {
		for (i = 0; i < gdma_nr_devs; i++) {
			cdev_del      (&gdma_devices[i].cdev);
			device_destroy(gdma_class, MKDEV(gdma_major, i));
		}
		kfree(gdma_devices);
	}

	class_destroy(gdma_class);

	platform_driver_unregister(&gdma_driver);
	/* cleanup_module is never called if registering failed */
	unregister_chrdev_region(devno, gdma_nr_devs);

	rtd_printk(KERN_INFO,TAG_NAME,"%s finish.\n", __FUNCTION__);

	return 0;
}

static int __init GDMA_init_module(void)
{
#ifdef CONFIG_RTK_KDRV_GDMA_MODULE
    rtk_parse_osd_split_mode();
#endif
	return platform_driver_register(&gdma_driver);
}

static void __exit GDMA_exit_module(void)
{
	platform_driver_unregister(&gdma_driver);
}

static int gdma_probe(struct platform_device* pdev)
{
	int result, i, j, k;
	gdma_dev *gdma = NULL;
    gdma_dma_RBUS  gdma_dma_reg;

#ifdef CONFIG_PM
#if 0
#ifdef CONFIG_SMP
	struct cpumask *cpu_mask;
#endif
#endif
	osdovl_mixer_ctrl2_RBUS mixer_ctrl2;
	osdovl_mixer_layer_sel_RBUS mixer_layer_sel_reg;
	ppoverlay_display_timing_ctrl2_RBUS display_timing_ctrl2;
	sys_reg_sys_dispclksel_RBUS sys_dispclksel_reg;
	dev_t dev = 0;
	ppoverlay_memcdtg_dh_den_start_end_RBUS dtg_dh;
	ppoverlay_memcdtg_dv_den_start_end_RBUS dtg_dv;
#if defined(CONFIG_ARCH_RTK287O) || defined(CONFIG_ARCH_RTK288O) || defined(CONFIG_ARCH_RTK2875) || defined(CONFIG_ARCH_RTK2885M) || defined(CONFIG_ARCH_RTK6748)
	unsigned int onlineProgDone = GDMA_CTRL_write_data(1);
#else
	unsigned int onlineProgDone = GDMA_CTRL_write_data(1) | GDMA_CTRL_osd_others_prog_done(1);
#endif
#if IS_ENABLED(CONFIG_REALTEK_PCBMGR)
	PANEL_CONFIG_PARAMETER *panel_parameter = (PANEL_CONFIG_PARAMETER *)&platform_info.panel_parameter;
#endif
	rtd_printk(KERN_INFO, TAG_NAME, "%s \n", __FUNCTION__);

#if IS_ENABLED(CONFIG_REALTEK_PCBMGR)
	pr_debug("[GDMA]panel_parameter->iCONFIG_PANEL_TYPE=%d  ******\n\n", panel_parameter->iCONFIG_PANEL_TYPE);
	pr_debug("[GDMA]panel_parameter=%s  ******\n\n", panel_parameter->sPanelName);
#endif

#if defined(CONFIG_RTK_KDRV_GDMA_MODULE)
	if(ptr_GDMA_Check_Dvr_Reclaim == 0)
		ptr_GDMA_Check_Dvr_Reclaim = GDMA_Check_Dvr_Reclaim;
#endif


	display_timing_ctrl2.regValue = rtd_inl(PPOVERLAY_Display_Timing_CTRL2_reg);
	dtg_dh.regValue = rtd_inl(PPOVERLAY_memcdtg_DH_DEN_Start_End_reg);
	dtg_dv.regValue = rtd_inl(PPOVERLAY_memcdtg_DV_DEN_Start_End_reg);

	/* clock check */
	sys_dispclksel_reg.regValue = rtd_inl(SYS_REG_SYS_DISPCLKSEL_reg);
	//if (sys_dispclksel_reg.clken_disp_gdma == 0)
	GDMA_Clk_Select(0, true);
	gdma_dma_reg.regValue = rtd_inl(GDMA_DMA_reg);
	gdma_dma_reg.tfbc1_req_num = 0xF;
	gdma_dma_reg.tfbc2_req_num = 0xF;
	
	
	 rtd_outl(GDMA_DMA_reg,gdma_dma_reg.regValue);

	if (gdma_major) {
		dev = MKDEV(gdma_major, gdma_minor);
		result = register_chrdev_region(dev, gdma_nr_devs, "gdma");
	} else {
		result = alloc_chrdev_region(&dev, gdma_minor, gdma_nr_devs, "gdma");
		gdma_major = MAJOR(dev);
	}

	if (result < 0) {
		DBG_PRINT(KERN_EMERG"gdma: can't get major %d\n", gdma_major);
		return result;
	}

	gdma_class = class_create(THIS_MODULE, "gdma");
	if (IS_ERR((gdma_class)))
		return PTR_ERR(gdma_class);

	gdma_class->devnode = GDMA_devnode;

	gdma_devices = kmalloc(gdma_nr_devs * sizeof(gdma_dev), GFP_KERNEL);
	if (!(gdma_devices)) {
		result = -ENOMEM;
		gdma_remove(pdev);   /* fail */
		return result;
	}

#if defined(CONFIG_ARM64)
	gdma_dev_addr = (unsigned long)virt_to_phys(gdma_devices);

#else
	gdma_dev_addr = (unsigned int)virt_to_phys(gdma_devices);
#endif

	memset(gdma_devices, 0, gdma_nr_devs * sizeof(gdma_dev));

	gdma = &gdma_devices[0];

	sema_init(&gdma->sem, 1);
	sema_init(&gdma->sem_receive, 1);
	sema_init(&gdma->sem_gfxreceive, 1);
	sema_init(&gdma->sem_get_fence, 1);

	for (j = 0; j < GDMA_PLANE_MAXNUM; j++)
		sema_init(&gdma->updateSem[j], 0);

	/*for OSD shift*/
	memset(&g_osdshift_ctrl, 0x00, sizeof(CONFIG_OSDSHIFT_STRUCT));
	/*MAIN is OSD1*/
	g_osdshift_ctrl.plane = VO_GRAPHIC_OSD1;

	GDMA_setup_cdev(gdma, 0);
#ifdef CONFIG_PM
	gdma->dev = &(pdev->dev);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 0, 0)) && defined(CONFIG_ARM64)
	arch_setup_dma_ops(gdma->dev, 0, 0, NULL, false);
#endif
	gdma->dev->dma_mask = &gdma->dev->coherent_dma_mask;
	if(dma_set_mask(gdma->dev, DMA_BIT_MASK(32))) {
		pr_debug(KERN_EMERG"[GDMA] DMA not supported\n");
	}
#endif
#if IS_ENABLED(CONFIG_REALTEK_PCBMGR)
	gdma->iCONFIG_PANEL_TYPE = panel_parameter->iCONFIG_PANEL_TYPE;
	gdma->iCONFIG_PANEL_CUSTOM_INDEX = panel_parameter->iCONFIG_PANEL_CUSTOM_INDEX;
	pr_debug("[GDMA]gdma->iCONFIG_PANEL_TYPE=%d\n", gdma->iCONFIG_PANEL_TYPE);
	pr_debug("[GDMA]gdma->iCONFIG_PANEL_CUSTOM_INDEX=%d\n", gdma->iCONFIG_PANEL_CUSTOM_INDEX);
	/*  3d support? PR or SG mode? */
	pr_debug("[GDMA]gdma->i3D_DISPLAY_SUPPORT=%d\n", panel_parameter->i3D_DISPLAY_SUPPORT);
	pr_debug("[GDMA]gdma->i3D_LINE_ALTERNATIVE_SUPPORT=%d\n", panel_parameter->i3D_LINE_ALTERNATIVE_SUPPORT);
	if (panel_parameter->i3D_DISPLAY_SUPPORT)
		gdma->ctrl.enable3D_PR_SGMode = panel_parameter->i3D_LINE_ALTERNATIVE_SUPPORT;
#endif
	gdma->f_inputsrc_4k2k = -1;/* FALSE; */
	/*modify for vsync between hwc and gdma*/
	gdma->vsync_enable = 0;
	gdma->f_palette_update = FALSE;
	sema_init(&gdma->vsync_sem, 0);
	gdma->f_3d_ui = FALSE;

	for (k = 0; k < GDMA_PLANE_MAXNUM; k++) {
		gdma->osdPlane[k].OSD_canvas.srcWin.x  =
		gdma->osdPlane[k].OSD_canvas.srcWin.y  = 0;
		gdma->osdPlane[k].OSD_canvas.srcWin.width  = 1280;
		gdma->osdPlane[k].OSD_canvas.srcWin.height = 720;

		gdma->osdPlane[k].OSD_canvas.dispWin.width = dtg_dh.memcdtg_dh_den_end-dtg_dh.memcdtg_dh_den_sta;
		gdma->osdPlane[k].OSD_canvas.dispWin.height = dtg_dv.memcdtg_dv_den_end-dtg_dv.memcdtg_dv_den_sta;

		gdma->osdPlane[k].OSD_ctrl.hwTurnOff = 0;

		for (j = 0; j < GDMA_MAX_NUM_OSD_WIN; j++) {
			gdma->osdPlane[k].OSDwin[j].ctrl.id     = j;
			gdma->osdPlane[k].OSDwin[j].ctrl.previd = 0;
			gdma->osdPlane[k].OSDwin[j].ctrl.nextid = 0;
		}
	}

	/* Yvonne_Sirius+ */
	for (k = 0; k < GDMA_PLANE_MAXNUM; k++) {
		gdma->dispWin[k].width = dtg_dh.memcdtg_dh_den_end-dtg_dh.memcdtg_dh_den_sta;
		gdma->dispWin[k].height = dtg_dv.memcdtg_dv_den_end-dtg_dv.memcdtg_dv_den_sta;
	}
	/* Yvonne_Sirius- */

	gdma->GDMA_osd.wi_endian = 0;/* Yvonne todo */
	/* gdma->GDMA_osd.dvb = 0; */
	gdma->GDMA_osd.osd = 1;
	gdma->GDMA_osd.write_data = 1;
	gdma->GDMA_osd_ctrl.write_data = 1;
/*
	rtd_printk(KERN_INFO,TAG_NAME,"[GDMA]W[end-sta]=[%d-%d]\n", dtg_dh.mh_den_end, dtg_dh.mh_den_sta);
	rtd_printk(KERN_INFO,TAG_NAME,"[GDMA]H[end-sta]=[%d-%d]\n", dtg_dv.mv_den_end, dtg_dv.mv_den_sta);
*/

	rtd_outl(GDMA_OSD1_reg, *((u32 *)&gdma->GDMA_osd));
	rtd_outl(GDMA_OSD2_reg, *((u32 *)&gdma->GDMA_osd));
	rtd_outl(GDMA_OSD3_reg, *((u32 *)&gdma->GDMA_osd));

	gdma_config_line_buffer();

	/* OSD programming done */
	onlineProgDone |= GDMA_CTRL_osd1_prog_done(1);
	onlineProgDone |= GDMA_CTRL_osd2_prog_done(1);
	onlineProgDone |= GDMA_CTRL_osd3_prog_done(1);
	rtd_outl(GDMA_CTRL_reg, onlineProgDone);

	/* onlineProgDone reset to default */
#if defined(CONFIG_ARCH_RTK287O) || defined(CONFIG_ARCH_RTK288O) || defined(CONFIG_ARCH_RTK2875) || defined(CONFIG_ARCH_RTK2885M) || defined(CONFIG_ARCH_RTK6748)
	onlineProgDone = GDMA_CTRL_write_data(1);
#else
	onlineProgDone = GDMA_CTRL_write_data(1) | GDMA_CTRL_osd_others_prog_done(1);
#endif
#if 0
	/* wait prog done */
	while ((rtd_inl(GDMA_CTRL_reg)&prog_done_mask) != 0) {
		gdma_usleep(1000);
		to_cnt++;
		if (to_cnt > 40) {
			pr_debug("%s, GDMA HW something wrong. Please check \n", __FUNCTION__);
			break;
		}
	}
	pr_debug("%s, 1st passed \n", __FUNCTION__);
#endif

	/*
	*	initialization flow
	*	1. line buffer setting
	*	2. OSD_SR switch
	*	3. go_middle_blend could be set together
	*	4. then write all online path programming done
	*/
	gdma_config_fbc();

	mixer_ctrl2.regValue = rtd_inl(OSDOVL_Mixer_CTRL2_reg);
#if OSD_DETECTION_SUPPORT
	mixer_ctrl2.measure_osd_zone_en = 1;
	mixer_ctrl2.measure_osd_zone_type = 0; //A=0, R=0, G=0, B=0
#endif
	mixer_ctrl2.video_src_sel = 1;
	mixer_ctrl2.mixero1_en    = 1;
#if defined(CONFIG_ARCH_RTK288O) || defined(CONFIG_ARCH_RTK2885M) || defined(CONFIG_ARCH_RTK6748)
	mixer_ctrl2.mixero2_en    = 1;
#else
	mixer_ctrl2.mixero2_en = 0;
#endif
	mixer_ctrl2.mixero3_en = 1;
	mixer_ctrl2.mixer_en = 1;
	rtd_outl(OSDOVL_Mixer_CTRL2_reg, mixer_ctrl2.regValue);

	/* wdma_bl set to 'd80 by HW RD suggestion */
	/*dma_bl_2_reg.regValue = rtd_inl(GDMA_DMA_BL_2_reg);
	dma_bl_2_reg.wdma_bl = 80;
	rtd_outl(GDMA_DMA_BL_2_reg, dma_bl_2_reg.regValue);
	*/

	/* mixer layer sel: video keep at c5 place */
	mixer_layer_sel_reg.regValue = rtd_inl(OSDOVL_Mixer_layer_sel_reg);
	mixer_layer_sel_reg.c0_sel = GDMA_PLANE_OSD1;
	mixer_layer_sel_reg.c1_sel = GDMA_PLANE_OSD2;
	mixer_layer_sel_reg.c2_sel = GDMA_PLANE_OSD3;
	mixer_layer_sel_reg.c3_sel = 0;
#if (defined(CONFIG_ARCH_RTK288O) || defined(CONFIG_ARCH_RTK2875) || defined(CONFIG_ARCH_RTK2885M)) || defined(CONFIG_ARCH_RTK6748) && defined(THRESHOLD_DETECT)

	rtd_outl(OSDOVL_alpha_osd_detect_0_reg,
			 OSDOVL_alpha_osd_detect_0_thd0(0x01) |
			 OSDOVL_alpha_osd_detect_0_thd1(0x40) |
			 OSDOVL_alpha_osd_detect_0_thd2(0x60) |
			 OSDOVL_alpha_osd_detect_0_thd3(0x80)
			);
	rtd_outl(OSDOVL_alpha_osd_detect_1_reg,
			 OSDOVL_alpha_osd_detect_1_thd4(0xA0) |
			 OSDOVL_alpha_osd_detect_1_thd5(0xC0) |
			 OSDOVL_alpha_osd_detect_1_thd6(0xE0) |
			 OSDOVL_alpha_osd_detect_1_thd7(0xFF)
			);
	mixer_layer_sel_reg.a_det_osd1_en = 1;
	memset(alpha_osd_detect_cnt,0x0,sizeof(alpha_osd_detect_cnt));

	osd_gain_info.on_off = 0;
	osd_gain_info.levelval = 0xff;
#endif

	rtd_outl(OSDOVL_Mixer_layer_sel_reg, mixer_layer_sel_reg.regValue);

	gdma_config_mid_blend();

	onlineProgDone |= GDMA_CTRL_osd1_prog_done(1);
	onlineProgDone |= GDMA_CTRL_osd2_prog_done(1);
	onlineProgDone |= GDMA_CTRL_osd3_prog_done(1);

	/* OSD programming done */
	rtd_outl(GDMA_CTRL_reg, onlineProgDone);

	rtd_printk(KERN_INFO,TAG_NAME,"%s, %d, MIXER = 0x%x\n", __FUNCTION__, __LINE__, rtd_inl(OSDOVL_Mixer_CTRL2_reg));
	rtd_printk(KERN_INFO,TAG_NAME,"[GDMA] Panel Width=%d, Height=%d \n", gdma->dispWin[0].width, gdma->dispWin[0].height);

	if (irq_of_parse_and_map(pdev->dev.of_node, 0)) {
		gdma->dev = &(pdev->dev);
		gdma->irq_no = irq_of_parse_and_map(gdma->dev->of_node, 0);
		printk(KERN_ERR"[%s]get irq = %d\n", __func__, gdma->irq_no);

		if (!gdma->irq_no) {
			printk(KERN_ERR"[%s] No gdma irq found!\n", __func__);
			return -ENODEV;
		}

		if (devm_request_irq(gdma->dev, gdma->irq_no, GDMA_irq_handler, IRQF_SHARED, DRV_NAME, (void*)gdma)) {
			printk(KERN_ERR"[%s] request irq fail!\n", __func__);
			return -ENODEV;
		}

	} else {
		/*if can't get irq , don't set below settings*/
		return -ENODEV;
	}

	gdma->dev->dma_mask = &gdma->dev->coherent_dma_mask;
	if (dma_set_mask(gdma->dev, DMA_BIT_MASK(32))) {
		pr_debug(KERN_EMERG"[GDMA] DMA not supported\n");
	}
#if 0
#ifdef CONFIG_SMP
	cpu_mask = (struct cpumask *)get_cpu_mask(2);	// ISR move to cpu1
	cpumask_set_cpu(2, cpu_mask);
	if (irq_can_set_affinity(gdma->irq_no))
		irq_set_affinity(gdma->irq_no, cpu_mask);
#endif
#endif
	gdma->plane = GDMA_PLANE_NONE;
	gdma->ctrl.displayEachPic = 1;
	gdma->ctrl.OSD1onTop = 0;

	/*	osd sync work queue init */
	//psOsdSyncWorkQueue = alloc_ordered_workqueue("OsdSync", WQ_FREEZABLE | WQ_MEM_RECLAIM | WQ_HIGHPRI);
	//INIT_WORK(&sOsdSyncWork, osd_sync_hwc);

	memset(&sGdmaReceiveWork[0], 0x0, sizeof(sGdmaReceiveWork));

	//dev = MKDEV(gdma_major, gdma_minor + gdma_nr_devs);

#ifdef USR_WORKQUEUE_UPDATE_BUFFER
	psOsd1RecWorkQueue = alloc_ordered_workqueue("Osd1Rec", WQ_FREEZABLE | WQ_MEM_RECLAIM | WQ_HIGHPRI | __WQ_ORDERED);

	for (i = 0; i < USE_TRIPLE_BUFFER_NUM; i++) {
		INIT_WORK(&sOSD1ReceiveWork[i].GdmaReceiveWork, osd1_send_hw);
		sOSD1ReceiveWork[i].used=0;
	}
		sema_init(&gdma->sem_work, 1);
#endif

#ifdef TRIPLE_BUFFER_SEMAPHORE
	for (i = 0; i < GDMA_PLANE_MAXNUM; i++)
		gSemaphore[i] = gSEMAPHORE_MAX;
#endif

	device_enable_async_suspend(&pdev->dev);
	GDMA_init_debug_proc();

	/* gdma INT rounting to SCPU */
	rtd_outl(SYS_REG_INT_CTRL_SCPU_reg, SYS_REG_INT_CTRL_SCPU_osd_int_scpu_routing_en_mask | SYS_REG_INT_CTRL_SCPU_write_data(1));

	/* enable GDMA interrupt */
	rtd_outl(GDMA_OSD_INTST_reg, ~1); /*  clear status */
#ifndef USING_GDMA_VSYNC
#ifdef TRIPLE_BUFFER_SEMAPHORE
	rtd_outl(GDMA_OSD_INTEN_reg, GDMA_OSD_INTEN_write_data(1) | GDMA_OSD_INTEN_osd1_vact_end(1) | GDMA_OSD_INTEN_osd1_vsync(1));
#else
	rtd_outl(GDMA_OSD_INTEN_reg, GDMA_OSD_INTEN_write_data(1) | GDMA_OSD_INTEN_osd1_vact_end(1));
#endif
#else
	rtd_outl(GDMA_OSD_INTEN_reg, GDMA_OSD_INTEN_write_data(1) | GDMA_OSD_INTEN_osd1_vsync(1));
#endif

#endif

#if OSD_DETECTION_SUPPORT
	g_jiffies_counter = 16; // 16ms
	memset(&gDetectParameter, 0x0, sizeof(GDMA_OSD_DETECTION_PARAMETER_T));
	GDMA_timer_init();
#endif

#if IS_ENABLED(CONFIG_RTK_KDRV_DC_MEMORY_TRASH_DETCTER)
	DCMT_DEBUG_INFO_REGISTER(osd_mdscpu, osd_debug_info);
#endif
	rtd_printk(KERN_INFO,TAG_NAME,"%s finish.\n", __FUNCTION__);

	gdma_total_ddr_size = get_memory_size(GFP_DCU1) + get_memory_size(GFP_DCU2) - 0x100;

	printk(KERN_EMERG "[GDMA]gg get total memsize = %08x\n", (unsigned int)gdma_total_ddr_size);

	if (gdma_total_ddr_size) {
		rtd_outl(GDMA_TFBC1_MIN_reg, 0x0);
		rtd_outl(GDMA_TFBC1_MAX_reg, gdma_total_ddr_size);
	} else {
		printk(KERN_ERR"%s gdma_total_ddr_size is empty!\n", __func__);
	}
	
	g_esticker_info.pre_data = g_esticker_info.cur_data = -1;
	g_esticker_info.cur_position = 150;
	g_esticker_info.change = 0;

#if USE_RLINK_INTERFACE
	GDMA_rlink_interface_init();
#endif
	gdma_fence_array = kzalloc(sizeof(struct gdma_fence)*GDMA_FENCE_ARRAY_SIZE, GFP_KERNEL);
    memset(buf_phy_addr,0x0,sizeof(unsigned int)*BUF_PHY_NUM);

	gGDMA_Monitor_ctl.enable=0;
	rtd_pr_gdma_info("func=%s line=%d OSDOVL_Mixer_CTRL2_reg=%x\n",__FUNCTION__,__LINE__,rtd_inl(OSDOVL_Mixer_CTRL2_reg));

	rtd_printk(KERN_INFO,TAG_NAME,"%s finish.\n", __FUNCTION__);

	return 0;
}

void GDMA_Clear (void)
{
	unsigned int onlineProgDone = GDMA_CTRL_osd1_prog_done(1)|GDMA_CTRL_write_data(1);

	rtd_outl(GDMA_OSD1_CTRL_reg, ~1);
	rtd_outl(GDMA_OSD1_CTRL_reg, 1);

	rtd_outl(GDMA_OSD1_WI_reg, 0);
	rtd_outl(GDMA_CTRL_reg, onlineProgDone);
}
EXPORT_SYMBOL(GDMA_Clear);

#ifdef CONFIG_PM
#if defined(CONFIG_CUSTOMER_TV006)
static int gdma_suspend (struct device *p_dev) {
#else
int gdma_suspend_by_vbe(void);
static int gdma_suspend(struct device *p_dev) {
	if(p_dev == NULL)
		gdma_suspend_by_vbe();
	else
		printk(KERN_NOTICE"[GDMA]skip func=%s\n", __FUNCTION__);
	return 0;

}
EXPORT_SYMBOL(gdma_suspend_by_vbe);

int gdma_suspend_by_vbe(void) {
#endif
	int i, size;
	gdma_dev *gdma = NULL;

	rtd_printk(KERN_EMERG,TAG_NAME,"%s \n", __FUNCTION__);

	/* disable GDMA interrupt */
	/* gdma INT rounting to SCPU */
	rtd_outl(SYS_REG_INT_CTRL_SCPU_reg, SYS_REG_INT_CTRL_SCPU_osd_int_scpu_routing_en_mask | SYS_REG_INT_CTRL_SCPU_write_data(0));

	/* rtd_outl(0xb801a644, rtd_inl(0xb801a644) | 0x00002000); // disable SCPU to receive OSD/SUB Interrupt */
	rtd_outl(GDMA_OSD_INTST_reg, ~1); /*  clear status */

#if USE_RLINK_INTERFACE
	GDMA_rlink_interface_call(2); // --> close session
	rlink_interface_open=0;
	if (change_OSD_mode==GDMA_SPLIT_MODE_0)
       g_esticker_info.cur_position = 150;
    else
       g_esticker_info.cur_position = 300;
    g_esticker_info.pre_data = g_esticker_info.cur_data = -1;
    g_esticker_info.change = 0;
#endif

#ifndef USING_GDMA_VSYNC
#ifdef TRIPLE_BUFFER_SEMAPHORE
	rtd_outl(GDMA_OSD_INTEN_reg, GDMA_OSD_INTEN_write_data(0) | GDMA_OSD_INTEN_osd1_vact_end(1) | GDMA_OSD_INTEN_osd1_vsync(1));
#else
	rtd_outl(GDMA_OSD_INTEN_reg, GDMA_OSD_INTEN_write_data(0) | GDMA_OSD_INTEN_osd1_vact_end(1));
#endif
#else
	rtd_outl(GDMA_OSD_INTEN_reg, GDMA_OSD_INTEN_write_data(0) | GDMA_OSD_INTEN_osd1_vsync(1));
#endif

	for (i = 0; i < gdma_nr_devs; i++) {
		gdma = &gdma_devices[i];
		free_irq(gdma->irq_no, &gdma_devices[i]);
	}
	GDMA_Clear();
	size = (sizeof(GDMA_PM_REGISTER_DATA)/4)/2;
	for (i = 0; i < size; i++) {
		GDMA_PM_REGISTER_DATA[i][1] = rtd_inl(GDMA_PM_REGISTER_DATA[i][0]);
		DBG_PRINT("func=%s line=%d GDMA_PM_REGISTER_DATA[i][0]=%x\n", __FUNCTION__, __LINE__, GDMA_PM_REGISTER_DATA[i][0]);
		DBG_PRINT("func=%s line=%d GDMA_PM_REGISTER_DATA[i][1]=%x\n", __FUNCTION__, __LINE__, GDMA_PM_REGISTER_DATA[i][1]);
	}

	osd_ctrl_data[GDMA_PLANE_OSD1] = rtd_inl(GDMA_OSD1_CTRL_reg);
	/*osd_ctrl_data[GDMA_PLANE_OSD2] = rtd_inl(GDMA_OSD2_CTRL_reg);*/
	osd_ctrl_data[GDMA_PLANE_OSD3] = rtd_inl(GDMA_OSD3_CTRL_reg);

	DBG_PRINT("func=%s GDMA_OSD1_CTRL_reg=0x%x\n", __FUNCTION__, osd_ctrl_data[GDMA_PLANE_OSD1]);
	DBG_PRINT("func=%s GDMA_OSD2_CTRL_reg=0x%x\n", __FUNCTION__, osd_ctrl_data[GDMA_PLANE_OSD2]);
	DBG_PRINT("func=%s GDMA_OSD3_CTRL_reg=0x%x\n", __FUNCTION__, osd_ctrl_data[GDMA_PLANE_OSD3]);

	GDMA_ConfigOSDxEnableFast(GDMA_PLANE_OSD1,false);
	gdma_suspend_disable_osd = 1;
	GDMA_Wait_OSD1_Prog_Done();

	//printk(KERN_EMERG"func=%s line=%d\n", __FUNCTION__,__LINE__);

	return 0;
}

#if defined(CONFIG_CUSTOMER_TV006)
static int gdma_resume (struct device *p_dev) {
#else
int gdma_resume_by_vbe (void);
static int gdma_resume (struct device *p_dev) {
	if(p_dev == NULL)
		gdma_resume_by_vbe();
	else
		printk(KERN_NOTICE"[GDMA]skip func=%s\n", __FUNCTION__);
	return 0;
}
EXPORT_SYMBOL(gdma_resume_by_vbe);

int gdma_resume_by_vbe (void) {
#endif
	int i, j, size;
	gdma_dev *gdma = NULL;
	osdovl_mixer_ctrl2_RBUS mixer_ctrl2;
    gdma_dma_RBUS  gdma_dma_reg;



#if IS_ENABLED(CONFIG_REALTEK_PCBMGR)
	PANEL_CONFIG_PARAMETER *panel_parameter = NULL;
#endif
#if defined(CONFIG_ARCH_RTK287O) || defined(CONFIG_ARCH_RTK288O) || defined(CONFIG_ARCH_RTK2875) || defined(CONFIG_ARCH_RTK2885M) || defined(CONFIG_ARCH_RTK6748)
	unsigned int onlineProgDone = GDMA_CTRL_write_data(1);
#else
	unsigned int onlineProgDone = GDMA_CTRL_write_data(1) | GDMA_CTRL_osd_others_prog_done(1);
#endif

	rtd_printk(KERN_EMERG,TAG_NAME,"%s \n", __FUNCTION__);
#if defined(CONFIG_CUSTOMER_TV006) && defined(CONFIG_SUPPORT_SCALER)
	vbe_disp_early_resume();/*patch for force enable d-domain clk https://jira.realtek.com/browse/K3LG-347, wait LG webos call, so we enable first*/
#endif

#if IS_ENABLED(CONFIG_REALTEK_PCBMGR)
	panel_parameter = (PANEL_CONFIG_PARAMETER *)&platform_info.panel_parameter;
	DBG_PRINT("[GDMA]panel_parameter->iCONFIG_PANEL_TYPE=%d  *********************\n\n\n\n", panel_parameter->iCONFIG_PANEL_TYPE);
	DBG_PRINT("[GDMA]panel_parameter=%s  *********************\n\n\n\n", panel_parameter->sPanelName);
#endif

	g_StartCheckFrameCnt = 100;
	/* clock select */
	GDMA_Clk_Select(0, true);
    gdma_dma_reg.regValue = rtd_inl(GDMA_DMA_reg);
	gdma_dma_reg.tfbc1_req_num = 0xF;
	gdma_dma_reg.tfbc2_req_num = 0xF;
	
	rtd_outl(GDMA_DMA_reg,gdma_dma_reg.regValue);

	size = (sizeof(GDMA_PM_REGISTER_DATA)/4)/2;
	for (i = 0; i < size; i++) {

		if (GDMA_OSD1_reg == GDMA_PM_REGISTER_DATA[i][0]){
			rtd_outl(GDMA_PM_REGISTER_DATA[i][0], GDMA_OSD1_write_data(1) | GDMA_PM_REGISTER_DATA[i][1]);
		}else if (GDMA_OSD2_reg == GDMA_PM_REGISTER_DATA[i][0]){
			rtd_outl(GDMA_PM_REGISTER_DATA[i][0], GDMA_OSD2_write_data(1) | GDMA_PM_REGISTER_DATA[i][1]);
		}else if (GDMA_OSD3_reg == GDMA_PM_REGISTER_DATA[i][0]){
			rtd_outl(GDMA_PM_REGISTER_DATA[i][0], GDMA_OSD3_write_data(1) | GDMA_PM_REGISTER_DATA[i][1]);
		}else{
			rtd_outl(GDMA_PM_REGISTER_DATA[i][0], GDMA_PM_REGISTER_DATA[i][1]);
		}
	}
	
	if( !(rtd_inl(OSDOVL_Mixer_CTRL2_reg) &OSDOVL_Mixer_CTRL2_mixer_en_mask ))
	{
		GDMA_ConfigOSDEnable(true);
	}
	for (i = 0; i < gdma_nr_devs; i++) {
		gdma = &gdma_devices[i];

		if (request_irq(gdma->irq_no, GDMA_irq_handler, IRQF_SHARED, "gdma", gdma)) {
			DBG_PRINT(KERN_EMERG"gdma: can't get assigned irq%i\n", GDMA_IRQ);
		}
	}



	for (i = 0; i < gdma_nr_devs; i++) {
		gdma = &gdma_devices[i];

		rtd_outl(GDMA_OSD1_reg, *((u32 *)&gdma->GDMA_osd));
		rtd_outl(GDMA_OSD2_reg, *((u32 *)&gdma->GDMA_osd));
		rtd_outl(GDMA_OSD3_reg, *((u32 *)&gdma->GDMA_osd));

		/* OSD_SR switch */

		/* line buffer setting */
#if 0
		online_reg.regValue = rtd_inl(GDMA_ONLINE_reg);
		if (online_reg.set_osd1_offline == 0)
			onlineProgDone |= GDMA_CTRL_osd1_prog_done(1);
		if (online_reg.set_osd2_offline == 0)
			onlineProgDone |= GDMA_CTRL_osd2_prog_done(1);
		if (online_reg.set_osd3_offline == 0)
			onlineProgDone |= GDMA_CTRL_osd3_prog_done(1);
		if (online_reg.set_osd4_offline == 0)
			onlineProgDone |= GDMA_CTRL_osd4_prog_done(1);
		if (online_reg.set_osd5_offline == 0)
			onlineProgDone |= GDMA_CTRL_osd5_prog_done(1);
		/* OSD programming done */
		rtd_outl(GDMA_CTRL_reg, onlineProgDone);
		pr_debug("%s, 1st onlineProgDone=0x%x \n", __FUNCTION__, onlineProgDone);

		/* onlineProgDone reset to default */
#if defined(CONFIG_ARCH_RTK287O) || defined(CONFIG_ARCH_RTK288O)
		onlineProgDone = GDMA_CTRL_write_data(1);
#else
		onlineProgDone = GDMA_CTRL_write_data(1) | GDMA_CTRL_osd_others_prog_done(1);
#endif

		/* wait prog done */
		while ((rtd_inl(GDMA_CTRL_reg)&prog_done_mask) != 0) {
			gdma_usleep(1000);
			to_cnt++;
			if (to_cnt > 40) {
				pr_debug("%s, GDMA HW something wrong. Please check \n", __FUNCTION__);
				break;
			}
		}
		pr_debug("%s, 1st passed \n", __FUNCTION__);
#endif

		/*
		*	initialization flow
		*	1. line buffer setting
		*	2. OSD_SR switch
		*	3. go_middle_blend could be set together
		*	4. then write all online path programming done
		*/
		gdma_config_fbc();

		if(flag_gdma_resume_std == 1){
			mixer_ctrl2.regValue = rtd_inl(OSDOVL_Mixer_CTRL2_reg);
			mixer_ctrl2.mixero2_en    = 1;
			rtd_outl(OSDOVL_Mixer_CTRL2_reg, mixer_ctrl2.regValue);
			flag_gdma_resume_std = 0;
		}

		gdma_config_mid_blend();

		/* restore GDMA register setting */
		rtd_outl(GDMA_OSD1_CTRL_reg, ~1);
		rtd_outl(GDMA_OSD2_CTRL_reg, ~1);
		rtd_outl(GDMA_OSD3_CTRL_reg, ~1);
#ifndef CONFIG_CUSTOMER_TV006
		rtd_outl(GDMA_OSD1_CTRL_reg, GDMA_OSD1_CTRL_write_data(1) | osd_ctrl_data[GDMA_PLANE_OSD1]);
#endif
		//rtd_outl(GDMA_OSD2_CTRL_reg, GDMA_OSD2_CTRL_write_data(1) | osd_ctrl_data[GDMA_PLANE_OSD2]);
		//rtd_outl(GDMA_OSD3_CTRL_reg, GDMA_OSD3_CTRL_write_data(1) | osd_ctrl_data[GDMA_PLANE_OSD3]);

		onlineProgDone |= GDMA_CTRL_osd1_prog_done(1);
		onlineProgDone |= GDMA_CTRL_osd2_prog_done(1);
		onlineProgDone |= GDMA_CTRL_osd3_prog_done(1);
		/* OSD programming done */
#if defined(CONFIG_ARM64)
		rtd_outl(GDMA_CTRL_reg, onlineProgDone);
#endif
		DBG_PRINT("%s, 2nd onlineProgDone=0x%x \n", __FUNCTION__, onlineProgDone);

#if 0	// dont wait
		/* wait prog done */
		to_cnt = 0;
		while ((rtd_inl(GDMA_CTRL_reg)&prog_done_mask) != 0) {
			gdma_usleep(1000);
			to_cnt++;
			if (to_cnt > 40) {
				pr_debug("%s, GDMA HW something wrong. Please check \n", __FUNCTION__);
				break;
			}
		}
		pr_debug("%s, 2nd passed \n", __FUNCTION__);
#endif

		for (i = 0; i < GDMA_PLANE_MAXNUM; i++) {
			gdma->ctrl.osdfirstEn[i] = 0;
		}

		/* gdma INT rounting to SCPU */
		rtd_outl(SYS_REG_INT_CTRL_SCPU_reg, SYS_REG_INT_CTRL_SCPU_osd_int_scpu_routing_en_mask | SYS_REG_INT_CTRL_SCPU_write_data(1));

		rtd_outl(GDMA_OSD_INTST_reg, ~1); /*  clear status */

#ifndef USING_GDMA_VSYNC
#ifdef TRIPLE_BUFFER_SEMAPHORE
		rtd_outl(GDMA_OSD_INTEN_reg, GDMA_OSD_INTEN_write_data(1) | GDMA_OSD_INTEN_osd1_vact_end(1) | GDMA_OSD_INTEN_osd1_vsync(1));
#else
		rtd_outl(GDMA_OSD_INTEN_reg, GDMA_OSD_INTEN_write_data(1) | GDMA_OSD_INTEN_osd1_vact_end(1));
#endif
#else
		rtd_outl(GDMA_OSD_INTEN_reg, GDMA_OSD_INTEN_write_data(1) | GDMA_OSD_INTEN_osd1_vsync(1));
#endif



		for (i = 0; i < 4; i++) {
			j = GDMA_coef_2t8p_swap[i << 1] << 16 | GDMA_coef_2t8p_swap[(i << 1) + 1];

			/* rtd_outl((GDMA_OSD1_VSYC_0_reg) + (i<<2), j); */
		}
	}

	/* fox add: tmp solution for Toshiba rotate 180 */
#if IS_ENABLED(CONFIG_REALTEK_PCBMGR)
	if (panel_parameter->iVFLIP) {
		/*  SFG_CTRL_0 */
		rtd_outl(SFG_SFG_CTRL_0_reg, rtd_inl(SFG_SFG_CTRL_0_reg) | SFG_SFG_CTRL_0_h_flip_en(1));
		GDMA_ConfigVFlip(1);
	}
#endif
	return 0;
}

#endif

#ifdef CONFIG_HIBERNATION
static int gdma_suspend_std (struct device *p_dev)
{
#if defined(CONFIG_RTK_KDRV_GAL)
	int i=0;
	unsigned int alloc_size=0;
	gal_std_save_dvr_phy = 0;
#if defined(CONFIG_ARM64)
	gal_std_save_dvr_ptr = (unsigned long)vmalloc(GFX_CURSOR_SIZE);
#else
	gal_std_save_dvr_ptr = (unsigned int)vmalloc(GFX_CURSOR_SIZE);
#endif
	if(gal_std_save_dvr_ptr == INVALID_VAL)
		BUG();

	for (i = 0; i < total_index; i++)
	{
		gfx_data[i].to_phyAddr = 0;
		gfx_data[i].to_virtAddr = gal_std_save_dvr_ptr + alloc_size;
		alloc_size +=  gfx_data[i].size;
#if defined(CONFIG_ARM64)
		gfx_data[i].virtAddr = (unsigned long)dvr_remap_cached_memory(gfx_data[i].phyAddr, PAGE_ALIGN(gfx_data[i].size), __builtin_return_address(0));
#else
		gfx_data[i].virtAddr = (unsigned int)dvr_remap_cached_memory(gfx_data[i].phyAddr, PAGE_ALIGN(gfx_data[i].size), __builtin_return_address(0));
#endif
		memcpy((void*)gfx_data[i].to_virtAddr, (void*)gfx_data[i].virtAddr, gfx_data[i].size);
	}
#endif
	gdma_suspend(p_dev);

	return 0;
}

static int gdma_resume_std_thraw (struct device *p_dev)
{		
		flag_gdma_resume_std = 1;
		
#if defined(CONFIG_RTK_KDRV_GAL)
		for (i = 0; i < total_index; i++) {
			memcpy((void*)gfx_data[i].virtAddr, (void*)gfx_data[i].to_virtAddr, gfx_data[i].size);
			dmac_flush_range((void*)gfx_data[i].virtAddr, (void*)(gfx_data[i].virtAddr + gfx_data[i].size));
			outer_flush_range((phys_addr_t)gfx_data[i].phyAddr, (phys_addr_t)(gfx_data[i].phyAddr + gfx_data[i].size));
			dvr_unmap_memory((void*)gfx_data[i].virtAddr, (size_t)(PAGE_ALIGN(gfx_data[i].size)));
		}
		vfree((const void*)gal_std_save_dvr_ptr);
#endif
		gdma_resume(p_dev);
		return 0;
}

static int gdma_resume_std (struct device *p_dev)
{
	//add by fox for gfx std
	unsigned long memset_ptr, memset_start,memset_start_osd1;
	unsigned int memsize, osd_wi_addr = 0;
	int i = 0, pmRegSize; //, onlineIdx = 0, disPlane, tmp;
	unsigned int *ptr;
#if defined(CONFIG_ARM64)
	unsigned long addr;
	unsigned int osd_w, osd_h;
#else
	unsigned int addr, osd_w, osd_h;
#endif
	gdma_dev *gdma = &gdma_devices[0];

	//GDMA_PIC_DATA *curPic;
	pmRegSize = (sizeof(GDMA_PM_REGISTER_DATA)/4)/2;

	flag_gdma_resume_std = 1;

#ifdef CONFIG_HIBERNATION
		pr_debug("%s, Before GAL buffer clear!!! \n", __FUNCTION__);

		if (osd_ctrl_data[GDMA_PLANE_OSD1] & GDMA_OSD1_CTRL_osd1_en_mask) {

			for (i = 0; i < pmRegSize; i++) {
				if (GDMA_OSD1_WI_reg == GDMA_PM_REGISTER_DATA[i][0]) {
					osd_wi_addr = GDMA_PM_REGISTER_DATA[i][1];
					break;
				}
			}
			//pr_debug("%s, osd_wi_addr=0x%x \n", __FUNCTION__, osd_wi_addr);
#if defined(CONFIG_ARM64)
			addr = (unsigned long )phys_to_virt(osd_wi_addr);
#else
			addr = (unsigned int )phys_to_virt(osd_wi_addr);
#endif
			ptr = (unsigned int *)addr;
			ptr += 2;		// osd size
			osd_w = *ptr & 0xFFFF;
			osd_h = (*ptr & 0xFFFF0000)>>16;
			//pr_debug("%s, osd_w=0x%x, osd_h=0x%x \n", __FUNCTION__, osd_w, osd_h);

			memsize = osd_w * osd_h * 4;
			ptr += 4;		// physical address
			memset_start_osd1 = memset_start = memset_ptr = *ptr;

			//pr_debug("%s, memset_start=0x%x, memset_ptr=0x%x, memsize=0x%x \n", __FUNCTION__, memset_start, memset_ptr, memsize);
#if defined(CONFIG_RTK_KDRV_GAL)
			if (memset_ptr != 0)
			{
				unsigned long memset_ptr_virt = (unsigned long)dvr_remap_cached_memory(memset_ptr, PAGE_ALIGN(memsize), __builtin_return_address(0));
				memset((void*)memset_ptr_virt, 0, memsize);
				dmac_flush_range((void*)memset_ptr_virt, (void*)(memset_ptr_virt + memsize));
				memset((void*)memset_ptr_virt, 0, memsize);
				outer_flush_range((phys_addr_t)memset_ptr, (phys_addr_t)(memset_ptr + memsize));
				dvr_unmap_memory((void*)memset_ptr_virt, memsize);
            }
#endif
			{
			volatile GDMA_PIC_DATA *curPic;
			unsigned int clear_index;
			if(gdma->curPic[GDMA_PLANE_OSD1] == 0)
				clear_index = GDMA_MAX_PIC_Q_SIZE-1;
			else
				clear_index = gdma->curPic[GDMA_PLANE_OSD1]-1;
			curPic = gdma->pic[GDMA_PLANE_OSD1] + clear_index;
			memset_start = memset_ptr = curPic->OSDwin.top_addr;
			memsize = curPic->OSDwin.winWH.height * curPic->OSDwin.pitch;

			//pr_debug("%s,clear_index =%d  memset_start=0x%x, memset_ptr=0x%x, memsize=0x%x \n", __FUNCTION__, clear_index,memset_start, memset_ptr, memsize);

			if((memset_start_osd1 != memset_start) && (memset_start != 0))
			{
#if defined(CONFIG_RTK_KDRV_GAL)
				unsigned long memset_ptr_virt = (unsigned long)dvr_remap_cached_memory(memset_ptr, PAGE_ALIGN(memsize), __builtin_return_address(0));
				memset((void*)memset_ptr_virt, 0, memsize);
				dmac_flush_range((void*)memset_ptr_virt, (void*)(memset_ptr_virt + memsize));
				outer_flush_range((phys_addr_t)memset_ptr, (phys_addr_t)(memset_ptr + memsize));
				dvr_unmap_memory((void*)memset_ptr_virt, memsize);
#endif
			}

		}

		pr_debug("%s, GAL buffer clear!!! \n", __FUNCTION__);
		// check picture queue
		//pr_debug("%s, cur=%d, qwr=%d \n", __FUNCTION__, gdma->curPic[GDMA_PLANE_OSD1], gdma->picQwr[GDMA_PLANE_OSD1]);
	}
#endif

#if defined(CONFIG_RTK_KDRV_GAL)
	for (i = 0; i < total_index; i++) {
		memcpy((void*)gfx_data[i].virtAddr, (void*)gfx_data[i].to_virtAddr, gfx_data[i].size);
		dmac_flush_range((void*)gfx_data[i].virtAddr, (void*)(gfx_data[i].virtAddr + gfx_data[i].size));
		outer_flush_range((phys_addr_t)gfx_data[i].phyAddr, (phys_addr_t)(gfx_data[i].phyAddr + gfx_data[i].size));
		dvr_unmap_memory((void*)gfx_data[i].virtAddr, (size_t)(PAGE_ALIGN(gfx_data[i].size)));
	}
	vfree((const void*)gal_std_save_dvr_ptr);
#endif
	gdma_resume(p_dev);
	return 0;
}
#endif

/*============= API for 3D Graphic =============*/

int GDMA_ReleaseBuffers(gdma_dev *gdma, int Qstart, int Qend, int disPlane)
{
	int i = Qstart;
	DBG_PRINT(KERN_EMERG"%s: disPlane=%d, Qstart=%d, Qend=%d gdma->picCount=%d \n", __FUNCTION__, disPlane, Qstart, Qend, gdma->picCount[disPlane]);

	do {
		gdma->picCount[disPlane]--;
	} while (i++ != Qend);

	return GDMA_SUCCESS;
}

/** @brief Flush GDMA & OSD_COMP queue work, but leave the latest picture and force to show
 *  @param None
 *  @return None
 */
void GDMA_QueueFlush(void)
{
	int disPlane = 0;
	gdma_dev *gdma = &gdma_devices[0];
	GDMA_PIC_DATA *curPic;

	for (disPlane = 0; disPlane < GDMA_PLANE_MAXNUM; disPlane++) {
		/*  check if the remainder picture in queue */
		if (gdma->curPic[disPlane] != gdma->picQwr[disPlane]) {
			gdma->picCount[disPlane] = 0;
			gdma->curPic[disPlane] = gdma->picQrd[disPlane] = gdma->picQwr[disPlane];
			pr_debug(KERN_EMERG "%s disPlane=%d : r %d,w %d c %d \n", __FUNCTION__, disPlane, gdma->picQrd[disPlane], gdma->picQwr[disPlane], gdma->curPic[disPlane]);

			/*  waiting for the latest picture is ready (work done) */
			curPic = gdma->pic[disPlane] + gdma->picQwr[disPlane];
			while (curPic->workqueueDone == 0)
				;
		}
	}

	GDMA_ENTER_CRITICAL();
	GDMA_Update(gdma, 0);

	for (disPlane = 0; disPlane < GDMA_PLANE_MAXNUM; disPlane++)
		gdma->ctrl.sync[disPlane] = 0;

	GDMA_EXIT_CRITICAL();
}

int GDMA_Flush (gdma_dev *gdma)
{
	int disPlane = 0;
	for (disPlane = 0; disPlane < GDMA_PLANE_MAXNUM; disPlane++) {
		if (gdma->picQrd[disPlane] != gdma->picQwr[disPlane]) {
			/*GDMA_ReleaseBuffers (gdma, gdma->picQrd[disPlane] + 1, gdma->picQwr[disPlane], disPlane);*/
			gdma->curPic[disPlane] = gdma->picQrd[disPlane] = gdma->picQwr[disPlane];
			gdma->ctrl.sync[disPlane] = 0;
			pr_debug(KERN_EMERG"%s : disPlane=%d r %d,w %d c %d \n", __FUNCTION__, disPlane, gdma->picQrd[disPlane], gdma->picQwr[disPlane], gdma->curPic[disPlane]);
		}
	}
	return GDMA_SUCCESS;
}

int GDMA_Hide (gdma_dev *gdma)
{
	if (gdma->plane == GDMA_PLANE_OSD1 && !gdma->osdPlane[GDMA_PLANE_OSD1].OSD_ctrl.en) {
		if (gdma->ctrl.OSD1onTop) {
			rtd_outl(GDMA_OSD2_CTRL_reg, ~1);
			rtd_outl(GDMA_CTRL_reg,  GDMA_CTRL_write_data(1) | GDMA_CTRL_osd2_prog_done(1));
			gdma->ctrl.osdEn[GDMA_PLANE_OSD2] = 0;
		} else {
			rtd_outl(GDMA_OSD1_CTRL_reg, ~1);
			rtd_outl(GDMA_CTRL_reg,  GDMA_CTRL_write_data(1) | GDMA_CTRL_osd1_prog_done(1));
			gdma->ctrl.osdEn[GDMA_PLANE_OSD1] = 0;
		}
	} else if (gdma->plane == GDMA_PLANE_OSD2 && !gdma->osdPlane[GDMA_PLANE_OSD2].OSD_ctrl.en) {
		if (gdma->ctrl.OSD1onTop) {
			rtd_outl(GDMA_OSD1_CTRL_reg, ~1);
			rtd_outl(GDMA_CTRL_reg,  GDMA_CTRL_write_data(1) | GDMA_CTRL_osd1_prog_done(1));
			gdma->ctrl.osdEn[GDMA_PLANE_OSD1] = 0;
		} else {
			rtd_outl(GDMA_OSD2_CTRL_reg, ~1);
			rtd_outl(GDMA_CTRL_reg, GDMA_CTRL_write_data(1) | GDMA_CTRL_osd2_prog_done(1));
			gdma->ctrl.osdEn[GDMA_PLANE_OSD2] = 0;
		}
	} else if (gdma->plane == GDMA_PLANE_OSD3 && !gdma->osdPlane[GDMA_PLANE_OSD3].OSD_ctrl.en) {
		rtd_outl(GDMA_OSD3_CTRL_reg, ~1);
		rtd_outl(GDMA_CTRL_reg, GDMA_CTRL_write_data(1) | GDMA_CTRL_osd3_prog_done(1));
		gdma->ctrl.osdEn[GDMA_PLANE_OSD3] = 0;
	}
	return GDMA_SUCCESS;
}


/** @brief Set OSD HW registers according to parameter disPlane which indicate OSD number
 *  @param disPlane HW OSD number (GDMA_DISPLAY_PLANE includes GDMA_PLANE_OSD1 & GDMA_PLANE_OSD2)
 *  @return None
 */
static void GDMA_OSDReg_Set(GDMA_DISPLAY_PLANE disPlane)
{
	gdma_dev *gdma = &gdma_devices[0];
	GDMA_PIC_DATA *curPic;
	unsigned int offset = 0;
	osdovl_mixer_layer_sel_RBUS mixer_layer_sel_reg;
#if defined(CONFIG_ARCH_RTK287O) || defined(CONFIG_ARCH_RTK288O) || defined(CONFIG_ARCH_RTK2875) || defined(CONFIG_ARCH_RTK2885M) || defined(CONFIG_ARCH_RTK6748)
#else
	gdma_resource_lb_RBUS resource_lb_reg;
#endif
	GDMA_REG_CONTENT *pReg = NULL;
	unsigned int cx_sel_old = 0, cx_sel_new = 0;

	curPic = gdma->pic[disPlane] + gdma->curPic[disPlane];
	pReg = &curPic->reg_content;

	DBG_PRINT(KERN_EMERG"%s line=%d disPlane=%d, workqueueIdx=%d \n", __FUNCTION__, __LINE__, disPlane, curPic->workqueueIdx);
	DBG_PRINT(KERN_EMERG"%s gdma->curPic[%d]=%d, curPic->onlineOrder=%d\n", __FUNCTION__, disPlane, gdma->curPic[disPlane], curPic->onlineOrder);


	if (disPlane == GDMA_PLANE_OSD2)
		offset = 0x100;
	else if (disPlane == GDMA_PLANE_OSD3)
		offset = 0x200;

#if defined(CONFIG_ARCH_RTK287O) || defined(CONFIG_ARCH_RTK288O) || defined(CONFIG_ARCH_RTK2875) || defined(CONFIG_ARCH_RTK2885M) || defined(CONFIG_ARCH_RTK6748)
#else
	/* do NOT go to mid-blend in default */
	resource_lb_reg.regValue = rtd_inl(GDMA_resource_lb_reg);
	resource_lb_reg.set_osd1_go_middle_blend = 0;
	resource_lb_reg.set_osd2_go_middle_blend = 0;
	resource_lb_reg.set_osd3_go_middle_blend = 0;
	rtd_outl(GDMA_resource_lb_reg, resource_lb_reg.regValue);
#endif

	/*  set clear region */
	rtd_outl(GDMA_OSD1_CLEAR1_reg+offset, pReg->osd_clear1_reg.regValue);
	rtd_outl(GDMA_OSD1_CLEAR2_reg+offset, pReg->osd_clear2_reg.regValue);
	rtd_outl(GDMA_OSD1_reg+offset, pReg->osd_reg.regValue);

	rtd_outl(GDMA_OSD1_CTRL_reg+offset, ~1);
	rtd_outl(GDMA_OSD1_CTRL_reg+offset, pReg->osd_ctrl_reg.regValue);

	rtd_outl(GDMA_OSD1_WI_reg+offset, pReg->osd_wi_reg.regValue);
	rtd_outl(GDMA_OSD1_WI_3D_reg+offset, pReg->osd_wi_3d_reg.regValue);
	rtd_outl(GDMA_OSD1_SIZE_reg+offset, pReg->osd_size_reg.regValue);

	DBG_PRINT(KERN_EMERG"%s GDMA_OSD%d_CLEAR1_reg=0x%x \n", __FUNCTION__, disPlane, pReg->osd_clear1_reg.regValue);
	DBG_PRINT(KERN_EMERG"%s GDMA_OSD%d_CLEAR2_reg=0x%x \n", __FUNCTION__, disPlane, pReg->osd_clear2_reg.regValue);
	DBG_PRINT(KERN_EMERG"%s GDMA_OSD%d_reg=0x%x \n", __FUNCTION__, disPlane, pReg->osd_reg.regValue);
	DBG_PRINT(KERN_EMERG"%s GDMA_OSD%d_CTRL_reg=0x%x \n", __FUNCTION__, disPlane, pReg->osd_ctrl_reg.regValue);
	DBG_PRINT(KERN_EMERG"%s GDMA_OSD%d_WI_reg=0x%x \n", __FUNCTION__, disPlane, pReg->osd_wi_reg.regValue);
	DBG_PRINT(KERN_EMERG"%s GDMA_OSD%d_WI_3D_reg=0x%x \n", __FUNCTION__, disPlane, pReg->osd_wi_3d_reg.regValue);
	DBG_PRINT(KERN_EMERG"%s GDMA_OSD%d_SIZE_reg=0x%x \n", __FUNCTION__, disPlane, pReg->osd_size_reg.regValue);
	DBG_PRINT(KERN_EMERG"%s GDMA_line_buffer_end_reg=0x%x \n", __FUNCTION__, pReg->lb_end_reg.regValue);

	/*  TODO:: call osd_sr  */
	drv_scaleup(1, disPlane);



	/*  TODO: Mixer : online blend order & plane alpha
	*	because k blend-factor setting, mixer layer order will reverse
	*/
	mixer_layer_sel_reg.regValue = rtd_inl(OSDOVL_Mixer_layer_sel_reg);
	if (curPic->onlineOrder == C0) {
		cx_sel_old = mixer_layer_sel_reg.c0_sel;
		cx_sel_new = mixer_layer_sel_reg.c0_sel = pReg->mixer_layer_sel_reg.c0_sel;

		if (pReg->mixer_layer_sel_reg.c0_plane_alpha_en != mixer_layer_sel_reg.c0_plane_alpha_en)
			mixer_layer_sel_reg.c0_plane_alpha_en = pReg->mixer_layer_sel_reg.c0_plane_alpha_en;

		rtd_outl(OSDOVL_Mixer_c0_plane_alpha1_reg, pReg->mixer_c0_plane_alpha1_reg.regValue);
		rtd_outl(OSDOVL_Mixer_c0_plane_alpha2_reg, pReg->mixer_c0_plane_alpha2_reg.regValue);
	} else if (curPic->onlineOrder == C1) {
		cx_sel_old = mixer_layer_sel_reg.c1_sel;
		cx_sel_new = mixer_layer_sel_reg.c1_sel = pReg->mixer_layer_sel_reg.c1_sel;

		if (pReg->mixer_layer_sel_reg.c1_plane_alpha_en != mixer_layer_sel_reg.c1_plane_alpha_en)
			mixer_layer_sel_reg.c1_plane_alpha_en = pReg->mixer_layer_sel_reg.c1_plane_alpha_en;

		rtd_outl(OSDOVL_Mixer_c1_plane_alpha1_reg, pReg->mixer_c1_plane_alpha1_reg.regValue);
		rtd_outl(OSDOVL_Mixer_c1_plane_alpha2_reg, pReg->mixer_c1_plane_alpha2_reg.regValue);
	} else if (curPic->onlineOrder == C2) {
		cx_sel_old = mixer_layer_sel_reg.c2_sel;
		cx_sel_new = mixer_layer_sel_reg.c2_sel = pReg->mixer_layer_sel_reg.c2_sel;

		if (pReg->mixer_layer_sel_reg.c2_plane_alpha_en != mixer_layer_sel_reg.c2_plane_alpha_en)
			mixer_layer_sel_reg.c2_plane_alpha_en = pReg->mixer_layer_sel_reg.c2_plane_alpha_en;

		rtd_outl(OSDOVL_Mixer_c2_plane_alpha1_reg, pReg->mixer_c2_plane_alpha1_reg.regValue);
		rtd_outl(OSDOVL_Mixer_c2_plane_alpha2_reg, pReg->mixer_c2_plane_alpha2_reg.regValue);
	} else if (curPic->onlineOrder == C3) {
		cx_sel_old = mixer_layer_sel_reg.c3_sel;
		cx_sel_new = mixer_layer_sel_reg.c3_sel = pReg->mixer_layer_sel_reg.c3_sel;

		if (pReg->mixer_layer_sel_reg.c3_plane_alpha_en != mixer_layer_sel_reg.c3_plane_alpha_en)
			mixer_layer_sel_reg.c3_plane_alpha_en = pReg->mixer_layer_sel_reg.c3_plane_alpha_en;

		rtd_outl(OSDOVL_Mixer_c3_plane_alpha1_reg, pReg->mixer_c3_plane_alpha1_reg.regValue);
		rtd_outl(OSDOVL_Mixer_c3_plane_alpha2_reg, pReg->mixer_c3_plane_alpha2_reg.regValue);
	}


	/* conflict protect */
	if (mixer_layer_sel_reg.c0_sel == cx_sel_new && curPic->onlineOrder != C0)
		mixer_layer_sel_reg.c0_sel = cx_sel_old;
	else if (mixer_layer_sel_reg.c1_sel == cx_sel_new && curPic->onlineOrder != C1)
		mixer_layer_sel_reg.c1_sel = cx_sel_old;
	else if (mixer_layer_sel_reg.c2_sel == cx_sel_new && curPic->onlineOrder != C2)
		mixer_layer_sel_reg.c2_sel = cx_sel_old;
	else if (mixer_layer_sel_reg.c3_sel == cx_sel_new && curPic->onlineOrder != C3)
		mixer_layer_sel_reg.c3_sel = cx_sel_old;

	if(enable_osd1osd3mixerorder)
	{
		mixer_layer_sel_reg.c0_sel = GDMA_PLANE_OSD2;
	
		mixer_layer_sel_reg.c1_sel = GDMA_PLANE_OSD1;
	}

	rtd_outl(OSDOVL_Mixer_layer_sel_reg, mixer_layer_sel_reg.regValue);
	DBG_PRINT(KERN_EMERG"%s, GDMA_line_buffer_end_reg=0x%x, OSDOVL_Mixer_layer_sel_reg=0x%x \n", __FUNCTION__, rtd_inl(GDMA_line_buffer_end_reg), mixer_layer_sel_reg.regValue);


	/*  work was done in workqueue */
	sGdmaReceiveWork[curPic->workqueueIdx].used = 0;

	/*  call register debug.... */
	/* GDMA_RegDebug(); */
}

/** @brief main dequeue function that prepare the register value and store in GDMA_REG_CONTENT
 *  @param disPlane HW OSD number (GDMA_DISPLAY_PLANE includes GDMA_PLANE_OSD1/2/3/4/5)
 *  @return None
 */
static void GDMA_PreOSDReg_Set(struct gdma_receive_work *ptr_work)
{
	VO_RECTANGLE srcWin;
	VO_RECTANGLE dispWin;
	int canvasW, canvasH;
	GDMA_WIN *win = NULL, *win_3d = NULL;
	gdma_dev *gdma = &gdma_devices[0];
	GDMA_PIC_DATA *curPic;
	GDMA_REG_CONTENT *pReg = NULL;
	GDMA_DISPLAY_PLANE disPlane = ptr_work->disPlane;
	int rotateBit = gdma->ctrl.enableVFlip;
	dma_addr_t addr, addr_3d;
#ifdef CONFIG_CUSTOMER_TV006
	GDMA_PLANE *osdPlane = &gdma->osdPlane[GDMA_PLANE_OSD1];
#endif

	memset(&srcWin, 0, sizeof(VO_RECTANGLE));
	memset(&dispWin, 0, sizeof(VO_RECTANGLE));

	curPic = gdma->pic[disPlane] + ptr_work->picQwr;
	pReg = &curPic->reg_content;

	DBG_PRINT(KERN_EMERG"%s, disPlane=%d ptr_work->picQwr=%d \n", __FUNCTION__, disPlane, ptr_work->picQwr);

	win = &curPic->OSDwin;
	win_3d = &curPic->OSDwin3D;

	if (win->used) {

		addr = dma_map_single(gdma->dev, (void *)win, sizeof(GDMA_WIN), DMA_TO_DEVICE);
		addr_3d = dma_map_single(gdma->dev, (void *)win_3d, sizeof(GDMA_WIN), DMA_TO_DEVICE);

		srcWin.x = win->winXY.x;
		srcWin.y = win->winXY.y;

		srcWin.width = win->winWH.width;
		srcWin.height = win->winWH.height;

		dispWin.x = win->dst_x;
		dispWin.y = win->dst_y;

		if (win->dst_width != 0 && win->dst_height != 0) {
			dispWin.width = win->dst_width;
			dispWin.height = win->dst_height;
		} else {
			dispWin.width = gdma->dispWin[disPlane].width;
			dispWin.height = gdma->dispWin[disPlane].height;
		}

#ifdef CONFIG_CUSTOMER_TV006
		/* follow AP GOR (grahpic output resolution) configuration which is maximum size for GDMA output */
		if (dispWin.width > osdPlane->OSD_canvas.dispWin.width)
			dispWin.width = osdPlane->OSD_canvas.dispWin.width;
		if (dispWin.height > osdPlane->OSD_canvas.dispWin.height)
			dispWin.height = osdPlane->OSD_canvas.dispWin.height;
#endif


		/* whether non-compressed or normal or FBDC layer, canvas always is source size
		*  this is different between mac3 and magellan2, mac3's compressed image that canvas is panel size
		*/
		canvasW = srcWin.width;
		canvasH = srcWin.height;



#if 0
		/*  3D PR Mode, output height scaling down 1/2 */
		if (gdma->ctrl.enable3D && gdma->ctrl.enable3D_PR_SGMode)
			vout = vout / 2;
#endif


		/*  set clear region */
		if (curPic->clear_x.value != 0 || curPic->clear_y.value != 0) {
			pReg->osd_clear1_reg.regValue = curPic->clear_x.value;
			pReg->osd_clear2_reg.regValue = curPic->clear_y.value;
			pReg->osd_reg.regValue = GDMA_OSD1_clear_region_en(1) | GDMA_OSD1_write_data(1);
		} else {
			pReg->osd_reg.regValue = GDMA_OSD1_clear_region_en(1) | GDMA_OSD1_write_data(0);
		}

		if (gdma->ctrl.osdfirstEn[disPlane] == 0) {

			pReg->osd_ctrl_reg.regValue = GDMA_OSD1_CTRL_write_data(1) | GDMA_OSD1_CTRL_osd1_en(curPic->show) | GDMA_OSD1_CTRL_rotate(rotateBit) | GDMA_OSD1_CTRL_d3_mode(gdma->ctrl.enable3D);

			gdma->ctrl.osdEn[disPlane] = 1;
			gdma->ctrl.osdfirstEn[disPlane] = 1;
			DBG_PRINT(KERN_EMERG"%s, win=0x%x, phy=0x%x\n", __FUNCTION__, (unsigned int)win, virt_to_phys(win));
		} else {
			/*  if osdEn changes to 0, driver will force curPic->show to zero and disable OSD */
			if (gdma->ctrl.osdEn[disPlane] == 0)
				curPic->show = 0;

			pReg->osd_ctrl_reg.regValue = GDMA_OSD1_CTRL_write_data(1) | GDMA_OSD1_CTRL_osd1_en(curPic->show) | GDMA_OSD1_CTRL_rotate(rotateBit);

			/*  3D SG mode use d3_mode (frame sequence) */
			if (gdma->ctrl.enable3D && (gdma->ctrl.enable3D_PR_SGMode == 0))
				pReg->osd_ctrl_reg.regValue |= GDMA_OSD1_CTRL_d3_mode(gdma->ctrl.enable3D);
		}

		/*
		if (gdma->f_box_mode && gdma->box_dst_height == VIDEO_USERDEFINED)
		   rtd_outl(OSD_XY_Addr, GDMA_OSD3_XY_x(dispWin.x) | GDMA_OSD3_XY_y(dispWin.y));
		else
			rtd_outl(OSD_XY_Addr, 0);
			rtd_outl(OSD_XY_Addr, GDMA_OSD3_XY_x(dispWin.x) | GDMA_OSD3_XY_y(dispWin.y));
		*/

		DBG_PRINT(KERN_EMERG"%s, curPic->show=%d, gdma->ctrl.osdEn[%d]=%d \n", __FUNCTION__, curPic->show, disPlane, gdma->ctrl.osdEn[disPlane]);
		DBG_PRINT(KERN_EMERG"%s, osd%d  win=0x%x, top=0x%x gdma->f_inputsrc_4k2k=%d \n", __FUNCTION__, disPlane, (unsigned int)win, win->top_addr, gdma->f_inputsrc_4k2k);
		DBG_PRINT(KERN_EMERG"%s, input src[x,y,w,h]= [%d,%d,%d,%d] \n", __FUNCTION__, win->winXY.x, win->winXY.y, canvasW, canvasH);
		DBG_PRINT(KERN_EMERG"%s, output[x,y,w,h]= [%d,%d,%d,%d] \n", __FUNCTION__, win->dst_x, win->dst_y, win->dst_width ,win->dst_height);
		DBG_PRINT(KERN_EMERG"%s, plane_alpha_r=0x%x, plane_alpha_a=0x%x \n", __FUNCTION__, curPic->plane_ar.plane_alpha_r, curPic->plane_ar.plane_alpha_a);
		DBG_PRINT(KERN_EMERG"%s, plane_alpha_g=0x%x, plane_alpha_b=0x%x \n", __FUNCTION__, curPic->plane_gb.plane_alpha_g, curPic->plane_gb.plane_alpha_b);
		pReg->osd_wi_reg.regValue = GDMA_OSD1_WI_addr (virt_to_phys(win));
        if (pReg->osd_wi_reg.regValue == 0) {
            #if defined(CONFIG_ARM64)
                printk(KERN_EMERG"[GDMA] addr = 0!!!!!!!! win = 0x%lx gIdxQwr = %d pitch = 0x%x\n", (unsigned long)win, ptr_work->picQwr, win->pitch);
            #else
                printk(KERN_EMERG"[GDMA] addr = 0!!!!!!!! win = 0x%x gIdxQwr = %d pitch = 0x%x\n", (unsigned int)win, ptr_work->picQwr, win->pitch);
            #endif
        } else if (pReg->osd_wi_reg.regValue > gdma_total_ddr_size) {
            #if defined(CONFIG_ARM64)
                printk(KERN_EMERG"[GDMA] addr = 0x%lx, win info addr over range!!\n", (unsigned long)win);
            #else
                printk(KERN_EMERG"[GDMA] addr = 0x%x, win info addr over range!!\n", (unsigned int)win);
            #endif
        }


		if(gGDMA_Monitor_ctl.enable){
			win->winWH.width =  gGDMA_Monitor_ctl.input_w;
			win->winWH.height  = gGDMA_Monitor_ctl.input_h;
		}

		pReg->osd_wi_3d_reg.regValue = GDMA_OSD1_WI_3D_addr (virt_to_phys(win_3d));
		pReg->osd_size_reg.regValue = GDMA_OSD1_SIZE_w(canvasW) | GDMA_OSD1_SIZE_h(canvasH);

		/*  TODO:: call osd_sr */
		drv_PreScale(1, srcWin, dispWin, disPlane, ptr_work);

		/*  TODO: Mixer : online blend order & plane alpha */
		if (curPic->onlineOrder == C0) {
			pReg->mixer_layer_sel_reg.c0_sel = disPlane;

			if ((curPic->plane_ar.plane_alpha_r != 0xff) || (curPic->plane_ar.plane_alpha_a != 0xff))
				pReg->mixer_layer_sel_reg.c0_plane_alpha_en = 1;

			if ((curPic->plane_gb.plane_alpha_g != 0xff) || (curPic->plane_gb.plane_alpha_b != 0xff))
				pReg->mixer_layer_sel_reg.c0_plane_alpha_en = 1;

			pReg->mixer_c0_plane_alpha1_reg.regValue = curPic->plane_ar.value;
			pReg->mixer_c0_plane_alpha2_reg.regValue = curPic->plane_gb.value;
		} else if (curPic->onlineOrder == C1) {
			pReg->mixer_layer_sel_reg.c1_sel = disPlane;

			if ((curPic->plane_ar.plane_alpha_r != 0xff) || (curPic->plane_ar.plane_alpha_a != 0xff))
				pReg->mixer_layer_sel_reg.c1_plane_alpha_en = 1;

			if ((curPic->plane_gb.plane_alpha_g != 0xff) || (curPic->plane_gb.plane_alpha_b != 0xff))
				pReg->mixer_layer_sel_reg.c1_plane_alpha_en = 1;

			pReg->mixer_c1_plane_alpha1_reg.regValue = curPic->plane_ar.value;
			pReg->mixer_c1_plane_alpha2_reg.regValue = curPic->plane_gb.value;
		} else if (curPic->onlineOrder == C2) {
			pReg->mixer_layer_sel_reg.c2_sel = disPlane;

			if ((curPic->plane_ar.plane_alpha_r != 0xff) || (curPic->plane_ar.plane_alpha_a != 0xff))
				pReg->mixer_layer_sel_reg.c2_plane_alpha_en = 1;

			if ((curPic->plane_gb.plane_alpha_g != 0xff) || (curPic->plane_gb.plane_alpha_b != 0xff))
				pReg->mixer_layer_sel_reg.c2_plane_alpha_en = 1;

			pReg->mixer_c2_plane_alpha1_reg.regValue = curPic->plane_ar.value;
			pReg->mixer_c2_plane_alpha2_reg.regValue = curPic->plane_gb.value;
		} else if (curPic->onlineOrder == C3) {
			pReg->mixer_layer_sel_reg.c3_sel = disPlane;

			if ((curPic->plane_ar.plane_alpha_r != 0xff) || (curPic->plane_ar.plane_alpha_a != 0xff))
				pReg->mixer_layer_sel_reg.c3_plane_alpha_en = 1;

			if ((curPic->plane_gb.plane_alpha_g != 0xff) || (curPic->plane_gb.plane_alpha_b != 0xff))
				pReg->mixer_layer_sel_reg.c3_plane_alpha_en = 1;

			pReg->mixer_c3_plane_alpha1_reg.regValue = curPic->plane_ar.value;
			pReg->mixer_c3_plane_alpha2_reg.regValue = curPic->plane_gb.value;
		}
		/*
		else if (curPic->onlineOrder == C5)
			pReg->mixer_layer_sel_reg.c5_sel = disPlane;
		*/

		dma_sync_single_for_device(gdma->dev, addr, sizeof(GDMA_WIN), DMA_TO_DEVICE);
		dma_sync_single_for_device(gdma->dev, addr_3d, sizeof(GDMA_WIN), DMA_TO_DEVICE);
		dma_unmap_single(gdma->dev, addr, sizeof(GDMA_WIN), DMA_TO_DEVICE);
		dma_unmap_single(gdma->dev, addr_3d, sizeof(GDMA_WIN), DMA_TO_DEVICE);

		/*  register value was ready */
		curPic->workqueueDone = 1;

		/*  call register debug.... */
		/* GDMA_RegDebug(); */
	} else
		pr_debug(KERN_EMERG"%s, win->used is NOT one, something wrong \n", __FUNCTION__);

}


/** @brief check the syncstamp of picture in other queue which is match the syncstamp number
 *  @param disPlane where OSD plane has syncstamp picture
 *  @return 0: mismatch, 1: all match
 */
static int GDMA_checkSyncStamp(int disPlane)
{
	volatile int syncPicNum = 0;
	volatile int loop;
	volatile GDMA_PIC_DATA *curPic, *otherPic;
	volatile gdma_dev *gdma = &gdma_devices[0];

	curPic = gdma->pic[disPlane] + gdma->curPic[disPlane];

	DBG_PRINT(KERN_EMERG"%s, param disPlane=%d, \n", __FUNCTION__, disPlane);
	DBG_PRINT(KERN_EMERG"%s, syncstamp=%lld, ", __FUNCTION__, curPic->syncInfo.syncstamp);
	DBG_PRINT(KERN_EMERG"%s, syncInfo.matchNum=%d \n", __FUNCTION__, curPic->syncInfo.matchNum);



	for (loop = 0; loop < curPic->syncInfo.matchNum; loop++) {
		otherPic = gdma->pic[curPic->syncInfo.matchPlane[loop]] + gdma->curPic[curPic->syncInfo.matchPlane[loop]];

		DBG_PRINT(KERN_EMERG"%s, syncInfo.matchPlane=%d, ", __FUNCTION__, curPic->syncInfo.matchPlane[loop]);
		DBG_PRINT(KERN_EMERG"%s, another OSD Num=%d, its syncstamp=%lld \n", __FUNCTION__, curPic->syncInfo.matchPlane[loop], otherPic->syncInfo.syncstamp);

		if (curPic->syncInfo.syncstamp == otherPic->syncInfo.syncstamp &&
			curPic->workqueueDone == 1) {  /*  check workqueueDone */
			syncPicNum++;
			DBG_PRINT(KERN_EMERG"%s, Match Case syncstamp=%lld \n", __FUNCTION__, curPic->syncInfo.syncstamp);
			//DBG_PRINT(KERN_EMERG"another OSD Num=%d, its syncstamp=%lld \n", curPic->syncInfo.matchPlane[loop], otherPic->syncInfo.syncstamp);
		}
		else
			break;
	}

	if (syncPicNum == curPic->syncInfo.matchNum)
		return 1;

	return 0;
}


/** @brief this function is dequeue process of GDMA workqueue
 *         prepare the related register value and store
 *  @param psWork the work was sent by GDMA_ReceivePicture and indicate the display plane number
 *  @return GDMA_SUCCESS
 */
void GDMA_PreUpdate(struct work_struct *psWork)
{
	struct gdma_receive_work *ptr_work;
	ptr_work = (struct gdma_receive_work *) psWork;

	if (ptr_work->used == 1) {
		GDMA_PreOSDReg_Set(ptr_work);
	} else
		pr_debug(KERN_EMERG"%s Unexpected case \n", __FUNCTION__);

}


int GDMA_Update (gdma_dev *gdma, int inISR)
{
	volatile int Qend, syncPicNum = 0, loop = 0;
	volatile GDMA_PIC_DATA *curPic;
	volatile int disPlane, disPlane_tmp, cnt = 0, syncPicFlag, curPicIdx = 0;
	volatile int QueueFlag[GDMA_PLANE_MAXNUM] = {0};	/*  1: indicate this queue has picture to show */
	volatile unsigned int updatedOSDx = 0;				/* actual updated OSD number */
	//volatile static int osdSyncStamp[GDMA_PLANE_MAXNUM] = {0};	/*  keep the setting which picture has syncstamp */
	//volatile static int osdSyncStampTimeout[GDMA_PLANE_MAXNUM] = {0};
	volatile int osdSyncStampNum[GDMA_PLANE_MAXNUM] = {0};  /*  remember syncstamp driver searched before */
	volatile osdovl_osd_db_ctrl_RBUS osd_db_ctrl_reg;
	volatile int onlinePlane[GDMA_PLANE_MAXNUM] = {GDMA_PLANE_OSD1, GDMA_PLANE_OSD2, GDMA_PLANE_OSD3, 0};
	volatile int onlineMaxNum = MAX_OSD_NUM, onlineIdx = 0;
#if defined(CONFIG_ARCH_RTK287O) || defined(CONFIG_ARCH_RTK288O) || defined(CONFIG_ARCH_RTK2875) || defined(CONFIG_ARCH_RTK2885M) || defined(CONFIG_ARCH_RTK6748)
	volatile unsigned int onlineProgDone = GDMA_CTRL_write_data(1);
#else
	volatile unsigned int onlineProgDone = GDMA_CTRL_write_data(1) | GDMA_CTRL_osd_others_prog_done(1);
#endif
	/* fox modify for use cookie to sync GDMA */
	static unsigned int cookie_flag = 0;


	/* search all updated picture of osd plane  */
	for (onlineIdx = 0; onlineIdx < onlineMaxNum; onlineIdx++) {
		disPlane = onlinePlane[onlineIdx];

#ifdef TRIPLE_BUFFER_SEMAPHORE
		if ((gdma->picQwr[disPlane] > gdma->curPic[disPlane])  && (gdma->picQwr[disPlane] - gdma->curPic[disPlane]) > 2)
			DBG_PRINT(KERN_EMERG"GDMA line=%d : r %d,w %d c %d \n", __LINE__, gdma->picQrd[disPlane], gdma->picQwr[disPlane], gdma->curPic[disPlane]);
		else {
			if ((gdma->picQwr[disPlane] < gdma->curPic[disPlane]) && ((gdma->picQwr[disPlane]+GDMA_MAX_PIC_Q_SIZE) >= gdma->curPic[disPlane])  && ((gdma->picQwr[disPlane]+GDMA_MAX_PIC_Q_SIZE) - gdma->curPic[disPlane]) > 2)
				DBG_PRINT(KERN_EMERG"GDMA line=%d : r %d,w %d c %d \n", __LINE__, gdma->picQrd[disPlane], gdma->picQwr[disPlane], gdma->curPic[disPlane]);
		}
#else
		if ((gdma->picQwr[disPlane] > gdma->curPic[disPlane])  && (gdma->picQwr[disPlane] - gdma->curPic[disPlane]) >= 2)
			DBG_PRINT(KERN_EMERG"GDMA line=%d : OSD%d r %d,w %d c %d \n", __LINE__, disPlane, gdma->picQrd[disPlane], gdma->picQwr[disPlane], gdma->curPic[disPlane]);
		else {
			if ((gdma->picQwr[disPlane] < gdma->curPic[disPlane]) && ((gdma->picQwr[disPlane]+GDMA_MAX_PIC_Q_SIZE) >= gdma->curPic[disPlane])  && ((gdma->picQwr[disPlane]+GDMA_MAX_PIC_Q_SIZE) - gdma->curPic[disPlane]) >= 2)
				DBG_PRINT(KERN_EMERG"GDMA line=%d : OSD%d r %d,w %d c %d \n", __LINE__, disPlane, gdma->picQrd[disPlane], gdma->picQwr[disPlane], gdma->curPic[disPlane]);
		}
#endif

#if 0
		Qend = ((gdma->curPic[disPlane] + GDMA_MAX_PIC_Q_SIZE - 2) & (GDMA_MAX_PIC_Q_SIZE - 1));
		//if (gdma->picQrd[disPlane] != gdma->curPic[disPlane] && gdma->picQrd[disPlane] != Qend)
		if (gdma->picCount[disPlane] && gdma->ctrl.sync[disPlane])
		{
			/* GDMA_ReleaseBuffers  (gdma, gdma->picQrd[disPlane] + 1, Qend, disPlane); */
			gdma->picCount[disPlane]--;
			gdma->picQrd[disPlane] = Qend;
		}
#endif

		/* check next picture */
		if (inISR) {
			if (!gdma->ctrl.sync[disPlane]) {

				if ((gdma->picQrd[disPlane] != gdma->picQwr[disPlane]) && (gdma->picQwr[disPlane] >= 1)) {

					curPicIdx = (gdma->picQrd[disPlane] + 1) & (GDMA_MAX_PIC_Q_SIZE - 1);
					curPic = gdma->pic[disPlane] + curPicIdx;
					if (curPic->workqueueDone == 1)    /*  register value was ready */
						gdma->curPic[disPlane] = curPicIdx;
					else
						continue;
					/* gdma->curPic[disPlane] = (gdma->picQrd[disPlane] + 1) & (GDMA_MAX_PIC_Q_SIZE - 1); */
					gdma->ctrl.sync[disPlane] = 1;
					QueueFlag[disPlane] = 1;
					DBG_PRINT(KERN_EMERG"%s, OSD[%d]Sync ......r %d, w %d\n", __FUNCTION__, disPlane, gdma->picQrd[disPlane], gdma->picQwr[disPlane]);

#ifndef TRIPLE_BUFFER_SEMAPHORE
					/* force reset prog done register for next new picture */
					rtd_outl(GDMA_CTRL_reg, rtd_inl(GDMA_CTRL_reg)&~GDMA_CTRL_write_data_mask);
					DBG_PRINT(KERN_EMERG"%s, GDMA_CTRL_reg=0x%x\n", __FUNCTION__, rtd_inl(GDMA_CTRL_reg));
#endif
				} else {
					/* Warning("%s, OSD[%d] has no picture\n", __FUNCTION__, disPlane); */
					continue;
				}
			} else {
				curPic = gdma->pic[disPlane] + gdma->curPic[disPlane];
				curPic->repeatCnt++;
				if (!curPic->status.obsolete) {
					if (curPic->context != -1)
						gContext = curPic->context;    /* set context value */

					curPic->status.obsolete = 1;
				}

				if (gdma->curPic[disPlane] != gdma->picQwr[disPlane]) { /* select next picture to display */
					/*  keep the syncstamp OSD waiting for another OSD which has the same syncstamp */
					if (osdSyncStamp[disPlane] != 1) {
						curPicIdx = gdma->ctrl.displayEachPic ? ((gdma->curPic[disPlane] + 1) & (GDMA_MAX_PIC_Q_SIZE - 1)) : gdma->picQwr[disPlane];
						/* gdma->curPic[disPlane] = gdma->ctrl.displayEachPic ? ((gdma->curPic[disPlane] + 1) & (GDMA_MAX_PIC_Q_SIZE - 1)) : gdma->picQwr[disPlane]; */

						curPic = gdma->pic[disPlane] + curPicIdx;
						if (curPic->workqueueDone == 1) {    /*  register value was ready */
#ifdef TRIPLE_BUFFER_SEMAPHORE
							//if ((rtd_inl(GDMA_CTRL_reg)&prog_done_mask) != 0) {
							//	pr_debug(KERN_EMERG"!_GDMA Not Ready for Next Picture, disPlane=%d, GDMA_CTRL_reg=0x%x \n", disPlane, rtd_inl(GDMA_CTRL_reg));
							//	return GDMA_FAIL;
							//}
#else
							/* force reset prog done register for next new picture */
							rtd_outl(GDMA_CTRL_reg, rtd_inl(GDMA_CTRL_reg)&~GDMA_CTRL_write_data_mask);
							DBG_PRINT(KERN_EMERG"%s, GDMA_CTRL_reg=0x%x\n", __FUNCTION__, rtd_inl(GDMA_CTRL_reg));
#endif
							gdma->curPic[disPlane] = curPicIdx;
						} else
							continue;
					}
					QueueFlag[disPlane] = 1;
					DBG_PRINT(KERN_EMERG"%s, disPlane=%d, next pic c %d, w %d\n", __FUNCTION__, disPlane, gdma->curPic[disPlane], gdma->picQwr[disPlane]);
				} else {
#ifdef GDMA_REPEAT_LOG
					static int prepic;
					if (prepic == gdma->curPic[disPlane]) {
						Warning("%d(%d)\n", gdma->curPic[disPlane], curPic->repeatCnt);
					} else {
						pr_debug("%d(%d)\n", gdma->curPic[disPlane], curPic->repeatCnt);
					}
					prepic = gdma->curPic[disPlane];
#endif

					/*  if osdSyncStamp has picture to sync another picture QueueFlag rises */
					if (osdSyncStamp[disPlane] == 1) {
						QueueFlag[disPlane] = 1;
						DBG_PRINT(KERN_EMERG"%s, check syncstamp osdSyncStamp[%d]=%d \n", __FUNCTION__, disPlane, osdSyncStamp[disPlane]);
					}
				}
			}

			/*  check Queueflag if picture wants to show */
			cnt += QueueFlag[disPlane];
			/*  check syncstamp OSD */
			curPic = gdma->pic[disPlane] + gdma->curPic[disPlane];
			if (curPic->syncInfo.syncstamp != 0 && QueueFlag[disPlane] == 1) {
				osdSyncStamp[disPlane] = 1;
				DBG_PRINT(KERN_EMERG"%s, syncstamp=%lld, ", __FUNCTION__, curPic->syncInfo.syncstamp);
			}
		}
	}


	onlineMaxNum = 0;
	/* only check updated OSD HW */
	if (QueueFlag[GDMA_PLANE_OSD1] == 1) {
		onlinePlane[onlineMaxNum++] = GDMA_PLANE_OSD1;
		onlineProgDone |= GDMA_CTRL_osd1_prog_done(1);
	}
	if (QueueFlag[GDMA_PLANE_OSD2] == 1) {
		onlinePlane[onlineMaxNum++] = GDMA_PLANE_OSD2;
		onlineProgDone |= GDMA_CTRL_osd2_prog_done(1);
	}
	if (QueueFlag[GDMA_PLANE_OSD3] == 1) {
		onlinePlane[onlineMaxNum++] = GDMA_PLANE_OSD3;
		onlineProgDone |= GDMA_CTRL_osd3_prog_done(1);
	}

	if (inISR) {
		if (cnt == 0)
			return GDMA_SUCCESS;		/*  means no picture to show */
		else {
			DBG_PRINT(KERN_EMERG"%s, how many OSD config at the same time, cnt=%d \n", __FUNCTION__, cnt);
		}
	} else {
		/*  force display and clear syncstamp */
		for (onlineIdx = 0; onlineIdx < onlineMaxNum; onlineIdx++) {
			disPlane = onlinePlane[onlineIdx];

			if (gdma->ctrl.sync[disPlane])
				QueueFlag[disPlane] = 1;
			osdSyncStamp[disPlane] = 0;
		}
	}


	/* update picture info to register */
	for (onlineIdx = cnt = 0; onlineIdx < onlineMaxNum; onlineIdx++) {
		disPlane = onlinePlane[onlineIdx];

		if (gdma->ctrl.sync[disPlane] && (QueueFlag[disPlane] == 1)) {

			curPic = gdma->pic[disPlane] + gdma->curPic[disPlane];
			DBG_PRINT(KERN_EMERG"%s, OSD[%d] c ptr = %d \n", __FUNCTION__, disPlane, gdma->curPic[disPlane]);

			if (osdSyncStamp[disPlane] == 1) {  /*  wait & check for other OSD which has the same syncstamp picture */
				/*  record syncstamp number and check this syncstamp happened previously */
				int synced = 0;
				for (loop = 0; loop < syncPicNum; loop++) {
					if (curPic->syncInfo.syncstamp == osdSyncStampNum[loop]) {
						synced = 1;
						break;
					}
				}
				if (synced == 1)
					continue;      /*  don't find this syncstamp picture again */
				else
					osdSyncStampNum[syncPicNum++] = curPic->syncInfo.syncstamp;  /*  new syncstamp to search it */

				syncPicFlag = GDMA_checkSyncStamp(disPlane);
				DBG_PRINT(KERN_EMERG"%s, after GDMA_checkSyncStamp syncPicFlag=%d \n", __FUNCTION__, syncPicFlag);
				DBG_PRINT(KERN_EMERG"%s, curPic->syncstamp=%lld \n", __FUNCTION__, curPic->syncInfo.syncstamp);

				if (syncPicFlag == 0) { /*  no match syncstamp and hold this picture, so do other HW OSD settings */
					osdSyncStampTimeout[disPlane]++;
					if (osdSyncStampTimeout[disPlane] <= 5)	/*  wait for 5 interrupt time */
						continue;		/*  search the picture of another queue */
					else {
						/*  syncstamp picture time-out, and then force display */
						osdSyncStampTimeout[disPlane] = 0;
						GDMA_OSDReg_Set(disPlane);
						osdSyncStamp[disPlane] = 0;
						pr_debug(KERN_EMERG"%s, force display : disPlane=%d, syncstamp timeout \n", __FUNCTION__, disPlane);
					}
				} else {
					GDMA_OSDReg_Set(disPlane);
					osdSyncStampTimeout[disPlane] = osdSyncStamp[disPlane] = 0;
					for (loop = 0; loop < curPic->syncInfo.matchNum; loop++) {
						DBG_PRINT(KERN_EMERG"%s, the same syncstamp OSD Num=%d \n", __FUNCTION__, curPic->syncInfo.matchPlane[loop]);
						GDMA_OSDReg_Set(curPic->syncInfo.matchPlane[loop]);
						osdSyncStampTimeout[curPic->syncInfo.matchPlane[loop]] = osdSyncStamp[curPic->syncInfo.matchPlane[loop]] = 0;
						DBG_PRINT(KERN_EMERG"%s, osdSyncStamp[%d]=%d \n", __FUNCTION__, curPic->syncInfo.matchPlane[loop], osdSyncStamp[loop]);
					}
				}
			} else { /* Normal case for OSD display */
				/*  call GDMA HW setting */
				GDMA_OSDReg_Set(disPlane);
				for (loop = 0; loop < onlineMaxNum; loop++) {
					disPlane_tmp = onlinePlane[loop];
					if (QueueFlag[disPlane_tmp] == 1 && (disPlane_tmp != disPlane) && osdSyncStamp[disPlane_tmp] == 0) {
						GDMA_OSDReg_Set(disPlane_tmp);
					}
				}
			}

			/*  TODO: record the actual updated OSD number */
			for (loop = 0; loop < onlineMaxNum; loop++) {
				disPlane_tmp = onlinePlane[loop];

				if (QueueFlag[disPlane_tmp] == 1 && osdSyncStamp[disPlane_tmp] == 0) {

					Qend = ((gdma->curPic[disPlane_tmp] + GDMA_MAX_PIC_Q_SIZE - 2) & (GDMA_MAX_PIC_Q_SIZE - 1));
					gdma->picCount[disPlane_tmp]--;
					gdma->picQrd[disPlane_tmp] = Qend;

					if (disPlane_tmp == GDMA_PLANE_OSD1)
						updatedOSDx |= GDMA_CTRL_osd1_prog_done(1);
					else if (disPlane_tmp == GDMA_PLANE_OSD2)
						updatedOSDx |= GDMA_CTRL_osd2_prog_done(1);
					else if (disPlane_tmp == GDMA_PLANE_OSD3)
						updatedOSDx |= GDMA_CTRL_osd3_prog_done(1);

#ifdef TRIPLE_BUFFER_SEMAPHORE
					gSemaphore[disPlane_tmp]++;
					if (gSemaphore[disPlane_tmp] > gSEMAPHORE_MAX) {
						pr_debug(KERN_EMERG"gdma : gSemaphore[%d] %d\n", disPlane_tmp, gSemaphore[disPlane_tmp]);
						gSemaphore[disPlane_tmp] = gSEMAPHORE_MAX;
					}
#endif

					/* clear queue flag */
					QueueFlag[disPlane_tmp] = 0;

					cnt++;
				}
			}

			osd_db_ctrl_reg.regValue = rtd_inl(OSDOVL_OSD_Db_Ctrl_reg);
			osd_db_ctrl_reg.db_load = 1;

#ifdef CONFIG_ANDROID_MOUSE
			/* disable other OSD which is NOT used at this moment */
			/* compress case : mouse go to OSD1 */
			if ((updatedOSDx & GDMA_CTRL_osd1_prog_done(1)) == 0)
				rtd_outl(GDMA_OSD1_CTRL_reg, ~1);
			if ((updatedOSDx & GDMA_CTRL_osd2_prog_done(1)) == 0)
				rtd_outl(GDMA_OSD2_CTRL_reg, ~1);
			if ((updatedOSDx & GDMA_CTRL_osd3_prog_done(1)) == 0)
				rtd_outl(GDMA_OSD3_CTRL_reg, ~1);
#endif

#if 0
			/*
			*	check layers status to enable mid-blend
			*/
			GDMA_MidBlendSet(cnt, updatedOSDx);

			/** line buffer calculation
			*   line buffer calculation should be done before writing programming done
			*	due to dynamic line buffer setting, progrmming done will apply with all online path
			*/
			GDMA_LineBufferCal();
#endif
#if defined(CONFIG_ARCH_RTK287O) || defined(CONFIG_ARCH_RTK288O) || defined(CONFIG_ARCH_RTK2875) || defined(CONFIG_ARCH_RTK2885M) || defined(CONFIG_ARCH_RTK6748)
			onlineProgDone = GDMA_CTRL_write_data(1);
#else
			onlineProgDone = GDMA_CTRL_write_data(1) | GDMA_CTRL_osd_others_prog_done(1);
#endif
			/*  only check real online OSD HW */
			onlineProgDone |= GDMA_CTRL_osd1_prog_done(1);
			onlineProgDone |= GDMA_CTRL_osd2_prog_done(1);
			onlineProgDone |= GDMA_CTRL_osd3_prog_done(1);


			/* osd_other_prog_done needs to program with online osd prog done */
			DBG_PRINT(KERN_EMERG"%s, updatedOSDx = 0x%x \n", __FUNCTION__, updatedOSDx);
			DBG_PRINT(KERN_EMERG"%s, w onlineProgDone = 0x%x \n", __FUNCTION__, onlineProgDone);
			DBG_PRINT(KERN_EMERG"%s, w osd_db_ctrl = 0x%x \n", __FUNCTION__, osd_db_ctrl_reg.regValue);
			rtd_outl(OSDOVL_OSD_Db_Ctrl_reg, osd_db_ctrl_reg.regValue); /*  mixer apply */
			rtd_outl(GDMA_CTRL_reg, onlineProgDone);  /*  gdma & sr programming done */
			DBG_PRINT(KERN_EMERG"%s, r rtd_inl(%x)=0x%x\n", __FUNCTION__, GDMA_CTRL_reg, rtd_inl(GDMA_CTRL_reg));

			if ((curPic->cookie != 0) && (cookie_flag != 0) && (gdma->GDMA_CookieCallBack)) {
				/*if (curPic->cookie != 0) {*/
					gdma->GDMA_CookieCallBack (0, curPic->cookie) ;
					curPic->cookie = 0;
				/*}*/
			}
			cookie_flag = 1;		/* delay one frame when really change picture, driver will call the callback */
			/* End - fox modify for use cookie to sync GDMA */
		}

	}

#if 0   /*  frame-rate calculation by Vsync */
	static unsigned int gdmaINT_cnt;
	static unsigned int spentTime, sta;
	if (gdmaINT_cnt == 0 && sta == 0 && updatedOSDx > 0) {
		sta = rtd_inl(SCPU_CLK90K_LO_reg);
		/* Warning("[GDMA] Starting counting for FPS, sta=%d \n", sta); */
	} else {
		if (rtd_inl(SCPU_CLK90K_LO_reg) > sta && updatedOSDx > 0) {
			spentTime = (rtd_inl(SCPU_CLK90K_LO_reg) - sta);
			gdmaINT_cnt++;
		} else if (gdmaINT_cnt < 1000) {
			if (gdmaINT_cnt) {
				DBG_PRINT(KERN_EMERG"[GDMA]FPS=%d, spend time %d, income time %d \n", (gdmaINT_cnt / (spentTime/90000)), spentTime, gdmaINT_cnt);
			}
		}

		if (gdmaINT_cnt >= 1000) {
			pr_debug(KERN_EMERG"[GDMA]FPS=%d, spend time %d, income time %d \n", (gdmaINT_cnt / (spentTime/90000)), spentTime, gdmaINT_cnt);
			sta = spentTime = gdmaINT_cnt = 0;
		}

		/* Warning("[GDMA] Counting , gdmaINT_cnt=%d \n", gdmaINT_cnt); */
	}
#endif


	return GDMA_SUCCESS;
}



int GDMA_Display (GDMA_DISPLAY *cmd)
{
	gdma_dev *gdma = &gdma_devices[0];
	if (cmd->plane == GDMA_PLANE_NONE) {
		gdma->ctrl.zeroBuffer = cmd->zeroBuffer;
		if ((gdma->ctrl.zeroBuffer))
			GDMA_Flush (gdma);
	} else
		gdma->ctrl.zeroBuffer = 0;

	GDMA_ENTER_CRITICAL ();

	GDMA_Hide (gdma);
	gdma->plane = cmd->plane;
	GDMA_Update (gdma, 0);

	GDMA_EXIT_CRITICAL ();

	return GDMA_SUCCESS;
}

int GDMA_OSD1OnTop (bool bOSD1OnTop)
{
	gdma_dev *gdma = &gdma_devices[0];
#if defined(CONFIG_ARCH_RTK287O) || defined(CONFIG_ARCH_RTK288O) || defined(CONFIG_ARCH_RTK2875) || defined(CONFIG_ARCH_RTK2885M) || defined(CONFIG_ARCH_RTK6748)
	unsigned int onlineProgDone = GDMA_CTRL_write_data(1);
#else
	unsigned int onlineProgDone = GDMA_CTRL_write_data(1) | GDMA_CTRL_osd_others_prog_done(1);
#endif

	GDMA_ENTER_CRITICAL ();
	gdma->ctrl.OSD1onTop = bOSD1OnTop;

	/*  only check online OSD HW */
	rtd_outl(GDMA_OSD1_CTRL_reg, ~1);
	onlineProgDone |= GDMA_CTRL_osd1_prog_done(1);
	rtd_outl(GDMA_OSD2_CTRL_reg, ~1);
	onlineProgDone |= GDMA_CTRL_osd2_prog_done(1);
	rtd_outl(GDMA_OSD3_CTRL_reg, ~1);
	onlineProgDone |= GDMA_CTRL_osd3_prog_done(1);
	rtd_outl(GDMA_CTRL_reg,  onlineProgDone);

	GDMA_Update (gdma, 0);
	GDMA_EXIT_CRITICAL ();

	if (gdma->osdPlane[VO_GRAPHIC_OSD1].OSD_ctrl.en)
		GDMA_UpdateGraphicwindow (1, VO_GRAPHIC_OSD1);
	if (gdma->osdPlane[VO_GRAPHIC_OSD2].OSD_ctrl.en)
		GDMA_UpdateGraphicwindow (1, VO_GRAPHIC_OSD2);
	if (gdma->osdPlane[VO_GRAPHIC_OSD3].OSD_ctrl.en)
		GDMA_UpdateGraphicwindow (1, VO_GRAPHIC_OSD3);
	return GDMA_SUCCESS;
}

int GDMA_Config3D (bool enable3D)
{
	gdma_dev *gdma = &gdma_devices[0];
	if(GAL_Runtime_Suspend_status)
	    return GDMA_SUCCESS;

	gdma->ctrl.enable3D = enable3D;
	if ((gdma->ctrl.enable3D)) {
		/* rtd_outl(GDMA_OSD1_CTRL_reg, GDMA_OSD1_CTRL_write_data(1) | GDMA_OSD1_CTRL_d3_mode (1)); */
		/* rtd_outl(GDMA_OSD2_CTRL_reg, GDMA_OSD2_CTRL_write_data(1) | GDMA_OSD2_CTRL_d3_mode (1)); */
		rtd_outl(GDMA_OSD_INTEN_3D_reg, GDMA_OSD_INTEN_3D_write_data(1) | GDMA_OSD_INTEN_3D_osd1_vact_end(1));
	} else { /* 2D */
		rtd_outl(GDMA_OSD1_CTRL_reg, GDMA_OSD1_CTRL_write_data(0) | GDMA_OSD1_CTRL_d3_mode (1));
		rtd_outl(GDMA_OSD2_CTRL_reg, GDMA_OSD2_CTRL_write_data(0) | GDMA_OSD2_CTRL_d3_mode (1));
		rtd_outl(GDMA_OSD3_CTRL_reg, GDMA_OSD3_CTRL_write_data(0) | GDMA_OSD3_CTRL_d3_mode (1));
		rtd_outl(GDMA_OSD_INTEN_3D_reg, GDMA_OSD_INTEN_3D_write_data(0) | GDMA_OSD_INTEN_3D_osd1_vact_end(1));

		*(volatile u32 *)&gdma->osd3Doffset[0] = 0;
		*(volatile u32 *)&gdma->osd3Doffset[1] = 0;
		*(volatile u32 *)&gdma->osd3Doffset[2] = 0;
		gdma->ctrl.enableOSD1Offset = 0;
		gdma->ctrl.enableOSD2Offset = 0;
		gdma->ctrl.enableOSD3Offset = 0;

		gdma->ctrl.updateOSD1Offset = 0;
		gdma->ctrl.updateOSD2Offset = 0;
		gdma->ctrl.updateOSD3Offset = 0;

#ifndef USING_GDMA_VSYNC
		rtd_outl(GDMA_OSD_INTEN_reg, GDMA_OSD_INTEN_write_data(1) | GDMA_OSD_INTEN_osd1_vact_end(1));
#else
		rtd_outl(GDMA_OSD_INTEN_reg, GDMA_OSD_INTEN_write_data(1) | GDMA_OSD_INTEN_osd1_vsync(1));
#endif

	}

	return GDMA_SUCCESS;
}
#if GDMA_GAMMA
int GDMA_ConfigGammaColorMapping(GAMMA_COLOR_MAPPING_DATA *data)
{
	unsigned int reg;

	rtd_outl(OSDOVL_Mixer_Osd3x3_Data_In_Offset_0_reg,
		OSDOVL_Mixer_Osd3x3_Data_In_Offset_0_x_offset(data->cm_x_offset)|
		OSDOVL_Mixer_Osd3x3_Data_In_Offset_0_y_offset(data->cm_y_offset));

	rtd_outl(OSDOVL_Mixer_Osd3x3_Data_In_Offset_1_reg,
			 OSDOVL_Mixer_Osd3x3_Data_In_Offset_1_z_offset(data->cm_z_offset));

	rtd_outl(OSDOVL_Mixer_Osd3x3_Data_0_reg,
			 OSDOVL_Mixer_Osd3x3_Data_0_k12(data->cm_k12)|
			 OSDOVL_Mixer_Osd3x3_Data_0_k11(data->cm_k11));

	rtd_outl(OSDOVL_Mixer_Osd3x3_Data_1_reg,
			 OSDOVL_Mixer_Osd3x3_Data_1_k21(data->cm_k21)|
			 OSDOVL_Mixer_Osd3x3_Data_1_k13(data->cm_k13));

	rtd_outl(OSDOVL_Mixer_Osd3x3_Data_2_reg,
			 OSDOVL_Mixer_Osd3x3_Data_2_k23(data->cm_k23)|
			 OSDOVL_Mixer_Osd3x3_Data_2_k22(data->cm_k22));

	rtd_outl(OSDOVL_Mixer_Osd3x3_Data_3_reg,
			 OSDOVL_Mixer_Osd3x3_Data_3_k32(data->cm_k32)|
			 OSDOVL_Mixer_Osd3x3_Data_3_k31(data->cm_k31));

	rtd_outl(OSDOVL_Mixer_Osd3x3_Data_4_reg,
			 OSDOVL_Mixer_Osd3x3_Data_4_k33(data->cm_k33));

	rtd_outl(OSDOVL_Mixer_Osd3x3_Out_offset_0_reg,
			 OSDOVL_Mixer_Osd3x3_Out_offset_0_goffset(data->cm_r_offset)|
			 OSDOVL_Mixer_Osd3x3_Out_offset_0_roffset(data->cm_g_offset));

	rtd_outl(OSDOVL_Mixer_Osd3x3_Out_offset_1_reg,
			 OSDOVL_Mixer_Osd3x3_Out_offset_1_boffset(data->cm_b_offset));

	return GDMA_SUCCESS;
}

int GDMA_ConfigGammaPosition(unsigned int data)
{
	unsigned int reg;

	reg = rtd_inl(OSDOVL_Mixer_OSD_GAMMA_CTRL_reg);

	reg &= ~(OSDOVL_Mixer_OSD_GAMMA_CTRL_osd_gamma_position_mask);

	reg |= OSDOVL_Mixer_OSD_GAMMA_CTRL_osd_gamma_position(data);

	rtd_outl(OSDOVL_Mixer_OSD_GAMMA_CTRL_reg,reg);

	return GDMA_SUCCESS;
}




int GDMA_GammaOperation(GDMA_GAMMA_OP* data)
{
	int repeat_cnt = 0;
	unsigned int gamma_reg_base = OSDOVL_Mixer_GAMMA1_CTRL_1_reg;
	unsigned int gamma_reg_port = OSDOVL_Mixer_gamma1_port_reg;
	osdovl_mixer_gamma1_ctrl_1_RBUS gctrl;
	osdovl_mixer_gamma1_port_RBUS gport;


	if(data->gamma_num != 1) {
		gamma_reg_base = OSDOVL_Mixer_GAMMA2_CTRL_1_reg;
		gamma_reg_port = OSDOVL_Mixer_gamma2_port_reg;
	}

	while(1) {
		gctrl.regValue = rtd_inl(gamma_reg_base);

		if(gctrl.gamma1_fifo_empty == 1 || gctrl.gamma1_fifo_full == 1)
			break;
		repeat_cnt++;
		if(repeat_cnt >=10) {
			printk(KERN_ERR"[GDMA] update gamma-%d timeout.", data->gamma_num);
			return -1;
		}
		gdma_usleep(3000);
	}
	for(repeat_cnt=0; repeat_cnt < data->gamma_size;repeat_cnt++) {
		/*1:wreite,  2:read*/
		gctrl.gamma1_ax_rw_sel = data->op;
		gctrl.gamma1_ax_ch_sel = data->channel;
		rtd_outl(gamma_reg_base,gctrl.regValue);
		gport.gamma1_tab_index = data->gamma_data[repeat_cnt].index;
		gport.gamma1_tab_d0 = data->gamma_data[repeat_cnt].d0;
		gport.gamma1_tab_d1 = data->gamma_data[repeat_cnt].d1;
		rtd_outl(gamma_reg_port,gport.regValue);
	}
	gctrl.gamma1_ax_rw_sel = 0;
	rtd_outl(gamma_reg_base,gctrl.regValue);


	gctrl.regValue = rtd_inl(gamma_reg_base);
	if(gctrl.gamma1_wr_error) {
		printk(KERN_ERR"[GDMA] update gamma-%d timeout.", data->gamma_num);
		return -1;
	}
	return 0;
}
#endif
int GDMA_ConfigVFlip(bool enable)
{
	gdma_dev *gdma = &gdma_devices[0];
	int linecnt_start, linecnt_end = 0;
#if defined(CONFIG_ARCH_RTK287O) || defined(CONFIG_ARCH_RTK288O) || defined(CONFIG_ARCH_RTK2875) || defined(CONFIG_ARCH_RTK2885M) || defined(CONFIG_ARCH_RTK6748)
	unsigned int onlineProgDone = GDMA_CTRL_write_data(1);
#else
	unsigned int onlineProgDone = GDMA_CTRL_write_data(1) | GDMA_CTRL_osd_others_prog_done(1);
#endif
#if defined(CONFIG_ARCH_RTK287O) || defined(CONFIG_ARCH_RTK288O) || defined(CONFIG_ARCH_RTK2875) || defined(CONFIG_ARCH_RTK2885M) || defined(CONFIG_ARCH_RTK6748)
	unsigned int prog_done_mask = GDMA_CTRL_osd3_prog_done_mask | GDMA_CTRL_osd1_prog_done_mask;
#else
	unsigned int prog_done_mask = GDMA_CTRL_osd_others_prog_done_mask |
		GDMA_CTRL_osd3_prog_done_mask | GDMA_CTRL_osd1_prog_done_mask;
#endif
	unsigned long timeout;

	linecnt_start = (rtd_inl(PPOVERLAY_new_meas0_linecnt_real_reg)&0x1FFF);
	pr_debug("%s, linecnt_start = 0x%x \n", __FUNCTION__, linecnt_start);

	/*  only check online OSD HW */
	rtd_outl(GDMA_OSD1_CTRL_reg, ~1);
	onlineProgDone |= GDMA_CTRL_osd1_prog_done(1);
	rtd_outl(GDMA_OSD2_CTRL_reg, ~1);
	onlineProgDone |= GDMA_CTRL_osd2_prog_done(1);
	rtd_outl(GDMA_OSD3_CTRL_reg, ~1);
	onlineProgDone |= GDMA_CTRL_osd3_prog_done(1);

	rtd_outl(GDMA_CTRL_reg, GDMA_CTRL_write_data(1) | onlineProgDone);

	timeout = jiffies + (HZ/50); /* timeout in 20 ms */
	while (rtd_inl(GDMA_CTRL_reg) & prog_done_mask) {
		rtd_outl(GDMA_OSD1_CTRL_reg, GDMA_OSD1_CTRL_write_data(enable) | GDMA_OSD1_CTRL_rotate (1));
		rtd_outl(GDMA_OSD2_CTRL_reg, GDMA_OSD2_CTRL_write_data(enable) | GDMA_OSD2_CTRL_rotate (1));
		rtd_outl(GDMA_OSD3_CTRL_reg, GDMA_OSD3_CTRL_write_data(enable) | GDMA_OSD3_CTRL_rotate (1));
		gdma->ctrl.enableVFlip = enable;

		if (time_before(jiffies, timeout) == 0) {		/*  over 20 ms */
			linecnt_end = (rtd_inl(PPOVERLAY_new_meas0_linecnt_real_reg)&0x1FFF);
			pr_debug("%s, TimeOut. linecnt_end = 0x%x \n", __FUNCTION__, linecnt_end);
			if (linecnt_start == linecnt_end)   /*  no sync signal by DTG measure */
				break;
		}
	}
	return GDMA_SUCCESS;
}
void GDMA_DUMP_ReceivePicture(int dump_count)
{	
	KGAL_SURFACE_INFO_T dSurface;
	osd_dmem_info dinfo;
	unsigned int width = 1920;
	unsigned int height = 1080;
	unsigned int phyaddr;

	memset(&dSurface, 0, sizeof(KGAL_SURFACE_INFO_T));

	phyaddr = pli_malloc(width * height * 4, GFP_DCU1);

	if (phyaddr == INVALID_VAL)
		printk(KERN_EMERG"[%s:%d] GDMA_DBG_CMD_OSD_CAP_FB pil malloc fail! size = %d\n", __func__, __LINE__, width * height * 4);
	else {
		dSurface.width = width;
		dSurface.height = height;
		dSurface.pitch = width * 4;
		dSurface.bpp = 32;
		dSurface.physicalAddress = phyaddr;
		dSurface.pixelFormat = KGAL_PIXEL_FORMAT_ARGB;

		GDMA_CaptureFramebuffer(&dSurface);

		dinfo.x = 0;
		dinfo.y = 0;
		dinfo.w = dSurface.width;
		dinfo.h = dSurface.height;
		dinfo.pitch = dSurface.pitch;
		dinfo.viraddr =  phys_to_virt(dSurface.physicalAddress);
		snprintf(dinfo.fname,128,"/data/gdma_dump_%d.bmp",dump_count);

		osd_deme_write_file(&dinfo);

		pli_free(phyaddr);
	}
}

int GDMA_Debug(KADP_HAL_GAL_CTRL_T *data)
{
	int ret = GDMA_SUCCESS;
	osd_dmem_info dinfo;
	memset(&dinfo,0,sizeof(osd_dmem_info));
	if (data->osdn >= GDMA_PLANE_MAXNUM)
		goto GDMA_DEBUG_OSD_ERR;

	switch (data->type) {
		case GDMA_DBG_CMD_SET_H_SHIFT:
		case GDMA_DBG_CMD_SET_V_SHIFT:
			/*for tv006, need this*/
			break;
		case GDMA_DBG_CMD_OSD_DUMP:
			if (data->osdn == 0)
				goto GDMA_DEBUG_OSD_ERR;
			snprintf(dinfo.fname,128,"%s",data->capinfo.fname);
			/*don't care about size, we read them from win info*/
			ret = gdma_osd_writedump(0,data->osdn, &dinfo, NULL);
			break;
		case GDMA_DBG_CMD_OSD_DUMP_EX:
			if (data->osdn == 0)
				goto GDMA_DEBUG_OSD_ERR;
			/*don't care about size, we read them from win info*/
			snprintf(dinfo.fname,128,"%s",data->capinfo.fname);
			ret = gdma_osd_writedump(1,data->osdn, &dinfo, NULL);
			break;
		case GDMA_DBG_CMD_OSD_DROP:
			OSD_Freeze(data->en);
			break;
		case GDMA_DBG_CMD_OSD_PATTERN:
			OSD_Pattern(data);
			break;
		case GDMA_DBG_CMD_OSD_VFLIP:
			ret = OSD_debug_vflip(data->en);
			break;
		case GDMA_DBG_CMD_OSD_CTRL:
			if(data->osdn == 0) {
				if (data->en)
					rtd_setbits(OSDOVL_Mixer_CTRL2_reg, 1);
				else
					rtd_clearbits(OSDOVL_Mixer_CTRL2_reg, 1);
			}else{
				if (data->en) {
					rtd_setbits(OSDOVL_Mixer_CTRL2_reg, 1<<data->osdn);
				}else{
					rtd_clearbits(OSDOVL_Mixer_CTRL2_reg, 1<<data->osdn);
				}
			}
			rtd_outl(OSDOVL_OSD_Db_Ctrl_reg, 0x3);
			break;
		case GDMA_DBG_CMD_OSD_SWAP:
		{
			unsigned int value = rtd_inl(OSDOVL_Mixer_layer_sel_reg)&0xff000000;
			int cnt = 0;
			for(cnt=0;cnt<6;cnt++) {
				if(data->c[cnt] < GDMA_PLANE_MAXNUM){
					value |= data->c[cnt] <<(cnt*4);
				}
			}
			rtd_outl(OSDOVL_Mixer_layer_sel_reg, value);
			rtd_outl(OSDOVL_OSD_Db_Ctrl_reg, 0x1);
			break;
		}
		case GDMA_DBG_CMD_OSD_CAP2MEM:
			if (data->osdn == 0)
				goto GDMA_DEBUG_OSD_ERR;
			ret = gdma_osd_dmp2mem(data);
			break;
		case GDMA_DBG_CMD_SHOW_INFO:
			ret = gdma_osd_winfo_dump();
			break;
		case GDMA_DBG_CMD_OSD_GET_INFO:
			ret = gdma_osd_winfo_read(data);
			break;
		case GDMA_DBG_CMD_OSD_CTRL_EN:
			ret = GDMA_ConfigOSDxEnable(data->osdn, data->en);
			break;
		case GDMA_DBG_CMD_SHOW_INT_STATUS:
			gdma_osd_int_status_dump();
			break;
		case GDMA_DBG_CMD_OSD_CLONE_EN:
			ret = GDMA_CloneEnable(data->en);
			break;
		case GDMA_DBG_CMD_OSD_BW_GEN_EN:
			ret = GDMA_BW_GEN_Enable(data->osdn,data->en,data->w,data->h);
			break;
		case GDMA_DBG_CMD_OSD_SYNC_EN:
			osd_sync_en = data->en;
			break;
		case GDMA_DBG_CMD_SPLIT_MODE:
#if USE_RLINK_INTERFACE
			GDMA_rlink_split(change_OSD_mode);
#endif
			break;
		case GDMA_DBG_CMD_STOP_UPDATE:
			break;	
		case GDMA_DBG_CMD_OSD_RLINK:
#if USE_RLINK_INTERFACE
			GDMA_rlink_interface_call(rlink_interface_cmd);
#endif
			break;
		case GDMA_DBG_CMD_ES_OSD_ADDR:
			GDMA_ES_OSD_Data_test(cp_src);
			break;
		case GDMA_DBG_CMD_OSD_GAL_AI_BLOCK:
		{
			KGAL_RECT_T ai_block[MAX_AI_BLOCK];
			unsigned int ai_colors[MAX_AI_BLOCK];
			unsigned int w=0,h=0;
			ai_block[0].x=0;
			ai_block[0].y=0;
			ai_block[0].w=50;
			ai_block[0].h=50;
			ai_block[1].x=50;
			ai_block[1].y=50;
			ai_block[1].w=50;
			ai_block[1].h=50;
			ai_block[2].x=100;
			ai_block[2].y=100;
			ai_block[2].w=50;
			ai_block[2].h=50;
			ai_block[3].x=150;
			ai_block[3].y=150;
			ai_block[3].w=50;
			ai_block[3].h=50;
			ai_block[4].x=200;
			ai_block[4].y=200;
			ai_block[4].w=50;
			ai_block[4].h=50;
			ai_colors[0] = ai_color;//0xf0f0;
			ai_colors[1] = ai_color;//0xf0ff;
			ai_colors[2] = ai_color;//0xfff0;
			ai_colors[3] = ai_color;//0x0ff0;
			ai_colors[4] = ai_color;//0x00ff;
			if(change_OSD_mode == GDMA_SPLIT_MODE_2
			|| change_OSD_mode == GDMA_SPLIT_MODE_3){
				w = AI_WIDTH_1920;
				h = AI_HEIGH_1080;
			}else{
				w = AI_WIDTH;
				h = AI_HEIGH;
			}
				
			if(ai_num>=0 && ai_num <=5)
				GDMA_AI_SE_draw_block(AI_WIDTH,AI_HEIGH,ai_num,
							  ai_colors,ai_block,0) ;
			else
				GDMA_AI_SE_draw_block(AI_WIDTH,AI_HEIGH,5,
									  ai_colors,ai_block,0) ;
		}
			break;
		case GDMA_DBG_CMD_OSD_GAL_AI_BLOCK_AFBC:
		{
			KGAL_RECT_T ai_block[MAX_AI_BLOCK];
			unsigned int ai_colors[MAX_AI_BLOCK];
			unsigned int w=0,h=0;
			ai_block[0].x=0;
			ai_block[0].y=0;
			ai_block[0].w=256;
			ai_block[0].h=256;
			ai_block[1].x=50;
			ai_block[1].y=50;
			ai_block[1].w=50;
			ai_block[1].h=50;
			ai_block[2].x=100;
			ai_block[2].y=100;
			ai_block[2].w=50;
			ai_block[2].h=50;
			ai_block[3].x=150;
			ai_block[3].y=150;
			ai_block[3].w=50;
			ai_block[3].h=50;
			ai_block[4].x=200;
			ai_block[4].y=200;
			ai_block[4].w=50;
			ai_block[4].h=50;
			ai_colors[0] = ai_color;//0xf0f0;
			ai_colors[1] = ai_color;//0xf0ff;
			ai_colors[2] = ai_color;//0xfff0;
			ai_colors[3] = ai_color;//0x0ff0;
			ai_colors[4] = ai_color;//0x00ff;
			if(change_OSD_mode == GDMA_SPLIT_MODE_2
			|| change_OSD_mode == GDMA_SPLIT_MODE_3){
				w = AI_WIDTH_1920;
				h = AI_HEIGH_1080;
			}else{
				w = AI_WIDTH;
				h = AI_HEIGH;
			}
			
			if(ai_num>=0 && ai_num <=5)
				GDMA_AI_SE_draw_block_afbc(AI_WIDTH,AI_HEIGH,1,
									  ai_colors,ai_block,0) ;
			else
				GDMA_AI_SE_draw_block_afbc(AI_WIDTH,AI_HEIGH,5,
									  ai_colors,ai_block,0) ;
		}
			break;
		case GDMA_DBG_CMD_OSD_GET_GOLBAL_ALPHA:
		{
			unsigned int alpha[16];
			int i = 0;
			GDMA_GetGlobalAlpha((unsigned int *)&alpha, 16);
			
			printk(KERN_ERR"global alpha table= \n");
			
			for (i = 0; i < 16; i++)
				printk(KERN_ERR"alpha[%d] = 0x%x\n", i, alpha[i]);
		}
			break;	
		case GDMA_DBG_CMD_OSD_SET_OSD_GAIN:
		{
			GDMA_ConfigRGBGain(data->en, data->gain);
		}
			break;
		case GDMA_DBG_CMD_OSD_CAP_FB:
		{
			KGAL_SURFACE_INFO_T dSurface;
			osd_dmem_info dinfo;
			unsigned int width = 1920;
			unsigned int height = 1080;
			unsigned int phyaddr;

			memset(&dSurface, 0, sizeof(KGAL_SURFACE_INFO_T));

			phyaddr = pli_malloc(width * height * 4, GFP_DCU1);

			if (phyaddr == INVALID_VAL)
				printk(KERN_EMERG"[%s:%d] GDMA_DBG_CMD_OSD_CAP_FB pil malloc fail! size = %d\n", __func__, __LINE__, width * height * 4);
			else {
				dSurface.width = width;
				dSurface.height = height;
				dSurface.pitch = width * 4;
				dSurface.bpp = 32;
				dSurface.physicalAddress = phyaddr;
				dSurface.pixelFormat = KGAL_PIXEL_FORMAT_ARGB;

				GDMA_CaptureFramebuffer(&dSurface);

				dinfo.x = 0;
				dinfo.y = 0;
				dinfo.w = dSurface.width;
				dinfo.h = dSurface.height;
				dinfo.pitch = dSurface.pitch;
				dinfo.viraddr =  phys_to_virt(dSurface.physicalAddress);
				snprintf(dinfo.fname,128,"/data/%s.bmp",data->capinfo.fname);

				osd_deme_write_file(&dinfo);

				pli_free(phyaddr);
			}
		}
			break;
		case GDMA_DBG_CMD_OSD_SET_OSD_ALPHA_DETECT:
		{
			GDMA_SetAlphaOsdDetectionParameter(data->en);
		}
			break;
		case GDMA_DBG_CMD_OSD_GET_OSD_ALPHA_RATIO:
		{
			int data=0;
			
			GDMA_GetAlphaOsdDetectionratio(&data);
			printk(" GDMA_DBG_CMD_OSD_GET_OSD_ALPHA_RATIO = %d \n",data);
		}
		break;
		case GDMA_DBG_CMD_OSD_MONITOR_CTL:
		{
			printk(" GDMA_DBG_CMD_OSD_MONITOR_CTL \n");
		}
		break;
		case GDMA_DBG_CMD_OSD_RUNTIME_CTL:
		{
			printk(" GDMA_DBG_CMD_OSD_RUNTIME_CTL \n");
		}
		break;
		case GDMA_DBG_CMD_OSD_CONTINUE_DUMP:
		{
			
			unsigned int wininfo_addr = rtd_inl(GDMA_OSD1_WI_reg); 
			if(gDebugGDMA_dump_count ==0)
				gDebugGDMA_dump_count = 10;
			printk(KERN_ERR" GDMA_DBG_CMD_OSD_CONTINUE_DUMP start %d \n",gDebugGDMA_dump_count);
			while(gDebugGDMA_dump_count)
			{	
				if(wininfo_addr != rtd_inl(GDMA_OSD1_WI_reg))
				{
					
					GDMA_DUMP_ReceivePicture(gDebugGDMA_dump_count);
					wininfo_addr = rtd_inl(GDMA_OSD1_WI_reg); 					
					printk(KERN_ERR" GDMA_DBG_CMD_OSD_CONTINUE_DUMP wininfo_addr= %d \n",wininfo_addr);
					gDebugGDMA_dump_count--;
				}
				
				
			}
		}
		break;
		default:
			break;
	}
	return ret;
GDMA_DEBUG_OSD_ERR:
	printk(KERN_ERR"OSD-%d is illegal.\n", data->osdn);
	ret = -1;
	return ret;
}

int GDMA_Config3DOffset (CONFIG_3D_OFFSET_STRUCT *data)
{
	gdma_dev *gdma = &gdma_devices[0];
	int i;

	GDMA_ENTER_CRITICAL ();
	if (data->plane == VO_GRAPHIC_OSD1) {
		gdma->osd3Doffset[0].left_dir     =  data->closeToViewer;
		gdma->osd3Doffset[0].left_offset  =  data->offset;
		gdma->osd3Doffset[0].right_dir    = !data->closeToViewer;
		gdma->osd3Doffset[0].right_offset =  data->offset;

		gdma->ctrl.enableOSD1Offset =
			gdma->ctrl.updateOSD1Offset = 1;
	} else if (data->plane == VO_GRAPHIC_OSD2) {
		gdma->osd3Doffset[1].left_dir     =  data->closeToViewer;
		gdma->osd3Doffset[1].left_offset  =  data->offset;
		gdma->osd3Doffset[1].right_dir    = !data->closeToViewer;
		gdma->osd3Doffset[1].right_offset =  data->offset;

		gdma->ctrl.enableOSD2Offset =
			gdma->ctrl.updateOSD2Offset = 1;
	} else if (data->plane == VO_GRAPHIC_OSD3) {
		gdma->osd3Doffset[2].left_dir     =  data->closeToViewer;
		gdma->osd3Doffset[2].left_offset  =  data->offset;
		gdma->osd3Doffset[2].right_dir    = !data->closeToViewer;
		gdma->osd3Doffset[2].right_offset =  data->offset;

		gdma->ctrl.enableOSD3Offset =
			gdma->ctrl.updateOSD3Offset = 1;
	} else if (data->plane == -1) {
		for (i = 0; i < MAX_OSD_NUM; i++) {
			gdma->osd3Doffset[i].left_dir     =  data->closeToViewer;
			gdma->osd3Doffset[i].left_offset  =  data->offset;
			gdma->osd3Doffset[i].right_dir    = !data->closeToViewer;
			gdma->osd3Doffset[i].right_offset =  data->offset;
		}
		gdma->ctrl.enableOSD1Offset = 1;
		gdma->ctrl.updateOSD1Offset = 1;
		gdma->ctrl.enableOSD2Offset = 1;
		gdma->ctrl.updateOSD2Offset = 1;
		gdma->ctrl.enableOSD3Offset = 1;
		gdma->ctrl.updateOSD3Offset = 1;
	}

	GDMA_EXIT_CRITICAL ();

	return GDMA_SUCCESS;
}

int GDMA_ConfigBlending (CONFIG_BLENDING_STRUCT *data)
{

	osdovl_mixer_ctrl2_RBUS mixer_ctrl2;
	*(volatile u32 *)&mixer_ctrl2 = rtd_inl(OSDOVL_Mixer_CTRL2_reg);
#if 0   /*  no blending setting in magellan2 */
	gdma_dev *gdma = &gdma_devices[0];
	if (gdma->ctrl.OSD1onTop) {
		if (data->blendingOSD1En >= 0)
			mixer_ctrl2.MAC3.blendingo2_en    = !data->blendingOSD1En;
		if (data->blendingOSD2En >= 0)
			mixer_ctrl2.MAC3.blendingo1_en    = !data->blendingOSD2En;
	} else {
		if (data->blendingOSD1En >= 0)
			mixer_ctrl2.MAC3.blendingo1_en    = !data->blendingOSD1En;
		if (data->blendingOSD2En >= 0)
			mixer_ctrl2.MAC3.blendingo2_en    = !data->blendingOSD2En;
	}
#endif
	rtd_outl(OSDOVL_Mixer_CTRL2_reg, *(volatile u32 *)&mixer_ctrl2);

	return GDMA_SUCCESS;
}

int GDMA_ConfigureDisplayWindow (GDMA_CONFIG_DISP_WIN *cmd)
{
	gdma_dev *gdma = &gdma_devices[0];
#ifdef CONFIG_CUSTOMER_TV006
	GDMA_PLANE *osdPlane =  NULL;
	if (cmd == NULL)
		return GDMA_FAIL;
	if(cmd->plane >= GDMA_PLANE_MAXNUM)
		return GDMA_FAIL;

	osdPlane = &gdma->osdPlane[cmd->plane];
	osdPlane->OSD_canvas.dispWin = *(VO_RECTANGLE *)&cmd->dispWin;
#else
	VO_RECTANGLE disp_rec;
	if (cmd == NULL)
		return GDMA_FAIL;
	if(cmd->plane >= GDMA_PLANE_MAXNUM)
		return GDMA_FAIL;

	getDispSize(&disp_rec);
	if (gdma->f_box_mode && gdma->box_dst_height == VIDEO_USERDEFINED) {
		memcpy(&gdma->dispWin[cmd->plane], &cmd->dispWin, sizeof(VO_RECTANGLE));
	} else {
		memcpy(&gdma->dispWin[cmd->plane], &disp_rec, sizeof(VO_RECTANGLE));
	}
#endif
	pr_debug(KERN_EMERG"%s, gdma->dispWin[%d].width=%d, %d\n", __FUNCTION__, cmd->plane, gdma->dispWin[cmd->plane].width, gdma->dispWin[cmd->plane].height);

	return GDMA_SUCCESS;
}


/** @brief receive the picture, then send to pic ring-queue and workqueue
 *  @param data the pointer to layer information which want to show
 *  @return GDMA_SUCCESS
 */

/*
 * When OSD shift value is minus (left shift),
 * we need to check right side of cursor shape is visible or not.
 * This lambda returns a value that will be cut at the right side.
 *
 * */

static void gdam_lambda_cursor_shift_h(GDMA_WIN *win, GDMA_PICTURE_OBJECT *picObj){
	int cut_h = 0;
	if(win->dst_x >= abs(g_osdshift_ctrl.h_shift_pixel)) {
		//right side
		cut_h = 3840 - picObj->dst_x;
		if(cut_h <= 0){
			win->dst_x = 0;
			picObj->dst_x = 0;
			win->attr.alpha = 0;
			win->attr.alphaEn = 1;
		}else if (cut_h <= win->winWH.width*2) {
			win->winXY.x = (win->winWH.width-cut_h/2);
			win->dst_x -= win->winXY.x*2;
			win->dst_x -= abs(g_osdshift_ctrl.h_shift_pixel);
			picObj->dst_x = win->dst_x;
		}else {
			//normal case
			win->dst_x -= abs(g_osdshift_ctrl.h_shift_pixel);
			picObj->dst_x = win->dst_x;
		}
	}else{
		//cut_h = x_offset of cursor , left side
		cut_h = (abs(g_osdshift_ctrl.h_shift_pixel) - win->dst_x)/2;
		if(cut_h < win->winWH.width){
			win->dst_x = 0;
			picObj->dst_x = 0;
			win->objOffset.objXoffset = cut_h;
			win->winWH.width -= win->objOffset.objXoffset;
			win->dst_width = win->winWH.width*2;
			picObj->dst_width = win->dst_width;
		}else{
			win->dst_x = 0;
			picObj->dst_x = 0;
			win->attr.alpha = 0;
			win->attr.alphaEn = 1;
		}
	}
	return;
}


static void gdam_lambda_cursor_shift_v(GDMA_WIN *win, GDMA_PICTURE_OBJECT *picObj){
	int cut_v = 0;
	if(win->dst_y >= abs(g_osdshift_ctrl.v_shift_pixel)) {
		//bottom
		cut_v = 2160 - picObj->dst_y;
		if(cut_v <= 0){
			win->dst_y = 0;
			picObj->dst_y = 0;
			win->attr.alpha = 0;
			win->attr.alphaEn = 1;
		}else if (cut_v <= win->winWH.height*2) {
			win->winXY.y = (win->winWH.height-cut_v/2);
			win->dst_y -= win->winXY.y*2;
			win->dst_y -= abs(g_osdshift_ctrl.v_shift_pixel);
			picObj->dst_y = win->dst_y;
		}else{
			//normal case
			win->dst_y -= abs(g_osdshift_ctrl.v_shift_pixel);
			picObj->dst_y -= abs(g_osdshift_ctrl.v_shift_pixel);
		}

	}else{
		//cut_v = y_offset of cursor, up
		cut_v = (abs(g_osdshift_ctrl.v_shift_pixel) - win->dst_y)/2;
		if(cut_v < win->winWH.height){
			win->dst_y = 0;
			picObj->dst_y = 0;
			win->objOffset.objYoffset = cut_v;
			win->winWH.height -= win->objOffset.objYoffset;
			win->dst_height = win->winWH.height*2;
			picObj->dst_height = win->dst_height;
		}else{
			win->dst_y = 0;
			picObj->dst_y = 0;
			win->attr.alpha = 0;
			win->attr.alphaEn = 1;
		}
	}
	return;
}




void GDMA_ReceivePicture_warpper(void)
{
  GDMA_ReceivePicture(&glastinfo);
}

int GDMA_ReceivePicture (PICTURE_LAYERS_OBJECT *data)
{
	gdma_dev *gdma = &gdma_devices[0];
	GDMA_PIC_DATA *pic = NULL;
	GDMA_WIN *win = NULL, *win3D = NULL;
	COMPOSE_PICTURE_OBJECT *com_picObj = NULL;
	int idx, ilayer, emptyIdx = 0, idxQwr = 0, sema_ret;
	GDMA_PICTURE_OBJECT *picObj = NULL;
	GDMA_DISPLAY_PLANE disPlane = GDMA_PLANE_OSD1;
	VO_RECTANGLE disp_res;
	int onlinePlane[GDMA_PLANE_MAXNUM], onlineMaxNum = 0;
	unsigned int saveBufAddr = 0;
	unsigned long timeout = jiffies + (HZ/50);	/* 20 ms */

#if IS_ENABLED(CONFIG_REALTEK_PCBMGR)
		PANEL_CONFIG_PARAMETER *panel_parameter = (PANEL_CONFIG_PARAMETER *)&platform_info.panel_parameter;
#endif

#if defined(CONFIG_RTK_KDRV_GAL)
#ifdef CONFIG_LG_SNAPSHOT_BOOT
	if (reserve_boot_memory == 0)
#endif
	{
		if(bstoplogo == false && (logo_dalay_init_bool_1 == 0) )
		{
			bstoplogo = true;
			printk("\033[1;45m ================== [gdmalogo][%s][%s][%d] ================== \033[0m\n",__FILE__, __FUNCTION__, __LINE__);

			#ifndef ENABLE_BRINGUP_WORKAROUND
            #if IS_ENABLED(CONFIG_RTK_KDRV_PM)
			do_dvr_reclaim(2);
            #endif

			#endif//
			
		}
	}

	if(logo_do_dvr_reclaim)
	{
		logo_do_dvr_reclaim = 0;
		bstoplogo = true;

		#ifndef ENABLE_BRINGUP_WORKAROUND
        #if IS_ENABLED(CONFIG_RTK_KDRV_PM)
		do_dvr_reclaim(2);
        #endif

		#endif//ENABLE_BRINGUP_WORKAROUND
		
		printk("\033[1;45m ================== [gdmalogo logodelayfn do_dvr_reclaim][%s][%s][%d] ================== \033[0m\n",__FILE__, __FUNCTION__, __LINE__);
    	}
#else
#if defined(CONFIG_ARM64)
	unsigned long osdwi = (unsigned long)rtd_inl(GDMA_OSD1_WI_reg);
#else
	unsigned int osdwi = (unsigned int)rtd_inl(GDMA_OSD1_WI_reg);
#endif
	static bool needlogo =true;
	if(osdwi && (osdwi > gdma_dev_addr) && needlogo) {
		needlogo = false;

#if defined(CONFIG_ARM64)
		printk(KERN_DEBUG"[GDMA]call reclaim, used=%lx  ..\n",osdwi);
#else
		printk(KERN_DEBUG"[GDMA]call reclaim, used=%08x  ..\n",osdwi);
#endif
		#ifndef ENABLE_BRINGUP_WORKAROUND
        #if IS_ENABLED(CONFIG_RTK_KDRV_PM)
		do_dvr_reclaim(1);
		do_dvr_reclaim(2);
        #endif
		#endif// ENABLE_BRINGUP_WORKAROUND
		
	}
#endif

#if IS_ENABLED(CONFIG_REALTEK_PCBMGR)
		pr_debug("[GDMA] panel vflip=%d \n", panel_parameter->iVFLIP);
		if (panel_parameter->iVFLIP)	{
			/*	SFG_CTRL_0 */
			rtd_outl(SFG_SFG_CTRL_0_reg, rtd_inl(SFG_SFG_CTRL_0_reg) | SFG_SFG_CTRL_0_h_flip_en(1));
			/*if bootcode has already enable vfip bit ,
			  we don't have to reset it again */
			if(GDMA_OSD1_CTRL_get_rotate(rtd_inl(GDMA_OSD1_CTRL_reg)) ==0) {
			GDMA_ConfigVFlip(1);
		}
			gdma->ctrl.enableVFlip = 1;
		}
#endif

	if(gdrop_counter > 0){
		gdrop_counter--;

		GDMA_PRINT(7, "  %s %d\n", __FUNCTION__, __LINE__ ); 
		
		return GDMA_SUCCESS;
	}
	if(gdrop_counter == -1){

		GDMA_PRINT(7, "  %s %d\n", __FUNCTION__, __LINE__ );
		
		return GDMA_SUCCESS;
	}
	if(enable_stop_update) {

		GDMA_PRINT(7, " %s %d\n", __FUNCTION__, __LINE__ );
		return GDMA_SUCCESS;
	}

	if(gosdpattern == 1 && data != NULL && data->layer[0].normal[0].address != gosdpattern_phy){
		GDMA_PRINT(7, " %s %d\n", __FUNCTION__, __LINE__ );
		
		return GDMA_SUCCESS;
	}else if(gosdpattern == 0){
		/*for restore last info after disable osd pattern
		 need to save osd1, osd2, osd3 ?*/
		memcpy(&glastinfo, data, sizeof(PICTURE_LAYERS_OBJECT));
	}
	/* protection to unreasonable parameters */
	if (data != NULL && data->layer_num) {
		if (data->layer[0].normal_num) {
			if (data->layer[0].normal[0].dst_width < data->layer[0].normal[0].width) {
				printk(KERN_EMERG"%s, wrong width size, src=%d, dst=%d\n", __FUNCTION__, data->layer[0].normal[0].width, data->layer[0].normal[0].dst_width);
				return GDMA_FAIL;
			}
			if (data->layer[0].normal[0].dst_height < data->layer[0].normal[0].height) {
				printk(KERN_EMERG"%s, wrong height size, src=%d, dst=%d\n", __FUNCTION__, data->layer[0].normal[0].height, data->layer[0].normal[0].dst_height);
				return GDMA_FAIL;
			}
		}
	}

	/*  DTG timing maybe changed at run-time, so always read DTG timing to decide the panel resolution */
	getDispSize(&disp_res);
	gdma->dispWin[0].width = disp_res.width;
	gdma->dispWin[0].height = disp_res.height;
	GDMA_PRINT(7, "%s, Panel [%d,%d]\n", __FUNCTION__, disp_res.width, disp_res.height);
	GDMA_PRINT(7, "%s, data->layer_num=%d\n", __FUNCTION__, data->layer_num);

	/*  take care DISPD_GDMA_CLK_SEL (0xB8000208[14]) will use 4K2K or not */
	/*  when 4K2K must select the higher frequency */
	if (disp_res.width >= FOURK_W && disp_res.height >= TWOK_H) {
		/*  TODO: pick higher freq. */
		/*
		 * 1: gdma_clk = dispd-clock div 1
		 * 0: gdma_clk = busH_clk = sysH_clk
		 */
		/*  TODO: this setting move to bootcode stage */
	}

	down(&gdma->sem_receive);
	GDMA_ENTER_CRITICAL();

	/* avoid interrupt routing to SCPU disable within 100 frames */
	if (g_StartCheckFrameCnt) {
		g_StartCheckFrameCnt--;
		//printk(KERN_EMERG"%s, g_StartCheckFrameCnt=%d \n", __FUNCTION__, g_StartCheckFrameCnt);
		if ((rtd_inl(SYS_REG_INT_CTRL_SCPU_reg) & SYS_REG_INT_CTRL_SCPU_osd_int_scpu_routing_en_mask) == 0) {
			GDMA_PRINT(3,"%s, GDMA interrupt routing to SCPU disable ! \n", __FUNCTION__);
			rtd_outl(SYS_REG_INT_CTRL_SCPU_reg, SYS_REG_INT_CTRL_SCPU_osd_int_scpu_routing_en_mask | SYS_REG_INT_CTRL_SCPU_write_data(1));
		}
	}

	for (ilayer = 0; ilayer < data->layer_num; ilayer++) {

		/*  search the empty work space */
		for (idx = 0; idx < GDMA_MAX_PIC_Q_SIZE; idx++) {
			if (sGdmaReceiveWork[idx].used == 0) {
				sGdmaReceiveWork[idx].used = 1;
				break;
			}
		}
		emptyIdx = idx;
		GDMA_PRINT(3, "%s, emptyIdx=%d\n", __FUNCTION__, emptyIdx);

		if (emptyIdx == GDMA_MAX_PIC_Q_SIZE) {
			GDMA_PRINT(3, "%s, Warning! GDMA workqueue is full \n", __FUNCTION__);
			GDMA_EXIT_CRITICAL();
			up(&gdma->sem_receive);
			return COMP_BUF_WORK_FULL;
		}

		com_picObj = &data->layer[ilayer];
		if (com_picObj->normal_num > 0)
			picObj = &com_picObj->normal[0];		/* just one normal OSD for one online path  */
		else if (com_picObj->fbdc_num > 0)
			picObj = &com_picObj->fbdc[0];
		else {
			GDMA_PRINT(3, "%s, PICTURE_LAYERS_OBJECT's COMPOSE_PICTURE_OBJECT has NO layer \n", __FUNCTION__);
			GDMA_EXIT_CRITICAL();
			up(&gdma->sem_receive);
			return GDMA_FAIL;
		}

		/*  in this function, one layer has only one picture in certain OSD HW */
		if (com_picObj->normal_num > 0 && com_picObj->fbdc_num > 0)
			GDMA_PRINT(3,"%s, PICTURE_LAYERS_OBJECT's COMPOSE_PICTURE_OBJECT layer has something wrong \n", __FUNCTION__);

		if (picObj->layer_used == 1) {
			if (picObj->plane == VO_GRAPHIC_OSD1)
				disPlane = GDMA_PLANE_OSD1;
			else if (picObj->plane == VO_GRAPHIC_OSD2)
				disPlane = GDMA_PLANE_OSD2;
			else if (picObj->plane == VO_GRAPHIC_OSD3)
				disPlane = GDMA_PLANE_OSD3;

			onlinePlane[onlineMaxNum++] = disPlane;
			gdma->dispWin[disPlane].width = disp_res.width;
			gdma->dispWin[disPlane].height = disp_res.height;
			GDMA_PRINT(6, "%s, gdma->dispWin[%d].width=%d, height=%d \n", __FUNCTION__, disPlane, gdma->dispWin[disPlane].width, gdma->dispWin[disPlane].height);

			if (gdma->picQrd[disPlane] == ((gdma->picQwr[disPlane] + 1) & (GDMA_MAX_PIC_Q_SIZE - 1))) {
				GDMA_PRINT(2, "GDMA: pic Queue %d fullness r %d,w %d,c %d\n", disPlane, gdma->picQrd[disPlane], gdma->picQwr[disPlane], gdma->curPic[disPlane]);
				sGdmaReceiveWork[emptyIdx].used = 0;	/* free work */

				/* drop frames and go ahead for in-stop on firse use
				*  SE maybe block by debug thread, then sends the queued picture very quickly when system not busy
				*/
				GDMA_PRINT(2,  "GDMA: disPlane=%d pid=%d\n", disPlane, current->pid);
				for (idx = 0; idx < GDMA_MAX_PIC_Q_SIZE; idx++)
					sGdmaReceiveWork[idx].used = 0;
				gdma->picQwr[disPlane] = gdma->curPic[disPlane];
				gdma->picQrd[disPlane] = ((gdma->curPic[disPlane] - 2) & (GDMA_MAX_PIC_Q_SIZE - 1));

				/* return -99; */
				/* continue; */
			} else if (gdma->ctrl.zeroBuffer) {
				GDMA_PRINT(2, "GDMA: zeroBuffer is 1\n");
				GDMA_EXIT_CRITICAL();
				sGdmaReceiveWork[emptyIdx].used = 0;	/* free work */
				up(&gdma->sem_receive);
				return GDMA_SUCCESS;
			}
			DBG_PRINT(KERN_EMERG"R\n");

			pic = gdma->pic[disPlane] + (idxQwr = (gdma->picQwr[disPlane] + 1) & (GDMA_MAX_PIC_Q_SIZE - 1));

			memset(pic, 0, sizeof(GDMA_PIC_DATA));

			pic->seg_num = picObj->seg_num;
			pic->current_comp_idx = picObj->current_comp_idx;
			pic->cookie = data->cookie;
			pic->show = picObj->show;
			pic->syncInfo = picObj->syncInfo;
			pic->workqueueDone = 0;
            //RTK mark for netfix
			if(enable_osd1osd3mixerorder)
			{
				if (picObj->plane == VO_GRAPHIC_OSD1)
					pic->onlineOrder = C1;
				else if (picObj->plane == VO_GRAPHIC_OSD2)
					pic->onlineOrder = C0;
				else
					pic->onlineOrder = com_picObj->onlineOrder;
			}
			else
			pic->onlineOrder = com_picObj->onlineOrder;

			pic->clear_x.value = picObj->clear_x.value;
			pic->clear_y.value = picObj->clear_y.value;
			*(char *)&pic->status = 0;
			pic->repeatCnt = 0;
			pic->context   = picObj->context;
			pic->workqueueIdx = emptyIdx;
			pic->isFromOffline = picObj->isFromOffline;
			pic->scale_factor = picObj->scale_factor;

			/*  mixer's plane alpha setting, only for normal OSD */
			/*if (com_picObj->normal_num > 0) */{
				pic->plane_ar.value = picObj->plane_ar.value;
				pic->plane_gb.value = picObj->plane_gb.value;
			}

			win = &pic->OSDwin;
			win3D = &pic->OSDwin3D;

			win->nxtAddr.addr     = 0;
			win->nxtAddr.last     = 1;

			/* win->winXY.x          = (gdma->f_box_mode && gdma->box_dst_height == VIDEO_USERDEFINED)? picObj->x: picObj->dst_x; //picObj->dst_x; //picObj->x; */
			/* win->winXY.y          = (gdma->f_box_mode && gdma->box_dst_height == VIDEO_USERDEFINED)? picObj->y: picObj->dst_y; //picObj->dst_y; //picObj->y; */

			/*  canvas X, Y always set the X, Y from picture */
			/*  destination x, y, width, height are for final resolution and position on panel */
			win->winXY.x          = picObj->x;
			win->winXY.y          = picObj->y;

			if(change_OSD_mode == GDMA_SPLIT_MODE_1){
				//picObj->width     = (picObj->width/3)*4;
				picObj->format = VO_OSD_COLOR_FORMAT_RGB888_LITTLE;
				//picObj->pitch  = picObj->width*3;
				picObj->dst_height = GDMA_SPLIT_MAX_Y+GDMA_SPLIT_EXT_Y;
				picObj->alpha     = 0xff;
				win->winWH.width  = (picObj->width/3)*4;
				win->pitch            = win->winWH.width*3;
				picObj->dst_width = win->winWH.width;
				picObj->height = GDMA_SPLIT_MAX_Y+GDMA_SPLIT_EXT_Y;
			}else if(change_OSD_mode == GDMA_SPLIT_MODE_2){
				if((picObj->width == 512 || picObj->width == 960) && picObj->height==2160){
					if (g_esticker_info.cur_position == 150)
						g_esticker_info.cur_position = 300;

					GDMA_ConfigEstickerPosition(1);

					if (g_esticker_info.change > 0) {
#if USE_RLINK_INTERFACE
						//GDMA_estiker_osd_enable(0);
						GDMA_estiker_osd_x_p(g_esticker_info.cur_position);
						//GDMA_estiker_osd_y_p(778);
						//GDMA_estiker_enable(1);
						//GDMA_estiker_osd_enable(1);
						g_esticker_info.change--;
#endif
					}
					picObj->width *= 2;
					picObj->height /=2;
					picObj->dst_x=0;
					picObj->dst_height = picObj->height;
					win->winWH.width  = (picObj->width);
					if(picObj->format == VO_OSD_COLOR_FORMAT_ARGB8888_LITTLE 
							|| picObj->format == VO_OSD_COLOR_FORMAT_RGBA8888)
						win->pitch            = win->winWH.width*4;
					else
						win->pitch            = win->winWH.width*2;					
					picObj->dst_width = win->winWH.width;
				}else{
					picObj->dst_height = picObj->height;
					win->winWH.width  = (picObj->width);
					if(picObj->format == VO_OSD_COLOR_FORMAT_ARGB8888_LITTLE 
							|| picObj->format == VO_OSD_COLOR_FORMAT_RGBA8888)
						win->pitch            = win->winWH.width*4;
					else
						win->pitch            = win->winWH.width*2;					
					picObj->dst_width = win->winWH.width;
				}
			}else if(change_OSD_mode == GDMA_SPLIT_MODE_3){
                picObj->dst_height = picObj->height*2;
				win->winWH.width  = (picObj->width/2);
				if(picObj->format == VO_OSD_COLOR_FORMAT_ARGB8888_LITTLE 
						|| picObj->format == VO_OSD_COLOR_FORMAT_RGBA8888)
					win->pitch            = win->winWH.width*4;
				else
					win->pitch            = win->winWH.width*2;
				picObj->dst_width = win->winWH.width;
				picObj->height = picObj->height*2;
			}else{
				win->winWH.width      = picObj->width;
				win->pitch            = picObj->pitch;
			}
#if 0
			if((change_OSD_mode==GDMA_SPLIT_MODE_0) && (picObj->height >= GDMA_SPLIT_MAX_Y))
			{
				if(picObj->format == VO_OSD_COLOR_FORMAT_RGB888_LITTLE)
					picObj->format = VO_OSD_COLOR_FORMAT_ARGB8888_LITTLE;
				else if(picObj->format == VO_OSD_COLOR_FORMAT_RGB888)
					picObj->format = VO_OSD_COLOR_FORMAT_RGBA8888;

				if(cp_src != 0 )
					win->winWH.height     = GDMA_SPLIT_MAX_Y+GDMA_SPLIT_EXT_Y;
				else
					win->winWH.height     = GDMA_SPLIT_MAX_Y;
				picObj->dst_height = disp_res.height;
				picObj->dst_width = disp_res.width;
				picObj->alpha     = 0x0;
				picObj->height = GDMA_SPLIT_MAX_Y;
			}else{
#endif
				win->winWH.height     = picObj->height;
#if 0
			}
#endif
            
#if ESTICKER_778
			if ((change_OSD_mode==GDMA_SPLIT_MODE_0) && (picObj->width==960)) {
#else
			if ((change_OSD_mode==GDMA_SPLIT_MODE_0) && (picObj->width==512)) {
#endif
				GDMA_ConfigEstickerPosition(0);

				picObj->dst_x = g_esticker_info.cur_position;
				picObj->height=2160;
				win->winWH.width = picObj->width;
				picObj->dst_height = picObj->height;
				win->pitch            = win->winWH.width*4;
				picObj->dst_width = win->winWH.width;
				win->winWH.height     = picObj->height;
			}
			if(gGDMA_Monitor_ctl.enable){
				picObj->dst_height = picObj->height;
				picObj->dst_width = picObj->width;
			}

			win->attr.extendMode  = 0;
			win->attr.rgbOrder    = picObj->rgb_order;/* VO_OSD_COLOR_RGB; */
			win->attr.objType     = 0;
			win->attr.type        = picObj->format & 0x1f;
			win->attr.littleEndian = picObj->format >> 5;
			win->attr.alphaEn     = (win->attr.alpha   = picObj->alpha) >  0 ? 1 : 0;

			//if (gdma->f_palette_update == TRUE)
				win->attr.keepPreCLUT = 0;
			//else
			//	win->attr.keepPreCLUT = 1;
			//gdma->f_palette_update = FALSE;
			win->attr.clut_fmt    = picObj->paletteformat;/* rgba format */
			if (win->attr.type < 3)
				win->CLUT_addr = virt_to_phys(gdma->OSD_CLUT[picObj->paletteIndex]);

			if (picObj->decompress) {
				win->objOffset.objXoffset = 0;
				win->objOffset.objYoffset = 0;
				gdma_config_line_buffer_afbc(FBC_ON);
			} else {
				gdma_config_line_buffer_afbc(FBC_OFF);
				//pr_debug(KERN_EMERG"GDMA, %s,picObj->dst_x=%d picObj->dst_y=%d \n", __FUNCTION__,picObj->dst_x,picObj->dst_y);
				//this for fix when cursor move out of bound of panel(0,0),need do offset.
					if(picObj->dst_x<0){
						win->objOffset.objXoffset = abs(picObj->dst_x);
						picObj->dst_x=0;
					}else{
						win->objOffset.objXoffset = picObj->x;
					}
					if(picObj->dst_y<0){
						win->objOffset.objYoffset = gdma->ctrl.enableVFlip ? 0 : abs(picObj->dst_y);
						picObj->dst_y=0;
					}else{
						win->objOffset.objYoffset = gdma->ctrl.enableVFlip ? 0 : picObj->y;
					}
			}

			win->dst_x = picObj->dst_x;
			win->dst_y = picObj->dst_y;
			// for OSD shift
			if (g_osdshift_ctrl.shift_h_enable && com_picObj->fbdc_num==0) {
				g_osdshift_ctrl.para_h_N = abs(g_osdshift_ctrl.h_shift_pixel) / 2;
				g_osdshift_ctrl.para_h_K = abs(g_osdshift_ctrl.h_shift_pixel) % 2;
				g_osdshift_ctrl.Canvas_H = win->winWH.width;
				if (g_osdshift_ctrl.h_shift_pixel >= 0) {
					/* shift right*/
					if(g_osdshift_ctrl.plane==picObj->plane) {
						win->winWH.width -= (g_osdshift_ctrl.para_h_N);
						/*win->winXY.x += (g_osdshift_ctrl.para_h_N+1);*/
					}else if(VO_GRAPHIC_OSD4==picObj->plane){
						/*osd_sta will add this value*/
						if((picObj->dst_x + g_osdshift_ctrl.h_shift_pixel) >= 3840) {
							win->dst_x = 0;
							picObj->dst_x = 0;
							win->attr.alpha = 0;
							win->attr.alphaEn = 1;
						} else {
							win->dst_x += g_osdshift_ctrl.h_shift_pixel;
							picObj->dst_x = win->dst_x;
						}
					}
				}else{
					/*shift left*/
					if(g_osdshift_ctrl.plane==picObj->plane) {
						win->winWH.width -= (g_osdshift_ctrl.para_h_N);
						win->objOffset.objXoffset += g_osdshift_ctrl.para_h_N;
					}else if(VO_GRAPHIC_OSD4==picObj->plane) {
						gdam_lambda_cursor_shift_h(win, picObj);
					}
				}
			}

			if (g_osdshift_ctrl.shift_v_enable && com_picObj->fbdc_num==0) {
				g_osdshift_ctrl.para_v_N = abs(g_osdshift_ctrl.v_shift_pixel) / 2;
				g_osdshift_ctrl.para_v_K = abs(g_osdshift_ctrl.v_shift_pixel) % 2;
				g_osdshift_ctrl.Canvas_V = win->winWH.height;

				if (g_osdshift_ctrl.v_shift_pixel >= 0) {
					/*shift down*/
					if(g_osdshift_ctrl.plane==picObj->plane) {
						win->winWH.height -= (g_osdshift_ctrl.para_v_N+1);
						/*win->winXY.y += (g_osdshift_ctrl.para_v_N+1);*/
					}else if(VO_GRAPHIC_OSD4==picObj->plane) {
						/*use adjust osd_sta instead of here*/
						if((picObj->dst_y + g_osdshift_ctrl.v_shift_pixel) >= 2160) {
							win->dst_y = 0;
							picObj->dst_y = 0;
							win->attr.alpha = 0;
							win->attr.alphaEn = 1;
						}else {
							win->dst_y += g_osdshift_ctrl.v_shift_pixel;
							picObj->dst_y = win->dst_y;
						}
					}
				}else {
					/*shift up*/
					if(g_osdshift_ctrl.plane==picObj->plane) {
						win->winWH.height -= (g_osdshift_ctrl.para_v_N);
						win->objOffset.objYoffset += (g_osdshift_ctrl.para_v_N);
					}else if(VO_GRAPHIC_OSD4==picObj->plane) {
						gdam_lambda_cursor_shift_v(win, picObj);
					}
				}
			}

			/*  destination width/height set as the panel resolution */
			if (picObj->dst_width == 0)     /*  0 means scaling to panel resolution */
				win->dst_width = disp_res.width;
			else
				win->dst_width = picObj->dst_width;
			if (picObj->dst_height == 0)    /*  0 means scaling to panel resolution */
				win->dst_height = disp_res.height;
			else
				win->dst_height = picObj->dst_height;

			win->attr.compress = picObj->decompress;
			win->used = 1;

			if (win->attr.compress) {
				win->colorKey.keyEn   = picObj->key_en;
				win->colorKey.key     = picObj->colorkey;
			} else {
				win->colorKey.keyEn   = (picObj->colorkey != -1);
				win->colorKey.key     = picObj->colorkey;
			}


			saveBufAddr = win->top_addr         = picObj->address + (gdma->ctrl.enableVFlip ? picObj->y + picObj->height - 1 : 0) * picObj->pitch;

			if(buf_phy_addr[0] == 0){
			    buf_phy_addr[0] = saveBufAddr;
			}else if(buf_phy_addr[1] == 0){
				buf_phy_addr[1] = saveBufAddr;				
			}else if(buf_phy_addr[2] == 0){
				buf_phy_addr[2] = saveBufAddr; 
			}
//for ANDROIDTV-305 test
			if(saveBufAddr >= (unsigned int)gdma_total_ddr_size){
				dump_stack();
				GDMA_PRINT(2, "%s, saveBufAddr=%x is over OSD_MAX_PHY_ADDR(%08x) \n", __FUNCTION__,saveBufAddr,(unsigned int)gdma_total_ddr_size);
				GDMA_EXIT_CRITICAL();
				up(&gdma->sem_receive);
				return GDMA_FAIL;
			}

			if (win->attr.compress && pic->isFromOffline) {
				win->attr.objType     =  (pic->seg_num > 0) ? 1 : 0;
				win->top_addr         = picObj->seg_addr[0];
				win->bot_addr         = picObj->seg_addr[1];
			}

			/*  3D mode configuration */
			if (gdma->ctrl.enable3D) {
				/*  3d mode test */
				/* picObj->picLayout = INBAND_CMD_GRAPHIC_SIDE_BY_SIDE; */
				/* picObj->picLayout = INBAND_CMD_GRAPHIC_TOP_AND_BOTTOM; */

				/*  1: PR mode */
				if (gdma->ctrl.enable3D_PR_SGMode) {
					if (picObj->picLayout == INBAND_CMD_GRAPHIC_TOP_AND_BOTTOM) {
						pr_debug(KERN_EMERG"GDMA, %s, Not support top-bottom in PR mode \n", __FUNCTION__);
					} else if (picObj->picLayout == INBAND_CMD_GRAPHIC_SIDE_BY_SIDE) {
						win->winWH.width = picObj->width >> 1;
						win->pitch >>= 1;
						win->objOffset.objXoffset += win3D->winWH.width;
					}
				} else {	/*  0: SG mode */
					memcpy(win3D, win, sizeof(GDMA_WIN));

					if (picObj->picLayout == INBAND_CMD_GRAPHIC_TOP_AND_BOTTOM) {
						win3D->winWH.height = win->winWH.height = picObj->height >> 1;
						if (gdma->ctrl.enableVFlip)
							win3D->top_addr -= (win3D->winWH.height * win3D->pitch);
						else
							win->objOffset.objYoffset += win3D->winWH.height;
					} else if (picObj->picLayout == INBAND_CMD_GRAPHIC_SIDE_BY_SIDE) {
						win3D->winWH.width = win->winWH.width = picObj->width >> 1;
						win->objOffset.objXoffset += win3D->winWH.width;
						win3D->pitch = win->pitch;
					}
				}
			}
#if defined(CONFIG_ARCH_RTK287O) || defined(CONFIG_ARCH_RTK288O) || defined(CONFIG_ARCH_RTK2875) || defined(CONFIG_ARCH_RTK2885M) || defined(CONFIG_ARCH_RTK6748)
#else
			win->fbdc_attr.pre_blend_c0_sel = 1;
			win->fbdc_attr.pre_blend_c1_sel = 2;
			win->fbdc_attr.pre_blend_c2_sel = 3;
#endif
			/*  TODO: FBDC information */
			win->attr.IMGcompress =  (com_picObj->fbdc_num > 0) ? 1 : 0;
			if (win->attr.IMGcompress) {
				win->attr.type = VO_OSD_COLOR_FORMAT_ARGB8888 & 0x1f;
				win->attr.littleEndian = VO_OSD_COLOR_FORMAT_ARGB8888 >> 5;
				win->attr.alphaEn = 0;		/* FBDC has alpha fucntion itself */
				win->attr.alpha = 0;		/* FBDC has alpha fucntion itself */
				win->CLUT_addr = 0;
				win->dst_x = win->dst_y = 0;
			}
			for (idx = 0; idx < com_picObj->fbdc_num; idx++) {
				if (idx < 3)
					picObj = &com_picObj->fbdc[idx];
				else {
					printk(KERN_EMERG"%s, too many FBDC layer into one OSD plane \n", __FUNCTION__);
					GDMA_EXIT_CRITICAL();
					up(&gdma->sem_receive);
					return GDMA_FAIL;
				}
#if (defined(CONFIG_ARCH_RTK287O) || defined(CONFIG_ARCH_RTK288O) || defined(CONFIG_ARCH_RTK2875) || defined(CONFIG_ARCH_RTK2885M)) || defined(CONFIG_ARCH_RTK6748)
				if (idx == 0) { 
					win->pitch = picObj->pitch;
					win->fbdc1_addr = picObj->address + ((picObj->width * picObj->height / 64) * 8); // 1 tile header = 64bit/64 pxl
					win->tfbc_format.tfbc_req_format = 0x0C; // ARGB8888
					win->tfbc_format.tfbc_req_lossy = picObj->compressed_ratio;
                                        win->tfbc_format.tfbc_argb_order = 0;
					win->objOffset.objYoffset = (unsigned short)((picObj->address&0xffff0000)>>16);
					win->objOffset.objXoffset = (unsigned short)(picObj->address&0x0000ffff);
				
					win->afbc_totaltile.x_total_tile = picObj->width;
					win->afbc_totaltile.y_total_tile = picObj->height;
					win->afbc_xy_pixel.x_pic_pixel = picObj->width;
					win->afbc_xy_pixel.y_pic_pixel = picObj->height;
					win->afbc_xy_sta.x_sta = picObj->x;
					win->afbc_xy_sta.y_sta = (gdma->ctrl.enableVFlip ? (picObj->height - 1) : picObj->y);
					if(gGDMA_Monitor_ctl.enable){
						win->afbc_xy_sta.x_sta = gGDMA_Monitor_ctl.input_x;
						win->afbc_xy_sta.y_sta =  gGDMA_Monitor_ctl.input_y;
						win->afbc_xy_pixel.x_pic_pixel = gGDMA_Monitor_ctl.input_w;
						win->afbc_xy_pixel.y_pic_pixel = gGDMA_Monitor_ctl.input_h;
						win->dst_x = gGDMA_Monitor_ctl.output_x;
						win->dst_y = gGDMA_Monitor_ctl.output_y;
					}
				}
#else
				if (idx == 0) {     /*  FBDC1 */
					win->fbdc_attr.pre_blend_en = 1;    /*  enable pre_blend */
					win->fbdc1_addr = picObj->address + (gdma->ctrl.enableVFlip ? picObj->y + picObj->height - 1 : 0) * picObj->pitch;
					win->fbdc_attr.fbdc_1st_en = 1;
					win->fbdc_attr.const_a_1st = ((win->fbdc_attr.alpha_1st = picObj->alpha) > 0) ? 1 : 0;
					win->fbdc_attr.plane_alpha_c0 = (picObj->plane_ar.plane_alpha_a != 0xff || picObj->plane_ar.plane_alpha_r != 0xff ||
													 picObj->plane_gb.plane_alpha_g != 0xff || picObj->plane_gb.plane_alpha_b != 0xff) ? 1 : 0;
					win->fbdc_plane_alpha_c2.plane_alpha_a = (picObj->plane_ar.plane_alpha_a > 255) ? 255 : picObj->plane_ar.plane_alpha_a;
					win->fbdc_plane_alpha_c2.plane_alpha_r = (picObj->plane_ar.plane_alpha_r > 255) ? 255 : picObj->plane_ar.plane_alpha_r;
					win->fbdc_plane_alpha_c2.plane_alpha_g = (picObj->plane_gb.plane_alpha_g > 255) ? 255 : picObj->plane_gb.plane_alpha_g;
					win->fbdc_plane_alpha_c2.plane_alpha_b = (picObj->plane_gb.plane_alpha_b > 255) ? 255 : picObj->plane_gb.plane_alpha_b;
					/*win->fbdc_attr.pre_blend_c0_sel = 1;*/
					win->fbdc_format.fbdc_fmt_1st = picObj->fbdc_format;
					win->fbdc1_totaltile.x_total_tile = (picObj->width / 16);

					win->fbdc1_totaltile.y_total_tile = (picObj->height / 4);

					win->fbdc1_xy_pixel.x_pic_pixel = picObj->width;
					win->fbdc1_xy_pixel.y_pic_pixel = picObj->height;
					win->fbdc1_xy_sta.x_sta = picObj->x;
					win->fbdc1_xy_sta.y_sta = (gdma->ctrl.enableVFlip ? (picObj->height-1) : picObj->y);
					win->fbdc1_preblend_xy_sta.x_sta = picObj->dst_x/picObj->scale_factor;
					win->fbdc1_preblend_xy_sta.y_sta = picObj->dst_y/picObj->scale_factor;
					// for OSD shift
					if (g_osdshift_ctrl.shift_h_enable && g_osdshift_ctrl.plane==picObj->plane) {
						win->fbdc1_xy_pixel.x_pic_pixel -= abs(g_osdshift_ctrl.h_shift_pixel);
						if (g_osdshift_ctrl.h_shift_pixel > 0)	// shift right
							win->fbdc1_preblend_xy_sta.x_sta += g_osdshift_ctrl.h_shift_pixel;
						else	// shift left
							win->fbdc1_xy_sta.x_sta += abs(g_osdshift_ctrl.h_shift_pixel);
					}
					if (g_osdshift_ctrl.shift_v_enable && g_osdshift_ctrl.plane==picObj->plane) {
						win->fbdc1_xy_pixel.y_pic_pixel -= abs(g_osdshift_ctrl.v_shift_pixel);
						if (g_osdshift_ctrl.v_shift_pixel > 0)	// shift down
							win->fbdc1_preblend_xy_sta.y_sta += g_osdshift_ctrl.v_shift_pixel;
						else	// shift up
							win->fbdc1_xy_sta.y_sta += abs(g_osdshift_ctrl.v_shift_pixel);
					}

					pr_debug(KERN_EMERG" [GDMA] FBDC1 addr = %08x\n ",win->fbdc1_addr);

				} else if (idx == 1) { /*  FBDC2 */
					win->fbdc2_addr = picObj->address + (gdma->ctrl.enableVFlip ? picObj->y + picObj->height - 1 : 0) * picObj->pitch;
					win->fbdc_attr.fbdc_2nd_en = 1;
					win->fbdc_attr.const_a_2nd = ((win->fbdc_attr.alpha_2nd = picObj->alpha) > 0) ? 1 : 0;
					win->fbdc_attr.plane_alpha_c1 = (picObj->plane_ar.plane_alpha_a != 0xff || picObj->plane_ar.plane_alpha_r != 0xff ||
													 picObj->plane_gb.plane_alpha_g != 0xff || picObj->plane_gb.plane_alpha_b != 0xff) ? 1 : 0;
					win->fbdc_plane_alpha_c1.plane_alpha_a = (picObj->plane_ar.plane_alpha_a > 255) ? 255 : picObj->plane_ar.plane_alpha_a;
					win->fbdc_plane_alpha_c1.plane_alpha_r = (picObj->plane_ar.plane_alpha_r > 255) ? 255 : picObj->plane_ar.plane_alpha_r;
					win->fbdc_plane_alpha_c1.plane_alpha_g = (picObj->plane_gb.plane_alpha_g > 255) ? 255 : picObj->plane_gb.plane_alpha_g;
					win->fbdc_plane_alpha_c1.plane_alpha_b = (picObj->plane_gb.plane_alpha_b > 255) ? 255 : picObj->plane_gb.plane_alpha_b;
					/*win->fbdc_attr.pre_blend_c1_sel = 2;*/
					win->fbdc_format.fbdc_fmt_2nd = picObj->fbdc_format;
					win->fbdc2_totaltile.x_total_tile = (picObj->width / 16);
					win->fbdc2_totaltile.y_total_tile = (picObj->height / 4);
					win->fbdc2_xy_pixel.x_pic_pixel = picObj->width;
					win->fbdc2_xy_pixel.y_pic_pixel = picObj->height;
					win->fbdc2_xy_sta.x_sta = picObj->x;
					win->fbdc2_xy_sta.y_sta = (gdma->ctrl.enableVFlip ? (picObj->height-1) : picObj->y);
					win->fbdc2_preblend_xy_sta.x_sta = picObj->dst_x/picObj->scale_factor;
					win->fbdc2_preblend_xy_sta.y_sta = picObj->dst_y/picObj->scale_factor;
					// for OSD shift
					if (g_osdshift_ctrl.shift_h_enable && g_osdshift_ctrl.plane==picObj->plane) {
						win->fbdc2_xy_pixel.x_pic_pixel -= abs(g_osdshift_ctrl.h_shift_pixel);
						if (g_osdshift_ctrl.h_shift_pixel > 0)	// shift right
							win->fbdc2_preblend_xy_sta.x_sta += g_osdshift_ctrl.h_shift_pixel;
						else	// shift left
							win->fbdc2_xy_sta.x_sta += abs(g_osdshift_ctrl.h_shift_pixel);
					}
					if (g_osdshift_ctrl.shift_v_enable && g_osdshift_ctrl.plane==picObj->plane) {
						win->fbdc2_xy_pixel.y_pic_pixel -= abs(g_osdshift_ctrl.v_shift_pixel);
						if (g_osdshift_ctrl.v_shift_pixel > 0)	// shift down
							win->fbdc2_preblend_xy_sta.y_sta += g_osdshift_ctrl.v_shift_pixel;
						else	// shift up
							win->fbdc2_xy_sta.y_sta += abs(g_osdshift_ctrl.v_shift_pixel);
					}

					pr_debug(KERN_EMERG" [GDMA] FBDC2 addr = %08x\n ",win->fbdc2_addr);
				} else if (idx == 2) { /*  FBDC3 */
					win->fbdc3_addr = picObj->address + (gdma->ctrl.enableVFlip ? picObj->y + picObj->height - 1 : 0) * picObj->pitch;
					win->fbdc_attr.fbdc_3rd_en = 1;
					win->fbdc_attr.const_a_3rd = ((win->fbdc_format.alpha_3rd = picObj->alpha) > 0) ? 1 : 0;
					win->fbdc_attr.plane_alpha_c2 = (picObj->plane_ar.plane_alpha_a != 0xff || picObj->plane_ar.plane_alpha_r != 0xff ||
													 picObj->plane_gb.plane_alpha_g != 0xff || picObj->plane_gb.plane_alpha_b != 0xff) ? 1 : 0;
					win->fbdc_plane_alpha_c0.plane_alpha_a = (picObj->plane_ar.plane_alpha_a > 255) ? 255 : picObj->plane_ar.plane_alpha_a;
					win->fbdc_plane_alpha_c0.plane_alpha_r = (picObj->plane_ar.plane_alpha_r > 255) ? 255 : picObj->plane_ar.plane_alpha_r;
					win->fbdc_plane_alpha_c0.plane_alpha_g = (picObj->plane_gb.plane_alpha_g > 255) ? 255 : picObj->plane_gb.plane_alpha_g;
					win->fbdc_plane_alpha_c0.plane_alpha_b = (picObj->plane_gb.plane_alpha_b > 255) ? 255 : picObj->plane_gb.plane_alpha_b;
					/*win->fbdc_attr.pre_blend_c2_sel = 3;*/
					win->fbdc_format.fbdc_fmt_3rd = picObj->fbdc_format;
					win->fbdc3_totaltile.x_total_tile = (picObj->width / 16);
					win->fbdc3_totaltile.y_total_tile = (picObj->height / 4);
					win->fbdc3_xy_pixel.x_pic_pixel = picObj->width;
					win->fbdc3_xy_pixel.y_pic_pixel = picObj->height;
					win->fbdc3_xy_sta.x_sta = picObj->x;
					win->fbdc3_xy_sta.y_sta = (gdma->ctrl.enableVFlip ? (picObj->height-1) : picObj->y);
					win->fbdc3_preblend_xy_sta.x_sta = picObj->dst_x/picObj->scale_factor;
					win->fbdc3_preblend_xy_sta.y_sta = picObj->dst_y/picObj->scale_factor;

					// for OSD shift
					if (g_osdshift_ctrl.shift_h_enable && g_osdshift_ctrl.plane==picObj->plane) {
						win->fbdc3_xy_pixel.x_pic_pixel -= abs(g_osdshift_ctrl.h_shift_pixel);
						if (g_osdshift_ctrl.h_shift_pixel > 0)	// shift right
							win->fbdc3_preblend_xy_sta.x_sta += g_osdshift_ctrl.h_shift_pixel;
						else	// shift left
							win->fbdc3_xy_sta.x_sta += abs(g_osdshift_ctrl.h_shift_pixel);
					}
					if (g_osdshift_ctrl.shift_v_enable && g_osdshift_ctrl.plane==picObj->plane) {
						win->fbdc3_xy_pixel.y_pic_pixel -= abs(g_osdshift_ctrl.v_shift_pixel);
						if (g_osdshift_ctrl.v_shift_pixel > 0)	// shift down
							win->fbdc3_preblend_xy_sta.y_sta += g_osdshift_ctrl.v_shift_pixel;
						else	// shift up
							win->fbdc3_xy_sta.y_sta += abs(g_osdshift_ctrl.v_shift_pixel);
					}

					pr_debug(KERN_EMERG" [GDMA] FBDC3 addr = %08x\n ",win->fbdc3_addr);
				}
#endif
				/* calculate the canvas size */
				win->winWH.width      = ((picObj->width+picObj->dst_x) > win->winWH.width) ? (picObj->width+picObj->dst_x) : win->winWH.width;
				win->winWH.height     = ((picObj->height+picObj->dst_y) > win->winWH.height) ? (picObj->height+picObj->dst_y) : win->winWH.height;
			}
#if defined(CONFIG_ARCH_RTK287O) || defined(CONFIG_ARCH_RTK288O) || defined(CONFIG_ARCH_RTK2875) || defined(CONFIG_ARCH_RTK2885M) || defined(CONFIG_ARCH_RTK6748)
#else
			/*  k blend factor for FBDC */
			win->fbdc_blend1st.k1_blend = 0xc;
			win->fbdc_blend1st.k2_blend = 0x3;
			win->fbdc_blend1st.k3_blend = 0x2d;
			win->fbdc_blend1st.k4_blend = 0x3;
			win->fbdc_blend2nd.k1_blend = 0xc;
			win->fbdc_blend2nd.k2_blend = 0x3;
			win->fbdc_blend2nd.k3_blend = 0x2d;
			win->fbdc_blend2nd.k4_blend = 0x3;
#endif
			/* check OSD over panel resolution */
			/*
			pr_debug(KERN_EMERG" [GDMA] picObj->dst_x=%d, picObj->dst_width=%d, picObj->width=%d ! \n ", picObj->dst_x, picObj->dst_width, picObj->width);
			pr_debug(KERN_EMERG" [GDMA] picObj->dst_y=%d, picObj->dst_height=%d, picObj->height=%d ! \n ", picObj->dst_y, picObj->dst_height, picObj->height);
			*/
			if (((picObj->dst_x+picObj->dst_width) > disp_res.width)) {
				/* change image width in osd windows info.  */
				if (picObj->dst_width > picObj->width) {
#if 1
					win->winWH.width = (unsigned int)((disp_res.width - picObj->dst_x) / (picObj->dst_width / picObj->width));
#else
					S4AP
					win->winWH.width = (unsigned int)((disp_res.width - picObj->dst_x) /3*2);
#endif
					if (win->winWH.width < 16)		/* OSD_SR input width size minimal 16 */
						win->winWH.width = 16;
#if 1
					win->dst_width = picObj->dst_width = (win->winWH.width * (picObj->dst_width / picObj->width));
#else
					win->dst_width = picObj->dst_width = (unsigned int)(win->winWH.width *3/2); //* (picObj->dst_width / picObj->width));
#endif
					if((disp_res.width - picObj->dst_x - win->dst_width) >=1) {
						win->dst_width = picObj->dst_width +=(disp_res.width - picObj->dst_x - win->dst_width);
					}

					if ((picObj->dst_x+picObj->dst_width) > disp_res.width)
						win->dst_x = picObj->dst_x = picObj->dst_x - ((picObj->dst_x+picObj->dst_width)-disp_res.width);
				} else if (picObj->dst_width <= win->winWH.width)
					win->winWH.width = win->dst_width = picObj->dst_width = disp_res.width - picObj->dst_x;

				/*pr_debug(KERN_EMERG"[GDMA]--> win->dst_x=%d, win->dst_width=%d, win->winWH.width=%d ! \n ", win->dst_x, win->dst_width, win->winWH.width);*/
				//printk(KERN_ERR "[GDMA]--> win->dst_x=%d, win->dst_width=%d, win->winWH.width=%d ! \n ", win->dst_x, win->dst_width, win->winWH.width);
			}
			if (((picObj->dst_y+picObj->dst_height) > disp_res.height)) {
				/* change image height in osd windows info.  */
				if (picObj->dst_height > picObj->height) {
#if 1
					win->winWH.height = ((disp_res.height - picObj->dst_y) / (picObj->dst_height / picObj->height));
#else
					win->winWH.height = ((disp_res.height - picObj->dst_y) /3*2); // (picObj->dst_height / picObj->height));
#endif
					if (win->winWH.height < 4)		/* OSD_SR input height size minimal 4 */
						win->winWH.height = 4;
#if 1
					win->dst_height = picObj->dst_height = (win->winWH.height * (picObj->dst_height / picObj->height));
#else
					win->dst_height = picObj->dst_height = (win->winWH.height *3/2);//* (picObj->dst_height / picObj->height));
#endif
					if((disp_res.height - picObj->dst_y - win->dst_height) >=1){
						win->dst_height = picObj->dst_height+=(disp_res.height - picObj->dst_y - win->dst_height);
					}
					picObj->dst_height = win->dst_height;
				} else if (picObj->dst_height <= win->winWH.height)
					win->winWH.height = win->dst_height = picObj->dst_height = disp_res.height - picObj->dst_y;

				//printk(KERN_ERR "[GDMA]--> win->dst_y=%d, win->dst_height=%d, win->winWH.height=%d ! \n ", win->dst_y, win->dst_height, win->winWH.height);
			}

#if !defined(CONFIG_ARM64)
	dsb();
#endif
			GDMA_PRINT(7, "%s, disPlane=%d win->attr.compress=%d \n", __FUNCTION__, disPlane, win->attr.compress);
			GDMA_PRINT(7, "%s, pic->seg_num = %d, pic->context = %d\n", __FUNCTION__, pic->seg_num, pic->context);
			GDMA_PRINT(7, "%s, win->winWH.width = %d, win->winWH.height= %d\n", __FUNCTION__, win->winWH.width, win->winWH.height);
			GDMA_PRINT(7, "%s, win->dst_width = %d, win->dst_height= %d\n", __FUNCTION__, win->dst_width, win->dst_height);
			GDMA_PRINT(7, "%s, win->pitch(fbdc3 addr) = %d,  win->attr.compress = %d\n", __FUNCTION__, win->pitch, win->attr.compress);
			GDMA_PRINT(7,"%s, win->top_addr(fbdc1 addr) = 0x%x,  win->bot_addr(fbdc2 addr) = 0x%x\n", __FUNCTION__, win->top_addr, win->bot_addr);
			GDMA_PRINT(7,"%s, picObj->alpha=0x%x, pic->show = %d\n", __FUNCTION__, picObj->alpha, pic->show);
			GDMA_PRINT(7, "%s, pic->syncstamp=%lld, matchnum=%d \n", __FUNCTION__, pic->syncInfo.syncstamp, pic->syncInfo.matchNum);
            //for OSD HSR feature
			if(disp_res.height == 1080 && win->winWH.height==540){
				picObj->height = 1080;
				win->winWH.height = 1080;
			}
			gdma->picCount[disPlane]++;
			gdma->picQwr[disPlane] = idxQwr;
			GDMA_PRINT(7,"%s, gdma->picQwr[%d]=%d \n", __FUNCTION__, disPlane, gdma->picQwr[disPlane]);

			/*  push work to workqueue */
			sGdmaReceiveWork[emptyIdx].disPlane = disPlane;
			sGdmaReceiveWork[emptyIdx].picQwr = idxQwr;
			/*
			*	Does NOT use workqueue mechanism to avoid system block sometimes
			*	driver calculates the register value immediately
			*/
			GDMA_PreUpdate(&(sGdmaReceiveWork[emptyIdx].GdmaReceiveWork));

#ifdef TRIPLE_BUFFER_SEMAPHORE
			gSemaphore[disPlane]--;
			if (gSemaphore[disPlane] < 0) {
				pr_debug(KERN_EMERG"gdma : gSemaphore[%d] %d\n", disPlane, gSemaphore[disPlane]);
				gSemaphore[disPlane] = 0;
			}

			if (gSemaphore[disPlane] == 0)
				if (!gdma->pendingRPC[disPlane])
					gdma->pendingRPC[disPlane] = 1;
#endif
			if (rtd_inl(SYS_REG_INT_CTRL_SCPU_reg) & SYS_REG_INT_CTRL_SCPU_osd_int_scpu_routing_en_mask)    // for debug
				GDMA_Update(gdma, 1);
			else {
				for (idx = 0; idx < GDMA_MAX_PIC_Q_SIZE; idx++)
					sGdmaReceiveWork[idx].used = 0;
				gdma->curPic[disPlane] = gdma->picQwr[disPlane];
				gdma->picQrd[disPlane] = ((gdma->curPic[disPlane] - 2) & (GDMA_MAX_PIC_Q_SIZE - 1));
				gdma->picCount[disPlane] = 0;
			}

		} else
			GDMA_PRINT(4,"%s, No layer in use \n", __FUNCTION__);
	}

	DBG_PRINT(KERN_EMERG"%s, layer loop done \n", __FUNCTION__);
	GDMA_EXIT_CRITICAL();
	up(&gdma->sem_receive);

#ifdef TRIPLE_BUFFER_SEMAPHORE
	DBG_PRINT(KERN_EMERG"%s, online updated picture MaxNum=%d  \n", __FUNCTION__, onlineMaxNum);

	/*
	*	when multi-layer come in GDMA_ReceivePicture, and certain layer has syncstamp
	*	In case of such condition, semaphore wait will take more time here
	*/
	for (idx = 0; idx < onlineMaxNum; idx++) {
		disPlane = onlinePlane[idx];
		//if (gSemaphore[disPlane] == 0) {
		//	if (!gdma->pendingRPC[disPlane]) {
		//		gdma->pendingRPC[disPlane] = 1;
				//GDMA_EXIT_CRITICAL();

				sema_ret = down_timeout(&gdma->updateSem[disPlane], msecs_to_jiffies(32));

				// to avoid the down_timeout is not accurate
				timeout = jiffies + (HZ/50);	/* 20 ms */
				while (rtd_inl(GDMA_CTRL_reg) && (disPlane==GDMA_PLANE_OSD1)) {
					rtd_outl(GDMA_DBG_WI_INFO_reg, disPlane);
					if (saveBufAddr == rtd_inl(GDMA_DBG_OSD_WI_2_reg))
						break;
					if (time_before(jiffies, timeout) == 0)
						break;
					gdma_usleep(1000);
				}

				if (sema_ret == -ETIME) {
					gdma->pendingRPC[disPlane] = 0;
					gSemaphore[disPlane]  = gSEMAPHORE_MAX;
				}

				if(disPlane==GDMA_PLANE_OSD1)
					gdma_to_dump();


		//	}
		//}
	}
	//GDMA_EXIT_CRITICAL();

#if 0	/* vsync checker */
	/* check now osd physical address */
	unsigned int phyaddr;
	static unsigned int saveaddr = 0;
	volatile unsigned int *ptr;
	volatile unsigned int addr_tmp;
	addr_tmp = phys_to_virt(rtd_inl(GDMA_OSD1_WI_reg));
	ptr = (unsigned int *)addr_tmp;
	ptr += 6;

	/*Warning(KERN_EMERG"physical 0x%x \n", *ptr); */

	phyaddr = win->top_addr;
	if (phyaddr != *ptr)	/* check vsync wait */
		pr_debug(KERN_EMERG"rec 0x%x osd_hw 0x%x \n", phyaddr, *ptr);

	if (disPlane == GDMA_PLANE_OSD1) {
		if (saveaddr == 0)
			saveaddr = phyaddr;
		else {
			if (saveaddr == phyaddr)	/* check buffer swapping */
				pr_debug(KERN_EMERG"ERROR: the same with previous picture. save & phy=0x%x \n", saveaddr);
			else
				saveaddr = phyaddr;
		}
	}
#endif
	//up(&gdma->sem_receive);
#else
	GDMA_EXIT_CRITICAL();
	//up(&gdma->sem_receive);
#endif
	if(gDebugGDMA_dump_count>0)
	{
		//GDMA_DUMP_ReceivePicture(gDebugGDMA_dump_count);
		//gDebugGDMA_dump_count --;
	}

	GDMA_PRINT(7, "%s, finish \n", __FUNCTION__);
	return GDMA_SUCCESS;
}


#if IS_ENABLED(CONFIG_DRM_REALTEK)

int GDMA_ReceivePictureNoBlock(PICTURE_LAYERS_OBJECT *data)
{
	gdma_dev *gdma = &gdma_devices[0];
	GDMA_PIC_DATA *pic = NULL;
	GDMA_WIN *win = NULL;
	GDMA_WIN *win3D = NULL;
	COMPOSE_PICTURE_OBJECT *com_picObj = NULL;
	int idx, ilayer, emptyIdx = 0, idxQwr = 0;
	GDMA_PICTURE_OBJECT *picObj = NULL;
	GDMA_DISPLAY_PLANE disPlane = GDMA_PLANE_OSD1;
	VO_RECTANGLE disp_res;
	int onlinePlane[GDMA_PLANE_MAXNUM], onlineMaxNum = 0;

#if IS_ENABLED(CONFIG_REALTEK_PCBMGR)
	PANEL_CONFIG_PARAMETER *panel_parameter = (PANEL_CONFIG_PARAMETER *)&platform_info.panel_parameter;
	if (panel_parameter->iVFLIP){
		/*	SFG_CTRL_0 */
		rtd_outl(SFG_SFG_CTRL_0_reg, rtd_inl(SFG_SFG_CTRL_0_reg) | SFG_SFG_CTRL_0_h_flip_en(1));
		/*if bootcode has already enable vfip bit ,
		  we don't have to reset it again */
		if(GDMA_OSD1_CTRL_get_rotate(rtd_inl(GDMA_OSD1_CTRL_reg)) ==0) {
			GDMA_ConfigVFlip(1);
		}
		gdma->ctrl.enableVFlip = 1;
	}
#endif

	/* protection to unreasonable parameters */
	if (data != NULL && data->layer_num) {
		if (data->layer[0].normal_num) {
			if (data->layer[0].normal[0].dst_width < data->layer[0].normal[0].width) {
				printk(KERN_EMERG"%s, wrong width size, src=%d, dst=%d\n", __FUNCTION__, data->layer[0].normal[0].width, data->layer[0].normal[0].dst_width);
				return GDMA_FAIL;
			}
			if (data->layer[0].normal[0].dst_height < data->layer[0].normal[0].height) {
				printk(KERN_EMERG"%s, wrong height size, src=%d, dst=%d\n", __FUNCTION__, data->layer[0].normal[0].height, data->layer[0].normal[0].dst_height);
				return GDMA_FAIL;
			}
		}
	}

	/*  DTG timing maybe changed at run-time, so always read DTG timing to decide the panel resolution */
	getDispSize(&disp_res);
	gdma->dispWin[0].width = disp_res.width;
	gdma->dispWin[0].height = disp_res.height;

	down(&gdma->sem_receive);
	GDMA_ENTER_CRITICAL();

	/* avoid interrupt routing to SCPU disable within 100 frames */
	if (g_StartCheckFrameCnt) {
		g_StartCheckFrameCnt--;
		//printk(KERN_EMERG"%s, g_StartCheckFrameCnt=%d \n", __FUNCTION__, g_StartCheckFrameCnt);
		if ((rtd_inl(SYS_REG_INT_CTRL_SCPU_reg) & SYS_REG_INT_CTRL_SCPU_osd_int_scpu_routing_en_mask) == 0) {
			GDMA_PRINT(3,"%s, GDMA interrupt routing to SCPU disable ! \n", __FUNCTION__);
			rtd_outl(SYS_REG_INT_CTRL_SCPU_reg, SYS_REG_INT_CTRL_SCPU_osd_int_scpu_routing_en_mask | SYS_REG_INT_CTRL_SCPU_write_data(1));
		}
	}

	for (ilayer = 0; ilayer < data->layer_num; ilayer++) {

		/*  search the empty work space */
		for (idx = 0; idx < GDMA_MAX_PIC_Q_SIZE; idx++) {
			if (sGdmaReceiveWork[idx].used == 0) {
				sGdmaReceiveWork[idx].used = 1;
				break;
			}
		}
		emptyIdx = idx;

		if (emptyIdx == GDMA_MAX_PIC_Q_SIZE) {
			GDMA_PRINT(3, "%s, Warning! GDMA workqueue is full \n", __FUNCTION__);
			GDMA_EXIT_CRITICAL();
			up(&gdma->sem_receive);
			return COMP_BUF_WORK_FULL;
		}

		com_picObj = &data->layer[ilayer];
		if (com_picObj->normal_num > 0)
			picObj = &com_picObj->normal[0];		/* just one normal OSD for one online path  */
		else if (com_picObj->fbdc_num > 0)
			picObj = &com_picObj->fbdc[0];
		else {
			GDMA_PRINT(3, "%s, PICTURE_LAYERS_OBJECT's COMPOSE_PICTURE_OBJECT has NO layer \n", __FUNCTION__);
			GDMA_EXIT_CRITICAL();
			up(&gdma->sem_receive);
			return GDMA_FAIL;
		}

		/*  in this function, one layer has only one picture in certain OSD HW */
		if (com_picObj->normal_num > 0 && com_picObj->fbdc_num > 0)
			GDMA_PRINT(3,"%s, PICTURE_LAYERS_OBJECT's COMPOSE_PICTURE_OBJECT layer has something wrong \n", __FUNCTION__);

		if (picObj->layer_used == 1) {
			if (picObj->plane == VO_GRAPHIC_OSD1)
				disPlane = GDMA_PLANE_OSD1;
			else if (picObj->plane == VO_GRAPHIC_OSD2)
				disPlane = GDMA_PLANE_OSD2;
			else if (picObj->plane == VO_GRAPHIC_OSD3)
				disPlane = GDMA_PLANE_OSD3;

			onlinePlane[onlineMaxNum++] = disPlane;
			gdma->dispWin[disPlane].width = disp_res.width;
			gdma->dispWin[disPlane].height = disp_res.height;
			GDMA_PRINT(6, "%s, gdma->dispWin[%d].width=%d, height=%d \n", __FUNCTION__, disPlane, gdma->dispWin[disPlane].width, gdma->dispWin[disPlane].height);

			if (gdma->picQrd[disPlane] == ((gdma->picQwr[disPlane] + 1) & (GDMA_MAX_PIC_Q_SIZE - 1))) {
				GDMA_PRINT(2, "GDMA: pic Queue %d fullness r %d,w %d,c %d\n", disPlane, gdma->picQrd[disPlane], gdma->picQwr[disPlane], gdma->curPic[disPlane]);
				sGdmaReceiveWork[emptyIdx].used = 0;	/* free work */

				/* drop frames and go ahead for in-stop on firse use
				*  SE maybe block by debug thread, then sends the queued picture very quickly when system not busy
				*/
				GDMA_PRINT(2,  "GDMA: disPlane=%d pid=%d\n", disPlane, current->pid);
				for (idx = 0; idx < GDMA_MAX_PIC_Q_SIZE; idx++)
					sGdmaReceiveWork[idx].used = 0;
				gdma->picQwr[disPlane] = gdma->curPic[disPlane];
				gdma->picQrd[disPlane] = ((gdma->curPic[disPlane] - 2) & (GDMA_MAX_PIC_Q_SIZE - 1));

				/* return -99; */
				/* continue; */
			} else if (gdma->ctrl.zeroBuffer) {
				GDMA_PRINT(2, "GDMA: zeroBuffer is 1\n");
				GDMA_EXIT_CRITICAL();
				sGdmaReceiveWork[emptyIdx].used = 0;	/* free work */
				up(&gdma->sem_receive);
				return GDMA_SUCCESS;
			}

			pic = gdma->pic[disPlane] + (idxQwr = (gdma->picQwr[disPlane] + 1) & (GDMA_MAX_PIC_Q_SIZE - 1));

			memset(pic, 0, sizeof(GDMA_PIC_DATA));

			pic->seg_num = picObj->seg_num;
			pic->current_comp_idx = picObj->current_comp_idx;
			pic->cookie = data->cookie;
			pic->show = picObj->show;
			pic->syncInfo = picObj->syncInfo;
			pic->workqueueDone = 0;
			pic->onlineOrder = com_picObj->onlineOrder;

			pic->clear_x.value = picObj->clear_x.value;
			pic->clear_y.value = picObj->clear_y.value;
			*(char *)&pic->status = 0;
			pic->repeatCnt = 0;
			pic->context   = picObj->context;
			pic->workqueueIdx = emptyIdx;
			pic->isFromOffline = picObj->isFromOffline;
			pic->scale_factor = picObj->scale_factor;

			pic->plane_ar.value = picObj->plane_ar.value;
			pic->plane_gb.value = picObj->plane_gb.value;

			win = &pic->OSDwin;
			win3D = &pic->OSDwin3D;

			win->nxtAddr.addr     = 0;
			win->nxtAddr.last     = 1;


			/*  canvas X, Y always set the X, Y from picture */
			/*  destination x, y, width, height are for final resolution and position on panel */
			win->winXY.x          = picObj->x;
			win->winXY.y          = picObj->y;
			win->winWH.width      = picObj->width;
			win->winWH.height     = picObj->height;
			
			win->pitch			  = picObj->pitch;
			win->attr.extendMode  = 0;
			win->attr.rgbOrder    = picObj->rgb_order;/* VO_OSD_COLOR_RGB; */
			win->attr.objType     = 0;
			win->attr.type        = picObj->format & 0x1f;
			win->attr.littleEndian = picObj->format >> 5;
			win->attr.alphaEn     = (win->attr.alpha   = picObj->alpha) >  0 ? 1 : 0;

			win->attr.keepPreCLUT = 0;

			win->attr.clut_fmt    = picObj->paletteformat;/* rgba format */
			if (win->attr.type < 3)
				win->CLUT_addr = virt_to_phys(gdma->OSD_CLUT[picObj->paletteIndex]);

			if (picObj->decompress) {
				win->objOffset.objXoffset = 0;
				win->objOffset.objYoffset = 0;
				gdma_config_line_buffer_afbc(FBC_ON);
			} else {
				gdma_config_line_buffer_afbc(FBC_OFF);
				//this for fix when cursor move out of bound of panel(0,0),need do offset.
				if(picObj->dst_x<0){
					win->objOffset.objXoffset = abs(picObj->dst_x);
					picObj->dst_x=0;
				}else{
					win->objOffset.objXoffset = picObj->x;
				}
				if(picObj->dst_y<0){
					win->objOffset.objYoffset = gdma->ctrl.enableVFlip ? 0 : abs(picObj->dst_y);
					picObj->dst_y=0;
				}else{
					win->objOffset.objYoffset = gdma->ctrl.enableVFlip ? 0 : picObj->y;
				}
			}

			win->dst_x = picObj->dst_x;
			win->dst_y = picObj->dst_y;
			// for OSD shift
			if (g_osdshift_ctrl.shift_h_enable && com_picObj->fbdc_num==0) {
				g_osdshift_ctrl.para_h_N = abs(g_osdshift_ctrl.h_shift_pixel) / 2;
				g_osdshift_ctrl.para_h_K = abs(g_osdshift_ctrl.h_shift_pixel) % 2;
				g_osdshift_ctrl.Canvas_H = win->winWH.width;
				if (g_osdshift_ctrl.h_shift_pixel >= 0) {
					/* shift right*/
					if(g_osdshift_ctrl.plane==picObj->plane) {
						win->winWH.width -= (g_osdshift_ctrl.para_h_N);
						/*win->winXY.x += (g_osdshift_ctrl.para_h_N+1);*/
					}else if(VO_GRAPHIC_OSD4==picObj->plane){
						/*osd_sta will add this value*/
						if((picObj->dst_x + g_osdshift_ctrl.h_shift_pixel) >= 3840) {
							win->dst_x = 0;
							picObj->dst_x = 0;
							win->attr.alpha = 0;
							win->attr.alphaEn = 1;
						} else {
							win->dst_x += g_osdshift_ctrl.h_shift_pixel;
							picObj->dst_x = win->dst_x;
						}
					}
				}else{
					/*shift left*/
					if(g_osdshift_ctrl.plane==picObj->plane) {
						win->winWH.width -= (g_osdshift_ctrl.para_h_N);
						win->objOffset.objXoffset += g_osdshift_ctrl.para_h_N;
					}else if(VO_GRAPHIC_OSD4==picObj->plane) {
						gdam_lambda_cursor_shift_h(win, picObj);
					}
				}
			}

			if (g_osdshift_ctrl.shift_v_enable && com_picObj->fbdc_num==0) {
				g_osdshift_ctrl.para_v_N = abs(g_osdshift_ctrl.v_shift_pixel) / 2;
				g_osdshift_ctrl.para_v_K = abs(g_osdshift_ctrl.v_shift_pixel) % 2;
				g_osdshift_ctrl.Canvas_V = win->winWH.height;

				if (g_osdshift_ctrl.v_shift_pixel >= 0) {
					/*shift down*/
					if(g_osdshift_ctrl.plane==picObj->plane) {
						win->winWH.height -= (g_osdshift_ctrl.para_v_N+1);
						/*win->winXY.y += (g_osdshift_ctrl.para_v_N+1);*/
					}else if(VO_GRAPHIC_OSD4==picObj->plane) {
						/*use adjust osd_sta instead of here*/
						if((picObj->dst_y + g_osdshift_ctrl.v_shift_pixel) >= 2160) {
							win->dst_y = 0;
							picObj->dst_y = 0;
							win->attr.alpha = 0;
							win->attr.alphaEn = 1;
						}else {
							win->dst_y += g_osdshift_ctrl.v_shift_pixel;
							picObj->dst_y = win->dst_y;
						}
					}
				}else {
					/*shift up*/
					if(g_osdshift_ctrl.plane==picObj->plane) {
						win->winWH.height -= (g_osdshift_ctrl.para_v_N);
						win->objOffset.objYoffset += (g_osdshift_ctrl.para_v_N);
					}else if(VO_GRAPHIC_OSD4==picObj->plane) {
						gdam_lambda_cursor_shift_v(win, picObj);
					}
				}
			}

			/*  destination width/height set as the panel resolution */
			if (picObj->dst_width == 0)     /*  0 means scaling to panel resolution */
				win->dst_width = disp_res.width;
			else
				win->dst_width = picObj->dst_width;

			if (picObj->dst_height == 0)    /*  0 means scaling to panel resolution */
				win->dst_height = disp_res.height;
			else
				win->dst_height = picObj->dst_height;

			win->attr.compress = picObj->decompress;
			win->used = 1;

			if (win->attr.compress) {
				win->colorKey.keyEn   = picObj->key_en;
				win->colorKey.key     = picObj->colorkey;
			} else {
				win->colorKey.keyEn   = (picObj->colorkey != -1);
				win->colorKey.key     = picObj->colorkey;
			}


			win->top_addr = picObj->address + (gdma->ctrl.enableVFlip ? picObj->y + picObj->height - 1 : 0) * picObj->pitch;

			if(buf_phy_addr[0] == 0){
			    buf_phy_addr[0] = win->top_addr;
			}else if(buf_phy_addr[1] == 0){
				buf_phy_addr[1] = win->top_addr;				
			}else if(buf_phy_addr[2] == 0){
				buf_phy_addr[2] = win->top_addr; 
			}

			if (win->attr.compress && pic->isFromOffline) {
				win->attr.objType     =  (pic->seg_num > 0) ? 1 : 0;
				win->top_addr         = picObj->seg_addr[0];
				win->bot_addr         = picObj->seg_addr[1];
			}

			/*  3D mode configuration */
			if (gdma->ctrl.enable3D) {
				/*  1: PR mode */
				if (gdma->ctrl.enable3D_PR_SGMode) {
					if (picObj->picLayout == INBAND_CMD_GRAPHIC_TOP_AND_BOTTOM) {
						pr_debug(KERN_EMERG"GDMA, %s, Not support top-bottom in PR mode \n", __FUNCTION__);
					} else if (picObj->picLayout == INBAND_CMD_GRAPHIC_SIDE_BY_SIDE) {
						win->winWH.width = picObj->width >> 1;
						win->pitch >>= 1;
						win->objOffset.objXoffset += win3D->winWH.width;
					}
				} else {	/*  0: SG mode */
					memcpy(win3D, win, sizeof(GDMA_WIN));

					if (picObj->picLayout == INBAND_CMD_GRAPHIC_TOP_AND_BOTTOM) {
						win3D->winWH.height = win->winWH.height = picObj->height >> 1;
						if (gdma->ctrl.enableVFlip)
							win3D->top_addr -= (win3D->winWH.height * win3D->pitch);
						else
							win->objOffset.objYoffset += win3D->winWH.height;
					} else if (picObj->picLayout == INBAND_CMD_GRAPHIC_SIDE_BY_SIDE) {
						win3D->winWH.width = win->winWH.width = picObj->width >> 1;
						win->objOffset.objXoffset += win3D->winWH.width;
						win3D->pitch = win->pitch;
					}
				}
			}
			/*  TODO: FBDC information */
			win->attr.IMGcompress =  (com_picObj->fbdc_num > 0) ? 1 : 0;
			if (win->attr.IMGcompress) {
				win->attr.type = VO_OSD_COLOR_FORMAT_ARGB8888 & 0x1f;
				win->attr.littleEndian = VO_OSD_COLOR_FORMAT_ARGB8888 >> 5;
				win->attr.alphaEn = 0;		/* FBDC has alpha fucntion itself */
				win->attr.alpha = 0;		/* FBDC has alpha fucntion itself */
				win->CLUT_addr = 0;
				win->dst_x = win->dst_y = 0;
			}
			for (idx = 0; idx < com_picObj->fbdc_num; idx++) {
				if (idx < 3)
					picObj = &com_picObj->fbdc[idx];
				else {
					printk(KERN_EMERG"%s, too many FBDC layer into one OSD plane \n", __FUNCTION__);
					GDMA_EXIT_CRITICAL();
					up(&gdma->sem_receive);
					return GDMA_FAIL;
				}

				if (idx == 0) { 
					win->pitch = picObj->pitch;
					win->fbdc1_addr = picObj->address + ((picObj->width * picObj->height / 64) * 8); // 1 tile header = 64bit/64 pxl
					win->tfbc_format.tfbc_req_format = 0x0C; // ARGB8888
					win->tfbc_format.tfbc_req_lossy = picObj->compressed_ratio;
                                        win->tfbc_format.tfbc_argb_order = 0;
					win->objOffset.objYoffset = (unsigned short)((picObj->address&0xffff0000)>>16);
					win->objOffset.objXoffset = (unsigned short)(picObj->address&0x0000ffff);
				
					win->afbc_totaltile.x_total_tile = picObj->width;
					win->afbc_totaltile.y_total_tile = picObj->height;
					win->afbc_xy_pixel.x_pic_pixel = picObj->width;
					win->afbc_xy_pixel.y_pic_pixel = picObj->height;
					win->afbc_xy_sta.x_sta = picObj->x;
					win->afbc_xy_sta.y_sta = (gdma->ctrl.enableVFlip ? (picObj->height - 1) : picObj->y);
				}
				/* calculate the canvas size */
				win->winWH.width      = ((picObj->width+picObj->dst_x) > win->winWH.width) ? (picObj->width+picObj->dst_x) : win->winWH.width;
				win->winWH.height     = ((picObj->height+picObj->dst_y) > win->winWH.height) ? (picObj->height+picObj->dst_y) : win->winWH.height;
			}


			if (((picObj->dst_x+picObj->dst_width) > disp_res.width)) {
				/* change image width in osd windows info.  */
				if (picObj->dst_width > picObj->width) {
					win->winWH.width = (unsigned int)((disp_res.width - picObj->dst_x) / (picObj->dst_width / picObj->width));
					if (win->winWH.width < 16)		/* OSD_SR input width size minimal 16 */
						win->winWH.width = 16;
					win->dst_width = picObj->dst_width = (win->winWH.width * (picObj->dst_width / picObj->width));
					if((disp_res.width - picObj->dst_x - win->dst_width) >=1) {
						win->dst_width = picObj->dst_width +=(disp_res.width - picObj->dst_x - win->dst_width);
					}

					if ((picObj->dst_x+picObj->dst_width) > disp_res.width)
						win->dst_x = picObj->dst_x = picObj->dst_x - ((picObj->dst_x+picObj->dst_width)-disp_res.width);
				} else if (picObj->dst_width <= win->winWH.width)
					win->winWH.width = win->dst_width = picObj->dst_width = disp_res.width - picObj->dst_x;

				/*pr_debug(KERN_EMERG"[GDMA]--> win->dst_x=%d, win->dst_width=%d, win->winWH.width=%d ! \n ", win->dst_x, win->dst_width, win->winWH.width);*/
				//printk(KERN_ERR "[GDMA]--> win->dst_x=%d, win->dst_width=%d, win->winWH.width=%d ! \n ", win->dst_x, win->dst_width, win->winWH.width);
			}
			if (((picObj->dst_y+picObj->dst_height) > disp_res.height)) {
				/* change image height in osd windows info.  */
				if (picObj->dst_height > picObj->height) {
					win->winWH.height = ((disp_res.height - picObj->dst_y) / (picObj->dst_height / picObj->height));
					if (win->winWH.height < 4)		/* OSD_SR input height size minimal 4 */
						win->winWH.height = 4;
					win->dst_height = picObj->dst_height = (win->winWH.height * (picObj->dst_height / picObj->height));
					if((disp_res.height - picObj->dst_y - win->dst_height) >=1){
						win->dst_height = picObj->dst_height+=(disp_res.height - picObj->dst_y - win->dst_height);
					}
					picObj->dst_height = win->dst_height;
				} else if (picObj->dst_height <= win->winWH.height)
					win->winWH.height = win->dst_height = picObj->dst_height = disp_res.height - picObj->dst_y;

				//printk(KERN_ERR "[GDMA]--> win->dst_y=%d, win->dst_height=%d, win->winWH.height=%d ! \n ", win->dst_y, win->dst_height, win->winWH.height);
			}


			gdma->picCount[disPlane]++;
			gdma->picQwr[disPlane] = idxQwr;
			GDMA_PRINT(7,"%s, gdma->picQwr[%d]=%d \n", __FUNCTION__, disPlane, gdma->picQwr[disPlane]);

			/*  push work to workqueue */
			sGdmaReceiveWork[emptyIdx].disPlane = disPlane;
			sGdmaReceiveWork[emptyIdx].picQwr = idxQwr;
			/*
			*	Does NOT use workqueue mechanism to avoid system block sometimes
			*	driver calculates the register value immediately
			*/
			GDMA_PreUpdate(&(sGdmaReceiveWork[emptyIdx].GdmaReceiveWork));

#ifdef TRIPLE_BUFFER_SEMAPHORE
			gSemaphore[disPlane]--;
			if (gSemaphore[disPlane] < 0) {
				pr_debug(KERN_EMERG"gdma : gSemaphore[%d] %d\n", disPlane, gSemaphore[disPlane]);
				gSemaphore[disPlane] = 0;
			}

			if (gSemaphore[disPlane] == 0)
				if (!gdma->pendingRPC[disPlane])
					gdma->pendingRPC[disPlane] = 1;
#endif
			if (rtd_inl(SYS_REG_INT_CTRL_SCPU_reg) & SYS_REG_INT_CTRL_SCPU_osd_int_scpu_routing_en_mask)    // for debug
				GDMA_Update(gdma, 1);
			else {
				for (idx = 0; idx < GDMA_MAX_PIC_Q_SIZE; idx++)
					sGdmaReceiveWork[idx].used = 0;
				gdma->curPic[disPlane] = gdma->picQwr[disPlane];
				gdma->picQrd[disPlane] = ((gdma->curPic[disPlane] - 2) & (GDMA_MAX_PIC_Q_SIZE - 1));
				gdma->picCount[disPlane] = 0;
			}

		} else
			GDMA_PRINT(4,"%s, No layer in use \n", __FUNCTION__);
	}

	GDMA_EXIT_CRITICAL();
	up(&gdma->sem_receive);

	return GDMA_SUCCESS;
}
EXPORT_SYMBOL(GDMA_ReceivePictureNoBlock);
#endif

int GDMA_ReceiveCmd (GDMA_CMD_TYPE type, void *cmd)
{
	gdma_dev *gdma = &gdma_devices[0];
	switch (type) {
	case GDMA_SET_DISPLAY_MODE:
		gdma->ctrl.displayEachPic = ((GDMA_DISPLAY_MODE_STRUCT *)cmd)->displayEachPic;
		break;
	default:
		pr_debug(KERN_EMERG"GDMA: unsupport command type %d\n", type);
		break;
	}

	return GDMA_SUCCESS;
}
int GDMA_Lock_Vsync_Sem( u64* nsec )
{
	int ret = 0;
	long rrr = 0;
	//gdma_dev *gdma = &gdma_devices[0];
  #if ENABLE_VSYNC_NOTIFY
	ktime_t ktime_1;
	ktime_t ktime_2;
	s64 wait_time_us ;
	unsigned int timeout_microSec = 34000;
	const unsigned int timeout_warn_us = 21000;
	unsigned long vsync_lock_flags;

	unsigned long timeout_val = usecs_to_jiffies(timeout_microSec);

	spin_lock_irqsave(&vsync_completion_lock, vsync_lock_flags);

		reinit_completion( &gdma_vsync_completion );  //[FIXME] ? how to prevent isr called twice ?
				
	spin_unlock_irqrestore(&vsync_completion_lock, vsync_lock_flags);

	ktime_1 = ktime_get();		

	rrr = wait_for_completion_interruptible_timeout( &gdma_vsync_completion, timeout_val);

	ktime_2 = ktime_get();
	wait_time_us =	ktime_us_delta(ktime_2, ktime_1); 

	if( rrr == 0 )
	{
		// timeout

		if( timeout_warn_us < (unsigned int)wait_time_us )	{
					//timeout + signaled, warning only 
				VSYNC_PRINT(3, " warn waitVsync long? %d  diffT:%lld\n",
							timeout_microSec, wait_time_us
			);

		}
				
	}	
	else if( rrr == -ERESTARTSYS ) 
	{
		//interrupted 
		VSYNC_PRINT(3, "waitVsync break intr %lld \n", wait_time_us);
	}
			
	*nsec = ktime_to_ns( atomic64_read(&gGDMA_VSYNC_USER_TIME ) );

	#else
	gdma_dev *gdma = &gdma_devices[0];

	/* pr_debug(KERN_EMERG"func=%s line=%d \n", __FUNCTION__, __LINE__); */
	/* gdma->vsync_enable = 1; */
	/* down(&gdma->vsync_sem); */
	ret = down_timeout(&gdma->vsync_sem, msecs_to_jiffies(1000));
	/* pr_debug(KERN_EMERG"func=%s line=%d \n", __FUNCTION__, __LINE__); */
	/* mutex_lock(&gdma_vsync_lock); */

	#endif// ENABLE_VSYNC_NOTIFY

	
	return ret;
}


int GDMA_ConfigInputSrc_4k2k (bool flag)
{
	gdma_dev *gdma = &gdma_devices[0];

	if (flag != gdma->f_inputsrc_4k2k) { /* input source change */
		gdma->f_inputsrc_4k2k = flag;
	}

	return GDMA_SUCCESS;
}

int GDMA_Config3dUi (bool flag)
{
	gdma_dev *gdma = &gdma_devices[0];
	gdma->f_3d_ui = flag;

	DBG_PRINT(KERN_EMERG"%s--%d gdma->f_3d_ui=%d,\n", __FUNCTION__, __LINE__, gdma->f_3d_ui);

	return GDMA_SUCCESS;
}

int GDMA_ConfigBoxSrcSize(int size)
{
	gdma_dev *gdma = &gdma_devices[0];
	gdma->box_src_height = size;

	return GDMA_SUCCESS;
}

int GDMA_ConfigBoxDstSize(int size)
{
	int ret = GDMA_SUCCESS;
/*
	gdma_dev *gdma = &gdma_devices[0];
	if (ret == GDMA_SUCCESS) {
		gdma->box_dst_height = size;
		gdma->f_box_mode = TRUE;
	} else {
		gdma->f_box_mode = FALSE;
	}
*/
	return ret;
}

int GDMA_ConfigOSDxMixerEnable(GDMA_DISPLAY_PLANE disPlane, bool flag)
{
	volatile osdovl_osd_db_ctrl_RBUS osd_db_ctrl_reg;
	int ltimeout =0xff;
	osdovl_mixer_ctrl2_RBUS mixer_ctrl2;
	/* disable OSD1/OSD2 on Mixer */
	*(volatile u32 *)&mixer_ctrl2 = rtd_inl(OSDOVL_Mixer_CTRL2_reg);

	if (disPlane == GDMA_PLANE_OSD1) {
	        mixer_ctrl2.mixero1_en = flag;
	} else if (disPlane == GDMA_PLANE_OSD2) {
	        mixer_ctrl2.mixero2_en = flag;
	} else if (disPlane == GDMA_PLANE_OSD3) {
	        mixer_ctrl2.mixero3_en = flag;
	}
	rtd_outl(OSDOVL_Mixer_CTRL2_reg, *(volatile u32 *)&mixer_ctrl2);
	osd_db_ctrl_reg.regValue = rtd_inl(OSDOVL_OSD_Db_Ctrl_reg);
	osd_db_ctrl_reg.db_load = 1;

	rtd_outl(OSDOVL_OSD_Db_Ctrl_reg, osd_db_ctrl_reg.regValue); /*  mixer apply */

	osd_db_ctrl_reg.regValue = rtd_inl(OSDOVL_OSD_Db_Ctrl_reg);

	if (flag == false) {
		while(osd_db_ctrl_reg.db_load && ltimeout--){
		    osd_db_ctrl_reg.regValue = rtd_inl(OSDOVL_OSD_Db_Ctrl_reg);
		    mdelay(1);
		}
	}

	DBG_PRINT(KERN_EMERG"%s, %d, MIXER             0x%x\n", __FUNCTION__, __LINE__, rtd_inl(OSDOVL_Mixer_CTRL2_reg));
	return GDMA_SUCCESS;
}

int GDMA_ConfigOSDxMixerEnableNoDelay(GDMA_DISPLAY_PLANE disPlane, bool flag)
{
	volatile osdovl_osd_db_ctrl_RBUS osd_db_ctrl_reg;
	osdovl_mixer_ctrl2_RBUS mixer_ctrl2;
	/* disable OSD1/OSD2 on Mixer */
	*(volatile u32 *)&mixer_ctrl2 = rtd_inl(OSDOVL_Mixer_CTRL2_reg);

	if (disPlane == GDMA_PLANE_OSD1) {
		mixer_ctrl2.mixero1_en = flag;
	} else if (disPlane == GDMA_PLANE_OSD2) {
		mixer_ctrl2.mixero2_en = flag;
	} else if (disPlane == GDMA_PLANE_OSD3) {
		mixer_ctrl2.mixero3_en = flag;
	}
	rtd_outl(OSDOVL_Mixer_CTRL2_reg, *(volatile u32 *)&mixer_ctrl2);
	osd_db_ctrl_reg.regValue = rtd_inl(OSDOVL_OSD_Db_Ctrl_reg);
	osd_db_ctrl_reg.db_load = 1;

	rtd_outl(OSDOVL_OSD_Db_Ctrl_reg, osd_db_ctrl_reg.regValue); /*  mixer apply */

	osd_db_ctrl_reg.regValue = rtd_inl(OSDOVL_OSD_Db_Ctrl_reg);


	DBG_PRINT(KERN_EMERG"%s, %d, MIXER             0x%x\n", __FUNCTION__, __LINE__, rtd_inl(OSDOVL_Mixer_CTRL2_reg));
	return GDMA_SUCCESS;
}

int GDMA_ConfigOSDxMixerEnableNoApply(GDMA_DISPLAY_PLANE disPlane, bool flag)
{
	osdovl_mixer_ctrl2_RBUS mixer_ctrl2;
	/* disable OSD1/OSD2 on Mixer */
	*(volatile u32 *)&mixer_ctrl2 = rtd_inl(OSDOVL_Mixer_CTRL2_reg);

	if (disPlane == GDMA_PLANE_OSD1) {
		mixer_ctrl2.mixero1_en = flag;
	} else if (disPlane == GDMA_PLANE_OSD2) {
		mixer_ctrl2.mixero2_en = flag;
	} else if (disPlane == GDMA_PLANE_OSD3) {
		mixer_ctrl2.mixero3_en = flag;
	}
	rtd_outl(OSDOVL_Mixer_CTRL2_reg, *(volatile u32 *)&mixer_ctrl2);
	return GDMA_SUCCESS;
}

int GDMA_ConfigOSDEnable (bool flag)
{
	osdovl_mixer_ctrl2_RBUS mixer_ctrl2;
	/* disable OSD1/OSD2 on Mixer */
	*(volatile u32 *)&mixer_ctrl2 = rtd_inl(OSDOVL_Mixer_CTRL2_reg);
	mixer_ctrl2.mixer_en = flag;
	rtd_outl(OSDOVL_Mixer_CTRL2_reg, *(volatile u32 *)&mixer_ctrl2);

	/* disable OSD1/OSD2/OSD3 on OSD_CTRL */
	rtd_outl(GDMA_OSD1_CTRL_reg, GDMA_OSD1_CTRL_write_data(flag) | GDMA_OSD1_CTRL_osd1_en (1));
	/*rtd_outl(GDMA_OSD2_CTRL_reg, GDMA_OSD2_CTRL_write_data(flag) | GDMA_OSD2_CTRL_osd2_en (1));*/
	rtd_outl(GDMA_OSD3_CTRL_reg, GDMA_OSD3_CTRL_write_data(flag) | GDMA_OSD3_CTRL_osd3_en (1));
	DBG_PRINT(KERN_EMERG"%s, %d, MIXER             0x%x\n", __FUNCTION__, __LINE__, rtd_inl(OSDOVL_Mixer_CTRL2_reg));
	
	rtd_pr_gdma_info("func=%s line=%d OSDOVL_Mixer_CTRL2_reg=%x\n",__FUNCTION__,__LINE__,rtd_inl(OSDOVL_Mixer_CTRL2_reg));
	
	return GDMA_SUCCESS;
}

int GDMA_Sync_Enable(void)
{
#ifdef TRIPLE_BUFFER_SEMAPHORE
	int i;
	for (i = 0; i < GDMA_PLANE_MAXNUM; i++)
		gSemaphore[i] = gSEMAPHORE_MAX = 1;
#endif
	return GDMA_SUCCESS;
}

int GDMA_Sync_Disable(void)
{
#ifdef TRIPLE_BUFFER_SEMAPHORE
	int i;
	for (i = 0; i < GDMA_PLANE_MAXNUM; i++)
		gSemaphore[i] = gSEMAPHORE_MAX = GDMA_MAX_PIC_Q_SIZE+1;
#endif
	return GDMA_SUCCESS;
}

int GDMA_BW_GEN_Enable(int osd,int en,int w,int h)
{
	unsigned int offset = 0;
	unsigned int progDone = GDMA_CTRL_write_data(1);
	
	printk("func=%s line=%d osd=%d en=%d w=%d h=%d \n",__FUNCTION__,__LINE__,osd,en,w,h);
	if (osd == GDMA_PLANE_OSD1) {
		if(en){
			offset = 0x100;
			memset(&winfo_osd1,0x0,sizeof(winfo_osd1));
			winfo_osd1.nxtAddr.last = 1;
			winfo_osd1.winWH.width = w;
			winfo_osd1.winWH.height = h;
			winfo_osd1.attr.type = 0x7;
			winfo_osd1.attr.littleEndian = 0x1;
			winfo_osd1.top_addr = 0x20000000;
			rtd_outl(GDMA_OSD1_WI_reg,virt_to_phys(&winfo_osd1));
			rtd_outl(GDMA_OSD1_SIZE_reg,((w<<16) | h));
			rtd_outl(GDMA_OSD1_CTRL_reg,0x3);
		}else{
			rtd_outl(GDMA_OSD1_CTRL_reg,0x2);
		}
		progDone |= GDMA_CTRL_osd1_prog_done(1);
	} else if (osd == GDMA_PLANE_OSD2) {
		if(en){
		 offset = 0x100;
		 memset(&winfo_osd2,0x0,sizeof(winfo_osd2));
		 winfo_osd2.nxtAddr.last = 1;
		 winfo_osd2.winWH.width = w;
		 winfo_osd2.winWH.height = h;
		 winfo_osd2.attr.type = 0x7;
		 winfo_osd2.attr.littleEndian = 0x1;
		 winfo_osd2.top_addr = 0x20000000;
         rtd_outl(GDMA_OSD2_WI_reg,virt_to_phys(&winfo_osd2));
		 rtd_outl(GDMA_OSD2_SIZE_reg,((w<<16) | h));
		 rtd_outl(GDMA_OSD2_CTRL_reg,0x3);
		}else{
		 rtd_outl(GDMA_OSD2_CTRL_reg,0x2);
		}
	    progDone |= GDMA_CTRL_osd2_prog_done(1);	
	} else if (osd == GDMA_PLANE_OSD3) {
		if(en){
		offset = 0x200;
		memset(&winfo_osd3,0x0,sizeof(winfo_osd3));
		winfo_osd3.nxtAddr.last = 1;
		winfo_osd3.attr.type = 0x7;
		winfo_osd3.attr.littleEndian = 0x1;
		winfo_osd3.top_addr = 0x20000000;
        rtd_outl(GDMA_OSD3_WI_reg,virt_to_phys(&winfo_osd2));
		rtd_outl(GDMA_OSD3_SIZE_reg,(w<<16 | h));
		rtd_outl(GDMA_OSD3_CTRL_reg,0x3);
		}else{
		 rtd_outl(GDMA_OSD3_CTRL_reg,0x2);
		}
		progDone |= GDMA_CTRL_osd3_prog_done(1);
	}
 	progDone |=  GDMA_CTRL_get_write_data(0x1);
	rtd_outl(GDMA_CTRL_reg, progDone);
	
	return 0;
}
	  
int GDMA_CloneEnable(bool flag)
{
	unsigned int progDone = GDMA_CTRL_write_data(1);
	
	printk("func=%s line=%d flag=%d GDMA_OSD2_reg=%x\n",__FUNCTION__,__LINE__,flag,GDMA_OSD2_reg);
	if(flag){

        rtd_outl(GDMA_OSD2_WI_reg,rtd_inl(GDMA_OSD1_WI_reg));		
        rtd_outl(GDMA_OSD2_SIZE_reg,rtd_inl(GDMA_OSD1_SIZE_reg));
        rtd_outl(GDMA_OSD2_CTRL_reg,0x3);
        //rtd_outl(OSD_SR_OSD_SR_2_Scaleup_Ctrl0_reg,rtd_inl(OSD_SR_OSD_SR_1_Scaleup_Ctrl0_reg));
        //rtd_outl(OSD_SR_OSD_SR_2_Scaleup_Ctrl1_reg,rtd_inl(OSD_SR_OSD_SR_1_Scaleup_Ctrl1_reg));
        //rtd_outl(OSD_SR_OSD_SR_2_H_Location_Ctrl_reg,rtd_inl(OSD_SR_OSD_SR_1_H_Location_Ctrl_reg));
        //rtd_outl(OSD_SR_OSD_SR_2_H_Location_Ctrl_reg,rtd_inl(OSD_SR_OSD_SR_1_V_Location_Ctrl_reg));
	
		printk("func=%s line=%d flag=%d rtd_inl(GDMA_OSD2_WI_reg)=%x\n",__FUNCTION__,__LINE__,flag,rtd_inl(GDMA_OSD2_WI_reg));
	
	}else{
		rtd_outl(GDMA_OSD2_CTRL_reg,0x2);
	}
	printk("func=%s line=%d GDMA_CTRL_reg=%x\n",__FUNCTION__,__LINE__,GDMA_CTRL_reg);
 	progDone |= GDMA_CTRL_osd2_prog_done(1) | GDMA_CTRL_get_write_data(0x1);
	rtd_outl(GDMA_CTRL_reg, progDone);
	return 0;
}

int GDMA_ConfigOSDxEnableFast(GDMA_DISPLAY_PLANE disPlane, bool flag)
{
	unsigned int offset = 0;
	unsigned int progDone = GDMA_CTRL_write_data(1);
	gdma_dev *gdma = &gdma_devices[0];
	if( enable_stop_update == 0 && GAL_Runtime_Suspend_status == 0 ){


	if (disPlane == GDMA_PLANE_OSD1) {
		offset = 0;
		progDone |= GDMA_CTRL_osd1_prog_done(1);
	} else if (disPlane == GDMA_PLANE_OSD2) {
		offset = 0x100;
		progDone |= GDMA_CTRL_osd2_prog_done(1);
	} else if (disPlane == GDMA_PLANE_OSD3) {
		offset = 0x200;
		progDone |= GDMA_CTRL_osd3_prog_done(1);
	}

	/* disable OSDx on OSD_CTRL */
	if (gdma) {
		gdma->ctrl.osdEn[disPlane] = flag;
	}

	/*  disable immediately */
	if (flag == false) {
#if defined(CONFIG_ARCH_RTK288O)
#else
		// add for check WOSQRTK-11734
		if (GDMA_OSD1_CTRL_osd1_en(1))
			printk(KERN_INFO"[GDMA] OSD1 is disabled\n");
#endif
		//wirte 1 to clear
		rtd_outl(GDMA_OSD1_CTRL_reg+offset,GDMA_OSD1_CTRL_osd1_en(1));
		rtd_outl(GDMA_CTRL_reg, progDone);
	}else{
		if(rtd_inl(GDMA_OSD1_WI_reg+offset) != 0)
		{
			rtd_outl(GDMA_OSD1_CTRL_reg+offset, GDMA_OSD1_CTRL_osd1_en(1)|
					 GDMA_OSD1_CTRL_write_data(1) );
			rtd_outl(GDMA_CTRL_reg, progDone);
		}
	}
	GDMA_ConfigOSDxMixerEnable(disPlane, flag);
	}

	rtd_pr_gdma_info("func=%s line=%d OSDOVL_Mixer_CTRL2_reg=%x\n",__FUNCTION__,__LINE__,rtd_inl(OSDOVL_Mixer_CTRL2_reg));
	return GDMA_SUCCESS;
}

int GDMA_ConfigOSDxEnable(GDMA_DISPLAY_PLANE disPlane, bool flag)
{
	unsigned int offset = 0;
	unsigned int progDone = GDMA_CTRL_write_data(1);
	gdma_dev *gdma = &gdma_devices[0];

	if (disPlane == GDMA_PLANE_OSD1) {
		offset = 0;
		progDone |= GDMA_CTRL_osd1_prog_done(1);
	/*
	} else if (disPlane == GDMA_PLANE_OSD2) {
		offset = 0x100;
		progDone |= GDMA_CTRL_osd2_prog_done(1);
	*/
	} else if (disPlane == GDMA_PLANE_OSD3) {
		offset = 0x200;
		progDone |= GDMA_CTRL_osd3_prog_done(1);
	}

	/* disable OSDx on OSD_CTRL */
	if (gdma) {
		gdma->ctrl.osdEn[disPlane] = flag;
	}

	/*  disable immediately */
	if (flag == false) {
		//wirte 1 to clear
		rtd_outl(GDMA_OSD1_CTRL_reg+offset,GDMA_OSD1_CTRL_osd1_en(1));
		rtd_outl(GDMA_CTRL_reg, progDone);
	}else{
		rtd_outl(GDMA_OSD1_CTRL_reg+offset, GDMA_OSD1_CTRL_osd1_en(1)|
			GDMA_OSD1_CTRL_write_data(1) );
		rtd_outl(GDMA_CTRL_reg, progDone);
	}

	GDMA_ConfigOSDxMixerEnable(disPlane, flag);
	return GDMA_SUCCESS;
}

int GAL_Runtime_Suspend_disable_OSD(void)
{
	unsigned int progDone = GDMA_CTRL_write_data(1);
	volatile gdma_dma_intst_RBUS dma_intst_reg;
	
	printk(KERN_ERR"%s GDMA_OSD1_CTRL_reg=%x\n", __func__, rtd_inl(GDMA_OSD1_CTRL_reg));
	GAL_Runtime_Suspend_status = 1;
	enable_stop_update = 1;
	gdma_usleep(16000);

	progDone |= GDMA_CTRL_osd1_prog_done(1);
	
	//wirte 1 to clear
	rtd_outl(GDMA_OSD1_CTRL_reg,GDMA_OSD1_CTRL_osd1_en(1));
	rtd_outl(GDMA_CTRL_reg, progDone);
	gdma_usleep(16000);

	dma_intst_reg.regValue=~(0x1);
	dma_intst_reg.write_data = 0;

	rtd_outl(GDMA_DMA_INTST_reg, dma_intst_reg.regValue);

	return 1;
}
EXPORT_SYMBOL(GAL_Runtime_Suspend_disable_OSD);

int GAL_Runtime_Suspend_enable_OSD(void)
{
	printk(KERN_ERR"func=%s line=%d\n", __func__,__LINE__);
	GAL_Runtime_Suspend_status = 0;
	enable_stop_update = 0;
	return 1;
}
EXPORT_SYMBOL(GAL_Runtime_Suspend_enable_OSD);

int GAL_Runtime_Suspend_printk_OSD(void)
{

	printk(KERN_ERR"%s GDMA_OSD1_CTRL_reg=%x\n", __func__, rtd_inl(GDMA_OSD1_CTRL_reg));

	return 1;
}
int GAL_get_Suspend_status(void)
{
	return GAL_Suspend_status;
}
int GAL_Runtime_Resume_receive(void)
{
    GDMA_ReceivePicture(&glastinfo);

	GDMA_ConfigOSDxEnableFast(GDMA_PLANE_OSD1,true);
	
    return 1;
}
EXPORT_SYMBOL(GAL_Runtime_Resume_receive);

int GAL_Runtime_Suspend(void)
{
#if 1
	gdma_dev *gdma = NULL;

  printk(KERN_EMERG"[GDMA][%d] GAL_Runtime_Suspend_status=%d \n",__LINE__,GAL_Runtime_Suspend_status);

  if(!GAL_Runtime_Suspend_status){
	 GDMA_ConfigOSDxEnableFast(GDMA_PLANE_OSD1,false);

     GAL_Runtime_Suspend_status = 1;
  }else{
     return 0;
  }

	printk(KERN_EMERG"[GDMA][%d] [%s]GAL_Runtime_Suspend_status=%d \n",__LINE__,__FUNCTION__,GAL_Runtime_Suspend_status);

	gdma = &gdma_devices[0];
	//gdma->dev = &(gdma_devs->dev);
#endif
	enable_stop_update = 1;

	/* gdma INT rounting to SCPU */
	rtd_outl(SYS_REG_INT_CTRL_SCPU_reg, SYS_REG_INT_CTRL_SCPU_osd_int_scpu_routing_en_mask | SYS_REG_INT_CTRL_SCPU_write_data(0));

	rtd_outl(GDMA_OSD_INTEN_reg, GDMA_OSD_INTEN_write_data(0) | GDMA_OSD_INTEN_osd1_vact_end(1) | GDMA_OSD_INTEN_osd1_vsync(1));

#if 1
	gdma_usleep(16000);

	gdma_suspend(gdma->dev);

	//disable OSD
	GDMA_ConfigOSDxEnableFast(GDMA_PLANE_OSD1,false);

	gdma_usleep(16000);

#endif
	//rtk_se_runtime_suspend();

	return 0;
}

int GAL_Runtime_Resume(void)
{
#if 1
	gdma_dev *gdma = NULL;

  printk(KERN_EMERG"[GDMA][%d][%s] GAL_Runtime_Suspend_status=%d \n",__LINE__,__FUNCTION__,GAL_Runtime_Suspend_status);

  if(GAL_Runtime_Suspend_status){
     GAL_Runtime_Suspend_status = 0;
  	 GDMA_ConfigOSDxEnableFast(GDMA_PLANE_OSD1,false);
  }else{
      return 0;
  }
	gdma = &gdma_devices[0];
	//gdma->dev = &(gdma_devs->dev);

	gdma_resume(gdma->dev);
#endif
	enable_stop_update = 0;
	rtd_outl(GDMA_OSD_INTEN_reg, GDMA_OSD_INTEN_write_data(1) | GDMA_OSD_INTEN_osd1_vact_end(1) | GDMA_OSD_INTEN_osd1_vsync(1));

	GDMA_ConfigOSDxEnableFast(GDMA_PLANE_OSD1,true);

    printk(KERN_EMERG"[GDMA][%d][reg %x] GDMA_OSD_INTEN_reg=%x \n",__LINE__,GDMA_OSD_INTEN_reg, rtd_inl(GDMA_OSD_INTEN_reg));
	//rtk_se_runtime_resume();

	return 0;
}



int GAL_Runtime_Suspend_light(void)
{
#if 1
	gdma_dev *gdma = NULL;

  printk(KERN_EMERG"[GDMA][%d] GAL_Runtime_Suspend_status=%d \n",__LINE__,GAL_Runtime_Suspend_status);

  if(!GAL_Runtime_Suspend_status){
	 GDMA_ConfigOSDxEnableFast(GDMA_PLANE_OSD1,false);

     GAL_Runtime_Suspend_status = 1;
  }else{
     return 0;
  }

	printk(KERN_EMERG"[GDMA][%d] [%s]GAL_Runtime_Suspend_status=%d \n",__LINE__,__FUNCTION__,GAL_Runtime_Suspend_status);

	gdma = &gdma_devices[0];
	//gdma->dev = &(gdma_devs->dev);
#endif
	enable_stop_update = 1;

	/* gdma INT rounting to SCPU */
	//rtd_outl(SYS_REG_INT_CTRL_SCPU_reg, SYS_REG_INT_CTRL_SCPU_osd_int_scpu_routing_en_mask | SYS_REG_INT_CTRL_SCPU_write_data(0));

	//rtd_outl(GDMA_OSD_INTEN_reg, GDMA_OSD_INTEN_write_data(0) | GDMA_OSD_INTEN_osd1_vact_end(1) | GDMA_OSD_INTEN_osd1_vsync(1));

#if 1
	gdma_usleep(16000);

	//gdma_suspend(gdma->dev);

	//disable OSD
	GDMA_ConfigOSDxEnableFast(GDMA_PLANE_OSD1,false);

	gdma_usleep(16000);

#endif
	//rtk_se_runtime_suspend();

	return 0;
}
EXPORT_SYMBOL(GAL_Runtime_Suspend_light);

int GAL_Runtime_Resume_light(void)
{
#if 1
	gdma_dev *gdma = NULL;

  printk(KERN_EMERG"[GDMA][%d][%s] GAL_Runtime_Suspend_status=%d \n",__LINE__,__FUNCTION__,GAL_Runtime_Suspend_status);

  if(GAL_Runtime_Suspend_status){
     GAL_Runtime_Suspend_status = 0;
  	 GDMA_ConfigOSDxEnableFast(GDMA_PLANE_OSD1,false);
  }else{
      return 0;
  }
	gdma = &gdma_devices[0];
	//gdma->dev = &(gdma_devs->dev);

	//gdma_resume(gdma->dev);
#endif
	enable_stop_update = 0;
	//rtd_outl(GDMA_OSD_INTEN_reg, GDMA_OSD_INTEN_write_data(1) | GDMA_OSD_INTEN_osd1_vact_end(1) | GDMA_OSD_INTEN_osd1_vsync(1));
	//GDMA_ConfigOSDEnable(true);

	GDMA_ConfigOSDxEnableFast(GDMA_PLANE_OSD1,true);

    printk(KERN_EMERG"[GDMA][%d][reg %x] GDMA_OSD_INTEN_reg=%x \n",__LINE__,GDMA_OSD_INTEN_reg, rtd_inl(GDMA_OSD_INTEN_reg));
	//rtk_se_runtime_resume();

	return 0;
}
EXPORT_SYMBOL(GAL_Runtime_Resume_light);

int GDMA_AddCallBackFunc (void (*fp)(void *, bool))
{
	gdma_dev *gdma = &gdma_devices[0];
	if (gdma) {
		gdma->GDMA_CallBack = fp;
		return GDMA_SUCCESS;
	} else
		return GDMA_FAIL;
}

#if IS_ENABLED(CONFIG_DRM_REALTEK)
void GDMA_RegisterIrqCallBackFunc(void (*fp)(void *), void * data)
{
	gdma_dev *gdma = &gdma_devices[0];
	if (gdma) {
		gdma->GDMA_IrqCallBack = fp;
		gdma->IrqCallBackData = data;
	}
}
EXPORT_SYMBOL(GDMA_RegisterIrqCallBackFunc);

#endif
int GDMA_AddCookieCallBackFunc (void (*fp)(void *, int))
{
	gdma_dev *gdma = &gdma_devices[0];
	if (gdma) {
		gdma->GDMA_CookieCallBack = fp;
		return GDMA_SUCCESS;
	} else
		return GDMA_FAIL;
}

int GDMA_GetPanelPixelMode(void)
{
	/* 0:ONE_PIXEL, 1:TWO_PIXEL, 2:FOUR_PIXEL */
	return ONE_PIXEL_SEG;
}

int GDMA_GetFenceInfo(GDMA_FENCE_INFO *getInfo)
{
    if(getInfo == 0)
        return GDMA_FAIL;

    {
        struct gdma_fence *new_gdma_fence = alloc_gdma_fence();
        if (new_gdma_fence)
        {
            getInfo->fence_fd = new_gdma_fence->gdma_fence_fd;
            getInfo->fence_handle = (uintptr_t) new_gdma_fence;
           GDMA_PRINT(9, "fence get %llu :%d\n",getInfo->fence_handle, getInfo->fence_fd);
        }
    }
    
    return GDMA_SUCCESS;
}

static int GDMA_SignalFence(struct gdma_fence *gf)
{
    int ret = 0;
 
    if (gf)
    {
        ret = dma_fence_signal(&gf->base);
    }

    return ret;
}

int GDMA_ConfigNonBlocking (bool enable)
{
#ifdef TRIPLE_BUFFER_SEMAPHORE
	gdma_dev *gdma = &gdma_devices[0];
	/*
	*	use gdma->sem_receive semaphore to avoid gSemaphore data inconsistent
	*	maybe cause the gdma->updateSem semaphore will not release, coming picture can't be updated
	*/
	down(&gdma->sem_receive);

	gdma->ctrl.enableNonBlocking = enable;

	/*GDMA_Flush(gdma);*/

	gSemaphore[GDMA_PLANE_OSD1] = gSEMAPHORE_MAX = (gdma->ctrl.enableNonBlocking) ? 2 : 1;

	pr_debug(KERN_EMERG"GDMA: Config Non-Blocking mode %d, gSemaphore[%d]=%d\n", gdma->ctrl.enableNonBlocking, GDMA_PLANE_OSD1, gSemaphore[GDMA_PLANE_OSD1]);

	up(&gdma->sem_receive);
#endif

	return GDMA_SUCCESS;
}

void GDMA_CopyLayerInfo(GDMA_PICTURE_OBJECT *dest, GDMA_LAYER_OBJECT *src)
{
	dest->address = src->address;
	dest->alpha = src->alpha;
	dest->clear_x = src->clear_x;
	dest->clear_y = src->clear_y;
	dest->colorkey = src->colorkey;
	dest->context = src->context;
	dest->decompress = src->decompress;
	dest->dst_height = src->dst_height;
	dest->dst_width = src->dst_width;
	dest->dst_x = src->dst_x;
	dest->dst_y = src->dst_y;
	dest->format = src->format;
	dest->fbdc_format = src->fbdc_format;
	dest->height = src->height;
	dest->key_en = src->key_en;
	dest->layer_used = 1;
	dest->paletteIndex = src->paletteIndex;
	dest->picLayout = src->picLayout;
	dest->pitch = src->pitch;
	dest->plane_ar.value = src->plane_ar.value;
	dest->plane_gb.value = src->plane_gb.value;
	dest->show = src->show;
	dest->src_type = src->src_type;
	dest->syncInfo.syncstamp = src->syncstamp;
	dest->width = src->width;
#if (defined(CONFIG_ARCH_RTK6748))
	if((src->decompress==1) || (src->compressed_ratio == TFBC_HWC_NON_COMPRESS))
		dest->rgb_order = 1;
#endif	
	dest->x = src->x;
	dest->y = src->y;
	dest->scale_factor = src->scale_factor;
	if(src->compressed_ratio == TFBC_HWC_NON_COMPRESS)
		dest->compressed_ratio = 0;
	else
		dest->compressed_ratio = src->compressed_ratio;
}

/** @brief this function plays as a composer role. Receive the graphic layers up to 6 layers (4k2k, 2k1k, one mouse)
* 		Graphic layer maybe is FBDC, Normal OSD, Mouse type, function will assign the layer to correct path
* 		that pre-blending or compress or pure blending, then save into temporary space.
* 		Those pre-blending image sends to online path, GDMA will get the layers and display with other layers.
* 		Function takes care about how to arrange layers to offline path and send pre-blend and direct display layer
* 		to online path. Blend order is also under control in this function.
*  @param data the content of structure is from upper level that wants to display
*  @return GDMA_SUCCESS or negative value
*/
int GDMA_ReceiveGraphicLayers(GRAPHIC_LAYERS_OBJECT *data)
{
	gdma_dev *gdma = &gdma_devices[0];
	int err = -50, loop = 0, loop2 = 0, compLayerNum = 0, gdmaLayerNum = 0,/* fbdc4k2kIdx = 0, fbdc2k1kIdx = 0,*/ fbdcIdx = 0, normalIdx = 0;
	COMPOSE_PICTURE_OBJECT *com_picObj = NULL, *com2_picObj = NULL;
	static PICTURE_LAYERS_OBJECT *picLayerObj = NULL, *sendCompLayerObj = NULL, *sendGdmaLayerObj = NULL;
	static int mouseKeepOnline = 0;
	static PICTURE_LAYERS_OBJECT *mousepicObj = NULL;
	GDMA_PICTURE_OBJECT *layer_obj = NULL;
	/*int ScaleRatio[GDMA_LAYER_NUM];*/     /*  MSB for height, LSB for width */
	GDMA_PIC_MATCH_SYNCSTAMP syncinfo[GDMA_LAYER_NUM];
	u64 syncstamp_tmp;
	int hasMouse = 0, Is4k2kp2k1kCase = 0, IsAll4k2kCase = 0, Is2k1kCase = 0;/*, IsFBDC4k2kCase = 0, IsFBDC2k1kCase = 0;*/
	int has4k2kMouseFBDC = 0, has2k1kMouseFBDC = 0, hasDecompress = 0, hasCompress = 0;
	int has4k2kMouseNormal = 0, has2k1kMouseNormal = 0;
	int num4k2kFBDC = 0, num2k1kFBDC = 0, num4k2kNormal = 0, num2k1kNormal = 0, start2k1kIdx = -1;
	int layerIdx = 0, offlineOrder = 0;
	VO_GRAPHIC_PLANE plane_tmp = VO_GRAPHIC_OSD1, offlinePlane = VO_GRAPHIC_OSD1, onlinePlane = VO_GRAPHIC_OSD1;
	VO_RECTANGLE disp_res;
#ifdef USR_WORKQUEUE_UPDATE_BUFFER
	int j=0;
	int try_wait=TRIPLE_BUFFER_TRY_NUM;
	int queue_work_found=0;
#endif
	getDispSize(&disp_res);

	if (data->layer_num < 0 || data->layer_num > GDMA_LAYER_NUM) {
		DBG_PRINT(KERN_EMERG"%s, Warning. too many layers \n", __FUNCTION__);
		return -100;
	}

	DBG_PRINT(KERN_EMERG"+++++++%s+++++++ \n", __FUNCTION__);


	down(&gdma->sem_gfxreceive);

	if (picLayerObj == NULL)
		picLayerObj = (PICTURE_LAYERS_OBJECT *)kmalloc(sizeof(PICTURE_LAYERS_OBJECT), GFP_KERNEL);
	if (sendCompLayerObj == NULL)
		sendCompLayerObj = (PICTURE_LAYERS_OBJECT *)kmalloc(sizeof(PICTURE_LAYERS_OBJECT), GFP_KERNEL);
	if (sendGdmaLayerObj == NULL)
		sendGdmaLayerObj = (PICTURE_LAYERS_OBJECT *)kmalloc(sizeof(PICTURE_LAYERS_OBJECT), GFP_KERNEL);
	if (mousepicObj == NULL)
		mousepicObj = (PICTURE_LAYERS_OBJECT *)kmalloc(sizeof(PICTURE_LAYERS_OBJECT), GFP_KERNEL);
	if (picLayerObj == NULL || sendCompLayerObj == NULL || sendGdmaLayerObj == NULL || mousepicObj == NULL) {
		up(&gdma->sem_gfxreceive);
		pr_debug(KERN_EMERG"%s, kmalloc failed... \n", __FUNCTION__);
		return -100;
	}

	/* get a random syncstamp */
	syncstamp_tmp = get_random_int();
	DBG_PRINT(KERN_EMERG"%s, random syncstamp = %lld \n", __FUNCTION__, syncstamp_tmp);

	/* memset(&osdcomp_picObj,0x0,sizeof(GRAPHIC_LAYERS_OBJECT)); */
	memset(&syncinfo, 0x0, sizeof(syncinfo));
	memset(picLayerObj, 0x0, sizeof(PICTURE_LAYERS_OBJECT));
	memset(sendCompLayerObj, 0x0, sizeof(PICTURE_LAYERS_OBJECT));
	memset(sendGdmaLayerObj, 0x0, sizeof(PICTURE_LAYERS_OBJECT));

	/*  collect & check all layers information */
	if (data->layer[0].width == FOURK_W && data->layer[0].height == TWOK_H)
		Is4k2kp2k1kCase = IsAll4k2kCase = 1;
	else
		Is2k1kCase = 1;

	for (loop = 0; loop < data->layer_num; loop++) {

		/*  related to 4K2K */
		if (disp_res.width == FOURK_W && disp_res.height == TWOK_H && data->layer[loop].scale_factor == 1) {

			if (data->layer[loop].src_type == SRC_NORMAL) {
				/*pr_debug(KERN_EMERG"%s, Layer[%d] 4K2K layer is NOT FBDC type \n", __FUNCTION__, loop);*/
				num4k2kNormal++;
			}
			else if (data->layer[loop].src_type == SRC_FBDC) {
				num4k2kFBDC++;
			}
		}
		/*  related to 2K1K or below */
		else if (data->layer[loop].width <= TWOK_W && data->layer[loop].height <= ONEK_H) {
			if (start2k1kIdx == -1)
				start2k1kIdx = loop;
			if (data->layer[loop].src_type == SRC_FBDC)
				num2k1kFBDC++;
			else if (data->layer[loop].src_type == SRC_NORMAL)
				num2k1kNormal++;
		}

		/* check compress picture */
		if (data->layer[loop].compressed)
			hasCompress = 1;

		/*  related to mouse */
		if (data->layer[loop].src_type == SRC_MOUSE_NORMAL || data->layer[loop].src_type == SRC_MOUSE_FBDC)
			hasMouse = 1;
		if (data->layer[loop].src_type == SRC_MOUSE_FBDC && data->layer[loop].scale_factor == 1 && disp_res.width == FOURK_W && disp_res.height == TWOK_H)
			has4k2kMouseFBDC = 1;
		if (data->layer[loop].src_type == SRC_MOUSE_NORMAL && data->layer[loop].scale_factor == 1 && disp_res.width == FOURK_W && disp_res.height == TWOK_H)
			has4k2kMouseNormal = 1;
		if (data->layer[loop].src_type == SRC_MOUSE_FBDC && data->layer[loop].scale_factor == 1 && disp_res.width <= TWOK_W && disp_res.height <= ONEK_H)
			has2k1kMouseFBDC = 1;
		if (data->layer[loop].src_type == SRC_MOUSE_NORMAL && data->layer[loop].scale_factor == 1 && disp_res.width <= TWOK_W && disp_res.height <= ONEK_H)
			has2k1kMouseNormal = 1;
		if (data->layer[loop].src_type == SRC_MOUSE_FBDC && data->layer[loop].scale_factor > 1 && disp_res.width == FOURK_W && disp_res.height == TWOK_H)
			has2k1kMouseFBDC = 1;
		if (data->layer[loop].src_type == SRC_MOUSE_NORMAL && data->layer[loop].scale_factor > 1 && disp_res.width == FOURK_W && disp_res.height == TWOK_H)
			has2k1kMouseNormal = 1;
		if (hasMouse) {
			if (loop != (data->layer_num-1))
				pr_debug(KERN_EMERG"%s, Layer[%d] Mouse layer is NOT at the latest position \n", __FUNCTION__, loop);
		}

		/*  record scale ratio */
		/*
		if (data->layer[loop].dst_width == 0 && data->layer[loop].dst_height == 0) {
			ScaleRatio[loop] = (gdma->dispWin[0].height/data->layer[loop].height)<<16;
			ScaleRatio[loop] |= ((gdma->dispWin[0].width/data->layer[loop].width)&0xFFFF);
		} else {
			ScaleRatio[loop] = (data->layer[loop].dst_height/data->layer[loop].height)<<16;
			ScaleRatio[loop] |= ((data->layer[loop].dst_width/data->layer[loop].width)&0xFFFF);
		}
		*/

		DBG_PRINT(KERN_EMERG"%s, Layer[%d] type %d compress %d \n", __FUNCTION__, loop, data->layer[loop].src_type, data->layer[loop].compressed);
	}

	/* force sticking syncstamp except for compress layers */
	for (loop = 0; loop < data->layer_num; loop++) {
		/* when compress case, mouse has another syncstamp */
		if (hasCompress)
			;/*
			if (data->layer[loop].src_type != SRC_MOUSE_NORMAL && data->layer[loop].src_type != SRC_MOUSE_FBDC)
				data->layer[loop].syncstamp = syncstamp_tmp;
			else
				data->layer[loop].syncstamp = get_random_int();
			*/
		else
			data->layer[loop].syncstamp = syncstamp_tmp;
	}


	/*	in compress case, driver automatically disable osd1 for mouse during mouse moves to mouse hides
	*	set the mouseHideFlag when mouse disappear
	*/
	if (hasMouse && hasCompress) {
		if (mouseKeepOnline == 0) {
			memset(mousepicObj, 0x00, sizeof(PICTURE_LAYERS_OBJECT));
			if (has4k2kMouseFBDC || has2k1kMouseFBDC) {
				GDMA_CopyLayerInfo(&mousepicObj->layer[0].fbdc[0], &data->layer[data->layer_num-1]);
				mousepicObj->layer[0].fbdc[0].plane = VO_GRAPHIC_OSD1;
				mousepicObj->layer[0].fbdc_num = 1;
				mousepicObj->layer[0].fbdc[0].show = 0;
			} else {
				GDMA_CopyLayerInfo(&mousepicObj->layer[0].normal[0], &data->layer[data->layer_num-1]);
				mousepicObj->layer[0].normal[0].plane = VO_GRAPHIC_OSD1;
				mousepicObj->layer[0].normal_num = 1;
				mousepicObj->layer[0].normal[0].show = 0;
			}

			mousepicObj->layer[0].onlineOrder = C0;
			mousepicObj->layer_num = 1;
			mousepicObj->cookie = 0;
			DBG_PRINT(KERN_EMERG"%s, keep mouse layer done \n", __FUNCTION__);
		}

		mouseKeepOnline = 1;
	} else {
		if (mouseKeepOnline && hasCompress) {
			mouseKeepOnline = 0;

			GDMA_ReceivePicture(mousepicObj);
			DBG_PRINT(KERN_EMERG"%s, make mouse layer disappear \n", __FUNCTION__);
		}
	}

	if ((num4k2kFBDC+num2k1kFBDC+num4k2kNormal+num2k1kNormal+hasMouse) != data->layer_num)
		pr_debug(KERN_EMERG"%s, Graphic layers are something wrong, Please check it! \n", __FUNCTION__);
	if ((num4k2kFBDC+num4k2kNormal+num2k1kFBDC+num2k1kNormal+hasMouse) > GDMA_LAYER_NUM)
		pr_debug(KERN_EMERG"%s, Too many layer=%d. Now Not support this case!, num4k2kFBDC=%d, num2k1kFBDC=%d, num2k1kNormal=%d \n", __FUNCTION__, (num4k2kFBDC+num2k1kFBDC+num2k1kNormal), num4k2kFBDC, num2k1kFBDC, num2k1kNormal);
	if (hasCompress && ((num2k1kNormal+hasMouse) >= 5))
		pr_debug(KERN_EMERG"%s, to compress normal layers over 4 layers, input num=%d \n", __FUNCTION__, (num2k1kNormal+hasMouse));


	DBG_PRINT(KERN_EMERG"%s, start layer arrangement \n", __FUNCTION__);
	/* only one layer to OSD4 display */
	if (data->layer_num == 1 && hasMouse == 0 && data->layer[0].compressed == 0 && data->layer[0].src_type == SRC_NORMAL)
		onlinePlane = VO_GRAPHIC_OSD4;
	if (data->layer_num == 1 && hasMouse == 0 && data->layer[0].compressed == 0 )		/* 2 pixels */
		onlinePlane = VO_GRAPHIC_OSD1;
	if (data->layer_num == 1 && hasMouse == 0 && data->layer[0].compressed == 0 && data->layer[0].src_type == SRC_NORMAL_1)		/* */
		onlinePlane = VO_GRAPHIC_OSD3;

	/* compress case: force mouse layer to OSD1, normal/fbdc layer starts from OSD2 */
	if (hasCompress && hasMouse)
		offlinePlane = VO_GRAPHIC_OSD3;

	/* uncompress case: when FBDC mouse + normal layers, force mouse to OSD1, normal layers starts from OSD2 */
	if (hasCompress == 0 && hasMouse && (has4k2kMouseFBDC || has2k1kMouseFBDC) && (num4k2kNormal > 0 || num2k1kNormal > 0))
		onlinePlane = VO_GRAPHIC_OSD3;

	for (loop = 0; loop < data->layer_num; loop++) {

		if (data->layer[loop].decompress) {
			plane_tmp = onlinePlane;
			onlinePlane = VO_GRAPHIC_OSD4;
			hasDecompress = 1;
		}

		/* only 3 normal layers which sets in OSD1/2/4, driver could NOT switch OSDSRx_switch_to register */
		if (data->layer_num == 3 && loop == 2 && data->layer[loop].src_type == SRC_NORMAL) {
			onlinePlane = VO_GRAPHIC_OSD4;
			normalIdx = 0;
			layerIdx++;
		}

		if (fbdcIdx > 2) {
			if (hasCompress) {
				offlinePlane = (VO_GRAPHIC_PLANE)(offlinePlane + 1);
				if (offlinePlane == VO_GRAPHIC_OSD4)
					offlinePlane = (VO_GRAPHIC_PLANE)(offlinePlane + 1);
			}
			else
				onlinePlane = (VO_GRAPHIC_PLANE)(onlinePlane + 1);
			fbdcIdx = 0;
			layerIdx++;
		}

		if (normalIdx > 0) {
			if (hasCompress) {
				offlinePlane = (VO_GRAPHIC_PLANE)(offlinePlane + 1);
				if (offlinePlane == VO_GRAPHIC_OSD4)
					offlinePlane = (VO_GRAPHIC_PLANE)(offlinePlane + 1);
			} else {
				if (data->layer[loop].decompress) {
					plane_tmp = onlinePlane;
					onlinePlane = VO_GRAPHIC_OSD4;
					hasDecompress = 1;
				} else {
					if (loop != 0 && data->layer[loop-1].decompress)
						onlinePlane = plane_tmp;
					else
						onlinePlane = (VO_GRAPHIC_PLANE)(onlinePlane + 1);
					if (hasDecompress && onlinePlane == VO_GRAPHIC_OSD4)
						onlinePlane = (VO_GRAPHIC_PLANE)(onlinePlane + 1);
				}
			}
			/* check normal layers + FBDC mouse mix case */
			if (data->layer[loop].src_type == SRC_MOUSE_FBDC)
				onlinePlane = VO_GRAPHIC_OSD1;

			normalIdx = 0;
			layerIdx++;
		}

		/* check mouse layer if let layerIdx++ */
		if ((data->layer[loop].src_type == SRC_MOUSE_NORMAL || data->layer[loop].src_type == SRC_MOUSE_FBDC) &&
			fbdcIdx > 0) {

			if (hasCompress == 0) {

				if (data->layer[loop].src_type == SRC_MOUSE_NORMAL) {
					layerIdx++;
					if (hasDecompress)
						onlinePlane = (VO_GRAPHIC_PLANE)(onlinePlane + 1);
					else
						onlinePlane = VO_GRAPHIC_OSD4;
				}
			}
		}

		if (data->layer[loop].src_type == SRC_FBDC) {
			GDMA_CopyLayerInfo(&picLayerObj->layer[layerIdx].fbdc[fbdcIdx], &data->layer[loop]);
			if (hasCompress) {
				picLayerObj->layer[layerIdx].fbdc[fbdcIdx].compressed = 1;
				picLayerObj->layer[layerIdx].fbdc[fbdcIdx].wbType = RTKCOMPRESS;
				picLayerObj->layer[layerIdx].fbdc[fbdcIdx].comp_plane = offlinePlane;
				picLayerObj->layer[layerIdx].fbdc[fbdcIdx].plane = VO_GRAPHIC_OSD4;
				picLayerObj->layer[layerIdx].fbdc[fbdcIdx].offlineOrder = (E_BLEND_ORDER)offlineOrder;
				offlineOrder++;
				picLayerObj->layer[layerIdx].onlineOrder = C0;
			} else {
				picLayerObj->layer[layerIdx].fbdc[fbdcIdx].plane = onlinePlane;
				picLayerObj->layer[layerIdx].onlineOrder = (E_BLEND_ORDER)layerIdx;
			}

			picLayerObj->layer[layerIdx].fbdc_num++;
			fbdcIdx++;
		}

		if (data->layer[loop].src_type == SRC_NORMAL
		   || data->layer[loop].src_type == SRC_NORMAL_1) {
			GDMA_CopyLayerInfo(&picLayerObj->layer[layerIdx].normal[normalIdx], &data->layer[loop]);
			if (hasCompress) {
				picLayerObj->layer[layerIdx].normal[normalIdx].compressed = 1;
				picLayerObj->layer[layerIdx].normal[normalIdx].wbType = RTKCOMPRESS;
				picLayerObj->layer[layerIdx].normal[normalIdx].comp_plane = offlinePlane;
				picLayerObj->layer[layerIdx].normal[normalIdx].plane = VO_GRAPHIC_OSD4;
				picLayerObj->layer[layerIdx].normal[normalIdx].offlineOrder = (E_BLEND_ORDER)offlineOrder;
				offlineOrder++;
				picLayerObj->layer[layerIdx].onlineOrder = C0;
			} else {
				picLayerObj->layer[layerIdx].normal[normalIdx].plane = onlinePlane;
				if(data->layer[loop].src_type == SRC_NORMAL_1)
					picLayerObj->layer[layerIdx].onlineOrder = C2;
				else
				picLayerObj->layer[layerIdx].onlineOrder = (E_BLEND_ORDER)layerIdx;
			}

			picLayerObj->layer[layerIdx].normal_num++;
			normalIdx++;
		}


		if (data->layer[loop].src_type == SRC_MOUSE_NORMAL ||
		   data->layer[loop].src_type == SRC_NORMAL_1 ) {
		   if (normalIdx < 1) {
			GDMA_CopyLayerInfo(&picLayerObj->layer[layerIdx].normal[normalIdx], &data->layer[loop]);
			picLayerObj->layer[layerIdx].normal[normalIdx].compressed = 0;
			if (hasCompress)
				picLayerObj->layer[layerIdx].normal[normalIdx].plane = VO_GRAPHIC_OSD1;
			else
				picLayerObj->layer[layerIdx].normal[normalIdx].plane = onlinePlane;
			picLayerObj->layer[layerIdx].normal_num++;
		   }
			if(data->layer[loop].src_type == SRC_NORMAL_1)
				picLayerObj->layer[layerIdx].onlineOrder = C2;
			else
				picLayerObj->layer[layerIdx].onlineOrder = C4;
		}
		if (data->layer[loop].src_type == SRC_MOUSE_FBDC) {
			if (fbdcIdx < 3) {
				GDMA_CopyLayerInfo(&picLayerObj->layer[layerIdx].fbdc[fbdcIdx], &data->layer[loop]);
				picLayerObj->layer[layerIdx].fbdc[fbdcIdx].compressed = 0;
				if (hasCompress)
					picLayerObj->layer[layerIdx].fbdc[fbdcIdx].plane = VO_GRAPHIC_OSD1;
				else
					picLayerObj->layer[layerIdx].fbdc[fbdcIdx].plane = onlinePlane;
			}
			picLayerObj->layer[layerIdx].fbdc_num++;
			picLayerObj->layer[layerIdx].onlineOrder = C4;
		}
	}
	picLayerObj->layer_num = ++layerIdx;

	/*  collect syncstamp information */
	for (loop = 0; loop < picLayerObj->layer_num; loop++) {
		com_picObj = &picLayerObj->layer[loop];

		if (com_picObj->normal[0].syncInfo.syncstamp != 0 ||
			com_picObj->fbdc[0].syncInfo.syncstamp != 0) {
			if (com_picObj->fbdc_num > 0)
				syncinfo[loop].syncstamp = com_picObj->fbdc[0].syncInfo.syncstamp;
			else if (com_picObj->normal_num > 0)
				syncinfo[loop].syncstamp = com_picObj->normal[0].syncInfo.syncstamp;
			DBG_PRINT(KERN_EMERG"%s, loop=%d, syncstamp=%lld \n", __FUNCTION__, loop, syncinfo[loop].syncstamp);

			/*  search other layers */
			for (loop2 = 0; loop2 < picLayerObj->layer_num; loop2++) {
				com2_picObj = &picLayerObj->layer[loop2];
				if (com2_picObj->fbdc_num > 0)
					syncstamp_tmp = com2_picObj->fbdc[0].syncInfo.syncstamp;
				else if (com2_picObj->normal_num > 0)
					syncstamp_tmp = com2_picObj->normal[0].syncInfo.syncstamp;

				if ((syncstamp_tmp == syncinfo[loop].syncstamp) && (loop2 != loop)) {
					if (com2_picObj->fbdc_num > 0)
						plane_tmp = com2_picObj->fbdc[0].plane;
					else if (com2_picObj->normal_num > 0)
						plane_tmp = com2_picObj->normal[0].plane;

					syncinfo[loop].matchPlane[syncinfo[loop].matchNum] = plane_tmp;
					syncinfo[loop].matchNum++;
					DBG_PRINT(KERN_EMERG"%s, loop=%d, plane_tmp=%d, matchNum=%d \n", __FUNCTION__, loop, plane_tmp, syncinfo[loop].matchNum);
				}
			}
		}
	}

	/*  check conflict condition & assgin syncinfo */
	for (loop = 0; loop < picLayerObj->layer_num; loop++) {
		com_picObj = &picLayerObj->layer[loop];
		if (com_picObj->fbdc_num > 0)
			plane_tmp = com_picObj->fbdc[0].plane;
		else if (com_picObj->normal_num > 0)
			plane_tmp = com_picObj->normal[0].plane;

		for (loop2 = 0; loop2 < com_picObj->normal_num; loop2++)
			com_picObj->normal[loop2].syncInfo = syncinfo[loop];
		for (loop2 = 0; loop2 < com_picObj->fbdc_num; loop2++)
			com_picObj->fbdc[loop2].syncInfo = syncinfo[loop];

		for (loop2 = 0; loop2 < syncinfo[loop].matchNum; loop2++) {
			if (syncinfo[loop].matchPlane[loop2] == plane_tmp && hasCompress == 0)
				pr_debug(KERN_EMERG"%s, Conflict condition for GDMA..loop=%d, loop2=%d, plane_tmp=%d \n", __FUNCTION__, loop, loop2, plane_tmp);
		}
	}

	/*
	 *  if picture want to compress or blend, then go osd_comp_receive_picture
	 *  else call GDMA_ReceivePicture
	 */
	DBG_PRINT(KERN_EMERG"%s..Starting layers info. \n", __FUNCTION__);
	for (loop = 0; loop < picLayerObj->layer_num; loop++) {
		com_picObj = &picLayerObj->layer[loop];
		if (com_picObj->fbdc_num > 0)
			layer_obj = &com_picObj->fbdc[0];
		else if (com_picObj->normal_num > 0)
			layer_obj = &com_picObj->normal[0];
		else
			pr_debug(KERN_EMERG"Warning..does NOT have fbdc & normal layer, Please check it \n");

		if (com_picObj->fbdc_num > 0) {
			DBG_PRINT(KERN_EMERG"fbdc_num=%d\n", com_picObj->fbdc_num);
			loop2 = com_picObj->fbdc_num;
		} else if (com_picObj->normal_num > 0) {
			DBG_PRINT(KERN_EMERG"normal_num=%d\n", com_picObj->normal_num);
			loop2 = com_picObj->normal_num;
		}
		for (layerIdx = 0; layerIdx < loop2; layerIdx++) {
			if (com_picObj->fbdc_num > 0)
				layer_obj = &com_picObj->fbdc[layerIdx];
			else
				layer_obj = &com_picObj->normal[layerIdx];
			DBG_PRINT(KERN_EMERG"layerIdx=%d\n", layerIdx);
			DBG_PRINT(KERN_EMERG"src_type=%d, plane=%d, comp_plane=%d \n", layer_obj->src_type, layer_obj->plane, layer_obj->comp_plane);
			DBG_PRINT(KERN_EMERG"layer_used=%d, compressed=%d, offlineOrder=%d \n", layer_obj->layer_used, layer_obj->compressed, layer_obj->offlineOrder);
			DBG_PRINT(KERN_EMERG"x=%d, y=%d, width=%d, height=%d \n", layer_obj->x, layer_obj->y, layer_obj->width, layer_obj->height);
			DBG_PRINT(KERN_EMERG"dst_x=%d, dst_y=%d, dst_width=%d, dst_height=%d \n", layer_obj->dst_x, layer_obj->dst_y, layer_obj->dst_width, layer_obj->dst_height);
			DBG_PRINT(KERN_EMERG"address=0x%x, pitch=%d, onlineOrder=%d \n", layer_obj->address, layer_obj->pitch, picLayerObj->layer[loop].onlineOrder);
		}
		if (com_picObj->fbdc_num > 0)
			layer_obj = &com_picObj->fbdc[0];
		else if (com_picObj->normal_num > 0)
			layer_obj = &com_picObj->normal[0];

		/*  osd compress or blend */
		if (layer_obj->compressed) {
			sendCompLayerObj->layer[compLayerNum] = picLayerObj->layer[loop];
			compLayerNum++;
		} else { /*  gdma picture */
			sendGdmaLayerObj->layer[gdmaLayerNum] = picLayerObj->layer[loop];
			gdmaLayerNum++;
		}
	}
	DBG_PRINT(KERN_EMERG"%s...End of layers info. \n", __FUNCTION__);

	sendCompLayerObj->layer_num = compLayerNum;
	sendGdmaLayerObj->layer_num = gdmaLayerNum;

	DBG_PRINT(KERN_EMERG"%s, compLayerNum=%d, gdmaLayerNum=%d \n", __FUNCTION__, compLayerNum, gdmaLayerNum);

	/* only for HW cursor control and go online path */
	sendGdmaLayerObj->cookie = data->cookie;
	DBG_PRINT(KERN_EMERG"%s, cookie=%d \n", __FUNCTION__, data->cookie);

#ifdef USR_WORKQUEUE_UPDATE_BUFFER
	if(data->cookie == NONBLOCKING_COOKIE){
		up(&gdma->sem_gfxreceive);
		while(try_wait-- > 0){
			for(j=0;j<USE_TRIPLE_BUFFER_NUM;j++){
				if(sOSD1ReceiveWork[j].used == 0){
			down(&gdma->sem_work);
					sOSD1ReceiveWork[j].used = 1;
			up(&gdma->sem_work);
					queue_work_found = 1;
					break;
				}
			}
			if(queue_work_found)
				break;
			//usleep 2ms
			//printk("msleep func=%s line=%d \n",__FUNCTION__,__LINE__);
            gdma_usleep(TRIPLE_BUFFER_TRY_SLEEP*1000);
		}
		if(j >= USE_TRIPLE_BUFFER_NUM){
			printk("error No queue func=%s line=%d \n",__FUNCTION__,__LINE__);

			return err;
		}
		sOSD1ReceiveWork[j].data = sendGdmaLayerObj;
		queue_work(psOsd1RecWorkQueue, &sOSD1ReceiveWork[j].GdmaReceiveWork);
		err = GDMA_SUCCESS;
	}else{
		if (gdmaLayerNum > 0)
			err = GDMA_ReceivePicture(sendGdmaLayerObj);

		up(&gdma->sem_gfxreceive);
	}
#else
	up(&gdma->sem_gfxreceive);

	if (gdmaLayerNum > 0){
		err = GDMA_ReceivePicture(sendGdmaLayerObj);
		if(gdma_suspend_disable_osd == 1){
			GDMA_ConfigOSDxEnableFast(GDMA_PLANE_OSD1, 1);
			gdma_suspend_disable_osd = 0;			
		}
	}
#endif
	GDMA_PRINT(9, "-------%s------- gdma->sem_gfxreceive.count=%d \n", __FUNCTION__, gdma->sem_gfxreceive.count);

    // to signal gdma fence
    if (VALID_GDMA_MAGIC_KEY(data->gdma_fence_magic_key))
    {
        if (data->gdma_fence_handle) {
            GDMA_SignalFence((struct gdma_fence *) ((uintptr_t)data->gdma_fence_handle));
            //printk(KERN_EMERG"fence signal %llu\n", data->gdma_fence_handle);
        }
    }
     else { 
		GDMA_PRINT(3, "gdma magic key FAIL! (0x%8x)?\n", data->gdma_fence_magic_key);
   	}

	return err;
}


bool GDMA_OSDShiftUpscale(int disPlane, int h_shift_pixels,
						  int v_shift_pixels, int h_move_range,
						  int v_move_range, int apply)
{
	GDMA_WIN *win = NULL;
	gdma_dev *gdma = &gdma_devices[0];
	osd_sr_osd_sr_1_scaleup_ctrl0_RBUS osd_sr_scaleup_ctrl0_reg;
	osd_sr_osd_sr_1_scaleup_ctrl1_RBUS osd_sr_scaleup_ctrl1_reg;
	unsigned int ver_factor = 0;
	unsigned int hor_factor = 0;
	GDMA_PIC_DATA *curPic;
	GDMA_REG_CONTENT *pReg = NULL;
	unsigned int onlineProgDone;
	dma_addr_t addr;

		
	if (gdma == NULL) {
		printk(KERN_ERR"[%s] gdma device is NULL!!\n", __FUNCTION__);
		return false;
	}
	
	curPic = gdma->pic[disPlane] + gdma->curPic[disPlane];
	pReg = &curPic->reg_content;
	
    if (apply)
		g_osdshift_upscale_ctrl.apply_enable = 0;
	else
		g_osdshift_upscale_ctrl.apply_enable = 1;
	
	if (disPlane == 1) {
		g_osdshift_upscale_ctrl.plane = GDMA_PLANE_OSD1;
		win = (GDMA_WIN *)phys_to_virt(rtd_inl(GDMA_OSD1_WI_reg));
	} else if (disPlane == 2) {
		g_osdshift_upscale_ctrl.plane = GDMA_PLANE_OSD2;
		win = (GDMA_WIN *)phys_to_virt(rtd_inl(GDMA_OSD2_WI_reg));
	} else if (disPlane == 3) {
		g_osdshift_upscale_ctrl.plane = GDMA_PLANE_OSD3;
		win = (GDMA_WIN *)phys_to_virt(rtd_inl(GDMA_OSD3_WI_reg));
	} else {
		printk(KERN_ERR"[%s] no OSD-%d, please check the plane setting!!\n", __FUNCTION__, disPlane);
		memset(&g_osdshift_upscale_ctrl, 0x00, sizeof(CONFIG_OSDSHIFT_UPSCALE_STRUCT));
		return false;
	}
	
	if (win == NULL) {
		printk(KERN_ERR"[%s] win is NULL!!\n", __FUNCTION__);
		return false;
	}

	if (h_shift_pixels != 0) {
		g_osdshift_upscale_ctrl.shift_h_enable = 1;
	} else {
		g_osdshift_upscale_ctrl.shift_h_enable = 0;
	}

	if (v_shift_pixels != 0) {
		g_osdshift_upscale_ctrl.shift_v_enable = 1;
	} else {
		g_osdshift_upscale_ctrl.shift_v_enable = 0;
	}

	g_osdshift_upscale_ctrl.h_shift_pixel = h_shift_pixels;
	g_osdshift_upscale_ctrl.v_shift_pixel = v_shift_pixels;

	g_osdshift_upscale_ctrl.crop_left = ((abs(h_shift_pixels) * g_osdshift_upscale_ctrl.ini_w) << 8) / (g_osdshift_upscale_ctrl.ini_dst_w + h_move_range);
	g_osdshift_upscale_ctrl.crop_up = ((abs(v_shift_pixels) * g_osdshift_upscale_ctrl.ini_h) << 8) / (g_osdshift_upscale_ctrl.ini_dst_h + v_move_range);

	down(&gdma->sem_receive);

	GDMA_ENTER_CRITICAL();
	
	printk(KERN_ERR"[%s] x = %d, y = %d, crop_left = %d, crop_up = %d\n", __FUNCTION__, h_shift_pixels, v_shift_pixels, abs(g_osdshift_upscale_ctrl.crop_left) >> 8, abs(g_osdshift_upscale_ctrl.crop_up) >> 8);

	g_osdshift_upscale_ctrl.apply_afbc_x_sta = 0;
	g_osdshift_upscale_ctrl.apply_afbc_y_sta = 0;

	if (g_osdshift_upscale_ctrl.shift_h_enable || g_osdshift_upscale_ctrl.shift_v_enable) {
		if (g_osdshift_upscale_ctrl.shift_h_enable && g_osdshift_upscale_ctrl.shift_v_enable) {
			if (!fbcOnOff) {
				win->top_addr = g_osdshift_upscale_ctrl.ini_addr + (unsigned int)(((abs(g_osdshift_upscale_ctrl.crop_left) >> 8) * 4) + ((abs(g_osdshift_upscale_ctrl.crop_up) >> 8) * (int)win->pitch));
			} else {
				if (apply) {
					win->afbc_xy_sta.x_sta = abs(g_osdshift_upscale_ctrl.crop_left) >> 8;
					win->afbc_xy_sta.y_sta = abs(g_osdshift_upscale_ctrl.crop_up) >> 8;
				} else {
					g_osdshift_upscale_ctrl.apply_afbc_x_sta = abs(g_osdshift_upscale_ctrl.crop_left) >> 8;
					g_osdshift_upscale_ctrl.apply_afbc_y_sta = abs(g_osdshift_upscale_ctrl.crop_up) >> 8;
				}
			}
		} else if (g_osdshift_upscale_ctrl.shift_h_enable) {
			if (apply) {
				win->afbc_xy_sta.x_sta = abs(g_osdshift_upscale_ctrl.crop_left) >> 8;
			} else {
				g_osdshift_upscale_ctrl.apply_afbc_x_sta = abs(g_osdshift_upscale_ctrl.crop_left) >> 8;						
			}
		} else if (g_osdshift_upscale_ctrl.shift_v_enable) {
			if (apply) {
				win->afbc_xy_sta.y_sta = abs(g_osdshift_upscale_ctrl.crop_up) >> 8;
			} else {
				g_osdshift_upscale_ctrl.apply_afbc_y_sta = abs(g_osdshift_upscale_ctrl.crop_up) >> 8;
			}
		}
	} else {
		if (!fbcOnOff) {
			win->top_addr = g_osdshift_upscale_ctrl.ini_addr;
		} else {
			if (apply) {
				win->afbc_xy_sta.x_sta = 0;
				win->afbc_xy_sta.y_sta = 0;
			} else {
				g_osdshift_upscale_ctrl.apply_afbc_x_sta = 0;
				g_osdshift_upscale_ctrl.apply_afbc_y_sta = 0;
			}
		}
	}
	
	if (apply) {
		win->dst_width = g_osdshift_upscale_ctrl.ini_dst_w + h_move_range;
		win->dst_height = g_osdshift_upscale_ctrl.ini_dst_h + v_move_range;
	} else {
		g_osdshift_upscale_ctrl.apply_dst_w = g_osdshift_upscale_ctrl.ini_dst_w + h_move_range;
		g_osdshift_upscale_ctrl.apply_dst_h = g_osdshift_upscale_ctrl.ini_dst_h + v_move_range;
	}
	
	hor_factor = (g_osdshift_upscale_ctrl.ini_w << 20) / (g_osdshift_upscale_ctrl.ini_dst_w + h_move_range); // rounding
	ver_factor = (g_osdshift_upscale_ctrl.ini_h << 20) / (g_osdshift_upscale_ctrl.ini_dst_h + v_move_range); // rounding

	rtd_outl(GDMA_OSD1_SIZE_reg, GDMA_OSD1_SIZE_w(win->winWH.width) | GDMA_OSD1_SIZE_h(win->winWH.height));
#if 1
	addr = dma_map_single(gdma->dev, (void *)win, sizeof(GDMA_WIN), DMA_TO_DEVICE);
	dma_sync_single_for_device(gdma->dev, addr, sizeof(GDMA_WIN), DMA_TO_DEVICE);
	dma_unmap_single(gdma->dev, addr, sizeof(GDMA_WIN), DMA_TO_DEVICE);
#endif
	if (apply) {	
	// osd_sr setting
		osd_sr_scaleup_ctrl1_reg.h_zoom_en = 0;
		osd_sr_scaleup_ctrl1_reg.hor_factor = 0;
		osd_sr_scaleup_ctrl0_reg.v_zoom_en = 0;
		osd_sr_scaleup_ctrl0_reg.ver_factor = 0;

		osd_sr_scaleup_ctrl1_reg.hor_ini = (h_shift_pixels * g_osdshift_upscale_ctrl.ini_w << 8) / 
			(g_osdshift_upscale_ctrl.ini_dst_w + h_move_range); // rounding
		osd_sr_scaleup_ctrl1_reg.hor_ini %= 1 << 8;
		osd_sr_scaleup_ctrl0_reg.ver_ini = (v_shift_pixels * g_osdshift_upscale_ctrl.ini_h << 8) /
			(g_osdshift_upscale_ctrl.ini_dst_h + h_move_range); // rounding
		osd_sr_scaleup_ctrl0_reg.ver_ini %= 1 << 8;

		printk(KERN_ERR"[%s] width = %d, win->dst_width = %d, hor_factor = 0x%x, h_ini = 0x%x, crop_left = %d\n", __FUNCTION__, win->winWH.width, win->dst_width, hor_factor, osd_sr_scaleup_ctrl1_reg.hor_ini, g_osdshift_upscale_ctrl.crop_left);
		printk(KERN_ERR"[%s] height = %d, win->dst_height = %d, ver_factor = 0x%x, v_ini = 0x%x, crop_up = %d\n", __FUNCTION__, win->winWH.height, win->dst_height, ver_factor, osd_sr_scaleup_ctrl0_reg.ver_ini, g_osdshift_upscale_ctrl.crop_up);

		if ((win->winWH.width < win->dst_width) && (hor_factor)) {
			osd_sr_scaleup_ctrl1_reg.h_zoom_en = 1;
			osd_sr_scaleup_ctrl1_reg.hor_factor = hor_factor;
		}

		pReg->osd_sr_scaleup_ctrl1_reg.regValue = osd_sr_scaleup_ctrl1_reg.regValue;
	
		if ((win->winWH.height < win->dst_height) && (ver_factor)) {
			osd_sr_scaleup_ctrl0_reg.v_zoom_en = 1;
			osd_sr_scaleup_ctrl0_reg.ver_factor = ver_factor;
		}
	
		pReg->osd_sr_scaleup_ctrl0_reg.regValue = osd_sr_scaleup_ctrl0_reg.regValue;

		drv_scaleup(1, g_osdshift_upscale_ctrl.plane);

		onlineProgDone = GDMA_CTRL_write_data(1);

		if (disPlane == 1) 
			onlineProgDone |= GDMA_CTRL_osd1_prog_done(1);
		else if (disPlane == 2)
			onlineProgDone |= GDMA_CTRL_osd2_prog_done(1);
		else if (disPlane == 3)
			onlineProgDone |= GDMA_CTRL_osd3_prog_done(1);

		rtd_outl(GDMA_CTRL_reg, onlineProgDone);
	} else {
		g_osdshift_upscale_ctrl.apply_hor_factor = hor_factor;
		g_osdshift_upscale_ctrl.apply_ver_factor = ver_factor;
		g_osdshift_upscale_ctrl.apply_hor_ini = 
			(h_shift_pixels * g_osdshift_upscale_ctrl.ini_w << 8) / (g_osdshift_upscale_ctrl.ini_dst_w + h_move_range); // rounding;
		g_osdshift_upscale_ctrl.apply_hor_ini %= 1 << 8;
		g_osdshift_upscale_ctrl.apply_ver_ini = 
			(v_shift_pixels * g_osdshift_upscale_ctrl.ini_h << 8) / (g_osdshift_upscale_ctrl.ini_dst_h + h_move_range); // rounding
		g_osdshift_upscale_ctrl.apply_ver_ini %= 1 << 8;
	}
	
	up(&gdma->sem_receive);
	
	GDMA_EXIT_CRITICAL();
	
	if (used_glastinfo)
		GDMA_ReceivePicture(&glastinfo);
	
	return true;
}
bool GDMA_OSDShiftUpscale_wrapper(int disPlane, int h_shift_pixels,
						  int v_shift_pixels, int h_move_range,
						  int v_move_range, int apply)
{
	g_osdshift_upscale_para.disPlane = disPlane;
	g_osdshift_upscale_para.h_shift_pixels = h_shift_pixels;	
	g_osdshift_upscale_para.v_shift_pixels = v_shift_pixels;
	g_osdshift_upscale_para.h_move_range = h_move_range;
	g_osdshift_upscale_para.v_move_range = v_move_range;
	g_osdshift_upscale_para.apply = apply;
	up(&sem_gdma_def);
	
	return true;
}

EXPORT_SYMBOL(GDMA_OSDShiftUpscale_wrapper);
#if 0
static int gdma_def_func(void* ptr) {

	while(1) {

		if(down_timeout(&sem_gdma_def, 2*HZ)) {
			continue;
		}
		GDMA_OSDShiftUpscale(g_osdshift_upscale_para.disPlane,
							g_osdshift_upscale_para.h_shift_pixels,
						   g_osdshift_upscale_para.v_shift_pixels,
						  g_osdshift_upscale_para.h_move_range,
						 g_osdshift_upscale_para.v_move_range,
						g_osdshift_upscale_para.apply);
	}
	return 0;
};
#endif

void GDMA_ConfigRGBGain(unsigned int on_off, unsigned int RGB_level)
{
	osd_gain_info.on_off = on_off;
	
	if (RGB_level > 255)
		printk(KERN_ERR"%s input gain value error! gain = 0x%x\n", __func__, RGB_level);
	else
		osd_gain_info.levelval = (~RGB_level) & 0xFF;
	
	if (used_glastinfo)
		GDMA_ReceivePicture(&glastinfo);
}

void GDMA_GetGlobalAlpha(unsigned int *alpha, int count)
{
	int i = 0;
	unsigned int alpha_range[8];
	unsigned int average_alpha[8];
	unsigned int total_alpha = 0;
	unsigned int result;

	alpha_range[0] = OSDOVL_alpha_osd_detect_0_get_thd0(rtd_inl(OSDOVL_alpha_osd_detect_0_reg));
	alpha_range[1] = OSDOVL_alpha_osd_detect_0_get_thd1(rtd_inl(OSDOVL_alpha_osd_detect_0_reg));
	alpha_range[2] = OSDOVL_alpha_osd_detect_0_get_thd2(rtd_inl(OSDOVL_alpha_osd_detect_0_reg));
	alpha_range[3] = OSDOVL_alpha_osd_detect_0_get_thd3(rtd_inl(OSDOVL_alpha_osd_detect_0_reg));
	alpha_range[4] = OSDOVL_alpha_osd_detect_1_get_thd4(rtd_inl(OSDOVL_alpha_osd_detect_1_reg));
	alpha_range[5] = OSDOVL_alpha_osd_detect_1_get_thd5(rtd_inl(OSDOVL_alpha_osd_detect_1_reg));
	alpha_range[6] = OSDOVL_alpha_osd_detect_1_get_thd6(rtd_inl(OSDOVL_alpha_osd_detect_1_reg));
	alpha_range[7] = OSDOVL_alpha_osd_detect_1_get_thd7(rtd_inl(OSDOVL_alpha_osd_detect_1_reg));
	
	average_alpha[0] = alpha_range[0] / 2;

	for (i = 1; i < ALPHA_THRESHOLD_NUM; i++) {
		if (i == (ALPHA_THRESHOLD_NUM - 1))
			average_alpha[i] = 0xff;
		else
			average_alpha[i] = alpha_range[i-1] + (alpha_range[i] - alpha_range[i-1]) / 2;
	}
	
	for (i = 0; i < ALPHA_THRESHOLD_NUM; i++)
		total_alpha += average_alpha[i] * alpha_osd_detect_cnt[i];
	
	result = total_alpha / GDMA_PM_ALPHA_ALL_ZERO_NUM;
	if((result == 0) && (total_alpha!=0)){
		if(alpha_osd_detect_cnt[0] < GDMA_PM_ALPHA_ALL_ZERO_NUM)
			result = 1;
	}
	
    if (count > 0 && count <= 16) {
		for (i = 0; i < count; i++)
			*(alpha+i) = result; //total_alpha / GDMA_PM_ALPHA_ALL_ZERO_NUM;
	}
}

int GDMA_CaptureFramebuffer(KGAL_SURFACE_INFO_T *pDstSurfaceInfo)
{
	KGAL_PIXEL_FORMAT_T pixelFormat;
	KGAL_SURFACE_INFO_T srcSurface;
	KGAL_RECT_T srcRect;
	KGAL_RECT_T dstRect;
	KGAL_BLIT_FLAGS_T blitFlags;
	KGAL_BLIT_SETTINGS_T blitSettings;
	KGAL_DRAW_FLAGS_T drawFlags;
	KGAL_DRAW_SETTINGS_T drawSettings;

	unsigned int width, height, pitch, top_addr;
	int compress;
	int ret = 0;

	width = GDMA_DBG_OSD_WI_0_get_width(rtd_inl(GDMA_DBG_OSD_WI_0_reg));
	height = GDMA_DBG_OSD_WI_0_get_height(rtd_inl(GDMA_DBG_OSD_WI_0_reg));
	pitch = GDMA_DBG_OSD_WI_4_get_pitch(rtd_inl(GDMA_DBG_OSD_WI_4_reg));
	top_addr = GDMA_DBG_OSD_WI_2_get_top_addr(rtd_inl(GDMA_DBG_OSD_WI_2_reg));
	compress = GDMA_DBG_OSD_WI_1_get_img_compress(rtd_inl(GDMA_DBG_OSD_WI_1_reg));

	if (GDMA_DBG_OSD_WI_1_get_colortype(rtd_inl(GDMA_DBG_OSD_WI_1_reg)) == VO_OSD_COLOR_FORMAT_ARGB8888 ||
	GDMA_DBG_OSD_WI_1_get_colortype(rtd_inl(GDMA_DBG_OSD_WI_1_reg)) == VO_OSD_COLOR_FORMAT_RGBA8888) {
		pixelFormat = KGAL_PIXEL_FORMAT_ARGB;
	} else {
		printk(KERN_ERR"%s: unsupport pixel format %d\n", __func__, GDMA_DBG_OSD_WI_1_get_colortype(rtd_inl(GDMA_DBG_OSD_WI_1_reg)));
		return GDMA_FAIL;
	}

	memset(&srcSurface, 0, sizeof(KGAL_SURFACE_INFO_T));
	memset(&srcRect, 0, sizeof(KGAL_RECT_T));
	memset(&dstRect, 0, sizeof(KGAL_RECT_T));
	memset(&blitFlags, 0, sizeof(KGAL_BLIT_FLAGS_T));
	memset(&blitSettings, 0, sizeof(KGAL_BLIT_SETTINGS_T));
	memset(&drawFlags, 0, sizeof(KGAL_DRAW_FLAGS_T));
	memset(&drawSettings, 0, sizeof(KGAL_DRAW_SETTINGS_T));

	blitFlags = KGAL_BLIT_NOFX;

	blitSettings.srcBlend = KGAL_BLEND_ONE;
	blitSettings.dstBlend = KGAL_BLEND_ZERO;

	if(compress)
		top_addr = top_addr - (width * height / (16 * 4) / 2 * 16);

	srcSurface.physicalAddress = top_addr;
	srcSurface.width = width;
	srcSurface.height = height;
	srcSurface.bpp = (pitch / width) * 8;
	srcSurface.pitch = pitch;
	if (compress)
		srcSurface.pixelFormat = KGAL_PIXEL_FORMAT_FBC;
	else
		srcSurface.pixelFormat = pixelFormat;

	srcRect.x = 0;
	srcRect.y = 0;
	srcRect.w = srcSurface.width;
	srcRect.h = srcSurface.height;

	if (pDstSurfaceInfo->width == srcSurface.width && pDstSurfaceInfo->height == srcSurface.height) {
		ret = KGAL_Blit(&srcSurface, &srcRect, pDstSurfaceInfo, 0, 0, &blitFlags, &blitSettings);
	} else if (srcSurface.width == 960 && srcSurface.height == 2160) { // for e-sticker
		drawFlags = KGAL_DRAW_NOFX;

		drawSettings.srcBlend = KGAL_BLEND_ONE;
		drawSettings.dstBlend = KGAL_BLEND_ZERO;

		dstRect.x = 0;
		dstRect.y = 0;
		dstRect.w = pDstSurfaceInfo->width;
		dstRect.h = pDstSurfaceInfo->height;

		ret = KGAL_FillRectangle(pDstSurfaceInfo, &dstRect, 0x00000000, &drawFlags, &drawSettings);
	} else if (pDstSurfaceInfo->width <= srcSurface.width && pDstSurfaceInfo->height <= srcSurface.height && pDstSurfaceInfo->width >= 0x10) {
		dstRect.x = 0;
		dstRect.y = 0;
		dstRect.w = pDstSurfaceInfo->width;
		dstRect.h = pDstSurfaceInfo->height;
		ret = KGAL_StretchBlit(&srcSurface, &srcRect, pDstSurfaceInfo, &dstRect, &blitFlags, &blitSettings);
	} else {
		printk(KERN_ERR"%s size check error, src(%d, %d), dst(%d, %d)!!!\n", __FUNCTION__, srcSurface.width, srcSurface.height, pDstSurfaceInfo->width, pDstSurfaceInfo->height);
		return GDMA_FAIL;
	}

	if (ret != 0 && ret != 1) {
		printk(KERN_ERR"%s fail:%d!!!\n", __FUNCTION__, ret);
		return ret;
	}
	//printk(KERN_EMERG"%s win addr = 0x%x %d(%d, %d) compress = %d\n", __func__, top_addr, pixelFormat, width, height, (int)compress);
	//printk(KERN_EMERG"%s dst addr = 0x%x %d(%d, %d)\n", __func__, pDstSurfaceInfo->physicalAddress, pDstSurfaceInfo->pixelFormat, pDstSurfaceInfo->width, pDstSurfaceInfo->height);
	return GDMA_SUCCESS;
}

void GDMA_ConfigEstickerPosition(int eightk)
{
	unsigned int *viraddr = NULL;
	GDMA_WIN *win;

	g_esticker_info.pre_data = g_esticker_info.cur_data;

	win = (GDMA_WIN *)phys_to_virt(rtd_inl(GDMA_OSD1_WI_reg));

	if (win == NULL) {
		printk(KERN_ERR"[%s:%d] win is NULL\n", __func__, __LINE__);
	}

	viraddr = (unsigned int *)dvr_remap_uncached_memory(win->top_addr, 0x1000, __builtin_return_address(0));

	if (viraddr == NULL) {
		printk(KERN_ERR"[%s:%d] dvr_remap_uncached_memory 0x%x error\n", __func__, __LINE__, win->top_addr);
	} else {
		//printk(KERN_EMERG"[%s:%d] *viraddr = 0x%x\n", __func__, __LINE__, *viraddr);

		if (*viraddr >= 0x60000000) {
			g_esticker_info.cur_data = 1;
		} else if (*viraddr <= 0x0000100) {
			g_esticker_info.cur_data = 0;
		}
		//printk(KERN_EMERG"[%s:%d] cur_data = %d, pre_data = %d\n", __func__, __LINE__, g_esticker_info.cur_data,g_esticker_info.pre_data);
		//printk(KERN_EMERG"[%s:%d] g_esticker_info.cur_position = %d change=%d\n", __func__, __LINE__, g_esticker_info.cur_position,g_esticker_info.change);
		if (g_esticker_info.pre_data == 1 && g_esticker_info.cur_data == 0) {
			g_esticker_info.change = 3;
			if (g_esticker_info.cur_position == 150 || g_esticker_info.cur_position == 300) {
				if (eightk)
					g_esticker_info.cur_position = 5760;//ALIGN_SIZE(5824, 16); // 7680-((778+150)*2)
				else
					g_esticker_info.cur_position = 2912; // 3840-(778+150)
			} else {
				if (eightk)
					g_esticker_info.cur_position = 300;
				else
					g_esticker_info.cur_position = 150;
			}
		} //else
			//g_esticker_info.change = 0;
		//printk(KERN_EMERG"[%s:%d] g_esticker_info.cur_position = %d change=%d\n", __func__, __LINE__, g_esticker_info.cur_position,g_esticker_info.change);
		dvr_unmap_memory((void *)viraddr, 0x1000);
	}
}


/** @brief trigger the OSD smooth toggle apply bit
 *  @param None
 *  @return None
 */
void GDMA_OSDSmoothToggle(void)
{
#if 0	//fix me later by ben
	rtd_outl(PPOVERLAY_osd_split_smooth_toggle_reg, PPOVERLAY_osd_split_smooth_toggle_osd_split_smooth_toggle_apply(1));
#endif
}


extern unsigned int Get_DISPLAY_CLOCK_TYPICAL(void);

/** @brief OSD/GDMA clock control
 *  @param src_type	0: for gdma; 1: for osd_split
 *  @param enable	 true for enable; false for disable
 *  @return None
 */
void GDMA_Clk_Select(int src_type, bool enable)
{
	sys_reg_sys_dclk_gate_sel0_RBUS sys_reg_sys_dclk_gate_sel0_reg;
	unsigned int disp_clk = 0;

	/* enable/disable clken_disp_gdma */
	if (enable == true)
		CRT_CLK_OnOff(GDMA, CLK_ON, &src_type);
	if (enable == false)
		CRT_CLK_OnOff(GDMA, CLK_OFF, &src_type);

	sys_reg_sys_dclk_gate_sel0_reg.regValue = rtd_inl(SYS_REG_SYS_DCLK_GATE_SEL0_reg);

	/* pick higher freq. as GDMA clock source */
#ifdef CONFIG_SUPPORT_SCALER
	disp_clk = Get_DISPLAY_CLOCK_TYPICAL();
//if(sys_reg_sys_dclk_gate_sel0_reg.dclk_s2_f2p_gate_sel == 1)//1 : 60hz  0:120Hz
#elif defined(CONFIG_RTK_KDRIVER_SUPPORT)
	disp_clk = pll_info_disp();
#endif

	if( pll_info_bush() > disp_clk )
     {
		rtd_outl(SYS_REG_SYS_DISPCLKSEL_reg, rtd_inl(SYS_REG_SYS_DISPCLKSEL_reg) & ~(1<<14));
	}
	else
	{
		rtd_outl(SYS_REG_SYS_DISPCLKSEL_reg, rtd_inl(SYS_REG_SYS_DISPCLKSEL_reg) | (1<<14));
	}


	printk(KERN_ERR"[%s]clk value = %08x, 0114 = %08x, 0104 = %08x disp:%d bush:%d panel:%d\n", __func__, rtd_inl(SYS_REG_SYS_DISPCLKSEL_reg),
	rtd_inl(SYS_REG_SYS_CLKEN1_reg),rtd_inl(SYS_REG_SYS_SRST1_reg),
	pll_info_disp(), pll_info_bush(), disp_clk
	);

}

int GDMA_AI_Get_UI_size(int *ui_size_w,int *ui_size_h)
{
	if(change_OSD_mode == GDMA_SPLIT_MODE_2 ||
	  change_OSD_mode == GDMA_SPLIT_MODE_3)
	{
		*ui_size_w = AI_WIDTH_1920;
		*ui_size_h = AI_HEIGH_1080;
	}else{
		*ui_size_w = AI_WIDTH;
		*ui_size_h = AI_HEIGH;
	}
	return 1;
}
int GDMA_AI_SE_draw_block_afbc(int s_w,int s_h,int num,
							   unsigned int *color,KGAL_RECT_T *ai_block, unsigned char draw_scene) 
{
	//unsigned int i;
	unsigned int dst_phy;
	KGAL_SURFACE_INFO_T dsurf;
	KGAL_RECT_T drect;
	KGAL_DRAW_FLAGS_T sflag = KGAL_DRAW_NOFX;
	KGAL_DRAW_SETTINGS_T sblend;
	GDMA_PICTURE_OBJECT *picObj;
	VO_RECTANGLE disp_res;
	osd_planealpha_ar osdcmp_plane_ar;
	osd_planealpha_gb osdcmp_plane_gb;
	
	memset(&dsurf,0, sizeof(KGAL_SURFACE_INFO_T));
	memset(&drect,0, sizeof(KGAL_RECT_T));
	memset(&sblend,0, sizeof(KGAL_DRAW_SETTINGS_T));
	sblend.srcBlend = KGAL_BLEND_ONE;
	sblend.dstBlend = KGAL_BLEND_ZERO;

	
    if(dst_order == 0){	
		if(dst_AI == 0){
			dst_AI = dvr_malloc_specific(s_w*s_h*4*2, GFP_DCU1);
		}
		if(dst_AI == 0)
			return 0;
		dst_phy = virt_to_phys(dst_AI);
		//memset(dst_AI,0x0,s_w*s_h*2);
		
		dst_order = 1;
	}else{
		if(dst_AI_2 == 0){
                dst_AI_2 = dvr_malloc_specific(s_w*s_h*4*2, GFP_DCU1);
		}
		if(dst_AI_2 == 0)
			return 0;
		dst_phy = virt_to_phys((void *)dst_AI_2);
		//memset(dst_AI,0x0,s_w*s_h*2);
		dst_order = 0;
	}

	//memset(dst_AI,0x80,s_w*s_h*2);
	dsurf.physicalAddress = dst_phy;
	dsurf.width = s_w;
	dsurf.height = s_h;
	dsurf.bpp = 32;//16;
	dsurf.pitch = s_w;//s_w*2;
	dsurf.pixelFormat = KGAL_PIXEL_FORMAT_FBC;//KGAL_PIXEL_FORMAT_ARGB4444;

	drect.x = 0;
	drect.y = 0;
	drect.w = s_w;
	drect.h = s_h;

	KGAL_FillRectangle(&dsurf,
					   &drect,
					   0x00000000,
					   &sflag,
					   &sblend);	
	drect.x = 64;
	drect.y = 64;
	drect.w = 64;
	drect.h = 64;
	KGAL_FillRectangle(&dsurf,
					   &drect,
					   0xff0000ff,
					   &sflag,
					   &sblend);	
#if 0
	for(i=0;i<num;i++)
	{
		drect.x = ai_block[i].x;
		drect.y = ai_block[i].y;
		drect.w = ai_block[i].w;
		drect.h = ai_block[i].h;
			KGAL_DrawRectangle(&dsurf,&drect, color[i], &sflag, &sblend);

	}
#endif
	KGAL_SyncGraphic_block_mode(1);
	
	memset(&patlayerAI,0x0,sizeof(PICTURE_LAYERS_OBJECT));
	picObj = &patlayerAI.layer[0].fbdc[0];

	picObj->format = VO_OSD_COLOR_FORMAT_ARGB8888;
	//if(get_platform() == PLATFORM_K6HP){
	//	gdma_config_line_buffer_for_AI();
	picObj->plane = VO_GRAPHIC_OSD2;
	patlayerAI.layer[0].onlineOrder = C0;
	//picObj->plane = VO_GRAPHIC_OSD1;
	//patlayerAI.layer[0].onlineOrder = C0;
	//}else{
	//	picObj->plane = VO_GRAPHIC_OSD2;
	//	patlayerAI.layer[0].onlineOrder = C1;
	//}
	patlayerAI.layer_num = 1;
	patlayerAI.layer[0].fbdc_num = 1;
	picObj->src_type = SRC_FBDC;
	picObj->alpha = 0;
	picObj->width = s_w;
	picObj->height = s_h;
	picObj->address = dst_phy;
	picObj->pitch = s_w;
	picObj->layer_used = 1;	
	picObj->show = 1;
	picObj->paletteformat = 1; /* rgba format */
	picObj->syncInfo.syncstamp = 0;
	picObj->scale_factor = 1;
	getDispSize(&disp_res);	
	picObj->dst_width = disp_res.width;
	picObj->dst_height = disp_res.height;
	osdcmp_plane_ar.plane_alpha_r = 0xff;
	osdcmp_plane_ar.plane_alpha_a = 0xff;
	osdcmp_plane_gb.plane_alpha_b = 0xff;
	osdcmp_plane_gb.plane_alpha_g = 0xff;
	
	picObj->plane_ar = osdcmp_plane_ar;
	picObj->plane_gb = osdcmp_plane_gb;
	
	GDMA_ReceivePicture(&patlayerAI);

	if(num == 0){
		GDMA_ConfigOSDxEnableFast(GDMA_PLANE_OSD2,false);

		//gdma_config_line_buffer();
		if(dst_AI != 0){
			dvr_free(dst_AI);
			dst_AI = 0;
		}
		if(dst_AI_2 != 0){
			dvr_free(dst_AI_2);
			dst_AI_2 = 0;
		}
		AI_Enable = 0;
	}else{
		if(AI_Enable == 0){
			GDMA_ConfigOSDxEnableFast(GDMA_PLANE_OSD2,true);
			AI_Enable = 1;
		}
	}	
	return 1;
}

int GDMA_AI_SE_draw_block(int s_w,int s_h,int num,
						  unsigned int *color,KGAL_RECT_T *ai_block, unsigned char draw_scene) 
{
	unsigned int i;
	unsigned int dst_phy;
	KGAL_SURFACE_INFO_T dsurf;
	KGAL_RECT_T drect;
	KGAL_DRAW_FLAGS_T sflag = KGAL_DRAW_NOFX;
	KGAL_DRAW_SETTINGS_T sblend;
	GDMA_PICTURE_OBJECT *picObj;
	VO_RECTANGLE disp_res;
	osd_planealpha_ar osdcmp_plane_ar;
	osd_planealpha_gb osdcmp_plane_gb;
	
	memset(&dsurf,0, sizeof(KGAL_SURFACE_INFO_T));
	memset(&drect,0, sizeof(KGAL_RECT_T));
	memset(&sblend,0, sizeof(KGAL_DRAW_SETTINGS_T));
	sblend.srcBlend = KGAL_BLEND_ONE;
	sblend.dstBlend = KGAL_BLEND_ZERO;

	//check if need to release memory
	if (ai_draw_by != AI_DRAW_BY_BLOCK){
		if(dst_AI){
			dvr_free(dst_AI);
			dst_AI=0;
		}
		if(dst_AI_2){
			dvr_free(dst_AI_2);
			dst_AI_2=0;
		}
		ai_draw_by=AI_DRAW_BY_BLOCK;
	}

	
    if(dst_order == 0){	
		if(dst_AI == 0){
			if(change_OSD_mode == GDMA_SPLIT_MODE_2
			|| change_OSD_mode == GDMA_SPLIT_MODE_3)
			dst_AI = dvr_malloc_specific(s_w*s_h*2, GFP_DCU1);
			else
			    dst_AI = kmalloc(s_w*s_h*2,GFP_KERNEL);

		}
		if(dst_AI == 0)
			return 0;
		dst_phy = dvr_to_phys((void *)dst_AI);
		//memset(dst_AI,0x80,s_w*s_h*2);
		dst_order = 1;
	}else{
		if(dst_AI_2 == 0){
			if(change_OSD_mode == GDMA_SPLIT_MODE_2
			|| change_OSD_mode == GDMA_SPLIT_MODE_3)
                dst_AI_2 = dvr_malloc_specific(s_w*s_h*2, GFP_DCU1);
			else
			    dst_AI_2 = kmalloc(s_w*s_h*2,GFP_KERNEL);

		}
		if(dst_AI_2 == 0)
			return 0;
		dst_phy = dvr_to_phys((void *)dst_AI_2);
		//memset(dst_AI,0x80,s_w*s_h*2);
		dst_order = 0;
	}

	//memset(dst_AI,0x80,s_w*s_h*2);
	dsurf.physicalAddress = dst_phy;
	dsurf.width = s_w;
	dsurf.height = s_h;
	dsurf.bpp = 16;
	dsurf.pitch = s_w*2;
	dsurf.pixelFormat = KGAL_PIXEL_FORMAT_ARGB4444;

	drect.x = 0;
	drect.y = 0;
	drect.w = s_w;
	drect.h = s_h;

	KGAL_FillRectangle(&dsurf,
						   &drect,
						   0x00000000,
						   &sflag,
						   &sblend);	
	for(i=0;i<num;i++)
	{
		drect.x = ai_block[i].x;
		drect.y = ai_block[i].y;
		drect.w = ai_block[i].w;
		drect.h = ai_block[i].h;
		if(i!=num-1 || draw_scene==0)
			KGAL_DrawRectangle(&dsurf,&drect, color[i], &sflag, &sblend);
		else
			KGAL_FillRectangle(&dsurf,&drect, color[i], &sflag, &sblend);
	}
	KGAL_SyncGraphic_block_mode(1);
	
	memset(&patlayerAI,0x0,sizeof(PICTURE_LAYERS_OBJECT));
	picObj = &patlayerAI.layer[0].normal[0];

	picObj->format = VO_OSD_COLOR_FORMAT_ARGB4444;
	//if(get_platform() == PLATFORM_K6HP){
	//	gdma_config_line_buffer_for_AI();
		picObj->plane = VO_GRAPHIC_OSD2;
		patlayerAI.layer[0].onlineOrder = C0;
		//picObj->plane = VO_GRAPHIC_OSD1;
		//patlayerAI.layer[0].onlineOrder = C0;
	//}else{
	//	picObj->plane = VO_GRAPHIC_OSD2;
	//	patlayerAI.layer[0].onlineOrder = C1;
	//}
	patlayerAI.layer_num = 1;
	patlayerAI.layer[0].normal_num = 1;
	picObj->src_type = SRC_NORMAL;
	picObj->alpha = 0;
	picObj->width = s_w;
	picObj->height = s_h;
	picObj->address = dst_phy;
	picObj->pitch = s_w*2;
	picObj->layer_used = 1;	
	picObj->show = 1;
	picObj->paletteformat = 1; /* rgba format */
	picObj->syncInfo.syncstamp = 0;
	picObj->scale_factor = 1;
	getDispSize(&disp_res);	
	picObj->dst_width = disp_res.width;
	picObj->dst_height = disp_res.height;
	osdcmp_plane_ar.plane_alpha_r = 0xff;
	osdcmp_plane_ar.plane_alpha_a = 0xff;
	osdcmp_plane_gb.plane_alpha_b = 0xff;
	osdcmp_plane_gb.plane_alpha_g = 0xff;
	
	picObj->plane_ar = osdcmp_plane_ar;
	picObj->plane_gb = osdcmp_plane_gb;
	
	GDMA_ReceivePicture(&patlayerAI);

	if(num == 0){
		GDMA_ConfigOSDxEnableFast(GDMA_PLANE_OSD2,false);
#if 0
		//gdma_config_line_buffer();
		if(dst_AI != 0){
			if(change_OSD_mode == GDMA_SPLIT_MODE_2
			|| change_OSD_mode == GDMA_SPLIT_MODE_3)
			dvr_free(dst_AI);
			else
			    kfree(dst_AI);
			dst_AI = 0;
		}
		if(dst_AI_2 != 0){
			if(change_OSD_mode == GDMA_SPLIT_MODE_2
			|| change_OSD_mode == GDMA_SPLIT_MODE_3)
			dvr_free(dst_AI_2);
			else
			    kfree(dst_AI_2);
			dst_AI_2 = 0;
		}
#endif
		AI_Enable = 0;
	}else{
		if(AI_Enable == 0){
			GDMA_ConfigOSDxEnableFast(GDMA_PLANE_OSD2,true);
			AI_Enable = 1;
		}
	}	
	return 1;
}

int GDMA_AI_SE_draw_hide(void)
{
	if(AI_Enable==1){
		GDMA_ConfigOSDxEnableFast(GDMA_PLANE_OSD2,false);
		AI_Enable=0;
	}
	return 1;	
}
int GDMA_Wait_OSD1_Prog_Done(void)
{
	int to_cnt=0;
	/* wait prog done */
	while ((rtd_inl(GDMA_CTRL_reg)&GDMA_CTRL_osd1_prog_done_mask) != 0) {
		gdma_usleep(1000); //1ms
		to_cnt++;
		if (to_cnt > 40) {
			printk(KERN_ERR"%s, GDMA HW something wrong. Please check \n", __FUNCTION__);
			break;
		}
	}
	return 1;
}


int GDMA_AI_SE_draw_buffer(int s_w,int s_h,KGAL_PIXEL_FORMAT_T format,unsigned char *buffer) 
{
	unsigned int dst_phy = 0;
	GDMA_PICTURE_OBJECT *picObj;
	VO_RECTANGLE disp_res;
	osd_planealpha_ar osdcmp_plane_ar;
	osd_planealpha_gb osdcmp_plane_gb;
	static int pre_w =0,pre_h =0;
	int pixel_size=0;
	static unsigned char *cache_ptr,*cache_ptr2;

	KGAL_SURFACE_INFO_T dsurf;
	KGAL_RECT_T drect;
	KGAL_DRAW_FLAGS_T sflag;
	KGAL_DRAW_SETTINGS_T sblend;
	
	memset(&dsurf,0, sizeof(KGAL_SURFACE_INFO_T));
	memset(&drect,0, sizeof(KGAL_RECT_T));
	memset(&sblend,0, sizeof(KGAL_DRAW_SETTINGS_T));
	sblend.srcBlend = KGAL_BLEND_ONE;
	sblend.dstBlend = KGAL_BLEND_ZERO;
	sflag = KGAL_DRAW_NOFX;

	//check if need to release memory
	if (ai_draw_by != AI_DRAW_BY_BUFFER){
		if(dst_AI){	
			dvr_free(dst_AI);
			dst_AI=0;
		}
		if(dst_AI_2){
			dvr_free(dst_AI_2);
			dst_AI_2=0;
		}
		ai_draw_by=AI_DRAW_BY_BUFFER;
	}else{
		if((pre_w!=s_w)||(pre_h!=s_h)){
			if(dst_AI){	
				dvr_free(dst_AI);
				dst_AI=0;
			}
			if(dst_AI_2){
				dvr_free(dst_AI_2);
				dst_AI_2=0;
			}
		}	
	}
	
	//check pitch size
	switch(format){
		case KGAL_PIXEL_FORMAT_ARGB:
			pixel_size=4;
			break;
		case KGAL_PIXEL_FORMAT_RGB888:
			pixel_size=3;
			break;	
		case KGAL_PIXEL_FORMAT_ARGB4444:
		case KGAL_PIXEL_FORMAT_ARGB1555:
		case KGAL_PIXEL_FORMAT_RGB16:
			pixel_size=2;
			break;
		default:
			pixel_size=1;
			break;	
	}	

	if (dst_order == 0) {
		if (dst_AI == 0) {
			if(1)
			//if(change_OSD_mode == GDMA_SPLIT_MODE_2	|| change_OSD_mode == GDMA_SPLIT_MODE_3)
				cache_ptr=dvr_malloc_uncached_specific(s_w*s_h*pixel_size, GFP_DCU1_FIRST,(void **)&dst_AI);
			else
				dst_AI = kmalloc(s_w*s_h*pixel_size,GFP_KERNEL);
		}
		//else
		//	memset(dst_AI,0x0,s_w*s_h*2);	
		if (dst_AI == 0){
			printk(KERN_ERR"[%s] dst_AI malloc fail %d\n", __func__,s_w*s_h*pixel_size);
			return 0;
		}
		dst_phy = dvr_to_phys((void *)cache_ptr);
		memcpy(dst_AI,buffer,s_w*s_h*pixel_size);
		dst_order = 1;
	} else {
		if (dst_AI_2 == 0) {
			//if(change_OSD_mode == GDMA_SPLIT_MODE_2	|| change_OSD_mode == GDMA_SPLIT_MODE_3)
			if(1)
				cache_ptr2=dvr_malloc_uncached_specific(s_w*s_h*pixel_size, 0,(void **)&dst_AI_2);
			else
				dst_AI_2 = kmalloc(s_w*s_h*pixel_size, GFP_KERNEL);
		}
		//else
		//	memset(dst_AI_2,0x0,s_w*s_h*2);	
		if (dst_AI_2 == 0){
			printk(KERN_ERR"[%s] dst_AI_2  malloc fail %d\n", __func__,s_w*s_h*pixel_size);
			return 0;
		}	
		dst_phy = dvr_to_phys((void *)cache_ptr2);
		memcpy(dst_AI_2,buffer,s_w*s_h*pixel_size);
		dst_order = 0;
	}

	pre_w=s_w;
	pre_h=s_h;
	
	//memset(dst_AI,0x80,s_w*s_h*3);
	dsurf.physicalAddress = dst_phy;
	dsurf.width = s_w;
	dsurf.height = s_h;
	dsurf.bpp = 8*pixel_size;
	dsurf.pitch = s_w*pixel_size;
	dsurf.pixelFormat = format;

	drect.x = 0;
	drect.y = 0;
	drect.w = s_w;
	drect.h = s_h;

	//KGAL_SyncGraphic_block_mode(1);

#if 0	//debug

	KGAL_FillRectangle(&dsurf,
						   &drect,
						   0x00000000,
						   &sflag,
						   &sblend);		


	drect.x = 0;
	drect.y = 0;
	drect.w = s_w/4;
	drect.h = s_h/4;

	KGAL_FillRectangle(&dsurf,
						   &drect,
						   0xFF000080,
						   &sflag,
						   &sblend);	

	drect.x = s_w/4;
	drect.y = 0;
	drect.w = s_w/4;
	drect.h = s_h/4;

	KGAL_FillRectangle(&dsurf,
						   &drect,
						   0x00FF0080,
						   &sflag,
						   &sblend);	

	drect.x = 0;
	drect.y = s_h/4;
	drect.w = s_w/4;
	drect.h = s_h/4;

	KGAL_FillRectangle(&dsurf,
						   &drect,
						   0x0000FF80,
						   &sflag,
						   &sblend);	

	drect.x = s_w/4;
	drect.y = s_h/4;
	drect.w = s_w/4;
	drect.h = s_h/4;

	KGAL_FillRectangle(&dsurf,
						   &drect,
						   0x00000080,
						   &sflag,
						   &sblend);	

#endif

	memset(&patlayerAI,0x0,sizeof(PICTURE_LAYERS_OBJECT));
	picObj = &patlayerAI.layer[0].normal[0];

	patlayerAI.layer[0].onlineOrder = C1;
	patlayerAI.layer_num = 1;
	patlayerAI.layer[0].normal_num = 1;

	picObj->format=(VO_OSD_COLOR_FORMAT)format;
	picObj->plane = VO_GRAPHIC_OSD2;
	picObj->src_type = SRC_NORMAL;
	picObj->alpha = 0;
	picObj->width = s_w;
	picObj->height = s_h;
	picObj->address = dst_phy;
	picObj->pitch = s_w*pixel_size;
	picObj->layer_used = 1;	
	picObj->show = 1;
	picObj->paletteformat = 1; /* rgba format */
	picObj->syncInfo.syncstamp = 0;
	picObj->scale_factor = 1;
	getDispSize(&disp_res);	
	picObj->dst_width = disp_res.width;
	picObj->dst_height = disp_res.height;
	osdcmp_plane_ar.plane_alpha_r = 0xff;
	osdcmp_plane_ar.plane_alpha_a = 0xff;
	osdcmp_plane_gb.plane_alpha_b = 0xff;
	osdcmp_plane_gb.plane_alpha_g = 0xff;
	
	picObj->plane_ar = osdcmp_plane_ar;
	picObj->plane_gb = osdcmp_plane_gb;
	
	GDMA_ReceivePicture(&patlayerAI);

	if(AI_Enable == 0){
		GDMA_ConfigOSDxEnableFast(GDMA_PLANE_OSD2,true);
		AI_Enable = 1;
	}

	return 1;
}

int GDMA_ES_OSD_Data(int s_w,int s_h,
					 int d_w,int d_h,int num,
					 unsigned int *cp_dst,
					 unsigned int cp_src) 
{
	KGAL_SURFACE_INFO_T ssurf;
	KGAL_SURFACE_INFO_T dsurf;
	KGAL_RECT_T drect;
    KGAL_BLIT_FLAGS_T sflag = KGAL_BLIT_NOFX;
	KGAL_BLIT_SETTINGS_T sblend;
	int i;
	unsigned int *ptr;
	unsigned int local_src;

	memset(&dsurf,0, sizeof(KGAL_SURFACE_INFO_T));
	memset(&drect,0, sizeof(KGAL_RECT_T));
	memset(&sblend,0, sizeof(KGAL_BLIT_SETTINGS_T));
	sblend.srcBlend = KGAL_BLEND_ONE;
	sblend.dstBlend = KGAL_BLEND_ZERO;
	
	drect.x = 0;
	drect.y = 0;
	drect.w = d_w;
	drect.h = d_h;

	ptr = kmalloc(1920*280*4, GFP_KERNEL);
	if(ptr == 0)
		return 1;
	for(i=0;i<1920*280;i++)
	{
		//if(i<128)
		//	*(ptr + i) = 0x5a5a5a5a;
		//else
			*(ptr + i) = 0x78787878;
	}
	//memset(ptr,0x5a,1920*280*4);
	local_src = virt_to_phys(ptr);
	
	ssurf.physicalAddress = local_src;
	ssurf.width = s_w;
	ssurf.height = s_h;
	ssurf.bpp = 32;
	ssurf.pitch = s_w*4;
	ssurf.pixelFormat = KGAL_PIXEL_FORMAT_ARGB;

	dsurf.width = d_w;
	dsurf.height = d_h;
	dsurf.bpp = 32;
	dsurf.pitch = d_w*4;
	dsurf.pixelFormat = KGAL_PIXEL_FORMAT_ARGB;

	for(i=0;i<num;i++){
		dsurf.physicalAddress = cp_dst[i]+(1920*1080*4);
		KGAL_FormatConv(&ssurf, &drect,&dsurf, 0, 0,&sflag, &sblend);
	}
	kfree(ptr);
	return 1;
}

int GDMA_ES_OSD_Data_test(unsigned int cp_src) 
{
	int s_w=1920;
	int s_h=280;
	int d_w=1920;
	int d_h=280;
	int num = 3;
		
	printk(KERN_ERR"Show es osd addr %x %x  %x  %x.\n",cp_src,es_osd_addr[0],es_osd_addr[1],es_osd_addr[2]);	
	if(cp_src == 0)
		GDMA_ES_OSD_Data(s_w,s_h,
						 d_w,d_h,num,es_osd_addr,es_osd_addr[0]); 
	else
	GDMA_ES_OSD_Data(s_w,s_h,
					 d_w,d_h,num,es_osd_addr,cp_src); 
	return 1;
}
static inline bool rtkgdma_dbg_parse_value(char *cmd_pointer, long long *parsed_data)
{
	if (kstrtoll(cmd_pointer, 0, parsed_data) == 0) {
		return true;
	} else {
		return false;
	}
}
static inline void rtkgdma_dbg_EXECUTE(const int cmd_index, char **cmd_pointer)
{
	long long parsed_data = 0;
	char tmpStr[128] = {0};
	KADP_HAL_GAL_CTRL_T st;
	memset(&st,0,sizeof(KADP_HAL_GAL_CTRL_T));
	st.osdn=1;
	
	if (cmd_pointer == NULL || *cmd_pointer == NULL) {
		printk(KERN_ERR"%s cmd-%d error. cmd_pointer invalid\n", __func__, cmd_index);
		return;
	}

	switch (cmd_index) {
		case GDMA_DBG_CMD_SET_H_SHIFT:
			rtkgdma_dbg_parse_value(*cmd_pointer, &parsed_data);
			g_osdshift_ctrl.h_shift_pixel = (int)parsed_data;
			if(g_osdshift_ctrl.h_shift_pixel)
				g_osdshift_ctrl.shift_h_enable=1;
			else
				g_osdshift_ctrl.shift_h_enable=0;
			printk("%s -- %d parsed_data=%d\n",__FUNCTION__,__LINE__,(int)parsed_data);
			return;
		case GDMA_DBG_CMD_SET_V_SHIFT:
			rtkgdma_dbg_parse_value(*cmd_pointer, &parsed_data);
			g_osdshift_ctrl.v_shift_pixel = (int)parsed_data;
			if(g_osdshift_ctrl.v_shift_pixel)
				g_osdshift_ctrl.shift_v_enable=1;
			else
				g_osdshift_ctrl.shift_v_enable=0;
			printk("%s -- %d parsed_data=%d\n",__FUNCTION__,__LINE__,(int)parsed_data);
			return;
		case GDMA_DBG_CMD_OSD_DUMP:
			sscanf(*cmd_pointer, "%d %s\n", &st.osdn, st.capinfo.fname);
			memcpy(tmpStr, st.capinfo.fname, strlen(st.capinfo.fname) + 1);
			snprintf(st.capinfo.fname, 128, "%s/dump_%08d.bmp", tmpStr, rtd_inl(TIMER_SCPU_CLK90K_LO_reg));
			break;
		case GDMA_DBG_CMD_OSD_DUMP_EX:
			sscanf(*cmd_pointer, "%d %s\n", &st.osdn, st.capinfo.fname);
			memcpy(tmpStr, st.capinfo.fname, strlen(st.capinfo.fname) + 1);
			snprintf(st.capinfo.fname, 128, "%s/dump_%08d.bmp", tmpStr, rtd_inl(TIMER_SCPU_CLK90K_LO_reg));
				break;
		case GDMA_DBG_CMD_OSD_DROP:
			sscanf(*cmd_pointer, "%x\n", &st.en);
			printk("%s -- %d parsed_data=%x\n",__FUNCTION__,__LINE__,st.en);
			break;
		case GDMA_DBG_CMD_OSD_PATTERN:
			sscanf(*cmd_pointer, "%d %d %x\n", &st.en, &st.osdn, &st.clr);
			break;
		case GDMA_DBG_CMD_OSD_VFLIP:
			sscanf(*cmd_pointer, "%d\n", &st.en);
			break;
		case GDMA_DBG_CMD_OSD_CTRL:
			sscanf(*cmd_pointer, "%d %d\n", &st.en,&st.osdn);
			break;
		case GDMA_DBG_CMD_OSD_SWAP:
			sscanf(*cmd_pointer, "%d %d %d %d %d %d\n",
				&st.c[0],&st.c[1],&st.c[2],
				&st.c[3],&st.c[4],&st.c[5]);
			break;
		case GDMA_DBG_CMD_OSD_CAP2MEM:
			sscanf(*cmd_pointer, "%d %x\n", &st.osdn, &st.clr);
			break;
		case GDMA_DBG_CMD_SHOW_INFO:
				break;
		case GDMA_DBG_CMD_OSD_GET_INFO:
			break;
		case GDMA_DBG_CMD_OSD_CTRL_EN:
			sscanf(*cmd_pointer, "%d %d\n", &st.en,&st.osdn);
			break;
		case GDMA_DBG_CMD_SHOW_INT_STATUS:
			break;
		case GDMA_DBG_CMD_OSD_CLONE_EN:
			sscanf(*cmd_pointer, "%d \n", &st.en);
			break;
		case GDMA_DBG_CMD_OSD_BW_GEN_EN:
			sscanf(*cmd_pointer, "%d %d %d %d\n",&st.osdn,&st.en,&st.w,&st.h);
			break;
		case GDMA_DBG_CMD_OSD_SYNC_EN:
			sscanf(*cmd_pointer, "%d \n", &st.en);
			break;
		case GDMA_DBG_CMD_SPLIT_MODE:
			sscanf(*cmd_pointer, "%2d \n", &change_OSD_mode);
			break;
		case GDMA_DBG_CMD_STOP_UPDATE:
			sscanf(*cmd_pointer, "%2d \n", &enable_stop_update);
			break;
		case GDMA_DBG_CMD_OSD_RLINK:
#if USE_RLINK_INTERFACE
			sscanf(*cmd_pointer, "%d \n", &rlink_interface_cmd);
#endif
			break;
		case GDMA_DBG_CMD_ES_OSD_ADDR:
			sscanf(*cmd_pointer, "%x \n", &cp_src);
			break;
		case GDMA_DBG_CMD_OSD_GAL_AI_BLOCK:
			sscanf(*cmd_pointer, "%d %x\n",&ai_num,&ai_color);
			break;
		case GDMA_DBG_CMD_OSD_GAL_AI_BLOCK_AFBC:
			sscanf(*cmd_pointer, "%d %x\n",&ai_num,&ai_color);
			break;
		case GDMA_DBG_CMD_OSD_GET_GOLBAL_ALPHA:
			break;
		case GDMA_DBG_CMD_OSD_SET_OSD_GAIN:
			sscanf(*cmd_pointer, "%d %d\n", &st.en,&st.gain);
			break;
		case GDMA_DBG_CMD_OSD_CAP_FB:
			sscanf(*cmd_pointer, "%s\n", st.capinfo.fname);
			memcpy(tmpStr, st.capinfo.fname, strlen(st.capinfo.fname) + 1);
			break;
		case GDMA_DBG_CMD_OSD_SET_OSD_ALPHA_DETECT:
			sscanf(*cmd_pointer, "%x \n", &st.en);
			break;	
		case GDMA_DBG_CMD_OSD_GET_OSD_ALPHA_RATIO:
			break;
		case GDMA_DBG_CMD_OSD_SET_LOGLEVEL:
			sscanf(*cmd_pointer, "%d \n", &gDebugGDMA_loglevel );

			pr_crit("osd loglevel: %d\n", gDebugGDMA_loglevel);
			break;
		case GDMA_DBG_CMD_OSD_MONITOR_CTL:
			sscanf(*cmd_pointer, "%d %d %d %d %d %d %d\n", 
				&gGDMA_Monitor_ctl.enable,&gGDMA_Monitor_ctl.input_x,&gGDMA_Monitor_ctl.input_y,
				&gGDMA_Monitor_ctl.input_w,&gGDMA_Monitor_ctl.input_h,
				&gGDMA_Monitor_ctl.output_x,&gGDMA_Monitor_ctl.output_y);
			if(gGDMA_Monitor_ctl.enable && (gGDMA_Monitor_ctl.input_w > 1920)){
				printk(KERN_ERR" ERROR !! input_w > 1920. \n");
				gGDMA_Monitor_ctl.enable = 0;
		
			}
			break;
		case GDMA_DBG_CMD_OSD_RUNTIME_CTL:
			{
				int data=-1;
				
				sscanf(*cmd_pointer, "%d \n", &data);
				if(data==1){

					GDMA_Clk_Select(0, true);
					GDMA_ReceivePicture_warpper();
					GDMA_ConfigOSDxMixerEnableNoDelay(GDMA_PLANE_OSD1,1);

					GAL_Runtime_Suspend_enable_OSD();
					GAL_Runtime_Resume_receive();		
				}else if(data==0){
					GDMA_ConfigOSDxMixerEnableNoDelay(GDMA_PLANE_OSD1,0);
					GAL_Runtime_Suspend_disable_OSD();
				}
			}
			break;
		case GDMA_DBG_CMD_OSD_CONTINUE_DUMP:
			{
				
				sscanf(*cmd_pointer, "%d \n", &gDebugGDMA_dump_count);
				//memcpy(gDebugGDMA_dump_name, st.capinfo.fname, strlen(st.capinfo.fname) + 1);
				break;
			}
		default:
			printk(KERN_ERR"%s cmd-%d error.\n",__func__,cmd_index);
			return;
		}
	st.type = cmd_index;
	if(GDMA_Debug(&st)!=0) {
		printk(KERN_ERR"[%s]%s , OSD-%d fail.\n",
		__func__,rtkgdma_cmd_str[cmd_index], st.osdn);
	}
}

bool GDMA_init_debug_proc(void)
{
	if (rtkgdma_proc_dir == NULL && rtkgdma_proc_entry == NULL) {
		rtkgdma_proc_dir = proc_mkdir(RTKGDMA_PROC_DIR , NULL);

		if (rtkgdma_proc_dir != NULL) {
			rtkgdma_proc_entry =
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0))
                proc_create(RTKGDMA_PROC_ENTRY, 0666,
                        rtkgdma_proc_dir, &gdma_proc_fops);
#else
				proc_create(RTKGDMA_PROC_ENTRY, 0666,
							rtkgdma_proc_dir, &gdma_fops);
#endif

			if (rtkgdma_proc_entry == NULL) {
				proc_remove(rtkgdma_proc_dir);
				rtkgdma_proc_dir = NULL;

				return false;
			}
		} else {

			return false;
		}
	}

	return true;
}

void GDMA_uninit_debug_proc(void)
{
	if (rtkgdma_proc_entry) {
		proc_remove(rtkgdma_proc_entry);
		rtkgdma_proc_entry = NULL;
	}

	if (rtkgdma_proc_dir) {
		proc_remove(rtkgdma_proc_dir);
		rtkgdma_proc_dir = NULL;
	}
}

//#if !IS_ENABLED(CONFIG_SUPPORT_SCALER)
void fwif_color_set_scaleosd_sharpness_driver(unsigned char src_idx, unsigned char value)
{
	//to do...if nesscery
	//need use new OSD SR
}

EXPORT_SYMBOL(fwif_color_set_scaleosd_sharpness_driver); //export for PQ use
//#endif

/*=============START OF GPU STD STUFF ===============*/
#if defined(CONFIG_ARM64)
unsigned long GDMA_gpuWrap_std_suspend(unsigned long phyAddr, unsigned int size) {
	return (unsigned long)dvr_remap_cached_memory( phyAddr, size, __builtin_return_address(0));
}
#else
unsigned int GDMA_gpuWrap_std_suspend(unsigned int phyAddr, unsigned int size) {
	return (unsigned int)dvr_remap_cached_memory( phyAddr, size, __builtin_return_address(0));
}
#endif

#if defined(CONFIG_ARM64)
void GDMA_gpuWrap_std_resume(unsigned long phyAddr, unsigned long virtAddr, unsigned int size) {
#else
void GDMA_gpuWrap_std_resume(unsigned int phyAddr, unsigned int virtAddr, unsigned int size) {
#endif
	dmac_flush_range((void*)virtAddr, (void*)(virtAddr + size));
	outer_flush_range((phys_addr_t)phyAddr, (phys_addr_t)(phyAddr + size));
	dvr_unmap_memory((void*)virtAddr, PAGE_ALIGN(size));
}
/*=============END    OF GPU STD STUFF ===============*/

module_init(GDMA_init_module);
module_exit(GDMA_exit_module);
MODULE_LICENSE("GPL");
MODULE_IMPORT_NS(VFS_internal_I_am_really_a_filesystem_and_am_NOT_a_driver);

EXPORT_SYMBOL(GDMA_ConfigureGraphicCanvas);
EXPORT_SYMBOL(GDMA_Check_Dvr_Reclaim);
EXPORT_SYMBOL(GDMA_CreateGraphicWindow);
EXPORT_SYMBOL(GDMA_DrawGraphicWindow);
EXPORT_SYMBOL(GDMA_ModifyGraphicWindow);
EXPORT_SYMBOL(GDMA_DeleteGraphicWindow);
EXPORT_SYMBOL(GDMA_AddCallBackFunc);
EXPORT_SYMBOL(GDMA_AddCookieCallBackFunc);
EXPORT_SYMBOL(GDMA_Display);
EXPORT_SYMBOL(GDMA_OSD1OnTop);
EXPORT_SYMBOL(GDMA_Config3D);
EXPORT_SYMBOL(GDMA_ConfigVFlip);
EXPORT_SYMBOL(GDMA_Debug);
EXPORT_SYMBOL(GDMA_Config3DOffset);
EXPORT_SYMBOL(GDMA_ConfigBlending);
EXPORT_SYMBOL(GDMA_ReceivePicture);
EXPORT_SYMBOL(GDMA_ConfigureDisplayWindow);
EXPORT_SYMBOL(GDMA_ReceiveCmd);
EXPORT_SYMBOL(gContext);
EXPORT_SYMBOL(GDMA_ISR_export);
EXPORT_SYMBOL(GDMA_ConfigInputSrc_4k2k);
EXPORT_SYMBOL(GDMA_Lock_Vsync_Sem);
EXPORT_SYMBOL(GDMA_Config3dUi);
EXPORT_SYMBOL(GDMA_ConfigOSDEnable);
EXPORT_SYMBOL(GDMA_ConfigOSDxEnable);
EXPORT_SYMBOL(GDMA_ConfigOSDxEnableFast);
EXPORT_SYMBOL(GDMA_GetPanelPixelMode);
EXPORT_SYMBOL(GDMA_ReceiveGraphicLayers);
EXPORT_SYMBOL(GDMA_CopyLayerInfo);
EXPORT_SYMBOL(getDispSize);
EXPORT_SYMBOL(GDMA_gpuWrap_std_suspend);
EXPORT_SYMBOL(GDMA_gpuWrap_std_resume);
#if USE_RLINK_INTERFACE
EXPORT_SYMBOL(GDMA_rlink_interface_register_rlink);
EXPORT_SYMBOL(GDMA_rlink_interface_remove_rlink);
#endif
EXPORT_SYMBOL(gdma_dev_addr);
EXPORT_SYMBOL(GDMA_AI_SE_draw_buffer);
EXPORT_SYMBOL(GDMA_GetGlobalAlpha);
EXPORT_SYMBOL(GDMA_AI_SE_draw_hide);
EXPORT_SYMBOL(GDMA_Clk_Select);
EXPORT_SYMBOL(GDMA_ReceivePicture_warpper);
EXPORT_SYMBOL(GAL_Runtime_Suspend);
EXPORT_SYMBOL(GAL_Runtime_Resume);

/*===============================================*/
