/*=============================================================
 * Copyright (c)      Realtek Semiconductor Corporation, 2016
 *
 * All rights reserved.
 *
 *============================================================*/
#include <linux/string.h>
#include <linux/init.h>
#include "hdmi_common.h"
#include "hdmi_vfe_config.h"
#include "hdmi_debug.h"

unsigned char g_hdmi_drv_port_mapping[HDMI_RX_PORT_MAX_NUM];
unsigned char g_hdmi_drv_socts_en = 0;

int get_hdmi_rx_hdmi_channel_number(void)
{
    return HDMI_RX_PORT_MAX_NUM;
}


/**********************HDMI_CFG format************************
 *  D63:60  : HDMI_PHY_PORT
 *  D59     : HDMI SUPPORT ARC
 *  D58:56  : HDMI_DDC
 *  D55:54  : HDMI_EDID_TYPE       (0: on chip, 1:EEPROM 2: with MUX, 3:Not used)
 *  D53     : Reserved
 *  D52     : BRSWAP               (0: no BRSWAP, 1: B/R Lane Swap)
 *  D51:49  : Reserved
 *  D48     : HDMI SUPPORT EARC
 *  D47:36  : Reserved
 *  D35:32  : Reserved
 *  D31     : DEDICATE_HPD_DET      (1 : use dedicate HPD/DET mothod, 0: use legacy GPIO HPD/DET)
 *  D30     : DEDICATE_HPD_INVERT   (0: non invert, 1: invert)
 *  D29:28  : DEDICATE_HPD_DET_SEL  (0~2)
 *  D27     : Reserved
 * ------ the following setting only valid when  DEDICATE_HPD_DET=0 ------
 *  D26     : 5V_DET_GPIO_INIT_VAL  (0: INIT_VALUE, 1: INIT_VALUE)
 *  D25     : 5V_DET_GPIO_POLARITY  (0: negative, 1: positive)
 *  D24     : 5V_DET_GPIO_PIN_TYPE  (0: MISC, 1: ISO)
 *  D23:16  : 5V_DET_GPIO_INDEX     (GPIO ID of 5V detect)
 *  D15:12  : Reserved
 *  D11     : HOTPLUG_GPIO_ENABLE   (0: No Gpio Det, 1: with gpio det)
 *  D10     : HOTPLUG_GPIO_INIT_VAL (0: INIT_VALUE, 1: INIT_VALUE)
 *  D9      : HOTPLUG_GPIO_POLARITY (0: negative, 1: positive)
 *  D8      : HOTPLUG_GPIO_PIN_TYPE (0: MISC, 1: ISO)
 *  D7:0    : HOTPLUG_GPIO_INDEX    (GPIO ID of Hotplug GPIO)
 *****************************************************************/
