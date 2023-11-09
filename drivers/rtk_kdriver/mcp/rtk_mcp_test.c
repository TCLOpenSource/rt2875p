#include <linux/kernel.h>
#include <linux/ioport.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/init.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/dma-mapping.h>
#include <linux/notifier.h>
#include <linux/list.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>
#include <rtk_kdriver/rtk_mcp.h>
#include <mcp/rtk_mcp_test.h>
#include <rtk_kdriver/io.h>
#include <linux/pageremap.h>
#include "mcp/rtk_mcp_dbg.h"
#include "rtk_mcp_wrapper.h"

extern dma_addr_t _mcp_map_single(void *ptr, size_t size, enum dma_data_direction dir);
extern void _mcp_unmap_single(dma_addr_t addr, size_t size, enum dma_data_direction dir);
extern void* mcp_dma_alloc_coherent(unsigned int size, dma_addr_t* p_dma_addr, unsigned long flags);
extern void mcp_dma_free_coherent(unsigned int size, unsigned char* p_buffer, dma_addr_t p_dma_addr);

/*----------------------------------------------------------------------
 * Func : mcp_dump_mem
 *
 * Desc : dump data in memory
 *
 * Parm : data : start address of data
 *        len  : length of data
 *
 * Retn : N/A
 *----------------------------------------------------------------------*/
void mcp_dump_mem(
    unsigned char*          data,
    unsigned int            len)
{
    unsigned int i;
    char out[128] = {0};
    for (i = 0; i < len; i++) {
        if ((i & 0xF) == 0) {
            mcp_info("%s\n", out);
            snprintf(out, 128, " %04x | ", i);
        }
        snprintf(out + strlen(out), 128 - strlen(out), "%02x ", data[i]);
    }
    mcp_info("%s\n", out);
}

/***************************************************************************
            ------------------- Auto Test ----------------
****************************************************************************/

int PrintCheckReuslt(unsigned int enc, unsigned char* pRefData, unsigned char* pOutData, unsigned int OutDataLen, const char *msg, const char *fun)
{

    if(enc == 1) {
        mcp_dump_data_with_text(pRefData, OutDataLen, "Expect  : ");
        mcp_dump_data_with_text(pOutData, OutDataLen, "Encrypt : ");
    } else {
        mcp_dump_data_with_text(pOutData, OutDataLen, "Decrypt : ");
    }

    if (strncmp(pRefData, pOutData, OutDataLen) != 0 ) { /* output not equual to except output */
        mcp_info("\033[0;32;31m" "%s %s %s Fail!\n""\033[m", msg, fun, enc ? "Enc" : "Dec");
        return 1;
    } else {
        mcp_info("\033[0;32;34m" "%s %s %s Sucess!\n""\033[m" , msg, fun, enc ? "Enc" : "Dec");
    }

    return 0;
}



/*------------------------------------------------------------------
* Func : MCP_DES_ECB_Test
*
* Desc : DES ECB Encrypt Decrypt Test
*
* Parm : Void
*
* Retn : Fail Count
*------------------------------------------------------------------*/
int MCP_DES_ECB_Test(void)
{
    unsigned int   BlockSize = 16;
    unsigned char  _Data[16] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x00};
    unsigned char  _RefD[16] = {0xa3, 0x2a, 0xd8, 0xac, 0xd2, 0x0c, 0x38, 0xfc, 0x28, 0x51, 0x07, 0x7e, 0x16, 0xf6, 0x38, 0x75};
    unsigned char  Key[7];
    unsigned char  *EncData, *DecData;
    unsigned char  *Data, *RefD;
    int ret = 0;

    Data = kmalloc(sizeof(_Data), GFP_KERNEL);
    if (!Data) {
        return -ENOMEM;
    }

    RefD = kmalloc(sizeof(_RefD), GFP_KERNEL);
    if (!RefD) {
        ret = -ENOMEM;
        goto fail_RefD;
    }

    EncData = kmalloc(BlockSize, GFP_KERNEL);
    if (!EncData) {
        ret = -ENOMEM;
        goto fail_EncData;
    }

    DecData  = kmalloc(BlockSize, GFP_KERNEL);
    if (!DecData) {
        ret = -ENOMEM;
        goto fail_DecData;
    }

    memcpy(Data, _Data, sizeof(_Data));
    memcpy(RefD, _RefD, sizeof(_RefD));

    mcp_info("========================= DES ECB ===============================\n");
    mcp_dump_data_with_text(Data, BlockSize, "Input   : ");
    memset(Key, 0xff, sizeof(Key));
    memset(EncData, 0x00, BlockSize);
    memset(DecData, 0x00, BlockSize);

    MCP_DES_ECB_Encryption(Key, Data, EncData, BlockSize);
    ret += PrintCheckReuslt(1, RefD, EncData, BlockSize, "[1.1.1]", __FUNCTION__);

    MCP_DES_ECB_Decryption(Key, EncData, DecData, BlockSize);
    ret += PrintCheckReuslt(0, Data, DecData, BlockSize, "[1.1.2]", __FUNCTION__);

    kfree(DecData);
fail_DecData:
    kfree(EncData);
fail_EncData:
    kfree(RefD);
fail_RefD:
    kfree(Data);

    return ret;
}

/*------------------------------------------------------------------
* Func : MCP_DES_CBC_Test
*
* Desc : DES CBC Encrypt Decrypt Test
*
* Parm : Void
*
* Retn : Fail Count
*------------------------------------------------------------------*/
int MCP_DES_CBC_Test(void)
{
    unsigned int   BlockSize = 16;
    unsigned char  _Data[16] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x00};
    unsigned char  _RefD[16] = {0x03, 0xd3, 0x76, 0x73, 0xa2, 0x79, 0x50, 0xd1, 0x76, 0x7a, 0x78, 0x3c, 0x47, 0x5b, 0x6a, 0x0a};
    unsigned char  Key[7];
    unsigned char  IV[16];
    unsigned char  *EncData, *DecData;
    unsigned char  *Data, *RefD;
    int ret = 0;

    Data = kmalloc(sizeof(_Data), GFP_KERNEL);
    if (!Data) {
        return -ENOMEM;
    }

    RefD = kmalloc(sizeof(_RefD), GFP_KERNEL);
    if (!RefD) {
        ret = -ENOMEM;
        goto fail_RefD;
    }

    EncData = kmalloc(BlockSize, GFP_KERNEL);
    if (!EncData) {
        ret = -ENOMEM;
        goto fail_EncData;
    }

    DecData  = kmalloc(BlockSize, GFP_KERNEL);
    if (!DecData) {
        ret = -ENOMEM;
        goto fail_DecData;
    }

    memcpy(Data, _Data, sizeof(_Data));
    memcpy(RefD, _RefD, sizeof(_RefD));

    mcp_info("========================= DES CBC ===============================\n");
    mcp_dump_data_with_text(Data, BlockSize, "Input   : ");
    memset(Key, 0xff, sizeof(Key));
    memset(IV , 0x22, BlockSize);
    memset(EncData, 0x00, BlockSize);
    memset(DecData, 0x00, BlockSize);

    MCP_DES_CBC_Encryption(Key, IV, Data, EncData, BlockSize);
    ret += PrintCheckReuslt(1, RefD, EncData, BlockSize, "[1.2.1]", __FUNCTION__);

    MCP_DES_CBC_Decryption(Key, IV, EncData, DecData, BlockSize);
    ret += PrintCheckReuslt(0, Data, DecData, BlockSize, "[1.2.2]", __FUNCTION__);

    kfree(DecData);
fail_DecData:
    kfree(EncData);
fail_EncData:
    kfree(RefD);
fail_RefD:
    kfree(Data);

    return ret;
}

/*------------------------------------------------------------------
* Func : MCP_TDES_ECB_Test
*
* Desc : TDES ECB Encrypt Decrypt Test
*
* Parm : Void
*
* Retn : Fail Count
*------------------------------------------------------------------*/
int MCP_TDES_ECB_Test(void)
{
    unsigned int   BlockSize = 16;
    unsigned char  _Data[16] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x00};
    unsigned char  _RefD[16] = {0xa3, 0x2a, 0xd8, 0xac, 0xd2, 0x0c, 0x38, 0xfc, 0x28, 0x51, 0x07, 0x7e, 0x16, 0xf6, 0x38, 0x75};
    unsigned char  Key[21];
    unsigned char  *EncData, *DecData;
    unsigned char  *Data, *RefD;
    int ret = 0;

    Data = kmalloc(sizeof(_Data), GFP_KERNEL);
    if (!Data) {
        return -ENOMEM;
    }

    RefD = kmalloc(sizeof(_RefD), GFP_KERNEL);
    if (!RefD) {
        ret = -ENOMEM;
        goto fail_RefD;
    }

    EncData = kmalloc(BlockSize, GFP_KERNEL);
    if (!EncData) {
        ret = -ENOMEM;
        goto fail_EncData;
    }

    DecData  = kmalloc(BlockSize, GFP_KERNEL);
    if (!DecData) {
        ret = -ENOMEM;
        goto fail_DecData;
    }

    memcpy(Data, _Data, sizeof(_Data));
    memcpy(RefD, _RefD, sizeof(_RefD));

    mcp_info("========================= TDES ECB ==============================\n");
    mcp_dump_data_with_text(Data, BlockSize, "Input   : ");
    memset(Key, 0xff, sizeof(Key));
    memset(EncData, 0x00, BlockSize);
    memset(DecData, 0x00, BlockSize);

    MCP_TDES_ECB_Encryption(Key, Data, EncData, BlockSize);
    ret += PrintCheckReuslt(1, RefD, EncData, BlockSize, "[2.1.1]", __FUNCTION__);

    MCP_TDES_ECB_Decryption(Key, EncData, DecData, BlockSize);
    ret += PrintCheckReuslt(0, Data, DecData, BlockSize, "[2.1.2] ", __FUNCTION__);

    kfree(DecData);
fail_DecData:
    kfree(EncData);
fail_EncData:
    kfree(RefD);
fail_RefD:
    kfree(Data);

    return ret;
}

