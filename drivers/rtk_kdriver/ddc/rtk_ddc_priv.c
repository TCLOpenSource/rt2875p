#include "rtk_ddc_config.h"
#include "rtk_ddc_dbg.h"
#include "rtk_ddc_priv.h"

#define GET_DDC_FSMS_STATUS(ddc)                           rtd_inl(ddc->DDC_DDC_FSMS)

#ifndef BUILD_QUICK_SHOW
#define DDC_LOCK(a,b)   spin_lock_irqsave(a,b)
#define DDC_UNLOCK(a,b) spin_unlock_irqrestore(a,b)
#else
#define DDC_LOCK(a,b)   spin_lock(a)
#define DDC_UNLOCK(a,b) spin_unlock(a)
#endif

const char* _ddc_bus_status_str(int fsm)
{
    switch(fsm) {
    case DDC_FSM_STATUS_IDEL:       return "IDEL";
    case DDC_FSM_STATUS_ID_DETECT:  return "ID_DETECT";
    case DDC_FSM_STATUS_WR_BIT_7_0: return "WR_BIT";
    case DDC_FSM_STATUS_WR_ACK:     return "WR_ACK";
    case DDC_FSM_STATUS_RD_BIT_7_0: return "RD_BIT";
    case DDC_FSM_STATUS_RD_ACK:     return "RD_ACK";
    default:                        return "Unknow";
    }
}

static int rtk_ddc_drv_edid_content_is_valid(unsigned char id, int content_len, const unsigned char* content)
{
    if(NULL==content)
    {
        RTK_DDC_ERR("check edid content failed, content should not be NULL\n");
        return 0;
    }

    if ((128 != content_len) && (256 != content_len) && (384 != content_len) && (512 != content_len))
    {
        RTK_DDC_ERR("check edid content failed, content len should be ether 128 ,256 ,384 or 512\n");
        return 0;
    }
    if(rtk_ddc_chip[id].reg_remap == NULL || id >= ARRAY_SIZE(rtk_ddc_chip) ){
        RTK_DDC_ERR("Check index failed. remap is null\n");
        return 0;
    }
    return 1;
}

int rtk_ddc_drv_set_scpu_init(int enable)
{

    rtd_maskl(INT_CTRL_REG, ~(INT_CTRL_DDC1_INT_TO_SCPU_EN_MASK), INT_CTRL_DDC1_INT_TO_SCPU_EN(enable?1 :0));
    rtd_maskl(INT_CTRL_REG, ~(INT_CTRL_DDC2_INT_TO_SCPU_EN_MASK), INT_CTRL_DDC2_INT_TO_SCPU_EN(enable?1 :0));
    rtd_maskl(INT_CTRL_REG, ~(INT_CTRL_DDC3_INT_TO_SCPU_EN_MASK), INT_CTRL_DDC3_INT_TO_SCPU_EN(enable?1 :0));
#if defined (CONFIG_ARCH_RTK2851A) || defined(CONFIG_ARCH_RTK2851C) || defined(CONFIG_ARCH_RTK2851F)

#else
    rtd_maskl(INT_CTRL_REG, ~(INT_CTRL_DDC4_INT_TO_SCPU_EN_MASK), INT_CTRL_DDC4_INT_TO_SCPU_EN(enable?1 :0));
#endif
    return 0;
}

int rtk_ddc_drv_set_emcu_init(int enable)
{
    rtd_maskl(INT_CTRL_REG, ~(INT_CTRL_DDC1_INT_TO_EMCU_EN_MASK), INT_CTRL_DDC1_INT_TO_EMCU_EN(enable?1 :0));
    rtd_maskl(INT_CTRL_REG, ~(INT_CTRL_DDC2_INT_TO_EMCU_EN_MASK), INT_CTRL_DDC2_INT_TO_EMCU_EN(enable?1 :0));
    rtd_maskl(INT_CTRL_REG, ~(INT_CTRL_DDC3_INT_TO_EMCU_EN_MASK), INT_CTRL_DDC3_INT_TO_EMCU_EN(enable?1 :0));

#if defined (CONFIG_ARCH_RTK2851A) || defined(CONFIG_ARCH_RTK2851C) || defined(CONFIG_ARCH_RTK2851F)

#else
    rtd_maskl(INT_CTRL_REG, ~(INT_CTRL_DDC4_INT_TO_EMCU_EN_MASK), INT_CTRL_DDC4_INT_TO_EMCU_EN(enable?1 :0));
#endif
    return 0;
}


int rtk_ddc_drv_segen_on(void)
{
    int id;
    unsigned long flags;
    srtk_ddc_reg_map *preg_map = NULL;

    for(id = 0; id < ARRAY_SIZE(rtk_ddc_chip); id++)
    {
        if(rtk_ddc_chip[id].reg_remap == NULL)
            continue;

        preg_map = rtk_ddc_chip[id].reg_remap;
        if (rtk_ddc_drv_set_enable(id, 1) !=0)
        {
            RTK_DDC_ERR("fail to enable ddc(0x%08x), can not write edid.\n",(unsigned int)preg_map->DDC_I2C_CR);
            return -EFAULT;
        }
        DDC_LOCK(&preg_map->g_ddc_spinlock,flags);
        //SCR Enable SGID_EN
        rtd_maskl(preg_map->DDC_DDC_SCR, ~DDC_DDC_SCR_SGID_EN_MASK, DDC_DDC_SCR_SGID_EN(1));

        //add the flow to enable the DDC_SSAR sgid en
        rtd_maskl(preg_map->DDC_DDC_SSAR, ~ISO_MISC_DDC1_DDC_SSAR_sgid_dtrden_mask, ISO_MISC_DDC1_DDC_SSAR_sgid_dtrden_mask);
        DDC_UNLOCK(&preg_map->g_ddc_spinlock,flags);
    }
    RTK_DDC_INFO("[rtk_ddc_drv_segen_on] DDC SEGEN ON\n");
    return 0;
}

