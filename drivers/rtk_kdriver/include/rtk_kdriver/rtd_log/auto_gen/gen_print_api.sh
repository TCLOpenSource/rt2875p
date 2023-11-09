TAG_NAME_FILE=$1
grep -rHn TAG_NAME_ $1 | awk '{printf "sh ./append_module_print.sh %s %s\n", $2, $3}' | sed 's/TAG_NAME_//' > .gen_print.sh
#cat .gen_print.sh
sh .gen_print.sh  