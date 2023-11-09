
#ifndef CONFIG_CRYPTO_RTK_ZACC
#include <rtd_log/rtd_module_log.h>
#define zacc_fmt(fmt) "rtk_zacc:  " fmt
#define printk_always(fmt,...)  do{console_loglevel=6;rtd_pr_misc_err(zacc_fmt(fmt),##__VA_ARGS__);}while(0)

#include <linux/module.h>

#include <linux/kernel.h>

#include <linux/dma-mapping.h>
#include <linux/highmem.h>
#include <linux/list.h>
#include <linux/mm.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/spinlock.h>
#include <linux/smp.h>
#include <rbus/lzma_reg.h>
#include <rtk_kdriver/io.h>
#include <linux/delay.h>
#include <linux/smp.h>

#if 1
static void dmac_flush_range_ex (unsigned long start, unsigned long end)
{
    unsigned long ctr_el0;
    unsigned int cacheline_size;
    asm volatile("mrs %0, ctr_el0" : "=r" (ctr_el0) :: "memory");
    cacheline_size=2<<((ctr_el0>>16)&0xF);

    for(;start<end;start=start+cacheline_size)
    {    
        asm volatile("dc civac, %0" : : "r" (start) : "memory");
    }    
}
static void dmac_inv_range_ex (unsigned long start, unsigned long end)
{
    unsigned long ctr_el0;
    unsigned int cacheline_size;
    asm volatile("mrs %0, ctr_el0" : "=r" (ctr_el0) :: "memory");
    cacheline_size=2<<((ctr_el0>>16)&0xF);

    for(;start<end;start=start+cacheline_size)
    {    
        asm volatile("dc ivac, %0" : : "r" (start) : "memory");
    }    
}
#undef dmac_flush_range
#define dmac_flush_range dmac_flush_range_ex
#undef dmac_inv_range
#define dmac_inv_range dmac_inv_range_ex
#endif

//#define ZACC_CRC32_ENABLE
#define ZACC_PERF_ENABLE

#ifdef CONFIG_ARCH_RTK6702
#define ZACC_HW_DONE_EN        0
#else
#define ZACC_HW_DONE_EN        1
#endif

#define ZACC_DEC_TIMEOUT_COUNT1     4000
#define ZACC_ENC_TIMEOUT_COUNT1 4000

#define ZACC_MAGIC_1 0xaa55aa55
#define  ZACC_MAGIC_2_1  0x2378beaf
#define  ZACC_MAGIC_2_2  0x2379beaf
#define  ZACC_MAGIC_3  0x5A414343

#define ZACC_DESC_SIZE      0x10
#define ZACC_QUEUE_SIZE 4
unsigned int zacc_hw_resource[ZACC_QUEUE_SIZE] = {0, 0, 0, 0};

const unsigned int reg_offset[ZACC_QUEUE_SIZE]= {0, 0x24, 0x4c, 0x70};

spinlock_t zacc_hw_lock;

#define CRC32_POLY  0x04c11db7      /* AUTODIN II, Ethernet, & FDDI */
unsigned int crc32_table[256];
static void init_crc32 (void)
{
    int i, j;
    unsigned int c;
    for (i = 0; i < 256; ++i)
    {
        for (c = i << 24, j = 8; j > 0; --j)
        {
            c = c & 0x80000000 ? (c << 1) ^ CRC32_POLY : (c << 1);
        }
        crc32_table[i] = c;
    }
}
unsigned int calc_crc32 (unsigned char *buf, u32 len)
{
    unsigned char   *p;
    unsigned int    crc;
    crc = 0xffffffff;       /* preload shift register, per CRC-32 spec */
    for (p = buf; len > 0; p++)
    {
        crc = (crc << 8) ^ crc32_table[(crc >> 24) ^ *(p)];
        if (len > 0)
        {
            len --;
        }
        else
        {
            len = 0;
        }
    }
    return(crc);
}

unsigned long par_64 (unsigned long addr, int user)
{
    unsigned long ret, phys_addr;
    if(user)
    {
        asm volatile("at s1e0r, %0; isb; dsb ish" : : "r" (addr));
    }
    else
    {
        asm volatile("at s1e1r, %0; isb; dsb ish" : : "r" (addr));
    }

    asm volatile("mrs %0, par_el1; dsb ish" : "=r" (ret));
    phys_addr=ret;
    //console_loglevel=6;
    //rtd_pr_misc_err("%s %d   %016llx   %016llx\n",__func__,__LINE__,addr,phys_addr);
    return phys_addr&0xFFFFF000;
}

#if 0
#include <linux/pageremap.h>
#include <linux/rtkrecord.h>
#else
//#include <rtk_kdriver/rmm/pageremap.h>
//#include <rtk_kdriver/rmm/rtkrecord.h>
void rtk_record_list_dump(void);
#endif
//void dump_decode_data (void const *src, unsigned int size, unsigned int crc32, unsigned int crc32_orig, unsigned int line )
void dump_decode_data (void const *src, unsigned int size, unsigned int line )
{
    console_loglevel = 7;

#if 0
    list_all_rtk_memory_allocation_sort(list_mem_generic,NULL,NULL);
    show_cma_avaliable();
#else
    rtk_record_list_dump();
#endif

    print_hex_dump(KERN_ERR, "rtlzma data input : ", DUMP_PREFIX_ADDRESS,16, 1, src, size, true);
    rtd_pr_misc_err("rtlzma data dump again  %d,    src: 0x%016llx|0x%016llx\n",line,src,par_64((unsigned long)src,0));
    print_hex_dump(KERN_ERR, "rtlzma data input : ", DUMP_PREFIX_ADDRESS,16, 1, (void *)((unsigned long)src&0xfffffffffffff000ULL), PAGE_SIZE, true);

#ifdef CONFIG_RTK_FEATURE_FOR_GKI
    dump_page(virt_to_page(src), "dump lzma decode buffer page");
#endif

    //panic("rtlzma data error! crc32: 0x%x|0x%x, src: 0x%016llx|0x%016llx\n", crc32, crc32_orig, src,par_64(src,0));
    panic("rtlzma data error! src: 0x%016llx|0x%016llx\n", src,par_64((unsigned long)src,0));
}


