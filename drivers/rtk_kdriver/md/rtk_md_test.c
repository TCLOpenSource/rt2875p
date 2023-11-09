/******************************************************************************
*
*   Copyright(c) 2016 Realtek Semiconductor Corp. All rights reserved.
*
*   @author realtek.com
*
******************************************************************************/
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/dma-mapping.h>
#include "rtk_md_debug.h"
#include <rtk_kdriver/md/rtk_md.h>
#include "rtk_md_hw.h"
#include <rtk_kdriver/io.h>


#define TIMER_REG_VAL_ADDR  0xb801b6b8
#define TIMER_REG_CTRL_ADDR  0xb801b6b0


extern struct md_dev *md_drv_dev;

/*******************************The MD driver test cases for driver porting*******************************************/

static void _rtk_md_memcpy_test(uint8_t *srcBuf, uint8_t *dstBuf, struct md_dev     *pdev, int length)
{
    uint32_t src_phys_addr = dma_map_single(pdev->md_device, (void *)srcBuf, length, DMA_BIDIRECTIONAL);
    uint32_t dst_phys_addr = dma_map_single(pdev->md_device, (void *)dstBuf, length, DMA_BIDIRECTIONAL);
    PDEWAR("[MD] virtual address: %lx, %lx\n", (unsigned long)srcBuf, (unsigned long)dstBuf);
    PDEWAR("[MD] physical address: %08x, %08x\n", (uint32_t)src_phys_addr, (uint32_t)dst_phys_addr);

    smd_memcpy(dst_phys_addr, src_phys_addr, length);

    smd_checkComplete();

    PDEWAR("[MD]u64InstCnt = %lld, u64IssuedInstCnt = %lld\n", pdev->u64InstCnt, pdev->u64IssuedInstCnt);
    dma_unmap_single(pdev->md_device, src_phys_addr, length, DMA_BIDIRECTIONAL);
    dma_unmap_single(pdev->md_device, dst_phys_addr, length, DMA_BIDIRECTIONAL);

    if(memcmp((void*)srcBuf, (void*)dstBuf, length) == 0) {
        PDEWAR("[MD] memcpy ok\n");
    } else {
        PDEERR("[MD] memcpy fail\n");
        PDEIFO("%d,%d,%d,%d", srcBuf[0], srcBuf[1], srcBuf[2], srcBuf[3]);
        PDEIFO("%d,%d,%d,%d\n", dstBuf[0], dstBuf[1], dstBuf[2], dstBuf[3]);
    }
}

void rtk_md_memcpy_test(int length)
{
    uint8_t *srcBuf = kmalloc(length, GFP_KERNEL);
    uint8_t *dstBuf = kmalloc(length, GFP_KERNEL);

    if((srcBuf != NULL) && (dstBuf != NULL)) {
        memset(srcBuf, 0xa0, length);
        memset(dstBuf, 0xb0, length);
        _rtk_md_memcpy_test(srcBuf, dstBuf, md_drv_dev, length);
    }
    kfree(srcBuf);
    kfree(dstBuf);
}

void rtk_md_fill_test(void)
{
    int i = 0;
    int length = 16;
    uint32_t dst_phys_addr;
    uint8_t *dstBuf = kmalloc(length, GFP_KERNEL);
    memset(dstBuf, 0x0, length);

    dst_phys_addr = dma_map_single(md_drv_dev->md_device, (void *)dstBuf, length, DMA_BIDIRECTIONAL);

    smd_memfill(dst_phys_addr, length, 0xaa);

    dma_unmap_single(md_drv_dev->md_device, dst_phys_addr, length, DMA_BIDIRECTIONAL);
    PDEWAR("rtk_md_fill_test:");
    for(i = 0; i < length; i++)
        PDEWAR("%02x ", dstBuf[i]);
    PDEWAR("\n");
    kfree(dstBuf);
}