int hdmi_vfe_get_rx_port_info(unsigned char port, HDMI_CHANNEL_T_PCB_INFO* p_info)
{
    unsigned long long val;
    char name[16] ={0};

    snprintf(name, sizeof(name)-1, "HDMI%d_CFG", port+1);

    if (pcb_mgr_get_enum_info_byname(name, &val) != 0)
    {
        HDMI_ERR("%s Get PCB Parameter fail name = %s is not existed !\n" , __func__  , name );
        p_info->phy_port = HDMI_PHY_PORT_NOTUSED;
        return -1;
    }
    p_info->valid_port = 1;
    p_info->phy_port     = (HDMI_PHY_PORT_T) ((val >> 60) & 0xf);
    p_info->ddc_selected = (DDC_NUMBER_T)((val >> 56) & 0x7);
    p_info->edid_type    = (HDMI_EDID_TYPE_T)((val >> 54) & 0x3);
    p_info->br_swap       = (val>>52) & 0x1;

    p_info->dedicate_hpd_det = (val>>31) & 0x1;
    p_info->dedicate_hpd_det_sel = (val>>28) & 0x3;
    p_info->dedicate_hpd_invert = (val>>30) & 0x1;

    p_info->hpd          = rtk_gpio_id((val & (0x1<<8))  ? ISO_GPIO : MIS_GPIO, (val & 0xFF));

    p_info->det_5v       = rtk_gpio_id((val & (0x1<<24)) ? ISO_GPIO : MIS_GPIO, ((val>>16) & 0xFF));
    p_info->arc_support  = (val >> 59) & 0x1;
    p_info->earc_support  = (val >> 48) & 0x1;

    if (p_info->dedicate_hpd_det)
    {
        HDMI_INFO("HDMI Port[%d] : Phy=%d, ddc_bus=%d, edid_type=%d, br_swap=%d, hpd=dedicate_hpd_%d, hpd_invert=%d, 5v_det=dedicate_det_%d, arc_support=%d, earc_support=%d\n",
            port, p_info->phy_port, p_info->ddc_selected, p_info->edid_type,
            p_info->br_swap,
            p_info->dedicate_hpd_det_sel,
            p_info->dedicate_hpd_invert,
            p_info->dedicate_hpd_det_sel,
            p_info->arc_support,
            p_info->earc_support);
    }
    else
    {
        HDMI_INFO("HDMI Port[%d] : Phy=%d, ddc_bus=%d, edid_type=%d, br_swap=%d, hpd=%s_%d, 5v_det=%s_%d, arc_support=%d, earc_support=%d\n",
            port, p_info->phy_port, p_info->ddc_selected, p_info->edid_type,
            p_info->br_swap,
            gpio_type(gpio_group(p_info->hpd)), gpio_idx(p_info->hpd),
            gpio_type(gpio_group(p_info->det_5v)), gpio_idx(p_info->det_5v),
            p_info->arc_support,
            p_info->earc_support);
    }

    return 0;
}


/*-------------------------------------------------
 * Func : board_hdmi_drv_port_mapping_Initial
 *
 * Desc : init board config mapping. the logical port
 *        mapping is defined by pcb parameter
 *
 * Para : N/A
 *
 * Retn : N/A
 *-------------------------------------------------*/
void board_hdmi_drv_port_mapping_Initial(void)
{
    unsigned long long hdmi_port_mapping;
    unsigned long long hdmi_port_tmp;
    unsigned char i =0;

    memset(g_hdmi_drv_port_mapping, 0xF, sizeof(g_hdmi_drv_port_mapping));

    //HDMI_PORT_ORDER order is
    //UI [HDMI4, HDMI3, HDMI2, HDMI1]
    //HW[bit 3, bit 2, bit 1, bit 0]
    //Ex : hdmi_port_mapping = 0x1234
    //HDMI4 = port 0, HDMI3 = port 1, HDMI2 = port 2, HDMI1 = port 3
    if (pcb_mgr_get_enum_info_byname("HDMI_PORT_ORDER", &hdmi_port_mapping) == 0)
    {
        for(i = 0; i < HDMI_RX_PORT_MAX_NUM; i++) {
            hdmi_port_tmp = ((hdmi_port_mapping>>(4*i)) & 0xF);

            if (hdmi_port_tmp > 0)
                g_hdmi_drv_port_mapping[i] = hdmi_port_tmp - 1;
        }
    }
    else
    {
        HDMI_INFO("%s, HDMI_PORT_ORDER not define, using default value\n" , __func__ );

        for(i = 0; i < HDMI_RX_PORT_MAX_NUM; i++) {
            g_hdmi_drv_port_mapping[i] = i;
        }
    }
}



/*-------------------------------------------------
 * Func : hdmi_vfe_get_hdmi_port
 *
 * Desc : convert from logical hdmi channel to
 *        phyiscal hdmi port
 *
 * Para : ch : hdmi channel input
 *        p_port : hdmi port output
 *
 * Retn : Physical HDMI port
 *-------------------------------------------------*/
int hdmi_vfe_get_hdmi_port(unsigned char ch, unsigned char* p_port)
{
    if (p_port==NULL || ch >= sizeof(g_hdmi_drv_port_mapping) || 
        g_hdmi_drv_port_mapping[ch]==0xF) 
    {
        //HDMI_WARN("%s failed, ch=%d\n", __func__, ch);
        return -1;
    }
    
    *p_port = g_hdmi_drv_port_mapping[ch];
    return 0;
}


