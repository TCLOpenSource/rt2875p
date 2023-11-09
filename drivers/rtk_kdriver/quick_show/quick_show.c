#ifndef BUILD_QUICK_SHOW
#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/export.h>
#include <linux/uaccess.h>
#include <linux/input.h> // for key event
#include "../audio_hw/rtd2851a/audio_hw_app.h"  // FIXME: do not include header in this way
// FIXME: use audio_hw driver API
#include <rbus/audio_reg.h>
#include <rtd_log/rtd_module_log.h>
#include <ir_input.h>
#ifdef CONFIG_COMPAT
#include <linux/compat.h>
#define to_user_ptr(x)          compat_ptr((unsigned int ) x)
#else
#define to_user_ptr(x)          ((void* __user)(x)) // convert 32 bit value to user pointer
#endif /* CONFIG_COMPAT */

#if IS_ENABLED(CONFIG_RTK_KDRV_USB_GADGET_HID)
extern void RTK_USB_Typec_Hidg_enable(bool enable);
#else
static void RTK_USB_Typec_Hidg_enable(bool enable) { }
#endif /* CONFIG_RTK_KDRV_USB_GADGET_HID */

#define SP_VOLUME_TO_DB_IN_QS (-10.625)   // volume 50 = -10.625db . // FIXME: read from factory to get db value and tranfom dB value to volume in that device
#define RTK_QS_DEV_NUM  1

static dev_t rtk_qs_dev_t;
static struct cdev rtk_qs_cdev;
static struct class *rtk_qs_class;
static bool mute = false;
static void (*audio_hwp_prescal)(int val, int ch_id) = NULL;
#else   /* !BUILD_QUICK_SHOW */
#include <no_os/export.h>
#endif  /* !BUILD_QUICK_SHOW */

#include <rtk_kdriver/io.h>
#include <rtk_kdriver/quick_show/quick_show.h>
#include <ioctrl/quick_show/quick_show_cmd_id.h>


bool in_quick_show = false;


static inline bool _is_QS_magic_valid(unsigned int val)
{
    return (val & QS_STATUS_magic_mask) == (QS_STATUS_magic_valid_data << QS_STATUS_magic_shift);
}

static inline bool _is_QS_active(unsigned int val)
{
    return _is_QS_magic_valid(val) && (val & QS_STATUS_active_mask);
}

bool is_QS_active(void)
{
    return _is_QS_active(rtd_inl(QS_STATUS_reg));
}
EXPORT_SYMBOL(is_QS_active);

bool is_QS_hdmi_enable(void)
{
    unsigned int val;

    val = rtd_inl(QS_STATUS_reg);
    return _is_QS_active(val) && (val & QS_STATUS_hdmi_mask);
}
EXPORT_SYMBOL(is_QS_hdmi_enable);

bool is_QS_dp_enable(void)
{
    unsigned int val;

    val = rtd_inl(QS_STATUS_reg);
    return _is_QS_active(val) && (val & QS_STATUS_dp_mask);
}
EXPORT_SYMBOL(is_QS_dp_enable);

bool is_QS_panel_enable(void)
{
    unsigned int val;

    val = rtd_inl(QS_STATUS_reg);
    return _is_QS_active(val) && (val & QS_STATUS_panel_mask);
}
EXPORT_SYMBOL(is_QS_panel_enable);

bool is_QS_scaler_enable(void)
{
    unsigned int val;

    val = rtd_inl(QS_STATUS_reg);
    return _is_QS_active(val) && (val & QS_STATUS_scaler_mask);
}
EXPORT_SYMBOL(is_QS_scaler_enable);

bool is_QS_i2c_enable(void)
{
    unsigned int val;

    val = rtd_inl(QS_STATUS_reg);
    return _is_QS_active(val) && (val & QS_STATUS_i2c_mask);
}
EXPORT_SYMBOL(is_QS_i2c_enable);

bool is_QS_pq_enable(void)
{
    unsigned int val;

    val = rtd_inl(QS_STATUS_reg);
    return _is_QS_active(val) && (val & QS_STATUS_pq_mask);
}
EXPORT_SYMBOL(is_QS_pq_enable);

bool is_QS_acpu_enable(void)
{
    unsigned int val;

    val = rtd_inl(QS_STATUS_reg);
    return _is_QS_magic_valid(val) && (val & QS_STATUS_acpu_mask);
}
EXPORT_SYMBOL(is_QS_acpu_enable);

bool is_QS_vcpu_enable(void)
{
    unsigned int val;

    val = rtd_inl(QS_STATUS_reg);
    return _is_QS_magic_valid(val) && (val & QS_STATUS_vcpu_mask);
}
EXPORT_SYMBOL(is_QS_vcpu_enable);

bool is_QS_amp_enable(void)
{
    unsigned int val;

    val = rtd_inl(QS_STATUS_reg);
    return _is_QS_active(val) && (val & QS_STATUS_amp_mask);
}
EXPORT_SYMBOL(is_QS_amp_enable);