/*------------------------------------------------------------------
* Func : MCP_TDES_CBC_Test
*
* Desc : TDES CBC Encrypt Decrypt Test
*
* Parm : Void
*
* Retn : Fail Count
*------------------------------------------------------------------*/
int MCP_TDES_CBC_Test(void)
{
    unsigned int   BlockSize = 16;
    unsigned char  _Data[16] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x00};
    unsigned char  _RefD[16] = {0x03, 0xd3, 0x76, 0x73, 0xa2, 0x79, 0x50, 0xd1, 0x76, 0x7a, 0x78, 0x3c, 0x47, 0x5b, 0x6a, 0x0a};
    unsigned char  Key[21];
    unsigned char  IV[16];
    unsigned char  *EncData, *DecData;
    unsigned char  *Data, *RefD;
    int ret = 0;

    Data = kmalloc(sizeof(_Data), GFP_KERNEL);
    if (!Data) {
        return -ENOMEM;
    }

    RefD = kmalloc(sizeof(_RefD), GFP_KERNEL);
    if (!RefD) {
        ret = -ENOMEM;
        goto fail_RefD;
    }

    EncData = kmalloc(BlockSize, GFP_KERNEL);
    if (!EncData) {
        ret = -ENOMEM;
        goto fail_EncData;
    }

    DecData  = kmalloc(BlockSize, GFP_KERNEL);
    if (!DecData) {
        ret = -ENOMEM;
        goto fail_DecData;
    }

    memcpy(Data, _Data, sizeof(_Data));
    memcpy(RefD, _RefD, sizeof(_RefD));

    mcp_info("========================= TDES CBC ==============================\n");
    mcp_dump_data_with_text(Data, BlockSize, "Input   : ");
    memset(Key, 0xff, sizeof(Key));
    memset(IV , 0x22, BlockSize);
    memset(EncData, 0x00, BlockSize);
    memset(DecData, 0x00, BlockSize);

    MCP_TDES_CBC_Encryption(Key, IV, Data, EncData, BlockSize);
    ret += PrintCheckReuslt(1, RefD, EncData, BlockSize, "[2.2.1] ", __FUNCTION__);

    MCP_TDES_CBC_Decryption(Key, IV, EncData, DecData, BlockSize);
    ret += PrintCheckReuslt(0, Data, DecData, BlockSize, "[2.2.2]", __FUNCTION__);

    kfree(DecData);
fail_DecData:
    kfree(EncData);
fail_EncData:
    kfree(RefD);
fail_RefD:
    kfree(Data);

    return ret;
}


/*------------------------------------------------------------------
* Func : MCP_AES_128_ECB_Test
*
* Desc : AES-128 ECB Encrypt Decrypt Test
*
* Parm : Void
*
* Retn : Fail Count
*------------------------------------------------------------------*/
int MCP_AES_128_ECB_Test(void)
{
    unsigned int   BlockSize = 16;
    unsigned char  _Data[16] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x00};
    unsigned char  _RefD[16] = {0x45, 0xd7, 0x4d, 0xac, 0xb3, 0x52, 0xf9, 0x3f, 0xd3, 0x09, 0xbf, 0x8b, 0x91, 0x59, 0xdf, 0x58};
    unsigned char  Key[16];
    unsigned char  *EncData, *DecData;
    unsigned char  *Data, *RefD;
    int ret = 0;

    Data = kmalloc(sizeof(_Data), GFP_KERNEL);
    if (!Data) {
        return -ENOMEM;
    }

    RefD = kmalloc(sizeof(_RefD), GFP_KERNEL);
    if (!RefD) {
        ret = -ENOMEM;
        goto fail_RefD;
    }

    EncData = kmalloc(BlockSize, GFP_KERNEL);
    if (!EncData) {
        ret = -ENOMEM;
        goto fail_EncData;
    }

    DecData  = kmalloc(BlockSize, GFP_KERNEL);
    if (!DecData) {
        ret = -ENOMEM;
        goto fail_DecData;
    }

    memcpy(Data, _Data, sizeof(_Data));
    memcpy(RefD, _RefD, sizeof(_RefD));

    mcp_info("========================== AES-128 ECB ==========================\n");
    mcp_dump_data_with_text(Data, BlockSize, "Input   : ");
    memset(Key, 0xff, sizeof(Key));
    memset(EncData, 0x00, BlockSize);
    memset(DecData, 0x00, BlockSize);

    MCP_AES_ECB_Encryption(Key, Data, EncData, BlockSize);
    ret += PrintCheckReuslt(1, RefD, EncData, BlockSize, "[3.1.1]", __FUNCTION__);

    MCP_AES_ECB_Decryption(Key, EncData, DecData, BlockSize);
    ret += PrintCheckReuslt(0, Data, DecData, BlockSize, "[3.1.2]", __FUNCTION__);

    kfree(DecData);
fail_DecData:
    kfree(EncData);
fail_EncData:
    kfree(RefD);
fail_RefD:
    kfree(Data);

    return ret;
}

/*------------------------------------------------------------------
* Func : MCP_AES_128_CBC_Test
*
* Desc : AES-128 CBC Encrypt Decrypt Test
*
* Parm : Void
*
* Retn : Fail Count
*------------------------------------------------------------------*/
int MCP_AES_128_CBC_Test(void)
{
    unsigned int   BlockSize = 16;
    unsigned char  _Data[16] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x00};
    unsigned char  _RefD[16] = {0x9a, 0x6c, 0x86, 0xce, 0xc3, 0x31, 0xdd, 0xb0, 0x14, 0x2c, 0xfb, 0x37, 0x06, 0x25, 0xbf, 0xfe};
    unsigned char  Key[16];
    unsigned char  IV[16];
    unsigned char  *EncData, *DecData;
    unsigned char  *Data, *RefD;
    int ret = 0;

    Data = kmalloc(sizeof(_Data), GFP_KERNEL);
    if (!Data) {
        return -ENOMEM;
    }

    RefD = kmalloc(sizeof(_RefD), GFP_KERNEL);
    if (!RefD) {
        ret = -ENOMEM;
        goto fail_RefD;
    }

    EncData = kmalloc(BlockSize, GFP_KERNEL);
    if (!EncData) {
        ret = -ENOMEM;
        goto fail_EncData;
    }

    DecData  = kmalloc(BlockSize, GFP_KERNEL);
    if (!DecData) {
        ret = -ENOMEM;
        goto fail_DecData;
    }

    memcpy(Data, _Data, sizeof(_Data));
    memcpy(RefD, _RefD, sizeof(_RefD));

    mcp_info("========================== AES-128 CBC ==========================\n");
    mcp_dump_data_with_text(Data, BlockSize, "Input   : ");
    memset(Key, 0xff, sizeof(Key));
    memset(IV , 0x22, BlockSize);
    memset(EncData, 0x00, BlockSize);
    memset(DecData, 0x00, BlockSize);

    MCP_AES_CBC_Encryption(Key, IV, Data, EncData, BlockSize);
    ret += PrintCheckReuslt(1, RefD, EncData, BlockSize, "[3.2.1]", __FUNCTION__);

    MCP_AES_CBC_Decryption(Key, IV, EncData, DecData, BlockSize);
    ret += PrintCheckReuslt(0, Data, DecData, BlockSize, "[3.2.2]", __FUNCTION__);

    kfree(DecData);
fail_DecData:
    kfree(EncData);
fail_EncData:
    kfree(RefD);
fail_RefD:
    kfree(Data);

    return ret;
}

/*------------------------------------------------------------------
* Func : MCP_AES_128_CTR_CW_Test
*
* Desc : AES-128 CTR Encrypt Decrypt Test
*
* Parm : Void
*
* Retn : Fail Count
*------------------------------------------------------------------*/
int MCP_AES_128_CTR_Test(void)
{
    unsigned int   BlockSize = 16;
    unsigned char  _Data[16] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x00};
    unsigned char  _RefD[16] = {0xd7, 0x9d, 0xe0, 0x8a, 0xbe, 0xa9, 0xa2, 0xc8, 0xf7, 0xf1, 0x85, 0xc7, 0x72, 0x57, 0x2e, 0xb5};
    unsigned char  Key[16];
    unsigned char  IV[16];
    unsigned char  *EncData, *DecData;
    unsigned char  *Data, *RefD;
    int ret = 0;

    Data = kmalloc(sizeof(_Data), GFP_KERNEL);
    if (!Data) {
        return -ENOMEM;
    }

    RefD = kmalloc(sizeof(_RefD), GFP_KERNEL);
    if (!RefD) {
        ret = -ENOMEM;
        goto fail_RefD;
    }

    EncData = kmalloc(BlockSize, GFP_KERNEL);
    if (!EncData) {
        ret = -ENOMEM;
        goto fail_EncData;
    }

    DecData  = kmalloc(BlockSize, GFP_KERNEL);
    if (!DecData) {
        ret = -ENOMEM;
        goto fail_DecData;
    }

    memcpy(Data, _Data, sizeof(_Data));
    memcpy(RefD, _RefD, sizeof(_RefD));

    mcp_info("========================= AES-128 CTR ===========================\n");
    mcp_dump_data_with_text(Data, BlockSize, "Input   : ");
    memset(Key, 0xff, sizeof(Key));
    memset(IV , 0x22, BlockSize);
    memset(EncData, 0x00, BlockSize);
    memset(DecData, 0x00, BlockSize);

    MCP_AES_CTR_Encryption(Key, IV, Data, EncData, BlockSize);
    ret += PrintCheckReuslt(1, RefD, EncData, BlockSize, "[3.3.1]", __FUNCTION__);

    MCP_AES_CTR_Decryption(Key, IV, EncData, DecData, BlockSize);
    ret += PrintCheckReuslt(0, Data, DecData, BlockSize, "[3.3.2]", __FUNCTION__);

    kfree(DecData);
fail_DecData:
    kfree(EncData);
fail_EncData:
    kfree(RefD);
fail_RefD:
    kfree(Data);

    return ret;
}

#ifdef RTK_MCP_PATTERN_TEST
void AES_PATTERN_GEN(int crypt, int skip, int mode, int len,
                     uint8_t* key, uint8_t* iv, uint8_t* in, uint8_t* out, unsigned char lastiv[16])
{
    int pattern_len = crypt + skip;
    int count = len / (pattern_len * 16);
    int partial = len - count * pattern_len * 16;
    int i = 0;
    for (i = 0; i < count; i++) {
        if (mode == MCP_BCM_CBC) {
            MCP_AES_CBC_Encryption(key, iv, (in + pattern_len * i * 16),
                                   (out + pattern_len * i * 16), crypt * 16);
            MCP_AES_PeekIV(iv);
        }
        if (mode == MCP_BCM_CTR) {
            MCP_AES_CTR_Encryption(key, iv, (in + pattern_len * i * 16),
                                   (out + pattern_len * i * 16), crypt * 16);
            MCP_AES_PeekIV(iv);
        }
        memcpy(out + (pattern_len * i + crypt) * 16,
               in + (pattern_len * i + crypt) * 16, skip * 16);
    }
    if (partial) {
        partial = partial > crypt * 16 ? crypt * 16 : partial;
        partial = partial & (~0xf);
        if (partial) {
            if (mode == MCP_BCM_CBC) {
                MCP_AES_CBC_Encryption(key, iv, (in + pattern_len * i * 16),
                                       (out + pattern_len * i * 16), partial);
                MCP_AES_PeekIV(iv);
            }
            if (mode == MCP_BCM_CTR) {
                MCP_AES_CTR_Encryption(key, iv, (in + pattern_len * i * 16),
                                             (out + pattern_len * i * 16), partial);
                MCP_AES_PeekIV(iv);
            }
        }
        if (len - count * pattern_len * 16 - partial) {
            memcpy(out + (pattern_len * i) * 16 + partial,
                   in + (pattern_len * i) * 16 + partial, len - count * pattern_len * 16 - partial);
        }
    }
    memcpy(lastiv, iv, 16);
}