void rtk_md_stress_test(int length, int loops)
{
    int i = 0;
    int count = 0;
    unsigned long dstAry[10];
    uint8_t *dstBuf;
    uint8_t *srcBuf = kmalloc(length, GFP_KERNEL);
    if(srcBuf != NULL) {
        for (i = 0; i < 10; i++) {
            dstBuf = kmalloc(length, GFP_KERNEL);
            dstAry[i] = (unsigned long)dstBuf;
        }

        while(count < loops) {
            for(i = 0; i < 10; i++) {
                memset((void *)dstAry[i], count, length);
                _rtk_md_memcpy_test(srcBuf, (uint8_t *)dstAry[i], md_drv_dev, length);
            }
            count++;
            udelay(100);
        }

        kfree(srcBuf);
        for(i = 0; i < 10; i++) {
            kfree((void*)dstAry[i]);
        }
    }
}
#ifndef CONFIG_RTK_MD_USE_SW
int rtk_md_memcpy_multi_desc(uint8_t *srcBuf, unsigned long *dstBuf,
                            struct md_dev *pdev, int length, unsigned int desc_num)
{
    int flags = 0, i = 0;
    unsigned long stime = 0;
    uint32_t pts_s = 0, pts_e = 0;
    uint32_t len_tmp = MD_SEQ_LEN_LIMIT;
    uint32_t (*dwcmdword)[4];
    int32_t wret = 0;
    uint32_t dst_phys_addr;
    uint32_t src_phys_addr = dma_map_single(md_drv_dev->md_device, (void *)srcBuf, length, DMA_BIDIRECTIONAL);

    md_drv_dev->CmdDone = 0;

    if(length <= 0)
        return wret;

    len_tmp = (length > MD_SEQ_LEN_LIMIT) ? MD_SEQ_LEN_LIMIT : length;

    dwcmdword = (uint32_t(*)[4])kmalloc(desc_num * 4 * sizeof(uint32_t), GFP_KERNEL);
    if (dwcmdword == NULL)
        return -1;
    for(i = 0; i < desc_num; i++) {
        dst_phys_addr = dma_map_single(md_drv_dev->md_device, (void *)dstBuf[i], length, DMA_BIDIRECTIONAL);
        dwcmdword[i][0] = (MD_SS | MD_1B | ((flags & BIT(0))  ? MD_CONST : 0) |  ((flags & BIT(1))  ? MD_MOVE_DIR : 0));
        dwcmdword[i][1] = dst_phys_addr;
        dwcmdword[i][2] = src_phys_addr;
        dwcmdword[i][3] = (len_tmp & MD_SEQ_LEN_LIMIT_MASK);
        //PDEWAR("daddress src=%p, dst=%p, dwd=%p\n", src_phys_addr, dst_phys_addr, dwcmdword[i][1]);
    }
    if (down_interruptible(&md_drv_dev->sem)) {
        kfree(dwcmdword);
        return -1;
    }
    stime = jiffies;
    pts_s = rtd_inl(TIMER_REG_VAL_ADDR);
    wret = rtk_write_cmd_wrapper(pdev, (uint8_t *)dwcmdword[0], desc_num * 4 * sizeof(uint32_t));
    up(&md_drv_dev->sem);
    if(wret != MD_SUCCESS) {
        PDEWAR("[MD]%s fail(%d).\n", __func__, wret);
    }

    smd_checkComplete();
    pts_e = rtd_inl(TIMER_REG_VAL_ADDR);
    PDEWAR("[MD] rtk_md_memcpy_test speed test len = %d, times = %d, %d us, %d ms\n", length, desc_num, (pts_e - pts_s) * (1000000 / 90090), jiffies_to_msecs(jiffies - stime));

