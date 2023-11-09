#include <linux/ctype.h>
#include "rtk_cec_hw.h"
#include "rtk_cec_sysfs.h"
#include "rtk_cec_msg.h"
#include "rtk_cec_defines.h"

#define cec_attr(_name) \
    static struct kobj_attribute _name##_attr = {    \
        .attr   = {                                  \
                  .name = __stringify(_name),        \
                  .mode = 0644,                      \
                  },                                 \
        .show   = _name##_show,                      \
        .store  = _name##_store,                     \
    }

static struct kobject *cec_kobj;
unsigned int g_cec_debug_enable = 2; // 0: close all, 1: open all, 2: smart parsing
unsigned int g_cec_send_enable = 1; // 0: close, 1: open

static int hexRmSpace2bin(u8 *dst, const char *src, size_t count)
{
    int len = 0;
    while (count--) {
        int hi, lo;
        if ((*src) == ' ') {
            src ++;
            continue;
        }
        hi = hex_to_bin(*src++);
        lo = hex_to_bin(*src++);
        if ((hi < 0) || (lo < 0))
            return len;
        *dst++ = (hi << 4) | lo;
        len ++;
    }
    return len;
}

static ssize_t cec_dbg_en_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    int  n;
    int count = PAGE_SIZE;
    char *ptr = buf;

    if (strcmp(attr->attr.name, "cec_dbg_en")==0)
    {
        n = snprintf(ptr, count, "g_cec_debug_enable=%s\n", g_cec_debug_enable ? "enable" : "disable");
        ptr+=n; count-=n;

        n = snprintf(ptr, count, "g_cec_send_enable=%s\n", g_cec_send_enable ? "enable" : "disable");
        ptr+=n; count-=n;

    }

    return ptr - buf;
}

static ssize_t cec_dbg_en_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t n)
{
    unsigned int val = 0;
    if (strcmp(attr->attr.name, "cec_dbg_en")==0)
    {
        if (sscanf(buf, "g_cec_debug_enable=%5d", &val)==1)
        {
            g_cec_debug_enable = val;
        }
        else if (sscanf(buf, "g_cec_send_enable=%5d", &val)==1)
        {
            if(val == 1 || val == 0) 
                g_cec_send_enable = val;
        }

        else
        {
            cec_dbg("Invaild parameter!!\n");
            return -EINVAL;
        }
    }

    return n;
}
cec_attr(cec_dbg_en);


static ssize_t cec_dump_current_setting_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    rtk_cec_dump_current_setting(buf, 0);
    return strlen(buf);
}

static ssize_t cec_dump_current_setting_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf,
        size_t n)
{
    return -EINVAL;
}
cec_attr(cec_dump_current_setting);


static ssize_t cec_msg_out_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    return snprintf(buf, PAGE_SIZE, "echo <device LA> <op code> <data> > cec_msg_out\nex. echo 05 83\n");;
}

static ssize_t cec_msg_out_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t n)
{
    struct cec_msg msg;
    int len;
    memset(&msg, 0, sizeof(struct cec_msg));
    len = hexRmSpace2bin(msg.msg, buf, n);
    if (len < 0)
        return -EINVAL;
    msg.len = len;
    rtk_cec_sysfs_msg_out(&msg);
    return n;
}
cec_attr(cec_msg_out);

