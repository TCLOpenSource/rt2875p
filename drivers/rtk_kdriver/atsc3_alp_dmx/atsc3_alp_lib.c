#include <linux/kernel.h>
#include <linux/slab.h>
#include "atsc3_alp_lib.h"

unsigned char* atsc3_alp_find_starter(
        unsigned char*      alp,
        unsigned long       alp_len,
        unsigned long       sb_len
)
{
        while(alp_len >= sb_len) {
                if( (*((unsigned char*)(alp + 1)) == 0x40) && (*((unsigned char*)(alp + 2)) == 0x00)) {
                        return alp;
                }

                alp++;
                alp_len--;
        }
        return NULL;
}