    /* unmap dma */
    dma_unmap_single(md_drv_dev->md_device, src_phys_addr, length, DMA_BIDIRECTIONAL);
    for(i = 0; i < desc_num; i++) {
        dma_unmap_single(md_drv_dev->md_device, dwcmdword[i][1], length, DMA_BIDIRECTIONAL);
    }
    /* compare result */
    for(i = 0; i < desc_num; i++) {
        if(memcmp((void*)srcBuf, (void*)dstBuf[i], length) == 0) {
            //PDEWAR("[MD][i=%d] memcpy ok\n", i);
        } else {
            PDEWAR(KERN_ALERT "[MD][i=%d] memcpy fail\n", i);
            PDEWAR(KERN_ALERT "%x,%x,%x,%x\n", srcBuf[0], srcBuf[1], srcBuf[2], srcBuf[3]);
            PDEWAR(KERN_ALERT "%x,%x,%x,%x\n", ((uint8_t*)dstBuf[i])[0], ((uint8_t*)dstBuf[i])[1], 
                ((uint8_t*)dstBuf[i])[2], ((uint8_t*)dstBuf[i])[3]);
        }
    }
    kfree(dwcmdword);
    return wret;
}


int rtk_multi_vs_signal(unsigned int length, unsigned int number)
{
    int i = 0;
    unsigned long *dstAry = kmalloc(number * sizeof(unsigned long), GFP_KERNEL);
    uint8_t *srcBuf;
    if (dstAry == NULL)
        return -1;

    for(i = 0; i < number; i++) {
        dstAry[i] = 0;
    }
    /* nable timer 90k */
    rtd_outl(TIMER_REG_CTRL_ADDR, 0x1);
    srcBuf = kmalloc(length, GFP_KERNEL);

    /* malloc dst address and set dst val */
    if(srcBuf != NULL) {
        memset((void *)srcBuf, 0xff, length);
        for (i = 0; i < number; i++) {
            dstAry[i] = (unsigned long)kmalloc(length, GFP_KERNEL);
            if((void*)dstAry[i] == NULL) {
                int j = 0;
                PDEWAR(" i = %d, length = %u,kmalloc failed...\n", i, length);
                kfree(srcBuf);
                for(j = 0; j < i; j++) {
                    kfree((void*)dstAry[j]);
                }
                kfree(dstAry);
                return -1;
            }
            memset((void *)dstAry[i], 0x33, length);
            //PDEWAR("daddress dstAry[i]=0x%x\n", dstAry[i]);
        }
    } else {
        kfree(dstAry);
        PDEWAR("The srcBuf = NULL, please set a vir address to srcBuf\n");
        return -1;
    }
    PDEWAR("[MD] enable md_multi_req\n");
    /* Enable MD_MULTI_REQ Test */
    if (down_interruptible(&md_drv_dev->sem)) {
        kfree(srcBuf);
        for(i = 0; i < number; i++) {
            kfree((void*)dstAry[i]);
        }
        kfree(dstAry);
        return -1;
    }

    rtk_md_enable_mult_reg(1);    //enable Multi-req

    up(&md_drv_dev->sem);

    rtk_md_memcpy_multi_desc(srcBuf, dstAry, md_drv_dev, length, number);
    udelay(50);
    PDEWAR("[MD] disable md_multi_req\n");
    /* Disable MD_MULTI_REQ Test */
    if (down_interruptible(&md_drv_dev->sem)) {
        kfree(srcBuf);
        for(i = 0; i < number; i++) {
            kfree((void*)dstAry[i]);
        }
        kfree(dstAry);
        return -1;
    }
    rtk_md_enable_mult_reg(0);    //disable Multi-req
    up(&md_drv_dev->sem);

    for (i = 0; i < number; i++)
        memset((void *)dstAry[i], 0x33, length);

    rtk_md_memcpy_multi_desc(srcBuf, dstAry, md_drv_dev, length, number);
    udelay(50);

    kfree(srcBuf);
    for(i = 0; i < number; i++) {
        kfree((void*)dstAry[i]);
    }
    kfree(dstAry);
    return 0;
}

