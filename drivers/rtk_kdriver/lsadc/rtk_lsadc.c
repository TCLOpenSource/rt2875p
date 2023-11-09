#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/miscdevice.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/ioport.h>
#include <linux/kfifo.h>
#include <linux/fs.h>
#include <linux/poll.h>
#include <linux/irq.h>
#include <linux/bitops.h>
#include <linux/ioctl.h>
#include <linux/signal.h>
#include <linux/interrupt.h>
#include <linux/jiffies.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/device.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/kthread.h>
#include <rbus/iso_misc_reg.h>
#include <rtk_kdriver/io.h>
#include <rtk_kdriver/rtk_crt.h>
#include <rtk_kdriver/rtk_lsadc.h>
#include "rtk_lsadc-attr.h"
#include "rtk_lsadc_reg.h"
#include "rtk_lsadc_reg_def.h"

static DEFINE_SPINLOCK(rtk_lsadc_irq_lock);
//static int main_log_ioctl=0;
unsigned int lsadc_init_status_mask = LSADC_INIT_MASK_NONE;
LSADC_PCBINFO _gLsadcInfo[MAX_LSADC_PCBINFO_COUNT];
int _gLsadcInfo_count=0;

static RTK_LSADC_IRQ rtk_lsadc_irq[MAX_LSADC_COUNT];
static LSADC_BLOCK_SOURCE rtk_lsadc_block_source[MAX_LSADC_INT_BLOCK_PIN] ;

#include <rtk_kdriver/io.h>
/* static void rtd_part_outl(unsigned int reg_addr, unsigned int endBit, unsigned int startBit, unsigned int value) */
/* { */
/*     unsigned int X,A,result; */
/*     X=(1<<(endBit-startBit+1))-1; */
/*     A=rtd_inl(reg_addr); */
/*     result = (A & (~(X<<startBit))) | (value<<startBit); */
/*     rtd_outl(reg_addr,result); */
/* } */

void LSADC_ClockEnable(unsigned char On)
{
    if (On) {
        CRT_CLK_OnOff(LSADC, CLK_ON, NULL);
    } else {
        CRT_CLK_OnOff(LSADC, CLK_OFF, NULL);
    }
}

int get_lsadc_init_status(unsigned int index)
{
    if(lsadc_init_status_mask & (0x1 << index))
        return 0;
    else
        return-1;
}
static RTK_LSADC_IRQ *_get_lsadc_entry(RTK_LSADC_ID index)
{
    if(index < MAX_LSADC_COUNT)
        return &rtk_lsadc_irq[index];
    else
        return NULL;
}

void clear_irq_status(RTK_LSADC_ID id) {
    if(id == 5) {
        rtd_part_outl(LSADC_PAD5_LEVEL_SET0_VADDR,0,0,0x1);         // pad5_set0[0]:clear int_pending_bit0
        rtd_part_outl(LSADC_PAD5_LEVEL_SET1_VADDR,0,0,0x1);         // pad5_set1[0]:clear int_pending_bit0
        rtd_part_outl(LSADC_PAD5_LEVEL_SET2_VADDR,0,0,0x1);         // pad5_set2[0]:clear int_pending_bit0
        rtd_part_outl(LSADC_PAD5_LEVEL_SET3_VADDR,0,0,0x1);         // pad5_set3[0]:clear int_pending_bit0
        rtd_part_outl(LSADC_PAD5_LEVEL_SET4_VADDR,0,0,0x1);         // pad5_set4[0]:clear int_pending_bit0
        rtd_part_outl(LSADC_PAD5_LEVEL_SET5_VADDR,0,0,0x1);         // pad5_set5[0]:clear int_pending_bit0
        rtd_outl(LSADC_INT_PAD5_VADDR, 0x0);
    }
    else if(id == 6) {
        rtd_part_outl(LSADC_PAD6_LEVEL_SET0_VADDR,0,0,0x1);         // pad6_set0[0]:clear int_pending_bit0
        rtd_part_outl(LSADC_PAD6_LEVEL_SET1_VADDR,0,0,0x1);         // pad6_set1[0]:clear int_pending_bit0
        rtd_part_outl(LSADC_PAD6_LEVEL_SET2_VADDR,0,0,0x1);         // pad6_set2[0]:clear int_pending_bit0
        rtd_part_outl(LSADC_PAD6_LEVEL_SET3_VADDR,0,0,0x1);         // pad6_set3[0]:clear int_pending_bit0
        rtd_part_outl(LSADC_PAD6_LEVEL_SET4_VADDR,0,0,0x1);         // pad6_set4[0]:clear int_pending_bit0
        rtd_part_outl(LSADC_PAD6_LEVEL_SET5_VADDR,0,0,0x1);         // pad6_set5[0]:clear int_pending_bit0
        rtd_outl(LSADC_INT_PAD6_VADDR, 0x0);
    }
    else if(id == 7) {
        rtd_part_outl(LSADC_PAD7_LEVEL_SET0_VADDR,0,0,0x1);         // pad7_set0[0]:clear int_pending_bit0
        rtd_part_outl(LSADC_PAD7_LEVEL_SET1_VADDR,0,0,0x1);         // pad7_set1[0]:clear int_pending_bit0
        rtd_part_outl(LSADC_PAD7_LEVEL_SET2_VADDR,0,0,0x1);         // pad7_set2[0]:clear int_pending_bit0
        rtd_part_outl(LSADC_PAD7_LEVEL_SET3_VADDR,0,0,0x1);         // pad7_set3[0]:clear int_pending_bit0
        rtd_part_outl(LSADC_PAD7_LEVEL_SET4_VADDR,0,0,0x1);         // pad7_set4[0]:clear int_pending_bit0
        rtd_part_outl(LSADC_PAD7_LEVEL_SET5_VADDR,0,0,0x1);         // pad7_set5[0]:clear int_pending_bit0
        rtd_outl(LSADC_INT_PAD7_VADDR, 0x0);
    }
}