static const uint8_t ciph_data_128_iv1[] = {
    0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, /* 12345678 */
    0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x30, /* 9ABCDEF0 */
};

static const uint8_t ciph_data_aes_key1[] = {
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, /* 01234567 */
    0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, /* 89ABCDEF */
};

int MCP_PATTERN_Test(int BCM, int len)
{
    int i = 0;
    unsigned char *buf = mcp_malloc(len * 3);
    unsigned char *in;
    unsigned char *enc;
    unsigned char *dec;
    unsigned char  enciv[16];
    unsigned char  deciv[16];
    int ret = 0;

    if (buf == NULL) {
        mcp_info("%s malloc fail\n",__func__);
        return 1;
    }
    in = buf;
    enc = in + len;
    dec = enc + len;
    for (i = 1; i < 3; i++) {
        unsigned char iv[16];
        memcpy(iv, ciph_data_128_iv1, 16);
        memset(in, 0xff, len);
        memset(enc, 0x0, len);
        memset(dec, 0x0, len);
        AES_PATTERN_GEN(i, 10 - i, BCM, len, (uint8_t*)ciph_data_aes_key1,
                        (uint8_t*)iv, in, enc, enciv);
        memcpy(iv, ciph_data_128_iv1, 16);
        MCP_AES_Pattern_Decryption(BCM, (uint8_t*)ciph_data_aes_key1, iv, enc,
                                   dec, len, i, 10 - i);
        MCP_AES_PeekIV(deciv);
        ret += PrintCheckReuslt(0, in, dec, len, "[dec]", __FUNCTION__);

        ret += PrintCheckReuslt(1, (unsigned char *)enciv, (unsigned char *)deciv, 16, "[iv]", __FUNCTION__);
    }
    if (buf != NULL)
        mcp_free(buf,3*len);

    return ret;
}

int MCP_PATTERN_DADR_Test(int BCM, int len1, int len2)
{
    int i = 0;
    int len = len1 + len2;
    unsigned char *buf = mcp_malloc(len * 3 + 32 + 32);
    unsigned char *in;
    unsigned char *enc;

    unsigned char *dec3;
    unsigned char *dec4;
    unsigned char *dec3_8;
    unsigned char *dec4_16;
    unsigned char* dec[2];
    unsigned char  piv[16];
    unsigned long  plen[2];
    unsigned char  enciv[16];
    unsigned char  deciv[16];
    int ret = 0;

    if (buf == NULL) {
        mcp_info("%s malloc fail\n",__func__);
        return 1;
    }
    in = buf;
    enc = buf + len;
    dec3 = enc + len;
    dec4 = dec3 + len1 + 32;
    dec3_8 = (unsigned char *)(((unsigned long)dec3 / 8 + 1) * 8);
    dec4_16 = (unsigned char *)(((unsigned long)dec4 / 16 + 1) * 16);
    plen[0] = len1;
    plen[1] = len2;

    for (i = 1; i < 3; i++) {
        memcpy(piv, (unsigned char*)ciph_data_128_iv1, 16);
        memset(in, 0xff, len);
        memset(enc, 0x0, len);
        AES_PATTERN_GEN(i, 10 - i, BCM, len, (uint8_t*)ciph_data_aes_key1,
                        (uint8_t*)piv, in, enc, enciv);
        memset(dec3_8, 0x0, len1);
        memset(dec4_16, 0x0, len2);

        dec[0] = (unsigned char*)dec3_8;
        dec[1] = (unsigned char*)dec4_16;
        memcpy(piv, (unsigned char*)ciph_data_128_iv1, 16);
        MCP_AES_Pattern_Decryption_DoubleDADR(BCM, (uint8_t*)ciph_data_aes_key1, piv, enc,
                                              dec, plen, i, 10 - i);

        ret += PrintCheckReuslt(0, in, dec3_8, len1, "[dec3_8]", __FUNCTION__);
        ret += PrintCheckReuslt(0, in + len1, dec4_16, len2, "[dec4_16]", __FUNCTION__);

        MCP_AES_PeekIV(deciv);
        ret += PrintCheckReuslt(1, (unsigned char *)enciv, (unsigned char *)deciv, 16, "[iv]", __FUNCTION__);

    }
    if (buf != NULL)
        mcp_free(buf,len * 3 + 32 + 32);

    return ret;

}
#endif
#ifdef CONFIG_MCP_SCPU_OTP_KEY_REMOVED
int MCP_AES_128_ECB_WITH_SCPU_OTP_KEY_REMOVED_Test(void)
{
    unsigned int   BlockSize = 16;
    unsigned char  _Data[16] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x00};
    unsigned char  _RefD[16] = {0x45, 0xd7, 0x4d, 0xac, 0xb3, 0x52, 0xf9, 0x3f, 0xd3, 0x09, 0xbf, 0x8b, 0x91, 0x59, 0xdf, 0x58};
    unsigned char  Key[16];
    unsigned char  *EncData, *DecData;
    unsigned char  *Data, *RefD;
    int ret = 0;

    Data = kmalloc(sizeof(_Data), GFP_KERNEL);
    if (!Data) {
        return -ENOMEM;
    }

    RefD = kmalloc(sizeof(_RefD), GFP_KERNEL);
    if (!RefD) {
        ret = -ENOMEM;
        goto fail_RefD;
    }

    EncData = kmalloc(BlockSize, GFP_KERNEL);
    if (!EncData) {
        ret = -ENOMEM;
        goto fail_EncData;
    }

    DecData  = kmalloc(BlockSize, GFP_KERNEL);
    if (!DecData) {
        ret = -ENOMEM;
        goto fail_DecData;
    }

    memcpy(Data, _Data, sizeof(_Data));
    memcpy(RefD, _RefD, sizeof(_RefD));

    mcp_info("========================== MCP_AES_128_ECB_WITH_SCPU_OTP_KEY_REMOVED_Test ==========================\n");
    memset(Key, 0xff, sizeof(Key));
    memset(EncData, 0x00, BlockSize);
    memset(DecData, 0x00, BlockSize);

    if(MCP_AES_ECB_Encryption(NULL, Data, EncData, BlockSize) != -1)
        ret+= 1;
    
    if(MCP_AES_ECB_Decryption(NULL, EncData, DecData, BlockSize) != -1)
        ret += 1;

    kfree(DecData);
fail_DecData:
    kfree(EncData);
fail_EncData:
    kfree(RefD);
fail_RefD:
    kfree(Data);

    return ret;
}
#endif

/*------------------------------------------------------------------
* Func : MCP_AES_192_ECB_Test
*
* Desc : AES-192 ECB Encrypt Decrypt Test
*
* Parm : Void
*
* Retn : Fail Count
*------------------------------------------------------------------*/
int MCP_AES_192_ECB_Test(void)
{
    unsigned int   BlockSize = 16;
    unsigned char  _Data[16] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x00};
    unsigned char  _RefD[16] = {0x3D, 0xA3, 0x68, 0x1F, 0x1E, 0xFE, 0x06, 0xB1, 0xCF, 0x3A, 0x89, 0x66, 0x04, 0x7B, 0x2F, 0x63};
    unsigned char  Key[24] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0x00, 0x11, 0x22, 0x33, 0x44};
    unsigned char  *EncData, *DecData;
    unsigned char  *Data, *RefD;
    int ret = 0;

    Data = kmalloc(sizeof(_Data), GFP_KERNEL);
    if (!Data) {
        return -ENOMEM;
    }

    RefD = kmalloc(sizeof(_RefD), GFP_KERNEL);
    if (!RefD) {
        ret = -ENOMEM;
        goto fail_RefD;
    }

    EncData = kmalloc(BlockSize, GFP_KERNEL);
    if (!EncData) {
        ret = -ENOMEM;
        goto fail_EncData;
    }

    DecData  = kmalloc(BlockSize, GFP_KERNEL);
    if (!DecData) {
        ret = -ENOMEM;
        goto fail_DecData;
    }

    memcpy(Data, _Data, sizeof(_Data));
    memcpy(RefD, _RefD, sizeof(_RefD));

    mcp_info("========================== AES-192 DDR ECB ======================\n");
    mcp_dump_data_with_text(Data, BlockSize, "Input   : ");
    memset(EncData, 0x00, BlockSize);
    memset(DecData, 0x00, BlockSize);

    MCP_AES_192_ECB_Encryption(Key, Data, EncData, BlockSize);
    ret += PrintCheckReuslt(1, RefD, EncData, BlockSize, "[4.1.1]", __FUNCTION__);

    MCP_AES_192_ECB_Decryption(Key, EncData, DecData, BlockSize);
    ret += PrintCheckReuslt(0, Data, DecData, BlockSize, "[4.1.2]", __FUNCTION__);

    kfree(DecData);
fail_DecData:
    kfree(EncData);
fail_EncData:
    kfree(RefD);
fail_RefD:
    kfree(Data);

    return ret;
}
/*------------------------------------------------------------------
* Func : MCP_AES_192_CBC_Test
*
* Desc : AES-192 CBC Encrypt Decrypt Test
*
* Parm : Void
*
* Retn : Fail Count
*------------------------------------------------------------------*/
int MCP_AES_192_CBC_Test(void)
{
    unsigned int   BlockSize = 16;
    unsigned char  _Data[16] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x00};
    unsigned char  _RefD[16] = {0x08, 0x1C, 0x30, 0xEA, 0xA6, 0xED, 0x9E, 0x93, 0x02, 0x59, 0xDB, 0x27, 0xF0, 0x60, 0xC5, 0x31};
    unsigned char  Key[24] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66,
                              0x77, 0x88, 0x99, 0x00, 0x11, 0x22, 0x33, 0x44
                             };
    unsigned char  IV[16] = {0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0xaa, 0xbb, 0xcc, 0xdd};
    unsigned char  *EncData, *DecData;
    unsigned char  *Data, *RefD;
    int ret = 0;

    Data = kmalloc(sizeof(_Data), GFP_KERNEL);
    if (!Data) {
        return -ENOMEM;
    }

    RefD = kmalloc(sizeof(_RefD), GFP_KERNEL);
    if (!RefD) {
        ret = -ENOMEM;
        goto fail_RefD;
    }

    EncData = kmalloc(BlockSize, GFP_KERNEL);
    if (!EncData) {
        ret = -ENOMEM;
        goto fail_EncData;
    }

    DecData  = kmalloc(BlockSize, GFP_KERNEL);
    if (!DecData) {
        ret = -ENOMEM;
        goto fail_DecData;
    }

    memcpy(Data, _Data, sizeof(_Data));
    memcpy(RefD, _RefD, sizeof(_RefD));

    mcp_info("========================== AES-192 DDR CBC ======================\n");
    mcp_dump_data_with_text(Data, BlockSize, "Input   : ");
    memset(EncData, 0x00, BlockSize);
    memset(DecData, 0x00, BlockSize);

    MCP_AES_192_CBC_Encryption(Key, IV, Data, EncData, BlockSize);
    ret += PrintCheckReuslt(1, RefD, EncData, BlockSize, "[4.2.1]", __FUNCTION__);

    MCP_AES_192_CBC_Decryption(Key, IV, EncData, DecData, BlockSize);
    ret += PrintCheckReuslt(0, Data, DecData, BlockSize, "[4.2.2]", __FUNCTION__);

    kfree(DecData);