/*******************************The MD driver test cases for some other function*******************************************/

#define MD_VECTOR                       (0)

static void rtk_md_memcpy_seq(
    uint32_t        addrdst,
    uint32_t        addrsrc,
    int32_t         len,
    int32_t         op,
    int32_t         movebyte,
    int32_t         datamode,
    int32_t         dir)
{
    uint32_t len_tmp = MD_SEQ_LEN_LIMIT;
    uint32_t dwcmdword[4] = {0};
    int32_t wret = 0;
    if(dir != 0) {
        rtk_md_enable_mult_reg(0);
    }
    dwcmdword[0] = (datamode | movebyte | (op ? MD_CONST : 0) | (dir ? BIT(7) : 0));
    do {
        len_tmp = (len > MD_SEQ_LEN_LIMIT) ? MD_SEQ_LEN_LIMIT : len;

        dwcmdword[1] = addrdst;
        dwcmdword[2] = addrsrc;
        dwcmdword[3] = (len_tmp & MD_SEQ_LEN_LIMIT_MASK);

        wret = rtk_md_do_cmds((uint8_t *)dwcmdword, sizeof(dwcmdword));
        if(wret != MD_SUCCESS) {
            PDEWAR("[MD]%s fail(%d).\n", __func__, wret);
            break;
        }
        len -= len_tmp;
        addrdst += len_tmp;
        addrsrc += len_tmp;
    } while(len);

    smd_checkComplete();

}


static void rtk_md_memcpy_pitch(
    uint32_t        addrdst,
    uint32_t        dstpitch,
    uint32_t        addrsrc,
    uint32_t        srcpitch,
    uint32_t        hlen,
    uint32_t        wlen,
    int32_t         movebyte)
{
    uint32_t dwcmdword[4] = {0};
    int32_t wret = 0;
    dwcmdword[0] = (MD_SS_PITCH | movebyte | dstpitch << 6 | srcpitch << 19);

    dwcmdword[1] = addrdst;
    dwcmdword[2] = addrsrc;
    dwcmdword[3] = (hlen << 21 | wlen << 7);;

    wret = rtk_md_do_cmds((uint8_t *)dwcmdword, sizeof(dwcmdword));
    if(wret != MD_SUCCESS) {
        PDEWAR("[MD]%s fail(%d).\n", __func__, wret);
    }

    smd_checkComplete();

}


static uint32_t rtk_md_check(
    unsigned long addr,
    uint32_t len,
    uint32_t val,
    uint32_t bytemode)
{
    uint32_t size = len;
    uint32_t cnt = 0;

    if(bytemode == MD_2B) {
        size /= 2;

    } else if(bytemode == MD_4B) {
        size /= 4;
    }

    for(cnt = 0; cnt < size; cnt++) {
        if(bytemode == MD_1B) {
            if(*((uint8_t*)addr + cnt) != (uint8_t)(val & 0xff)) {
                PDEWAR("[MD]%s error. address = %lx, shift = %d, addr value = 0x%2x, it should be 0x%2x!\n", __func__, addr, cnt, *((uint8_t*)(unsigned long)addr + cnt), (uint8_t)(val & 0xff));
                return -1;
            }
        } else if(bytemode == MD_2B) {
            if(*((uint16_t*)addr + cnt) != (uint16_t)(val & 0xffff)) {
                PDEWAR("[MD]%s error. address = %lx, shift = %d, addr value = 0x%4x, it should be 0x%4x!\n", __func__, addr, cnt, *((uint16_t*)(unsigned long)addr + cnt), (uint16_t)(val & 0xffff));
                return -1;
            }
        } else if(bytemode == MD_4B) {
            if(*((uint32_t*)addr + cnt) != (uint32_t)(val & 0xffffffff)) {
                PDEWAR("[MD]%s error. address = %lx, shift = %d, addr value = 0x%8x, it should be 0x%8x!\n", __func__, addr, cnt, *((uint32_t*)(unsigned long)addr + cnt), (uint32_t)(val & 0xffffffff));
                return -1;
            }
        } else {
            PDEWAR("[MD]%s parameter error!\n", __func__);
            return -1;
        }
    }
    return 0;
}

