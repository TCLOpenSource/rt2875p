#include <linux/kconfig.h>
#include <linux/compiler.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/stat.h>         /* permission */
#include <linux/fs.h>           /* fs, ioctl */
#include <linux/errno.h>        /* error codes */
#include <linux/types.h>        /* size_t */
#include <linux/fcntl.h>        /* O_ACCMODE */
#include <linux/uaccess.h>      /* copy_*_user */
#include <linux/mutex.h>        /* mutex */
#include <linux/device.h>       /* device */
#include <linux/cdev.h>         /* char device */
#if 0//ndef USE_EARC_NEW
    #include <linux/videodev2.h>
    #include <../linux/v4l2-ext/v4l2-controls-ext.h>
    #include <../linux/v4l2-ext/videodev2-ext.h>
#endif

#include "audio_hw_port.h"
#include "audio_hw_atv.h"
#include "audio_hw_aio.h"
#include "audio_hw_app.h"
#include "audio_hw_earc_driver.h"
#include "audio_hw_earc_ioctl.h"

#define __ALOG_SUBTAG "ioctl"

#define data_from_user(d, arg) do { \
        if (copy_from_user((void *)&d, arg, sizeof(d))) { \
            alog_err("copy_from_user() failed\n"); \
            return -EFAULT; \
        } \
    } while(0)

#define data_to_user(d, arg) do { \
        if (copy_to_user(arg, (void *)&d, sizeof(d))) { \
            alog_err("copy_to_user() failed\n"); \
            return -EFAULT; \
        } \
    } while(0)

extern EARC_STATE_T g_Earc_State;

extern char g_EarcPort;


#define EARC_READ_DEBOUNCE 2

static unsigned int EARC_INIT_STATE = 0;
unsigned int EARC_READ_COUNT = 0;
unsigned char g_writecmd_flag = 0, g_writecmd = 0;
#if 1//def USE_EARC_NEW
static long audio_hw_Set_EARC(void __user *arg)
{
    long ret = 0;
    AUDIOHW_EARC_T Para;

    data_from_user(Para, arg);
    alog_err("earc, en=%d, port=%d", Para.earc_enable_state, Para.port);
//if need to check port with ap, reopen
#if 0
    if (Para.port != g_EarcPort) {
		alog_info("earc port mismatch: %x\n",Para.port);
        return -ENOTTY;
    }
#endif

    if (Para.earc_enable_state == AUDIOHW_EARC_ENABLE && EARC_INIT_STATE == 0) {
            Audio_HW_Earc_En(1);
            Audio_HW_Earc_Init_Detection();
            // enable route to SCPU
            rtd_outl(SYS_REG_INT_CTRL_SCPU_2_reg, VERIFY_BIT31 | VERIFY_BIT0);
            EARC_INIT_STATE = 1;
        }
    else if (Para.earc_enable_state == AUDIOHW_EARC_DISABLE && EARC_INIT_STATE == 1) {
            int32_t IRQ_Reg = 0;

            Audio_HW_Earc_En(0);
            // disable route to SCPU
            IRQ_Reg = AIO_ReadRegister(SYS_REG_INT_CTRL_SCPU_2_reg);
            IRQ_Reg = (IRQ_Reg & (~VERIFY_BIT31)) | VERIFY_BIT0;
            AIO_WriteRegister(SYS_REG_INT_CTRL_SCPU_2_reg, IRQ_Reg);
            Audio_HW_Earc_Disable_Detection();
            EARC_INIT_STATE = 0;
            EARC_READ_COUNT = 0;
        }


    return ret;
}

static long audio_hw_Get_EARC(void __user *arg)
{
    long ret = 0;
    AUDIOHW_EARC_T Para;

    data_from_user(Para, arg);
//if need to check port with ap, reopen
#if 0
    if (Para.port != g_EarcPort) {
		alog_info("earc port mismatch: %x\n",Para.port);
        return -ENOTTY;
    }
#endif

    if (EARC_INIT_STATE == 1) {
            Para.earc_enable_state = AUDIOHW_EARC_ENABLE;
        }
    else {
            Para.earc_enable_state = AUDIOHW_EARC_DISABLE;
        }

    data_to_user(Para, arg);
    return ret;
}