fail_DecData:
    kfree(EncData);
fail_EncData:
    kfree(RefD);
fail_RefD:
    kfree(Data);

    return ret;
}


/*------------------------------------------------------------------
* Func : MCP_AES_192_CTR_Test
*
* Desc : AES-192 CTR Encrypt Decrypt Test
*
* Parm : Void
*
* Retn : Fail Count
*------------------------------------------------------------------*/
int MCP_AES_192_CTR_Test(void)
{
    unsigned int   BlockSize = 16;
    unsigned char  _Data[16] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x00};
    unsigned char  _RefD[16] = {0x9f, 0xe3, 0xc0, 0xbc, 0xa6, 0x65, 0x19, 0x89, 0xbd, 0x96, 0x2e, 0xb5, 0xf8, 0x46, 0xac, 0x31};
    unsigned char  Key[24] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66,
                              0x77, 0x88, 0x99, 0x00, 0x11, 0x22, 0x33, 0x44
                             };
    unsigned char  IV[16];
    unsigned char  *EncData, *DecData;
    unsigned char  *Data, *RefD;
    int ret = 0;

    Data = kmalloc(sizeof(_Data), GFP_KERNEL);
    if (!Data) {
        return -ENOMEM;
    }

    RefD = kmalloc(sizeof(_RefD), GFP_KERNEL);
    if (!RefD) {
        ret = -ENOMEM;
        goto fail_RefD;
    }

    EncData = kmalloc(BlockSize, GFP_KERNEL);
    if (!EncData) {
        ret = -ENOMEM;
        goto fail_EncData;
    }

    DecData  = kmalloc(BlockSize, GFP_KERNEL);
    if (!DecData) {
        ret = -ENOMEM;
        goto fail_DecData;
    }

    memcpy(Data, _Data, sizeof(_Data));
    memcpy(RefD, _RefD, sizeof(_RefD));

    mcp_info("========================== AES-192 DDR CTR ======================\n");
    mcp_dump_data_with_text(Data, BlockSize, "Input   : ");
    memset(IV , 0x22, BlockSize);
    memset(EncData, 0x00, BlockSize);
    memset(DecData, 0x00, BlockSize);

    MCP_AES_192_CTR_Encryption(Key, IV, Data, EncData, BlockSize);
    ret += PrintCheckReuslt(1, RefD, EncData, BlockSize, "[4.3.1]", __FUNCTION__);

    MCP_AES_192_CTR_Decryption(Key, IV, EncData, DecData, BlockSize);
    ret += PrintCheckReuslt(0, Data, DecData, BlockSize, "[4.3.2]", __FUNCTION__);

    kfree(DecData);
fail_DecData:
    kfree(EncData);
fail_EncData:
    kfree(RefD);
fail_RefD:
    kfree(Data);

    return ret;
}
/*------------------------------------------------------------------
* Func : MCP_AES_256_ECB_Test
*
* Desc : AES-256 ECB Encrypt Decrypt Test
*
* Parm : Void
*
* Retn : Fail Count
*------------------------------------------------------------------*/
int MCP_AES_256_ECB_Test(void)
{
    unsigned int   BlockSize = 16;
    unsigned char  _Data[16] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x00};
    unsigned char  _RefD[16] = {0x76, 0xFB, 0xD8, 0x07, 0x6B, 0x21, 0x20, 0x82, 0xD5, 0x8D, 0xE6, 0x97, 0x3A, 0x17, 0x1C, 0x5D};
    unsigned char  Key[32] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66,
                              0x77, 0x88, 0x99, 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0x00, 0x11, 0x22
                             };
    unsigned char  *EncData, *DecData;
    unsigned char  *Data, *RefD;
    int ret = 0;

    Data = kmalloc(sizeof(_Data), GFP_KERNEL);
    if (!Data) {
        return -ENOMEM;
    }

    RefD = kmalloc(sizeof(_RefD), GFP_KERNEL);
    if (!RefD) {
        ret = -ENOMEM;
        goto fail_RefD;
    }

    EncData = kmalloc(BlockSize, GFP_KERNEL);
    if (!EncData) {
        ret = -ENOMEM;
        goto fail_EncData;
    }

    DecData  = kmalloc(BlockSize, GFP_KERNEL);
    if (!DecData) {
        ret = -ENOMEM;
        goto fail_DecData;
    }

    memcpy(Data, _Data, sizeof(_Data));
    memcpy(RefD, _RefD, sizeof(_RefD));

    mcp_info("========================== AES-256 DDR ECB ======================\n");
    mcp_dump_data_with_text(Data, BlockSize, "Input   : ");
    memset(EncData, 0x00, BlockSize);
    memset(DecData, 0x00, BlockSize);

    MCP_AES_256_ECB_Encryption(Key, Data, EncData, BlockSize);
    ret += PrintCheckReuslt(1, RefD, EncData, BlockSize, "[5.1.1]", __FUNCTION__);

    MCP_AES_256_ECB_Decryption(Key, EncData, DecData, BlockSize);
    ret += PrintCheckReuslt(0, Data, DecData, BlockSize, "[5.1.2]", __FUNCTION__);

    kfree(DecData);
fail_DecData:
    kfree(EncData);
fail_EncData:
    kfree(RefD);
fail_RefD:
    kfree(Data);

    return ret;
}
/*------------------------------------------------------------------
* Func : MCP_AES_256_CBC_Test
*
* Desc : AES-256 CBC Encrypt Decrypt Test
*
* Parm : Void
*
* Retn : Fail Count
*------------------------------------------------------------------*/
int MCP_AES_256_CBC_Test(void)
{
    unsigned int   BlockSize = 16;
    unsigned char  _Data[16] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x00};
    unsigned char  _RefD[16] = {0xA5, 0xBD, 0x95, 0xA8, 0xD4, 0x59, 0xEC, 0xCE, 0x55, 0xD0, 0xE3, 0xBC, 0xF7, 0x98, 0xC2, 0xC0};
    unsigned char  Key[32] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66,
                              0x77, 0x88, 0x99, 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0x00, 0x11, 0x22
                             };
    unsigned char  IV[16] = {0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0xaa, 0xbb, 0xcc, 0xdd};
    unsigned char  *EncData, *DecData;
    unsigned char  *Data, *RefD;
    int ret = 0;

    Data = kmalloc(sizeof(_Data), GFP_KERNEL);
    if (!Data) {
        return -ENOMEM;
    }

    RefD = kmalloc(sizeof(_RefD), GFP_KERNEL);
    if (!RefD) {
        ret = -ENOMEM;
        goto fail_RefD;
    }

    EncData = kmalloc(BlockSize, GFP_KERNEL);
    if (!EncData) {
        ret = -ENOMEM;
        goto fail_EncData;
    }

    DecData  = kmalloc(BlockSize, GFP_KERNEL);
    if (!DecData) {
        ret = -ENOMEM;
        goto fail_DecData;
    }

    memcpy(Data, _Data, sizeof(_Data));
    memcpy(RefD, _RefD, sizeof(_RefD));

    mcp_info("========================== AES-256 DDR CBC ======================\n");
    mcp_dump_data_with_text(Data, BlockSize, "Input   : ");
    memset(EncData, 0x00, BlockSize);
    memset(DecData, 0x00, BlockSize);

    MCP_AES_256_CBC_Encryption(Key, IV, Data, EncData, BlockSize);
    ret += PrintCheckReuslt(1, RefD, EncData, BlockSize, "[5.2.1]", __FUNCTION__);

    MCP_AES_256_CBC_Decryption(Key, IV, EncData, DecData, BlockSize);
    ret += PrintCheckReuslt(0, Data, DecData, BlockSize, "[5.2.2]", __FUNCTION__);

    kfree(DecData);
fail_DecData:
    kfree(EncData);
fail_EncData:
    kfree(RefD);
fail_RefD:
    kfree(Data);

    return ret;
}
/*------------------------------------------------------------------
* Func : MCP_AES_256_CTR_Test
*
* Desc : AES-256 CTR Encrypt Decrypt Test
*
* Parm : Void
*
* Retn : Fail Count
*------------------------------------------------------------------*/
int MCP_AES_256_CTR_Test(void)
{
    unsigned int   BlockSize = 16;
    unsigned char  _Data[16] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x00};
    unsigned char  _RefD[16] = {0xf5, 0x89, 0xa3, 0xe5, 0x91, 0x82, 0x02, 0x08, 0xa0, 0xea, 0xb1, 0x81, 0x66, 0x7e, 0x20, 0x04};
    unsigned char  Key[32];
    unsigned char  IV[16];
    unsigned char  *EncData, *DecData;
    unsigned char  *Data, *RefD;
    int ret = 0;

    Data = kmalloc(sizeof(_Data), GFP_KERNEL);
    if (!Data) {
        return -ENOMEM;
    }

    RefD = kmalloc(sizeof(_RefD), GFP_KERNEL);
    if (!RefD) {
        ret = -ENOMEM;
        goto fail_RefD;
    }

    EncData = kmalloc(BlockSize, GFP_KERNEL);
    if (!EncData) {
        ret = -ENOMEM;
        goto fail_EncData;
    }

    DecData  = kmalloc(BlockSize, GFP_KERNEL);
    if (!DecData) {
        ret = -ENOMEM;
        goto fail_DecData;
    }

    memcpy(Data, _Data, sizeof(_Data));
    memcpy(RefD, _RefD, sizeof(_RefD));

    mcp_info("========================== AES-256 DDR CTR ======================\n");
    mcp_dump_data_with_text(Data, BlockSize, "Input   : ");
    memset(Key, 0xff, sizeof(Key));
    memset(IV , 0x22, BlockSize);
    memset(EncData, 0x00, BlockSize);
    memset(DecData, 0x00, BlockSize);

    MCP_AES_256_CTR_Encryption(Key, IV, Data, EncData, BlockSize);
    ret += PrintCheckReuslt(1, RefD, EncData, BlockSize, "[5.3.1]", __FUNCTION__);

    MCP_AES_256_CTR_Decryption(Key, IV, EncData, DecData, BlockSize);
    ret += PrintCheckReuslt(0, Data, DecData, BlockSize, "[5.3.2]", __FUNCTION__);

    kfree(DecData);