bool is_QS_hp_enable(void)
{
    unsigned int val;

    val = rtd_inl(QS_STATUS_reg);
    return _is_QS_active(val) && (val & QS_STATUS_hp_mask);
}
EXPORT_SYMBOL(is_QS_hp_enable);

bool is_QS_typec_enable(void)
{
    unsigned int val;

    val = rtd_inl(QS_STATUS_reg);
    return _is_QS_active(val) && (val & QS_STATUS_typec_mask);
}
EXPORT_SYMBOL(is_QS_typec_enable);

int get_QS_portnum(void)
{
    unsigned int val;

    val = rtd_inl(QS_STATUS_reg);
    if (_is_QS_active(val))
        return (val & QS_STATUS_portnum_mask) >> QS_STATUS_portnum_shift;
    else
        return -ENODEV;
}
EXPORT_SYMBOL(get_QS_portnum);


int get_QS_volume(void)
{
    unsigned int val;

    val = rtd_inl(QS_STATUS_reg);
    if (_is_QS_active(val))
        return (val & QS_STATUS_volume_mask) >> QS_STATUS_volume_shift;
    else
        return -ENODEV;
}
EXPORT_SYMBOL(get_QS_volume);

int get_QS_mute(void)
{
    unsigned int val;

    val = rtd_inl(QS_STATUS_reg);
    if (_is_QS_active(val))
        return (val & QS_STATUS_mute_mask) >> QS_STATUS_mute_shift;
    else
        return -ENODEV;
}
EXPORT_SYMBOL(get_QS_mute);

bool is_QS_game_mode(void)
{
#ifndef BUILD_QUICK_SHOW
    unsigned int val;

    val = rtd_inl(QS_STATUS_reg);
    return _is_QS_active(val) && (rtd_inl(QS_PIC_MODE_reg) & QS_PIC_MODE_GAME_mask);
#else
    extern bool qs_game_mode;
    return qs_game_mode;
#endif
}
EXPORT_SYMBOL(is_QS_game_mode);

bool is_QS_pc_mode(void)
{
#ifndef BUILD_QUICK_SHOW
    unsigned int val;

    val = rtd_inl(QS_STATUS_reg);
    return _is_QS_active(val) && (rtd_inl(QS_PIC_MODE_reg) & QS_PIC_MODE_PC_mask);
#else
    extern bool qs_pc_mode;
    return qs_pc_mode;
#endif
}
EXPORT_SYMBOL(is_QS_pc_mode);

bool is_QS_allm_enable(void)
{
#ifndef BUILD_QUICK_SHOW
    unsigned int val;

    val = rtd_inl(QS_STATUS_reg);
    return _is_QS_active(val) && (rtd_inl(QS_PIC_MODE_reg) & QS_ALLM_EN_mask);
#else
    extern bool qs_allm_en;
    return qs_allm_en;
#endif
}
EXPORT_SYMBOL(is_QS_allm_enable);

bool is_QS_freesync_enable(void)
{
#ifndef BUILD_QUICK_SHOW
    unsigned int val;

    val = rtd_inl(QS_STATUS_reg);
    return _is_QS_active(val) && (rtd_inl(QS_PIC_MODE_reg) & QS_FREESYNC_EN_mask);
#else
    extern bool qs_freesync_en;
    return qs_freesync_en;
#endif
}
EXPORT_SYMBOL(is_QS_freesync_enable);

#ifndef BUILD_QUICK_SHOW
static void set_QS_inactive(void)
{
    rtd_clearbits(QS_STATUS_reg, QS_STATUS_active_mask);
}


static int rtk_qs_open(struct inode *inode, struct file *filp)
{
    return 0;
}


static int rtk_qs_release(struct inode *inode, struct file *filp)
{
    return 0;
}


static int volume_adj = 0;

enum SOURCE {
    SOURCE_ATV,
    SOURCE_DTV,
    SOURCE_HDMI,
    SOURCE_USB,
    SOURCE_AV,
    SOURCE_YPP,
    SOURCE_VGA,
    SOURCE_DP,
    SOURCE_NONE,
};

enum AOUTDEV {
    AOUTDEV_SPK,
    AOUTDEV_HP,
};


//refs : https://mm2sd.sdlc.rd.realtek.com/gerrit/plugins/gitiles/kernel/rtk_aosp/vendor/tvconfigs/ProjectConfigs/+/refs/heads/realtek/gaia/AQ/AudioCurve/VolumeCurve.ini
static int PRESCALE_SPK[] = {
    [SOURCE_ATV] = -10,
    [SOURCE_DTV] = -13,
    [SOURCE_HDMI] = -13,
    [SOURCE_USB] = -13,
    [SOURCE_DP] = -13,
    [SOURCE_AV] = -12,
    [SOURCE_YPP] =  -12,
    [SOURCE_VGA] = -12,
};