/*-------------------------------------------------
 * Func : hdmi_vfe_get_hdmi_channel
 *
 * Desc : convert from phyiscal hdmi port to logical hdmi channel
 *
 * Para : port : hdmi port input
 *        p_ch : hdmi channel output
 *
 * Retn : Physical HDMI port
 *-------------------------------------------------*/
int hdmi_vfe_get_hdmi_channel(unsigned char port, unsigned char* p_ch)
{
    unsigned char ch;

    if (p_ch==NULL || port >= HDMI_RX_PORT_MAX_NUM) {
        //HDMI_WARN("%s failed, port=%d\n", __func__, port);
        return -1;
    }

    for (ch = 0; ch < sizeof(g_hdmi_drv_port_mapping); ch++) {
        if (port == g_hdmi_drv_port_mapping[ch]) {
            *p_ch = ch;
            return 0;
        }
    }
    return -1;
}

/*---------------------------------------------------
 * Func : newbase_hdmi_calculate_entry_max_value
 *
 * Desc : calculate max value of cfg_table's entry.
 *
 * Para : group_id: cfg_table index, entry_id, entry_id in the table.
 *
 * Retn : > 0 : valid max value, =0 : failed
 *--------------------------------------------------*/
unsigned int newbase_hdmi_calculate_entry_max_value(HDMI_FLOW_CFG_GROUP group_id, unsigned char entry_id)
{
    unsigned int max_value = 0;

    if(group_id >=  HDMI_FLOW_CFG_GROUP_NUM)
    {
        HDMI_ERR("[%s] Wrong value , Read group_id %d fail!\n" , __func__ , group_id);
    }
    else
    {
        if(entry_id >= p_flow_cfg_group_table[group_id].cfg_table_length)
        {
            HDMI_ERR("[%s] Wrong value , Read entry_id %d fail!\n" , __func__ , entry_id);	
        }
        else
        {
            HDMI_CFG_ENTRY_T* p_current_cfg_table = p_flow_cfg_group_table[group_id].p_cfg_table;
            int bit_diff = p_current_cfg_table[entry_id].end_bit_index - p_current_cfg_table[entry_id].start_bit_index;
            if(bit_diff<0)
            {
                HDMI_ERR("[newbase_hdmi_calculate_entry_max_value] ERROR, invalid bit index !! start_bit_index = %d, end_bit_index = %d \n",  p_current_cfg_table[entry_id].start_bit_index, p_current_cfg_table[entry_id].end_bit_index);
            }
            else
            {
                max_value = (1<<(bit_diff +1)) - 1;
                //HDMI_PRINTF("[newbase_hdmi_calculate_entry_max_value] entry_id = %d, max_value = %d\n",  entry_id , max_value);
            }
        }
    }
    return max_value;
}

int  newbase_hdmi_set_flow_cfg_entry(HDMI_FLOW_CFG_GROUP group_id, unsigned char entry_id, unsigned int set_value)
{
    int result = -1;
    if(group_id >=  HDMI_FLOW_CFG_GROUP_NUM)
    {
        result = -1;
        HDMI_ERR("[%s] Wrong value , Read group_id %d fail!\n" , __func__ , group_id);
    }
    else
    {
        if(entry_id >= p_flow_cfg_group_table[group_id].cfg_table_length)
        {
            result = -2;
            HDMI_ERR("[%s] Wrong value , Read entry_id %d fail!\n" , __func__ , entry_id);	
        }
        else
        {
            HDMI_CFG_ENTRY_T* p_current_cfg_table = p_flow_cfg_group_table[group_id].p_cfg_table;
            if(set_value >newbase_hdmi_calculate_entry_max_value(group_id, entry_id))
            {
                result = -3;
                HDMI_ERR("[%s] Wrong set value ,  %s=%d!\n" , __func__ ,
                    GET_FLOW_CFG_NAME(group_id, entry_id), set_value);	
            }
            else
            {
                 //HDMI_PRINTF("[%s] Set %s= %d\n ", __func__ , newbase_hdmi_get_pcbinfo_name(group_id, entry_id), p_current_cfg_table[entry_id].default_value);
                result = 0;
                if(CTRL_SW_WRITE_ENABLE_FLAG == (CTRL_SW_WRITE_ENABLE_FLAG & p_current_cfg_table[entry_id].ctrl_config))
                {
                    unsigned int original_default_value = p_current_cfg_table[entry_id].default_value;
                    p_current_cfg_table[entry_id].default_value = set_value;
                    if(original_default_value != set_value)
                        p_current_cfg_table[entry_id].entry_status |= STATUS_SW_UPDATE_FLAG;

                }
                else
                {
                    HDMI_WARN("[%s] SW write permissiion denied ,  %s=%d!\n" , __func__ ,
                        GET_FLOW_CFG_NAME(group_id, entry_id), set_value);	
                }
            }
        }
    }
    return result;
}