static ssize_t cec_print_en_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    int  n;
    int count = PAGE_SIZE;
    char *ptr = buf;

    if (strcmp(attr->attr.name, "cec_print_en")==0)
    {
        n = scnprintf(ptr, count, "\nCEC_POLLING=%d,                                <-1>\n", 
            cec_msg_get_print_enable(-1));
        ptr+=n; count-=n;

        //---------------------------------------------------
        // Message Descriptions for the One Touch Play Feature 
        //---------------------------------------------------

        n = scnprintf(ptr, count, "CEC_ACTIVE_SOURCE=%d,                        <0x82>\n", 
            cec_msg_get_print_enable(0x82));
        ptr+=n; count-=n;

        n = scnprintf(ptr, count, "CEC_IMAGE_VIEW_ON=%d,                        <0x04>\n", 
            cec_msg_get_print_enable(0x04));
        ptr+=n; count-=n;

        n = scnprintf(ptr, count, "CEC_TEXT_VIEW_ON=%d,                         <0x0d>\n", 
            cec_msg_get_print_enable(0x0d));
        ptr+=n; count-=n;

        //---------------------------------------------------
        // Message Descriptions for the Routing Control Feature
        //---------------------------------------------------

        n = scnprintf(ptr, count, "CEC_INACTIVE_SOURCE=%d,                      <0x9d>\n", 
            cec_msg_get_print_enable(0x9d));
        ptr+=n; count-=n;

        n = scnprintf(ptr, count, "CEC_REQUEST_ACTIVE_SOURCE=%d,                <0x85>\n", 
            cec_msg_get_print_enable(0x85));
        ptr+=n; count-=n;

        n = scnprintf(ptr, count, "CEC_ROUTING_CHANGE=%d,                       <0x80>\n", 
            cec_msg_get_print_enable(0x80));
        ptr+=n; count-=n;

        n = scnprintf(ptr, count, "CEC_ROUTING_INFORMATION=%d,                  <0x81>\n", 
            cec_msg_get_print_enable(0x81));
        ptr+=n; count-=n;

        n = scnprintf(ptr, count, "CEC_SET_STREAM_PATH=%d,                      <0x86>\n", 
            cec_msg_get_print_enable(0x86));
        ptr+=n; count-=n;

        //---------------------------------------------------
        // Message Descriptions for the Standby Feature
        //---------------------------------------------------

        n = scnprintf(ptr, count, "CEC_STANDBY=%d,                              <0x36>\n", 
            cec_msg_get_print_enable(0x36));
        ptr+=n; count-=n;

        //---------------------------------------------------
        // Message Descriptions for the One Touch Record Feature 
        //---------------------------------------------------

        n = scnprintf(ptr, count, "CEC_RECORD_OFF=%d,                           <0x0b>\n", 
            cec_msg_get_print_enable(0x0b));
        ptr+=n; count-=n;

        n = scnprintf(ptr, count, "CEC_RECORD_ON=%d,                            <0x09>\n", 
            cec_msg_get_print_enable(0x09));
        ptr+=n; count-=n;

        n = scnprintf(ptr, count, "CEC_RECORD_STATUS=%d,                        <0x0a>\n", 
            cec_msg_get_print_enable(0x0a));
        ptr+=n; count-=n;

        n = scnprintf(ptr, count, "CEC_RECORD_TV_SCREEN=%d,                     <0x0f>\n", 
            cec_msg_get_print_enable(0x0f));
        ptr+=n; count-=n;

        //---------------------------------------------------
        // Message Descriptions for the CEC_VERSION
        //---------------------------------------------------

        n = scnprintf(ptr, count, "CEC_CEC_VERSION=%d,                          <0x9e>\n", 
            cec_msg_get_print_enable(0x9e));
        ptr+=n; count-=n;

        n = scnprintf(ptr, count, "CEC_GET_CEC_VERSION=%d,                      <0x9f>\n", 
            cec_msg_get_print_enable(0x9f));
        ptr+=n; count-=n;

        n = scnprintf(ptr, count, "CEC_REPORT_FEATURES=%d,                      <0xa6>\n", 
            cec_msg_get_print_enable(0xa6));
        ptr+=n; count-=n;

        n = scnprintf(ptr, count, "CEC_GIVE_FEATURES=%d,                        <0xa5>\n", 
            cec_msg_get_print_enable(0xa5));
        ptr+=n; count-=n;

        //---------------------------------------------------
        // Message Descriptions for the System Information Feature
        //---------------------------------------------------

        n = scnprintf(ptr, count, "CEC_GIVE_PHYSICAL_ADDR=%d,                   <0x83>\n", 
            cec_msg_get_print_enable(0x83));
        ptr+=n; count-=n;

        n = scnprintf(ptr, count, "CEC_GET_MENU_LANGUAGE=%d,                    <0x91>\n", 
            cec_msg_get_print_enable(0x91));
        ptr+=n; count-=n;

        n = scnprintf(ptr, count, "CEC_REPORT_PHYSICAL_ADDR=%d,                 <0x84>\n", 
            cec_msg_get_print_enable(0x84));
        ptr+=n; count-=n;

        n = scnprintf(ptr, count, "CEC_SET_MENU_LANGUAGE=%d,                    <0x32>\n", 
            cec_msg_get_print_enable(0x32));
        ptr+=n; count-=n;

        //---------------------------------------------------
        // Message Descriptions for the Deck Control Feature
        //---------------------------------------------------

        n = scnprintf(ptr, count, "CEC_DECK_CONTROL=%d,                         <0x42>\n", 
            cec_msg_get_print_enable(0x42));
        ptr+=n; count-=n;

        n = scnprintf(ptr, count, "CEC_DECK_STATUS=%d,                          <0x1b>\n", 
            cec_msg_get_print_enable(0x1b));
        ptr+=n; count-=n;

        n = scnprintf(ptr, count, "CEC_GIVE_DECK_STATUS=%d,                     <0x1a>\n", 
            cec_msg_get_print_enable(0x1a));
        ptr+=n; count-=n;

        n = scnprintf(ptr, count, "CEC_PLAY=%d,                                 <0x41>\n", 
            cec_msg_get_print_enable(0x41));
        ptr+=n; count-=n;

        //---------------------------------------------------
        // Message Descriptions for the Tuner Control Feature
        //---------------------------------------------------

        n = scnprintf(ptr, count, "CEC_GIVE_TUNER_DEVICE_STATUS=%d,             <0x08>\n", 
            cec_msg_get_print_enable(0x08));
        ptr+=n; count-=n;

        n = scnprintf(ptr, count, "CEC_SELECT_ANALOGUE_SERVICE=%d,              <0x92>\n", 
            cec_msg_get_print_enable(0x92));
        ptr+=n; count-=n;

        n = scnprintf(ptr, count, "CEC_SELECT_DIGITAL_SERVICE=%d,               <0x93>\n", 
            cec_msg_get_print_enable(0x93));
        ptr+=n; count-=n;

        n = scnprintf(ptr, count, "CEC_TUNER_DEVICE_STATUS=%d,                  <0x07>\n", 
            cec_msg_get_print_enable(0x07));
        ptr+=n; count-=n;

        n = scnprintf(ptr, count, "CEC_TUNER_STEP_DECREMENT=%d,                 <0x06>\n", 
            cec_msg_get_print_enable(0x06));
        ptr+=n; count-=n;

        n = scnprintf(ptr, count, "CEC_TUNER_STEP_INCREMENT=%d,                 <0x05>\n", 
            cec_msg_get_print_enable(0x05));
        ptr+=n; count-=n;
        
        //---------------------------------------------------
        // Message Descriptions for the Vendor Specific Commands Feature
        //---------------------------------------------------

        n = scnprintf(ptr, count, "CEC_DEVICE_VENDOR_ID=%d,                     <0x87>\n", 
            cec_msg_get_print_enable(0x87));
        ptr+=n; count-=n;

        n = scnprintf(ptr, count, "CEC_GIVE_DEVICE_VENDOR_ID=%d,                <0x8c>\n", 
            cec_msg_get_print_enable(0x8c));
        ptr+=n; count-=n;

        n = scnprintf(ptr, count, "CEC_VENDOR_COMMAND=%d,                       <0x89>\n", 
            cec_msg_get_print_enable(0x89));
        ptr+=n; count-=n;

        n = scnprintf(ptr, count, "CEC_VENDOR_COMMAND_WITH_ID=%d,               <0xa0>\n", 
            cec_msg_get_print_enable(0xa0));
        ptr+=n; count-=n;

        n = scnprintf(ptr, count, "CEC_VENDOR_REMOTE_BUTTON_DOWN=%d,            <0x8a>\n", 
            cec_msg_get_print_enable(0x8a));
        ptr+=n; count-=n;

        n = scnprintf(ptr, count, "CEC_VENDOR_REMOTE_BUTTON_UP=%d,              <0x8b>\n", 
            cec_msg_get_print_enable(0x8b));
        ptr+=n; count-=n;

        //---------------------------------------------------
        // Message Descriptions for the OSD Status Display Feature
        //---------------------------------------------------

        n = scnprintf(ptr, count, "CEC_SET_OSD_STRING=%d,                       <0x64>\n", 
            cec_msg_get_print_enable(0x64));
        ptr+=n; count-=n;

        //---------------------------------------------------
        // Message Descriptions for the OSD Transfer Feature
        //---------------------------------------------------

        n = scnprintf(ptr, count, "CEC_GIVE_OSD_NAME=%d,                        <0x46>\n", 
            cec_msg_get_print_enable(0x46));
        ptr+=n; count-=n;

        n = scnprintf(ptr, count, "CEC_SET_OSD_NAME=%d,                         <0x47>\n", 
            cec_msg_get_print_enable(0x47));
        ptr+=n; count-=n;

        //---------------------------------------------------
        // Message Descriptions for the Device Menu Control Feature
        //---------------------------------------------------

        n = scnprintf(ptr, count, "CEC_MENU_REQUEST=%d,                         <0x8d>\n", 
            cec_msg_get_print_enable(0x8d));
        ptr+=n; count-=n;

        n = scnprintf(ptr, count, "CEC_MENU_STATUS=%d,                          <0x8e>\n", 
            cec_msg_get_print_enable(0x8e));
        ptr+=n; count-=n;

        //---------------------------------------------------
        // Message Descriptions for the Power Status Feature
        //---------------------------------------------------
        n = scnprintf(ptr, count, "CEC_GIVE_DEVICE_POWER_STATUS=%d,             <0x8f>\n", 
            cec_msg_get_print_enable(0x8f));
        ptr+=n; count-=n;

        n = scnprintf(ptr, count, "CEC_REPORT_POWER_STATUS=%d,                  <0x90>\n", 
            cec_msg_get_print_enable(0x90));
        ptr+=n; count-=n;

        //---------------------------------------------------
        // Message Descriptions for the General Protocol messages
        //---------------------------------------------------

        n = scnprintf(ptr, count, "CEC_FEATURE_ABORT=%d,                        <0x00>\n", 
            cec_msg_get_print_enable(0x00));
        ptr+=n; count-=n;

        n = scnprintf(ptr, count, "CEC_ABORT=%d,                                <0xff>\n", 
            cec_msg_get_print_enable(0xff));
        ptr+=n; count-=n;

        //---------------------------------------------------
        // Message Descriptions for the System Audio Contorl Feature
        //---------------------------------------------------

        n = scnprintf(ptr, count, "CEC_GIVE_AUDIO_STATUS=%d,                    <0x71>\n", 
            cec_msg_get_print_enable(0x71));
        ptr+=n; count-=n;

        n = scnprintf(ptr, count, "CEC_GIVE_SYSTEM_AUDIO_MODE_STATUS=%d,        <0x7d>\n", 
            cec_msg_get_print_enable(0x7d));
        ptr+=n; count-=n;

        n = scnprintf(ptr, count, "CEC_REPORT_AUDIO_STATUS=%d,                  <0x7a>\n", 
            cec_msg_get_print_enable(0x7a));
        ptr+=n; count-=n;

        n = scnprintf(ptr, count, "CEC_REPORT_SHORT_AUDIO_DESCRIPTOR=%d,        <0xa3>\n", 
            cec_msg_get_print_enable(0xa3));
        ptr+=n; count-=n;

        n = scnprintf(ptr, count, "CEC_REQUEST_SHORT_AUDIO_DESCRIPTOR=%d,       <0xa4>\n", 
            cec_msg_get_print_enable(0xa4));
        ptr+=n; count-=n;

        n = scnprintf(ptr, count, "CEC_SET_SYSTEM_AUDIO_MODE=%d,                <0x72>\n", 
            cec_msg_get_print_enable(0x72));
        ptr+=n; count-=n;

        n = scnprintf(ptr, count, "CEC_SYSTEM_AUDIO_MODE_REQUEST=%d,            <0x70>\n", 
            cec_msg_get_print_enable(0x70));
        ptr+=n; count-=n;

        n = scnprintf(ptr, count, "CEC_SYSTEM_AUDIO_MODE_STATUS=%d,             <0x7e>\n", 
            cec_msg_get_print_enable(0x7e));
        ptr+=n; count-=n;

        n = scnprintf(ptr, count, "CEC_USER_CONTROL_PRESSED=%d,                 <0x44>\n", 
            cec_msg_get_print_enable(0x44));
        ptr+=n; count-=n;

        n = scnprintf(ptr, count, "CEC_USER_CONTROL_RELEASED=%d,                <0x45>\n", 
            cec_msg_get_print_enable(0x45));
        ptr+=n; count-=n;

        //---------------------------------------------------
        // Message Descriptions for the Audio Rate Contorl Feature 
        //---------------------------------------------------

        n = scnprintf(ptr, count, "CEC_SET_AUDIO_RATE=%d,                       <0x9a>\n", 
            cec_msg_get_print_enable(0x9a));
        ptr+=n; count-=n;

        //---------------------------------------------------
        // Message Descriptions for the Audio Return Channel Contorl Feature
        //---------------------------------------------------

        n = scnprintf(ptr, count, "CEC_INITIATE_ARC=%d,                         <0xc0>\n", 
            cec_msg_get_print_enable(0xc0));
        ptr+=n; count-=n;

        n = scnprintf(ptr, count, "CEC_REPORT_ARC_INITIATED=%d,                 <0xc1>\n", 
            cec_msg_get_print_enable(0xc1));
        ptr+=n; count-=n;   

        n = scnprintf(ptr, count, "CEC_REPORT_ARC_TERMINATED=%d,                <0xc2>\n", 
            cec_msg_get_print_enable(0xc2));
        ptr+=n; count-=n;

        n = scnprintf(ptr, count, "CEC_REQUEST_ARC_INITIATION=%d,               <0xc3>\n", 
            cec_msg_get_print_enable(0xc3));
        ptr+=n; count-=n;

        n = scnprintf(ptr, count, "CEC_REQUEST_ARC_TERMINATION=%d,              <0xc4>\n", 
            cec_msg_get_print_enable(0xc4));
        ptr+=n; count-=n;

        n = scnprintf(ptr, count, "CEC_TERMINATE_ARC=%d,                        <0xc5>\n", 
            cec_msg_get_print_enable(0xc5));
        ptr+=n; count-=n;

        //---------------------------------------------------
        // Message Descriptions for the CEC CURRENT LATENCY
        //---------------------------------------------------

        n = scnprintf(ptr, count, "CEC_REQUEST_CURRENT_LATENCY=%d,              <0xa7>\n", 
            cec_msg_get_print_enable(0xa7));
        ptr+=n; count-=n;

        n = scnprintf(ptr, count, "CEC_REPORT_CURRENT_LATENCY=%d,               <0xa8>\n", 
            cec_msg_get_print_enable(0xa8));
        ptr+=n; count-=n;
    }

    n = scnprintf(ptr, count, "echo <op code> <print_enable> > cec_print_en\nex. echo 82 01 > cec_print_en, 0x82 print_en is enable\n");
    ptr+=n; count-=n;

    return ptr - buf;
}