static void rtk_init_block_source(void)
{
    rtk_lsadc_block_source[0].index = 5;
    rtk_lsadc_block_source[1].index = 6;
    rtk_lsadc_block_source[2].index = 7;
    rtk_lsadc_block_source[0].status = LSADC_NONE_USED;
    rtk_lsadc_block_source[1].status = LSADC_NONE_USED;
    rtk_lsadc_block_source[2].status = LSADC_NONE_USED;
    rtk_lsadc_block_source[0].int_status= ADC_NONE;
    rtk_lsadc_block_source[1].int_status = ADC_NONE;
    rtk_lsadc_block_source[2].int_status = ADC_NONE;
    if((lsadc_init_status_mask & LSADC_INIT_MASK_BIT_5) != 0){
        rtk_lsadc_block_source[0].int_status = ADC_BLOCK_MODE;
    }
    if((lsadc_init_status_mask & LSADC_INIT_MASK_BIT_6) != 0){
        rtk_lsadc_block_source[1].int_status = ADC_BLOCK_MODE;
    }
    if((lsadc_init_status_mask & LSADC_INIT_MASK_BIT_7) != 0){
        rtk_lsadc_block_source[2].int_status = ADC_BLOCK_MODE;
    }
}

static void lsadc_dump_irq_info(RTK_LSADC_IRQ_SET* irq_info)
{
    int i;
    LSADC_DEBUG("Dump irq setting info: pin_name = %s, irq_mode = %s, ",irq_info->pin_name,
        (irq_info->irq_mode == ADC_BLOCK_MODE)?"BLOCK MODE":"THREDHOLD MODE");
    if(irq_info->irq_mode == ADC_BLOCK_MODE){
        LSADC_DEBUG(" block_count = %d \n",irq_info->block_cnt);
        for(i = 0; i < irq_info->block_cnt; i++)
            LSADC_DEBUG(" block%d: low_bound= 0x%2x,top_bound = 0x%2x \n",i,
                irq_info->block[i].low_bound,irq_info->block[i].top_bound);
    }else
        LSADC_DEBUG(" thredHold = %d \n",irq_info->thredHold);
}
static int enable_holdThread_int(unsigned int index,unsigned char thredHold)
{
    st_lsadc_status_RBUS st_status_reg;

    st_status_reg.regValue = rtd_inl(lsadcArray[index].p_reg_map->status);
    st_status_reg.irq_en = 0x1 << index;
    rtd_outl(lsadcArray[index].p_reg_map->status,st_status_reg.regValue);
    if(thredHold != 0)
        rtk_lsadc_set_threshold(index,thredHold);
    return 0;
}

static int disable_holdThread_int(unsigned int index)
{
    st_lsadc_status_RBUS st_status_reg;

    st_status_reg.regValue = rtd_inl(lsadcArray[index].p_reg_map->status);
    st_status_reg.irq_en &= ~(0x1 << index);
    rtd_outl(lsadcArray[index].p_reg_map->status,st_status_reg.regValue);
    return 0;
}


