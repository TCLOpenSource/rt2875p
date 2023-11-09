# step1 : jump to target folder
cd $1

# step2 : gen target file
echo "# PATH: $1" > dprx_drv.mk
find prebuild/objs -name "*.o" > dprx_drv.mk

# step3 : rename to new file
sed 's/prebuild/dprx_vfe-objs += \$(SRC)\/prebuild/g' -i dprx_drv.mk
sleep 1
