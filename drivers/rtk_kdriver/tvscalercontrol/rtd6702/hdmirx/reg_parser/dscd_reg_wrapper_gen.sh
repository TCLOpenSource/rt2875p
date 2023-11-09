DSCD_REG_FILE=$1
DSCD_PORT_NUM=2
OUTPUT_PATH=./tmp/dscd
DSCD_REG_OUT_FILE=$OUTPUT_PATH/dscd_reg_wrapper.h
DSCD_REG_OUT_FILE2=$OUTPUT_PATH/dscd_reg_wrapper.c

echo " ==========================================================="
echo " = Stage 0) cleanup                                         "
echo " ==========================================================="

mkdir -p $OUTPUT_PATH
rm $OUTPUT_PATH/*

echo " ==========================================================="
echo " = Stage 1) get register address from $DSCD_REG_FILE        "
echo " ==========================================================="
echo " DSCD reg file : $DSCD_REG_FILE"
STAGE1_IN=$DSCD_REG_FILE
STAGE1_OUT=$OUTPUT_PATH/dsc_reg_wrapper-1.h

#cat $STAGE1_IN | grep "_reg " | grep 0xB8 > $STAGE1_OUT
#cat $STAGE1_IN | grep "_reg " | grep 0xB8 | awk '{printf "%-8s %-80s %-80s\n",  $1, $2, $2}' > $STAGE1_OUT
cat $STAGE1_IN | grep "_reg " | grep 0xB8 | awk '{printf "%-8s %-80s %-80s\n",  $1, $2, $2}' | sed 's/DSCD_P0_//1' > $STAGE1_OUT
#cat $STAGE1_OUT

echo " ==========================================================="
echo " = Stage 2) Generate data structure                         "
echo " ==========================================================="
STAGE2_IN=$STAGE1_OUT
STAGE2_OUT=$OUTPUT_PATH/dsc_reg_wrapper-2.h

echo "struct dscd_reg_st" > $STAGE2_OUT
echo "{" >> $STAGE2_OUT
cat $STAGE2_IN | sed 's/_reg//1' | awk '{$2=tolower($2)}1' | awk '{printf "    uint32_t     %s;\n",  $2}' >> $STAGE2_OUT
echo "};" >> $STAGE2_OUT
#cat $STAGE2_OUT


echo " ==========================================================="
echo " = Stage 3) Generate regsister warpper                      "
echo " ==========================================================="
echo " generate from                                              "
echo " #define PPS_DW00_reg    DSCD_P0_PPS_DW00_reg               "
echo " to                                                         "
echo " #define DSCD_PPS_DW00_reg    (dscd_hw[nport].pps_dw00)     "
echo " ==========================================================="

STAGE3_IN=$STAGE1_OUT
STAGE3_OUT=$OUTPUT_PATH/dsc_reg_wrapper-3.h

echo "#define DSCD_NUM   $DSCD_PORT_NUM" > $STAGE3_OUT
echo "extern const struct dscd_reg_st dscd_hw[DSCD_NUM]; \n" >> $STAGE3_OUT
#cat $STAGE3_IN
#cat $STAGE3_IN | awk '{printf "%-8s DSCD_%-80s %-80s\n",  $1, $2, $3}' | sed 's/_reg//2' | sed 's/DSCD_P0_//1' >> $STAGE3_OUT
#cat $STAGE3_IN | awk '{printf "%-8s DSCD_%-80s %-80s\n",  $1, $2, $3}' | sed 's/_reg//2' | sed 's/DSCD_P0_//1' | awk '{$3=tolower($3)}1'  >> $STAGE3_OUT
cat $STAGE3_IN | awk '{printf "%-8s DSCD_%-80s %-80s\n",  $1, $2, $3}' | sed 's/_reg//2' | sed 's/DSCD_P0_//1' | awk '{$3=tolower($3)}1' | awk '{printf "%-8s %-80s (dscd_hw[nport].%s)\n",  $1, $2, $3}'  >> $STAGE3_OUT
#echo "\n" >> $STAGE3_OUT
cat $STAGE3_OUT

echo " ==========================================================="
echo " = Stage 4) Generate regsister macro wrapper                "
echo " ==========================================================="
echo " generate from                                              "
echo " #define PPS_DW00_reg    DSCD_P0_PPS_DW00_reg               "
echo " to                                                         "
echo " #define DSCD_PPS_DW00_reg    (dscd_hw[nport].pps_dw00)     "
echo " ==========================================================="

STAGE4_IN=$DSCD_REG_FILE
STAGE4_OUT=$OUTPUT_PATH/dsc_reg_wrapper-4.h

#cat $STAGE4_IN
#cat $STAGE4_IN | grep -e "(data)" -e "_mask"    > $STAGE4_OUT
#cat $STAGE4_IN | grep -e "(data)" -e "_mask" | grep -v "define  set_" > $STAGE4_OUT
#cat $STAGE4_IN | grep -e "(data)" -e "_mask" | grep -v "define  set_" | awk '{printf "%-8s %-80s %-80s\n",  $1, $2, $2}' > $STAGE4_OUT
cat $STAGE4_IN | grep -e "(data)" -e "_mask" | grep -v "define  set_" | awk '{printf "%-8s %-80s %-80s\n",  $1, $2, $2}' | sed 's/_P0_/_/1' > $STAGE4_OUT
#cat $STAGE4_OUT


echo " ==========================================================="
echo " = Stage 5) Generate register mapping table                 "
echo " ==========================================================="
echo " generate from                                              "
echo " onst struct dscd_reg_st dscd_hw[DSCD_NUM] =                "
echo " {                                                          "
echo "     {                                                      "
echo "         .reg = xxxx,                                        "
echo "     },                                                     "
echo "     {                                                      "
echo "     },                                                     "
echo " };                                                         "
echo " ==========================================================="
STAGE5_IN=$STAGE1_OUT
STAGE5_OUT=$OUTPUT_PATH/dsc_reg_wrapper.c

#cat $STAGE5_IN
echo "const struct dscd_reg_st dscd_hw[DSCD_NUM] =\n" > $STAGE5_OUT
echo "{" >> $STAGE5_OUT
echo "    {" >> $STAGE5_OUT
# reg for DSCD_p0
cat $STAGE5_IN | awk '{$2=tolower($2)}1' | awk '{printf "        .%-40s = %s,\n", $2, $3}' | sed 's/_reg//1' >> $STAGE5_OUT
echo "    }," >> $STAGE5_OUT
# reg for DSCD_p1
echo "    {" >> $STAGE5_OUT
cat $STAGE5_IN | awk '{$2=tolower($2)}1' | awk '{printf "        .%-40s = %s,\n", $2, $3}' | sed 's/_reg//1' | sed 's/_P0_/_P1_/1' >> $STAGE5_OUT
echo "    }," >> $STAGE5_OUT
echo "};" >> $STAGE5_OUT
#cat $STAGE5_OUT

echo " ==========================================================="
echo " = Stage 6) Generate final output                           "
echo " ==========================================================="
echo " Ouput file: $DSCD_REG_OUT_FILE                             "
echo " Ouput file: $DSCD_REG_OUT_FILE2                            "
echo " ==========================================================="

echo "\n\n" >> $DSCD_REG_OUT_FILE
cat $STAGE2_OUT >> $DSCD_REG_OUT_FILE

echo "\n\n" >> $DSCD_REG_OUT_FILE
cat $STAGE3_OUT >> $DSCD_REG_OUT_FILE

echo "\n\n" >> $DSCD_REG_OUT_FILE
cat $STAGE4_OUT >> $DSCD_REG_OUT_FILE

#cat $DSCD_REG_OUT_FILE
#cat $STAGE5_OUT > $DSCD_REG_OUT_FILE2
