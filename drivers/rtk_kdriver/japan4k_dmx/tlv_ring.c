#include <linux/kernel.h>
#include <linux/slab.h>
#include "tlv_ring.h"
#include "japan4k_dmx.h"

#define malloc(size)        kmalloc(size, GFP_KERNEL)
#define free(addr)          kfree(addr)

tlv_ring_buffer* tlv_ring_alloc(unsigned int size)
{
        tlv_ring_buffer* ring = (tlv_ring_buffer*) malloc(size * 2 + sizeof(tlv_ring_buffer));
        ring->base = ((unsigned char*) ring) + sizeof(tlv_ring_buffer);
        ring->rp = ring->base;
        ring->wp = ring->base;
        ring->limit = ring->base + size;
        return ring;
}

void tlv_ring_destroy(tlv_ring_buffer* ring)
{
        if (ring)
                free(ring);
}

void tlv_ring_dump_status(tlv_ring_buffer* ring)
{
        JAPAN4K_DMX_INFO("ring[%p] = %p/%p/%p/%p\n",
               ring,
               ring->base,
               ring->rp,
               ring->wp,
               ring->limit);
}

int tlv_ring_get_free_space(
        tlv_ring_buffer*        ring,
        unsigned char**     pp_wp,
        unsigned int*       p_size
)
{
        unsigned int free_size;

        if (ring == NULL || pp_wp == NULL || p_size == NULL) {
                JAPAN4K_DMX_ERROR("tlv_ring_release_data failed, invalid arguments\n");
                return -1;
        }

        //ring_dump_status(ring);

        *pp_wp = ring->wp;

        if (ring->wp >= ring->rp) {
                *p_size   = (unsigned int) (ring->limit - ring->wp);
                free_size = (unsigned int) (ring->limit - ring->wp) +
                            (unsigned int) (ring->rp - ring->base);
        } else {
                *p_size = (unsigned int) (ring->wp - ring->rp);
                free_size = (unsigned int) (ring->rp - ring->wp);
        }

        if (*p_size == free_size)   // to make sure enough space remain
                *p_size = (free_size > 512) ? (free_size - 512) : 0;

        return 0;
}

int tlv_ring_put_data(
        tlv_ring_buffer*        ring,
        unsigned char*      p_wp,
        unsigned int        len
)
{
        if (ring == NULL) {
                JAPAN4K_DMX_ERROR("tlv_ring_release_data failed, invalid arguments\n");
                return -1;
        }

        if (p_wp != ring->wp) {
                // humm ... should not happen
                JAPAN4K_DMX_ERROR("tlv_ring_release_data failed, wp out off sync\n");
                return -1;
        }

        if (p_wp > ring->limit || p_wp < ring->base)
                return -1;

        ring->wp = p_wp + len;

        if (ring->wp >= ring->limit)
                ring->wp = ring->base;

        return 0;
}

int tlv_ring_get_data(
        tlv_ring_buffer*        ring,
        unsigned char**     pp_rp,
        unsigned int*       p_len
)
{
        if (ring == NULL || pp_rp == NULL || p_len == NULL) {
                JAPAN4K_DMX_ERROR("tlv_ring_release_data failed, rp out off sync\n");
                return -1;
        }

        *pp_rp = ring->rp;

        if (ring->wp >= ring->rp)
                *p_len = (unsigned int) (ring->wp - ring->rp);
        else
                *p_len = (unsigned int) (ring->limit - ring->rp);

        return 0;
}

int tlv_ring_release_data(
        tlv_ring_buffer*        ring,
        unsigned char*      p_rp,
        unsigned int        len
)
{
        if (ring == NULL) {
                JAPAN4K_DMX_ERROR("tlv_ring_release_data failed, invalid arguments\n");
                return -1;
        }

        if (p_rp != ring->rp) {
                // humm ... should not happen
                JAPAN4K_DMX_ERROR("tlv_ring_release_data failed, rp out off sync\n");
                return -1;
        }

        if (p_rp > ring->limit || p_rp < ring->base)
                return -1;

        ring->rp = p_rp + len;

        if (ring->rp >= ring->limit)
                ring->rp = ring->base;

        return 0;
}
