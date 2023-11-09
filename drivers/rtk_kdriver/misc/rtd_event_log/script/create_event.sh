#!/bin/bash
read -p "enter your fw:" fw
if [ -d "../fw/$fw" ]; then
    echo "fw: $fw exists"
else
    echo "create fw: $fw"
    read -p "enter your fw id (0x00 ~ 0x0f):" fw_id
    if grep -q  $fw_id ../core/event_log_core.h; then
    echo "module: $fw_id exists"
    exit 0
    fi
    bash event_log_creat_fw_file.sh $fw $fw_id
fi

read -p "enter your module:" module
if [ -f "../fw/$fw/event_list-$module.h" ]; then
    echo "module: $module exists"
else
    echo "create module: $module"
    bash event_log_add_module.sh $fw $module
fi

read -p "enter your event type:" eventtype
if grep -q ${fw^^}_${module^^}_${eventtype^^}_TYPE_ID ../fw/$fw/event_list-$module.h; then
    echo "type: ${fw^^}_${module^^}_${eventtype^^}_TYPE_ID exists"
else
    echo "create type: ${fw^^}_${module^^}_${eventtype^^}_TYPE_ID"
    bash event_log_add_type.sh $fw $module $eventtype
fi

read -p "enter your event:" event
if grep -q ${fw^^}_${module^^}_${eventtype^^}_${event^^}_EVENT_ID ../fw/$fw/event_list-$module.h; then
    echo "event: ${fw^^}_${module^^}_${eventtype^^}_${event^^}_EVENT_ID exists"
else
    echo "create event: ${fw^^}_${module^^}_${eventtype^^}_${event^^}_EVENT_ID"
    bash event_log_add_event.sh $fw $module $eventtype $event
fi
#read -p "enter your event:" event