int rtk_ddc_drv_isr_on(unsigned char ch)
{
    srtk_ddc_reg_map *preg_map = NULL;
    unsigned long flags;

    preg_map = rtk_ddc_chip[ch].reg_remap;
    if(preg_map == NULL)
        return -1;

    if (rtk_ddc_drv_set_enable(ch, 1) !=0)
    {
        RTK_DDC_ERR("fail to enable ddc(0x%08x), can not write edid.\n",(unsigned int)preg_map->DDC_I2C_CR);
        return -EFAULT;
    }
    //EDID_IR Enable I2C_M_RD_SP_IE
    DDC_LOCK(&preg_map->g_ddc_spinlock,flags);
    rtd_maskl(preg_map->DDC_EDID_IR, ~DDC_EDID_IR_I2C_M_RD_SP_IE_MASK, DDC_EDID_IR_I2C_M_RD_SP_IE(1));
    DDC_UNLOCK(&preg_map->g_ddc_spinlock,flags);
    RTK_DDC_INFO("[rtk_ddc_drv_isr_on] DDC ISR ON\n");
    return 0;
}

int rtk_ddc_drv_set_read_delay(unsigned char id, unsigned char delay)
{
    unsigned long flags;
    srtk_ddc_reg_map *preg_map = rtk_ddc_chip[id].reg_remap;

    if(rtk_ddc_chip[id].reg_remap == NULL || id >= ARRAY_SIZE(rtk_ddc_chip))
    {
        RTK_DDC_ERR("[set_read_delay], Id = %d doesn't exist or not creating.\n", id);
        return -EINVAL;
    }

    if(delay > 7)
    {
        RTK_DDC_ERR("Input parameter is invalid in delay of read\n");
        return -EINVAL;
    }
    DDC_LOCK(&preg_map->g_ddc_spinlock,flags);
    rtd_maskl(preg_map->DDC_DDC_SCR, ~DDC_SCR_DATA_OUTPUT_DELAY_SEL_MASK, DDC_SCR_DATA_OUTPUT_DELAY_SEL(delay));
    DDC_UNLOCK(&preg_map->g_ddc_spinlock,flags);

    return 0;
}

int rtk_ddc_drv_set_write_debounce(unsigned char id, unsigned char clock)
{
    int write_debounce = clock;
    unsigned long flags;
    srtk_ddc_reg_map *preg_map = rtk_ddc_chip[id].reg_remap;

    if(rtk_ddc_chip[id].reg_remap == NULL || id >= ARRAY_SIZE(rtk_ddc_chip))
    {
        RTK_DDC_ERR("[set_write_debounce], Id = %d doesn't exist or not creating.\n", id);
        return -EINVAL;
    }

    if(write_debounce <= 0 || write_debounce > 3)
    {
        RTK_DDC_ERR("%s , invalid write_debounce = %d \n" ,__func__ , write_debounce );
        return -EINVAL;
    }
    DDC_LOCK(&preg_map->g_ddc_spinlock,flags);
    rtd_maskl(preg_map->DDC_I2C_CR, ~DDC_I2C_CR_DEBOUNCE_MODE_MASK, DDC_I2C_CR_DEBOUNCE_MODE(write_debounce));
    DDC_UNLOCK(&preg_map->g_ddc_spinlock,flags);
    return 0;
}

int rtk_ddc_drv_set_enable(unsigned char id, int enable)
{
    srtk_ddc_reg_map *preg_map = rtk_ddc_chip[id].reg_remap;
    unsigned long flags;

    if(rtk_ddc_chip[id].reg_remap == NULL || id >= ARRAY_SIZE(rtk_ddc_chip))
    {
        RTK_DDC_ERR("[%s], Id = %d doesn't exist or not creating.\n",__func__ ,id);
        return -EINVAL;
    }

    if(enable)
    {
        DDC_LOCK(&preg_map->g_ddc_spinlock,flags);
        // RSTN : 1  : clock diable / inactive reset.
        rtd_outl(STB_ST_SRST1_REG,  (STB_ST_SRST1_WRITE_DATA_MASK| preg_map->SRST1_RSTN_DDC_RESET_MASK));
        udelay(100);
        // clocken : 1 , /clock enable  inactive reset.
        rtd_outl(STB_ST_CLKEN1_REG, (STB_ST_CLKEN1_WRITE_DATA_MASK| preg_map->CLKEN1_CLKEN_DDC_ENABLE_MSAK));
        udelay(100);
        DDC_UNLOCK(&preg_map->g_ddc_spinlock,flags);
        // restore delay & debounce setting of DDC
        rtk_ddc_drv_set_read_delay(id, DDC_DATA_READ_DELAY);
        rtk_ddc_drv_set_write_debounce(id, DDC_DATA_WRITE_DEBOUNCE);
    }
    else
    {
        DDC_LOCK(&preg_map->g_ddc_spinlock,flags);
        // RSTN : 0
        rtd_outl(STB_ST_SRST1_REG,  preg_map->SRST1_RSTN_DDC_RESET_MASK);
        // clocken : 1  : clock enable / reset pulse enable
        rtd_outl(STB_ST_CLKEN1_REG, (STB_ST_CLKEN1_WRITE_DATA_MASK| preg_map->CLKEN1_CLKEN_DDC_ENABLE_MSAK));
        udelay(100);
        // clocken: 0
        rtd_outl(STB_ST_CLKEN1_REG, preg_map->CLKEN1_CLKEN_DDC_ENABLE_MSAK);
        DDC_UNLOCK(&preg_map->g_ddc_spinlock,flags);
    }
    return 0;
}

int rtk_ddc_get_edid_state(unsigned char id, unsigned char* start_sts, unsigned char* stop_sts)
{
    unsigned char st_sts;
    unsigned char sp_sts;
    unsigned long flags;
    srtk_ddc_reg_map *preg_map = rtk_ddc_chip[id].reg_remap;

    if(rtk_ddc_chip[id].reg_remap == NULL || id >= ARRAY_SIZE(rtk_ddc_chip))
    {
        RTK_DDC_ERR("[%s], Id = %d doesn't exist or not creating.\n",__func__ ,id);
        return -EINVAL;
    }
    DDC_LOCK(&preg_map->g_ddc_spinlock,flags);
    st_sts = DDC_DDC_SCR_GET_I2C_M_RD_ST_STS(rtd_inl(preg_map->DDC_DDC_SCR));
    sp_sts = DDC_DDC_SCR_GET_I2C_M_RD_SP_STS(rtd_inl(preg_map->DDC_DDC_SCR));

    *start_sts = st_sts;
    *stop_sts = sp_sts;

    if(st_sts == 1)
    {
        rtd_maskl(preg_map->DDC_DDC_SCR, ~DDC_DDC_SCR_I2C_M_RD_ST_STS_MASK, DDC_DDC_SCR_I2C_M_RD_ST_STS(1));
    }
    if(st_sts == 1 && sp_sts == 1)
    {
        rtd_maskl(preg_map->DDC_DDC_SCR, ~DDC_DDC_SCR_I2C_M_RD_SP_STS_MASK, DDC_DDC_SCR_I2C_M_RD_SP_STS(1));
    }
    DDC_UNLOCK(&preg_map->g_ddc_spinlock,flags);

    return 0;
}

