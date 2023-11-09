TARGET=$1
TMP_FILE=$TARGET.tmp
echo "TARGET_FILE=$TARGET, TMP_FILE=$TMP_FILE"
cat $TARGET > $TMP_FILE

MODULE_LOG_PREFIX=$2

# HOW TO USE
# MODULE_LOG_PREFIX : rtd_pr_xxxx_info(
sed -i '{s/<linux\/kernel.h>/<rtd_log\/rtd_module_log.h>/g}' $TMP_FILE

#remove blanks before KERN_XXX
sed -i '{s/printk([[:blank:]]*KERN_/printk(KERN_/g}' $TMP_FILE

#remove blanks after KERN_XXX
sed -i '{s/printk(KERN_EMERG[[:blank:]]*/printk(KERN_EMERG/g}' $TMP_FILE
sed -i '{s/printk(KERN_ALERT[[:blank:]]*/printk(KERN_ALERT/g}' $TMP_FILE
sed -i '{s/printk(KERN_CRIT[[:blank:]]*/printk(KERN_CRIT/g}' $TMP_FILE
sed -i '{s/printk(KERN_ERR[[:blank:]]*/printk(KERN_ERR/g}' $TMP_FILE
sed -i '{s/printk(KERN_WARNING[[:blank:]]*/printk(KERN_WARNING/g}' $TMP_FILE
sed -i '{s/printk(KERN_NOTICE[[:blank:]]*/printk(KERN_NOTICE/g}' $TMP_FILE
sed -i '{s/printk(KERN_INFO[[:blank:]]*/printk(KERN_INFO/g}' $TMP_FILE
sed -i '{s/printk(KERN_DEBUG[[:blank:]]*/printk(KERN_DEBUG/g}' $TMP_FILE

#replace new api
sed -i '{s/pr_emerg(/rtd_pr_MODULE_LOG_PREFIX_emerg(/g}' $TMP_FILE
sed -i '{s/pr_alert(/rtd_pr_MODULE_LOG_PREFIX_alert(/g}' $TMP_FILE
sed -i '{s/pr_crit(/rtd_pr_MODULE_LOG_PREFIX_crit(/g}' $TMP_FILE
sed -i '{s/pr_err(/rtd_pr_MODULE_LOG_PREFIX_err(/g}' $TMP_FILE
sed -i '{s/pr_warn(/rtd_pr_MODULE_LOG_PREFIX_warn(/g}' $TMP_FILE
sed -i '{s/pr_notice(/rtd_pr_MODULE_LOG_PREFIX_notice(/g}' $TMP_FILE
sed -i '{s/pr_info(/rtd_pr_MODULE_LOG_PREFIX_info(/g}' $TMP_FILE
sed -i '{s/pr_debug(/rtd_pr_MODULE_LOG_PREFIX_debug(/g}' $TMP_FILE

#replace printk xxx
sed -i '{s/printk(KERN_EMERG/rtd_pr_MODULE_LOG_PREFIX_emerg(/g}' $TMP_FILE
sed -i '{s/printk(KERN_ALERT/rtd_pr_MODULE_LOG_PREFIX_alert(/g}' $TMP_FILE
sed -i '{s/printk(KERN_CRIT/rtd_pr_MODULE_LOG_PREFIX_crit(/g}' $TMP_FILE
sed -i '{s/printk(KERN_ERR/rtd_pr_MODULE_LOG_PREFIX_err(/g}' $TMP_FILE
sed -i '{s/printk(KERN_WARNING/rtd_pr_MODULE_LOG_PREFIX_warn(/g}' $TMP_FILE
sed -i '{s/printk(KERN_NOTICE/rtd_pr_MODULE_LOG_PREFIX_notice(/g}' $TMP_FILE
sed -i '{s/printk(KERN_INFO/rtd_pr_MODULE_LOG_PREFIX_info(/g}' $TMP_FILE
sed -i '{s/printk(KERN_DEBUG/rtd_pr_MODULE_LOG_PREFIXc_debug(/g}' $TMP_FILE
sed -i '{s/printk(/rtd_pr_MODULE_LOG_PREFIX_info(/g}' $TMP_FILE
sed -i '{s/printk(/rtd_pr_MODULE_LOG_PREFIX_info(/g}' $TMP_FILE

#replace module name prefix
sed -i "{s/MODULE_LOG_PREFIX/$MODULE_LOG_PREFIX/g}" $TMP_FILE

diff $TARGET  $TMP_FILE
cp $TMP_FILE $TARGET
rm $TMP_FILE
