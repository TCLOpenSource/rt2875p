#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/string.h>
#include <mach/rtk_platform.h>
#include <asm-generic/errno-base.h>
#include <asm-generic/ioctl.h>
#ifdef CONFIG_COMPAT
#include <linux/compat.h>
#define to_user_ptr(x)          compat_ptr((unsigned int) x)
#else
#define to_user_ptr(x)          ((void* __user)(x)) // convert 32 bit value to user pointer
#endif

#include <ioctrl/scaler/vfe_cmd_id.h>
#include <rtk_kdriver/dprx/dprx_vfe_api.h>
#include "scaler_vfedev-dprx.h"


static long __copy_attr_to_user(VFE_DPRX_ATTR* p_usr_attr, VFE_DPRX_ATTR* p_kernel_attr)
{
    if (p_usr_attr==NULL || p_kernel_attr==NULL ||
        copy_to_user((void __user *)p_usr_attr, (void *)p_kernel_attr, sizeof(VFE_DPRX_ATTR)))
    {
        DPRX_VFE_ERR("copy dprx attribute to user space failed\n");
        return -EFAULT;
    }
    return 0;
}

static long __copy_attr_from_user(VFE_DPRX_ATTR* p_kernel_attr, VFE_DPRX_ATTR* p_usr_attr)
{
    if (p_usr_attr==NULL || p_kernel_attr==NULL ||
        copy_from_user((void *)p_kernel_attr, (void __user *)p_usr_attr, sizeof(VFE_DPRX_ATTR)))
    {
        DPRX_VFE_ERR("copy dprx attribute from user space failed\n");
        return -EFAULT;
    }

    return 0;
}

static int copy_dp_port_capability_to_kadp(KADP_VFE_DPRX_PORT_CAP_T* pCap, DP_PORT_CAPABILITY* pDpCap)
{
    int ret = DPRX_DRV_ARG_ERR;
    if((pCap != NULL) && (pDpCap != NULL))
    {
        memset(pCap, 0, sizeof(KADP_VFE_DPRX_PORT_CAP_T));
        switch(pDpCap->type)
        {
            case DP_TYPE_DP:
                pCap->type = KADP_VFE_DPRX_PORT_TYPE_DP;
                break;

            case DP_TYPE_PSEUDO_HDMI_DP:
                pCap->type = KADP_VFE_DPRX_PORT_TYPE_DP;
                break;

            case DP_TYPE_PSEUDO_DP:
                pCap->type = KADP_VFE_DPRX_PORT_TYPE_DP;
                break;

            case DP_TYPE_mDP:
                pCap->type = KADP_VFE_DPRX_PORT_TYPE_MDP;
                break;

            case DP_TYPE_USB_TYPE_C:
                pCap->type = KADP_VFE_DPRX_PORT_TYPE_TYPEC;
                break;

            case DP_TYPE_MAX:
            case DP_TYPE_NONE:
            default:
                pCap->type = KADP_VFE_DPRX_PORT_TYPE_NONE;
                break;
        }

        switch(pDpCap->lane_cnt)
        {
            case DP_LANE_MODE_1_LANE:
                pCap->max_lane_cnt = KADP_VFE_DPRX_PORT_LANE_CNT_1;
                break;

            case DP_LANE_MODE_2_LANE:
                pCap->max_lane_cnt = KADP_VFE_DPRX_PORT_LANE_CNT_2;
                break;

            case DP_LANE_MODE_4_LANE:
                pCap->max_lane_cnt = KADP_VFE_DPRX_PORT_LANE_CNT_4;
                break;

            default:
                pCap->max_lane_cnt = KADP_VFE_DPRX_PORT_LANE_CNT_NONE;
                break;
        }

        // Link Rate Mask
        if (pDpCap->link_rate_mask & DP_LINK_RATE_RBR_MASK)
            pCap->link_rate_mask |= KADP_VFE_DPRX_PORT_LINK_RATE_RBR_MASK;

        if (pDpCap->link_rate_mask & DP_LINK_RATE_HBR1_MASK)
            pCap->link_rate_mask |= KADP_VFE_DPRX_PORT_LINK_RATE_HBR1_MASK;

        if (pDpCap->link_rate_mask & DP_LINK_RATE_HBR2_MASK)
            pCap->link_rate_mask |= KADP_VFE_DPRX_PORT_LINK_RATE_HBR2_MASK;

        if (pDpCap->link_rate_mask & DP_LINK_RATE_HBR3_MASK)
            pCap->link_rate_mask |= KADP_VFE_DPRX_PORT_LINK_RATE_HBR3_MASK;

        if (pDpCap->link_rate_mask & DP_LINK_RATE_UHBR10_MASK)
            pCap->link_rate_mask |= KADP_VFE_DPRX_PORT_LINK_RATE_UHBR10_MASK;

        if (pDpCap->link_rate_mask & DP_LINK_RATE_UHBR13_MASK)
            pCap->link_rate_mask |= KADP_VFE_DPRX_PORT_LINK_RATE_UHBR13P5_MASK;

        if (pDpCap->link_rate_mask & DP_LINK_RATE_UHBR20_MASK)
            pCap->link_rate_mask |= KADP_VFE_DPRX_PORT_LINK_RATE_UHBR20_MASK;

        // Capability
        if (pDpCap->capability.fec_support)
            pCap->capablity_mask |= KADP_VFE_DPRX_PORT_CAP_FEC_MASK;

        if (pDpCap->capability.dsc_support)
            pCap->capablity_mask |= KADP_VFE_DPRX_PORT_CAP_DSC_1P3;

        ret = DPRX_DRV_NO_ERR;
    }
    return ret;
}