fail_DecData:
    kfree(EncData);
fail_EncData:
    kfree(RefD);
fail_RefD:
    kfree(Data);

    return ret;
}


/*------------------------------------------------------------------
* Func : MCP_CAM_128_ECB_Test
*
* Desc : CAM-128 ECB Encrypt Decrypt Test
*
* Parm : Void
*
* Retn : Fail Count
*------------------------------------------------------------------*/

int MCP_CAM_128_ECB_Test(void)
{
    unsigned int   BlockSize = 16;
    unsigned char  _Data[16] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x00};
    //unsigned char  _RefD[16] = {0x45, 0xd7, 0x4d, 0xac, 0xb3, 0x52, 0xf9, 0x3f, 0xd3, 0x09, 0xbf, 0x8b, 0x91, 0x59, 0xdf, 0x58};
    unsigned char  _RefD[16] = {0x4C, 0xE6, 0x0E, 0x91, 0x3F, 0x30, 0xAD, 0x46, 0x57, 0x58, 0x2B, 0x5F, 0x9D, 0x46, 0x77, 0x1D}; //camellia
    unsigned char  Key[16];
    unsigned char  *EncData, *DecData;
    unsigned char  *Data, *RefD;
    int ret = 0;

    Data = kmalloc(sizeof(_Data), GFP_KERNEL);
    if (!Data) {
        return -ENOMEM;
    }

    RefD = kmalloc(sizeof(_RefD), GFP_KERNEL);
    if (!RefD) {
        ret = -ENOMEM;
        goto fail_RefD;
    }

    EncData = kmalloc(BlockSize, GFP_KERNEL);
    if (!EncData) {
        ret = -ENOMEM;
        goto fail_EncData;
    }

    DecData  = kmalloc(BlockSize, GFP_KERNEL);
    if (!DecData) {
        ret = -ENOMEM;
        goto fail_DecData;
    }

    memcpy(Data, _Data, sizeof(_Data));
    memcpy(RefD, _RefD, sizeof(_RefD));

    mcp_info("========================= CAM-128 ECB ==========================\n");
    mcp_dump_data_with_text(Data, BlockSize, "Input   : ");
    memset(Key, 0xff, sizeof(Key));
    memset(EncData, 0x00, BlockSize);
    memset(DecData, 0x00, BlockSize);

    MCP_CAM_ECB_Encryption(Key, Data, EncData, BlockSize);
    ret += PrintCheckReuslt(1, RefD, EncData, BlockSize, "[3.1.1]", __FUNCTION__);

    MCP_CAM_ECB_Decryption(Key, EncData, DecData, BlockSize);
    ret += PrintCheckReuslt(0, Data, DecData, BlockSize, "[3.1.2]", __FUNCTION__);

    kfree(DecData);
fail_DecData:
    kfree(EncData);
fail_EncData:
    kfree(RefD);
fail_RefD:
    kfree(Data);

    return ret;
}

/*------------------------------------------------------------------
* Func : MCP_CAM_128_CBC_Test
*
* Desc : CAM-128 CBC Encrypt Decrypt Test
*
* Parm : Void
*
* Retn : Fail Count
*------------------------------------------------------------------*/
int MCP_CAM_128_CBC_Test(void)
{
    unsigned int   BlockSize = 16;
    unsigned char _Data[16] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x00};
    //unsigned char  _RefD[16] = {0x9a, 0x6c, 0x86, 0xce, 0xc3, 0x31, 0xdd, 0xb0, 0x14, 0x2c, 0xfb, 0x37, 0x06, 0x25, 0xbf, 0xfe};
    unsigned char  _RefD[16] = {0xE9, 0x1F, 0xF6, 0x46, 0x88, 0x64, 0x1F, 0x57, 0xF4, 0xEB, 0x2A, 0xCC, 0x7B, 0x7B, 0x6E, 0x59}; //camellia
    unsigned char  Key[16];
    unsigned char  IV[16];
    unsigned char  *EncData, *DecData;
    unsigned char  *Data, *RefD;
    int ret = 0;

    Data = kmalloc(sizeof(_Data), GFP_KERNEL);
    if (!Data) {
        return -ENOMEM;
    }

    RefD = kmalloc(sizeof(_RefD), GFP_KERNEL);
    if (!RefD) {
        ret = -ENOMEM;
        goto fail_RefD;
    }

    EncData = kmalloc(BlockSize, GFP_KERNEL);
    if (!EncData) {
        ret = -ENOMEM;
        goto fail_EncData;
    }

    DecData  = kmalloc(BlockSize, GFP_KERNEL);
    if (!DecData) {
        ret = -ENOMEM;
        goto fail_DecData;
    }

    memcpy(Data, _Data, sizeof(_Data));
    memcpy(RefD, _RefD, sizeof(_RefD));

    mcp_info("========================== CAM-128 CBC ==========================\n");
    mcp_dump_data_with_text(Data, BlockSize, "Input   : ");
    memset(Key, 0xff, sizeof(Key));
    memset(IV , 0x22, BlockSize);
    memset(EncData, 0x00, BlockSize);
    memset(DecData, 0x00, BlockSize);

    MCP_CAM_CBC_Encryption(Key, IV, Data, EncData, BlockSize);
    ret += PrintCheckReuslt(1, RefD, EncData, BlockSize, "[3.2.1]", __FUNCTION__);

    MCP_CAM_CBC_Decryption(Key, IV, EncData, DecData, BlockSize);
    ret += PrintCheckReuslt(0, Data, DecData, BlockSize, "[3.2.2]", __FUNCTION__);

    kfree(DecData);
fail_DecData:
    kfree(EncData);
fail_EncData:
    kfree(RefD);
fail_RefD:
    kfree(Data);

    return ret;
}

/*------------------------------------------------------------------
* Func : MCP_CAM_128_CTR_Test
*
* Desc : CAM-128 CTR Encrypt Decrypt Test
*
* Parm : Void
*
* Retn : Fail Count
*------------------------------------------------------------------*/
int MCP_CAM_128_CTR_Test(void)
{
    unsigned int   BlockSize = 16;
    unsigned char  _Data[16] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x00};
    //unsigned char  _RefD[16] = {0xd7, 0x9d, 0xe0, 0x8a, 0xbe, 0xa9, 0xa2, 0xc8, 0xf7, 0xf1, 0x85, 0xc7, 0x72, 0x57, 0x2e, 0xb5};
    unsigned char  _RefD[16] = {0xC8, 0xE4, 0xA6, 0x1B, 0x84, 0x8D, 0x8E, 0x32, 0xC0, 0x91, 0x34, 0x83, 0x6C, 0x91, 0x34, 0x31}; //camellia
    unsigned char  Key[16];
    unsigned char  IV[16];
    unsigned char  *EncData, *DecData;
    unsigned char  *Data, *RefD;
    int ret = 0;

    Data = kmalloc(sizeof(_Data), GFP_KERNEL);
    if (!Data) {
        return -ENOMEM;
    }

    RefD = kmalloc(sizeof(_RefD), GFP_KERNEL);
    if (!RefD) {
        ret = -ENOMEM;
        goto fail_RefD;
    }

    EncData = kmalloc(BlockSize, GFP_KERNEL);
    if (!EncData) {
        ret = -ENOMEM;
        goto fail_EncData;
    }

    DecData  = kmalloc(BlockSize, GFP_KERNEL);
    if (!DecData) {
        ret = -ENOMEM;
        goto fail_DecData;
    }

    memcpy(Data, _Data, sizeof(_Data));
    memcpy(RefD, _RefD, sizeof(_RefD));

    mcp_info("========================= CAM-128 CTR ===========================\n");
    mcp_dump_data_with_text(Data, BlockSize, "Input   : ");
    memset(Key, 0xff, sizeof(Key));
    memset(IV , 0x22, BlockSize);
    memset(EncData, 0x00, BlockSize);
    memset(DecData, 0x00, BlockSize);

    MCP_CAM_CTR_Encryption(Key, IV, Data, EncData, BlockSize);
    ret += PrintCheckReuslt(1, RefD, EncData, BlockSize, "[3.3.1]", __FUNCTION__);

    MCP_CAM_CTR_Decryption(Key, IV, EncData, DecData, BlockSize);
    ret += PrintCheckReuslt(0, Data, DecData, BlockSize, "[3.3.2]", __FUNCTION__);

    kfree(DecData);
fail_DecData:
    kfree(EncData);
fail_EncData:
    kfree(RefD);
fail_RefD:
    kfree(Data);

    return ret;
}

/*------------------------------------------------------------------
* Func : MCP_CAM_192_ECB_CW_Test
*
* Desc : CAM-192 ECB CW Encrypt Decrypt Test
*
* Parm : Void
*
* Retn : Fail Count
*------------------------------------------------------------------*/
int MCP_CAM_192_ECB_Test(void)
{
    unsigned int   BlockSize = 16;
    unsigned char  _Data[16] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x00};
    unsigned char  _RefD[16] = {0x06, 0x97, 0x54, 0xB5, 0x17, 0xA8, 0xF4, 0xF1, 0x8F, 0x00, 0x76, 0x62, 0x7E, 0xFC, 0x6B, 0x59}; //camellia
    unsigned char  Key[24];
    unsigned char  *EncData, *DecData;
    unsigned char  *Data, *RefD;
    int ret = 0;

    Data = kmalloc(sizeof(_Data), GFP_KERNEL);
    if (!Data) {
        return -ENOMEM;
    }

    RefD = kmalloc(sizeof(_RefD), GFP_KERNEL);
    if (!RefD) {
        ret = -ENOMEM;
        goto fail_RefD;
    }

    EncData = kmalloc(BlockSize, GFP_KERNEL);
    if (!EncData) {
        ret = -ENOMEM;
        goto fail_EncData;
    }

    DecData  = kmalloc(BlockSize, GFP_KERNEL);
    if (!DecData) {
        ret = -ENOMEM;
        goto fail_DecData;
    }

    memcpy(Data, _Data, sizeof(_Data));
    memcpy(RefD, _RefD, sizeof(_RefD));

    mcp_info("========================== CAM-192 DDR ECB ==========================\n");
    mcp_dump_data_with_text(Data, BlockSize, "Input   : ");
    memset(Key, 0xff, sizeof(Key));
    memset(EncData, 0x00, BlockSize);
    memset(DecData, 0x00, BlockSize);

    MCP_CAM_192_ECB_Encryption(Key, Data, EncData, BlockSize);
    ret += PrintCheckReuslt(1, RefD, EncData, BlockSize, "[4.1.1]", __FUNCTION__);

    MCP_CAM_192_ECB_Decryption(Key, EncData, DecData, BlockSize);
    ret += PrintCheckReuslt(0, Data, DecData, BlockSize, "[4.1.2]", __FUNCTION__);

    kfree(DecData);
