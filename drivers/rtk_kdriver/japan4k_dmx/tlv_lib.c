#include <linux/kernel.h>
#include <linux/slab.h>
#include "tlv_lib.h"

unsigned char* tlv_find_starter(unsigned char* oif, unsigned long oif_len, unsigned long sb_len)
{
        // find tlv sync byte 0x7f
        while(oif_len >= 1 + sb_len) {
                if(*((unsigned char*)(oif + sb_len)) == TLV_STARTER_VAL) {
                        return oif;
                }

                oif++;
                oif_len--;
        }

        return NULL;
}