static ssize_t cec_print_en_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t n)
{
    int val = 0;
    int opcode = 0;
    int len = 0;
    u8 arr[2];
    if (strcmp(attr->attr.name, "cec_print_en")==0)
    {
        memset(&arr, 0, sizeof(arr));
        len = hexRmSpace2bin(arr, buf, n);
        if (len <= 0)
            return -EINVAL;
        opcode = arr[0];
        val = arr[1];
        if(opcode == 0x01) opcode = -1;
        cec_msg_set_print_enable(opcode, val);
    }
    return n;
}
cec_attr(cec_print_en);

static ssize_t cec_msg_count_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    int  n;
    int count = PAGE_SIZE;
    char *ptr = buf;

    if (strcmp(attr->attr.name, "cec_msg_count")==0)
    {
        n = scnprintf(ptr, count, "\nCEC_POLLING=%d,                                <-1>\n", 
            cec_msg_get_msg_count(-1));
        ptr+=n; count-=n;

        //---------------------------------------------------
        // Message Descriptions for the One Touch Play Feature 
        //---------------------------------------------------

        n = scnprintf(ptr, count, "CEC_ACTIVE_SOURCE=%d,                        <0x82>\n", 
            cec_msg_get_msg_count(0x82));
        ptr+=n; count-=n;

        n = scnprintf(ptr, count, "CEC_IMAGE_VIEW_ON=%d,                        <0x04>\n", 
            cec_msg_get_msg_count(0x04));
        ptr+=n; count-=n;

        n = scnprintf(ptr, count, "CEC_TEXT_VIEW_ON=%d,                         <0x0d>\n", 
            cec_msg_get_msg_count(0x0d));
        ptr+=n; count-=n;

        //---------------------------------------------------
        // Message Descriptions for the Routing Control Feature
        //---------------------------------------------------

        n = scnprintf(ptr, count, "CEC_INACTIVE_SOURCE=%d,                      <0x9d>\n", 
            cec_msg_get_msg_count(0x9d));
        ptr+=n; count-=n;

        n = scnprintf(ptr, count, "CEC_REQUEST_ACTIVE_SOURCE=%d,                <0x85>\n", 
            cec_msg_get_msg_count(0x85));
        ptr+=n; count-=n;

        n = scnprintf(ptr, count, "CEC_ROUTING_CHANGE=%d,                       <0x80>\n", 
            cec_msg_get_msg_count(0x80));
        ptr+=n; count-=n;

        n = scnprintf(ptr, count, "CEC_ROUTING_INFORMATION=%d,                  <0x81>\n", 
            cec_msg_get_msg_count(0x81));
        ptr+=n; count-=n;

        n = scnprintf(ptr, count, "CEC_SET_STREAM_PATH=%d,                      <0x86>\n", 
            cec_msg_get_msg_count(0x86));
        ptr+=n; count-=n;

        //---------------------------------------------------
        // Message Descriptions for the Standby Feature
        //---------------------------------------------------

        n = scnprintf(ptr, count, "CEC_STANDBY=%d,                              <0x36>\n", 
            cec_msg_get_msg_count(0x36));
        ptr+=n; count-=n;

        //---------------------------------------------------
        // Message Descriptions for the One Touch Record Feature 
        //---------------------------------------------------

        n = scnprintf(ptr, count, "CEC_RECORD_OFF=%d,                           <0x0b>\n", 
            cec_msg_get_msg_count(0x0b));
        ptr+=n; count-=n;

        n = scnprintf(ptr, count, "CEC_RECORD_ON=%d,                            <0x09>\n", 
            cec_msg_get_msg_count(0x09));
        ptr+=n; count-=n;

        n = scnprintf(ptr, count, "CEC_RECORD_STATUS=%d,                        <0x0a>\n", 
            cec_msg_get_msg_count(0x0a));
        ptr+=n; count-=n;

        n = scnprintf(ptr, count, "CEC_RECORD_TV_SCREEN=%d,                     <0x0f>\n", 
            cec_msg_get_msg_count(0x0f));
        ptr+=n; count-=n;

        //---------------------------------------------------
        // Message Descriptions for the CEC_VERSION
        //---------------------------------------------------

        n = scnprintf(ptr, count, "CEC_CEC_VERSION=%d,                          <0x9e>\n", 
            cec_msg_get_msg_count(0x9e));
        ptr+=n; count-=n;

        n = scnprintf(ptr, count, "CEC_GET_CEC_VERSION=%d,                      <0x9f>\n", 
            cec_msg_get_msg_count(0x9f));
        ptr+=n; count-=n;

        n = scnprintf(ptr, count, "CEC_REPORT_FEATURES=%d,                      <0xa6>\n", 
            cec_msg_get_msg_count(0xa6));
        ptr+=n; count-=n;

        n = scnprintf(ptr, count, "CEC_GIVE_FEATURES=%d,                        <0xa5>\n", 
            cec_msg_get_msg_count(0xa5));
        ptr+=n; count-=n;

        //---------------------------------------------------
        // Message Descriptions for the System Information Feature
        //---------------------------------------------------

        n = scnprintf(ptr, count, "CEC_GIVE_PHYSICAL_ADDR=%d,                   <0x83>\n", 
            cec_msg_get_msg_count(0x83));
        ptr+=n; count-=n;

        n = scnprintf(ptr, count, "CEC_GET_MENU_LANGUAGE=%d,                    <0x91>\n", 
            cec_msg_get_msg_count(0x91));
        ptr+=n; count-=n;

        n = scnprintf(ptr, count, "CEC_REPORT_PHYSICAL_ADDR=%d,                 <0x84>\n", 
            cec_msg_get_msg_count(0x84));
        ptr+=n; count-=n;

        n = scnprintf(ptr, count, "CEC_SET_MENU_LANGUAGE=%d,                    <0x32>\n", 
            cec_msg_get_msg_count(0x32));
        ptr+=n; count-=n;

        //---------------------------------------------------
        // Message Descriptions for the Deck Control Feature
        //---------------------------------------------------

        n = scnprintf(ptr, count, "CEC_DECK_CONTROL=%d,                         <0x42>\n", 
            cec_msg_get_msg_count(0x42));
        ptr+=n; count-=n;

        n = scnprintf(ptr, count, "CEC_DECK_STATUS=%d,                          <0x1b>\n", 
            cec_msg_get_msg_count(0x1b));
        ptr+=n; count-=n;

        n = scnprintf(ptr, count, "CEC_GIVE_DECK_STATUS=%d,                     <0x1a>\n", 
            cec_msg_get_msg_count(0x1a));
        ptr+=n; count-=n;

        n = scnprintf(ptr, count, "CEC_PLAY=%d,                                 <0x41>\n", 
            cec_msg_get_msg_count(0x41));
        ptr+=n; count-=n;

        //---------------------------------------------------
        // Message Descriptions for the Tuner Control Feature
        //---------------------------------------------------

        n = scnprintf(ptr, count, "CEC_GIVE_TUNER_DEVICE_STATUS=%d,             <0x08>\n", 
            cec_msg_get_msg_count(0x08));
        ptr+=n; count-=n;

        n = scnprintf(ptr, count, "CEC_SELECT_ANALOGUE_SERVICE=%d,              <0x92>\n", 
            cec_msg_get_msg_count(0x92));
        ptr+=n; count-=n;

        n = scnprintf(ptr, count, "CEC_SELECT_DIGITAL_SERVICE=%d,               <0x93>\n", 
            cec_msg_get_msg_count(0x93));
        ptr+=n; count-=n;

        n = scnprintf(ptr, count, "CEC_TUNER_DEVICE_STATUS=%d,                  <0x07>\n", 
            cec_msg_get_msg_count(0x07));
        ptr+=n; count-=n;

        n = scnprintf(ptr, count, "CEC_TUNER_STEP_DECREMENT=%d,                 <0x06>\n", 
            cec_msg_get_msg_count(0x06));
        ptr+=n; count-=n;

        n = scnprintf(ptr, count, "CEC_TUNER_STEP_INCREMENT=%d,                 <0x05>\n", 
            cec_msg_get_msg_count(0x05));
        ptr+=n; count-=n;
        
        //---------------------------------------------------
        // Message Descriptions for the Vendor Specific Commands Feature
        //---------------------------------------------------

        n = scnprintf(ptr, count, "CEC_DEVICE_VENDOR_ID=%d,                     <0x87>\n", 
            cec_msg_get_msg_count(0x87));
        ptr+=n; count-=n;

        n = scnprintf(ptr, count, "CEC_GIVE_DEVICE_VENDOR_ID=%d,                <0x8c>\n", 
            cec_msg_get_msg_count(0x8c));
        ptr+=n; count-=n;

        n = scnprintf(ptr, count, "CEC_VENDOR_COMMAND=%d,                       <0x89>\n", 
            cec_msg_get_msg_count(0x89));
        ptr+=n; count-=n;

        n = scnprintf(ptr, count, "CEC_VENDOR_COMMAND_WITH_ID=%d,               <0xa0>\n", 
            cec_msg_get_msg_count(0xa0));
        ptr+=n; count-=n;

        n = scnprintf(ptr, count, "CEC_VENDOR_REMOTE_BUTTON_DOWN=%d,            <0x8a>\n", 
            cec_msg_get_msg_count(0x8a));
        ptr+=n; count-=n;

        n = scnprintf(ptr, count, "CEC_VENDOR_REMOTE_BUTTON_UP=%d,              <0x8b>\n", 
            cec_msg_get_msg_count(0x8b));
        ptr+=n; count-=n;

        //---------------------------------------------------
        // Message Descriptions for the OSD Status Display Feature
        //---------------------------------------------------

        n = scnprintf(ptr, count, "CEC_SET_OSD_STRING=%d,                       <0x64>\n", 
            cec_msg_get_msg_count(0x64));
        ptr+=n; count-=n;

        //---------------------------------------------------
        // Message Descriptions for the OSD Transfer Feature
        //---------------------------------------------------

        n = scnprintf(ptr, count, "CEC_GIVE_OSD_NAME=%d,                        <0x46>\n", 
            cec_msg_get_msg_count(0x46));
        ptr+=n; count-=n;

        n = scnprintf(ptr, count, "CEC_SET_OSD_NAME=%d,                         <0x47>\n", 
            cec_msg_get_msg_count(0x47));
        ptr+=n; count-=n;

        //---------------------------------------------------
        // Message Descriptions for the Device Menu Control Feature
        //---------------------------------------------------

        n = scnprintf(ptr, count, "CEC_MENU_REQUEST=%d,                         <0x8d>\n", 
            cec_msg_get_msg_count(0x8d));
        ptr+=n; count-=n;

        n = scnprintf(ptr, count, "CEC_MENU_STATUS=%d,                          <0x8e>\n", 
            cec_msg_get_msg_count(0x8e));
        ptr+=n; count-=n;

        //---------------------------------------------------
        // Message Descriptions for the Power Status Feature
        //---------------------------------------------------
        n = scnprintf(ptr, count, "CEC_GIVE_DEVICE_POWER_STATUS=%d,             <0x8f>\n", 
            cec_msg_get_msg_count(0x8f));
        ptr+=n; count-=n;

        n = scnprintf(ptr, count, "CEC_REPORT_POWER_STATUS=%d,                  <0x90>\n", 
            cec_msg_get_msg_count(0x90));
        ptr+=n; count-=n;

        //---------------------------------------------------
        // Message Descriptions for the General Protocol messages
        //---------------------------------------------------

        n = scnprintf(ptr, count, "CEC_FEATURE_ABORT=%d,                        <0x00>\n", 
            cec_msg_get_msg_count(0x00));
        ptr+=n; count-=n;

        n = scnprintf(ptr, count, "CEC_ABORT=%d,                                <0xff>\n", 
            cec_msg_get_msg_count(0xff));
        ptr+=n; count-=n;

        //---------------------------------------------------
        // Message Descriptions for the System Audio Contorl Feature
        //---------------------------------------------------

        n = scnprintf(ptr, count, "CEC_GIVE_AUDIO_STATUS=%d,                    <0x71>\n", 
            cec_msg_get_msg_count(0x71));
        ptr+=n; count-=n;

        n = scnprintf(ptr, count, "CEC_GIVE_SYSTEM_AUDIO_MODE_STATUS=%d,        <0x7d>\n", 
            cec_msg_get_msg_count(0x7d));
        ptr+=n; count-=n;

        n = scnprintf(ptr, count, "CEC_REPORT_AUDIO_STATUS=%d,                  <0x7a>\n", 
            cec_msg_get_msg_count(0x7a));
        ptr+=n; count-=n;

        n = scnprintf(ptr, count, "CEC_REPORT_SHORT_AUDIO_DESCRIPTOR=%d,        <0xa3>\n", 
            cec_msg_get_msg_count(0xa3));
        ptr+=n; count-=n;

        n = scnprintf(ptr, count, "CEC_REQUEST_SHORT_AUDIO_DESCRIPTOR=%d,       <0xa4>\n", 
            cec_msg_get_msg_count(0xa4));
        ptr+=n; count-=n;

        n = scnprintf(ptr, count, "CEC_SET_SYSTEM_AUDIO_MODE=%d,                <0x72>\n", 
            cec_msg_get_msg_count(0x72));
        ptr+=n; count-=n;

        n = scnprintf(ptr, count, "CEC_SYSTEM_AUDIO_MODE_REQUEST=%d,            <0x70>\n", 
            cec_msg_get_msg_count(0x70));
        ptr+=n; count-=n;

        n = scnprintf(ptr, count, "CEC_SYSTEM_AUDIO_MODE_STATUS=%d,             <0x7e>\n", 
            cec_msg_get_msg_count(0x7e));
        ptr+=n; count-=n;

        n = scnprintf(ptr, count, "CEC_USER_CONTROL_PRESSED=%d,                 <0x44>\n", 
            cec_msg_get_msg_count(0x44));
        ptr+=n; count-=n;

        n = scnprintf(ptr, count, "CEC_USER_CONTROL_RELEASED=%d,                <0x45>\n", 
            cec_msg_get_msg_count(0x45));
        ptr+=n; count-=n;

        //---------------------------------------------------
        // Message Descriptions for the Audio Rate Contorl Feature 
        //---------------------------------------------------

        n = scnprintf(ptr, count, "CEC_SET_AUDIO_RATE=%d,                       <0x9a>\n", 
            cec_msg_get_msg_count(0x9a));
        ptr+=n; count-=n;

        //---------------------------------------------------
        // Message Descriptions for the Audio Return Channel Contorl Feature
        //---------------------------------------------------

        n = scnprintf(ptr, count, "CEC_INITIATE_ARC=%d,                         <0xc0>\n", 
            cec_msg_get_msg_count(0xc0));
        ptr+=n; count-=n;

        n = scnprintf(ptr, count, "CEC_REPORT_ARC_INITIATED=%d,                 <0xc1>\n", 
            cec_msg_get_msg_count(0xc1));
        ptr+=n; count-=n;   

        n = scnprintf(ptr, count, "CEC_REPORT_ARC_TERMINATED=%d,                <0xc2>\n", 
            cec_msg_get_msg_count(0xc2));
        ptr+=n; count-=n;

        n = scnprintf(ptr, count, "CEC_REQUEST_ARC_INITIATION=%d,               <0xc3>\n", 
            cec_msg_get_msg_count(0xc3));
        ptr+=n; count-=n;

        n = scnprintf(ptr, count, "CEC_REQUEST_ARC_TERMINATION=%d,              <0xc4>\n", 
            cec_msg_get_msg_count(0xc4));
        ptr+=n; count-=n;

        n = scnprintf(ptr, count, "CEC_TERMINATE_ARC=%d,                        <0xc5>\n", 
            cec_msg_get_msg_count(0xc5));
        ptr+=n; count-=n;

        //---------------------------------------------------
        // Message Descriptions for the CEC CURRENT LATENCY
        //---------------------------------------------------

        n = scnprintf(ptr, count, "CEC_REQUEST_CURRENT_LATENCY=%d,              <0xa7>\n", 
            cec_msg_get_msg_count(0xa7));
        ptr+=n; count-=n;

        n = scnprintf(ptr, count, "CEC_REPORT_CURRENT_LATENCY=%d,               <0xa8>\n", 
            cec_msg_get_msg_count(0xa8));
        ptr+=n; count-=n;
    }

    n = scnprintf(ptr, count, "echo <op code> <clear_code> > cec_msg_count\nex. echo 82 01 > cec_msg_count\n");
    ptr+=n; count-=n;

    return ptr - buf;
}

