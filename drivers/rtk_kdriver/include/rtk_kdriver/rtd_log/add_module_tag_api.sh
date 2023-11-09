TAG_NAME=$1
MODULE_TAG_FILE=$PWD/rtd_module_tag.h
MODULE_LOG_FILE=$PWD/rtd_module_log.h
AUTO_GEN_PATH=$PWD/auto_gen/
MODULE_TAG_FILE_TMP=$AUTO_GEN_PATH/rtd_module_tag.h
MODULE_LOG_FILE_TMP=$AUTO_GEN_PATH/rtd_module_log.h

echo "MODULE_TAG_FILE=$MODULE_TAG_FILE"
echo "MODULE_LOG_FILE=$MODULE_LOG_FILE"
echo "AUTO_GEN_PATH=$AUTO_GEN_PATH"


#####################################
# check tag name
#####################################
TAG_NAME_PRESENT=$(grep TAG_NAME_$TAG_NAME $MODULE_TAG_FILE)

if [ -z "$TAG_NAME_PRESENT" ]
then
    echo "TAG_NAME_$TAG_NAME absent, force add it"
    sed -i '/#define __RTK_MODULE_TAG_H__/a\#define TAG_NAME_APPEND_MODULE_TAG             \"APPEND_MODULE_TAG\"' $MODULE_TAG_FILE
    sed -i '/#define __RTK_MODULE_TAG_H__/a\' $MODULE_TAG_FILE
    sed -i "{s/APPEND_MODULE_TAG/$TAG_NAME/g}" $MODULE_TAG_FILE
else
    echo "TAG_NAME_$TAG_NAME present, ignore it"
fi

#####################################
# check log api
#####################################
API_PRESENT=$(grep "print api for $TAG_NAME" $MODULE_LOG_FILE)


if [ -z "$API_PRESENT" ]
then
    echo "API_PRESENT absent, force add it"
    cat $MODULE_TAG_FILE | grep $TAG_NAME > $MODULE_TAG_FILE_TMP
    cat  $MODULE_TAG_FILE_TMP
    cd $AUTO_GEN_PATH
    sh gen_print_api.sh $MODULE_TAG_FILE_TMP > $MODULE_LOG_FILE_TMP
    cat $MODULE_LOG_FILE_TMP >> $MODULE_LOG_FILE

else
    echo "API_PRESENT present, ignore it"
fi


#####################################
# Show Diff
#####################################
git diff $MODULE_TAG_FILE
git diff $MODULE_LOG_FILE

#####################################
# clean up
#####################################
rm -f $MODULE_TAG_FILE_TMP $MODULE_LOG_FILE_TMP