static int Vol_TvSrc_SPK[101] = {
    -576,-536,-512,-488,-464,-448,-432,-416,-400,-384,
    -368,-353,-338,-323,-308,-294,-280,-266,-252,-238,
    -224,-216,-208,-200,-192,-184,-176,-168,-160,-152,
    -144,-139,-134,-129,-124,-119,-114,-109,-104,-100,
    -96,-94,-92,-90,-88,-86,-84,-83,-82,-81,
    -80,-78,-76,-74,-72,-70,-68,-67,-66,-65,
    -64,-62,-60,-58,-56,-54,-52,-51,-50,-49,
    -48,-46,-44,-42,-40,-38,-36,-35,-34,-33,
    -32,-30,-28,-26,-24,-22,-20,-19,-18,-17,
    -16,-14,-12,-10,-8,-6,-4,-3,-2,-1,
    0
};

static int PRESCALE_HP[] = {
    [SOURCE_ATV] = -4,
    [SOURCE_DTV] = -1,
    [SOURCE_HDMI] = -1,
    [SOURCE_USB] = -1,
    [SOURCE_AV] = 0,
    [SOURCE_YPP] = 0,
    [SOURCE_VGA] = 0,
};

static int Vol_TvSrc_HP[101] = {
    -576,-541,-506,-471,-436,-402,-368,-334,-300,-266,
    -232,-225,-218,-211,-204,-198,-192,-186,-180,-174,
    -168,-164,-160,-157,-154,-151,-148,-145,-142,-139,
    -136,-132,-128,-124,-120,-116,-112,-108,-104,-100,
    -96,-94,-92,-90,-88,-86,-84,-83,-82,-81,
    -80,-78,-76,-74,-72,-69,-67,-65,-63,-60,
    -58,-56,-54,-51,-49,-47,-45,-42,-40,-38,
    -36,-33,-31,-29,-27,-24,-24,-23,-22,-21,
    -24,-22,-20,-18,-16,-15,-14,-13,-12,-11,
    -10,-9,-8,-7,-6,-5,-4,-3,-2,-1,
    0
};


__maybe_unused static unsigned int db_to_volume(double db, enum SOURCE source, enum AOUTDEV aoutdev)
{
    int dbPrescale;
    int dvVol_index = 0;
    int db_tmp = 0;

    //int dbVol = -80;  look up table
    //int dbTotal = dbVol + (dbPrescale % 8);
    db *= APP_VOL_STEP_1DB;
    db_tmp = db;

    if (aoutdev == AOUTDEV_SPK) {
        dbPrescale = PRESCALE_SPK[source];
        db_tmp -= (dbPrescale % 8);
        for (dvVol_index = 0; dvVol_index < ARRAY_SIZE(Vol_TvSrc_SPK); dvVol_index++) {
            if (db_tmp < Vol_TvSrc_SPK[dvVol_index])
                break;
        }
    }
    else if (aoutdev == AOUTDEV_HP) {
        dbPrescale = PRESCALE_HP[source];
        db_tmp -= (dbPrescale % 8);
        for (dvVol_index = 0; dvVol_index < ARRAY_SIZE(Vol_TvSrc_HP); dvVol_index++) {
            if (db_tmp < Vol_TvSrc_HP[dvVol_index])
                break;
        }
    } else {
        return -ENODEV;
    }

    return dvVol_index;
}


static int volume_to_db(unsigned int volume, enum SOURCE source, enum AOUTDEV aoutdev)
{
    int dbPrescale, dbVol, dbTotal;


    if (volume > 100)
        return -EINVAL;

    if (aoutdev == AOUTDEV_SPK) {
        dbPrescale = PRESCALE_SPK[source];
        dbVol = Vol_TvSrc_SPK[volume];

    } else if (aoutdev == AOUTDEV_HP) {
        dbPrescale = PRESCALE_HP[source];
        dbVol = Vol_TvSrc_HP[volume];

    } else {
        return -ENODEV;
    }

    //int dbVol = -80;  look up table
    //int dbTotal = dbVol + (dbPrescale % 8);

    dbTotal = dbVol + (dbPrescale % 8);

    return dbTotal;
}


