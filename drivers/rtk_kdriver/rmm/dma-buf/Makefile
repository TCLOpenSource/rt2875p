# SPDX-License-Identifier: GPL-2.0

#FIXME no need to check duplicated file after GKI integration and upstream is clean
ifneq (,$(wildcard drivers/dma-buf/heaps/carveout_heap.c))
$(info #[rmm][FIXME] drivers/dma-buf/heaps/carveout_heap.c exist!)
else

obj-$(CONFIG_DMABUF_CARVEOUT_HEAP)  += carveout_heap__.o

endif


#FIXME no need to check duplicated file after GKI integration and upstream is clean
ifneq (,$(wildcard drivers/dma-buf/heaps/dvr_heap.c))
$(info #[rmm][FIXME] drivers/dma-buf/heaps/dvr_heap.c exist!)
else

obj-$(CONFIG_DMABUF_HEAPS_DVR)      += dvr_heap__.o

endif


#FIXME no need to check duplicated file after GKI integration and upstream is clean
ifneq (,$(wildcard drivers/dma-buf/heaps/vbm_heap.c))
$(info #[rmm][FIXME] drivers/dma-buf/heaps/vbm_heap.c exist!)
else

obj-$(CONFIG_DMABUF_HEAPS_VBM)      += vbm_heap__.o

endif

dvr_heap__-objs      := dvr_heap.o
vbm_heap__-objs      := vbm_heap.o genpool.o
carveout_heap__-objs := carveout_heap.o genpool.o