static int enable_block_int(unsigned int index,LSADC_BLOCK *block,unsigned char cnt)
{
    lsadc_lsadc_pad1_level_set0_RBUS st_block_set_reg;
    unsigned char i,j;
    int block_souce_index;
    block_souce_index = -1;
    rtd_part_outl(ST_PAD6_VADDR,31,31,0x1);         // ST_pad6-> set Pad6_active for Mac6p
    if(index < 5 || index > 7){
        for(i = 0;i < MAX_LSADC_INT_BLOCK_PIN; i++){
            if(rtk_lsadc_block_source[i].status == LSADC_NONE_USED){
                 /*store block info to block soure table*/
                rtk_lsadc_block_source[i].status = USED_AS_BLOCK_SOURCE;
                rtk_lsadc_block_source[i].int_status = ADC_BLOCK_MODE;
                rtk_lsadc_block_source[i].pad_switch = index;
                rtk_lsadc_block_source[i].index = index;
                rtk_lsadc_block_source[i].block_cnt = cnt;
                for(j=0;j<cnt;j++){
                    rtk_lsadc_block_source[i].block[j].low_bound = block[j].low_bound;
                    rtk_lsadc_block_source[i].block[j].top_bound = block[j].top_bound;
                }
                block_souce_index = rtk_lsadc_block_source[i].index;
                break;
            }
        }
        rtk_lsadc_set_active(block_souce_index,1);
        rtk_lsadc_set_padSwitch(block_souce_index,index);
        rtk_lsadc_set_voltageMode(block_souce_index,1);
    }else{
        rtk_lsadc_block_source[index-5].status = USED_AS_BLOCK_SOURCE;
        rtk_lsadc_block_source[index-5].int_status = ADC_BLOCK_MODE;
        rtk_lsadc_block_source[index-5].pad_switch = index;
        rtk_lsadc_block_source[index-5].index = index;
        rtk_lsadc_block_source[index-5].block_cnt = cnt;
        for(j=0;j<cnt;j++){
            rtk_lsadc_block_source[index-5].block[j].low_bound = block[j].low_bound;
            rtk_lsadc_block_source[index-5].block[j].top_bound = block[j].top_bound;
        }
        block_souce_index = rtk_lsadc_block_source[index-5].index;
    }
    if(block_souce_index < 0){
        LSADC_ERROR("%s lsadc(%d) has no block interrupt source to use!!!",__FUNCTION__,index);
        return -1;
    }
    LSADC_DEBUG("%s index =%d, block_source_index = %d \n",__FUNCTION__,index,block_souce_index);
    for(i = 0; i < cnt; i ++){
        st_block_set_reg.regValue = rtd_inl(lsadcArray[block_souce_index].p_reg_map->padCompare0 + i*4);
        st_block_set_reg.int_en0 = 1;
        st_block_set_reg.block0en = 1;
        st_block_set_reg.level_0_start_val= block[i].low_bound;
        st_block_set_reg.level_0_end_val = block[i].top_bound;
        rtd_outl(lsadcArray[block_souce_index].p_reg_map->padCompare0 + i*4,st_block_set_reg.regValue);
    }
    return 0;
}

static int disable_block_int(unsigned int index)
{
    lsadc_lsadc_pad1_level_set0_RBUS st_block_set_reg;
    unsigned char i,j;
    int block_souce_index;
    block_souce_index = -1;
    if(index < 5 || index > 7){
        for(i = 0;i < MAX_LSADC_INT_BLOCK_PIN; i++){
            if(rtk_lsadc_block_source[i].pad_switch == index){
                 /*delete block info to block soure table*/
                rtk_lsadc_block_source[i].status = LSADC_NONE_USED;
                rtk_lsadc_block_source[i].int_status = ADC_NONE;
                rtk_lsadc_block_source[i].pad_switch = rtk_lsadc_block_source[i].index;
                for(j=0;j<rtk_lsadc_block_source[i].block_cnt;j++){
                    rtk_lsadc_block_source[i].block[j].low_bound = 0;
                    rtk_lsadc_block_source[i].block[j].top_bound = 0;
                }
                rtk_lsadc_block_source[i].block_cnt = 0;
                block_souce_index = rtk_lsadc_block_source[i].index;
                break;
            }
        }
    }else{
        rtk_lsadc_block_source[index-5].int_status = ADC_NONE;
        rtk_lsadc_block_source[index-5].pad_switch = rtk_lsadc_block_source[index-5].index;
        for(j=0;j<rtk_lsadc_block_source[index-5].block_cnt;j++){
            rtk_lsadc_block_source[index-5].block[j].low_bound = 0;
            rtk_lsadc_block_source[index-5].block[j].top_bound = 0;
        }
        rtk_lsadc_block_source[index-5].block_cnt = 0;

        block_souce_index = rtk_lsadc_block_source[index-5].index;
    }

    LSADC_DEBUG("%s index =%d, block_source_index = %d \n",__FUNCTION__,index,block_souce_index);

    if(block_souce_index < 0){
        LSADC_ERROR("%s lsadc(%d) has no block interrupt source to use!!!",__FUNCTION__,index);
        return -1;
    }
    rtk_lsadc_set_active(block_souce_index,0);

    for(i = 0; i < MAX_LSADC_INT_BLOCK; i ++){
        st_block_set_reg.regValue = rtd_inl(lsadcArray[block_souce_index].p_reg_map->padCompare0 + i*4);
        st_block_set_reg.int_en0 = 0;
        st_block_set_reg.block0en = 0;
        st_block_set_reg.level_0_start_val = 0;
        st_block_set_reg.level_0_end_val  = 0;
        rtd_outl(lsadcArray[block_souce_index].p_reg_map->padCompare0 + i*4,st_block_set_reg.regValue);
    }
    return 0;
}

void rtk_lsadc_isr(RTK_LSADC_ID id, unsigned char assert, void *dev_id)
{
   // For custom do something
}