static long rtk_qs_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    long ret = 0;
    int active = 0;
    RTK_QS_SOURCE_PORT_INFO source_port;
    RTK_QS_SND_DEV_INFO snd_dev;

    switch (cmd) {
    case RTK_QS_IOC_GET_ACTIVE_STATE:
        active = (int)is_QS_active();
        if (copy_to_user(to_user_ptr(arg), (const void *)&active, sizeof(active)))
            ret = -EFAULT;
        break;

    case RTK_QS_IOC_SET_INACTIVE:
        set_QS_inactive();
        set_QS_handle_key(false);
        break;

    case RTK_QS_IOC_GET_SOURCE_PORT:
        source_port.source = QS_SOURCE_NONE;
        source_port.portnum = 0;

        if (is_QS_active()) {
            if (is_QS_hdmi_enable()) {
                source_port.source = QS_SOURCE_HDMI;
                source_port.portnum = get_QS_portnum();
            } else if (is_QS_dp_enable()) {
                source_port.source = QS_SOURCE_DP;
                source_port.portnum = get_QS_portnum();
            }
        }


        if (copy_to_user(to_user_ptr(arg), (const void *)&source_port, sizeof(source_port)))
            ret = -EFAULT;

        break;

    case RTK_QS_IOC_GET_VOLUME:
        snd_dev.dev = QS_SND_DEV_NONE;
        snd_dev.volume = 0;
        snd_dev.is_mute = false;

        if (is_QS_active()) {
            if (is_QS_hp_enable()) {
                snd_dev.dev = QS_SND_DEV_HEADPHONE;
            } else {
                snd_dev.dev = QS_SND_DEV_SPEAKER;
            }
            snd_dev.volume = volume_adj;
            snd_dev.is_mute = mute;
        }

        if (copy_to_user(to_user_ptr(arg), (const void *)&snd_dev, sizeof(snd_dev)))
            ret = -EFAULT;

        break;

    case RTK_QS_IOC_SET_NOT_HANDLE_KEY:
        set_QS_handle_key(false);
        break;

    default:
        rtd_pr_quick_show_err("%s: Unknow commnad: %x\n", __func__, cmd);
    }

    return ret;
}


static long rtk_qs_compat_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    void __user *compat_arg = compat_ptr(arg);
    return rtk_qs_ioctl(filp, cmd, (unsigned long)compat_arg);
}


struct file_operations rtk_qs_fops = {
    .owner            = THIS_MODULE,
    .open             = rtk_qs_open,
    .release          = rtk_qs_release,
#if 0
    .read             = rtk_qs_read,
    .write            = rtk_qs_write,
#endif
    .unlocked_ioctl   = rtk_qs_ioctl,
    .compat_ioctl     = rtk_qs_compat_ioctl,
};


static ssize_t qs_active_show(struct device *dev,
        struct device_attribute *attr, char *buf)
{
    return snprintf(buf, PAGE_SIZE, "%d\n", is_QS_active());
}

static ssize_t qs_active_store(struct device *dev,
        struct device_attribute *attr, const char *buf, size_t count)
{

    // TODO: wrtie QS disable to facotry file

    return count;
}
static DEVICE_ATTR(active, S_IRUGO | S_IWUSR,
        qs_active_show, qs_active_store);



static ssize_t qs_source_show(struct device *dev,
        struct device_attribute *attr, char *buf)
{
    RTK_QS_SOURCE_PORT_INFO source_port;
    char source_name[5] = "NONE";

    source_port.source = QS_SOURCE_NONE;
    source_port.portnum = 0;

    if (is_QS_active()) {
        if (is_QS_hdmi_enable()) {
            source_port.source = QS_SOURCE_HDMI;
            source_port.portnum = get_QS_portnum();
            snprintf(source_name, 5, "HDMI");
        } else if (is_QS_dp_enable()) {
            source_port.source = QS_SOURCE_DP;
            source_port.portnum = get_QS_portnum();
            snprintf(source_name, 5, "DP");
        }
    }

    return snprintf(buf, PAGE_SIZE, "%s:%d\n", source_name, source_port.portnum);
}
static DEVICE_ATTR(source, S_IRUGO,
        qs_source_show, NULL);



static ssize_t qs_volume_show(struct device *dev,
        struct device_attribute *attr, char *buf)
{
    char source_name[5] = "NONE";
    RTK_QS_SND_DEV_INFO snd_dev;

    snd_dev.dev = QS_SND_DEV_NONE;
    snd_dev.volume = 0;

    if (is_QS_active()) {
        if (is_QS_hp_enable()) {
            snd_dev.dev = QS_SND_DEV_HEADPHONE;
            snprintf(source_name, 5, "HP");
            snd_dev.volume = volume_adj;
        } else {
            snd_dev.dev = QS_SND_DEV_SPEAKER;
            snprintf(source_name, 5, "SP");
            snd_dev.volume = volume_adj;
        }
    }

    return snprintf(buf, PAGE_SIZE, "%s:%d\n", source_name, snd_dev.volume);
}
static DEVICE_ATTR(volume, S_IRUGO,
        qs_volume_show, NULL);



static ssize_t qs_handle_key_show(struct device *dev,
        struct device_attribute *attr, char *buf)
{
    return snprintf(buf, PAGE_SIZE, "%d\n", get_QS_handle_key());
}

static ssize_t qs_handle_key_store(struct device *dev,
        struct device_attribute *attr, const char *buf, size_t count)
{
    set_QS_handle_key(false);
    return count;
}
static DEVICE_ATTR(handle_key, S_IRUGO | S_IWUSR,
        qs_handle_key_show, qs_handle_key_store);


static struct attribute *dev_attrs[] = {
    &dev_attr_active.attr,
    &dev_attr_source.attr,
    &dev_attr_volume.attr,
    &dev_attr_handle_key.attr,
    NULL,
};


static struct attribute_group dev_attr_grp = {
    .attrs = dev_attrs,
};