static int copy_dp_link_status_to_kadp(KADP_VFE_DPRX_PORT_LINK_STATUS_T* pLink, DP_LINK_STATUS_T* pDpLink)
{
    int ret = DPRX_DRV_ARG_ERR, i=0;
    if((pLink != NULL) && (pDpLink != NULL))
    {
        memset(pLink, 0, sizeof(KADP_VFE_DPRX_PORT_LINK_STATUS_T));

        // lane mode
        switch(pDpLink->lane_mode)
        {
            case DP_LANE_MODE_1_LANE:
                pLink->lane_mode = KADP_VFE_DPRX_PORT_LANE_CNT_1;
                break;

            case DP_LANE_MODE_2_LANE:
                pLink->lane_mode = KADP_VFE_DPRX_PORT_LANE_CNT_2;
                break;

            case DP_LANE_MODE_4_LANE:
                pLink->lane_mode = KADP_VFE_DPRX_PORT_LANE_CNT_4;
                break;

            default:
                pLink->lane_mode = KADP_VFE_DPRX_PORT_LANE_CNT_NONE;
                break;
        }

        // link rate
        switch(pDpLink->link_rate)
        {
            case DP_LINK_RATE_RBR_1p62G:
                pLink->link_rate = KADP_VFE_DPRX_PORT_LINK_RATE_RBR;
                break;

            case DP_LINK_RATE_HBR1_2p7G:
                pLink->link_rate = KADP_VFE_DPRX_PORT_LINK_RATE_HBR1;
                break;

            case DP_LINK_RATE_HBR2_5p4G:
                pLink->link_rate = KADP_VFE_DPRX_PORT_LINK_RATE_HBR2;
                break;

            case DP_LINK_RATE_HBR3_8p1G:
                pLink->link_rate = KADP_VFE_DPRX_PORT_LINK_RATE_HBR3;
                break;

            case DP_LINK_RATE_UHBR10_10G:
                pLink->link_rate = KADP_VFE_DPRX_PORT_LINK_RATE_UHBR10;
                break;

            case DP_LINK_RATE_UHBR13_13p5G:
                pLink->link_rate = KADP_VFE_DPRX_PORT_LINK_RATE_UHBR13P5;
                break;

            case DP_LINK_RATE_UHBR20_20G:
                pLink->link_rate = KADP_VFE_DPRX_PORT_LINK_RATE_UHBR20;
                break;

            case DP_LINK_RATE_MAX:
            default:
                pLink->link_rate = KADP_VFE_DPRX_PORT_LINK_RATE_RBR;
                break;
        }

        // channel align
        pLink->ch_aligned = pDpLink->channel_align;

        // lane status
        for (i=0; i<4; i++)
        {
            pLink->lane_status[i].valid = pDpLink->channel_status[i].valid;
            pLink->lane_status[i].lock  = pDpLink->channel_status[i].lock;
            pLink->lane_status[i].error_cnt = pDpLink->channel_status[i].error_cnt;
        }

        ret = DPRX_DRV_NO_ERR;
    }
    return ret;
}

static int dprx_vfe_ctrl_handler(unsigned long arg)
{
    int ret = DPRX_DRV_ARG_ERR;
    switch(GET_VFE_DPRX_CTRL_CMD(arg))
    {
        case DPRX_VFE_CTRL_INIT:
            if(arg == VFE_DPRX_CTRL_QS_INIT)
                ret = 0;
            else
                ret = vfe_dprx_drv_init();
            break;

        case DPRX_VFE_CTRL_UNINIT:
            ret = vfe_dprx_drv_uninit();
            break;

        case DPRX_VFE_CTRL_OPEN:
            ret = vfe_dprx_drv_open();
            break;

        case DPRX_VFE_CTRL_CLOSE:
            ret = vfe_dprx_drv_close();
            break;

        case DPRX_VFE_CTRL_CONNECT:
            ret = vfe_dprx_drv_connect(GET_VFE_DPRX_CTRL_PORT(arg));
            break;

        case DPRX_VFE_CTRL_DISCONNECT:
            ret = vfe_dprx_drv_disconnect(GET_VFE_DPRX_CTRL_PORT(arg));
            break;

        default:
            DPRX_VFE_ERR("%s, do vfe_ctrl failed, unknown ctrl command (%lx)\n", __FUNCTION__, arg);
            ret =  -ENOTTY;
            break;
    }
    return ret;
}

static int dprx_vfe_get_attr_handler(VFE_DPRX_ATTR *attr)
{
    int ret = DPRX_DRV_ARG_ERR;
    unsigned char bval;
    if(attr != NULL)
    {
        switch(GET_ATTR_ID(attr->attr))
        {
            case DPRX_VFE_ATTR_RX_PORT_CNT:
                if (attr->data_array==0 && vfe_dprx_drv_get_rx_port_cnt(&bval)==DPRX_DRV_NO_ERR)
                {
                    attr->data = bval;
                    ret = DPRX_DRV_NO_ERR;
                }
                break;

            case DPRX_VFE_ATTR_PORT_CAPABILITY:
                if (attr->data_array==1 && attr->data_len==sizeof(KADP_VFE_DPRX_PORT_CAP_T) && attr->p_data)
                {
                    DP_PORT_CAPABILITY dp_cap;
                    if (vfe_dprx_drv_get_rx_port_capability(attr->port, &dp_cap)==DPRX_DRV_NO_ERR)
                    {
                        KADP_VFE_DPRX_PORT_CAP_T cap;
                        ret = copy_dp_port_capability_to_kadp(&cap,&dp_cap);
                        if(ret ==  DPRX_DRV_NO_ERR)
                        {
                            // copy data
                            if (copy_to_user( (void __user *)to_user_ptr(attr->p_data), (void *)&cap, attr->data_len))
                            {
                                DPRX_VFE_ERR("%s, get port capability failed, copy capability data to user space failed\n", __FUNCTION__);
                                return -EFAULT;
                            }
                        }
                    }
                }
                break;

            case DPRX_VFE_ATTR_PORT_LINK_STATUS:
                if (attr->data_array==1 && attr->data_len==sizeof(KADP_VFE_DPRX_PORT_LINK_STATUS_T) && attr->p_data)
                {
                    DP_LINK_STATUS_T dp_link_status;
                    if (vfe_dprx_drv_get_link_status(attr->port, &dp_link_status)==DPRX_DRV_NO_ERR)
                    {
                        KADP_VFE_DPRX_PORT_LINK_STATUS_T link_status;
                        ret = copy_dp_link_status_to_kadp(&link_status,&dp_link_status);
                        if(ret ==  DPRX_DRV_NO_ERR)
                        {
                            // copy data
                            if (copy_to_user( (void __user *)to_user_ptr(attr->p_data), (void *) &link_status, attr->data_len))
                            {
                                DPRX_VFE_ERR("%s, get port link status failed, copy link status to user space failed\n", __FUNCTION__);
                                return -EFAULT;
                            }
                        }
                    }
                }
                break;

            case DPRX_VFE_ATTR_PORT_STREAM_TYPE:
                if (attr->data_array==0)
                {
                    DP_STREAM_TYPE_E stream_type;
                    if (vfe_dprx_drv_get_stream_type(attr->port, &stream_type)==DPRX_DRV_NO_ERR)
                    {
                        switch(stream_type)
                        {
                            case DPRX_STREAM_TYPE_UNKNOWN:
                                attr->data = KADP_VFE_DPRX_STREAM_TYPE_UNKNOWN;
                                break;

                            case DPRX_STREAM_TYPE_AUDIO_ONLY:
                                attr->data = KADP_VFE_DPRX_STREAM_TYPE_AUDIO_ONLY;
                                break;

                            case DPRX_STREAM_TYPE_VIDEO_ONLY:
                                attr->data = KADP_VFE_DPRX_STREAM_TYPE_VIDEO_ONLY;
                                break;

                            case DPRX_STREAM_TYPE_AUDIO_VIDEO:
                                attr->data = KADP_VFE_DPRX_STREAM_TYPE_AUDIO_VIDEO;
                                break;

                            case DPRX_STREAM_TYPE_MAX:
                            default:
                                attr->data = KADP_VFE_DPRX_STREAM_TYPE_UNKNOWN;
                                break;
                        }
                        ret = DPRX_DRV_NO_ERR;
                    }
                }
                break;

            default:
                DPRX_VFE_ERR("%s, get vfe attr failed, unknown attr(%x)\n", __FUNCTION__, attr->attr);
                return -ENOTTY;
            }
    }
    return ret;
}