void rtk_lsadc_interrupt_init(LSADC_PCBINFO pcb_info, unsigned int en)
{
    int ret=0;
    RTK_LSADC_IRQ_SET irq_set = {0};
    RTK_LSADC_ID index = {0};
    memset(irq_set.pin_name, 0, sizeof(irq_set.pin_name));
    strncpy(irq_set.pin_name,pcb_info.pinName,sizeof(irq_set.pin_name) - 1);
    if(pcb_info.irq_mode == ADC_THREDHOLD_MODE){
        irq_set.block_cnt = 0;
        irq_set.irq_mode = ADC_THREDHOLD_MODE;
        irq_set.thredHold = pcb_info.irq_thredHold;
    }else{
        irq_set.irq_mode = ADC_BLOCK_MODE;
        irq_set.block_cnt = 1;
        irq_set.block[0].top_bound = pcb_info.hitValue + pcb_info.torrence;
        irq_set.block[0].low_bound = pcb_info.hitValue - pcb_info.torrence;
    }
    if(en){
        ret = rtk_lsadc_enable_int(irq_set,1);

        LSADC_INFO("rtk_lsadc_interrupt_init enabel %d  %s \n",__LINE__,irq_set.pin_name);
        if(ret !=0)
            LSADC_WARN("%d enable interrupt error!!!\n",__LINE__);
        if(rtk_lsadc_get_index_byName(irq_set.pin_name,&index) != 0){
            LSADC_ERROR("%s Can not get inde by pin name \n",__FUNCTION__);
            return;
        }
        ret = rtk_lsadc_request_irq(index,rtk_lsadc_isr,"rtk_lsadc",rtk_lsadc_isr);
        if(ret !=0)
            LSADC_WARN("%d request interrupt error!!!\n",__LINE__);
    }else{
        ret = rtk_lsadc_enable_int(irq_set,0);
        if(ret !=0)
            LSADC_WARN("%d enable interrupt error!!!\n",__LINE__);
    }
}

int rtk_lsadc_dump_all_pininfo(RTK_ALL_LSADC_PIN_T* pcbInfo)
{
    int i;
    RTK_LSADC_INFO lsadc_info;
    for(i = 0; i < _gLsadcInfo_count; i ++){
        memcpy(pcbInfo->pcbInfo[i].pin_name, _gLsadcInfo[i].pinName,sizeof(pcbInfo->pcbInfo[i].pin_name));
        memset(&lsadc_info,0,sizeof(RTK_LSADC_INFO));
        lsadc_info.pinNumber = _gLsadcInfo[i].index;
        rtk_lsadc_get_lsadc_config(&lsadc_info);
        memcpy(&(pcbInfo->pcbInfo[i].info), &lsadc_info,sizeof(RTK_LSADC_INFO));
    }
    pcbInfo->count = _gLsadcInfo_count;
    return 0;
}

void rtk_lsadc_init_pcb_pin(void){
    int i;

    for(i = 0; i < PCB_ENUM_MAX; i++){
        if((memcmp(pcb_enum_all[i].name, "PIN", 3) == 0) && (pcb_enum_all[i].value != 0) &&( strstr(pcb_enum_all[i].name, "GPIO_INFO") == NULL))
        {
            if(GET_PIN_TYPE(pcb_enum_all[i].value) == PCB_PIN_TYPE_LSADC)
            {
                memset(_gLsadcInfo[_gLsadcInfo_count].pinName, 0, sizeof(_gLsadcInfo[_gLsadcInfo_count].pinName));
                strncpy(_gLsadcInfo[_gLsadcInfo_count].pinName, pcb_enum_all[i].name, sizeof(_gLsadcInfo[_gLsadcInfo_count].pinName) - 1);
                _gLsadcInfo[_gLsadcInfo_count].ullPcbMgrValue = pcb_enum_all[i].value;
                _gLsadcInfo[_gLsadcInfo_count].index = GET_PIN_INDEX(pcb_enum_all[i].value);
                _gLsadcInfo[_gLsadcInfo_count].hitValue = GET_PIN_PARAM2(pcb_enum_all[i].value);
                _gLsadcInfo[_gLsadcInfo_count].torrence = GET_PIN_PARAM3(pcb_enum_all[i].value);
                _gLsadcInfo[_gLsadcInfo_count].irq_mode = GET_PIN_PARAM4(pcb_enum_all[i].value);
                _gLsadcInfo[_gLsadcInfo_count].irq_thredHold = GET_PIN_PARAM6(pcb_enum_all[i].value);
                _gLsadcInfo_count ++;
            }
        }
    }
}

int rtk_hw_setting_adc_init(void)
{
    int i;
    LSADC_ClockEnable(1);
    rtk_lsadc_init_pcb_pin();
    if(0 == _gLsadcInfo_count){
        return -1;
    }
    for(i = 0;i < _gLsadcInfo_count; i++)
    {
        if(0 != get_lsadc_init_status(_gLsadcInfo[i].index)){
            rtk_lsadc_defualt_enable(_gLsadcInfo[i].index,VOLTAGE_MODE);
            lsadc_init_status_mask = lsadc_init_status_mask | (0x1 << _gLsadcInfo[i].index);
            if(_gLsadcInfo[i].irq_mode != ADC_NONE)
                rtk_lsadc_interrupt_init(_gLsadcInfo[i],1);

        }
    }
    rtk_init_block_source();
    return 0;
}