int rtk_ddc_set_edid_address(unsigned char id, unsigned char  address_num)
{
    srtk_ddc_reg_map *preg_map = rtk_ddc_chip[id].reg_remap;
    unsigned long flags;

    if(rtk_ddc_chip[id].reg_remap == NULL)
    {
        RTK_DDC_ERR("%s, Id = %d doesn't exist or not creating.\n",__func__, id);
        return -EINVAL;
    }
    DDC_LOCK(&preg_map->g_ddc_spinlock,flags);
    rtd_maskl(preg_map->DDC_EDID_CR, ~DDC_DDC_EDID_CR_EDID_ADDRESS_MASK, DDC_DDC_EDID_CR_EDID_ADDRESS(address_num));
    DDC_UNLOCK(&preg_map->g_ddc_spinlock,flags);
    return 0;
}

int rtk_ddc_drv_crt_on(unsigned char ch)
{
    unsigned int clk_val = 0;
    unsigned int reset_val = 0;
    unsigned long flags;
    srtk_ddc_reg_map *preg_map = rtk_ddc_chip[ch].reg_remap;

    RTK_DDC_ERR("rtk_ddc_drv_crt_on (Synchronous DDC reset)+++\n");

    switch (ch)
    {
        case 0:
            clk_val = STB_ST_CLKEN1_clken_ddc_mask;
            reset_val = STB_ST_SRST1_rstn_ddc_mask;
        break;
        case 1:
            clk_val = STB_ST_CLKEN1_clken_ddc1_mask;
            reset_val = STB_ST_SRST1_rstn_ddc1_mask;
        break;
        case 2:
            clk_val = STB_ST_CLKEN1_clken_ddc2_mask;
            reset_val = STB_ST_SRST1_rstn_ddc2_mask;
        break;
        case 3:
            clk_val = STB_ST_CLKEN1_clken_ddc3_mask;
            reset_val = STB_ST_SRST1_rstn_ddc3_mask;
        break;
#ifndef CONFIG_ARCH_RTK2851A
        case 4:
            clk_val = STB_ST_CLKEN1_clken_ddc4_mask;
            reset_val = STB_ST_SRST1_rstn_ddc4_mask;
        break;
#endif
        default:
            return -1;
    }
    DDC_LOCK(&preg_map->g_ddc_spinlock, flags);
    //----------------------------------------------------------------------------
    // Sync Reset flow
    // clock off -> rst on -> clock on -> clk off ->  rst off -> clock on
    //----------------------------------------------------------------------------
    rtd_outl(STB_ST_SRST1_REG, reset_val | STB_ST_SRST1_WRITE_DATA(1));   // reset off
    udelay(100);
    rtd_outl(STB_ST_CLKEN1_REG, clk_val | STB_ST_CLKEN1_WRITE_DATA(1));    // clk on
    udelay(100);
    DDC_UNLOCK(&preg_map->g_ddc_spinlock, flags);

    RTK_DDC_ERR("rtk_ddc_drv_crt_on (Synchronous DDC reset)---\n");
    return 0;
}

int rtk_ddc_drv_enable_external_dev_access(unsigned char id, int enable)
{
    unsigned long flags;
    srtk_ddc_reg_map *preg_map = rtk_ddc_chip[id].reg_remap;
    if(rtk_ddc_chip[id].reg_remap == NULL || id >= ARRAY_SIZE(rtk_ddc_chip) )
    {
        RTK_DDC_ERR("[%s], Id = %d doesn't exist or not creating.\n",__func__ ,id);
        return -EINVAL;
    }
    DDC_LOCK(&preg_map->g_ddc_spinlock, flags);
    rtd_maskl(preg_map->DDC_EDID_CR, ~DDC_EDID_CR_EDID_EN_MASK, DDC_EDID_CR_EDID_EN((enable?1 :0)));
    DDC_UNLOCK(&preg_map->g_ddc_spinlock, flags);
    return 0;
}

int rtk_ddc_drv_crt_reset(unsigned char id)
{
    int ret = 0;
    unsigned long flags;
    srtk_ddc_reg_map *preg_map = rtk_ddc_chip[id].reg_remap;
    if(rtk_ddc_chip[id].reg_remap == NULL || id >= ARRAY_SIZE(rtk_ddc_chip))
    {
        RTK_DDC_ERR("[%s], Id = %d doesn't exist or not creating.\n",__func__ ,id);
        return -EINVAL;
    }
    RTK_DDC_ERR("rtk_ddc_drv_crt_reset (%d)+++\n", id);
    DDC_LOCK(&preg_map->g_ddc_spinlock, flags);
    //----------------------------------------------------------------------------
    // Sync Reset flow
    // rst off -> clock on ->  clock off -> rst on ->  clock on
    //----------------------------------------------------------------------------

    // RSTN : 0
    rtd_outl(STB_ST_SRST1_REG,  preg_map->SRST1_RSTN_DDC_RESET_MASK);
    // clocken : 1  : clock enable / reset pulse enable
    rtd_outl(STB_ST_CLKEN1_REG, (STB_ST_CLKEN1_WRITE_DATA_MASK| preg_map->CLKEN1_CLKEN_DDC_ENABLE_MSAK));
    udelay(100);
    // clocken: 0
    rtd_outl(STB_ST_CLKEN1_REG, preg_map->CLKEN1_CLKEN_DDC_ENABLE_MSAK);
    udelay(100);

    // RSTN : 1  : clock diable / inactive reset.
    rtd_outl(STB_ST_SRST1_REG,  (STB_ST_SRST1_WRITE_DATA_MASK| preg_map->SRST1_RSTN_DDC_RESET_MASK));
    udelay(100);
    // clocken : 1 , /clock enable  inactive reset.
    rtd_outl(STB_ST_CLKEN1_REG, (STB_ST_CLKEN1_WRITE_DATA_MASK| preg_map->CLKEN1_CLKEN_DDC_ENABLE_MSAK));
    udelay(100);
    DDC_UNLOCK(&preg_map->g_ddc_spinlock, flags);

    // restore delay & debounce setting of DDC
    rtk_ddc_drv_set_read_delay(id, DDC_DATA_READ_DELAY);
    rtk_ddc_drv_set_write_debounce(id, DDC_DATA_WRITE_DEBOUNCE);
    RTK_DDC_ERR("rtk_ddc_drv_crt_reset (%d)---\n", id);
    return ret;
}

