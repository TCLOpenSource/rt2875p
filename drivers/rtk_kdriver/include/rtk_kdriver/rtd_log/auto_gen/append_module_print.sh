MODULE_NAME=$1
MODUL_TAG=$2

#echo "MODULE_NAME=$MODULE_NAME"
#echo "MODUL_TAG=$MODUL_TAG"

MODULE_NAME_S=$(echo $MODULE_NAME | sed 's/[A-Z]/\L&/g')
#echo "MODULE_NAME_S=$MODULE_NAME_S"

cp template/module_print_api.temp    .temp 
sed -i "s/MODULE_NAME/$MODULE_NAME/g" .temp
sed -i "s/MOUDLE_TAG_STR/$MODUL_TAG/g" .temp
sed -i "s/MODULE_TAG/TAG_NAME_$MODULE_NAME/g" .temp
sed -i "s/PRINT_PREFIX/$MODULE_NAME_S/g" .temp
cat .temp