fail_DecData:
    kfree(EncData);
fail_EncData:
    kfree(RefD);
fail_RefD:
    kfree(Data);

    return ret;
}

/*------------------------------------------------------------------
* Func : MCP_CAM_192_CBC_CW_Test
*
* Desc : CAM-192 CBC CW Encrypt Decrypt Test
*
* Parm : Void
*
* Retn : Fail Count
*------------------------------------------------------------------*/
int MCP_CAM_192_CBC_Test(void)
{
    unsigned int   BlockSize = 16;
    unsigned char  _Data[16] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x00};
    unsigned char  _RefD[16] = {0xBE, 0x14, 0x78, 0x8D, 0x88, 0x38, 0x9D, 0x7C, 0xE7, 0x71, 0xC9, 0x4B, 0xCF, 0x9F, 0x1C, 0x0A}; //camellia
    unsigned char  Key[24];
    unsigned char  IV[16];
    unsigned char  *EncData, *DecData;
    unsigned char  *Data, *RefD;
    int ret = 0;

    Data = kmalloc(sizeof(_Data), GFP_KERNEL);
    if (!Data) {
        return -ENOMEM;
    }

    RefD = kmalloc(sizeof(_RefD), GFP_KERNEL);
    if (!RefD) {
        ret = -ENOMEM;
        goto fail_RefD;
    }

    EncData = kmalloc(BlockSize, GFP_KERNEL);
    if (!EncData) {
        ret = -ENOMEM;
        goto fail_EncData;
    }

    DecData  = kmalloc(BlockSize, GFP_KERNEL);
    if (!DecData) {
        ret = -ENOMEM;
        goto fail_DecData;
    }

    memcpy(Data, _Data, sizeof(_Data));
    memcpy(RefD, _RefD, sizeof(_RefD));

    mcp_info("========================== CAM-192 DDR CBC ======================\n");
    mcp_dump_data_with_text(Data, BlockSize, "Input   : ");
    memset(Key, 0xff, sizeof(Key));
    memset(IV , 0x22, BlockSize);
    memset(EncData, 0x00, BlockSize);
    memset(DecData, 0x00, BlockSize);

    MCP_CAM_192_CBC_Encryption(Key, IV, Data, EncData, BlockSize);
    ret += PrintCheckReuslt(1, RefD, EncData, BlockSize, "[4.2.1]", __FUNCTION__);

    MCP_CAM_192_CBC_Decryption(Key, IV, EncData, DecData, BlockSize);
    ret += PrintCheckReuslt(0, Data, DecData, BlockSize, "[4.2.2]", __FUNCTION__);

    kfree(DecData);
fail_DecData:
    kfree(EncData);
fail_EncData:
    kfree(RefD);
fail_RefD:
    kfree(Data);

    return ret;
}
/*------------------------------------------------------------------
* Func : MCP_CAM_192_CTR_CW_Test
*
* Desc : CAM-192 CTR CW Encrypt Decrypt Test
*
* Parm : Void
*
* Retn : Fail Count
*------------------------------------------------------------------*/
int MCP_CAM_192_CTR_Test(void)
{
    unsigned int   BlockSize = 16;
    unsigned char  _Data[16] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x00};
    unsigned char  _RefD[16] = {0x8C, 0x87, 0x80, 0xC8, 0x41, 0xC2, 0x45, 0x69, 0x36, 0xCE, 0x83, 0x88, 0xEA, 0xF5, 0x70, 0xAE}; //camellia
    unsigned char  Key[24];
    unsigned char  IV[16];
    unsigned char  *EncData, *DecData;
    unsigned char  *Data, *RefD;
    int ret = 0;

    Data = kmalloc(sizeof(_Data), GFP_KERNEL);
    if (!Data) {
        return -ENOMEM;
    }

    RefD = kmalloc(sizeof(_RefD), GFP_KERNEL);
    if (!RefD) {
        ret = -ENOMEM;
        goto fail_RefD;
    }

    EncData = kmalloc(BlockSize, GFP_KERNEL);
    if (!EncData) {
        ret = -ENOMEM;
        goto fail_EncData;
    }

    DecData  = kmalloc(BlockSize, GFP_KERNEL);
    if (!DecData) {
        ret = -ENOMEM;
        goto fail_DecData;
    }

    memcpy(Data, _Data, sizeof(_Data));
    memcpy(RefD, _RefD, sizeof(_RefD));

    mcp_info("========================== CAM-192 DDR CTR ======================\n");
    mcp_dump_data_with_text(Data, BlockSize, "Input   : ");
    memset(Key, 0xff, sizeof(Key));
    memset(IV , 0x22, BlockSize);
    memset(EncData, 0x00, BlockSize);
    memset(DecData, 0x00, BlockSize);

    MCP_CAM_192_CTR_Encryption(Key, IV, Data, EncData, BlockSize);
    ret += PrintCheckReuslt(1, RefD, EncData, BlockSize, "[4.3.1]", __FUNCTION__);

    MCP_CAM_192_CTR_Decryption(Key, IV, EncData, DecData, BlockSize);
    ret += PrintCheckReuslt(0, Data, DecData, BlockSize, "[4.3.2]", __FUNCTION__);

    kfree(DecData);
fail_DecData:
    kfree(EncData);
fail_EncData:
    kfree(RefD);
fail_RefD:
    kfree(Data);

    return ret;
}
/*------------------------------------------------------------------
* Func : MCP_CAM_256_ECB_CW_Test
*
* Desc : CAM-256 ECB CW Encrypt Decrypt Test
*
* Parm : Void
*
* Retn : Fail Count
*------------------------------------------------------------------*/
int MCP_CAM_256_ECB_Test(void)
{
    unsigned int   BlockSize = 16;
    unsigned char  _Data[16] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x00};
    unsigned char  _RefD[16] = {0x63, 0xF2, 0x81, 0x94, 0x51, 0x46, 0x71, 0x9F, 0x8F, 0x70, 0xDA, 0x4F, 0xC1, 0x80, 0x6C, 0x26}; //camellia
    unsigned char  Key[32];
    unsigned char  *EncData, *DecData;
    unsigned char  *Data, *RefD;
    int ret = 0;

    Data = kmalloc(sizeof(_Data), GFP_KERNEL);
    if (!Data) {
        return -ENOMEM;
    }

    RefD = kmalloc(sizeof(_RefD), GFP_KERNEL);
    if (!RefD) {
        ret = -ENOMEM;
        goto fail_RefD;
    }

    EncData = kmalloc(BlockSize, GFP_KERNEL);
    if (!EncData) {
        ret = -ENOMEM;
        goto fail_EncData;
    }

    DecData  = kmalloc(BlockSize, GFP_KERNEL);
    if (!DecData) {
        ret = -ENOMEM;
        goto fail_DecData;
    }

    memcpy(Data, _Data, sizeof(_Data));
    memcpy(RefD, _RefD, sizeof(_RefD));

    mcp_info("========================== CAM-256 DDR ECB ======================\n");
    mcp_dump_data_with_text(Data, BlockSize, "Input   : ");
    memset(Key, 0xff, sizeof(Key));
    memset(EncData, 0x00, BlockSize);
    memset(DecData, 0x00, BlockSize);

    MCP_CAM_256_ECB_Encryption(Key, Data, EncData, BlockSize);
    ret += PrintCheckReuslt(1, RefD, EncData, BlockSize, "[5.1.1]", __FUNCTION__);

    MCP_CAM_256_ECB_Decryption(Key, EncData, DecData, BlockSize);
    ret += PrintCheckReuslt(0, Data, DecData, BlockSize, "[5.1.2]", __FUNCTION__);

    kfree(DecData);
fail_DecData:
    kfree(EncData);
fail_EncData:
    kfree(RefD);
fail_RefD:
    kfree(Data);

    return ret;
}
/*------------------------------------------------------------------
* Func : MCP_CAM_256_CBC_CW_Test
*
* Desc : CAM-256 CBC CW Encrypt Decrypt Test
*
* Parm : Void
*
* Retn : Fail Count
*------------------------------------------------------------------*/
int MCP_CAM_256_CBC_Test(void)
{
    unsigned int   BlockSize = 16;
    unsigned char  _Data[16] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x00};
    unsigned char  _RefD[16] = {0x9C, 0x0B, 0xA0, 0x2E, 0x32, 0xB1, 0xCC, 0xB7, 0x39, 0xA3, 0x67, 0xB6, 0x8C, 0x18, 0x2F, 0xFB}; //camellia
    unsigned char  Key[32];
    unsigned char  IV[16];
    unsigned char  *EncData, *DecData;
    unsigned char  *Data, *RefD;
    int ret = 0;

    Data = kmalloc(sizeof(_Data), GFP_KERNEL);
    if (!Data) {
        return -ENOMEM;
    }

    RefD = kmalloc(sizeof(_RefD), GFP_KERNEL);
    if (!RefD) {
        ret = -ENOMEM;
        goto fail_RefD;
    }

    EncData = kmalloc(BlockSize, GFP_KERNEL);
    if (!EncData) {
        ret = -ENOMEM;
        goto fail_EncData;
    }

    DecData  = kmalloc(BlockSize, GFP_KERNEL);
    if (!DecData) {
        ret = -ENOMEM;
        goto fail_DecData;
    }

    memcpy(Data, _Data, sizeof(_Data));
    memcpy(RefD, _RefD, sizeof(_RefD));

    mcp_info("========================== CAM-256 DDR CBC ======================\n");
    mcp_dump_data_with_text(Data, BlockSize, "Input   : ");
    memset(Key, 0xff, sizeof(Key));
    memset(IV , 0x22, BlockSize);
    memset(EncData, 0x00, BlockSize);
    memset(DecData, 0x00, BlockSize);

    MCP_CAM_256_CBC_Encryption(Key, IV, Data, EncData, BlockSize);
    ret += PrintCheckReuslt(1, RefD, EncData, BlockSize, "[5.2.1]", __FUNCTION__);

    MCP_CAM_256_CBC_Decryption(Key, IV, EncData, DecData, BlockSize);
    ret += PrintCheckReuslt(0, Data, DecData, BlockSize, "[5.2.2]", __FUNCTION__);

    kfree(DecData);
fail_DecData:
    kfree(EncData);
fail_EncData:
    kfree(RefD);