#pragma pack(push, 1)
struct zacc_encode_desc
{
    u32 encode:             1;      /* 0      - Encode mode (SW) */
    u32 decode:             1;      /* 1      - Decode mode (SW) */
    u32 bypss:              1;      /* 2      - Bypass mode (SW) */
    u32 cmd_err:            1;      /* 3      - Cmd error (HW) */
    u32 src_addr:           28;     /* 31:4   - Data source address (SW) */
    u32 hw_done:            1;      /* 32     - HW done (HW) */
    u32 over_4k:            1;      /* 33     - Size over 4KB (HW) */
    u32 unused_2:           2;      /* 35:34  - Unused */
    u32 dst_addr:           28;     /* 63:36  - Data destination address (SW) */
    u32 comp_size:          13;     /* 76:64  - Compressed size (HW) */
    u32 unused_1:           3;      /* 79:77  - Unused */
    u32 crc:                16;     /* 95:80  - Crc result (HW) */
    u32 unused_0:           32;     /* 127:96 - Unused */
};

struct zacc_decode_desc
{
    u32 encode:             1;      /*0       - Encode mode (SW) */
    u32 decode:             1;      /*1       - Decode mode (SW) */
    u32 bypass:             1;      /*2       - Bypass mode (SW) */
    u32 cmd_err:            1;      /*3       - Cmd error (HW) */
    u32 src_addr:           28;     /*31:4    - Data source address (SW) */
    u32 hw_done:            1;      /*32      - HW done (HW) */
    u32 size_err:           1;      /*33      - Size error (HW) */
    u32 decode_err:         1;      /*34      - Decode error (HW) */
    u32 dist_err:           1;      /*35      - Distance error (HW) */
    u32 dst_addr:           28;     /*63:36   - Data destination address (SW) */
    u32 comp_size:          23;     /*86:64   - Compressed size, 16bytes alignmen (SW) */
    u32 crc_err:            1;      /*87      - CRC error (HW) */
    u32 crc_msb:            8;      /*95:88   - Golden crc msb 8 bit (SW) */
    u32 decomp_size:        23;     /*118:96  - Decompressed size, byte (SW) */
    u32 unused_0:           1;      /*119     - Unused */
    u32 crc_lsb:            8;      /*127:120 - Golden crc lsb 8 bit (SW) */
};

struct zacc_desc
{
    u32 data[64];
};

#pragma pack(pop)

static void *desc_uncached;
static struct page *desc_page;
static dma_addr_t desc_phys;

static struct zacc_desc *desc;
static struct page *dst_pg[ZACC_QUEUE_SIZE];
static struct page *src_pg[ZACC_QUEUE_SIZE];
static struct page *src_tmp_pg[ZACC_QUEUE_SIZE];


unsigned int zacc_get_hw_resource (void)
{
    unsigned long flags;
    unsigned int index = 0xffffffff;
    unsigned int i;

    do
    {
        spin_lock_irqsave(&zacc_hw_lock, flags);

        for (i = 0; i < ZACC_QUEUE_SIZE; i++)
        {
            if (zacc_hw_resource[i] == 0)
            {
                index = i;
                zacc_hw_resource[i] = 1;
                break;
            }
        }

        spin_unlock_irqrestore(&zacc_hw_lock, flags);
    }
    while (index == 0xffffffff);
    return index;
}

void zacc_put_hw_resource(unsigned int index)
{
    unsigned long flags;
    spin_lock_irqsave(&zacc_hw_lock, flags);

    if (zacc_hw_resource[index] == 1)
    {
        zacc_hw_resource[index] = 0;
    }

    spin_unlock_irqrestore(&zacc_hw_lock, flags);
}


static int enc_cmd_parser ( struct zacc_encode_desc *enc_desc, dma_addr_t src_pa, dma_addr_t dst_pa )
{
    if(NULL==enc_desc)
    {
        return 1;
    }

    if(enc_desc->encode!=1)
    {
        printk_always("%s %d  encode=%d\n",__func__,__LINE__,enc_desc->encode);
        return 1;
    }

    if(enc_desc->cmd_err)
    {
        printk_always("%s %d  cmd_err\n",__func__,__LINE__);
        return 1;
    }

    if(enc_desc->over_4k)
    {
        //printk_always("%s %d  over_4k\n",__func__,__LINE__);
        return 0;
    }

    if(enc_desc->unused_1 || enc_desc->unused_2)
    {
        printk_always("%s %d  unused_1=%d unused_2=%d\n",__func__,__LINE__,enc_desc->unused_1, enc_desc->unused_2);
        return 1;
    }

    if(enc_desc->src_addr!=(src_pa >> 4))
    {
        printk_always("%s %d  src_addr=%x   dst_pa >> 4=%x (%x)=%x\n",__func__,__LINE__,enc_desc->src_addr,(src_pa >> 4),src_pa);
        return 1;
    }

    if(enc_desc->dst_addr!=(dst_pa >> 4))
    {
        printk_always("%s %d  dst_addr=%x   dst_pa >> 4=%x (%x)=%x\n",__func__,__LINE__,enc_desc->dst_addr,(dst_pa >> 4),dst_pa);
        return 1;
    }

    return 0;
}