int rtk_lsadc_get_index_byName(char* pinName,unsigned int *val)
{
    int i = 0;
    for(i = 0; i < _gLsadcInfo_count; i++) {
        LSADC_DEBUG("%s:_gLsadcInfo.pinName = %s pinName =%s,_gLsadcInfo.index = %d\n",
                __FUNCTION__,_gLsadcInfo[i].pinName ,pinName,_gLsadcInfo[i].index);
        if(strcmp(_gLsadcInfo[i].pinName, pinName) == 0 ) {
            *val = _gLsadcInfo[i].index;
            return 0;
        }
    }
    return -1;
}

int rtk_lsadc_get_adcValue_byName(RTK_LSADC_NAME name, unsigned char* value)
{
    st_pad0_RBUS st_pad0_reg;
    RTK_LSADC_ID index;

    if(rtk_lsadc_get_index_byName(name,&index) != 0){
        LSADC_ERROR("Invalid pin name \n");
        return -1;
    }

    st_pad0_reg.regValue = rtd_inl(lsadcArray[index].p_reg_map->padCtrl);
    *value = st_pad0_reg.adc_val0;

    return 0;
}

EXPORT_SYMBOL(rtk_lsadc_get_adcValue_byName);


int rtk_lsadc_get_adcValue(unsigned int index, unsigned char* value)
{
    st_pad0_RBUS st_pad0_reg;

    if(index < 0 || index >= MAX_LSADC_COUNT || value == NULL){
        LSADC_ERROR("Invalid parameter \n");
        return -1;
    }

    st_pad0_reg.regValue = rtd_inl(lsadcArray[index].p_reg_map->padCtrl);
    *value = st_pad0_reg.adc_val0;

    return 0;
}

EXPORT_SYMBOL(rtk_lsadc_get_adcValue);

int rtk_lsadc_get_threshold(unsigned int index, unsigned char* threshold)
{
    st_pad0_RBUS st_pad0_reg;

    if(index < 0 || index >= MAX_LSADC_COUNT || threshold == NULL){
        LSADC_ERROR("Invalid parameter \n");
        return -1;
    }

    st_pad0_reg.regValue = rtd_inl(lsadcArray[index].p_reg_map->padCtrl);
    *threshold = st_pad0_reg.pad0_thred;

    return 0;
}

EXPORT_SYMBOL(rtk_lsadc_get_threshold);

int rtk_lsadc_get_activeStatus(unsigned int index, unsigned char* activeStatus)
{
    st_pad0_RBUS st_pad0_reg;

    if(index < 0 || index >= MAX_LSADC_COUNT || activeStatus == NULL){
        LSADC_ERROR("Invalid parameter \n");
        return -1;
    }

    st_pad0_reg.regValue = rtd_inl(lsadcArray[index].p_reg_map->padCtrl);
    *activeStatus = st_pad0_reg.pad0_active;

    return 0;

}

EXPORT_SYMBOL(rtk_lsadc_get_activeStatus);

int rtk_lsadc_get_padSwitch(unsigned int index, unsigned char* padSwitch)
{
    st_pad0_RBUS st_pad0_reg;

    if(index < 0 || index >= MAX_LSADC_COUNT || padSwitch == NULL){
        LSADC_ERROR("Invalid parameter \n");
        return -1;
    }

    st_pad0_reg.regValue = rtd_inl(lsadcArray[index].p_reg_map->padCtrl);
    *padSwitch = st_pad0_reg.pad0_sw;

    return 0;

}

EXPORT_SYMBOL(rtk_lsadc_get_padSwitch);

int rtk_lsadc_get_voltageMode(unsigned int index, unsigned char* voltageMode)
{
    st_pad0_RBUS st_pad0_reg;

    if(index < 0 || index >= MAX_LSADC_COUNT || voltageMode == NULL){
        LSADC_ERROR("Invalid parameter \n");
        return -1;
    }

    st_pad0_reg.regValue = rtd_inl(lsadcArray[index].p_reg_map->padCtrl);
    *voltageMode = (st_pad0_reg.pad0_ctrl == 0) ? 1 : 0;

    return 0;

}

EXPORT_SYMBOL(rtk_lsadc_get_voltageMode);

int rtk_lsadc_set_threshold(unsigned int index, unsigned char threshold)
{
    st_pad0_RBUS st_pad0_reg;

    if((index < 0 )|| (index >= MAX_LSADC_COUNT ))
    {
        LSADC_ERROR("Invalid parameter \n");
        return -1;
    }

    st_pad0_reg.regValue = rtd_inl(lsadcArray[index].p_reg_map->padCtrl);
    st_pad0_reg.pad0_thred = threshold;
    rtd_outl(lsadcArray[index].p_reg_map->padCtrl,st_pad0_reg.regValue);
    return 0;

}

EXPORT_SYMBOL(rtk_lsadc_set_threshold);

int rtk_lsadc_set_active(unsigned int index, unsigned char activeStatus)
{
    st_pad0_RBUS st_pad0_reg;

    if(index < 0 || index >= MAX_LSADC_COUNT){
        LSADC_ERROR("Invalid parameter \n");
        return -1;
    }

    st_pad0_reg.regValue = rtd_inl(lsadcArray[index].p_reg_map->padCtrl);
    st_pad0_reg.pad0_active = (activeStatus == 0)? 0 : 1;
    rtd_outl(lsadcArray[index].p_reg_map->padCtrl,st_pad0_reg.regValue);
    return 0;

}

