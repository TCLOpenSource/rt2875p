
#ifndef __PVR_DMA_RESV_H__
#define __PVR_DMA_RESV_H__

#include <linux/version.h>

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
#include <linux/dma-resv.h>
#else
#include <linux/reservation.h>

/* Reservation object types */
#define dma_resv			reservation_object
#define dma_resv_list			reservation_object_list

/* Reservation object functions */
#define dma_resv_add_excl_fence		reservation_object_add_excl_fence
#define dma_resv_add_shared_fence	reservation_object_add_shared_fence
#define dma_resv_fini			reservation_object_fini
#define dma_resv_get_excl		reservation_object_get_excl
#define dma_resv_get_list		reservation_object_get_list
#define dma_resv_held			reservation_object_held
#define dma_resv_init			reservation_object_init
#define dma_resv_reserve_shared		reservation_object_reserve_shared
#define dma_resv_test_signaled_rcu	reservation_object_test_signaled_rcu
#define dma_resv_wait_timeout_rcu	reservation_object_wait_timeout_rcu
#endif

#endif /* __PVR_DMA_RESV_H__ */