unsigned int newbase_hdmi_get_flow_cfg_entry(HDMI_FLOW_CFG_GROUP group_id, unsigned char entry_id)
{
    unsigned int  result = 0xFFFF;
    if(group_id >=  HDMI_FLOW_CFG_GROUP_NUM)
    {
        HDMI_ERR("[%s] Wrong value , Read group_id %d fail!\n" , __func__ , group_id);
    }
    else
    {
        if(entry_id >= p_flow_cfg_group_table[group_id].cfg_table_length)
        {
            HDMI_ERR("[%s] Wrong value , Read entry_id %d fail!\n" , __func__ , entry_id);	
        }
        else
        {
            HDMI_CFG_ENTRY_T* p_current_cfg_table = p_flow_cfg_group_table[group_id].p_cfg_table;

            result = p_current_cfg_table[entry_id].default_value;
            //HDMI_PRINTF("[%s] Get %s= %d\n ", __func__ , GET_FLOW_CFG_NAME(group_id, entry_id), result);
        }
    }
    return result;
}

const char* newbase_hdmi_get_flow_cfg_name(HDMI_FLOW_CFG_GROUP group_id, unsigned char entry_id)
{
    if(group_id >=  HDMI_FLOW_CFG_GROUP_NUM)
    {
        HDMI_ERR("[%s] Wrong value , Read group_id %d fail!\n" , __func__ , group_id);
    }
    else
    {
        if(entry_id >= p_flow_cfg_group_table[group_id].cfg_table_length)
        {
            HDMI_ERR("[%s] Wrong value , Read entry_id %d fail!\n" , __func__ , entry_id);	
        }
        else
        {
            HDMI_CFG_ENTRY_T* p_current_cfg_table = p_flow_cfg_group_table[group_id].p_cfg_table;
            return p_current_cfg_table[entry_id].entry_name;
        }
    }
    return "ERROR";;
}

