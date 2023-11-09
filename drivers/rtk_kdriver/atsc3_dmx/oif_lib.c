#include <linux/kernel.h>
#include <linux/slab.h>
#include "oif_lib.h"
#include "atsc3_dmx.h"

#define OIF_HDR_ERR_EN      (1<<7)
#define OIF_HDR_ERR         (1<<6)

int oif_encapsulate_bbf(
        unsigned char*      bbf,
        unsigned short      bbf_sz,
        unsigned char*      out,
        unsigned long       out_len,
        unsigned char       plp,
        unsigned char*      p_cc,
        unsigned char       tmode,
        unsigned int        tinfo_l,
        unsigned int        tinfo_h,
        unsigned char       err
)
{
        if (out_len < bbf_sz + OIF_HDR_SIZE)
                return -1;

        memset(out, 0x5A, 4);
        out[4] = plp & 0x3F;

        if (err)
                out[4] |= OIF_HDR_ERR_EN | OIF_HDR_ERR;     // err & err_en

        out[5] = (bbf_sz >> 8) & 0xFF;
        out[6] = (bbf_sz) & 0xFF;
        out[7] = *p_cc;
        *p_cc = out[7] + 1;
        out[8] = tmode & 0xFF;
#if 1
        atsc_memcpy(&out[9],  (unsigned char*)&tinfo_l, sizeof(int));
        atsc_memcpy(&out[13], (unsigned char*)&tinfo_h, sizeof(int));
#else
        if(tmode == 0x80) {
        } else if(tmode == 0x81) {
                tinfo_l = tinfo_l & 0xFFC00000;
                atsc_memcpy(&out[9],  &tinfo_l, sizeof(int));
                atsc_memcpy(&out[13], &tinfo_h, sizeof(int));
        } else if(tmode == 0x82) {
                tinfo_l = tinfo_l & 0xFFFFF000;
                atsc_memcpy(&out[9],  &tinfo_l, sizeof(int));
                atsc_memcpy(&out[13], &tinfo_h, sizeof(int));
        } else if(tmode == 0x83) {
                tinfo_l = tinfo_l & 0xFFFFFFFC0;
                atsc_memcpy(&out[9],  &tinfo_l, sizeof(int));
                atsc_memcpy(&out[13], &tinfo_h, sizeof(int));
        }
#endif
        ATSC3_DMX_INFO("cc=%d\n", out[7]);

        atsc_memcpy(out + OIF_HDR_SIZE, bbf, bbf_sz);

        return bbf_sz + OIF_HDR_SIZE;
}

int oif_parse_sync_byte(
        unsigned char* oif_in,
        unsigned short oif_sb_sz,
        unsigned char* oif_out,
        unsigned long  sb_len)
{

        unsigned int size = oif_sb_sz;
        unsigned int oif_len = 0;
        unsigned int cnt = 0;

        if (oif_in == NULL || oif_out == NULL) {
                ATSC3_DMX_WARN("parse oif sync byte failed - bbp_in is null\n");
                return 0;
        }

        if (oif_sb_sz < 188) {
                ATSC3_DMX_WARN("oif_sb long is not enough one TS (oif_sb_sz=%d)\n", oif_sb_sz);
                return 0;
        }

        // parse oif len include header(17B)
        if(/**(unsigned int*)oif_in == 0x47 &&*/ * ((unsigned int*)(oif_in + sb_len)) == OIF_STARTER_VAL) {
                oif_len = OIF_HDR_SIZE + ((oif_in[sb_len + 5] << 8) + (oif_in[sb_len + 6]));
                ATSC3_DMX_INFO("OIF packet len %d include header(17B), oif_sb_sz = %d\n", oif_len, size); //188*((oif/187)+1)
                if(sb_len == 0) {
                        atsc_memcpy(oif_out, oif_in, oif_len);
                        return oif_len;
                }
        }

        while(size) {

                // Normal case, remove sync byte (0x47)
                atsc_memcpy(oif_out + (188 - sb_len)*cnt, oif_in + 188 * cnt + sb_len, 188 - sb_len);
                size -= 188;
                cnt++;

                // last 188 need handle fill character
                if(size <= 188) {
                        if(oif_len - ((188 - sb_len) * cnt))
                                atsc_memcpy(oif_out + (188 - sb_len)*cnt, oif_in + 188 * cnt + sb_len, oif_len - ((188 - sb_len) * cnt));
                        break;
                }
        }

        return oif_len;
}

/*******************************************************************************
 * FUNC : oif_generate_sync_byte
 * DESC : add oif sync byte (0x47)
 * PARM : oif_sb_sz total length not include sync byte
 * RET  : Total (oif + header + 0x47) data len
 *******************************************************************************/
int oif_generate_sync_byte(
        unsigned char*      oif_in,
        unsigned short      oif_sz,
        unsigned char*      oif_out
)
{
        unsigned int size = oif_sz;
        unsigned int cnt = 0, i;

        if (oif_in == NULL || oif_out == NULL) {
                ATSC3_DMX_WARN("do oif file divided into 187 byte failed, buff is null\n");
                return 0;
        }

        while(size) {

                oif_out[188 * cnt] = 0x47;
                if(SYNC_BYTE_LEN >= 2) {
                        for(i = 1; i <= SYNC_BYTE_LEN; i++) {
                                oif_out[188 * cnt + i] = 0x7F;
                        }
                }
                // Finally, handle less than 188 length
                if(size < (188 - SYNC_BYTE_LEN)) {

                        atsc_memcpy(oif_out + 188 * cnt + SYNC_BYTE_LEN, oif_in + (188 - SYNC_BYTE_LEN)*cnt, size);

                        memset(oif_out + 188 * cnt + SYNC_BYTE_LEN + size, 0xEE, (188 - SYNC_BYTE_LEN) - size);

                        cnt++;

                        break;
                }
                // Normal case, add sync byte (0x47)
                atsc_memcpy(oif_out + 188 * cnt + SYNC_BYTE_LEN, oif_in + (188 - SYNC_BYTE_LEN)*cnt, 188 - SYNC_BYTE_LEN);
                size -= (188 - SYNC_BYTE_LEN);
                cnt++;
        }

        return 188 * cnt;
}
unsigned char* oif_find_starter(
        unsigned char*      oif,
        unsigned long       oif_len,
        unsigned long       sb_len
)
{
        //unsigned int i;
        // find oif sync byte 0x47 0x5A 0x5A 0x5A 0x5A
        while(oif_len >= 4 + sb_len) {
                /*
                for(i = 0;i < sb_len; i++){
                    if(oif[i] != 0x47){
                        oif++;
                        oif_len--;
                    }
                }
                */
                if(*((unsigned int*)(oif + sb_len)) == OIF_STARTER_VAL) {
                        //ATSC3_DMX_WARN("find stater = %p\n", oif);
                        return oif;
                }

                oif++;
                oif_len--;
        }
        //OIF_WARNING("find stater failed\n"/*, oif*/);

        return NULL;
}
/*
unsigned char* oif_find_sb_starter(
    unsigned char*      oif,
    unsigned long       oif_len
    )
{
    while(oif_len >= 5)
    {
                if(oif[0] == 0x47 && oif[1] == 0x5A && oif[2] == 0x5A && oif[3] == 0x5A && oif[4] == 0x5A){
            //ATSC3_DMX_WARN("find stater = %p\n", oif);
            return oif;
        }
        oif++;
        oif_len--;
    }

    ATSC3_DMX_WARN("find stater failed oif=%x\n", oif[0]);

    return NULL;
}
*/