static int __init rtk_qs_cdev_init(void)
{
    int ret = 0;
    struct device *dev;

    ret = alloc_chrdev_region(&rtk_qs_dev_t, 0, RTK_QS_DEV_NUM, "rtk_qs");
    if (ret) {
        rtd_pr_quick_show_err("fail to get char dev Major and Minor \n");
        goto FAIL_ALLOC_CHRDEV_MAJOR;
    }

    cdev_init(&rtk_qs_cdev, &rtk_qs_fops);

    ret = cdev_add(&rtk_qs_cdev, rtk_qs_dev_t, RTK_QS_DEV_NUM);
    if (ret) {
        rtd_pr_quick_show_err("fail to add char dev to system \n");
        goto FAIL_ADD_CHRDEV;
    }

    rtk_qs_class = class_create(THIS_MODULE, "rtk_qs");
    if (IS_ERR(rtk_qs_class)) {
        rtd_pr_quick_show_err("fail to cearte class \n");
        ret = PTR_ERR(rtk_qs_class);
        goto FAIL_CREATE_CLASS;
    }

    dev = device_create(rtk_qs_class, NULL, rtk_qs_cdev.dev, NULL, "rtk_qs");
    if (!dev) {
        rtd_pr_quick_show_err("fail to cearte device node for rtk_qs \n");
        ret = -ENOMEM;
        goto FAIL_CREATE_DEVICE;
    }

    if (sysfs_create_group(&dev->kobj, &dev_attr_grp)) {
        rtd_pr_quick_show_warn("Create self-defined sysfs attributes fail \n");
    }


    return ret;

FAIL_CREATE_DEVICE:
    class_destroy(rtk_qs_class);
    rtk_qs_class = NULL;
FAIL_CREATE_CLASS:
    cdev_del(&rtk_qs_cdev);
FAIL_ADD_CHRDEV:
    unregister_chrdev_region(rtk_qs_dev_t, RTK_QS_DEV_NUM);
FAIL_ALLOC_CHRDEV_MAJOR:
    return ret;
}

static void rtk_qs_cdev_uninit(void)
{
    device_destroy(rtk_qs_class, rtk_qs_cdev.dev);
    class_destroy(rtk_qs_class);
    rtk_qs_class = NULL;
    cdev_del(&rtk_qs_cdev);
    unregister_chrdev_region(rtk_qs_dev_t, RTK_QS_DEV_NUM);
}


static int HwpSetMute(unsigned int ch_id, unsigned int is_mute)
{
    audio_lr_digital_volume_control_RBUS DVOL_CH_LR_REG;
    audio_digital_volume_control_1_RBUS DVOL_CTRL_REG;
    audio_lsrs_digital_volume_control_RBUS DVOL_CH_LSRS_REG;
    audio_csw_digital_volume_control_RBUS DVOL_CH_CSW_REG;

    if(ch_id == 0)
        return -1;

    // change dvol setting to MUTE mode
    DVOL_CTRL_REG.regValue = rtd_inl(AUDIO_Digital_Volume_Control_1_reg);
    DVOL_CTRL_REG.mute_apply = TRUE;
    rtd_outl(AUDIO_Digital_Volume_Control_1_reg, DVOL_CTRL_REG.regValue);

    if(ch_id & APP_CH_ID_L) {
        DVOL_CH_LR_REG.regValue = rtd_inl(AUDIO_LR_Digital_Volume_Control_reg);
        DVOL_CH_LR_REG.mute_l = is_mute;
        DVOL_CH_LR_REG.rep_l_status = TRUE;
        DVOL_CH_LR_REG.hpf_l_enable = TRUE;
        rtd_outl(AUDIO_LR_Digital_Volume_Control_reg, DVOL_CH_LR_REG.regValue);
    }
    if(ch_id & APP_CH_ID_R) {
        DVOL_CH_LR_REG.regValue = rtd_inl(AUDIO_LR_Digital_Volume_Control_reg);
        DVOL_CH_LR_REG.mute_r = is_mute;
        DVOL_CH_LR_REG.rep_r_status = TRUE;
        DVOL_CH_LR_REG.hpf_r_enable = TRUE;
        rtd_outl(AUDIO_LR_Digital_Volume_Control_reg, DVOL_CH_LR_REG.regValue);
    }
    if(ch_id & APP_CH_ID_LS) {
        DVOL_CH_LSRS_REG.regValue = rtd_inl(AUDIO_LsRs_Digital_Volume_Control_reg);
        DVOL_CH_LSRS_REG.mute_ls = is_mute;
        DVOL_CH_LSRS_REG.rep_ls_status = TRUE;
        DVOL_CH_LSRS_REG.hpf_ls_enable = TRUE;
        rtd_outl(AUDIO_LsRs_Digital_Volume_Control_reg, DVOL_CH_LSRS_REG.regValue);
    }
    if(ch_id & APP_CH_ID_RS) {
        DVOL_CH_LSRS_REG.regValue = rtd_inl(AUDIO_LsRs_Digital_Volume_Control_reg);
        DVOL_CH_LSRS_REG.mute_rs = is_mute;
        DVOL_CH_LSRS_REG.rep_rs_status = TRUE;
        DVOL_CH_LSRS_REG.hpf_rs_enable = TRUE;
        rtd_outl(AUDIO_LsRs_Digital_Volume_Control_reg, DVOL_CH_LSRS_REG.regValue);
    }
    if(ch_id & APP_CH_ID_C) {
        DVOL_CH_CSW_REG.regValue = rtd_inl(AUDIO_CSW_Digital_Volume_Control_reg);
        DVOL_CH_CSW_REG.mute_c = is_mute;
        DVOL_CH_CSW_REG.rep_c_status = TRUE;
        DVOL_CH_CSW_REG.hpf_c_enable = TRUE;
        rtd_outl(AUDIO_CSW_Digital_Volume_Control_reg, DVOL_CH_CSW_REG.regValue);
    }
    if(ch_id & APP_CH_ID_SW) {
        DVOL_CH_CSW_REG.regValue = rtd_inl(AUDIO_CSW_Digital_Volume_Control_reg);
        DVOL_CH_CSW_REG.mute_sw = is_mute;
        DVOL_CH_CSW_REG.rep_sw_status = TRUE;
        DVOL_CH_CSW_REG.hpf_sw_enable = TRUE;
        rtd_outl(AUDIO_CSW_Digital_Volume_Control_reg, DVOL_CH_CSW_REG.regValue);
    }

    DVOL_CTRL_REG.regValue = rtd_inl(AUDIO_Digital_Volume_Control_1_reg);
    DVOL_CTRL_REG.dvol_apply = TRUE;
    rtd_outl(AUDIO_Digital_Volume_Control_1_reg, DVOL_CTRL_REG.regValue);

    return 0;
}


