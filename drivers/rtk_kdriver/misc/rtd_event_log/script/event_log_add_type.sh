fw="$1"
module="${2}"
type="${3}"
module_file=../fw/${fw}/event_list-${module,,}.h
module_string_file=../fw/${fw}/event_string_list-${module,,}.h
module_id="${fw^^}_MODULE_ID_${module^^}"

event_type_ID=${fw^^}_${module^^}_${type^^}_TYPE_ID
event_type_array=${fw,,}_${module,,}_${type,,}_event

#LIST
module_file_insert=$(grep -n "${module^^} EVENT DEFINE" ${module_file} | awk -F':' '{print $1}')
module_file_insert_add_2=$(($module_file_insert+3))
module_file_insert_add_3=$(($module_file_insert+4))
module_file_insert_add_4=$(($module_file_insert+5))
module_file_insert_add_5=$(($module_file_insert+6))
module_file_insert_add_6=$(($module_file_insert+7))
sed -i ''"$module_file_insert_add_2"' i'"typedef enum"'' ${module_file}
sed -i ''"$module_file_insert_add_3"' i'"{"'' ${module_file}
sed -i ''"$module_file_insert_add_4"'i  \ \ \ \ \'"${fw^^}_${module^^}_${type^^}_EVENT_MAX"'' ${module_file}
sed -i ''"$module_file_insert_add_5"' i'"}${fw^^}_${module^^}_${type^^}_EVENT_ID;\n"'' ${module_file}

type_insert=$(grep -n "${fw^^}_${module^^}_TYPE_MAX" ${module_file} | awk -F':' '{print $1}')
sed -i ''"$type_insert"'i  \ \ \ \ \'"${event_type_ID},"'' ${module_file}

#STRING LIST
module_string_file_insert=$(grep -n "${module^^} EVENT STRING DEFINE" ${module_string_file} | awk -F':' '{print $1}')
module_string_file_insert_add_2=$(($module_file_insert+3))
module_string_file_insert_add_3=$(($module_file_insert+4))
module_string_file_insert_add_4=$(($module_file_insert+5))
sed -i ''"$module_string_file_insert_add_2"' i'"\static struct event_event_s ${event_type_array}[]={"'' ${module_string_file}
sed -i ''"$module_string_file_insert_add_3"'i  \ \ \ \ \'"{EVENT_LIMIT_MAX,NULL},"'' ${module_string_file}
sed -i ''"$module_string_file_insert_add_4"' i'"};\n"'' ${module_string_file}

type_string_insert=$(grep -n "event_eventtype_s" ${module_string_file} | awk -F':' '{print $1}')
type_string_insert_add_2=$(($type_string_insert+1))
sed -i ''"$type_string_insert_add_2"'i  \ \ \ \ \'"{${event_type_ID}, \"${type}\", ${event_type_array}},"'' ${module_string_file}
#sed -i ''"$type_insert"'i'"};"'' ${module_file}