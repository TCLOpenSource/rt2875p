#! /bin/bash
# Copyright (C) 2017 The Android Open Source Project
#

ALIGN=4

page_size=4096

DTS_LIST="
  overlay_test.dts
  overlay_chosen_dtbo_idx.dts
"
DTB_LIST=(
  "overlay_test.dts.tmp"
  "overlay_chosen_dtbo_idx.dts.tmp"
)

dtc_toolchain="../dtc/dtc_android"
OUTDIR="../out"

for dts in ${DTS_LIST}; do
  echo "Building $dts..."
  src_dts="${dts}"
  out_dtb="${OUTDIR}/${dts}.tmp"
  "$dtc_toolchain" -O dtb -@ -qq -a "$ALIGN" -o "$out_dtb" "$src_dts"
done

echo "Creating dtbo image with mkdtboimg"
./mkdtimg create  ${OUTDIR}/dtbo.img --page_size="$page_size" --custom0=0x1 --version=0 \
    "${OUTDIR}/overlay_test.dts.tmp" --id=0x1 > /dev/null
    
echo "remove tmp file"    
rm "${OUTDIR}"/*.tmp