int rtk_ddc_drv_set_isr(unsigned char id, int enable)
{
    int ret = 1;
    unsigned long flags;
    srtk_ddc_reg_map *preg_map = rtk_ddc_chip[id].reg_remap;
    if(rtk_ddc_chip[id].reg_remap == NULL || id >= ARRAY_SIZE(rtk_ddc_chip))
    {
        RTK_DDC_ERR("[rtk_ddc_drv_set_isr] Id = %d doesn't exist or not creating.\n", id);
        return -EINVAL;
    }
    DDC_LOCK(&preg_map->g_ddc_spinlock,flags);
    //EDID_IR Enable I2C_M_RD_SP_IE
    rtd_maskl(preg_map->DDC_EDID_IR, ~DDC_EDID_IR_I2C_M_RD_SP_IE_MASK, DDC_EDID_IR_I2C_M_RD_SP_IE(enable));
    DDC_UNLOCK(&preg_map->g_ddc_spinlock,flags);

    RTK_DDC_INFO("[rtk_ddc_drv_set_isr] isr set %d\n", enable);

    return ret;
}

int rtk_ddc_drv_set_segen(unsigned char id, int enable)
{
    int ret = 1;
    unsigned long flags;
    srtk_ddc_reg_map *preg_map = rtk_ddc_chip[id].reg_remap;
    if(rtk_ddc_chip[id].reg_remap == NULL || id >= ARRAY_SIZE(rtk_ddc_chip))
    {
        RTK_DDC_ERR("[rtk_ddc_drv_set_segen] Id = %d doesn't exist or not creating.\n", id);
        return -EINVAL;
    }

    if ((enable<0) || (enable>1))
    {
        RTK_DDC_ERR("invalid enable value (%d), SGID_Enable be 0 or 1\n", enable);
        return -EINVAL;
    }
    DDC_LOCK(&preg_map->g_ddc_spinlock,flags);
    rtd_maskl(preg_map->DDC_DDC_SCR, ~DDC_DDC_SCR_SGID_EN_MASK, DDC_DDC_SCR_SGID_EN(enable));
#ifdef DDC_DDC_SPR_SEG_CLR_EN_MASK
    rtd_maskl(preg_map->DDC_DDC_SPR, ~DDC_DDC_SPR_SEG_CLR_EN_MASK, DDC_DDC_SPR_SEG_CLR_EN_MASK);  // always enable DDC segment auto clear
#endif

    //add the flow to enable the DDC_SSAR sgid en
    rtd_maskl(preg_map->DDC_DDC_SSAR, ~ISO_MISC_DDC1_DDC_SSAR_sgid_dtrden_mask, ISO_MISC_DDC1_DDC_SSAR_get_sgid_dtrden(enable));
    DDC_UNLOCK(&preg_map->g_ddc_spinlock,flags);

    RTK_DDC_INFO("[rtk_ddc_drv_set_segen] segen set %d\n", enable);
    return ret;
}

int rtk_ddc_drv_is_enable(unsigned char id)
{
    int enable = 0;
    unsigned long flags;
    srtk_ddc_reg_map *preg_map = rtk_ddc_chip[id].reg_remap;

    if(rtk_ddc_chip[id].reg_remap == NULL || id >= ARRAY_SIZE(rtk_ddc_chip))
    {
        RTK_DDC_ERR("[%s], Id = %d doesn't exist or not creating.\n",__func__ ,id);
        return -EINVAL;
    }
    DDC_LOCK(&preg_map->g_ddc_spinlock,flags);
    enable = ((rtd_inl(STB_ST_SRST1_REG) & preg_map->SRST1_RSTN_DDC_RESET_MASK) > 0 )?1:0;
    if(enable)
    {
        enable =((rtd_inl(STB_ST_CLKEN1_REG) & preg_map->CLKEN1_CLKEN_DDC_ENABLE_MSAK) > 0 )?1:0;
    }
    DDC_UNLOCK(&preg_map->g_ddc_spinlock,flags);
    return enable;
}

int rtk_ddc_drv_get_write_debounce(unsigned char id)
{
    int write_debounce = 0;
    unsigned long flags;
    srtk_ddc_reg_map *preg_map = rtk_ddc_chip[id].reg_remap;

    if(rtk_ddc_chip[id].reg_remap == NULL || id >= ARRAY_SIZE(rtk_ddc_chip))
    {
        RTK_DDC_ERR("[%s], Id = %d doesn't exist or not creating.\n",__func__ ,id);
        return -EINVAL;
    }
    DDC_LOCK(&preg_map->g_ddc_spinlock,flags);
    write_debounce = DDC_I2C_CR_GET_DEBOUNCE_MODE(rtd_inl(preg_map->DDC_I2C_CR));
    DDC_UNLOCK(&preg_map->g_ddc_spinlock,flags);
    return write_debounce;
}

