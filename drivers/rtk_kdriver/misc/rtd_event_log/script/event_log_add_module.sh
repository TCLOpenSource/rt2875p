fw="$1"
module="${2}"
module_file=../fw/${fw}/event_list-${module,,}.h
module_string_file=../fw/${fw}/event_string_list-${module,,}.h
module_id="${fw^^}_${module^^}_MODULE_ID"

module_list_file=../fw/${fw}/module_list.h
module_string_list_file=../fw/${fw}/module_string_list.h

#LIST
module_list_file_insert=$(grep -n "#define __RTK_" ${module_list_file} | awk -F':' '{print $1}')
module_list_file_insert_add_1=$(($module_list_file_insert+1))
sed -i ''"$module_list_file_insert_add_1"' i'"#include <fw/${fw,,}/event_list-${module,,}.h>"'' ${module_list_file}

module_list_file_max_insert=$(grep -n "${fw^^}_MODULE_MAX" ${module_list_file} | awk -F':' '{print $1}')
sed -i ''"$module_list_file_max_insert"'i  \ \ \ \ \'"$module_id,"'' ${module_list_file}

#STRING LIST
module_string_list_file_insert=$(grep -n "#include <fw/${fw,,}/module_list.h>" ${module_string_list_file} | awk -F':' '{print $1}')
module_string_list_file_insert_add_1=$(($module_string_list_file_insert+1))
sed -i ''"$module_string_list_file_insert_add_1"' i'"#include <fw/${fw,,}/event_string_list-${module,,}.h>"'' ${module_string_list_file}

module_string_list_file_max_insert=$(grep -n "EVENT_LIMIT_MAX" ${module_string_list_file} | awk -F':' '{print $1}')
sed -i ''"$module_string_list_file_max_insert"'i  \ \ \ \ \'"{$module_id, \"${module}\", ${fw,,}_${module,,}_eventtype},"'' ${module_string_list_file}


echo "#ifndef __RTK_MODULE_EVENT_${module^^}_H__
#define __RTK_MODULE_EVENT_${module^^}_H__
/*
######################################################################################
# ${module^^} EVENT DEFINE
######################################################################################
*/
/*
######################################################################################
# ${module^^} EVENT TYPE DEFINE
######################################################################################
*/
typedef enum
{
    ${fw^^}_${module^^}_TYPE_MAX
}${fw^^}_${module^^}_TYPE_ID;

/*
######################################################################################
# ${module^^} EVENT MERGER DW1 DEFINE
######################################################################################
*/

#define rtd_${module,,}_event_log(type, event, event_val, module_reserved)     rtd_${fw,,}_event_log(type, $module_id, event, event_val, module_reserved)
#endif /* __RTK_MODULE_EVENT_${module^^}_H__ */
" > ${module_file}

echo "#ifndef __RTK_MODULE_EVENT_${module^^}_STRING_H__
#define __RTK_MODULE_EVENT_${module^^}_STRING_H__
/*
######################################################################################
# ${module^^} EVENT STRING DEFINE
######################################################################################
*/
/*
######################################################################################
# ${module^^} EVENT TYPE STRING DEFINE
######################################################################################
*/
struct event_eventtype_s ${fw,,}_${module,,}_eventtype[]={
    {EVENT_LIMIT_MAX,NULL,NULL},
};
#endif /* __RTK_MODULE_EVENT_${module^^}_STRING_H__ */
" > ${module_string_file}