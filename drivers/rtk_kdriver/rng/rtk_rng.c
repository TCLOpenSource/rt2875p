#include <linux/syscalls.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <rtk_kdriver/io.h>
#include <linux/delay.h>
#include "rtk_rng_reg.h"

#include <linux/hw_random.h>

#define MIN_RNG_READ_SIZE sizeof(unsigned int)

static bool get_rtk_rng(unsigned int *p_random_data)
{
    bool in_atomic_context = false;
    unsigned int poll_times = 0;
    unsigned int zero_times = 0;

    if (!p_random_data) {
        return false;
    }

    in_atomic_context = in_atomic();

AGAIN:
    poll_times = 0;
    while ((rtd_inl(RNG_RETURN3) & 0x1) == 0)  {
        if (in_atomic_context) {
            udelay(100);
        }
        else {
            usleep_range(100, 100);
        }

        if (poll_times++ > 10000) {
            return false;
        }
    }

    *p_random_data = rtd_inl(RNG_RESULTR);
    if (*p_random_data == 0x0) {
        if (zero_times++ > 100) {
            return false;
        }
        goto AGAIN;
    }

    return true;
}


bool rtk_rng_get_random_data(unsigned char *buf, unsigned int len)
{
    unsigned char *ptr = buf;
    unsigned int loop = 0;
    unsigned int i  = 0;
    unsigned int sb = len & 0x3;

    if (!buf || !len) {
        return false;
    }

    loop = len / MIN_RNG_READ_SIZE;
    for (i = 0; i < loop; i++) {
        unsigned int random_number = 0;
        if (get_rtk_rng(&random_number) == false) {
            return false;
        }
        memcpy(ptr, &random_number, MIN_RNG_READ_SIZE);
        ptr += 4;
    }

    if (sb) {
        unsigned int random_number = 0;
        if (get_rtk_rng(&random_number) == false) {
            return false;
        }
        memcpy(ptr, &random_number, sb);
    }

    return true;
}
EXPORT_SYMBOL(rtk_rng_get_random_data);
#if 0 //defined(CONFIG_HW_RANDOM)
struct hwrng rtk_rng;

static int rtk_rng_data_read(struct hwrng *rng, u32 *data)
{
    if (get_rtk_rng(data))
        return 4;
    return 0;
}

static int rtk_rng_read(struct hwrng *rng, void *data, size_t max, bool wait)
{
    if(rtk_rng_get_random_data(data, max))
        return max;
    return 0;
}

int rtk_rng_register(void)
{
    memset(&rtk_rng, 0, sizeof(rtk_rng));
    rtk_rng.name = "rtk_rng";
    rtk_rng.init = NULL;
    rtk_rng.cleanup = NULL;
    rtk_rng.cleanup = NULL;
    rtk_rng.data_present = NULL;
    rtk_rng.data_read = rtk_rng_data_read;
    rtk_rng.read = rtk_rng_read;
    rtk_rng.quality = 1024;
    return hwrng_register(&rtk_rng);
}

void rtk_rng_unregister(void)
{
    return hwrng_unregister(&rtk_rng);
}
#endif