static void rtk_md_gen_data(
    unsigned long addr,
    uint32_t len,
    uint32_t val,
    uint32_t bytemode)
{

    uint32_t size = len;
    uint32_t cnt = 0;
    if(bytemode == MD_2B) {
        size /= 2;
    } else if(bytemode == MD_4B) {
        size /= 4;
    }

    for(cnt = 0; cnt < size; cnt++) {
        if(bytemode == MD_1B) {
            *((uint8_t*)addr + cnt) = (uint8_t)(val & 0xff);
        } else if(bytemode == MD_2B) {
            *((uint16_t*)addr + cnt) = (uint16_t)(val & 0xffff);
        } else if(bytemode == MD_4B) {
            *((uint32_t*)addr + cnt) = (uint32_t)(val & 0xffffffff);
        } else {
            PDEWAR("[MD]%s parameter error!\n", __func__);
        }
    }
    rtk_md_check(addr, len, val, bytemode);
}

static void rtk_md_testcases_pitch(
    uint32_t addrSrc,
    uint32_t addrDst,
    unsigned long VaddrSrc,
    unsigned long VaddrDst,
    uint32_t len)
{
    uint32_t pts_s = 0;
    uint32_t pts_e = 0;
    uint32_t result = -1;


    /*CASE - pitch 1B */
    rtk_md_memcpy_seq(addrSrc, 0x00, len, MD_CONST, MD_1B, MD_SS, 0);
    rtk_md_memcpy_seq(addrDst, 0x00, len, MD_CONST, MD_1B, MD_SS, 0);
    rtk_md_gen_data(VaddrSrc, 1024, 0x77, MD_1B);

    pts_s = rtd_inl(TIMER_REG_VAL_ADDR);

    rtk_md_memcpy_pitch(addrDst, 256, addrSrc, 256, 3, 16, MD_1B);
    pts_e = rtd_inl(TIMER_REG_VAL_ADDR);
    result = rtk_md_check(VaddrDst, 16, 0x77, MD_1B);
    result += rtk_md_check(VaddrDst + 16, 256 - 16, 0x00, MD_1B);
    result += rtk_md_check(VaddrDst + 256, 16, 0x77, MD_1B);
    result += rtk_md_check(VaddrDst + 256 + 16, 256 - 16, 0x00, MD_1B);
    result += rtk_md_check(VaddrDst + 256 * 2, 16, 0x77, MD_1B);
    result += rtk_md_check(VaddrDst + 256 * 2 + 16, 256 - 16, 0x00, MD_1B);

    if(result == 0) {
        PDEWAR("[MD] COPY %d bytes with 1B pitch mode success. elapsed %d us\n", len, (pts_e - pts_s) * 1000000 / 90090);
    } else {
        PDEWAR("[MD] COPY %d bytes with 1B pitch mode fail! result = %d\n", len, result);
        return;
    }

    /*CASE - pitch 2B */
    rtk_md_memcpy_seq(addrSrc, 0x00, len, MD_CONST, MD_1B, MD_SS, 0);
    rtk_md_memcpy_seq(addrDst, 0x00, len, MD_CONST, MD_1B, MD_SS, 0);
    rtk_md_gen_data(VaddrSrc, 1024, 0x7117, MD_2B);

    pts_s = rtd_inl(TIMER_REG_VAL_ADDR);

    rtk_md_memcpy_pitch(addrDst, 256, addrSrc, 256, 3, 16, MD_2B);
    pts_e = rtd_inl(TIMER_REG_VAL_ADDR);
    result = rtk_md_check(VaddrDst, 16, 0x1771, MD_2B);
    result += rtk_md_check(VaddrDst + 16, 256 - 16, 0x00, MD_2B);
    result += rtk_md_check(VaddrDst + 256, 16, 0x1771, MD_2B);
    result += rtk_md_check(VaddrDst + 256 + 16, 256 - 16, 0x00, MD_2B);
    result += rtk_md_check(VaddrDst + 256 * 2, 16, 0x1771, MD_2B);
    result += rtk_md_check(VaddrDst + 256 * 2 + 16, 256 - 16, 0x00, MD_2B);

    if(result == 0) {
        PDEWAR("[MD] COPY %d bytes with 2B pitch mode success. elapsed %d us\n", len, (pts_e - pts_s) * 1000000 / 90090);
    } else {
        PDEWAR("[MD] COPY %d bytes with 2B pitch mode fail! result = %d\n", len, result);
        return;
    }

    /*CASE - pitch 4B */
    rtk_md_memcpy_seq(addrSrc, 0x00, len, MD_CONST, MD_1B, MD_SS, 0);
    rtk_md_memcpy_seq(addrDst, 0x00, len, MD_CONST, MD_1B, MD_SS, 0);
    rtk_md_gen_data(VaddrSrc, 1024, 0x12345678, MD_4B);

    pts_s = rtd_inl(TIMER_REG_VAL_ADDR);

    rtk_md_memcpy_pitch(addrDst, 256, addrSrc, 256, 3, 16, MD_4B);
    pts_e = rtd_inl(TIMER_REG_VAL_ADDR);
    result = rtk_md_check(VaddrDst, 16, 0x78563412, MD_4B);
    result += rtk_md_check(VaddrDst + 16, 256 - 16, 0x00, MD_4B);
    result += rtk_md_check(VaddrDst + 256, 16, 0x78563412, MD_4B);
    result += rtk_md_check(VaddrDst + 256 + 16, 256 - 16, 0x00, MD_4B);
    result += rtk_md_check(VaddrDst + 256 * 2, 16, 0x78563412, MD_4B);
    result += rtk_md_check(VaddrDst + 256 * 2 + 16, 256 - 16, 0x00, MD_4B);

    if(result == 0) {
        PDEWAR("[MD] COPY %d bytes with 4B pitch mode success. elapsed %d us\n", len, (pts_e - pts_s) * 1000000 / 90090);
    } else {
        PDEWAR("[MD] COPY %d bytes with 4B pitch mode fail! result = %d\n", len, result);
        return;
    }

    return;
}