/*---------------------------------------------------
 * Func : _ioctl_ops_dprx_vfe
 *
 * Desc : ioctl function of DPRX VFE
 *
 * Para : cmd   :
 *        arg   :
 *
 * Retn : 0 : success, <0 : failed,
 *--------------------------------------------------*/
static long _ioctl_ops_dprx_vfe(unsigned int cmd,  unsigned long arg)
{
    VFE_DPRX_ATTR attr;
    int ret = DPRX_DRV_ARG_ERR;

    DPRX_VFE_DBG("%s (cmd=%08x, arg=%lx)\n", __FUNCTION__, cmd, arg);

    switch(cmd)
    {
    case VFE_IOC_DPRX_VFE_CTRL:
        ret = dprx_vfe_ctrl_handler(arg);
        if(ret==-ENOTTY)
        {
            return ret;
        }
        break;

    case VFE_IOC_DPRX_VFE_SET_ATTR:
        // TODO:
        return -ENOTTY;

    case VFE_IOC_DPRX_VFE_GET_ATTR:

        if (__copy_attr_from_user(&attr, (VFE_DPRX_ATTR *)arg)!=0)
        {
            DPRX_VFE_ERR("%s, get vfe attr failed, copy attr from user space failed\n", __FUNCTION__);
            break;
        }

        if (IS_ATTR_READABLE(attr.attr)==0)
        {
            DPRX_VFE_ERR("%s, get vfe attr failed, attr(%x) is not readable\n", __FUNCTION__, attr.attr);
            break;
        }

        ret = dprx_vfe_get_attr_handler(&attr);
        if((ret == -ENOTTY) ||(ret == -EFAULT) )
        {
             return ret;
        }
        if (attr.data_array==0 && ret == DPRX_DRV_NO_ERR)
        {
            return __copy_attr_to_user((VFE_DPRX_ATTR*)arg, &attr);
        }
        break;

    default:
        DPRX_VFE_ERR("%s, do vfe ioctl failed, unknown command (%x)\n", __FUNCTION__, cmd);
        return -ENOTTY;
    }

    return (ret == DPRX_DRV_NO_ERR) ? 0 : -EFAULT;
}


/*---------------------------------------------------
 * Func : _ioctl_ops_dprx_hpd_det
 *
 * Desc : ioctl function of DPRX VFE
 *
 * Para : cmd   :
 *        arg   :
 *
 * Retn : 0 : success, <0 : failed,
 *--------------------------------------------------*/
static long _ioctl_ops_dprx_hpd_det(unsigned int cmd,  unsigned long arg)
{
    VFE_DPRX_ATTR attr;
    int ret = DPRX_DRV_ARG_ERR;
    unsigned char bval;

    DPRX_VFE_DBG("%s (cmd=%08x, arg=%lx)\n", __FUNCTION__, cmd, arg);

    switch(cmd)
    {
    case VFE_IOC_DPRX_HPD_DET_CTRL:

        switch(GET_VFE_DPRX_CTRL_CMD(arg))
        {
        case DPRX_HPD_CTRL_ENABLE_HPD:  ret = vfe_dprx_drv_enable_hpd(GET_VFE_DPRX_CTRL_VALUE(arg) ? 1 : 0); break;
        case DPRX_HPD_CTRL_SET_HPD:     ret = vfe_dprx_drv_set_hpd(GET_VFE_DPRX_CTRL_PORT(arg), GET_VFE_DPRX_CTRL_VALUE(arg) ? 1 : 0); break;
        case DPRX_HPD_CTRL_HPD_TOGGLE:  ret = vfe_dprx_drv_hpd_toggle(GET_VFE_DPRX_CTRL_PORT(arg)); break;
        default:
            DPRX_VFE_ERR("%s, do hpd_det_ctrl failed, unknown ctrl command (%lx)\n", __FUNCTION__, arg);
            return -ENOTTY;
        }
        break;

    case VFE_IOC_DPRX_HPD_DET_SET_ATTR:

        if (__copy_attr_from_user(&attr, (VFE_DPRX_ATTR *)arg)!=0)
        {
            DPRX_VFE_ERR("%s, set hpd_det_attr failed, copy attr from user space failed\n", __FUNCTION__);
            break;
        }

        if (IS_ATTR_WRITABLE(attr.attr)==0)
        {
            DPRX_VFE_ERR("%s, set hpd_det_attr failed, attr(%x) is not writable\n", __FUNCTION__, attr.attr);
            break;
        }

        // set attribute
        switch(GET_ATTR_ID(attr.attr))
        {
        case DPRX_HPD_ATTR_HPD_LOW_DELAY:

            if (attr.data_array==0 && vfe_dprx_drv_set_hpd_low_delay(attr.data)==DPRX_DRV_NO_ERR)
                ret = DPRX_DRV_NO_ERR;
            break;

        default:
            DPRX_VFE_ERR("%s, set vfe attr failed, unknown attr(%x)\n", __FUNCTION__, attr.attr);
            return -ENOTTY;
        }
        break;

    case VFE_IOC_DPRX_HPD_DET_GET_ATTR:

        if (__copy_attr_from_user(&attr, (VFE_DPRX_ATTR *)arg)!=0)
        {
            DPRX_VFE_ERR("%s, get hpd_det_attr failed, copy attr from user space failed\n", __FUNCTION__);
            break;
        }

        if (IS_ATTR_READABLE(attr.attr)==0)
        {
            DPRX_VFE_ERR("%s, get hpd_det_attr failed, attr(%x) is not readable\n", __FUNCTION__, attr.attr);
            break;
        }

        // get attribute
        switch(GET_ATTR_ID(attr.attr))
        {
        case DPRX_HPD_ATTR_HPD_LOW_DELAY:

            if (attr.data_array==0)
            {
                attr.data = 2;   // humm... output hpd low delay here
                ret = DPRX_DRV_NO_ERR;
            }
            break;

        case DPRX_HPD_ATTR_DPRX_CONNECT_STATE:

            if (attr.data_array==0 && vfe_dprx_drv_get_connection_state(attr.port, &bval)==DPRX_DRV_NO_ERR)
            {
                attr.data = bval;
                ret = DPRX_DRV_NO_ERR;
            }
            break;

        default:
            DPRX_VFE_ERR("%s, get hpd_det_attr failed, unknown attr(%x)\n", __FUNCTION__, attr.attr);
            return -ENOTTY;
        }

        if (attr.data_array==0 && ret == DPRX_DRV_NO_ERR)
            return __copy_attr_to_user((VFE_DPRX_ATTR*)arg, &attr);

        break;

    default:
        DPRX_VFE_ERR("%s, do ioctl failed, unknown command (%x)\n", __FUNCTION__, cmd);
    }

    return (ret == DPRX_DRV_NO_ERR) ? 0 : -EFAULT;
}