fail_RefD:
    kfree(Data);

    return ret;
}
/*------------------------------------------------------------------
* Func : MCP_CAM_256_CTR_CW_Test
*
* Desc : CAM-256 CTR CW Encrypt Decrypt Test
*
* Parm : Void
*
* Retn : Fail Count
*------------------------------------------------------------------*/
int MCP_CAM_256_CTR_Test(void)
{
    unsigned int   BlockSize = 16;
    unsigned char  _Data[16] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x00};
    unsigned char  _RefD[16] = {0xE9, 0x56, 0x6A, 0xB9, 0x23, 0xC4, 0xD9, 0x98, 0x6B, 0x90, 0x17, 0x80, 0x19, 0xB2, 0x38, 0x0B}; //camellia
    unsigned char  Key[32];
    unsigned char  IV[16];
    unsigned char  *EncData, *DecData;
    unsigned char  *Data, *RefD;
    int ret = 0;

    Data = kmalloc(sizeof(_Data), GFP_KERNEL);
    if (!Data) {
        return -ENOMEM;
    }

    RefD = kmalloc(sizeof(_RefD), GFP_KERNEL);
    if (!RefD) {
        ret = -ENOMEM;
        goto fail_RefD;
    }

    EncData = kmalloc(BlockSize, GFP_KERNEL);
    if (!EncData) {
        ret = -ENOMEM;
        goto fail_EncData;
    }

    DecData  = kmalloc(BlockSize, GFP_KERNEL);
    if (!DecData) {
        ret = -ENOMEM;
        goto fail_DecData;
    }

    memcpy(Data, _Data, sizeof(_Data));
    memcpy(RefD, _RefD, sizeof(_RefD));

    mcp_info("========================== CAM-256 DDR CTR ======================\n");
    mcp_dump_data_with_text(Data, BlockSize, "Input   : ");
    memset(Key, 0xff, sizeof(Key));
    memset(IV , 0x22, BlockSize);
    memset(EncData, 0x00, BlockSize);
    memset(DecData, 0x00, BlockSize);

    MCP_CAM_256_CTR_Encryption(Key, IV, Data, EncData, BlockSize);
    ret += PrintCheckReuslt(1, RefD, EncData, BlockSize, "[5.3.1]", __FUNCTION__);

    MCP_CAM_256_CTR_Decryption(Key, IV, EncData, DecData, BlockSize);
    ret += PrintCheckReuslt(0, Data, DecData, BlockSize, "[5.3.2]", __FUNCTION__);

    kfree(DecData);
fail_DecData:
    kfree(EncData);
fail_EncData:
    kfree(RefD);
fail_RefD:
    kfree(Data);

    return ret;
}


/*------------------------------------------------------------------
 * Func : MCP_AES_H_DataHashTest
 *
 * Desc : Compute hash value of Data via 128 bits AES_H_Hasing
 *
 * Parm : pData      : Start address of data
 *        DataLen    : Length of data
 *        Hash[16]   : 128 bits hash value of the file
 *        BlockSize  : Block Size that used to compute Hashing Value
 *
 * Retn : S_OK  /  S_FALSE
 *------------------------------------------------------------------*/
int MCP_AES_H_DataHashTest(void)
{
    unsigned char *Data;
    unsigned char *Hash;
    unsigned int data_size = 256, hash_size = 16;
    unsigned char RefD[16] = {0x61, 0x8f, 0x75, 0xc2, 0x0c, 0xb5, 0xd1, 0xe4, 0x63, 0xdc, 0xcb, 0xa6, 0x41, 0x33, 0xb9, 0x6c};
    int ret = 0;

    mcp_info("========================= AES DataHash ==========================\n");
    Data = kmalloc(data_size, GFP_KERNEL);
    if (!Data)
        return -ENOMEM;

    Hash = kmalloc(hash_size, GFP_KERNEL);
    if (!Hash) {
        ret = -ENOMEM;
        goto fail_hash;
    }

    memset(Data, 0x33, data_size);

    MCP_AES_H_DataHash(Data,  data_size, Hash, AES_H_FILE_HASHING_BUFFER_SIZE, 1);

    ret = PrintCheckReuslt(1, RefD, Hash, hash_size, "[3.3.1]", __FUNCTION__);

    kfree(Hash);
fail_hash:
    kfree(Data);

    return ret;
}

/*------------------------------------------------------------------
 * Func : MCP_SHA1_HASH_Test
 *
 * Desc : Compute hash value of Data via 128 bits AES_H_Hasing
 *
 * Parm : Void
 *
 * Retn : Fail Count
 *------------------------------------------------------------------*/
int MCP_SHA1_HASH_Test(void)
{
    unsigned int error = 0;
    unsigned char *EncData; // hash value calculated by CP engine
    mcp_desc desc;
    unsigned int pad_sz;
    unsigned char RefD[20] = {0x44, 0xee, 0x7a, 0x16, 0x54, 0xda, 0xfe, 0x62, 0x9f, 0xe0, 0xa3, 0xd8, 0x5a, 0x00, 0x2b, 0x18, 0xcd, 0xd6, 0xc2, 0x45};
    unsigned char *Data;
    int ret = 0;

    mcp_info("========================== SHA1 HASH ============================\n");

    Data = kzalloc(256 + 64, GFP_KERNEL);
    if (!Data)
        return -ENOMEM;

    EncData = kmalloc(32, GFP_KERNEL);
    if (!EncData) {
        ret = -ENOMEM;
        goto fail_encdata;
    }

    memset(Data, 0x33, 256);
    MCP_SHA1_HASH_INIT(&desc);
    mcp_dump_data_with_text(Data, 20, "Input   : ");

    pad_sz = MCP_SHA1_PADDING((unsigned char *)&Data[256], 64, 256);
    msleep(50);

    error = MCP_SHA1_Hashing(&desc, (unsigned char *)Data, 256 + pad_sz, EncData);
    if( error ) {
        mcp_info("[ERR] %s: caculate hash1 fail(%d)\n", __FUNCTION__, error );
        ret = 1;
        goto ERR;
    }

    ret = PrintCheckReuslt(1, RefD, EncData, 20, "[7.1]", __FUNCTION__);
ERR:
    kfree(EncData);
fail_encdata:
    kfree(Data);

    return ret;
}
#ifdef CONFIG_RTK_KDRV_MCP_SHA256
/*------------------------------------------------------------------
* Func : MCP_SHA256_HASH_Test
*
* Desc : SHA256 HASH Test
*
* Parm : Void
*
* Retn : Fail Count
*------------------------------------------------------------------*/
int MCP_SHA256_HASH_Test(void)
{
    unsigned int error = 0;
    unsigned char *EncData; // hash value calculated by CP engine
    unsigned int pad_sz = 0;
    unsigned char RefD[32] = {0xd6, 0xd8, 0x16, 0xab, 0xa8, 0x41, 0xe9, 0x26, 0x29, 0xe7, 0x2d, 0x08, 0x49, 0x89, 0xcf, 0x87, 0xe6, 0x18, 0xbe, 0xaf, 0x56, 0x13, 0xd3, 0x92, 0xb4, 0x2f, 0x16, 0x85, 0xc1, 0x54, 0xd0, 0xcb};

    unsigned char *Data;
    mcp_desc desc;
    int ret = 0;

    mcp_info("========================== SHA-256 HASH =========================\n");

    Data = kzalloc(256 + 64, GFP_KERNEL);
    if (!Data)
        return -ENOMEM;

    EncData = kmalloc(32, GFP_KERNEL);
    if (!EncData) {
        ret = -ENOMEM;
        goto fail_encdata;
    }

    MCP_SHA256_HASH_INIT(&desc);

    memset(Data, 0x33, 256);
    mcp_dump_data_with_text(Data, 20, "Input   : ");

    pad_sz += MCP_SHA256_PADDING((unsigned char *)&Data[256], 64, 256);
    msleep(50);
    //EncData = (unsigned char *)SECURE_IMAGE2HASH_BUF; // temp use this address (use malloc is better?)

    error = MCP_SHA256_Hashing(&desc, (unsigned char *)Data, 256 + pad_sz, EncData);
    if( error ) {
        mcp_info("[ERR] %s: caculate hash1 fail(%d)\n", __FUNCTION__, error );
        ret = 1;
        goto ERR;
    }
    ret = PrintCheckReuslt(1, RefD, EncData, 32, "[7.2]", __FUNCTION__);
ERR:
    kfree(EncData);
fail_encdata:
    kfree(Data);

    return ret;
}

int MCP_SHA256_test(void)
{
    unsigned int error = 0;
    unsigned char EncData[32]; // hash value calculated by CP engine
    unsigned char RefD[32] = {0xd6, 0xd8, 0x16, 0xab, 0xa8, 0x41, 0xe9, 0x26, 0x29, 0xe7, 0x2d, 0x08, 0x49, 0x89, 0xcf, 0x87, 0xe6, 0x18, 0xbe, 0xaf, 0x56, 0x13, 0xd3, 0x92, 0xb4, 0x2f, 0x16, 0x85, 0xc1, 0x54, 0xd0, 0xcb};
    unsigned char Data[256];

    int ret = 0;
    
    memset(Data, 0x33, 256);
    mcp_info("========================== MCP_SHA256_test =========================\n");

    error = MCP_SHA256(Data, 256, EncData);
    if( error ) {
        mcp_info("[ERR] %s: caculate hash256 fail(%d)\n", __FUNCTION__, error );
        return 1;
    }
    ret = PrintCheckReuslt(1, RefD, EncData, 32, "[7.3]", __FUNCTION__);
    return ret;
}

#endif

static unsigned char* _mcp_test_buff_init(dma_addr_t* p_dma_addr)
{
    static dma_addr_t stress_test_dma_addr;
    static unsigned char* p_test_buff = NULL;

    if (p_test_buff == NULL)
        p_test_buff = mcp_dma_alloc_coherent((1024 * 1024) + ( sizeof(mcp_desc) * 512)  , &stress_test_dma_addr, GFP_KERNEL);

    *p_dma_addr = stress_test_dma_addr;
    return p_test_buff;
}

int do_mcp_burst_test(unsigned int round)
{
    dma_addr_t dma_addr;
    unsigned char* p_buff = _mcp_test_buff_init(&dma_addr);
    unsigned char Key[16];
    int i;
    mcp_desc* desc;

    if (p_buff == NULL)
        return -1;

    mcp_info("allocated buffer, p_buff=%08lx, dma_addr=%08lx\n", (unsigned long)p_buff, (unsigned long)dma_addr);

    memset(Key, 0, sizeof(Key));
    memset(p_buff, 0, 188 * 512);

    desc = (mcp_desc*) (p_buff + (1024 * 1024));

    for (i = 0; i < 256; i++) {
        MCP_AES_DESC_INIT(&desc[i], 1, MCP_BCM_CBC, Key, Key);
        desc[i].data_in  = dma_addr;
        desc[i].data_out = dma_addr;
        desc[i].length   = 1024 * 1024;
    }

    for(i = 0; i < round; i++) {
        mcp_do_command(desc, 1);
        udelay(100);
        mcp_do_command(desc, 1);
        udelay(100);
        mcp_do_command(desc, 1);
        udelay(100);
        mcp_do_command(desc, 1);
        msleep(10);
        mcp_info("*");
    }
    return 0;
}