static void HwpSetVolume(unsigned int ch_id, int vol)
{
    audio_digital_volume_control_1_RBUS DVOL_CTRL_REG;
    audio_lr_digital_volume_control_RBUS DVOL_CH_LR_REG;
    audio_lsrs_digital_volume_control_RBUS DVOL_CH_LSRS_REG;
    audio_csw_digital_volume_control_RBUS DVOL_CH_CSW_REG;
    int a,b,c;

    if(vol > APP_MAX_VOL) vol = APP_MAX_VOL;
    if(vol < APP_MIN_VOL) vol = APP_MIN_VOL;

    // change dvol setting to DVOL mode
    DVOL_CTRL_REG.regValue = rtd_inl(AUDIO_Digital_Volume_Control_1_reg);
    DVOL_CTRL_REG.mute_apply = FALSE;
    rtd_outl(AUDIO_Digital_Volume_Control_1_reg, DVOL_CTRL_REG.regValue);

    c = vol & 0x7;
    a = (vol>>3) / 6;
    b = (vol>>3) % 6;
    if(b < 0) {
        a--;
        b += 6;
    }

    if(ch_id & APP_CH_ID_L) {
        DVOL_CH_LR_REG.regValue = rtd_inl(AUDIO_LR_Digital_Volume_Control_reg);
        DVOL_CH_LR_REG.rep_l_status= 1;
        DVOL_CH_LR_REG.hpf_l_enable = 1;
        DVOL_CH_LR_REG.vol_l_a = a;
        DVOL_CH_LR_REG.vol_l_b = (b<<3) | c;
        rtd_outl(AUDIO_LR_Digital_Volume_Control_reg, DVOL_CH_LR_REG.regValue);
    }
    if(ch_id & APP_CH_ID_R) {
        DVOL_CH_LR_REG.regValue = rtd_inl(AUDIO_LR_Digital_Volume_Control_reg);
        DVOL_CH_LR_REG.rep_r_status= 1;
        DVOL_CH_LR_REG.hpf_r_enable = 1;
        DVOL_CH_LR_REG.vol_r_a = a;
        DVOL_CH_LR_REG.vol_r_b = (b<<3) | c;
        rtd_outl(AUDIO_LR_Digital_Volume_Control_reg, DVOL_CH_LR_REG.regValue);
    }
    if(ch_id & APP_CH_ID_LS) {
        DVOL_CH_LSRS_REG.regValue = rtd_inl(AUDIO_LsRs_Digital_Volume_Control_reg);
        DVOL_CH_LSRS_REG.rep_ls_status= 1;
        DVOL_CH_LSRS_REG.hpf_ls_enable = 1;
        DVOL_CH_LSRS_REG.vol_ls_a = a;
        DVOL_CH_LSRS_REG.vol_ls_b = (b<<3) | c;
        rtd_outl(AUDIO_LsRs_Digital_Volume_Control_reg, DVOL_CH_LSRS_REG.regValue);
    }
    if(ch_id & APP_CH_ID_RS) {
        DVOL_CH_LSRS_REG.regValue = rtd_inl(AUDIO_LsRs_Digital_Volume_Control_reg);
        DVOL_CH_LSRS_REG.rep_rs_status= 1;
        DVOL_CH_LSRS_REG.hpf_rs_enable = 1;
        DVOL_CH_LSRS_REG.vol_rs_a = a;
        DVOL_CH_LSRS_REG.vol_rs_b = (b<<3) | c;
        rtd_outl(AUDIO_LsRs_Digital_Volume_Control_reg, DVOL_CH_LSRS_REG.regValue);
    }
    if(ch_id & APP_CH_ID_C) {
        DVOL_CH_CSW_REG.regValue = rtd_inl(AUDIO_CSW_Digital_Volume_Control_reg);
        DVOL_CH_CSW_REG.rep_c_status= 1;
        DVOL_CH_CSW_REG.hpf_c_enable = 1;
        DVOL_CH_CSW_REG.vol_c_a = a;
        DVOL_CH_CSW_REG.vol_c_b = (b<<3) | c;
        rtd_outl(AUDIO_CSW_Digital_Volume_Control_reg, DVOL_CH_CSW_REG.regValue);
    }
    if(ch_id & APP_CH_ID_SW) {
        DVOL_CH_CSW_REG.regValue = rtd_inl(AUDIO_CSW_Digital_Volume_Control_reg);
        DVOL_CH_CSW_REG.rep_sw_status= 1;
        DVOL_CH_CSW_REG.hpf_sw_enable = 1;
        DVOL_CH_CSW_REG.vol_sw_a = a;
        DVOL_CH_CSW_REG.vol_sw_b = (b<<3) | c;
        rtd_outl(AUDIO_CSW_Digital_Volume_Control_reg, DVOL_CH_CSW_REG.regValue);
    }

    DVOL_CTRL_REG.regValue = rtd_inl(AUDIO_Digital_Volume_Control_1_reg);
    DVOL_CTRL_REG.dvol_apply = 1;
    rtd_outl(AUDIO_Digital_Volume_Control_1_reg, DVOL_CTRL_REG.regValue);
}