int rtk_ddc_drv_get_read_delay(unsigned char id)
{
    int read_delay = 0;
    unsigned long flags;
    srtk_ddc_reg_map *preg_map = rtk_ddc_chip[id].reg_remap;

    if(rtk_ddc_chip[id].reg_remap == NULL || id >= ARRAY_SIZE(rtk_ddc_chip))
    {
        RTK_DDC_ERR("[%s], Id = %d doesn't exist or not creating.\n",__func__ ,id);
        return -EINVAL;
    }
    DDC_LOCK(&preg_map->g_ddc_spinlock,flags);
    read_delay = DDC_DDC_SCR_GET_DATAOUTPUTDELAY_SEL(rtd_inl(preg_map->DDC_DDC_SCR));
    DDC_UNLOCK(&preg_map->g_ddc_spinlock,flags);
    return read_delay;
}

int rtk_ddc_drv_get_segen(unsigned char id)
{
    int segen = 0;
    unsigned long flags;
    srtk_ddc_reg_map *preg_map = rtk_ddc_chip[id].reg_remap;

    if(rtk_ddc_chip[id].reg_remap == NULL || id >= ARRAY_SIZE(rtk_ddc_chip))
    {
        RTK_DDC_ERR("[%s], Id = %d doesn't exist or not creating.\n",__func__ ,id);
        return -EINVAL;
    }
    DDC_LOCK(&preg_map->g_ddc_spinlock,flags);
    segen = DDC_DDC_SCR_GET_SGID_EN(rtd_inl(preg_map->DDC_DDC_SCR));
    DDC_UNLOCK(&preg_map->g_ddc_spinlock,flags);
    return segen;
}

int rtk_ddc_drv_read(unsigned char id, srtk_edid* pedid)
{
    int i   = 0;
    unsigned long flags;
    srtk_ddc_reg_map *preg_map = rtk_ddc_chip[id].reg_remap;

    if(NULL == pedid || 0 == rtk_ddc_drv_edid_content_is_valid(id, pedid->len, pedid->content))
    {
        RTK_DDC_ERR("%s : input parameter is invalid in read edid\n", __func__);
        return -EINVAL;
    }
    DDC_LOCK(&preg_map->g_ddc_spinlock, flags);
    {
        rtd_outl(preg_map->DDC_DDC_SIR, 0x0);
        for(i=0;i < pedid->len;i++)
        {
            pedid->content[i] = rtd_inl(preg_map->DDC_DDC_SAP);
        }
    }
    DDC_UNLOCK(&preg_map->g_ddc_spinlock, flags);
    return 0;
}

int rtk_ddc_drv_write(unsigned char id, srtk_edid* pedid)
{
    int i   = 0;
    int ret = 0;
    int channel_enble = 0;
    unsigned long flags;
    srtk_ddc_reg_map *preg_map = rtk_ddc_chip[id].reg_remap;

    if (NULL == pedid || 0 == rtk_ddc_drv_edid_content_is_valid(id, pedid->len, pedid->content))
    {
        RTK_DDC_ERR("%s : input parameter is invalid in read edid\n", __func__);
        return -EINVAL;
    }

    if (rtk_ddc_drv_set_enable(id, 1) !=0)
    {
        RTK_DDC_ERR("fail to enable ddc(0x%08x), can not write edid.\n",(unsigned int)preg_map->DDC_I2C_CR);
        return -EFAULT;
    }
    DDC_LOCK(&preg_map->g_ddc_spinlock,flags);
    // check edid is enabled or not
    channel_enble = DDC_EDID_CR_GET_EDID_EN(rtd_inl(preg_map->DDC_EDID_CR));
    DDC_UNLOCK(&preg_map->g_ddc_spinlock,flags);

    // make sure no one is accessing EDID if edid is enabled
    while(channel_enble && i++ < 40)
    {
        int fsm = 0;
        DDC_LOCK(&preg_map->g_ddc_spinlock,flags);
        fsm = GET_DDC_FSMS_STATUS(preg_map);
        DDC_UNLOCK(&preg_map->g_ddc_spinlock,flags);

        if (fsm == DDC_FSM_STATUS_IDEL || fsm == DDC_FSM_STATUS_ID_DETECT)
            break;

        if ((i & 0x7)==0)
        {
            RTK_DDC_ERR("rtk_ddc_drv_write : ddc bus is busy, waiting for ddc free (0x%08x) fsm = %d (%s)...(%d)\n",
                (unsigned int)preg_map->DDC_I2C_CR, fsm, _ddc_bus_status_str(fsm), i);
        }

        mdelay(1);
    }

    if (i>=40)
    {
        RTK_DDC_ERR("rtk_ddc_drv_write : ddc bus free (0x%08x) timeout, continue ddc write (%d)\n", (unsigned int)preg_map->DDC_I2C_CR, i);
    }

    // disable edid access
    if (0 != rtk_ddc_drv_enable_external_dev_access(id, 0))
        RTK_DDC_ERR("id%d fail to switch to internal access\n", id);

    DDC_LOCK(&preg_map->g_ddc_spinlock,flags);
    // write EDID
    rtd_outl(preg_map->DDC_DDC_SIR, 0);

    for(i=0;i<pedid->len;i++)
        rtd_outl(preg_map->DDC_DDC_SAP, pedid->content[i]);

    DDC_UNLOCK(&preg_map->g_ddc_spinlock,flags);

    // enable edid access if possibile
    if(0 != rtk_ddc_drv_enable_external_dev_access(id, channel_enble))
        RTK_DDC_ERR("id%d fail to switch to access right\n", id);

    return ret;
}

int rtk_ddc_drv_set_aux_rx_enable(unsigned char id, int enable)
{
    int ret = 1;
    if(rtk_ddc_chip[id].reg_remap == NULL || id >= ARRAY_SIZE(rtk_ddc_chip))
    {
        RTK_DDC_ERR("[rtk_ddc_drv_set_aux_rx_enable] Id = %d doesn't exist or not creating.\n", id);
        return -EINVAL;
    }

    if ((enable<0) || (enable>1))
    {
        RTK_DDC_ERR("invalid enable value (%d), aux_rx_enable be 0 or 1\n", enable);
        return -EINVAL;
    }

    if(rtk_ddc_chip[id].id != 3)        //only DDC3 support i2c over aux
    {
        RTK_DDC_ERR("[rtk_ddc_drv_set_aux_rx_enable] Id = %d don't support i2c over aux.\n", id);
        return -EINVAL;
    }
    rtd_maskl(DDC_CTRL_REG, ~DDC_CTRL_AUXRX_DDC_EN_MASK, DDC_CTRL_AUXRX_DDC_EN(enable));
    RTK_DDC_INFO("[rtk_ddc_drv_set_segen] segen set %d\n", enable);
    return ret;
}