EXPORT_SYMBOL(rtk_lsadc_set_active);

int rtk_lsadc_set_padSwitch(unsigned int index, unsigned char padSwitch)
{
    st_pad0_RBUS st_pad0_reg;

    if(index < 0 || index >= MAX_LSADC_COUNT || padSwitch >= MAX_LSADC_COUNT){
        LSADC_ERROR("Invalid parameter \n");
        return -1;
    }


    st_pad0_reg.regValue = rtd_inl(lsadcArray[index].p_reg_map->padCtrl);
    st_pad0_reg.pad0_sw = padSwitch;
    rtd_outl(lsadcArray[index].p_reg_map->padCtrl,st_pad0_reg.regValue);

    return 0;

}

EXPORT_SYMBOL(rtk_lsadc_set_padSwitch);

int rtk_lsadc_set_voltageMode(unsigned int index, unsigned char voltageMode)
{
    st_pad0_RBUS st_pad0_reg;

    if(index < 0 || index >= MAX_LSADC_COUNT){
        LSADC_ERROR("Invalid parameter \n");
        return -1;
    }

    st_pad0_reg.regValue = rtd_inl(lsadcArray[index].p_reg_map->padCtrl);
    st_pad0_reg.pad0_ctrl = (voltageMode == 1)? 0 : 1;
    rtd_outl(lsadcArray[index].p_reg_map->padCtrl,st_pad0_reg.regValue);

    return 0;

}

EXPORT_SYMBOL(rtk_lsadc_set_voltageMode);

int rtk_lsadc_set_adcValue(unsigned int index, unsigned char adcValue)
{
    st_pad0_RBUS st_pad0_reg;

    if(index >= MAX_LSADC_COUNT){
        LSADC_ERROR("Invalid parameter \n");
        return -1;
    }

    st_pad0_reg.regValue = rtd_inl(lsadcArray[index].p_reg_map->padCtrl);
    st_pad0_reg.adc_val0 = adcValue;
    rtd_outl(lsadcArray[index].p_reg_map->padCtrl,st_pad0_reg.regValue);

    return 0;

}

EXPORT_SYMBOL(rtk_lsadc_set_adcValue);

int rtk_lsadc_enable_int(RTK_LSADC_IRQ_SET irq_info,unsigned char enable)
{
    unsigned int index;
    unsigned char value = 0;
    int ret = -1;
    RTK_LSADC_IRQ *p_irq = NULL;

    if(rtk_lsadc_get_index_byName(irq_info.pin_name,&index) != 0){
        LSADC_ERROR("%s Can not get inde by pin name \n",__FUNCTION__);
        return ret;
    }
    if(enable){
        lsadc_dump_irq_info(&irq_info);
        if((irq_info.irq_mode == ADC_BLOCK_MODE) && irq_info.block_cnt == 0){
            LSADC_ERROR("%s Invalid parameter : Set block mode, but block cnt = 0.\n",__FUNCTION__);
            return ret;
        }
        if((irq_info.irq_mode == ADC_BLOCK_MODE) && irq_info.block[0].top_bound == 0){
            LSADC_ERROR("%s Invalid parameter : Set block mode, but block[0].top_bound = 0.\n",__FUNCTION__);
            return ret;
        }
        if((irq_info.irq_mode == ADC_THREDHOLD_MODE) && (irq_info.thredHold ==0)){
            LSADC_WARN("%s Set thredHold mode, but thredHold = 0, used the default value from init.\n",__FUNCTION__);
        }
        if(irq_info.irq_mode == ADC_BLOCK_MODE)
            ret = enable_block_int(index,irq_info.block,irq_info.block_cnt);
        else {
            ret = enable_holdThread_int(index, irq_info.thredHold);
            mdelay(5);
            rtk_lsadc_get_adcValue(index, &value);
            LSADC_WARN("%s rtk_lsadc_get_adcValue = %u\n",__FUNCTION__, value);
            rtd_outl(ST_LSADC_STATUS_VADDR, rtd_inl(ST_LSADC_STATUS_VADDR) | LSADC_INT_THREHOLD_STATUS_MASK);
        }
        p_irq = _get_lsadc_entry(index);
        if (NULL == p_irq)
        {
            LSADC_ERROR("%s line:%d _get_lsadc_entry error\n", __func__, __LINE__);
            return ret;
        }
        p_irq->irq_info.block_cnt = irq_info.block_cnt;
        p_irq->irq_info.irq_mode = irq_info.irq_mode;
        p_irq->irq_info.thredHold = irq_info.thredHold;
        strncpy(p_irq->irq_info.pin_name,irq_info.pin_name,sizeof(p_irq->irq_info.pin_name));
        memcpy(p_irq->irq_info.block,irq_info.block,(irq_info.block_cnt) *sizeof(LSADC_BLOCK));
    }else{
        if(irq_info.irq_mode == ADC_BLOCK_MODE)
            ret = disable_block_int(index);
        else
            ret = disable_holdThread_int(index);
    }
    return ret;
}
EXPORT_SYMBOL(rtk_lsadc_enable_int);

