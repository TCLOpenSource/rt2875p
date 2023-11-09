fw="$1"
fw_id="$2"
file=../fw/${fw}/module_list.h
file_s=../fw/${fw}/module_string_list.h
core_file_c=../core/event_log_core.c
core_file_h=../core/event_log_core.h

cd ../fw/;mkdir ${fw};
echo "
#ifndef __RTK_MODULE_EVENT_${fw^^}_DEFINE_H__
#define __RTK_MODULE_EVENT_${fw^^}_DEFINE_H__
/*
######################################################################################
# MODULE DEFINE
######################################################################################
*/
typedef enum{
    ${fw^^}_MODULE_MAX
}${fw^^}_MODULE_ID;
#endif /* __RTK_MODULE_EVENT_${fw^^}_DEFINE_H__ */" > ${file}

echo "
#ifndef __RTK_MODULE_EVENT_${fw^^}_STRING_DEFINE_H__
#define __RTK_MODULE_EVENT_${fw^^}_STRING_DEFINE_H__
#include <fw/${fw,,}/module_list.h>
/*
######################################################################################
# MODULE STRING DEFINE
######################################################################################
*/
struct event_module_s ${fw,,}_module[]={
    {EVENT_LIMIT_MAX,NULL,NULL},
};
#endif /* __RTK_MODULE_EVENT_${fw^^}_STRING_DEFINE_H__ */" > ${file_s}

core_insert=$(grep -n "{EVENT_LIMIT_MAX,NULL,NULL}" ${core_file_c} | awk -F':' '{print $1}')
sed -i ''"$core_insert"'i  \ \ \ \ \'"{EVENT_FW_${fw^^}, \"${fw^^}\", ${fw,,}_module},"'' ${core_file_c}

core_ID_insert=$(grep -n "FW TAG DEFINE" ${core_file_h} | awk -F':' '{print $1}')
core_ID_insert=$(($core_ID_insert+3))
sed -i ''"$core_ID_insert"'i'"#define EVENT_FW_${fw^^}                   ${fw_id}"'' ${core_file_h}

core_list_insert=$(grep -n "All FW Define" ${core_file_h} | awk -F':' '{print $1}')
core_list_insert=$(($core_list_insert+3))
sed -i ''"$core_list_insert"'i'"#include <fw/${fw,,}/module_list.h>"'' ${core_file_h}

core_string_list_insert=$(grep -n "INCLUDE EVENT PARSE LIBRARY" ${core_file_c} | awk -F':' '{print $1}')
core_string_list_insert=$(($core_string_list_insert+3))
sed -i ''"$core_string_list_insert"'i'"#include <fw/${fw,,}/module_string_list.h>"'' ${core_file_c}