#ifdef CONFIG_RTK_KDRV_DDC_WAKEUP
void rtk_ddc_drv_wakeup_init(unsigned char ch)
{
    // enable wakeup
    rtk_ddc_drv_enable_ddc_wakeup(ch, 1);
    // disable interrupt
    rtk_ddc_drv_set_ddc_wakeup_irq(ch, 0);

    // Set wakeup addresses and enable them
    rtk_ddc_drv_set_ddc_wakeup_addr(ch, 0x50, 1); // 0xA0 EDID
    rtk_ddc_drv_set_ddc_wakeup_addr(ch, 0x52, 1); // 0xA4 EDID
    rtk_ddc_drv_set_ddc_wakeup_addr(ch, 0x30, 1); // 0x60 E-EDID
    rtk_ddc_drv_set_ddc_wakeup_addr(ch, 0x37, 1); // 0x6E reserved for DDC/CI
    rtk_ddc_drv_set_ddc_wakeup_addr(ch, 0x3A, 1); // 0x74 HDCP
    rtk_ddc_drv_set_ddc_wakeup_addr(ch, 0x3B, 1); // 0x76 HDCP
    rtk_ddc_drv_set_ddc_wakeup_addr(ch, 0x54, 1); // 0xA8 SCDC
    rtk_ddc_drv_set_ddc_wakeup_addr(ch, 0x40, 1); // 0x80 reserved for display port(Dual mode video Adaptor[2])
}

/**
 * @func: Enable or disable the ddc wakeup function.
 * @param[in] id: DDC channel id [1,2,3,4]
 * @param[in] enable: Enable or disable wakeup
 * @return
 *      - 0  success
 *      - other fail
 */
int rtk_ddc_drv_enable_ddc_wakeup(unsigned char id, unsigned char enable)
{
    iso_misc_ddc1_wakeup_hdcp_RBUS reg_value;

    if(rtk_ddc_chip[id].reg_remap == NULL || id >= ARRAY_SIZE(rtk_ddc_chip))
    {
        RTK_DDC_ERR("[%s] Id = %d doesn't exist or not creating.\n", __func__, id);
        return -EINVAL;
    }

    reg_value.regValue = rtd_inl(rtk_ddc_chip[id].reg_remap->WAKEUP_HDCP);
    reg_value.wakeup_en = enable?1:0;
    rtd_outl(rtk_ddc_chip[id].reg_remap->WAKEUP_HDCP, reg_value.regValue);

    return 0;
}


/**
 * @func: Gets the enable status of wakeup
 * @param[in] id: DDC channel id [1,2,3,4]
 * @param[in] enable: Enable or disable wakeup
 * @return
 *      - 0  success
 *      - other fail
 */
int rtk_ddc_drv_get_ddc_wakeup_enable(unsigned char id, unsigned char *enable)
{
    iso_misc_ddc1_wakeup_hdcp_RBUS reg_value;

    if(rtk_ddc_chip[id].reg_remap == NULL || id >= ARRAY_SIZE(rtk_ddc_chip))
    {
        RTK_DDC_ERR("[%s] Id = %d doesn't exist or not creating.\n", __func__, id);
        return -EINVAL;
    }

    reg_value.regValue = rtd_inl(rtk_ddc_chip[id].reg_remap->WAKEUP_HDCP);
    *enable = reg_value.wakeup_en;

    return 0;
}


/**
 * @func: Get the address which has been hitted.
 * @param[in] id: DDC channel id [1,2,3,4]
 * @param[out] addr: Which address has been hitted
 * @return
 *      - 0  success
 *      - other fail
 */
int rtk_ddc_drv_get_ddc_hitted_wakeup_addr(unsigned char id, unsigned char *addr)
{
    iso_misc_ddc1_wakeup_hdcp_RBUS reg_value;

    if(rtk_ddc_chip[id].reg_remap == NULL || id >= ARRAY_SIZE(rtk_ddc_chip))
    {
        RTK_DDC_ERR("[%s] Id = %d doesn't exist or not creating.\n", __func__, id);
        return -EINVAL;
    }

    reg_value.regValue = rtd_inl(rtk_ddc_chip[id].reg_remap->WAKEUP_HDCP);

    if (reg_value.hit)
        *addr = reg_value.addr;

    return 0;
}


/**
 * @func: Clear trigger state.
 * @param[in] id: DDC channel id [1,2,3,4]
 * @return
 *      - 0  success
 *      - other fail
 */
int rtk_ddc_drv_clr_ddc_hitted_wakeup_addr(unsigned char id)
{
    iso_misc_ddc1_wakeup_hdcp_RBUS reg_value;

    if(rtk_ddc_chip[id].reg_remap == NULL || id >= ARRAY_SIZE(rtk_ddc_chip))
    {
        RTK_DDC_ERR("[%s] Id = %d doesn't exist or not creating.\n", __func__, id);
        return -EINVAL;
    }

    reg_value.regValue = rtd_inl(rtk_ddc_chip[id].reg_remap->WAKEUP_HDCP);
    rtd_outl(rtk_ddc_chip[id].reg_remap->WAKEUP_HDCP, reg_value.regValue);

    return 0;
}


/**
 * @func: Enable or disable the wakeup of the specified address.
 * @param[in] id: DDC channel id [1,2,3,4]
 * @param[in] addr: The specified address for wakeup
 * @param[in] enable: Enable or disable wakeup
 * @return
 *      - 0  success
 *      - other fail
 */
