# 0. download an generic arm architecture toolchains and clang

# 1. build kernel 

## cd kernel path

cd kernel 

## generate configs

cp arch/arm64/configs/realtek/android.12.rtd6748_dv.tv030_defconfig .config

make menuconfig 

## compile with prebuilts toolchains

make -jN ARCH=arm64 LLVM=1 LLVM_IAS=1 HOSTCC=clang HOSTCXX=clang++ CC=clang LD=ld.lld AR=llvm-ar NM=llvm-nm OBJCOPY=llvm-objcopy OBJDUMP=llvm-objdump READELF=llvm-readelf OBJSIZE=llvm-size STRIP=llvm-strip DEPMOD=depmod CROSS_COMPILE=${CROSS_COMPILE} CHIP_NAME=rtd6748