static void rtk_md_testcases_seq2seq(
    uint32_t addrSrc,
    uint32_t addrDst,
    unsigned long VaddrSrc,
    unsigned long VaddrDst,
    uint32_t len)
{
    uint32_t pts_s = 0;
    uint32_t pts_e = 0;
    uint32_t result = -1;


    /*CASE - FILL 1B 0x1c*/
    pts_s = rtd_inl(TIMER_REG_VAL_ADDR);
    rtk_md_memcpy_seq(addrDst, 0x83, len, MD_CONST, MD_1B, MD_SS, 0);
    pts_e = rtd_inl(TIMER_REG_VAL_ADDR);
    result = rtk_md_check(VaddrDst, len, 0x83, MD_1B);

    if(result == -1) {
        PDEWAR("[MD] fill %d bytes with 1B mode fail!\n", len);
        return;
    } else {
        PDEWAR("[MD] fill %d bytes with 1B mode success. elapsed %d us\n", len, (pts_e - pts_s) * 1000000 / 90090);
    }


    /*CASE - COPY 1B*/
    rtk_md_memcpy_seq(addrSrc, 0x00, len, MD_CONST, MD_1B, MD_SS, 0);
    rtk_md_memcpy_seq(addrDst, 0x00, len, MD_CONST, MD_1B, MD_SS, 0);
    rtk_md_gen_data(VaddrSrc, len, 0x45, MD_1B);

    pts_s = rtd_inl(TIMER_REG_VAL_ADDR);
    rtk_md_memcpy_seq(addrDst, addrSrc, len, MD_VECTOR, MD_1B, MD_SS, 0);
    pts_e = rtd_inl(TIMER_REG_VAL_ADDR);
    result = rtk_md_check(VaddrDst, len, 0x45, MD_1B);

    if(result == -1) {
        PDEWAR("[MD] COPY %d bytes with 1B mode fail!\n", len);
        return;
    } else {
        PDEWAR("[MD] COPY %d bytes with 1B mode success. elapsed %d us\n", len, (pts_e - pts_s) * 1000000 / 90090);
    }


    /*CASE - COPY 2B*/
    rtk_md_memcpy_seq(addrSrc, 0x00, len, MD_CONST, MD_1B, MD_SS, 0);
    rtk_md_memcpy_seq(addrDst, 0x00, len, MD_CONST, MD_1B, MD_SS, 0);
    rtk_md_gen_data(VaddrSrc, len, 0xf123, MD_2B);

    pts_s = rtd_inl(TIMER_REG_VAL_ADDR);
    rtk_md_memcpy_seq(addrDst, addrSrc, len, MD_VECTOR, MD_2B, MD_SS, 0);
    pts_e = rtd_inl(TIMER_REG_VAL_ADDR);
    result = rtk_md_check(VaddrDst, len, 0x23f1, MD_2B);

    if(result == -1) {
        PDEWAR("[MD] COPY %d bytes with 2B mode fail!\n", len);
        return;
    } else {
        PDEWAR("[MD] COPY %d bytes with 2B mode success. elapsed %d us\n", len, (pts_e - pts_s) * 1000000 / 90090);
    }


    /*CASE - COPY 4B*/
    rtk_md_memcpy_seq(addrSrc, 0x00, len, MD_CONST, MD_1B, MD_SS, 0);
    rtk_md_memcpy_seq(addrDst, 0x00, len, MD_CONST, MD_1B, MD_SS, 0);
    rtk_md_gen_data(VaddrSrc, len, 0xff9abcde, MD_4B);

    pts_s = rtd_inl(TIMER_REG_VAL_ADDR);
    rtk_md_memcpy_seq(addrDst, addrSrc, len, MD_VECTOR, MD_4B, MD_SS, 0);
    pts_e = rtd_inl(TIMER_REG_VAL_ADDR);
    result = rtk_md_check(VaddrDst, len, 0xdebc9aff, MD_4B);

    if(result == -1) {
        PDEWAR("[MD] COPY %d bytes with 4B mode fail!\n", len);
        return;
    } else {
        PDEWAR("[MD] COPY %d bytes with 4B mode success. elapsed %d us\n", len, (pts_e - pts_s) * 1000000 / 90090);
    }
#if 1
    /*CASE - COPY 1B forward*/
    rtk_md_memcpy_seq(addrSrc, 0x00, len, MD_CONST, MD_1B, MD_SS, 0);
    rtk_md_memcpy_seq(addrDst, 0x00, len, MD_CONST, MD_1B, MD_SS, 0);
    rtk_md_gen_data(VaddrSrc, 1024, 0x77, MD_1B);
    rtk_md_gen_data(VaddrSrc + 1024, 1024, 0x11, MD_1B);

    pts_s = rtd_inl(TIMER_REG_VAL_ADDR);
    rtk_md_memcpy_seq(addrSrc, addrSrc + 512, 1024, MD_VECTOR, MD_1B, MD_SS, 0);
    pts_e = rtd_inl(TIMER_REG_VAL_ADDR);
    result = rtk_md_check(VaddrSrc, 512, 0x77, MD_1B);
    result += rtk_md_check(VaddrSrc + 512, 512, 0x11, MD_1B);

    if(result != 0) {
        PDEWAR("[MD] COPY %d bytes with 1B forward mode fail!\n", len);
        return;
    } else {
        PDEWAR("[MD] COPY %d bytes with 1B forward mode success. elapsed %d us\n", len, (pts_e - pts_s) * 1000000 / 90090);
    }
#endif

    /*CASE - COPY 1B backward*/
    rtk_md_memcpy_seq(addrSrc, 0x00, len, MD_CONST, MD_1B, MD_SS, 0);
    rtk_md_memcpy_seq(addrDst, 0x00, len, MD_CONST, MD_1B, MD_SS, 0);
    rtk_md_gen_data(VaddrSrc, 1024, 0x77, MD_1B);
    rtk_md_gen_data(VaddrSrc + 1024, 1024, 0x11, MD_1B);

    pts_s = rtd_inl(TIMER_REG_VAL_ADDR);
    rtk_md_memcpy_seq(addrSrc, addrSrc + 512, 1024, MD_VECTOR, MD_1B, MD_SS, 1);
    pts_e = rtd_inl(TIMER_REG_VAL_ADDR);
    result = rtk_md_check(VaddrSrc, 1024, 0x11, MD_1B);

    if(result == -1) {
        PDEWAR("[MD] COPY %d bytes with 1B backward mode fail!\n", len);
        return;
    } else {
        PDEWAR("[MD] COPY %d bytes with 1B backward mode success. elapsed %d us\n", len, (pts_e - pts_s) * 1000000 / 90090);
    }
    return;
}