static long audio_hw_EARC_CONNECTION_INFO(void __user *arg)
{
    long ret = 0;
    AUDIOHW_EARC_INFO_T Para;
    AIO_AUDIO_EARC_RX_STATUS_T RxState = AIO_AUDIO_EARC_RX_STATUS_NONE;
    AIO_AUDIO_EARC_CMDC_READ_T ReadTransaction;

    data_from_user(Para, arg);
    if (g_Earc_State.ConnectState == AIO_EARC_STATE_IDLE1) {
        Para.status = AUDIOHW_EARC_IDLE1;
    }
    else if (g_Earc_State.ConnectState == AIO_EARC_STATE_IDLE2) {
        Para.status = AUDIOHW_EARC_IDLE2;
    }
    else if (g_Earc_State.ConnectState == AIO_EARC_STATE_DISC1) {
        Para.status = AUDIOHW_EARC_DISC1;
    }
    else if (g_Earc_State.ConnectState == AIO_EARC_STATE_DISC2) {
        Para.status = AUDIOHW_EARC_DISC2;
    }
    else {
        Para.status = AUDIOHW_EARC_EARC;
    }

    if (g_Earc_State.ReadCapReady == false) {
        Para.erx_readcap_ready = false;
    }
    else {
        Para.erx_readcap_ready = true;
    }
    //alog_info("HandshakingState = %d\n", HandshakingState);
    if (g_Earc_State.ConnectState == AIO_EARC_STATE_EARC) {
        if (EARC_READ_COUNT < 50) {
            ++EARC_READ_COUNT;
        }
        Audio_HW_Earc_Get_Rx_Status(&RxState);
        if (1) { //EARC_READ_COUNT > 4)
            if (EARC_READ_COUNT > 5 && g_writecmd_flag == 1) { //delay write laency, if <3 HB issue write latency, Rx AVR will have problem
                if (Audio_HW_Earc_Write_Latency((uint8_t)g_writecmd) != AIO_OK) {
                    alog_err("[eARC] write latency err\n");
                }
                else {
                    alog_err("[eARC] write latency pass=%x\n", g_writecmd);
                    g_writecmd_flag = 0;
                }
            }

            if (EARC_READ_COUNT > 3) {
                //alog_err("[eARC] read latency s\n");
                ReadTransaction.Type = AIO_AUDIO_EARC_READ_LATENCY;
                ReadTransaction.ReadData = &Para.erx_latency;
                if (Audio_HW_Earc_Read_Transaction(&ReadTransaction) != AIO_OK) {
                    alog_err("[eARC] read latency err\n");
                    EARC_READ_COUNT = 0;
                }
                else {
                    //alog_err("[eARC] read latency pass, latency=%d\n", *ReadTransaction.ReadData);
                }
            }

            if (EARC_READ_COUNT > 3) {
                ReadTransaction.Type = AIO_AUDIO_EARC_READ_CAP;
                ReadTransaction.ReadData = &Para.capability[0];
                if (Audio_HW_Earc_Read_Transaction(&ReadTransaction) != AIO_OK) {
                    alog_err("[eARC] read cap err\n");
                    EARC_READ_COUNT = 0;
                }
                else {
                    //alog_err("[eARC] read cap pass\n");
#if 0
                    for (int i = 0; i < 255; ++i) {
                        alog_err("%d ", Para.capability[i]);
                    }
#endif
                }
            }
        }
    }
    else {
        EARC_READ_COUNT = 0;
    }

    data_to_user(Para, arg);
    return ret;
}

static long audio_hw_EARC_SetHdmiHpd(void __user *arg)
{
    long ret = 0;

    if (g_Earc_State.ConnectState == AIO_EARC_STATE_EARC) {
        Audio_HW_Earc_ResetHdmiHpd();
    }
    else {
        if (g_Earc_State.ConnectState != AIO_EARC_STATE_EARC) {
            alog_err("set hdmi_hpd error, not in earc state\n");
        }

        ret = -1;
    }

    return ret;
}

