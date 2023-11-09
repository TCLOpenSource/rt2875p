fw="$1"
module="${2}"
type="${3}"
event="${4}"
module_file=../fw/${fw}/event_list-${module,,}.h
module_string_file=../fw/${fw}/event_string_list-${module,,}.h
fw_ID=EVENT_FW_${fw^^}
module_ID=${fw^^}_${module^^}_MODULE_ID
type_ID=${fw^^}_${module^^}_${type^^}_type_ID
event_ID=${fw^^}_${module^^}_${type^^}_${event^^}_EVENT_ID

#LIST
event_file_insert=$(grep -n "${fw^^}_${module^^}_${type^^}_EVENT_MAX" ${module_file} | awk -F':' '{print $1}')
sed -i ''"$event_file_insert"'i  \ \ \ \ \'"${event_ID},"'' ${module_file}

#EVENT MERGER DW1 DEFINE
event_file_insert=$(grep -n "EVENT MERGER DW1 DEFINE" ${module_file} | awk -F':' '{print $1}')
event_file_insert_add_3=$(($event_file_insert+3))
sed -i ''"$event_file_insert_add_3"'i'"#define   ${module^^}_DW1_${type}_${event}    MERGER_EVENT_DW1($fw_ID, $module_ID, $type_ID, $event_ID)"'' ${module_file}


#STRING LIST
event_string_file_insert=$(grep -n "event_event_s ${fw,,}_${module,,}_${type,,}_event" ${module_string_file} | awk -F':' '{print $1}')
event_string_file_insert_add_2=$(($event_string_file_insert+1))
sed -i ''"$event_string_file_insert_add_2"'i  \ \ \ \ \'"{${event_ID}, \"${event}\"},"'' ${module_string_file}