/*---------------------------------------------------
 * Func : _ioctl_ops_dprx_edid
 *
 * Desc : ioctl function of DPRX VFE
 *
 * Para : cmd   :
 *        arg   :
 *
 * Retn : 0 : success, <0 : failed,
 *--------------------------------------------------*/
static long _ioctl_ops_dprx_edid(unsigned int cmd,  unsigned long arg)
{
    VFE_DPRX_ATTR attr;
    int ret = DPRX_DRV_ARG_ERR;
    unsigned char* p_edid;

    DPRX_VFE_DBG("%s (cmd=%08x, arg=%lx)\n", __FUNCTION__, cmd, arg);

    switch(cmd)
    {
    case VFE_IOC_DPRX_EDID_CTRL:
        // TODO:
        DPRX_VFE_ERR("%s, do edid_ioctl failed, unknown command (%lx)\n", __FUNCTION__, arg);
        return -ENOTTY;

    case VFE_IOC_DPRX_EDID_SET_ATTR:

        if (__copy_attr_from_user(&attr, (VFE_DPRX_ATTR *)arg)!=0)
        {
            DPRX_VFE_ERR("%s, set edid_attr failed, copy attr from user space failed\n", __FUNCTION__);
            break;
        }

        if (IS_ATTR_WRITABLE(attr.attr)==0)
        {
            DPRX_VFE_ERR("%s, set edid_attr failed, attr(%x) is not writable\n", __FUNCTION__, attr.attr);
            break;
        }

        // set attribute
        switch(GET_ATTR_ID(attr.attr))
        {
        case DPRX_EDID_ATTR_DPRX_EDID:

            if (IS_VALID_DPRX_EDID_ATTR_DATA(attr))
            {
                p_edid = kmalloc(attr.data_len, GFP_KERNEL);

                if (p_edid==NULL)
                {
                    DPRX_VFE_ERR("%s, set dprx edid failed, alloc edid mem failed\n", __FUNCTION__);
                    return -EFAULT;
                }

                if (copy_from_user((void *)p_edid, (const void __user *)to_user_ptr(attr.p_data), attr.data_len))
                {
                    DPRX_VFE_ERR("%s, set dprx edid failed, copy edid from user space failed\n", __FUNCTION__);
                    return -EFAULT;
                }

                ret = vfe_dprx_drv_write_edid(attr.port, p_edid, attr.data_len);
                kfree(p_edid);
            }
            break;

        default:
            DPRX_VFE_ERR("%s, set edid_attr failed, unknown attr(%x)\n", __FUNCTION__, attr.attr);
            return -ENOTTY;
        }

        break;

    case VFE_IOC_DPRX_EDID_GET_ATTR:

        if (__copy_attr_from_user(&attr, (VFE_DPRX_ATTR *)arg)!=0)
        {
            DPRX_VFE_ERR("%s, get edid_attr failed, copy attr from user space failed\n", __FUNCTION__);
            break;
        }

        if (IS_ATTR_READABLE(attr.attr)==0)
        {
            DPRX_VFE_ERR("%s, get edid_attr failed, attr(%x) is not readable\n", __FUNCTION__, attr.attr);
            break;
        }

        // get attribute
        switch(GET_ATTR_ID(attr.attr))
        {
        case DPRX_EDID_ATTR_DPRX_EDID:

            if (IS_VALID_DPRX_EDID_ATTR_DATA(attr))
            {
                p_edid = kmalloc(attr.data_len, GFP_KERNEL);

                if (p_edid==NULL)
                {
                    DPRX_VFE_ERR("%s, get dprx edid failed, alloc edid mem failed\n", __FUNCTION__);
                    return -EFAULT;
                }

                ret = vfe_dprx_drv_read_edid(attr.port, p_edid, attr.data_len);

                if (ret==DPRX_DRV_NO_ERR && copy_to_user(to_user_ptr(attr.p_data), p_edid, attr.data_len)!=0)
                {
                    DPRX_VFE_ERR("%s, get dprx edid failed, copy edid to user space failed\n", __FUNCTION__);
                    ret = DPRX_DRV_HW_ERR;
                }

                kfree(p_edid);
            }
            break;

        default:
            DPRX_VFE_ERR("%s, get edid_attr failed, unknown attr(%x)\n", __FUNCTION__, attr.attr);
            return -ENOTTY;
        }

        if (attr.data_array==0 && ret == DPRX_DRV_NO_ERR)
            return __copy_attr_to_user((VFE_DPRX_ATTR*)arg, &attr);
        break;

    default:
        DPRX_VFE_ERR("%s, do ioctl failed, unknown command (%x)\n", __FUNCTION__, cmd);
        return -ENOTTY;
    }

    return (ret == DPRX_DRV_NO_ERR) ? 0 : -EFAULT;
}



/*---------------------------------------------------
 * Func : _ioctl_ops_dprx_hdcp
 *
 * Desc : ioctl ops of DPRX HDCP
 *
 * Para : cmd   :
 *        arg   :
 *
 * Retn : 0 : success, <0 : failed,
 *--------------------------------------------------*/