static int dec_cmd_parser ( struct zacc_decode_desc *dec_desc, dma_addr_t src_pa, dma_addr_t dst_pa, size_t  comp_size )
{
    if(NULL==dec_desc)
    {
        return 1;
    }

    if(dec_desc->decode!=1)
    {
        printk_always("%s %d  decode=%d\n\n\n",__func__,__LINE__,dec_desc->decode);
        return 1;
    }

    if(dec_desc->cmd_err)
    {
        printk_always("%s %d  cmd_err\n\n\n",__func__,__LINE__);
        return 1;
    }

    if(dec_desc->size_err)
    {
        printk_always("%s %d  size_err\n\n\n",__func__,__LINE__);
        return 1;
    }

    if(dec_desc->decode_err)
    {
        printk_always("%s %d  decode_err\n\n\n",__func__,__LINE__);
        return 1;
    }

    if(dec_desc->dist_err)
    {
        printk_always("%s %d  dist_err\n\n\n",__func__,__LINE__);
        return 1;
    }

    if(dec_desc->crc_err)
    {
        printk_always("%s %d  crc_err\n\n\n",__func__,__LINE__);
        return 1;
    }

    if(dec_desc->unused_0)
    {
        printk_always("%s %d  unused_0\n\n\n",__func__,__LINE__);
        return 1;
    }

    if(dec_desc->decomp_size!=PAGE_SIZE)
    {
        printk_always("%s %d  decomp_size=%x\n\n\n",__func__,__LINE__,dec_desc->decomp_size);
        return 1;
    }

    if(dec_desc->src_addr!=(src_pa >> 4))
    {
        printk_always("%s %d  src_addr=%x   dst_pa >> 4=%x (%x)=%x\n\n\n",__func__,__LINE__,dec_desc->src_addr,(src_pa >> 4),src_pa);
        return 1;
    }

    if(dec_desc->dst_addr!=(dst_pa >> 4))
    {
        printk_always("%s %d  dst_addr=%x   dst_pa >> 4=%x (%x)=%x\n\n\n",__func__,__LINE__,dec_desc->dst_addr,(dst_pa >> 4),dst_pa);
        return 1;
    }

    if(dec_desc->comp_size!=comp_size )
    {
        printk_always("%s %d  comp_size=%x  %x\n\n\n",__func__,__LINE__,dec_desc->comp_size,comp_size);
        return 1;
    }

    return 0;
}