static long audio_hw_EARC_SET_ERX_LATENCY_REQ(void __user *arg)
{
    long ret = 0;
    AUDIOHW_EARC_INFO_T Para;
    int retry = 0;

    data_from_user(Para, arg);
    if (g_Earc_State.ConnectState == AIO_EARC_STATE_EARC/* && EARC_READ_COUNT > 3*/) {
        if (EARC_READ_COUNT < 4) {
            g_writecmd_flag = 1;
            g_writecmd = (uint8_t)Para.erx_latency_req;
        }
        else {
            while (retry < 3) {
            if (Audio_HW_Earc_Write_Latency((uint8_t)Para.erx_latency_req) != AIO_OK) {
                ret = -ENOTTY;
                alog_err("[eARC] write latency err\n");
                    retry++;
                }
                else {
                    ret = 0;
                    alog_err("[eARC] write latency pass=%x\n", Para.erx_latency_req);
                    break;
                }
            }
        }
    }
    else { ret = -ENOTTY; }

    return ret;
}

long audio_hw_earc_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    struct audio_hw_earc_drvdata *drvdata = NULL;
    void __user *uarg = (void __user *)arg;
    int ret = 0;

    drvdata = (struct audio_hw_earc_drvdata *)filp->private_data;
    if (_IOC_NR(cmd) >= AUDIO_IOC_EARC_MAXNR) {
        alog_err("cmd nr overflow\n");
        return -ENOIOCTLCMD;
    }

    if (!drvdata) {
        alog_err("null drvdata\n");
        ret = -EFAULT;
        goto exit_error;
    }

    if (mutex_lock_interruptible(&drvdata->mtx)) {
        alog_warning("interrupted, no mutex held\n");
        ret = -ERESTARTSYS;
        goto exit_error;
    }

    switch (cmd) {
        case AUDIO_IOC_SET_EARC_EN: {
            ret = audio_hw_Set_EARC(uarg);
            break;
        }
        case AUDIO_IOC_GET_EARC_EN: {
            ret = audio_hw_Get_EARC(uarg);
            break;
        }
        case AUDIO_IOC_EARC_INFO: {
            ret = audio_hw_EARC_CONNECTION_INFO(uarg);
            break;
        }
        case AUDIO_IOC_EARC_SET_ERX_LATENCY_REQ: {
            ret = audio_hw_EARC_SET_ERX_LATENCY_REQ(uarg);
            break;
        }
        case AUDIO_IOC_EARC_RESET_HDMI_HPD_BIT: {
            ret = audio_hw_EARC_SetHdmiHpd(uarg);
            break;
        }
        default: {
            ret = -ENOIOCTLCMD;
            break;
        }
    }

    if (ret) {
        goto exit_mutex_unlock;
    }

    mutex_unlock(&drvdata->mtx);
    return 0;

exit_mutex_unlock:
    mutex_unlock(&drvdata->mtx);
exit_error:
    alog_err("earc ioctl failed with %d cmd %x (%c, %d)\n", ret, cmd,
             (char)_IOC_TYPE(cmd), _IOC_NR(cmd));
    return ret;
}

#else
#ifdef SUPPORT_EARC
static long audio_hw_Set_EARC(unsigned long *arg)
{
    long ret = 0;
    void __user *uarg = (void __user *)arg;
    struct v4l2_ext_earc Para;

    data_from_user(Para, uarg);
    alog_err("earc, en=%d, port=%d", Para.earc_enable_state, Para.port);
    if (Para.port != V4L2_EXT_EARC_OUTPUT_PORT_3 && Para.port != V4L2_EXT_EARC_OUTPUT_PORT_NONE) {
        return -ENOTTY;
    }
    else {
        if (Para.earc_enable_state == V4L2_EXT_EARC_ENABLE && EARC_INIT_STATE == 0) {
            Audio_HW_Earc_En(1);
            Audio_HW_Earc_Init_Detection();
            // enable route to SCPU
            rtd_outl(SYS_REG_INT_CTRL_SCPU_2_reg, VERIFY_BIT31 | VERIFY_BIT0);
            EARC_INIT_STATE = 1;
        }
        else if (Para.earc_enable_state == V4L2_EXT_EARC_DISABLE && EARC_INIT_STATE == 1) {
            int32_t IRQ_Reg = 0;

            Audio_HW_Earc_En(0);
            // disable route to SCPU
            IRQ_Reg = AIO_ReadRegister(SYS_REG_INT_CTRL_SCPU_2_reg);
            IRQ_Reg = (IRQ_Reg & (~VERIFY_BIT31)) | VERIFY_BIT0;
            AIO_WriteRegister(SYS_REG_INT_CTRL_SCPU_2_reg, IRQ_Reg);
            Audio_HW_Earc_Disable_Detection();
            EARC_INIT_STATE = 0;
            EARC_READ_COUNT = 0;
        }
    }

    return ret;
}