static long _ioctl_ops_dprx_hdcp(unsigned int cmd,  unsigned long arg)
{
    VFE_DPRX_ATTR attr;
    int ret = DPRX_DRV_ARG_ERR;
    DPRX_HDCP_KEY_T* p_hdcp_key = NULL;

    DPRX_VFE_DBG("%s (cmd=%08x, arg=%lx)\n", __FUNCTION__, cmd, arg);

    switch(cmd)
    {
    case VFE_IOC_DPRX_HDCP_CTRL:

        switch(GET_VFE_DPRX_CTRL_CMD(arg))
        {
        case DPRX_HDCP_CTRL_FORCE_REAUTH:
            // FIXME : trigger hdcp reauth here
            DPRX_VFE_WARN("%s, force HDCP reauth is not implemented yet, do nothing\n", __FUNCTION__);
            return 0;

        default:
            DPRX_VFE_ERR("%s, do hdcp_ctrl failed, unknown command (%lx)\n", __FUNCTION__, arg);
        }

        return -ENOTTY;

    case VFE_IOC_DPRX_HDCP_SET_ATTR:

        if (__copy_attr_from_user(&attr, (VFE_DPRX_ATTR *)arg)!=0)
        {
            DPRX_VFE_ERR("%s, set hdcp_attr failed, copy attr from user space failed\n", __FUNCTION__);
            break;
        }

        if (IS_ATTR_WRITABLE(attr.attr)==0)
        {
            DPRX_VFE_ERR("%s, set hdcp_attr failed, attr(%x) is not writable\n", __FUNCTION__, attr.attr);
            break;
        }

        // set attribute
        switch(GET_ATTR_ID(attr.attr))
        {
        case DPRX_HDCP_ATTR_HDCP2_ENABLE:

            if (attr.data_array)
            {
                DPRX_VFE_ERR("%s, set HDCP2 ENABLE attr failed, attr data sould not be an array\n", __FUNCTION__);
                return -EFAULT;
            }

            // DOTO: enable hdcp2 over here
            DPRX_VFE_WARN("%s, HDCP2 ENABLE is not implemented yet, do nothing....\n", __FUNCTION__);
            return 0;

        case DPRX_HDCP_ATTR_HDCP1x_KEY:

            if (attr.data_array==1 && attr.data_len==HDCP1X_KEY_SIZE && attr.p_data)
            {
                p_hdcp_key = kmalloc(sizeof(DPRX_HDCP_KEY_T), GFP_KERNEL);

                if (p_hdcp_key==NULL)
                {
                    DPRX_VFE_ERR("%s, set HDCP1x key failed, alloc mem failed\n", __FUNCTION__);
                    return -EFAULT;
                }

                p_hdcp_key->version = DP_HDCP_VER_HDCP13;

                if (copy_from_user((void *)p_hdcp_key->hdcp1x_key, (const void __user *)to_user_ptr(attr.p_data), attr.data_len))
                {
                    DPRX_VFE_ERR("%s, set HDCP1x key failed, copy key data from user space failed\n", __FUNCTION__);
                    kfree(p_hdcp_key);
                    return -EFAULT;
                }

                ret = vfe_dprx_drv_write_hdcp_key(p_hdcp_key, 1);
                kfree(p_hdcp_key);
            }
            else
            {
                DPRX_VFE_ERR("%s, set HDCP1x key attr failed, incorrect attr data\n", __FUNCTION__);
            }
            break;

        case DPRX_HDCP_ATTR_HDCP2x_KEY:

            if (attr.data_array==1 && attr.data_len==HDCP2X_KEY_SIZE && attr.p_data)
            {
                p_hdcp_key = kmalloc(sizeof(DPRX_HDCP_KEY_T), GFP_KERNEL);

                if (p_hdcp_key==NULL)
                {
                    DPRX_VFE_ERR("%s, set HDCP2x key failed, alloc mem failed\n", __FUNCTION__);
                    return -EFAULT;
                }

                p_hdcp_key->version = DP_HDCP_VER_HDCP22;

                if (copy_from_user((void *)p_hdcp_key->hdcp2x_key, (const void __user *)to_user_ptr(attr.p_data), attr.data_len))
                {
                    DPRX_VFE_ERR("%s, set HDCP2x key failed, copy key data from user space failed\n", __FUNCTION__);
                    kfree(p_hdcp_key);
                    return -EFAULT;
                }

                ret = vfe_dprx_drv_write_hdcp_key(p_hdcp_key, 1);
                kfree(p_hdcp_key);
            }
            else
            {
                DPRX_VFE_ERR("%s, set HDCP2x key failed, incorrect attr data\n", __FUNCTION__);
            }
            break;

        default:
            DPRX_VFE_ERR("%s, set hdcp_attr failed, unknown attr(%x)\n", __FUNCTION__, attr.attr);
            return -EFAULT;
        }

        break;

    case VFE_IOC_DPRX_HDCP_GET_ATTR:

        if (__copy_attr_from_user(&attr, (VFE_DPRX_ATTR *)arg)!=0)
        {
            DPRX_VFE_ERR("%s, get hdcp_attr failed, copy attr from user space failed\n", __FUNCTION__);
            break;
        }

        if (IS_ATTR_READABLE(attr.attr)==0)
        {
            DPRX_VFE_ERR("%s, get hdcp_attr failed, attr(%x) is not readable\n", __FUNCTION__, attr.attr);
            break;
        }

        // do command
        switch(GET_ATTR_ID(attr.attr))
        {
        case DPRX_HDCP_ATTR_CAPABILITY:

            if (attr.data_array==0)
            {
                unsigned int get_hdcp_cap=0;
                unsigned int ret_hdcp_cap=0;

                if (vfe_dprx_drv_get_hdcp_capability(&get_hdcp_cap)==DPRX_DRV_NO_ERR)
                {
                    if(get_hdcp_cap & DP_HDCP_CAPBILITY_HDCP13)
                       ret_hdcp_cap |= VFE_DP_HDCP_CAPBILITY_HDCP13;

                    if(get_hdcp_cap & DP_HDCP_CAPBILITY_HDCP22)
                       ret_hdcp_cap |= VFE_DP_HDCP_CAPBILITY_HDCP22;

                    if(get_hdcp_cap & DP_HDCP_CAPBILITY_HDCP23)
                       ret_hdcp_cap |= VFE_DP_HDCP_CAPBILITY_HDCP23;

                    if(get_hdcp_cap & DP_HDCP_CAPBILITY_HDCP_REPEATER)
                       ret_hdcp_cap |= VFE_DP_HDCP_CAPBILITY_HDCP_REPEATER;

                    attr.data = ret_hdcp_cap;
                    ret = DPRX_DRV_NO_ERR;
                }
            }
            break;

        case DPRX_HDCP_ATTR_DPRX_HDCP_STATUS:

            if (attr.data_array==1 && attr.data_len==sizeof(VFE_DPRX_HDCP_STATUS_T) && attr.p_data)
            {
                DPRX_HDCP_STATUS_T get_hdcp_status;
                VFE_DPRX_HDCP_STATUS_T ret_hdcp_status;
                if (vfe_dprx_drv_get_hdcp_status(attr.port, &get_hdcp_status)==DPRX_DRV_NO_ERR)
                {
                    memset(&ret_hdcp_status, 0, sizeof(VFE_DPRX_HDCP_STATUS_T));
                    ret_hdcp_status.mode = (VFE_DP_HDCP_MODE)get_hdcp_status.mode;
                    ret_hdcp_status.state = (VFE_DP_HDCP_STATE)get_hdcp_status.state;
                    memcpy(&(ret_hdcp_status.hdcp2x.receiverid),&(get_hdcp_status.hdcp2x.receiverid),VFE_HDCP2X_RECEIVER_ID_LEN);
                    if (copy_to_user( (void __user *)to_user_ptr(attr.p_data), (void *) &ret_hdcp_status, attr.data_len))
                    {
                        DPRX_VFE_ERR("%s, get hdcp status failed, copy hdcp status to user space failed\n", __FUNCTION__);
                        return -EFAULT;
                    }
                    ret = DPRX_DRV_NO_ERR;
                }
            }
            break;

        case DPRX_HDCP_ATTR_HDCP2_ENABLE:

            if (attr.data_array==0)
            {
                attr.data = 1;
                ret = DPRX_DRV_NO_ERR;
            }
            else
            {
                DPRX_VFE_ERR("%s, get hdcp2 enable attr failed, invalid attr data\n", __FUNCTION__);
            }
            break;

        default:
            DPRX_VFE_ERR("%s, get hdcp_attr failed, unknown attr(%x)\n", __FUNCTION__, attr.attr);
            return -ENOTTY;
        }

        if (attr.data_array==0 && ret == DPRX_DRV_NO_ERR)
            return __copy_attr_to_user((VFE_DPRX_ATTR*)arg, &attr);

        break;

    default:
        DPRX_VFE_ERR("%s, do ioctl failed, unknown command (%x)\n", __FUNCTION__, cmd);
        return -ENOTTY;
    }

    return (ret==DPRX_DRV_NO_ERR) ? 0 : -EFAULT;
}