static ssize_t cec_msg_count_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t n)
{
    int val = 0;
    int opcode = 0;
    int len = 0;
    u8 arr[2];
    if (strcmp(attr->attr.name, "cec_msg_count")==0)
    {
        memset(&arr, 0, sizeof(arr));
        len = hexRmSpace2bin(arr, buf, n);
        if (len <= 0)
            return -EINVAL;
        opcode = arr[0];
        val = arr[1];
        if(opcode == 0x01) opcode = -1;
        if(val == 1) cec_msg_clear_msg_count(opcode);
    }

    return n;
}
cec_attr(cec_msg_count);

static struct attribute *cec_attrs[] = {
    &cec_dbg_en_attr.attr,
    &cec_dump_current_setting_attr.attr,
    &cec_msg_out_attr.attr,
	&cec_print_en_attr.attr,
    &cec_msg_count_attr.attr,
    NULL,
};

static struct attribute_group cec_attr_group = {
    .attrs = cec_attrs,
};

void rtk_cec_sysfs_init(void)
{
    cec_kobj = kobject_create_and_add("cec", NULL);
    if (cec_kobj) {
        if (sysfs_create_group(cec_kobj, &cec_attr_group) != 0) {
            kobject_put(cec_kobj);
            cec_kobj = NULL;
        }
    }
}

void rtk_cec_sysfs_uninit(void)
{
    if (cec_kobj) {
        sysfs_remove_group(cec_kobj, &cec_attr_group);
        kobject_put(cec_kobj);
        cec_kobj = NULL;
    }
}