ssize_t zacc_encode (void *dst, void const *src)
{
    struct zacc_encode_desc *encode_desc;
    //struct zacc_encode_desc tmp_encode_desc;
    dma_addr_t dst_pa, src_pa, desc_pa ;
    //dma_addr_t _dst, _src, ;
    unsigned int err, index;
    unsigned int comp_size, aligned_size, crc, err_count, timeout, cmd_status;
    bool need_recovery = false;
    unsigned int *ptr;

#if (ZACC_HW_DONE_EN == 1)
    unsigned int hw_done_count;
#endif

    if (!dst || !src)
    {
        return -EINVAL;
    }

    preempt_disable();

    //rtd_pr_misc_err("%s %d\n",__func__,__LINE__);
    index = zacc_get_hw_resource();
    //desc_pa = desc_uncached+index;
    desc_pa = desc_phys+index*sizeof(struct zacc_desc);

    dst_pa=par_64((unsigned long)dst,0)+((unsigned long)dst&0xFFF);//page_to_phys(src_pg);
    src_pa=par_64((unsigned long)src,0)+((unsigned long)src&0xFFF);//page_to_phys(src_pg);
    dmac_flush_range((unsigned long)src,((unsigned long)src)+PAGE_SIZE);
    dmac_inv_range((unsigned long)dst,(unsigned long)dst+PAGE_SIZE);

    encode_desc = (struct zacc_encode_desc *)((struct zacc_desc*)desc+index);
    memset((void *)encode_desc, 0, ZACC_DESC_SIZE);
    encode_desc->encode = 1;
    encode_desc->dst_addr = dst_pa >> 4;
    encode_desc->src_addr = src_pa >> 4;
    dmac_flush_range((unsigned long)encode_desc,(unsigned long)((struct zacc_desc*)encode_desc+1));

    rtd_outl(LZMA_CMD_0_BASE_reg + reg_offset[index], desc_pa);
    rtd_outl(LZMA_CMD_0_LIMIT_reg + reg_offset[index], (desc_pa + 0x20));
    rtd_outl(LZMA_CMD_0_RPTR_reg + reg_offset[index], desc_pa);
    rtd_outl(LZMA_CMD_0_WPTR_reg + reg_offset[index], (desc_pa + 0x10));

#if (ZACC_HW_DONE_EN == 1)
    hw_done_count = rtd_inl(LZMA_CMD_0_dec_reg + reg_offset[index]) & 0x0f;
#endif
    wmb();

    rtd_outl(LZMA_CMD_GO_SWAP_reg, 0x04000000 | (0xf << (0x6 * index)));
    wmb();

    err_count = 0;
    timeout = ZACC_ENC_TIMEOUT_COUNT1;
    while (1)
    {
        dmac_inv_range((unsigned long)encode_desc,(unsigned long)((struct zacc_desc*)encode_desc+1));

        cmd_status = rtd_inl(LZMA_CMD_GO_SWAP_reg);
        if (((cmd_status & (0x1 << (0x6 * index))) == 0) && encode_desc->hw_done
#if (ZACC_HW_DONE_EN == 1)
                && (hw_done_count != (rtd_inl(LZMA_CMD_0_dec_reg + reg_offset[index]) & 0x0f))
#endif
           )
        {
            break;
        }

        if (++err_count >= timeout)
        {
            need_recovery = true;
            printk_always("%s %d  timeout 111    go bit status =%x  index=%d   0x1 0x40 0x1000 0x40000\n",__func__,__LINE__,rtd_inl(LZMA_CMD_GO_SWAP_reg),index);
#if (ZACC_HW_DONE_EN == 1)
            printk_always("%s %d  timeout 111    hw_done_count =%x  %x\n",__func__,__LINE__,hw_done_count,(rtd_inl(LZMA_CMD_0_dec_reg + reg_offset[index]) & 0x0f));
#endif
            break;
        }
        //udelay(1);
    }

    //memcpy((void *)&tmp_encode_desc, (void *)encode_desc, ZACC_DESC_SIZE);
    if(enc_cmd_parser(encode_desc, src_pa, dst_pa))
    {
        rtd_pr_misc_err("%s %d  error\n\n\n",__func__,__LINE__);
        need_recovery = true;
    }

    comp_size = encode_desc->comp_size;
    aligned_size = ((comp_size + 0xf) >> 4) << 4;
    if((encode_desc->over_4k)||( (aligned_size+16) >= PAGE_SIZE))
    {
        aligned_size = PAGE_SIZE;
    }

    if (need_recovery)
    {
#ifdef ZACC_ENABLE_RECOVERY
        zacc_enc_recovery();
#else
        printk_always("src_pa = %lx, dst_pa = %lx\n", (unsigned long)src_pa, (unsigned long)dst_pa);
        panic("rtk-zacc encode error !! desc(0x%lx): 0x%x, 0x%x, 0x%x, 0x%x\n",
              (unsigned long)encode_desc, ((struct zacc_desc*)encode_desc)->data[0], ((struct zacc_desc*)encode_desc)->data[1],
              ((struct zacc_desc*)encode_desc)->data[2], ((struct zacc_desc*)encode_desc)->data[3]);
#endif
    }

#if (ZACC_HW_DONE_EN == 0)
    udelay(1);
#endif

    //memset(desc, 0, ZACC_DESC_SIZE << 2);
    //dma_unmap_page(dev, dst_pa, PAGE_SIZE, DMA_FROM_DEVICE);
    //dma_unmap_page(dev, src_pa, PAGE_SIZE, DMA_TO_DEVICE);
    dmac_inv_range((unsigned long)dst,(unsigned long)dst+PAGE_SIZE);

    rmb();

    if (aligned_size == PAGE_SIZE)
    {
        memcpy(dst, src, PAGE_SIZE);
        err = PAGE_SIZE;
    }
    else
    {
        crc = encode_desc->crc;
        ((unsigned char *)dst)[aligned_size + 3] = 0xFF;
        ((unsigned char *)dst)[aligned_size + 2] = aligned_size - comp_size;
        ((unsigned char *)dst)[aligned_size + 1] = (crc >> 8) & 0xff;
        ((unsigned char *)dst)[aligned_size] = crc & 0xff;

        ptr=dst+aligned_size+4;
#ifdef ZACC_CRC32_ENABLE
        *ptr=calc_crc32(dst, aligned_size);
#else
        *ptr=ZACC_MAGIC_1;
#endif

        ptr=dst+aligned_size+8;
        *ptr=ZACC_MAGIC_2_1;
        *(++ptr)=ZACC_MAGIC_3;

        aligned_size += 16;
        err = aligned_size;
    }

    zacc_put_hw_resource(index);
    preempt_enable();
    return err;
}