/*---------------------------------------------------
 * Func : _ioctl_ops_dprx_video
 *
 * Desc : ioctl function of DPRX VFE
 *
 * Para : cmd   :
 *        arg   :
 *
 * Retn : 0 : success, <0 : failed,
 *--------------------------------------------------*/
static long _ioctl_ops_dprx_video(unsigned int cmd,  unsigned long arg)
{
    VFE_DPRX_ATTR attr;
    int ret = DPRX_DRV_ARG_ERR;
    DPRX_VFE_DBG("%s (cmd=%08x, arg=%lx)\n", __FUNCTION__, cmd, arg);

    switch(cmd)
    {
    case VFE_IOC_DPRX_VIDEO_CTRL:
        DPRX_VFE_WARN("%s, do video ctrl failed, ctrl (%lx) is not implemented yet\n", __FUNCTION__, arg);
        break;

    case VFE_IOC_DPRX_VIDEO_GET_ATTR:
        if (__copy_attr_from_user(&attr, (VFE_DPRX_ATTR *)arg)!=0)
        {
            DPRX_VFE_ERR("%s, get video_attr failed, copy attr from user space failed\n", __FUNCTION__);
            break;
        }

        if (IS_ATTR_READABLE(attr.attr)==0)
        {
            DPRX_VFE_ERR("%s, get video_attr failed, attr(%x) is not readable\n", __FUNCTION__, attr.attr);
            break;
        }

        switch(GET_ATTR_ID(attr.attr))
        {
        case DPRX_VIDEO_ATTR_VIDEO_TIMING:
            if (attr.data_array==1 && attr.data_len==sizeof(KADP_VFE_DPRX_TIMING_INFO_T) && attr.p_data)
            {
                DPRX_TIMING_INFO_T get_timing_info;
                KADP_VFE_DPRX_TIMING_INFO_T ret_timing_info;

                if (vfe_dprx_drv_get_port_timing_info(attr.port, &get_timing_info)==DPRX_DRV_NO_ERR)
                {
                    memset(&ret_timing_info, 0, sizeof(KADP_VFE_DPRX_TIMING_INFO_T));
                    ret_timing_info.color_space = (KADP_VFE_DPRX_COLOR_SPACE_E)get_timing_info.color_space;
                    ret_timing_info.color_depth = (KADP_VFE_DPRX_COLOR_DEPTH_E)get_timing_info.color_depth;
                    ret_timing_info.ptg_mode = (KADP_VFE_DPRX_PTG_REGEN_E)get_timing_info.ptg_mode;
                    ret_timing_info.htotal = get_timing_info.htotal;
                    ret_timing_info.hporch = get_timing_info.hporch;
                    ret_timing_info.hstart = get_timing_info.hstart;
                    ret_timing_info.hsync = get_timing_info.hsync;
                    ret_timing_info.hact = get_timing_info.hact;
                    ret_timing_info.vtotal = get_timing_info.vtotal;
                    ret_timing_info.vporch = get_timing_info.vporch;
                    ret_timing_info.vstart = get_timing_info.vstart;
                    ret_timing_info.vsync = get_timing_info.vsync;
                    ret_timing_info.vact = get_timing_info.vact;
                    ret_timing_info.hfreq_hz = get_timing_info.hfreq_hz;
                    ret_timing_info.vfreq_hz_x100 = get_timing_info.vfreq_hz_x100;
                    ret_timing_info.is_interlace = get_timing_info.is_interlace;
                    ret_timing_info.is_dsc = get_timing_info.is_dsc;
                    ret_timing_info.drr_mode = get_timing_info.drr_mode;
                    ret_timing_info.curr_vfreq_hz_x100 = get_timing_info.curr_vfreq_hz_x100;
                    ret_timing_info.isALLM= get_timing_info.isALLM;
                    if (copy_to_user( (void __user *)to_user_ptr(attr.p_data), (void *) &ret_timing_info, attr.data_len))
                    {
                        DPRX_VFE_ERR("%s, get timing info failed, copy timing info to user space failed\n", __FUNCTION__);
                        return -EFAULT;
                    }
                    ret = DPRX_DRV_NO_ERR;
                }
            }
            break;

        case DPRX_VIDEO_ATTR_AVI_INFO:

            if(attr.data_array==1 && attr.data_len==sizeof(KADP_VFE_DPRX_AVI_INFO_T) && attr.p_data)
            {
                DPRX_AVI_INFO_T get_avi_info;
                KADP_VFE_DPRX_AVI_INFO_T ret_avi_info;

                if (vfe_dprx_drv_get_port_avi_info(attr.port, &get_avi_info)==DPRX_DRV_NO_ERR)
                {
                    memset(&ret_avi_info, 0, sizeof(KADP_VFE_DPRX_AVI_INFO_T));
                    ret_avi_info.ePixelEncoding = (KADP_HAL_VFE_DPRX_AVI_CSC_T)get_avi_info.ePixelEncoding;
                    ret_avi_info.eActiveInfo = (KADP_HAL_VFE_DPRX_AVI_ACTIVE_INFO_T)get_avi_info.eActiveInfo;
                    ret_avi_info.eBarInfo = (KADP_HAL_VFE_DPRX_AVI_BAR_INFO_T)get_avi_info.eBarInfo;
                    ret_avi_info.eScanInfo = (KADP_HAL_VFE_DPRX_AVI_SCAN_INFO_T)get_avi_info.eScanInfo;
                    ret_avi_info.eColorimetry = (KADP_HAL_VFE_DPRX_AVI_COLORIMETRY_T)get_avi_info.eColorimetry;
                    ret_avi_info.ePictureAspectRatio = (KADP_HAL_VFE_DPRX_AVI_PICTURE_ARC_T)get_avi_info.ePictureAspectRatio;
                    ret_avi_info.eActiveFormatAspectRatio = (KADP_HAL_VFE_DPRX_AVI_ACTIVE_FORMAT_ARC_T)get_avi_info.eActiveFormatAspectRatio;
                    ret_avi_info.eScaling = (KADP_HAL_VFE_DPRX_AVI_SCALING_T)get_avi_info.eScaling;
                    ret_avi_info.VideoIdCode = get_avi_info.VideoIdCode;
                    ret_avi_info.PixelRepeat = get_avi_info.PixelRepeat;
                    ret_avi_info.eITContent = (KADP_HAL_VFE_DPRX_AVI_IT_CONTENT_T)get_avi_info.eITContent;
                    ret_avi_info.eExtendedColorimetry = (KADP_HAL_VFE_DPRX_AVI_EXT_COLORIMETRY_T)get_avi_info.eExtendedColorimetry;
                    ret_avi_info.eRGBQuantizationRange = (KADP_HAL_VFE_DPRX_AVI_RGB_QUANTIZATION_RANGE_T)get_avi_info.eRGBQuantizationRange;
                    ret_avi_info.eYCCQuantizationRange = (KADP_HAL_VFE_DPRX_AVI_YCC_QUANTIZATION_RANGE_T)get_avi_info.eYCCQuantizationRange;
                    ret_avi_info.eContentType = (KADP_HAL_VFE_DPRX_AVI_CONTENT_TYPE_T)get_avi_info.eContentType;
                    ret_avi_info.TopBarEndLineNumber = get_avi_info.TopBarEndLineNumber;
                    ret_avi_info.BottomBarStartLineNumber = get_avi_info.BottomBarStartLineNumber;
                    ret_avi_info.LeftBarEndPixelNumber = get_avi_info.LeftBarEndPixelNumber;
                    ret_avi_info.RightBarEndPixelNumber = get_avi_info.RightBarEndPixelNumber;
                    ret_avi_info.eAdditionalColorimetry = (KADP_HAL_VFE_DPRX_AVI_ADDITIONAL_COLORIMETRY_T)get_avi_info.eAdditionalColorimetry;
                    ret_avi_info.f14 = get_avi_info.f14;
                    ret_avi_info.packetStatus = (KADP_HAL_VFE_DPRX_PACKET_STATUS_T)get_avi_info.packetStatus;
                    ret_avi_info.packet.type = get_avi_info.packet.type;
                    ret_avi_info.packet.version = get_avi_info.packet.version;
                    ret_avi_info.packet.length = get_avi_info.packet.length;
                    if (copy_to_user( (void __user *)to_user_ptr(attr.p_data), (void *) &ret_avi_info, attr.data_len))
                    {
                        DPRX_VFE_ERR("%s, get avi info failed, copy avi info to user space failed\n", __FUNCTION__);
                        return -EFAULT;
                    }
                    ret = DPRX_DRV_NO_ERR;
                }
            }
            break;

        case DPRX_VIDEO_ATTR_DRM_INFO:

            if (attr.data_array==1 && attr.data_len==sizeof(KADP_VFE_DPRX_DRM_INFO_T) && attr.p_data)
            {
                DPRX_DRM_INFO_T get_drm_info;
                KADP_VFE_DPRX_DRM_INFO_T ret_drm_info;

                if (vfe_dprx_drv_get_port_drm_info(attr.port, &get_drm_info)==DPRX_DRV_NO_ERR)
                {
                    memset(&ret_drm_info, 0, sizeof(KADP_VFE_DPRX_DRM_INFO_T));
                    ret_drm_info.nVersion = get_drm_info.nVersion;
                    ret_drm_info.nLength = get_drm_info.nLength;
                    ret_drm_info.eEOTFtype = (KADP_HAL_DPRX_DRM_EOTF_T)get_drm_info.eEOTFtype;
                    ret_drm_info.eMeta_Desc = (KADP_HAL_DPRX_DRM_META_DESC_T)get_drm_info.eMeta_Desc;
                    ret_drm_info.display_primaries_x0 = get_drm_info.display_primaries_x0;
                    ret_drm_info.display_primaries_y0 = get_drm_info.display_primaries_y0;
                    ret_drm_info.display_primaries_x1 = get_drm_info.display_primaries_x1;
                    ret_drm_info.display_primaries_y1 = get_drm_info.display_primaries_y1;
                    ret_drm_info.display_primaries_x2 = get_drm_info.display_primaries_x2;
                    ret_drm_info.display_primaries_y2 = get_drm_info.display_primaries_y2;
                    ret_drm_info.white_point_x = get_drm_info.white_point_x;
                    ret_drm_info.white_point_y = get_drm_info.white_point_y;
                    ret_drm_info.max_display_mastering_luminance = get_drm_info.max_display_mastering_luminance;
                    ret_drm_info.min_display_mastering_luminance = get_drm_info.min_display_mastering_luminance;
                    ret_drm_info.maximum_content_light_level = get_drm_info.maximum_content_light_level;
                    ret_drm_info.maximum_frame_average_light_level = get_drm_info.maximum_frame_average_light_level;
                    if (copy_to_user( (void __user *)to_user_ptr(attr.p_data), (void *) &ret_drm_info, attr.data_len))
                    {
                        DPRX_VFE_ERR("%s, get drm info failed, copy drm info to user space failed\n", __FUNCTION__);
                        return -EFAULT;
                    }
                    ret = DPRX_DRV_NO_ERR;
                }
            }
            break;

        default:
            DPRX_VFE_ERR("%s, get video_attr failed, unknown attr(%x)\n", __FUNCTION__, attr.attr);
            return -ENOTTY;
        }

        if (attr.data_array==1 && ret == DPRX_DRV_NO_ERR)
            return __copy_attr_to_user((VFE_DPRX_ATTR*)arg, &attr);

        break;

    case VFE_IOC_DPRX_VIDEO_SET_ATTR:
        DPRX_VFE_WARN("%s, set video attr failed, get attr is not implemented yet\n", __FUNCTION__);
        break;

    default:
        DPRX_VFE_ERR("%s, do ioctl failed, unknown command (%x)\n", __FUNCTION__, cmd);
        return -ENOTTY;
    }

    return (ret==DPRX_DRV_NO_ERR) ? 0 : -EFAULT;
}



