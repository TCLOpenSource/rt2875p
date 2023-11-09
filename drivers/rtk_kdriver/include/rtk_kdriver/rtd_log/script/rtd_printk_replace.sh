TARGET=$1
TMP_FILE=$TARGET.tmp
echo "TARGET_FILE=$TARGET, TMP_FILE=$TMP_FILE"
cat $TARGET > $TMP_FILE

# HOW TO USE
# Please Replase the following string to your target
# TAG_NAME : rtd_printk( KERN_XXX, TAG_NAME ...
# MODULE_TAG_STR : rtd_printk( KERN_XXX, TAG_NAME ...
# vpq : rtd_pr_xxxx_info(

#remove define
sed -i '{s/#define TAG_NAME[[:blank:]]*"VPQ"//g}' $TMP_FILE

#remove blanks before KERN_XXX
sed -i '{s/rtd_printk([[:blank:]]*KERN_/rtd_printk(KERN_/g}' $TMP_FILE
sed -i '{s/rtd_printk([[:blank:]]*level/rtd_printk(level/g}' $TMP_FILE

#remove blanks after KERN_XXX
sed -i '{s/rtd_printk(KERN_EMERG[[:blank:]]*/rtd_printk(KERN_EMERG/g}' $TMP_FILE
sed -i '{s/rtd_printk(KERN_ALERT[[:blank:]]*/rtd_printk(KERN_ALERT/g}' $TMP_FILE
sed -i '{s/rtd_printk(KERN_CRIT[[:blank:]]*/rtd_printk(KERN_CRIT/g}' $TMP_FILE
sed -i '{s/rtd_printk(KERN_ERR[[:blank:]]*/rtd_printk(KERN_ERR/g}' $TMP_FILE
sed -i '{s/rtd_printk(KERN_WARNING[[:blank:]]*/rtd_printk(KERN_WARNING/g}' $TMP_FILE
sed -i '{s/rtd_printk(KERN_NOTICE[[:blank:]]*/rtd_printk(KERN_NOTICE/g}' $TMP_FILE
sed -i '{s/rtd_printk(KERN_INFO[[:blank:]]*/rtd_printk(KERN_INFO/g}' $TMP_FILE
sed -i '{s/rtd_printk(KERN_DEBUG[[:blank:]]*/rtd_printk(KERN_DEBUG/g}' $TMP_FILE
sed -i '{s/rtd_printk(level[[:blank:]]*/rtd_printk(level/g}' $TMP_FILE

#remove blanks before module name
sed -i '{s/rtd_printk(KERN_EMERG,[[:blank:]]*/rtd_printk(KERN_EMERG,/g}' $TMP_FILE
sed -i '{s/rtd_printk(KERN_ALERT,[[:blank:]]*/rtd_printk(KERN_ALERT,/g}' $TMP_FILE
sed -i '{s/rtd_printk(KERN_CRIT,[[:blank:]]*/rtd_printk(KERN_CRIT,/g}' $TMP_FILE
sed -i '{s/rtd_printk(KERN_ERR,[[:blank:]]*/rtd_printk(KERN_ERR,/g}' $TMP_FILE
sed -i '{s/rtd_printk(KERN_WARNING,[[:blank:]]*/rtd_printk(KERN_WARNING,/g}' $TMP_FILE
sed -i '{s/rtd_printk(KERN_NOTICE,[[:blank:]]*/rtd_printk(KERN_NOTICE,/g}' $TMP_FILE
sed -i '{s/rtd_printk(KERN_INFO,[[:blank:]]*/rtd_printk(KERN_INFO,/g}' $TMP_FILE
sed -i '{s/rtd_printk(KERN_DEBUG,[[:blank:]]*/rtd_printk(KERN_DEBUG,/g}' $TMP_FILE
sed -i '{s/rtd_printk(level,[[:blank:]]*/rtd_printk(level,/g}' $TMP_FILE