static unsigned int newbase_hdmi_pcbinfo_read(void)
{
    unsigned int check_result = 0;
    int i = 0;
    int j = 0;
    unsigned long long val = 0;
    HDMI_WARN("-------Read HDMI PCB info start------\n"  );

    for(i = 0; i < HDMI_FLOW_CFG_GROUP_NUM; i++)
    {
        val = 0;
        if ((pcb_mgr_get_enum_info_byname((char* )p_flow_cfg_group_table[i].pcb_enum_name, &val) == 0) &&
            ((val&1)==TRUE))
        {
            HDMI_CFG_ENTRY_T* p_current_cfg_table = p_flow_cfg_group_table[i].p_cfg_table;
            for(j = 0; j< p_flow_cfg_group_table[i].cfg_table_length; j++)
            {
                unsigned char start_bit = p_current_cfg_table[j].start_bit_index;
                unsigned char end_bit = p_current_cfg_table[j].end_bit_index;
                unsigned char bit_length = end_bit - start_bit +1;
                unsigned int bit_mask = (1<<bit_length)-1;
                unsigned int pcb_value =  (val>>start_bit)&bit_mask;

                if(CTRL_PCB_WRITE_ENABLE_FLAG == (p_current_cfg_table[j].ctrl_config & CTRL_PCB_WRITE_ENABLE_FLAG))
                {
                    if(pcb_value == HDMI_PCB_USE_DEFAULT)
                    {//Use deault value
                        HDMI_WARN("[%d] Read 0 from PCB , Use driver default value %s =%d\n", j,
                            GET_FLOW_CFG_NAME(i, j),
                            p_current_cfg_table[j].default_value);
                    }
                    else
                    {//Use PCB value
                        p_current_cfg_table[j].default_value = pcb_value;
                        p_current_cfg_table[j].entry_status |= STATUS_PCB_UPDATE_FLAG;
                        HDMI_WARN("[%d] Update from pcb info, %s =%d\n", j,
                            GET_FLOW_CFG_NAME(i, j),
                            pcb_value);
                    }
                }
                else
                {
                    HDMI_WARN("[%d] permission denied. Use default %s =%d\n", j, GET_FLOW_CFG_NAME(i, j), p_current_cfg_table[j].default_value);
                }
            }
        }
        else
        {
            HDMI_WARN("Read [%s] fail ,  not define!valid bit = %d\n" ,  p_flow_cfg_group_table[i].pcb_enum_name , (unsigned char)(val&1) );
            check_result |= (1<<i);
        }
    }
    HDMI_WARN(" -------Read HDMI PCB info End------\n" );

    return check_result;
}
/*---------------------------------------------------
 * Func : newbase_hdmi_flow_cfg_table_value_check
 *
 * Desc : read flow_cfg_table and check format.
 *
 * Para : nport : HDMI port number
 *
 * Retn : = 0 : no error, >0 : has error per bit.
 *--------------------------------------------------*/
unsigned int  newbase_hdmi_flow_cfg_table_default_value_check(HDMI_FLOW_CFG_GROUP group_id)
{
    unsigned int  check_result = 0;
    int i = 0;
    HDMI_CFG_ENTRY_T* p_current_cfg_table = p_flow_cfg_group_table[group_id].p_cfg_table;
    for(i = 0; i< p_flow_cfg_group_table[group_id].cfg_table_length ; i++)
    {
        unsigned int max_value = newbase_hdmi_calculate_entry_max_value(group_id, i);
        if(max_value ==0)
        {
            HDMI_WARN("invalid bit index !!group_id, entry_id)=(%d, %d), start_bit_index = %d, end_bit_index = %d \n", group_id, i,  p_current_cfg_table[i].start_bit_index, p_current_cfg_table[i].end_bit_index);
            check_result |= 1<<i;
            continue;
        }

        if(p_current_cfg_table->default_value > max_value)
        {
            HDMI_WARN("invalid default value make overflow!!defaule = %d, max = %d \n",  p_current_cfg_table[i].default_value, max_value);
            check_result |= 1<<i;
            continue;
        }
    }

    return check_result;
}