int rtk_lsadc_request_irq(RTK_LSADC_ID index,
                 void (*handler) (RTK_LSADC_ID id,
                         unsigned char assert, void *dev_id),
                 char *int_name,void *dev_id)
{
    unsigned long flags;
    RTK_LSADC_IRQ *p_irq = NULL;

    if (int_name == NULL || handler == NULL || dev_id == NULL)
    {
        LSADC_ERROR("%s failed - in valid argument\n",__FUNCTION__);
        return -1;
    }

    p_irq = _get_lsadc_entry(index);

    if (p_irq == NULL){
        LSADC_ERROR("%s lsadc(%d) does not exist\n",__FUNCTION__,index);
        return -1;
    }

    spin_lock_irqsave(&rtk_lsadc_irq_lock, flags);

    if (p_irq->handler){
        LSADC_ERROR("%s failed - lsadc(%d) is occupied by [%s]\n",
             __FUNCTION__, index, p_irq->name);
        spin_unlock_irqrestore(&rtk_lsadc_irq_lock, flags);
        return -1;
    }

    strncpy(p_irq->name, int_name,sizeof(p_irq->name)-1);
    p_irq->index = index;
    p_irq->dev_id = dev_id;
    p_irq->handler = handler;
    p_irq->irq_flag = 1;

    spin_unlock_irqrestore(&rtk_lsadc_irq_lock, flags);

    LSADC_INFO("%s - LSADC (%d) irq requested by (%s)\n",
         __FUNCTION__,index, p_irq->name);

    return 0;

}
EXPORT_SYMBOL(rtk_lsadc_request_irq);

void rtk_lsadc_free_irq(RTK_LSADC_ID index, void *dev_id)
{
    RTK_LSADC_IRQ *p_irq = NULL;
    unsigned long flags;

    p_irq = _get_lsadc_entry(index);

    if (p_irq)
    {
        spin_lock_irqsave(&rtk_lsadc_irq_lock, flags);

        if (p_irq->dev_id == dev_id)
        {
            memset(p_irq, 0, sizeof(RTK_LSADC_IRQ));
            LSADC_INFO(" %s LSADC %d irq released\n",__FUNCTION__, index);
        }

        spin_unlock_irqrestore(&rtk_lsadc_irq_lock, flags);
    }
    else
    {
        LSADC_INFO(" %s LSADC %d irq does not exists \n",__FUNCTION__,index);
    }
}

EXPORT_SYMBOL(rtk_lsadc_free_irq);

static void _do_rtk_lsadc_block_irq(RTK_LSADC_ID id,unsigned int regValue,RTK_LSADC_IRQ *p_irq)
{
    int i;
    if((regValue & LSADC_INT_BLOCKSTATUS_MASK ) != 0){
        clear_irq_status(id);
        for(i = 0; i < MAX_LSADC_INT_BLOCK; i++){
            //block interrupt need to check block source table pad switch mapping
            //lsadc5 block interrupt soure maybe means lsadc1 pin interrupt trigger
            p_irq = _get_lsadc_entry(rtk_lsadc_block_source[id-5].pad_switch);
            if((regValue >> i) & 0x1){
                if(p_irq){
                    lsadc_dump_irq_info(&(p_irq->irq_info));
                    if (p_irq->handler){
                        if(p_irq->irq_info.irq_mode == ADC_BLOCK_MODE) {
                            p_irq->handler(id, 0x1 << i, p_irq ->dev_id);
                        }
                    }
                }
            }
        }
    }
}

irqreturn_t rtk_lsadc_interrupt_handler(int irq, void *dev_id)
{
    RTK_LSADC_IRQ *p_irq = NULL;
    unsigned long flags;
    int i = 0;
    st_lsadc_status_RBUS lsadc_status_reg;

    if((rtd_inl(ISO_MISC_ISR_reg) & ISO_MISC_ISR_lsadc_int_mask) == 0)
        return IRQ_NONE;

    lsadc_status_reg.regValue = rtd_inl(ST_LSADC_STATUS_VADDR);
    if((lsadc_status_reg.regValue & LSADC_INT_THREHOLD_STATUS_MASK) == 0)
        return IRQ_NONE;

    rtd_pr_lsadc_warn("rtk_lsadc_interrupt_handler : irq_info TEST\n");
    if((lsadc_status_reg.regValue & LSADC_INT_THREHOLD_STATUS_MASK) !=0){
        /*clear interrupt status*/
        rtd_outl(ST_LSADC_STATUS_VADDR,lsadc_status_reg.regValue);
        /*handle interrupt status*/
        for(i =0; i < MAX_LSADC_COUNT;i++){
            if((lsadc_status_reg.regValue >> i) & 0x1){
                p_irq = _get_lsadc_entry(i);
                if (p_irq){
                    spin_lock_irqsave(&rtk_lsadc_irq_lock, flags);
                    if (p_irq->handler){
                        if(p_irq->irq_info.irq_mode == ADC_THREDHOLD_MODE) {
                             p_irq->handler(i, 0x1 << 7, p_irq->dev_id);
                             LSADC_DEBUG("handler %d access thredHold mode!!!\n",__LINE__);
                        }
                        else if((p_irq->irq_info.irq_mode == ADC_BLOCK_MODE) && (i >= 5) && (i <= 7)) {
                            _do_rtk_lsadc_block_irq(i, rtd_inl(LSADC_INT_PAD5_VADDR + (i-5)*4), p_irq);
                            LSADC_DEBUG("handler %d access block mode!!!\n",__LINE__);
                        }
                    }
                    spin_unlock_irqrestore(&rtk_lsadc_irq_lock, flags);
                     LSADC_DEBUG("handler %d access thredHold mode!!!\n",__LINE__);
                }
            }
        }
    }

   return IRQ_HANDLED;
}