#remove blanks after module name
sed -i '{s/rtd_printk(KERN_EMERG,TAG_NAME[[:blank:]]*/rtd_printk(KERN_EMERG,TAG_NAME/g}' $TMP_FILE
sed -i '{s/rtd_printk(KERN_ALERT,TAG_NAME[[:blank:]]*/rtd_printk(KERN_ALERT,TAG_NAME/g}' $TMP_FILE
sed -i '{s/rtd_printk(KERN_CRIT,TAG_NAME[[:blank:]]*/rtd_printk(KERN_CRIT,TAG_NAME/g}' $TMP_FILE
sed -i '{s/rtd_printk(KERN_ERR,TAG_NAME[[:blank:]]*/rtd_printk(KERN_ERR,TAG_NAME/g}' $TMP_FILE
sed -i '{s/rtd_printk(KERN_WARNING,TAG_NAME[[:blank:]]*/rtd_printk(KERN_WARNING,TAG_NAME/g}' $TMP_FILE
sed -i '{s/rtd_printk(KERN_NOTICE,TAG_NAME[[:blank:]]*/rtd_printk(KERN_NOTICE,TAG_NAME/g}' $TMP_FILE
sed -i '{s/rtd_printk(KERN_INFO,TAG_NAME[[:blank:]]*/rtd_printk(KERN_INFO,TAG_NAME/g}' $TMP_FILE
sed -i '{s/rtd_printk(KERN_DEBUG,TAG_NAME[[:blank:]]*/rtd_printk(KERN_DEBUG,TAG_NAME/g}' $TMP_FILE
sed -i '{s/rtd_printk(level,TAG_NAME[[:blank:]]*/rtd_printk(level,TAG_NAME/g}' $TMP_FILE

#remove blanks before fmt
sed -i '{s/rtd_printk(KERN_EMERG,TAG_NAME,[[:blank:]]*/rtd_printk(KERN_EMERG,TAG_NAME,/g}' $TMP_FILE
sed -i '{s/rtd_printk(KERN_ALERT,TAG_NAME,[[:blank:]]*/rtd_printk(KERN_ALERT,TAG_NAME,/g}' $TMP_FILE
sed -i '{s/rtd_printk(KERN_CRIT,TAG_NAME,[[:blank:]]*/rtd_printk(KERN_CRIT,TAG_NAME,/g}' $TMP_FILE
sed -i '{s/rtd_printk(KERN_ERR,TAG_NAME,[[:blank:]]*/rtd_printk(KERN_ERR,TAG_NAME,/g}' $TMP_FILE
sed -i '{s/rtd_printk(KERN_WARNING,TAG_NAME,[[:blank:]]*/rtd_printk(KERN_WARNING,TAG_NAME,/g}' $TMP_FILE
sed -i '{s/rtd_printk(KERN_NOTICE,TAG_NAME,[[:blank:]]*/rtd_printk(KERN_NOTICE,TAG_NAME,/g}' $TMP_FILE
sed -i '{s/rtd_printk(KERN_INFO,TAG_NAME,[[:blank:]]*/rtd_printk(KERN_INFO,TAG_NAME,/g}' $TMP_FILE
sed -i '{s/rtd_printk(KERN_DEBUG,TAG_NAME,[[:blank:]]*/rtd_printk(KERN_DEBUG,TAG_NAME,/g}' $TMP_FILE
sed -i '{s/rtd_printk(level,TAG_NAME,[[:blank:]]*/rtd_printk(level,TAG_NAME,/g}' $TMP_FILE

#replace new api
sed -i '{s/rtd_printk(KERN_EMERG,TAG_NAME,/rtd_pr_vpq_emerg(/g}' $TMP_FILE
sed -i '{s/rtd_printk(KERN_ALERT,TAG_NAME,/rtd_pr_vpq_alert(/g}' $TMP_FILE
sed -i '{s/rtd_printk(KERN_CRIT,TAG_NAME,/rtd_pr_vpq_crit(/g}' $TMP_FILE
sed -i '{s/rtd_printk(KERN_ERR,TAG_NAME,/rtd_pr_vpq_err(/g}' $TMP_FILE
sed -i '{s/rtd_printk(KERN_WARNING,TAG_NAME,/rtd_pr_vpq_warn(/g}' $TMP_FILE
sed -i '{s/rtd_printk(KERN_NOTICE,TAG_NAME,/rtd_pr_vpq_notice(/g}' $TMP_FILE
sed -i '{s/rtd_printk(KERN_INFO,TAG_NAME,/rtd_pr_vpq_info(/g}' $TMP_FILE
sed -i '{s/rtd_printk(KERN_DEBUG,TAG_NAME,/rtd_pr_vpq_debug(/g}' $TMP_FILE
sed -i '{s/rtd_printk(level,TAG_NAME,/rtd_pr_vpq_print(level,/g}' $TMP_FILE

diff $TARGET  $TMP_FILE
cp $TMP_FILE $TARGET
rm $TMP_FILE