static long audio_hw_Get_EARC(unsigned long *arg)
{
    long ret = 0;
    void __user *uarg = (void __user *)arg;
    struct v4l2_ext_earc Para;

    data_from_user(Para, uarg);
    if (Para.port != V4L2_EXT_EARC_OUTPUT_PORT_3 && Para.port != V4L2_EXT_EARC_OUTPUT_PORT_NONE) {
        return -ENOTTY;
    }
    else {
        if (EARC_INIT_STATE == 1) {
            Para.earc_enable_state = V4L2_EXT_EARC_ENABLE;
        }
        else {
            Para.earc_enable_state = V4L2_EXT_EARC_DISABLE;
        }
    }

    data_to_user(Para, uarg);
    return ret;
}


static long audio_hw_EARC_CONNECTION_INFO(unsigned long *arg)
{
    long ret = 0;
    void __user *uarg = (void __user *)arg;
    struct v4l2_ext_earc_connection_info Para;
    AIO_AUDIO_EARC_RX_STATUS_T RxState = AIO_AUDIO_EARC_RX_STATUS_NONE;
    AIO_AUDIO_EARC_CMDC_READ_T ReadTransaction;

    data_from_user(Para, uarg);
    if (g_Earc_State.ConnectState == AIO_EARC_STATE_IDLE1) {
        Para.status = V4L2_EXT_EARC_IDLE1;
    }
    else if (g_Earc_State.ConnectState == AIO_EARC_STATE_IDLE2) {
        Para.status = V4L2_EXT_EARC_IDLE2;
    }
    else if (g_Earc_State.ConnectState == AIO_EARC_STATE_DISC1) {
        Para.status = V4L2_EXT_EARC_DISC1;
    }
    else if (g_Earc_State.ConnectState == AIO_EARC_STATE_DISC2) {
        Para.status = V4L2_EXT_EARC_DISC2;
    }
    else {
        Para.status = V4L2_EXT_EARC_EARC;
    }

    //alog_info("HandshakingState = %d\n", HandshakingState);
    if (g_Earc_State.ConnectState == AIO_EARC_STATE_EARC) {
        if (EARC_READ_COUNT < 50) {
            ++EARC_READ_COUNT;
        }

        Audio_HW_Earc_Get_Rx_Status(&RxState);
        if (1) { //EARC_READ_COUNT > 4)
            if (EARC_READ_COUNT > 3) {
                //alog_err("[eARC] read latency s\n");
                ReadTransaction.Type = AIO_AUDIO_EARC_READ_LATENCY;
                ReadTransaction.ReadData = &Para.erx_latency;
                if (Audio_HW_Earc_Read_Transaction(&ReadTransaction) != AIO_OK) {
                    alog_err("[eARC] read latency err\n");
                }
                else {
                    //alog_err("[eARC] read latency pass, latency=%d\n", *ReadTransaction.ReadData);
                }
            }

            if (EARC_READ_COUNT > 3) {
                ReadTransaction.Type = AIO_AUDIO_EARC_READ_CAP;
                ReadTransaction.ReadData = &Para.capability[0];
                if (Audio_HW_Earc_Read_Transaction(&ReadTransaction) != AIO_OK) {
                    alog_err("[eARC] read cap err\n");
                }
                else {
                    //alog_err("[eARC] read cap pass\n");
#if 0
                    for (int i = 0; i < 255; ++i) {
                        alog_err("%d ", Para.capability[i]);
                    }
#endif
                }
            }
        }
    }

    data_to_user(Para, uarg);
    return ret;
}