int rtk_ddc_drv_set_ddc_wakeup_addr(unsigned char id, unsigned char addr, unsigned char enable)
{
    iso_misc_ddc1_wakeup_addr_1_RBUS reg_value;
    unsigned int tmp_value = 0;
    unsigned int addr_mask = 0;
    unsigned char shift_bit = 0;
    unsigned char i,j;
    unsigned char wakeup_enabled = 0;
    unsigned int wakeup_addr = 0;

    if(rtk_ddc_chip[id].reg_remap == NULL || id >= ARRAY_SIZE(rtk_ddc_chip))
    {
        RTK_DDC_ERR("[%s] Id = %d doesn't exist or not creating.\n", __func__, id);
        return -EINVAL;
    }

    for (i=0; i< DDC_WAKEUP_ADDR_REG_NUM; i++)
    {
        wakeup_addr = rtk_ddc_chip[id].reg_remap->WAKEUP_ADDR_1+i*0x4;

        if (!wakeup_addr)
            continue;

        reg_value.regValue = rtd_inl(wakeup_addr);

        tmp_value = reg_value.regValue;
        for (j=0; j< DDC_WAKEUP_ADDR_NUM_PER_REG; j++)
        {
            shift_bit = j*8;
            addr_mask = 0xFFUL<<shift_bit;
            if (enable)
            {
                if (!wakeup_enabled)
                {
                    // Already enabled
                    if (reg_value.wakeup_address_1_en && reg_value.wakeup_address_1 == addr)
                        return 0;

                    // Found a unused address
                    if (!reg_value.wakeup_address_1_en)
                    {
                        reg_value.wakeup_address_1 = addr&0x7F;
                        reg_value.wakeup_address_1_en = 1;
                        tmp_value &= ~addr_mask;
                        tmp_value |= (reg_value.regValue & 0xFFUL)<<shift_bit;

                        rtd_outl(wakeup_addr, tmp_value);

                        wakeup_enabled = 1;
                    }
                }
                else
                {
                    // Find duplicate enabled addresses, disable it.
                    if (reg_value.wakeup_address_1_en && reg_value.wakeup_address_1 == addr)
                    {
                        reg_value.wakeup_address_1_en = 0;
                        tmp_value &= ~addr_mask;
                        tmp_value |= (reg_value.regValue & 0xFFUL)<<shift_bit;
 
                        rtd_outl(wakeup_addr, tmp_value);

                    }
                }
            }
            else
            {
                // Already enabled
                if (reg_value.wakeup_address_1_en && reg_value.wakeup_address_1 == addr)
                {
                    tmp_value &= ~addr_mask;

                    rtd_outl(wakeup_addr, tmp_value);

                }
            }

            reg_value.regValue >>= 8;
        }
    }

    if (wakeup_enabled || !enable)
        return 0;
    return -1;
}


/**
 * @func: Get all wakeup addresses.
 * @param[in] id: DDC channel id [1,2,3,4]
 * @param[out] *addr: The pointer for save wakeup addresses
 * @param[in] addr_len: The length of *addr
 * @param[out] *addr_cnt: how many addresses are enabled
 * @return
 *      - 0  success
 *      - other fail
 */
int rtk_ddc_drv_get_ddc_wakeup_addr(unsigned char id, unsigned char *addr, unsigned char addr_len, unsigned char *addr_cnt)
{
    iso_misc_ddc1_wakeup_addr_1_RBUS reg_value;
    unsigned int wakeup_addr = 0;
    unsigned char total_wakeup_addr_cnt = DDC_WAKEUP_ADDR_REG_NUM*DDC_WAKEUP_ADDR_NUM_PER_REG;
    unsigned char i = 0;
    unsigned char j = 0;

    if(rtk_ddc_chip[id].reg_remap == NULL || id >= ARRAY_SIZE(rtk_ddc_chip))
    {
        RTK_DDC_ERR("[%s] Id = %d doesn't exist or not creating.\n", __func__, id);
        return -EINVAL;
    }

    if (addr_len < total_wakeup_addr_cnt)
    {
        RTK_DDC_ERR("[%s] Address length should be greater than %d.\n", __func__, total_wakeup_addr_cnt);
        return -EINVAL;
    }

    while (i< DDC_WAKEUP_ADDR_REG_NUM)
    {
        wakeup_addr = rtk_ddc_chip[id].reg_remap->WAKEUP_ADDR_1+i*0x4;
        i++;

        if (!wakeup_addr)
            continue;

        reg_value.regValue = rtd_inl(wakeup_addr);

        if (reg_value.wakeup_address_1_en)
            addr[j++] = reg_value.wakeup_address_1;
        if (reg_value.wakeup_address_2_en)
            addr[j++] = reg_value.wakeup_address_2;
        if (reg_value.wakeup_address_3_en)
            addr[j++] = reg_value.wakeup_address_3;
        if (reg_value.wakeup_address_4_en)
            addr[j++] = reg_value.wakeup_address_4;
    }

    *addr_cnt = j;
    return 0;
}


// Clear all wake up address and disable wake up.
int rtk_ddc_drv_reset_ddc_wakeup_addr(unsigned char id)
{
    unsigned char i;
    unsigned int wakeup_addr = 0;

    if(rtk_ddc_chip[id].reg_remap == NULL || id >= ARRAY_SIZE(rtk_ddc_chip))
    {
        RTK_DDC_ERR("[%s] Id = %d doesn't exist or not creating.\n", __func__, id);
        return -EINVAL;
    }

    for (i=0; i< DDC_WAKEUP_ADDR_REG_NUM; i++)
    {
        wakeup_addr = rtk_ddc_chip[id].reg_remap->WAKEUP_ADDR_1+i*0x4;
        if (!wakeup_addr)
            continue;

        rtd_outl(wakeup_addr, 0);

    }

    return 0;
}


int rtk_ddc_drv_set_ddc_wakeup_irq(unsigned char id, unsigned char irq)
{
    iso_misc_ddc1_wakeup_hdcp_RBUS reg_value;

    if(rtk_ddc_chip[id].reg_remap == NULL || id >= ARRAY_SIZE(rtk_ddc_chip))
    {
        RTK_DDC_ERR("[%s] Id = %d doesn't exist or not creating.\n", __func__, id);
        return -EINVAL;
    }

    reg_value.regValue = rtd_inl(rtk_ddc_chip[id].reg_remap->WAKEUP_HDCP);
    reg_value.wakeup_irq_en = irq?1:0;
    rtd_outl(rtk_ddc_chip[id].reg_remap->WAKEUP_HDCP, reg_value.regValue);

    return 0;
}