ssize_t zacc_decode (void *dst, void const *src, size_t size)
{
    struct zacc_decode_desc *decode_desc;
    dma_addr_t dst_pa, src_pa, _src, desc_pa;
    //dma_addr_t _dst;
    int err=PAGE_SIZE, index;
    unsigned int padding_size, err_count, timeout, cmd_status;
    //bool do_retry = false, do_recovery = false;
    //unsigned long flags;
    unsigned int *ptr;

#if (ZACC_HW_DONE_EN == 1)
    unsigned int hw_done_count;
#endif

    if (!dst || !src || ((unsigned long)src&0x3) || ((unsigned long)dst&0x3) )
    {
        return -EINVAL;
    }

    if (size == PAGE_SIZE)
    {
        memcpy(dst, src, PAGE_SIZE);
        return PAGE_SIZE;
    }

    preempt_disable();

    //rtd_pr_misc_err("%s %d\n",__func__,__LINE__);
    index = zacc_get_hw_resource();
    //desc_pa = (struct zacc_desc *)desc_uncached+index;
    desc_pa = desc_phys+index*sizeof(struct zacc_desc);

#if 0
    dmac_flush_range(src,src+size);
    src_pa=par_64(src,0)+((unsigned int)(src)&0xFFF);//page_to_phys(src_pg);
#else
    _src=(dma_addr_t)page_address(src_tmp_pg[index]);
    memcpy((void*)_src,src,size);

    dmac_flush_range(_src,_src+size);
    src_pa= page_to_phys(src_tmp_pg[index]);
#endif

    dmac_inv_range((unsigned long)dst,(unsigned long)dst+PAGE_SIZE);

    dst_pa=par_64((unsigned long)dst,0)+((unsigned long)dst&0xFFF);//page_to_phys(src_pg);

    decode_desc = (struct zacc_decode_desc *)((struct zacc_desc*)desc+index);
    memset((void *)decode_desc, 0, ZACC_DESC_SIZE);
    decode_desc->decode = 1;
    decode_desc->dst_addr = dst_pa >> 4;
    decode_desc->src_addr = src_pa >> 4;

    padding_size = 16 + ((unsigned char *)src)[size - 14];
    decode_desc->comp_size = size - padding_size;
    decode_desc->decomp_size = PAGE_SIZE;
    decode_desc->crc_msb = ((unsigned char *)src)[size - 15];
    decode_desc->crc_lsb = ((unsigned char *)src)[size - 16];
    dmac_flush_range((unsigned long)decode_desc,(unsigned long)((struct zacc_desc*)decode_desc+1));

    ptr=(unsigned int*)(((unsigned char *)src)+size-12);

#ifdef ZACC_CRC32_ENABLE
    if(ptr[0]!=calc_crc32((unsigned char *)src, size-16))
    {
        printk_always("%s %d.  enc crc check fail!   0x%08x 0x%08x\n",__func__,__LINE__,ptr[0],calc_crc32((unsigned char *)src, size-16));
        dump_decode_data(src,size,__LINE__);
    }
#endif

    if(ptr[1]!=ZACC_MAGIC_2_1 || ptr[2]!=ZACC_MAGIC_3
#ifndef ZACC_CRC32_ENABLE
            || ptr[0]!=ZACC_MAGIC_1
#endif
      )
    {
        dump_decode_data(src,size,__LINE__);
    }

    rtd_outl(LZMA_CMD_0_BASE_reg + reg_offset[index], desc_pa);
    rtd_outl(LZMA_CMD_0_LIMIT_reg + reg_offset[index], (desc_pa + 0x20));
    rtd_outl(LZMA_CMD_0_RPTR_reg + reg_offset[index], desc_pa);
    rtd_outl(LZMA_CMD_0_WPTR_reg + reg_offset[index], (desc_pa + 0x10));

#if (ZACC_HW_DONE_EN == 1)
    hw_done_count = rtd_inl(LZMA_CMD_0_dec_reg + reg_offset[index]) & 0x0f;
#endif

    wmb();
    rtd_outl(LZMA_CMD_GO_SWAP_reg, 0x04000000 | (0xf << (0x6 * index)));
    wmb();

    err_count = 0;
    timeout = ZACC_DEC_TIMEOUT_COUNT1;

    while (1)
    {
        dmac_inv_range((unsigned long)decode_desc,(unsigned long)((struct zacc_desc*)decode_desc+1));

        cmd_status = rtd_inl(LZMA_CMD_GO_SWAP_reg);
        if (((cmd_status & (0x1 << (0x6 * index))) == 0) && (decode_desc->hw_done)
#if (ZACC_HW_DONE_EN == 1)
                && (hw_done_count != (rtd_inl(LZMA_CMD_0_dec_reg + reg_offset[index]) & 0x0f))
#endif
           )
        {
            break;
        }

        if (++err_count >= timeout)
        {
            console_loglevel=6;
            printk_always("%s %d  timeout 111    go bit status =%x  index=%d   0x1 0x40 0x1000 0x40000\n",__func__,__LINE__,rtd_inl(LZMA_CMD_GO_SWAP_reg),index);
#if (ZACC_HW_DONE_EN == 1)
            printk_always("%s %d  timeout 111    hw_done_count =%x  %x\n",__func__,__LINE__,hw_done_count,(rtd_inl(LZMA_CMD_0_dec_reg + reg_offset[index]) & 0x0f));
#endif
            break;
        }

        //udelay(1);
    }

    //memcpy((void *)&tmp_decode_desc, (void *)decode_desc, ZACC_DESC_SIZE);
    if (dec_cmd_parser(decode_desc,src_pa,dst_pa,(size - padding_size)))
    {
        rtd_pr_misc_err("%s %d  error\n\n\n",__func__,__LINE__);
        dump_decode_data(src,size,__LINE__);
    }

#if (ZACC_HW_DONE_EN == 0)
    udelay(1);
#endif

    dmac_inv_range((unsigned long)dst,(unsigned long)dst+PAGE_SIZE);
    rmb();

    zacc_put_hw_resource(index);
    preempt_enable();

    return err;
}


//#include <linux/module.h>
//#include <linux/kernel.h>

#include <linux/crypto.h>
#include <linux/init.h>
//#include <linux/zacc.h>
static int zacc_alg_init(struct crypto_tfm *tfm)
{
    return 0;
}

static void zacc_alg_exit(struct crypto_tfm *tfm)
{
}

static int zacc_alg_compress (struct crypto_tfm *tfm, u8 const *src, unsigned int slen, u8 *dst, unsigned int *dlen)
{
    ssize_t rv = zacc_encode(dst, src);
    /*
     * On any error-return from zacc_encode, just pretend not to be compressed at all.
     * This may invoke zcomp to memcpy the page.
     */
    *dlen = (rv < 0) ? PAGE_SIZE : (unsigned int)rv;
    return 0;
}

static int zacc_alg_decompress (struct crypto_tfm *tfm, u8 const *src, unsigned int slen, u8 *dst, unsigned int *dlen)
{
    ssize_t rv = zacc_decode(dst, src, slen);
    /* Return 0 on success, or pass any error-return. */
    return (rv < 0) ? (int)rv : 0;
}

static struct crypto_alg zacc_alg =
{
    .cra_list       = LIST_HEAD_INIT(zacc_alg.cra_list),
    .cra_name       = "zacc",
    .cra_driver_name = "rtk-zacc",
    .cra_flags      = CRYPTO_ALG_TYPE_COMPRESS,
    .cra_compress.coa_compress      = zacc_alg_compress,
    .cra_compress.coa_decompress    = zacc_alg_decompress,
    .cra_init       = zacc_alg_init,
    .cra_exit       = zacc_alg_exit,
    .cra_module     = THIS_MODULE,
};