static void set_HP_mute(bool mute)
{
    HwpSetMute(APP_CH_ID_LS|APP_CH_ID_RS, mute);
}


static void set_SPEAKER_mute(bool mute)
{
    HwpSetMute(APP_CH_ID_L|APP_CH_ID_R, mute);
}


static int update_volume(int volume)
{
    int db, prescale;
    enum SOURCE source;

    if (volume < 0 || volume > 100)
        return -EINVAL;

    if (is_QS_hdmi_enable()) {
        source = SOURCE_HDMI;
    } else if (is_QS_dp_enable()) {
        source = SOURCE_DP;
    } else {
        return -ENODEV;
    }

    if (is_QS_hp_enable()) {
        db = volume_to_db(volume, source, AOUTDEV_HP);
        //HwpSetVolume(APP_CH_ID_LS|APP_CH_ID_RS, (db * APP_VOL_STEP_1DB));
        HwpSetVolume(APP_CH_ID_LS|APP_CH_ID_RS, db); // FIXME: we don't need to * 8 here ?

    } else {
        prescale = PRESCALE_SPK[source] / 8;
        if (audio_hwp_prescal) {
            audio_hwp_prescal(prescale, APP_CH_ID_CH0);
            audio_hwp_prescal(prescale, APP_CH_ID_CH1);
        }

        db = volume_to_db(volume, source, AOUTDEV_SPK);
        //HwpSetVolume(APP_CH_ID_L|APP_CH_ID_R, (db * APP_VOL_STEP_1DB));
        HwpSetVolume(APP_CH_ID_L|APP_CH_ID_R, db); // FIXME: we don't need to * 8 here ?
    }

    return 0;
}

static int handle_volume_change(int volume)
{
    int ret = 0;

    if (!is_QS_active())
        return -EPERM;

    volume_adj += volume;

    if (volume_adj <= 0) {
        mute = true;

        if (is_QS_hp_enable())
            set_HP_mute(mute);
        else
            set_SPEAKER_mute(mute);

        volume_adj = 1;
    } else {
        ret = update_volume(volume_adj);
        if (ret) {
            volume_adj -= volume;  // roll back
        }
    }

    return ret;
}


void qs_hwp_prescale_register(void (*func)(int, int))
{
    audio_hwp_prescal = func;
}
EXPORT_SYMBOL(qs_hwp_prescale_register);


void qs_hwp_prescale_unregister(void (*func)(int, int))
{
    audio_hwp_prescal = NULL;
}
EXPORT_SYMBOL(qs_hwp_prescale_unregister);