/*---------------------------------------------------
 * Func : _ioctl_ops_dprx_audio
 *
 * Desc : ioctl operation of DPRX AUDIO
 *
 * Para : cmd   :
 *        arg   :
 *
 * Retn : 0 : success, <0 : failed,
 *--------------------------------------------------*/
static long _ioctl_ops_dprx_audio(unsigned int cmd,  unsigned long arg)
{
    VFE_DPRX_ATTR attr;
    int ret = DPRX_DRV_ARG_ERR;
    DPRX_VFE_DBG("%s (cmd=%08x, arg=%lx)\n", __FUNCTION__, cmd, arg);

    switch(cmd)
    {
    case VFE_IOC_DPRX_AUDIO_CTRL:
        DPRX_VFE_WARN("%s, do audio ctrl failed, ctrl (%lx) is not implemented yet\n", __FUNCTION__, arg);
        break;

    case VFE_IOC_DPRX_AUDIO_GET_ATTR:

        if (__copy_attr_from_user(&attr, (VFE_DPRX_ATTR *)arg)!=0)
        {
            DPRX_VFE_ERR("%s, get audio_attr failed, copy attr from user space failed\n", __FUNCTION__);
            break;
        }

        if (IS_ATTR_READABLE(attr.attr)==0)
        {
            DPRX_VFE_ERR("%s, get audio_attr failed, attr(%x) is not readable\n", __FUNCTION__, attr.attr);
            break;
        }

        switch(GET_ATTR_ID(attr.attr))
        {
        case DPRX_AUDIO_ATTR_AUDIO_STATUS:

            if (attr.data_array==1 && attr.data_len==sizeof(KADP_VFE_DPRX_AUDIO_STATUS_T) && attr.p_data)
            {
                DP_AUDIO_STATUS_T get_audio_status;
                KADP_VFE_DPRX_AUDIO_STATUS_T ret_audio_status;

                if (vfe_dprx_drv_get_audio_status(&get_audio_status)==DPRX_DRV_NO_ERR)
                {
                    memset(&ret_audio_status, 0, sizeof(KADP_VFE_DPRX_AUDIO_STATUS_T));
                    ret_audio_status.coding_type = (KADP_VFE_DPRX_AUDIO_CODING_TYPE)get_audio_status.coding_type;
                    ret_audio_status.ch_num = get_audio_status.ch_num;
                    ret_audio_status.sampling_frequency_khz_x100 = get_audio_status.sampling_frequency_khz_x100;
                    if (copy_to_user( (void __user *)to_user_ptr(attr.p_data), (void *) &ret_audio_status, attr.data_len))
                    {
                        DPRX_VFE_ERR("%s, get audio status failed, copy audio status to user space failed\n", __FUNCTION__);
                        return -EFAULT;
                    }
                    ret = DPRX_DRV_NO_ERR;
                }
            }
            break;

        default:
            DPRX_VFE_ERR("%s, get audio_attr failed, unknown attr(%x)\n", __FUNCTION__, attr.attr);
            return -ENOTTY;
        }

        if (attr.data_array==0 && ret == DPRX_DRV_NO_ERR)
            return __copy_attr_to_user((VFE_DPRX_ATTR*)arg, &attr);

        break;

    case VFE_IOC_DPRX_AUDIO_SET_ATTR:
        DPRX_VFE_WARN("%s, set audio attr failed, set attr is not implemented yet\n", __FUNCTION__);
        break;

    default:
        DPRX_VFE_ERR("%s, do ioctl failed, unknown command (%x)\n", __FUNCTION__, cmd);
        return -ENOTTY;
    }

    return (ret==DPRX_DRV_NO_ERR) ? 0 : -EFAULT;
}