static int rtk_zacc_init (void)
{
    rtd_maskl(LZMA_LZMA_DMA_RD_Ctrl_reg, 0xfff0fcfe, (0x2 << 18) | (0x3 << 16) | (0x3 << 8) | (0x1 << 0));
    rtd_maskl(LZMA_LZMA_DMA_WR_Ctrl_reg, 0xfffcfcfe, (0x2 << 16) | (0x3 << 8) | (0x1 << 0));
    rtd_maskl(LZMA_INT_STATUS_reg, 0x3fffffdf, (ZACC_HW_DONE_EN << 31) | (0x0 << 30) | (0x1 << 5));
    rtd_maskl(LZMA_LZMA_DUMMY_reg, 0xbfffffff, 0x1 << 30);
    return 0;
}


#ifdef ZACC_PERF_ENABLE
#include <linux/cdev.h>

#define NUM_LOOPS       128
#define BUFF_OFFS       4

enum lzo_perf_minor
{
#if 0
    LZO_0 = 0,
    LZO_1,
    LZO_2,
    LZO_3,
    LZ4_0,
    LZ4_1,
    LZ4_2,
    LZ4_3,
#endif
    ZACC_0,
    ZACC_1,
    ZACC_2,
    ZACC_3,
    NUM_MINOR
};

struct lzo_perf_drv
{
    char const *name;

    size_t work_size;

    int (*compress)(u8 const *, size_t, u8 *, size_t *, void *);
    int (*decompress)(u8 const *, size_t, u8 *, size_t *);
};

struct lzo_perf_stat
{
    char const *name;

    u64 bytes_comp;
    u64 bytes_decomp;

    u64 nsecs_comp;
    u64 nsecs_decomp;

    bool busy;

    struct mutex mutex;
};

static struct lzo_perf_stat lzo_perf_stat[NUM_MINOR] =
{
#if 0
    { .name = "lzo-perf0", },
    { .name = "lzo-perf1", },
    { .name = "lzo-perf2", },
    { .name = "lzo-perf3", },
    { .name = "lz4-perf0", },
    { .name = "lz4-perf1", },
    { .name = "lz4-perf2", },
    { .name = "lz4-perf3", },
#endif
    { .name = "zacc-perf0", },
    { .name = "zacc-perf1", },
    { .name = "zacc-perf2", },
    { .name = "zacc-perf3", },
};

static int zacc_compress (u8 const *src, size_t ssz, u8 *dst, size_t *dsz, void *priv)
{
    ssize_t rv;
    rv = zacc_encode(dst, src);
    if (rv < 0)
    {
        return (int)rv;
    }

    *dsz = (size_t)rv;
    return 0;
}

static int zacc_decompress (u8 const *src, size_t ssz, u8 *dst, size_t *dsz)
{
    ssize_t rv;
    rv = zacc_decode(dst, src, ssz);
    if (rv < 0)
    {
        return (int)rv;
    }

    *dsz = (size_t)rv;
    return 0;
}

static struct lzo_perf_drv lzo_perf_drv[NUM_MINOR] =
{
#if 0
    {
        .name           = "lzo",
        .work_size      = LZO1X_1_MEM_COMPRESS,
        .compress       = lzo1x_1_compress,
        .decompress     = lzo1x_decompress_safe,
    }, {
        .name           = "lzo",
        .work_size      = LZO1X_1_MEM_COMPRESS,
        .compress       = lzo1x_1_compress,
        .decompress     = lzo1x_decompress_safe,
    }, {
        .name           = "lzo",
        .work_size      = LZO1X_1_MEM_COMPRESS,
        .compress       = lzo1x_1_compress,
        .decompress     = lzo1x_decompress_safe,
    }, {
        .name           = "lzo",
        .work_size      = LZO1X_1_MEM_COMPRESS,
        .compress       = lzo1x_1_compress,
        .decompress     = lzo1x_decompress_safe,
    }, {
        .name           = "lz4",
        .work_size      = LZ4_MEM_COMPRESS,
        .compress       = lz4_compress,
        .decompress     = lz4_decompress_unknownoutputsize,
    }, {
        .name           = "lz4",
        .work_size      = LZ4_MEM_COMPRESS,
        .compress       = lz4_compress,
        .decompress     = lz4_decompress_unknownoutputsize,
    }, {
        .name           = "lz4",
        .work_size      = LZ4_MEM_COMPRESS,
        .compress       = lz4_compress,
        .decompress     = lz4_decompress_unknownoutputsize,
    }, {
        .name           = "lz4",
        .work_size      = LZ4_MEM_COMPRESS,
        .compress       = lz4_compress,
        .decompress     = lz4_decompress_unknownoutputsize,
    },
#endif
    {
        .name           = "zacc",
        .work_size      = 0,
        .compress       = zacc_compress,
        .decompress     = zacc_decompress,
    }, {
        .name           = "zacc",
        .work_size      = 0,
        .compress       = zacc_compress,
        .decompress     = zacc_decompress,
    }, {
        .name           = "zacc",
        .work_size      = 0,
        .compress       = zacc_compress,
        .decompress     = zacc_decompress,
    }, {
        .name           = "zacc",
        .work_size      = 0,
        .compress       = zacc_compress,
        .decompress     = zacc_decompress,
    },
};

struct lzo_perf_desc
{
    struct lzo_perf_drv *drv;
    void *work_buf;

    struct page *data_page;
    struct page *comp_page;

    void *data_buf;
    void *comp_buf;

    spinlock_t lock;
};