static int input_QS_key_event_notifier_fn(struct notifier_block *nb, unsigned long action, void *data)
{
    int ret = 0;

    rtd_pr_quick_show_debug("%s:: action = %x, type = %x, code =%x, value = %x\n",
            __func__,
            action, QS_GET_EV_TYPE(action),
            QS_GET_EV_CODE(action), QS_GET_EV_VALUE(action));

    // TODO: base on different key event, call related handle
    switch (QS_GET_EV_CODE(action)) {
        case KEY_HOME:
            break;
        case KEY_VOLUMEDOWN:
            if (QS_GET_EV_VALUE(action) == 0) { // release key
                ret = handle_volume_change(-1);
            }
            break;
        case KEY_VOLUMEUP:
            if (QS_GET_EV_VALUE(action) == 0) { // release key
                ret = handle_volume_change(1);
            }
            break;
        case KEY_MUTE:
            if (QS_GET_EV_VALUE(action) == 0) { // release key
                mute = !mute;

                if (is_QS_hp_enable())
                    set_HP_mute(mute);
                else
                    set_SPEAKER_mute(mute);
            }
            break;
        default:
            break;
    }

    return ret;
}

static struct notifier_block g_QS_key_event_input_notifier = {
	.notifier_call = input_QS_key_event_notifier_fn,
};


struct virtual_mouse_state {
    struct input_handle handle;
};

#undef bool
static bool input_handler_registered = false;

static bool virtual_mouse_filter(struct input_handle *handle,
             unsigned int type, unsigned int code, int value)
{
    return false;
}

static int virtual_mouse_connect(struct input_handler *handler,
             struct input_dev *dev,
             const struct input_device_id *id)
{
    struct virtual_mouse_state *virtual_mouse;
    int error;

    virtual_mouse = kzalloc(sizeof(struct virtual_mouse_state), GFP_KERNEL);
    if (!virtual_mouse)
        return -ENOMEM;

    virtual_mouse->handle.dev = dev;
    virtual_mouse->handle.handler = handler;
    virtual_mouse->handle.name = "virtual_mouse";
    virtual_mouse->handle.private = virtual_mouse;

    error = input_register_handle(&virtual_mouse->handle);
    if (error) {
        pr_err("Failed to register input virtual_mouse handler, error %d\n",
            error);
        goto err_free;
    }

    error = input_open_device(&virtual_mouse->handle);
    if (error) {
        pr_err("Failed to open input device, error %d\n", error);
        goto err_unregister;
    }

    return 0;

 err_unregister:
    input_unregister_handle(&virtual_mouse->handle);
 err_free:
    kfree(virtual_mouse);
    return error;
}

static void virtual_mouse_disconnect(struct input_handle *handle)
{
    struct virtual_mouse_state *virtual_mouse = handle->private;

    input_close_device(handle);
    input_unregister_handle(handle);
    kfree(virtual_mouse);
}


static const struct input_device_id virtual_mouse_ids[] = {
    {
        .driver_info = 1
    },
    { },
};


static struct input_handler virtual_mouse_handler = {
    .filter     = virtual_mouse_filter,
    .connect    = virtual_mouse_connect,
    .disconnect = virtual_mouse_disconnect,
    .name       = "virtual_mouse",
    .id_table   = virtual_mouse_ids,
};


static int __init rtk_qs_init(void)
{
    int ret = 0;
    enum SOURCE source = SOURCE_NONE;
    enum AOUTDEV aoutdev;
    bool active = is_QS_active();

    if (active) {
        if (is_QS_hdmi_enable())
            source = SOURCE_HDMI;
        else if (is_QS_dp_enable())
            source = SOURCE_DP;

        volume_adj = get_QS_volume();
        mute = get_QS_mute();

        update_volume(volume_adj);

        if (source == SOURCE_HDMI || source == SOURCE_DP) {
            if (source == SOURCE_DP) {
                int status;

                RTK_USB_Typec_Hidg_enable(true);

                status = input_register_handler(&virtual_mouse_handler);
                if (status)
                    pr_err("%s: Failed to register input handler, error %d", __func__, status);
                else
                    input_handler_registered = true;
            }

            if (is_QS_hp_enable()) {
                aoutdev = AOUTDEV_HP;
                set_HP_mute(mute);
            } else {
                aoutdev = AOUTDEV_SPK;
                set_SPEAKER_mute(mute);
            }
        }

        ret = ir_input_register_QS_key_event_notifier(&g_QS_key_event_input_notifier);
        if (ret) {
            rtd_pr_quick_show_emerg("%s: input_register_QS_key_event_notifier fail. ret=%d\n", __func__, ret);
            goto FAIL_REG_EVENT;
        }
        set_QS_handle_key(true);
    }

    ret = rtk_qs_cdev_init();
    if (ret)
        goto FAIL_CDEV_INIT;

    rtd_pr_quick_show_info("%s: done\n", __func__);

    return 0;

FAIL_CDEV_INIT:
    if (active)
        ir_input_unregister_QS_key_event_notifier(&g_QS_key_event_input_notifier);
FAIL_REG_EVENT:
    return ret;
}


static void __exit rtk_qs_exit(void)
{
    if (input_handler_registered)
        input_unregister_handler(&virtual_mouse_handler);

    rtk_qs_cdev_uninit();
}


module_init(rtk_qs_init);
module_exit(rtk_qs_exit);

MODULE_LICENSE("GPL");
#endif /* !BUILD_QUICK_SHOW */