static long audio_hw_EARC_SetHdmiHpd(unsigned long *arg)
{
    long ret = 0;
    struct v4l2_ext_earc_connection_info Para;
    void __user *uarg = (void __user *)arg;

    data_from_user(Para, uarg);
    if (g_Earc_State.ConnectState == AIO_EARC_STATE_EARC &&
            (Para.port == V4L2_EXT_EARC_OUTPUT_PORT_3 || Para.port == V4L2_EXT_EARC_OUTPUT_PORT_NONE)) {
        Audio_HW_Earc_ResetHdmiHpd();
    }
    else {
        if (g_Earc_State.ConnectState != AIO_EARC_STATE_EARC) {
            alog_err("set hdmi_hpd error, not in earc state\n");
        }
        if (Para.port != V4L2_EXT_EARC_OUTPUT_PORT_2) {
            alog_err("set hdmi_hpd error, not V4L2_EXT_EARC_OUTPUT_PORT_2\n");
        }

        ret = -1;
    }

    return ret;
}

static long audio_hw_EARC_SET_ERX_LATENCY_REQ(unsigned long *arg)
{
    long ret = 0;
    void __user *uarg = (void __user *)arg;
    struct v4l2_ext_earc_connection_info Para;

    data_from_user(Para, uarg);
    if (g_Earc_State.ConnectState == AIO_EARC_STATE_EARC/* && EARC_READ_COUNT > 3*/) {
        if (Audio_HW_Earc_Write_Latency((uint8_t)Para.erx_latency_req) != AIO_OK) {
            ret = -ENOTTY;
            alog_err("[eARC] write latency err\n");
        }
        else { alog_err("[eARC] write latency pass\n"); }
    }
    else { ret = -ENOTTY; }

    return ret;
}
#endif
long audio_hw_earc_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    struct audio_hw_earc_drvdata *drvdata = NULL;
    void __user *uarg = (void __user *)arg;
    int ret = 0;
    struct v4l2_ext_controls ext_controls;
    struct v4l2_ext_control ext_control;

    memset(&ext_controls, 0, sizeof(struct v4l2_ext_controls));
    memset(&ext_control, 0, sizeof(struct v4l2_ext_control));
    drvdata = (struct audio_hw_earc_drvdata *)filp->private_data;
    if (!drvdata) {
        alog_err("null drvdata\n");
        ret = -EFAULT;
        goto exit_error;
    }

    if (mutex_lock_interruptible(&drvdata->mtx)) {
        alog_warning("interrupted, no mutex held\n");
        ret = -ERESTARTSYS;
        goto exit_error;
    }

    data_from_user(ext_controls, uarg);
    uarg = (void __user *)ext_controls.controls;
    data_from_user(ext_control, uarg);
    switch (ext_control.id) {
        case V4L2_CID_EXT_EARC: {
            if (cmd == VIDIOC_S_EXT_CTRLS) {
                ret = audio_hw_Set_EARC(ext_control.ptr);
            }
            else {
                ret = audio_hw_Get_EARC(ext_control.ptr);
            }

            break;
        }
        case V4L2_CID_EXT_EARC_CONNECTION_INFO: {
            ret = audio_hw_EARC_CONNECTION_INFO(ext_control.ptr);
            break;
        }
        case V4L2_CID_EXT_EARC_SET_ERX_LATENCY_REQ: {
            ret = audio_hw_EARC_SET_ERX_LATENCY_REQ(ext_control.ptr);
            break;
        }
        case V4L2_CID_EXT_EARC_RESET_HDMI_HPD_BIT: {
            if (cmd == VIDIOC_S_EXT_CTRLS) {
                ret = audio_hw_EARC_SetHdmiHpd(ext_control.ptr);
            }

            break;
        }
        default: {
            ret = -ENOIOCTLCMD;
            break;
        }
    }

    if (ret) {
        goto exit_mutex_unlock;
    }

    mutex_unlock(&drvdata->mtx);
    return 0;

exit_mutex_unlock:
    mutex_unlock(&drvdata->mtx);
exit_error:
    alog_err("ioctl failed with %d cmd %x (%c, %d)\n", ret, cmd,
             (char)_IOC_TYPE(cmd), _IOC_NR(cmd));
    return ret;
}
#endif