static ssize_t lzo_perf_read (struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
    struct lzo_perf_stat *stat = &lzo_perf_stat[iminor(file->f_inode)];
    char str[85];
    ssize_t size;

    if (*ppos)
    {
        return 0;
    }

    mutex_lock(&stat->mutex);

    size = sprintf(str, "%s %llu %llu %llu %llu\n", stat->name,
                   stat->bytes_comp, stat->bytes_decomp,
                   stat->nsecs_comp, stat->nsecs_decomp);

    mutex_unlock(&stat->mutex);

    if (count < size)
    {
        return -EINVAL;
    }
    if (copy_to_user(buf, str, size))
    {
        return -EFAULT;
    }

    *ppos += size;

    return size;
}

static ssize_t lzo_perf_write (struct file *file, char const __user *buf, size_t count, loff_t *ppos)
{
    struct lzo_perf_stat *stat = &lzo_perf_stat[iminor(file->f_inode)];
    struct lzo_perf_desc *desc = file->private_data;
    ktime_t comp_time, decomp_time;
    ssize_t written = min(count, (size_t)PAGE_SIZE);
    size_t data_size, comp_size;
    int err, i;

    //rtd_pr_misc_err("[lzo-perf] %s: written size = %d\n", stat->name, written);
    if (copy_from_user(desc->data_buf, buf, written))
    {
        return -EFAULT;
    }

    if (written < PAGE_SIZE)
    {
        memset(desc->data_buf + written, 0, PAGE_SIZE - written);
    }

    spin_lock_irq(&desc->lock);

    memcpy(desc->comp_buf+PAGE_SIZE, desc->data_buf, PAGE_SIZE);

    /* loop compression */
    comp_time = ktime_get();
    for (i = 0; i < NUM_LOOPS; i++)
    {
        comp_size = PAGE_SIZE * 2;
        err = desc->drv->compress(desc->data_buf, PAGE_SIZE,
                                  desc->comp_buf, &comp_size,
                                  desc->work_buf);
        if (err < 0)
        {
            panic("lzma compress err! %d\n",err);

            /* compression failed -- memcpy */
            memcpy(desc->comp_buf, desc->data_buf, PAGE_SIZE);
            comp_size = PAGE_SIZE;
        }
    }
    comp_time = ktime_sub(ktime_get(), comp_time);

    spin_unlock_irq(&desc->lock);

#if 0
    if (comp_size != PAGE_SIZE)
    {
        memmove(desc->comp_buf + BUFF_OFFS, desc->comp_buf, comp_size);
    }
#endif

    spin_lock_irq(&desc->lock);

    /* loop decompression */
    decomp_time = ktime_get();
    for (i = 0; i < NUM_LOOPS; i++)
    {
        if (comp_size == PAGE_SIZE)
        {
            /* compression failed -- memcpy */
            memcpy(desc->data_buf, desc->comp_buf, PAGE_SIZE);
            data_size = PAGE_SIZE;
            continue;
        }

        data_size = PAGE_SIZE;
        //err = desc->drv->decompress(desc->comp_buf + BUFF_OFFS, comp_size,
        err = desc->drv->decompress(desc->comp_buf, comp_size, desc->data_buf, &data_size);
        if (err < 0)
        {
            panic("failed to decompress data: %d\n", err);
            break;
        }
    }
    decomp_time = ktime_sub(ktime_get(), decomp_time);

    if(memcmp(desc->data_buf,desc->comp_buf+PAGE_SIZE,PAGE_SIZE))
    {
        console_loglevel=6;
        print_hex_dump(KERN_ERR, "data_buf: ", DUMP_PREFIX_ADDRESS, 16, 1, desc->data_buf, 0x100, true);
        print_hex_dump(KERN_ERR, "data_buf+PAGE_SIZE: ", DUMP_PREFIX_ADDRESS, 16, 1, desc->comp_buf+PAGE_SIZE, 0x100, true);
        panic("decompress data error!\n");
    }

    spin_unlock_irq(&desc->lock);

    mutex_lock(&stat->mutex);

    /* update statistics */
    stat->bytes_comp += PAGE_SIZE * NUM_LOOPS;
    stat->bytes_decomp += comp_size * NUM_LOOPS;
    stat->nsecs_comp += ktime_to_ns(comp_time);
    stat->nsecs_decomp += ktime_to_ns(decomp_time);

    mutex_unlock(&stat->mutex);

    *ppos += written;

    return written;
}

static int lzo_perf_init_desc (struct lzo_perf_desc *desc, int minor)
{
    desc->drv = &lzo_perf_drv[minor];

    desc->work_buf = kmalloc(desc->drv->work_size, GFP_KERNEL);
    if (!desc->work_buf)
    {
        return -ENOMEM;
    }

    desc->data_page = alloc_page(GFP_KERNEL);
    if (!desc->data_page)
    {
        return -ENOMEM;
    }
    desc->data_buf = page_address(desc->data_page);

    desc->comp_page = alloc_pages(GFP_KERNEL, 1);
    if (!desc->comp_page)
    {
        return -ENOMEM;
    }
    desc->comp_buf = page_address(desc->comp_page);

    spin_lock_init(&desc->lock);

    return 0;
}

static int lzo_perf_open (struct inode *inode, struct file *file)
{
    struct lzo_perf_stat *stat = &lzo_perf_stat[iminor(inode)];
    struct lzo_perf_desc *desc;
    int err = 0;

    if (!(file->f_mode & FMODE_WRITE))
    {
        goto _return_;
    }

    mutex_lock(&stat->mutex);

    if (stat->busy)
    {
        err = -EBUSY;
        goto _unlock_;
    }

    desc = vzalloc(sizeof(*desc));
    if (!desc)
    {
        err = -ENOMEM;
        goto _unlock_;
    }

    lzo_perf_init_desc(desc, iminor(inode));

    file->private_data = desc;

    /* reset statistics */
    stat->bytes_comp = 0;
    stat->bytes_decomp = 0;
    stat->nsecs_comp = 0;
    stat->nsecs_decomp = 0;

    stat->busy = true;
_unlock_:
    mutex_unlock(&stat->mutex);
_return_:
    return err;
}