void newbase_hdmi_flow_cfg_info_init(void)
{
    int i = 0;
    HDMI_WARN("[newbase_hdmi_flow_cfg_info_init] HDMI Compatible Parameter Setting ---------------\n");

    for(i = 0; i < HDMI_FLOW_CFG_GROUP_NUM; i++)
    {
        unsigned int check_result = newbase_hdmi_flow_cfg_table_default_value_check(i);
        HDMI_WARN("check_result = %d\n", check_result);
    }

    if(newbase_hdmi_pcbinfo_read() !=0)
    {
        HDMI_WARN("[newbase_hdmi_flow_pcbinfo_init] cfg0_pcbinfo read fail, use code default!\n");
    }
	
    newbase_hdmi_set_hpd_low_delay(GET_FLOW_CFG(HDMI_FLOW_CFG_HPD, HDMI_FLOW_CFG1_DEF_HPD_LOW_10MS )*10);
    for(i = 0; i <HDMI_PORT_TOTAL_NUM; i++)
    {
        newbase_hdmi_set_str_hpd_low_ms(i,GET_FLOW_CFG(HDMI_FLOW_CFG_HPD, HDMI_FLOW_CFG1_DEF_STR0_HPD_10MS+i )*10);
    }


    for(i=0; i<g_panel_vfreq_hd21_pll_mapping_table_num; i++)
    {
        if(GET_PANEL_REFRESH_RATE() == g_panel_vfreq_hd21_pll_mapping_table[i].panel_max_v_freq)
        {// for 4k144 special setting
            SET_FLOW_CFG(HDMI_FLOW_CFG_GENERAL, HDMI_FLOW_CFG0_SUPPORT_DSC, HDMI_PCB_FORCE_ENABLE); //ENABLE DSC
            SET_FLOW_CFG(HDMI_FLOW_CFG_HDMI21_LT2, HDMI_FLOW_CFG3_HD21_SETUP_PLL_6G, g_panel_vfreq_hd21_pll_mapping_table[i].hd21_max_pll); //Set Video PLL Max to 680M
            HDMI_EMG("[newbase_hdmi_flow_cfg_info_init] detect special panel freq=%d, use hd21 PLL setting=%d\n", GET_PANEL_REFRESH_RATE(), g_panel_vfreq_hd21_pll_mapping_table[i].hd21_max_pll);
            break;
        }
    }
}

//////////////////////////////////////////////////////////////////////
// SOCTS config
//////////////////////////////////////////////////////////////////////

static void _hdmi_hack_flow_config_table(void)
{
    if (g_hdmi_drv_socts_en)
    {
        // For "SOCTS" or "Audio Only mode". If scaler video path is not be exectuted, don't need to check hdmi_display_ready.
        SET_FLOW_CFG(HDMI_FLOW_CFG_GENERAL, HDMI_FLOW_CFG0_ENABLE_HDMI_AV_SYNC, 0);
        HDMI_WARN("socts enabled, default disable HDMI AV function\n");
        // No need to run PS mode.
        SET_FLOW_CFG(HDMI_FLOW_CFG_DFE, HDMI_FLOW_CFG5_FW_LE_SCAN_ENABLE, 0);
        SET_FLOW_CFG(HDMI_FLOW_CFG_ERR_DET, HDMI_FLOW_CFG4_ERR_HANDLER_RESETPHY_EN, 0);
        SET_FLOW_CFG(HDMI_FLOW_CFG_ERR_DET, HDMI_FLOW_CFG4_ERR_HANDLER_HPD_EN, 0);
        SET_FLOW_CFG(HDMI_FLOW_CFG_GENERAL, HDMI_FLOW_CFG0_FAST_DETECT_ENABLE, HDMI_PCB_FORCE_ENABLE);
        // bypass connect & disconnect
        g_hdmi_bypass_connect_disconnect = 1;
    }
}

unsigned char newbase_hdmi_is_socts_mode(void)
{
    return g_hdmi_drv_socts_en;
}

int hdmi_vfe_config_systemd_unit_init(char *options)
{
    if (options!=NULL && strncmp(options, "socts.target", 12)==0)
        g_hdmi_drv_socts_en = 1;

    HDMI_WARN("systemd.unit=%s (socts_en=%d)\n", options, g_hdmi_drv_socts_en);

    _hdmi_hack_flow_config_table();
    return 1;
}

#ifdef MODULE
extern bool rtk_parse_commandline_equal(const char *string, char *output_string, int string_size);
void get_bootparam_systemd_unit(void)
{
    char para_buff[20]={'\0'};

    memset(para_buff,'\0',sizeof(para_buff));
    if(rtk_parse_commandline_equal("systemd.unit", para_buff,sizeof(para_buff)-1) == 0)
    {
        rtd_pr_hdmi_err("Error : can't get systemd.unit parameterr from bootargs\n");
        return;
    }
    hdmi_vfe_config_systemd_unit_init(para_buff);
}
#else
early_param("systemd.unit", hdmi_vfe_config_systemd_unit_init);
#endif