int rtk_ddc_drv_get_ddc_wakeup_irq(unsigned char id, unsigned char *irq)
{
    iso_misc_ddc1_wakeup_hdcp_RBUS reg_value;

    if(rtk_ddc_chip[id].reg_remap == NULL || id >= ARRAY_SIZE(rtk_ddc_chip))
    {
        RTK_DDC_ERR("[%s] Id = %d doesn't exist or not creating.\n", __func__, id);
        return -EINVAL;
    }

    reg_value.regValue = rtd_inl(rtk_ddc_chip[id].reg_remap->WAKEUP_HDCP);

    *irq = reg_value.wakeup_irq_en;

    return 0;
}


#endif // CONFIG_RTK_KDRV_DDC_WAKEUP


#ifndef BUILD_QUICK_SHOW
#ifdef CONFIG_RTK_KDRV_DDCCI	
bool rtk_ddcci_pooling_t(void);
#endif
static irqreturn_t rtk_ddc_isr(int this_irq, void *dev_id)
{
    srtk_ddc_chip *p_this = (srtk_ddc_chip *) dev_id;
    int SP_STS = 0;
    int SP_SEG_PT = 0;
    int LATCHED_STS = 0;
    unsigned long flags;

    #define GET_IC_SCR(adp)        rtd_inl(adp->reg_remap->DDC_DDC_SCR)
    #define GET_IC_SPR(adp)        rtd_inl(adp->reg_remap->DDC_DDC_SPR)
    #define GET_IC_ISR(adp)        rtd_inl(adp->reg_remap->MISC_ISR)

    spin_lock_irqsave(&p_this->reg_remap->g_ddc_spinlock, flags);
    if (!(GET_IC_ISR(p_this) & p_this->reg_remap->ISR_DDC_INT_MASK))
    {
        spin_unlock_irqrestore(&p_this->reg_remap->g_ddc_spinlock,flags);
        return IRQ_NONE;
    }

    SP_STS = DDC_DDC_SCR_GET_I2C_M_RD_SP_STS(GET_IC_SCR(p_this));
    spin_unlock_irqrestore(&p_this->reg_remap->g_ddc_spinlock, flags);

    if(SP_STS == 0)
    {
#ifdef CONFIG_RTK_KDRV_DDCCI	
#ifndef CONFIG_RTK_KDRV_DDCCI_USING_POLLING
    	 if(rtk_ddcci_pooling_t())
            return IRQ_HANDLED;
#endif
#endif
        return IRQ_NONE;
    }
    spin_lock_irqsave(&p_this->reg_remap->g_ddc_spinlock, flags);
    LATCHED_STS = DDC_DDC_SPR_GET_SEGPT_LATCHED(GET_IC_SPR(p_this));
    SP_SEG_PT = DDC_DDC_SPR_GET_SEG_PT(GET_IC_SPR(p_this));

    if(LATCHED_STS == 1)
    {
        // clear LATCHED_STS
        rtd_maskl(p_this->reg_remap->DDC_DDC_SPR, ~DDC_DDC_SPR_SEGPT_LATCHED_MASK, DDC_DDC_SPR_SEGPT_LATCHED(1));
    }
    spin_unlock_irqrestore(&p_this->reg_remap->g_ddc_spinlock,flags);
    if(SP_SEG_PT > 0)
    {
        int iDebounce = rtk_ddc_drv_get_write_debounce(p_this->id);
        int iReadDelay = rtk_ddc_drv_get_read_delay(p_this->id);

        rtk_ddc_drv_crt_reset(p_this->id);
        rtk_ddc_drv_set_segen(p_this->id, 1);
        rtk_ddc_drv_set_isr(p_this->id, 1);

        rtk_ddc_drv_set_write_debounce(p_this->id, iDebounce);
        rtk_ddc_drv_set_read_delay(p_this->id, iReadDelay);
        rtk_ddc_drv_enable_external_dev_access(p_this->id, 1);

    }

    spin_lock_irqsave(&p_this->reg_remap->g_ddc_spinlock, flags);
    // clear SP_STS
    rtd_maskl(p_this->reg_remap->DDC_DDC_SCR, ~DDC_DDC_SCR_I2C_M_RD_SP_STS_MASK, DDC_DDC_SCR_I2C_M_RD_SP_STS(1));

    LATCHED_STS = DDC_DDC_SPR_GET_SEGPT_LATCHED(GET_IC_SPR(p_this));
    SP_SEG_PT = DDC_DDC_SPR_GET_SEG_PT(GET_IC_SPR(p_this));

    spin_unlock_irqrestore(&p_this->reg_remap->g_ddc_spinlock,flags);
#ifdef CONFIG_RTK_KDRV_DDCCI	
#ifndef CONFIG_RTK_KDRV_DDCCI_USING_POLLING
    rtk_ddcci_pooling_t();
#endif
#endif
    RTK_DDC_INFO("[rtk_ddc_isr] DDC interrupt : SP_STS = %d, LATCHED_STS = %d, SP_SEG_PT = %d\n", SP_STS, LATCHED_STS, SP_SEG_PT);
    return IRQ_HANDLED;
}

int rtk_ddc_drv_interrupt(int ddc_irq_t)
{
    int id = 0 ;
    int ret = 0;
    srtk_ddc_chip *p_this;
    
    for(id = 0; id < ARRAY_SIZE(rtk_ddc_chip); id++)
    {
        if(rtk_ddc_chip[id].reg_remap == NULL)
            continue;

        p_this = &rtk_ddc_chip[id];
        if ((ret =request_irq(ddc_irq_t, rtk_ddc_isr, IRQF_SHARED, "ddc", (void *)p_this)) < 0)
        {
            RTK_DDC_ERR("Request irq%d failed(ret=%d)\n", ddc_irq_t, ret);
            return -1;
        }
    }
    
    return ret;
}


int rtk_ddc_drv_free_interrupt(int ddc_irq_t)
{
    int ret = 0;
    int id = 0;
    srtk_ddc_chip *p_this;
    
    for(id = 0; id < ARRAY_SIZE(rtk_ddc_chip); id++)
    {
        if(rtk_ddc_chip[id].reg_remap == NULL)
            continue;

        p_this = &rtk_ddc_chip[id];
        free_irq(ddc_irq_t, (void *)p_this);
    }


    return ret;
}

#endif