void rtk_md_unit_test(void)
{
    void *srcdata = NULL;
    void *dstdata = NULL;
    dma_addr_t SPhysicalAddress = 0;
    dma_addr_t DPhysicalAddress = 0;

    srcdata = dma_alloc_coherent(md_drv_dev->md_device, 0x1000, &SPhysicalAddress, GFP_ATOMIC);//(unsigned long)rtd_inl(MD_SMQBASE_reg)+0x2000;
    if(srcdata == 0) {
        PDEWAR("[MD]alloc src fail.\n");
        goto err;
    }

    dstdata = dma_alloc_coherent(md_drv_dev->md_device, 0x1000, &DPhysicalAddress, GFP_ATOMIC);//(unsigned long)rtd_inl(MD_SMQBASE_reg)+0x4000;
    if(dstdata == 0) {
        PDEWAR("[MD]alloc dst fail.\n");
        //free_noncache(srcdata);
        goto err;
    }

    PDEWAR("[MD]test src addr = %lx, dst addr = %lx.\n", (unsigned long)srcdata, (unsigned long)dstdata);
    /*enable timer 90k */
    rtd_outl(TIMER_REG_CTRL_ADDR, 0x1);
    /*CASE - Seq to Seq test*/

    rtk_md_testcases_seq2seq((uint32_t)SPhysicalAddress, (uint32_t)DPhysicalAddress, (unsigned long)srcdata, (unsigned long)dstdata, 0x1000);

    /*CASE - pitch test*/
    rtk_md_testcases_pitch((uint32_t)SPhysicalAddress, (uint32_t)DPhysicalAddress, (unsigned long)srcdata, (unsigned long)dstdata, 0x1000);


err:
    if(srcdata != NULL)
        dma_free_coherent(md_drv_dev->md_device, 0x1000, (void*)srcdata, SPhysicalAddress);
    if(dstdata != NULL)
        dma_free_coherent(md_drv_dev->md_device, 0x1000, (void*)dstdata, DPhysicalAddress);

    return;
}


void rtk_md_bstlen_test(void)
{
    rtk_md_hw_set_burst_len(384);
    rtk_md_unit_test();

    rtk_md_hw_set_burst_len(512);
    rtk_md_unit_test();

    rtk_md_hw_set_burst_len(768);
    rtk_md_unit_test();
}
#endif