int rtk_lsadc_set_lsadc_config(RTK_LSADC_INFO  *lsadc_info)
{
    int ret   = -1;
    st_pad0_RBUS st_pad0_reg;
    st_lsadc_ctrl_RBUS st_lsadc_ctrl_reg;

    if(lsadc_info == NULL)
        return ret;

    if(lsadc_info->pinNumber >= 0) {
        st_pad0_reg.regValue = 0;
        st_pad0_reg.pad0_active = lsadc_info->pad_active;
        st_pad0_reg.pad0_thred = lsadc_info->pad_thred;
        st_pad0_reg.pad0_sw = lsadc_info->pad_sw;
        st_pad0_reg.pad0_ctrl = lsadc_info->pad_ctrl;
        rtd_outl(lsadcArray[lsadc_info->pinNumber].p_reg_map->padCtrl, st_pad0_reg.regValue);

        st_lsadc_ctrl_reg.regValue = rtd_inl(lsadcArray[lsadc_info->pinNumber].p_reg_map->lsadc_ctrl);
        if(!st_lsadc_ctrl_reg.enable && lsadc_info->pad_active) {
            st_lsadc_analog_ctrl_RBUS st_ana_ctrl_reg;
            st_ana_ctrl_reg.regValue = rtd_inl(lsadcArray[lsadc_info->pinNumber].p_reg_map->analog_ctrl);
            st_ana_ctrl_reg.jd_power = 0x1;

            rtd_outl(lsadcArray[lsadc_info->pinNumber].p_reg_map->analog_ctrl, st_ana_ctrl_reg.regValue);

            st_lsadc_ctrl_reg.sel_wait = 0x1;
            st_lsadc_ctrl_reg.sel_adc_ck = 0x6;
            st_lsadc_ctrl_reg.debounce_cnt = 0x1;
            st_lsadc_ctrl_reg.enable = 1;
            rtd_outl(lsadcArray[lsadc_info->pinNumber].p_reg_map->lsadc_ctrl, st_lsadc_ctrl_reg.regValue);
        }
        ret = 0;
    }
    return ret;
}

int rtk_lsadc_get_lsadc_config(RTK_LSADC_INFO  *lsadc_info)
{
    st_pad0_RBUS st_pad0_reg;

    int ret   = -1;

    if(lsadc_info == NULL)
        return ret;

    if(lsadc_info->pinNumber >= 0) {

        st_pad0_reg.regValue = rtd_inl(lsadcArray[lsadc_info->pinNumber].p_reg_map->padCtrl);
        lsadc_info->pad_active = st_pad0_reg.pad0_active;
        lsadc_info->pad_thred = st_pad0_reg.pad0_thred;
        lsadc_info->pad_sw = st_pad0_reg.pad0_sw;
        lsadc_info->pad_ctrl = st_pad0_reg.pad0_ctrl;
        lsadc_info->adc_val = st_pad0_reg.adc_val0;
        ret = 0;
    }
    return ret;
}

int rtk_lsadc_dump_mainAnalogCtrl(void)
{
    st_lsadc_analog_ctrl_RBUS anaglog;
    anaglog.regValue = rtd_inl(ST_LSADC_ANALOG_CTRL_VADDR);
    return anaglog.regValue;
}
int rtk_lsadc_dump_mainLsadcCtrl(void)
{
    st_lsadc_ctrl_RBUS ctrl;
    ctrl.regValue = rtd_inl(ST_LSADC_CTRL_VADDR);
    return ctrl.regValue;
}


/*
 * Add lsadc API for kernel call
 */

int rtk_lsadc_defualt_enable(int index, LSADC_MODE lsadc_mode)
{
    RTK_LSADC_INFO lsadc_info;
    if (index >= MAX_LSADC_COUNT || index < 0)
        return -1;
    
    memset(&lsadc_info, 0, sizeof(lsadc_info));
    lsadc_info.pinNumber = index;
    lsadc_info.pad_active = 1;
    lsadc_info.pad_sw = index;
    lsadc_info.pad_ctrl = lsadc_mode;
    return rtk_lsadc_set_lsadc_config(&lsadc_info);
}

EXPORT_SYMBOL(rtk_lsadc_defualt_enable);