/*---------------------------------------------------
 * Func : vfe_dprx_ioctl
 *
 * Desc : ioctl function of DPRX VFE
 *
 * Para : file  :
 *        cmd   :
 *        arg   :
 *
 * Retn : 0 : success, <0 : failed,
 *--------------------------------------------------*/
long vfe_dprx_ioctl(struct file *file, unsigned int cmd,  unsigned long arg)
{
    long ret = -ENOTTY;

    DPRX_VFE_DBG("%s (cmd=%08x, arg=%lx)\n", __FUNCTION__, cmd, arg);

    if (!IS_VFE_DPRX_IOC(cmd))
    {
        DPRX_VFE_ERR("%s, do ioctl failed, cmd = %x is not belongs to DPRX VFE\n", __FUNCTION__, cmd);
        return -EFAULT;
    }

    switch(GET_DPRX_VFE_IOC_IDX(cmd))
    {
    case VFE_IOC_DPRX_VFE_CTRL_IDX:     ret = _ioctl_ops_dprx_vfe(cmd, arg);     break;
    case VFE_IOC_DPRX_HPD_DET_CTRL_IDX: ret = _ioctl_ops_dprx_hpd_det(cmd, arg); break;
    case VFE_IOC_DPRX_EDID_CTRL_IDX:    ret = _ioctl_ops_dprx_edid(cmd, arg);    break;
    case VFE_IOC_DPRX_HDCP_CTRL_IDX:    ret = _ioctl_ops_dprx_hdcp(cmd, arg);    break;
    case VFE_IOC_DPRX_VIDEO_CTRL_IDX:   ret = _ioctl_ops_dprx_video(cmd, arg);   break;
    case VFE_IOC_DPRX_AUDIO_CTRL_IDX:   ret = _ioctl_ops_dprx_audio(cmd, arg);   break;
    default:
        DPRX_VFE_ERR("%s, do ioctl failed, unkonwn command (%x)\n", __FUNCTION__, cmd);
        return -ENOTTY;
    }

    return ret;
}
