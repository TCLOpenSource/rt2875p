# SPDX-License-Identifier: GPL-2.0

#FIXME no need to check duplicated file after GKI integration and upstream is clean
ifneq (,$(wildcard drivers/staging/android/ion/heaps/ion_dvr_heap.c))
$(info #[rmm][FIXME] drivers/staging/android/ion/heaps/ion_dvr_heap.c exist!)
else

obj-$(CONFIG_ION_DVR_HEAP) += ion_dvr_heap.o

endif