static int lzo_perf_release (struct inode *inode, struct file *file)
{
    struct lzo_perf_stat *stat = &lzo_perf_stat[iminor(inode)];
    struct lzo_perf_desc *desc = file->private_data;

    if (!(file->f_mode & FMODE_WRITE))
    {
        goto _return_;
    }

    mutex_lock(&stat->mutex);

    stat->busy = false;

    mutex_unlock(&stat->mutex);

    __free_page(desc->data_page);
    __free_pages(desc->comp_page, 1);
    kfree(desc->work_buf);
    vfree(desc);
_return_:
    return 0;
}

static struct file_operations const lzo_perf_fops =
{
    .owner          = THIS_MODULE,
    .read           = lzo_perf_read,
    .write          = lzo_perf_write,
    .open           = lzo_perf_open,
    .release        = lzo_perf_release,
};

static struct cdev lzo_perf_cdev;
static dev_t lzo_perf_dev;
static int lzo_perf_init (void)
{
    int err, i;

    rtd_pr_misc_err("%s %d\n",__func__,__LINE__);
    err = alloc_chrdev_region(&lzo_perf_dev, 0, NUM_MINOR, "lzo-perf");
    if (err < 0)
    {
        goto _return_;
    }

    cdev_init(&lzo_perf_cdev, &lzo_perf_fops);
    lzo_perf_cdev.owner = THIS_MODULE;

    err = cdev_add(&lzo_perf_cdev, lzo_perf_dev, NUM_MINOR);
    if (err < 0)
    {
        goto _unregister_;
    }

    rtd_pr_misc_err("[lzo-perf] major = %d, NUM_MINOR = %d\n",
           MAJOR(lzo_perf_dev), NUM_MINOR);

    for (i = 0; i < NUM_MINOR; i++)
    {
        mutex_init(&lzo_perf_stat[i].mutex);
    }

    rtd_pr_misc_err("%s %d\n",__func__,__LINE__);
    return 0;

_unregister_:
    unregister_chrdev_region(lzo_perf_dev, NUM_MINOR);
_return_:
    return err;
}

#if 0
static void lzo_perf_exit(void)
{
    cdev_del(&lzo_perf_cdev);
    unregister_chrdev_region(lzo_perf_dev, NUM_MINOR);
}
#endif

#endif


int zacc_suspend (void)
{
    //zacc_stop(zdev);
    return 0;
}

void zacc_resume (void)
{
    //struct zacc_dev *zdev = dev_get_drvdata(dev);
    rtk_zacc_init();
    return ;
}

#include <linux/syscore_ops.h>
static struct syscore_ops zacc_syscore_ops =
{
    .suspend = zacc_suspend,
    .resume = zacc_resume,
};

static int __init zacc_init (void)
{
    int i;
    int err;

    spin_lock_init(&zacc_hw_lock);
    rtk_zacc_init();
    init_crc32();
    register_syscore_ops(&zacc_syscore_ops);

    err=crypto_register_alg(&zacc_alg);
    if (err)
    {
        rtd_pr_misc_err("%s %d.  zacc register alg error %d!\n",__func__,__LINE__,err);
        return -1;
    }

    //desc=dvr_malloc_uncached(sizeof(struct zacc_desc)*ZACC_QUEUE_SIZE, (void **)&desc_uncached);
    desc_page=(void*)alloc_pages(GFP_DMA32, 0);
    if(NULL==desc_page)
    {
        rtd_pr_misc_err("%s %d.  descriptor alloc fail!\n",__func__,__LINE__);
        goto failed;
    }

    desc=page_address(desc_page);
    desc_phys=page_to_phys(desc_page);

    for (i = 0; i < ZACC_QUEUE_SIZE; i++)
    {
        dst_pg[i] = alloc_pages(GFP_DMA32, 1);
        if (!dst_pg[i])
        {
            goto failed;
        }

        src_pg[i] = alloc_pages(GFP_DMA32, 0);
        if (!src_pg[i])
        {
            goto failed;
        }

        src_tmp_pg[i] = alloc_pages(GFP_DMA32, 0);
        if (!src_tmp_pg[i])
        {
            goto failed;
        }
    }

#ifdef ZACC_PERF_ENABLE
    lzo_perf_init();
#endif

    rtd_pr_misc_err("%s %d success!\n",__func__,__LINE__);
    return 0;

failed:
    if(desc_page)
    {
        __free_page(desc_page);
        desc=NULL;
        desc_uncached=NULL;
    }

    for (i = 0; i < ZACC_QUEUE_SIZE; i++)
    {
        if (dst_pg[i])
        {
            __free_pages(dst_pg[i], 1);
            dst_pg[i]=0;
        }

        if (!src_pg[i])
        {
            __free_pages(src_pg[i], 1);
            src_pg[i]=0;
        }

        if (!src_tmp_pg[i])
        {
            __free_pages(src_tmp_pg[i], 1);
            src_tmp_pg[i]=0;
        }
    }

    return -1;
}


#ifdef MODULE
module_init(zacc_init);
#else
late_initcall(zacc_init);
#endif

MODULE_AUTHOR("Alex-KC Yuan <alexkc.yuan@realtekc.com>");
MODULE_DESCRIPTION("Realtek ZACC HW compressor support");
MODULE_LICENSE("GPL");
MODULE_SOFTDEP("pre: zsmalloc zram");

#endif