int do_mcp_error_test(unsigned int round)
{
    dma_addr_t dma_addr;
    unsigned char* p_buff = _mcp_test_buff_init(&dma_addr);
    unsigned char Key[16];
    int i;
    mcp_desc* desc;

    if (p_buff == NULL)
        return -1;

    memset(Key, 0, sizeof(Key));
    memset(p_buff, 0, 188 * 512);
    desc = (mcp_desc*) (p_buff + (188 * 512));

    for (i = 0; i < 4; i++) {
        MCP_AES_DESC_INIT(&desc[i], 1, MCP_BCM_CBC, Key, Key);
        desc[i].data_in  = dma_addr + (i * 188) + 4;
        desc[i].data_out = dma_addr + (i * 188) + 4;
        desc[i].length   = 184;
    }

    desc[3].length = 0;

    for(i = 0; i < round; i++) {
        mcp_do_command(desc, 4);
        msleep(10);
    }
    return 0;
}


int do_mcp_throughput_test(unsigned int size)
{
    static dma_addr_t dma_addr = 0;
    unsigned char* p_buff = NULL;
    unsigned char* Key = NULL;
    unsigned int data_size = 1024 * size;
    unsigned char* Hash = NULL;
    unsigned long stime = 0;
    uint32_t pts_s = 0, pts_e = 0;
    int bcm_type, en_de;
    int pad_sz;
    dma_addr_t key_buff_phy;
    mcp_desc* desc;
    unsigned int total_size = data_size + sizeof(mcp_desc) + 64 + 20;
    rtd_outl(TIMER_REG_CTRL_ADDR, 0x1);

    Key = mcp_dma_alloc_coherent(32, &key_buff_phy, GFP_KERNEL);
    if(!Key)
        goto EXIT;
    memset(Key, 0x33, 32);

    if (size > 0)
        //p_buff = mcp_dma_alloc_coherent(data_size + sizeof(mcp_desc) + 64 + 20, &dma_addr, GFP_KERNEL);
        p_buff = dvr_malloc_specific(total_size, GFP_DCU1_LIMIT);
    if(!p_buff)
		goto EXIT;
	
    memset(p_buff, 0x33, data_size);
    desc = (mcp_desc*) (p_buff + data_size + 64); //shift to *mcp_desc
    Hash = p_buff + total_size - 20;
    dma_addr = _mcp_map_single(p_buff, total_size, DMA_BIDIRECTIONAL);
    msleep(10);


    for(bcm_type = MCP_BCM_ECB; bcm_type <= MCP_BCM_CBC; bcm_type++) {
        for(en_de = 1; en_de >= 0 ; en_de--) {

            MCP_DES_DESC_INIT(desc, en_de, bcm_type, Key, Key);

            do_mcp_throughput_calculate(desc, &dma_addr, &dma_addr, data_size, en_de, bcm_type, "DES   ");
        }

        for(en_de = 1; en_de >= 0 ; en_de--) {

            MCP_TDES_DESC_INIT(desc, en_de, bcm_type, Key, Key);

            do_mcp_throughput_calculate(desc, &dma_addr, &dma_addr, data_size, en_de, bcm_type, "TDES  ");
        }
    }

    for(bcm_type = MCP_BCM_ECB; bcm_type <= MCP_BCM_CTR; bcm_type++) {
        for(en_de = 1; en_de >= 0 ; en_de--) {

            MCP_AES_DESC_INIT(desc, en_de, bcm_type, Key, Key);

            do_mcp_throughput_calculate(desc, &dma_addr, &dma_addr, data_size, en_de, bcm_type, "AES128");
        }

        for(en_de = 1; en_de >= 0 ; en_de--) {

            MCP_AES_192_DESC_INIT(desc, en_de, bcm_type, Key, Key, key_buff_phy);

            do_mcp_throughput_calculate(desc, &dma_addr, &dma_addr, data_size, en_de, bcm_type, "AES192");
        }

        for(en_de = 1; en_de >= 0 ; en_de--) {

            MCP_AES_256_DESC_INIT(desc, en_de, bcm_type, Key, Key, key_buff_phy);

            do_mcp_throughput_calculate(desc, &dma_addr, &dma_addr, data_size, en_de, bcm_type, "AES256");
        }

        for(en_de = 1; en_de >= 0 ; en_de--) {

            MCP_CAM_DESC_INIT(desc, en_de, bcm_type, Key, Key);

            do_mcp_throughput_calculate(desc, &dma_addr, &dma_addr, data_size, en_de, bcm_type, "CAM128");
        }

        for(en_de = 1; en_de >= 0 ; en_de--) {

            MCP_CAM_192_DESC_INIT(desc, en_de, bcm_type, Key, Key, key_buff_phy);

            do_mcp_throughput_calculate(desc, &dma_addr, &dma_addr, data_size, en_de, bcm_type, "CAM192");
        }

        for(en_de = 1; en_de >= 0 ; en_de--) {

            MCP_CAM_256_DESC_INIT(desc, en_de, bcm_type, Key, Key, key_buff_phy);

            do_mcp_throughput_calculate(desc, &dma_addr, &dma_addr, data_size, en_de, bcm_type, "CAM256");
        }

    }

    stime = jiffies;
    pts_s = rtd_inl(TIMER_REG_VAL_ADDR);
    MCP_AES_H_DataHash(p_buff,  data_size, Hash, AES_H_FILE_HASHING_BUFFER_SIZE, 1);
    pts_e = rtd_inl(TIMER_REG_VAL_ADDR);
    mcp_info("[MCP] AES_H  Throughput %d Kbytes elapsed %d us, \033[0;32;34m %d ms\n\033[m", 
            data_size / 1024, (pts_e - pts_s) * (1000000 / 90090), jiffies_to_msecs(jiffies - stime));

    MCP_SHA1_HASH_INIT(desc);
    pad_sz = MCP_SHA1_PADDING((unsigned char *)&p_buff[data_size], 64, data_size);
    msleep(50);
    stime = jiffies;
    pts_s = rtd_inl(TIMER_REG_VAL_ADDR);
    MCP_SHA1_Hashing(desc, (unsigned char *)p_buff, data_size + pad_sz, Hash);
    pts_e = rtd_inl(TIMER_REG_VAL_ADDR);
    mcp_info("[MCP] SHA1   Throughput %d Kbytes elapsed %d us, \033[0;32;34m %d ms\n\033[m", 
            data_size / 1024, (pts_e - pts_s) * (1000000 / 90090), jiffies_to_msecs(jiffies - stime));
EXIT:
    if(Key)
        mcp_dma_free_coherent(32, Key, key_buff_phy);
    if(dma_addr)
        _mcp_unmap_single(dma_addr, total_size, DMA_BIDIRECTIONAL);
    if(p_buff)
        dvr_free(p_buff);
    return 0;
}


void do_mcp_function_test(void)
{
    int ErrorCount = 0;

    ErrorCount += MCP_DES_ECB_Test();
    ErrorCount += MCP_DES_CBC_Test();
    ErrorCount += MCP_TDES_ECB_Test();
    ErrorCount += MCP_TDES_CBC_Test();
    ErrorCount += MCP_AES_128_ECB_Test();
    ErrorCount += MCP_AES_128_CBC_Test();
    ErrorCount += MCP_AES_128_CTR_Test();
    ErrorCount += MCP_AES_192_ECB_Test();
    ErrorCount += MCP_AES_192_CBC_Test();
    ErrorCount += MCP_AES_192_CTR_Test();
    ErrorCount += MCP_AES_256_ECB_Test();
    ErrorCount += MCP_AES_256_CBC_Test();
    ErrorCount += MCP_AES_256_CTR_Test();

    ErrorCount += MCP_CAM_128_ECB_Test();
    ErrorCount += MCP_CAM_128_CBC_Test();
    ErrorCount += MCP_CAM_128_CTR_Test();
    ErrorCount += MCP_CAM_192_ECB_Test();
    ErrorCount += MCP_CAM_192_CBC_Test();
    ErrorCount += MCP_CAM_192_CTR_Test();
    ErrorCount += MCP_CAM_256_ECB_Test();
    ErrorCount += MCP_CAM_256_CBC_Test();
    ErrorCount += MCP_CAM_256_CTR_Test();


    ErrorCount += MCP_AES_H_DataHashTest();
    ErrorCount += MCP_SHA1_HASH_Test();
#ifdef CONFIG_MCP_SCPU_OTP_KEY_REMOVED
    ErrorCount += MCP_AES_128_ECB_WITH_SCPU_OTP_KEY_REMOVED_Test();
#endif

#ifdef CONFIG_RTK_KDRV_MCP_SHA256
    ErrorCount += MCP_SHA256_HASH_Test();
#endif
#ifdef RTK_MCP_PATTERN_TEST
    ErrorCount += MCP_PATTERN_Test(MCP_BCM_CBC, 160);
    ErrorCount += MCP_PATTERN_Test(MCP_BCM_CTR, 160);
    ErrorCount += MCP_PATTERN_DADR_Test(MCP_BCM_CBC, 160, 160);
    ErrorCount += MCP_PATTERN_DADR_Test(MCP_BCM_CTR, 160, 160);
    ErrorCount += MCP_PATTERN_DADR_Test(MCP_BCM_CBC, 161, 159);
    ErrorCount += MCP_PATTERN_DADR_Test(MCP_BCM_CBC, 159, 161);
#endif
    if(ErrorCount == 0) {
        mcp_info("\nTest All Success\nTest All Success\n");
    } else {
        mcp_info("\nTest Fail Count %d\nTest Fail Count %d\n", ErrorCount, ErrorCount);
    }
}


int do_mcp_help(void)
{
    mcp_warning("----------------- mcp test command set --------------\n");
    mcp_warning(" do_help : print help message \n");
    mcp_warning(" do_mcp_burst_test : do burst test (to issue command burst) \n");
    mcp_warning(" do_mcp_error_test : do error test (to test mcp error handling and recovery procedure) \n");
    mcp_warning(" do_mcp_function_test : do function test (to test mcp function test) \n");
    mcp_warning(" do_mcp_throughput_test : do throughput test (to test mcp throughput) \n");
    mcp_warning(" do_dump_desc : do dump_desc  (to dump the content of desc) \n");
    mcp_warning(" do_dump_fifo : do dump_fifo  (to dump the content of fifo) \n");
    return 0;
}



