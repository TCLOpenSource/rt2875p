//Copyright (C) 2007-2021 Realtek Semiconductor Corporation.

#define pr_fmt(fmt) "[arm_wrapper]:  " fmt

#include <linux/interrupt.h>
#include <linux/of_irq.h>
#include <asm/io.h>
#include <rtk_kdriver/io.h>
#include <rtd_log/rtd_module_log.h>
//#include <linux/nmi.h>

#include <asm/sections.h>
#include <mach/rtk_platform.h>
#include <linux/platform_device.h>
#include <linux/module.h>
#include "rtd_logger.h"
#include "arm_wrapper.h"
#include <rbus/scpu_wrapper_reg.h>

#include <linux/version.h>

#ifdef CONFIG_REALTEK_MEMORY_MANAGEMENT_MODULE
#include <rtk_kdriver/rmm/rtkcacheflush.h>
#else
//#include <asm/cacheflush.h>
void flush_dcache_area (const void *virt_start, const void *virt_end);
void inval_dcache_area (unsigned long start, unsigned long size);

#undef dmac_flush_range
#undef dmac_inv_range
#define dmac_flush_range flush_dcache_area
#define dmac_inv_range inval_dcache_area

#endif

int arm_wrapper_set_id=1;
int arm_wrapper_kernel_trap_warning = 1;  // change default to 1
extern int need_monitor_range_dft;
extern unsigned int avk_range[6];//avk auto sync range for hw-monitor
static int arm_wrapper_skip_init[WRAPPER_NUM] = {0};


#if defined(CONFIG_ARCH_RTK6748)
#define SCPU_MEM_TRASH_DBG_START_0 SCPU_WRAPPER_mem_trash_dbg_start0_reg
#define SCPU_MEM_TRASH_DBG_END_0 SCPU_WRAPPER_mem_trash_dbg_end0_reg
#define SCPU_MEM_TRASH_DBG_CTRL_0 SCPU_WRAPPER_mem_trash_dbg_ctrl0_reg
#elif defined(CONFIG_ARCH_RTK6702)
#define SCPU_MEM_TRASH_DBG_START_0 SCPU_WRAPPER_mem_trash_dbg_start_0_reg
#define SCPU_MEM_TRASH_DBG_END_0 SCPU_WRAPPER_mem_trash_dbg_end_0_reg
#define SCPU_MEM_TRASH_DBG_CTRL_0 SCPU_WRAPPER_mem_trash_dbg_ctrl_0_reg
#elif defined(CONFIG_ARCH_RTK2851A)
#define SCPU_MEM_TRASH_DBG_START_0 SCPU_WRAPPER_mem_trash_dbg_start_0_reg
#define SCPU_MEM_TRASH_DBG_END_0 SCPU_WRAPPER_mem_trash_dbg_end_0_reg
#define SCPU_MEM_TRASH_DBG_CTRL_0 SCPU_WRAPPER_mem_trash_dbg_ctrl_0_reg
#elif defined(CONFIG_ARCH_RTK2851C) || defined(CONFIG_ARCH_RTK2851F)
#define SCPU_MEM_TRASH_DBG_START_0 SCPU_WRAPPER_mem_trash_dbg_start0_reg
#define SCPU_MEM_TRASH_DBG_END_0 SCPU_WRAPPER_mem_trash_dbg_end0_reg
#define SCPU_MEM_TRASH_DBG_CTRL_0 SCPU_WRAPPER_mem_trash_dbg_ctrl0_reg
#elif defined(CONFIG_ARCH_RTK2885P)
#define SCPU_MEM_TRASH_DBG_START_0 SCPU_WRAPPER_mem_trash_dbg_start0_reg
#define SCPU_MEM_TRASH_DBG_END_0 SCPU_WRAPPER_mem_trash_dbg_end0_reg
#define SCPU_MEM_TRASH_DBG_CTRL_0 SCPU_WRAPPER_mem_trash_dbg_ctrl0_reg
#endif

#if 0
void dump_stacks (void);
#else
#define dump_stacks rtk_dump_stacks
void rtk_dump_stacks(void);
#endif
int clear_arm_wrapper_arange (int set_id)
{
    int id=set_id-1;
    unsigned int start_reg=SCPU_MEM_TRASH_DBG_START_0+id*4;
    unsigned int end_reg =SCPU_MEM_TRASH_DBG_END_0+id*4;
    unsigned int ctrl_reg =SCPU_MEM_TRASH_DBG_CTRL_0+id*4;
    int arm_wrapper_disable=BIT(1);

    if (set_id<1||id>WRAPPER_NUM)
    {
        ARMWRAP_ERR("Set fail! set Id[%d] not in (1-4)\n", set_id);
        return -1;
    }

    //rtd_pr_hw_monitor_debug("arm_wrapper_enable=%x\n",arm_wrapper_enable);
    rtd_outl(ctrl_reg,arm_wrapper_disable);
    rtd_outl(start_reg, 0);
    rtd_outl(end_reg, 0);

    return 0;
}

int scpu_access_addr (const char * buf)
{
    int set_id;
    unsigned long start;
    unsigned long access_addr;
    char access_type[10];

    memset(access_type, 0, sizeof(access_type));
    if(sscanf(buf, "set%d %s %lx", &set_id, access_type, &start)<3)
    {
        rtd_pr_hw_monitor_err("arm wrapper %s failed, invalid argument - %s\n",access_type, buf);
        return -1;
    }

    access_addr = (unsigned long)phys_to_virt(start);
    rtd_pr_hw_monitor_err("[%s,%d,%s], access_type=%s, phys addr = 0x%016llx , virt =0x%016llx\n",__FILE__,__LINE__,__FUNCTION__,access_type, start,access_addr);

    dmac_flush_range((const void *)access_addr,(const void *)(access_addr+0x1000));
    //dmac_inv_range((const void *)access_addr,(const void *)(access_addr+0x1000));
    if((strncmp(access_type, "read",4)==0))
    {
        rtd_inl(access_addr);
    }
    else if(strncmp(access_type, "write",5)==0)
    {
        rtd_outl(access_addr,0);
        dmac_flush_range((const void *)access_addr,(const void *)(access_addr+0x1000));
        //dmac_flush_range((const void *)access_addr,(const void *)(access_addr+0x1000));
    }
    else
    {
        rtd_pr_hw_monitor_err("arm wrapper %s failed, invalid argument - %s\n",access_type, buf);
        return -1;
    }

    return 0;
}

void arm_wrapper_dump (void)
{
    int i=0;
    rtd_pr_hw_monitor_err("\n\n");

    for (i=0; i<WRAPPER_NUM; i++)
    {
        rtd_pr_hw_monitor_err("set%d: 0x%08x [0x%08x]  - 0x%08x [0x%08x], 0x%08x [0x%08x]\n",
               i+1,(SCPU_MEM_TRASH_DBG_START_0+4*i),rtd_inl(SCPU_MEM_TRASH_DBG_START_0+4*i),
               (SCPU_MEM_TRASH_DBG_END_0+4*i),rtd_inl(SCPU_MEM_TRASH_DBG_END_0+4*i),
               (SCPU_MEM_TRASH_DBG_CTRL_0+4*i),rtd_inl(SCPU_MEM_TRASH_DBG_CTRL_0+4*i));
    }

    rtd_pr_hw_monitor_err("dbg_int:   0x%08x [0x%08x]\n",SCPU_WRAPPER_mem_trash_dbg_int_reg,rtd_inl(SCPU_WRAPPER_mem_trash_dbg_int_reg));
    rtd_pr_hw_monitor_err("dbg_addr:  0x%08x [0x%08x]\n",SCPU_WRAPPER_mem_trash_dbg_addr_reg,rtd_inl(SCPU_WRAPPER_mem_trash_dbg_addr_reg));
    rtd_pr_hw_monitor_err("dbg_rw:  0x%08x [0x%08x]\n",SCPU_WRAPPER_mem_trash_dbg_rw_reg,rtd_inl(SCPU_WRAPPER_mem_trash_dbg_rw_reg));

#ifdef CONFIG_ARCH_RTK6702
    rtd_pr_hw_monitor_err("dbg_addr_bit_32:  0x%08x [0x%08x]\n",SCPU_WRAPPER_mem_trash_dbg_addr_bit_32_reg,rtd_inl(SCPU_WRAPPER_mem_trash_dbg_addr_bit_32_reg));
#endif

    rtd_pr_hw_monitor_err("\n\n");
}

int set_arm_wrapper_arange (unsigned long start,unsigned long end, int set_id,WRAPPER_RW rw)
{
    int id=set_id-1;
    unsigned int start_reg=SCPU_MEM_TRASH_DBG_START_0+id*4;
    unsigned int end_reg =SCPU_MEM_TRASH_DBG_END_0+id*4;
    unsigned int ctrl_reg =SCPU_MEM_TRASH_DBG_CTRL_0+id*4;

#ifdef CONFIG_ARCH_RTK6702
    unsigned int bit_32_val;
#endif

    int arm_wrapper_enable= BIT(1)|BIT(0)|BIT(4)|((int)rw<<2);
    int arm_wrapper_disable=BIT(1);
    //rtd_pr_hw_monitor_debug("arm_wrapper_enable=%x\n",arm_wrapper_enable);

    if (id<0||id>3)
    {
        rtd_pr_hw_monitor_err("Set fail! set Id[%d] not in (1-4)\n", set_id);
        return -1;
    }

    if(start > end)
    {
        rtd_pr_hw_monitor_err("Set fail! set[%d] start address(0x%lx) > end address(0x%lx) \n", set_id, start, end);
        return -1;
    }

    rtd_outl(ctrl_reg,arm_wrapper_disable);

    rtd_outl(start_reg, (unsigned int)start);
    rtd_outl(end_reg, (unsigned int)end);

#ifdef CONFIG_ARCH_RTK6702
    if((start>>32)&0x1)
    {
        bit_32_val =rtd_inl(SCPU_WRAPPER_mem_trash_dbg_addr_bit_32_reg);
        rtd_outl(SCPU_WRAPPER_mem_trash_dbg_addr_bit_32_reg, bit_32_val | (1<<id) );
        //rtd_pr_hw_monitor_err("%s %d  0x%016llx  %llx\n",__func__,__LINE__,start,(start>>32)&0x1);
    }

    if((end>>32)&0x1)
    {
        bit_32_val =rtd_inl(SCPU_WRAPPER_mem_trash_dbg_addr_bit_32_reg);
        rtd_outl(SCPU_WRAPPER_mem_trash_dbg_addr_bit_32_reg, bit_32_val | (1<<(id+4)) );
        //rtd_pr_hw_monitor_err("%s %d  0x%016llx  %llx\n",__func__,__LINE__,end,(end>>32)&0x1);
    }
#endif

    //arm_wrapper_dump();

#if 1
    rtd_outl(ctrl_reg,arm_wrapper_enable);
#else
    rtd_outl(ctrl_reg,arm_wrapper_enable|BIT(4)|BIT(2));
#endif
    arm_wrapper_set_id = (set_id%WRAPPER_NUM)+1;
    return 0;
}


//cat /sys/realtek_boards/arm_wrapper
void arm_wrapper_get (void)
{
    unsigned char old_loglevel=console_loglevel;
    console_loglevel=7;
    arm_wrapper_dump();
    console_loglevel=old_loglevel;
}

#if 0
static int is_buf_full(int ofst, int len)
{
    int n= PAGE_SIZE-ofst-len-1;

    if(n<0)
    {
        return -1;
    }
    return 0;
}
#endif
static int is_pagebuf_full (int ofst)
{
    //int n= PAGE_SIZE-ofst-len-1;
    int n= PAGE_SIZE-ofst -1;

    if(n<0)
    {
        return -1;
    }
    return 0;
}

static int show_msg (char *buf, int ofst, const char * fmt, ...)
{
    va_list args;
    int ret = 0;

    if(buf != NULL)
    {
        buf += ofst;
        if(is_pagebuf_full(ofst) != -1)
        {
            va_start(args, fmt);
            ret = vsnprintf(buf, PAGE_SIZE-ofst-1, fmt, args);
            va_end(args);
        }
    }
    else
    {
        va_start(args, fmt);
        //vprintf(fmt,args);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 8, 0))
        vprintk_emit(0, LOGLEVEL_ERR, NULL, fmt, args);
#else
        vprintk_emit(0, LOGLEVEL_ERR, NULL, 0, fmt, args);
#endif
        va_end(args);
    }

    return ret;
}

int arm_wrapper_get_info(char *buf)
{
    int i=0;
    int len=0;

    char* read_write[]= {"R/W  ","read","write","R/W  "};
    unsigned int ctrl;

    len += show_msg(buf,len,"\n");

    for (i=0; i<WRAPPER_NUM; i++)
    {
        ctrl=rtd_inl(SCPU_MEM_TRASH_DBG_CTRL_0+i*4);
        if (ctrl&BIT(0))
        {

#ifdef CONFIG_ARCH_RTK6702
            len += show_msg(buf,len,"set%d: 0x%01x%08x-0x%01x%08x, 0x%08x[ctrl]. \e[1;31m %s monitor enabled!\e[0m\n",
                            i+1,(rtd_inl(SCPU_WRAPPER_mem_trash_dbg_addr_bit_32_reg)>>i)&0x1,rtd_inl(SCPU_MEM_TRASH_DBG_START_0+4*i),
                            (rtd_inl(SCPU_WRAPPER_mem_trash_dbg_addr_bit_32_reg)>>(i+4))&0x1,rtd_inl(SCPU_MEM_TRASH_DBG_END_0+4*i),
                            rtd_inl(SCPU_MEM_TRASH_DBG_CTRL_0+4*i),read_write[ctrl>>2&0x3]);
#else
            len += show_msg(buf,len,"set%d: 0x%08x-0x%08x, 0x%08x[ctrl]. \e[1;31m %s monitor enabled!\e[0m\n",
                            i+1,rtd_inl(SCPU_MEM_TRASH_DBG_START_0+4*i),
                            rtd_inl(SCPU_MEM_TRASH_DBG_END_0+4*i),
                            rtd_inl(SCPU_MEM_TRASH_DBG_CTRL_0+4*i),read_write[ctrl>>2&0x3]);
#endif
        }
        else
        {

#ifdef CONFIG_ARCH_RTK6702
            len += show_msg(buf,len,"set%d: 0x%01x%08x-0x%01x%08x, 0x%08x[ctrl].\n",
                            i+1,(rtd_inl(SCPU_WRAPPER_mem_trash_dbg_addr_bit_32_reg)>>i)&0x1,rtd_inl(SCPU_MEM_TRASH_DBG_START_0+4*i),
                            (rtd_inl(SCPU_WRAPPER_mem_trash_dbg_addr_bit_32_reg)>>(i+4))&0x1,rtd_inl(SCPU_MEM_TRASH_DBG_END_0+4*i),
                            rtd_inl(SCPU_MEM_TRASH_DBG_CTRL_0+4*i));
#else
            len += show_msg(buf,len,"set%d: 0x%08x-0x%08x, 0x%08x[ctrl].\n",
                            i+1,rtd_inl(SCPU_MEM_TRASH_DBG_START_0+4*i),
                            rtd_inl(SCPU_MEM_TRASH_DBG_END_0+4*i),
                            rtd_inl(SCPU_MEM_TRASH_DBG_CTRL_0+4*i));
#endif
        }
    }

    len += show_msg(buf,len,"dbg_int:   0x%08x-0x%08x\n",SCPU_WRAPPER_mem_trash_dbg_int_reg,rtd_inl(SCPU_WRAPPER_mem_trash_dbg_int_reg));
    len += show_msg(buf,len, "dbg_addr:  0x%08x-0x%08x\n",SCPU_WRAPPER_mem_trash_dbg_addr_reg,rtd_inl(SCPU_WRAPPER_mem_trash_dbg_addr_reg));
    len += show_msg(buf,len, "dbg_rw:  0x%08x [0x%08x]\n",SCPU_WRAPPER_mem_trash_dbg_rw_reg,rtd_inl(SCPU_WRAPPER_mem_trash_dbg_rw_reg));

#ifdef CONFIG_ARCH_RTK6702
    len+=show_msg(buf,len,"dbg_addr_bit_32:  0x%08x [0x%08x]\n",SCPU_WRAPPER_mem_trash_dbg_addr_bit_32_reg,rtd_inl(SCPU_WRAPPER_mem_trash_dbg_addr_bit_32_reg));
#endif

    len += show_msg(buf,len,"\n\n");
    return len;
}

void arm_wrapper_show_last_set(void)
{
    char* read_write[]= {"R/W  ","read","write","R/W  "};
    unsigned int ctrl;
    int i;
    int last_set_id = ((arm_wrapper_set_id-1)?(arm_wrapper_set_id-1):4);

    for (i=0; i<WRAPPER_NUM; i++)
    {
        ctrl=rtd_inl(SCPU_MEM_TRASH_DBG_CTRL_0+i*4);
        if((i+1) == last_set_id)
        {
            if (ctrl&BIT(0))
            {
                rtd_pr_hw_monitor_err("\e[1;33mset%d: 0x%08x-0x%08x, 0x%08x[ctrl]. %s monitor enabled!\e[0m\n",
                            i+1,rtd_inl(SCPU_MEM_TRASH_DBG_START_0+4*i),rtd_inl(SCPU_MEM_TRASH_DBG_END_0+4*i),
                            rtd_inl(SCPU_MEM_TRASH_DBG_CTRL_0+4*i),read_write[ctrl>>2&0x3]);
            }
            else
            {
                rtd_pr_hw_monitor_err("\e[1;33mset%d: 0x%08x-0x%08x, 0x%08x[ctrl].\e[0m\n",
                            i+1,rtd_inl(SCPU_MEM_TRASH_DBG_START_0+4*i),rtd_inl(SCPU_MEM_TRASH_DBG_END_0+4*i),
                            rtd_inl(SCPU_MEM_TRASH_DBG_CTRL_0+4*i));
            }
        }
        else
        {
            if (ctrl&BIT(0))
            {
                rtd_pr_hw_monitor_err("set%d: 0x%08x-0x%08x, 0x%08x[ctrl]. \e[1;31m %s monitor enabled!\e[0m\n",
                            i+1,rtd_inl(SCPU_MEM_TRASH_DBG_START_0+4*i),rtd_inl(SCPU_MEM_TRASH_DBG_END_0+4*i),
                            rtd_inl(SCPU_MEM_TRASH_DBG_CTRL_0+4*i),read_write[ctrl>>2&0x3]);
            }
            else
            {
                rtd_pr_hw_monitor_err("set%d: 0x%08x-0x%08x, 0x%08x[ctrl].\n",
                            i+1,rtd_inl(SCPU_MEM_TRASH_DBG_START_0+4*i),rtd_inl(SCPU_MEM_TRASH_DBG_END_0+4*i),
                            rtd_inl(SCPU_MEM_TRASH_DBG_CTRL_0+4*i));
            }
        }
    }
    rtd_pr_hw_monitor_err("\n\n");
}

void arm_wrapper_show_last_set_only(void)
{
    char* read_write[]= {"R/W  ","read","write","R/W  "};
    unsigned int ctrl;
    int i;
    int last_set_id = ((arm_wrapper_set_id-1)?(arm_wrapper_set_id-1):4);

    for (i=0; i<WRAPPER_NUM; i++)
    {
        ctrl=rtd_inl(SCPU_MEM_TRASH_DBG_CTRL_0+i*4);
        if((i+1) == last_set_id)
        {
            if (ctrl&BIT(0))
            {
                rtd_pr_hw_monitor_err("\e[1;33mset%d: 0x%08x-0x%08x, 0x%08x[ctrl]. %s monitor enabled!\e[0m\n",
                            i+1,rtd_inl(SCPU_MEM_TRASH_DBG_START_0+4*i),rtd_inl(SCPU_MEM_TRASH_DBG_END_0+4*i),
                            rtd_inl(SCPU_MEM_TRASH_DBG_CTRL_0+4*i),read_write[ctrl>>2&0x3]);
            }
            else
            {
                rtd_pr_hw_monitor_err("\e[1;33mset%d: 0x%08x-0x%08x, 0x%08x[ctrl].\e[0m\n",
                            i+1,rtd_inl(SCPU_MEM_TRASH_DBG_START_0+4*i),rtd_inl(SCPU_MEM_TRASH_DBG_END_0+4*i),
                            rtd_inl(SCPU_MEM_TRASH_DBG_CTRL_0+4*i));
            }
        }
    }
}

//eg:  echo "set1 write 0x1a900000" > /sys/realtek_boards/arm_wrapper
void arm_wrapper_set (const char *buffer)
{
    unsigned long start = 0,end = 0,set_id = 0;
    char str_rw[4] = {0}; // match r/w/rw
    char str1[8] = {0}; // match first string like : set1/unset/unset1
    char str2[8] = {0}; // match second string like : clear/all/read/write
    WRAPPER_RW rw;
    int i=  0;
    
    str_rw[3]=0;
    console_loglevel=7;
	
    if (sscanf(buffer, "%7s %7s %3s", str1, str2, str_rw)<1)
    {
        ARMWRAP_ERR("arm wrapper failed, invalid argument - %s\n", buffer);
        goto Fail;
    }
    
    if (strlen(str1) == (strlen("unset") + 1))
    {
        set_id=str1[strlen("unset")]-'0';
        str1[strlen("unset")] = '\0';
    }
    if ((strcmp("unset",str1) == 0) && (strlen(str_rw) == 0))
    {
        if (strcmp(str2,"all")==0)
        {
            for (i = 0; i < WRAPPER_NUM; i++)
            {
                clear_arm_wrapper_arange(i+1);
            }
            return;
        }
        
        if (((set_id<1) || (set_id>WRAPPER_NUM)) || (strlen(str2) > 0))
        {
            rtd_pr_hw_monitor_err("arm wrapper unset failed, expected range[1-%d] \n", WRAPPER_NUM);
            goto Fail;
        }
        
        clear_arm_wrapper_arange(set_id);
        return;
    }
    
    if (strlen(str1) == (strlen("set") + 1))
    {
        set_id=str1[strlen("set")]-'0';
        str1[strlen("set")] = '\0';
    }
    if (strcmp("set",str1))
    {
        goto Fail;
    }

    if ((set_id<1) || (set_id>WRAPPER_NUM))
    {
        rtd_pr_hw_monitor_err("arm wrapper set failed, expected range[1-%d] \n", WRAPPER_NUM);
        goto Fail;
    }

    if ((strcmp(str2,"clear")==0) && (strlen(str_rw) == 0))
    {
        clear_arm_wrapper_arange(set_id);
        return;
    }
    
    if ((strcmp(str2,"read")==0) || (strcmp(str2,"write")==0))
    {
        scpu_access_addr(buffer);
        return;
    }

    if (sscanf(buffer, "set%d %llx-%llx %2s", &set_id,&start,&end,str_rw)<1)
    {
        ARMWRAP_ERR("arm wrapper set failed, invalid argument - %s\n", buffer);
        goto Fail;
    }
    else
    {
        if(start > end)
        {
            rtd_pr_hw_monitor_err("\n arm wrapper set failed, MT start range:0x%08x > end range:0x%08x\n", start,end);
            goto Fail;
        }
        if (strcmp("rw",str_rw)==0)
        {
            rw=WRAPPER_READ_WRITE;
        }
        else if (strcmp("r",str_rw)==0)
        {
            rw=WRAPPER_READ;
        }
        else if (strcmp("w",str_rw)==0)
        {
            rw=WRAPPER_WRTIE;
        }
        else
        {
            goto Fail;
        }
    }

    if(set_arm_wrapper_arange(start,end,set_id,rw)==0)
    {
        return;
    }

Fail:
    rtd_pr_hw_monitor_err("%s fail.  %s\n\n\n",__func__,buffer);//0x00cd2d00[start], 0xb805c030-0x00cd2d40

    rtd_pr_hw_monitor_err("\e[1;32marm wrapper related commands as follow\e[0m: (set no must between 1 and 4)\n");
    rtd_pr_hw_monitor_err("cat /sys/realtek_boards/arm_wrapper\n");
    rtd_pr_hw_monitor_err("echo \"set2 0x00cd2d00-0x00cd2d40 rw\" > /sys/realtek_boards/arm_wrapper\n");
    rtd_pr_hw_monitor_err("echo \"set2 clear\" > /sys/realtek_boards/arm_wrapper\n\n\n");
    return ;
}

int get_arm_wraper_set_id(void)
{
    return arm_wrapper_set_id;
}

#define ROUND_UP(x, n) (((x) + (n) - 1u) & ~((n) - 1u))

#if IS_ENABLED(CONFIG_REALTEK_ARM_WRAPPER_MONITOR_TEXT)
#ifdef CONFIG_ARM64
void arm_wrapper_dft_set_linux_text (int on)
{
    unsigned long addr_start; 
    unsigned long addr_end; 

    if(arm_wrapper_skip_init[WRAPPER_DFT_ENTRY_SCPU_TEXT-1])
    {
        rtd_pr_hw_monitor_err("[%s] skip this setting...\n",__FUNCTION__);
        return;
    }
    
#ifdef MODULE
    addr_start = (unsigned long)virt_to_phys((unsigned long *)get_kernel_default_monitor_text_start_addr());
    addr_end = (unsigned long)virt_to_phys((unsigned long *)get_kernel_default_monitor_text_end_addr());
#else
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
    addr_start = __pa_symbol(_stext);
#else
    addr_start = __pa_symbol(_text);
#endif
    addr_end = __pa_symbol(__start_rodata);
#endif

    if (on)
    {
#if 1//def CONFIG_DYNAMIC_FTRACE
        /* promise arm_wrapper of this range is already disabled before we flush cache of text section */
        clear_arm_wrapper_arange(WRAPPER_DFT_ENTRY_SCPU_TEXT);
#ifdef MODULE
        dmac_flush_range((const void *)get_kernel_default_monitor_text_start_addr(),(const void *)get_kernel_default_monitor_text_end_addr());
#else
        dmac_flush_range(_stext,__start_rodata);
#endif
#endif

        set_arm_wrapper_arange(addr_start, addr_end-1, WRAPPER_DFT_ENTRY_SCPU_TEXT, WRAPPER_WRTIE);
    }
    else
    {
        clear_arm_wrapper_arange(WRAPPER_DFT_ENTRY_SCPU_TEXT);
    }
}

#else //CONFIG_ARM64
extern char __v7_setup_stack[];
void arm_wrapper_dft_set_linux_text (int on)
{
    unsigned long addr_start = virt_to_phys(_text);               // linux text start address
    unsigned long addr_end =   virt_to_phys(__start_rodata);      // linux text end address
    //unsigned long addr_end =   virt_to_phys(__end_rodata);      // cannot use __end_rodata

    if(arm_wrapper_skip_init[WRAPPER_DFT_ENTRY_SCPU_TEXT-1])
    {
        rtd_pr_hw_monitor_err("[%s] skip this setting...\n",__FUNCTION__);
        return;
    }
    
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
    addr_start = virt_to_phys(_stext);
    set_arm_wrapper_arange(addr_start, addr_end-1, WRAPPER_DFT_ENTRY_SCPU_TEXT, WRAPPER_WRTIE);
#else
    addr_end = (((unsigned long)__v7_setup_stack & 0xffffff00)-1);
    addr_end = virt_to_phys((void*)addr_end);
    set_arm_wrapper_arange(addr_start, addr_end, WRAPPER_DFT_ENTRY_SCPU_TEXT, WRAPPER_WRTIE);

    addr_start = ROUND_UP((unsigned long)__v7_setup_stack,0x100)+0x100;
    addr_start = virt_to_phys((void*)addr_start);
    addr_end   = virt_to_phys(__start_rodata);
    set_arm_wrapper_arange(addr_start, addr_end-1, WRAPPER_DFT_ENTRY_02, WRAPPER_WRTIE);
#endif

}
#endif //CONFIG_ARM64
#endif //CONFIG_REALTEK_ARM_WRAPPER_MONITOR_TEXT

static void arm_wrapper_dft_set_avcpu (void)
{
    unsigned long addr, size;
    size = carvedout_buf_query(CARVEDOUT_V_OS,(void *)( &addr));
    if (addr && size )
    {
        if (arm_wrapper_skip_init[WRAPPER_DFT_ENTRY_VCPU_TEXT-1] == 0)
        {
            set_arm_wrapper_arange(addr, addr + size - 1, WRAPPER_DFT_ENTRY_VCPU_TEXT, WRAPPER_WRTIE);//protect v area
        }
    }

#if !defined(CONFIG_ARCH_RTK2851C) && !defined(CONFIG_ARCH_RTK2851F) && !defined(CONFIG_ARCH_RTK6702) && !defined(CONFIG_ARCH_RTK6748)
    //set acpu text monitor
    size = carvedout_buf_query(CARVEDOUT_A_OS,(void *)( &addr));
    //if (avk_range[RANGE_SYNC_A_START] && avk_range[RANGE_SYNC_A_END])
    if (addr && size )
    {
        if (arm_wrapper_skip_init[WRAPPER_DFT_ENTRY_ACPU_TEXT-1] == 0)
        {
            set_arm_wrapper_arange(addr, addr + size - 1, WRAPPER_DFT_ENTRY_ACPU_TEXT, WRAPPER_WRTIE);//protect v area
            //set_arm_wrapper_arange(avk_range[RANGE_SYNC_A_START], avk_range[RANGE_SYNC_A_END] - 1, WRAPPER_DFT_ENTRY_ACPU_TEXT, WRAPPER_WRTIE);//protect a area
        }
    }
#endif
}

static void arm_wrapper_dft_set_overrange (void)
{
    unsigned long total_ddr_size = get_memory_size(GFP_DCU1) + get_memory_size(GFP_DCU2);
    if (total_ddr_size >= 0x100000000LL)
    {
        return;
    }

#ifdef CONFIG_ARCH_RTK2885P
#define GPU_RBUS_RANGE_START 0xff7f0000
#define GPU_RBUS_RANGE_END   (0xff7f0000+16*1024)
#define PCIE_RANGE_START 0xC0000000
    if (total_ddr_size < PCIE_RANGE_START)
    {
        set_arm_wrapper_arange(total_ddr_size, PCIE_RANGE_START-1, WRAPPER_DFT_ENTRY_OVERRANGE, WRAPPER_WRTIE);
    }
    //set_arm_wrapper_arange(GPU_RBUS_RANGE_END, 0xffffffff, WRAPPER_DFT_ENTRY_OVERRANGE, WRAPPER_WRTIE);
#else
    set_arm_wrapper_arange(total_ddr_size, 0xffffffff, WRAPPER_DFT_ENTRY_OVERRANGE, WRAPPER_WRTIE);
#endif
}

static void arm_wrapper_dft_set (void)
{
    //flush_cache_all();
#if IS_ENABLED(CONFIG_REALTEK_ARM_WRAPPER_MONITOR_TEXT)
    arm_wrapper_dft_set_linux_text(1);
#endif
    arm_wrapper_dft_set_avcpu();
    arm_wrapper_dft_set_overrange();
}

extern void show_regs(struct pt_regs *);
void arm_wrapper_dump_trash_ddr (unsigned long trap_address)
{
    unsigned int * addr = (unsigned int * )phys_to_virt (trap_address);
    int i=0;

    for(i=0;i<8;i++)
    {
        rtd_pr_hw_monitor_err("%016llx:    %08x %08x %08x %08x \n",addr,addr[0],addr[1],addr[2],addr[3]);
        addr=addr+4;
    }
}


#include <linux/delay.h>
int arm_wrapper_check (int call_flag)
{
    unsigned int value=rtd_inl(SCPU_WRAPPER_mem_trash_dbg_int_reg);
    unsigned int read_write = 0;
    unsigned long trap_address=0,trap_address_high=0;
    static volatile int flag_slverr;
    //unsigned char old_loglevel=console_loglevel;
    if(!SCPU_WRAPPER_mem_trash_dbg_int_get_scpu_int_m0(value))
    {
        return 0;
    }

    if(0==call_flag)
    {
        flag_slverr=1;
    }

    console_loglevel=7;
    read_write=SCPU_WRAPPER_mem_trash_dbg_rw_get_scpu_dbg_write_m0(rtd_inl(SCPU_WRAPPER_mem_trash_dbg_rw_reg));
    trap_address_high=(rtd_inl(SCPU_WRAPPER_mem_trash_dbg_rw_reg)&0x2)>>1;
    trap_address=(trap_address_high<<32)|rtd_inl(SCPU_WRAPPER_mem_trash_dbg_addr_reg);
    rtd_pr_hw_monitor_err("\n\n\n\n");
    rtd_pr_hw_monitor_err("[Memory trash] SCPU has trashed itself (%s) : \e[1;31m%s 0x%016llx\e[0m invalid.\n\n",
           call_flag?"interrupt":"exception",read_write?"write":"read",trap_address);

    if(call_flag)
    {
        mdelay(2000);
        // only dump one time log
        if(flag_slverr)
        {
            return 0;
        }
    }

    //arm_wrapper_dump_trash_ddr(trap_address);
    arm_wrapper_dump();
    dump_stacks();

    //disable arm-wrapper function!
    rtd_outl(SCPU_WRAPPER_mem_trash_dbg_int_reg,SCPU_WRAPPER_mem_trash_dbg_int_scpu_int_m0_mask|
                    SCPU_WRAPPER_mem_trash_dbg_int_scpu_spi_en_mask|SCPU_WRAPPER_mem_trash_dbg_int_scpu_int_en_m0_mask);
    rtd_save_keylog(KERN_ERR,WRAP_TAG,"[Memory trash] SCPU has trashed itself (%s) : %s 0x%016llx invalid.\n\n",
                    call_flag?"interrupt":"exception",read_write?"write":"read",trap_address);
    //console_loglevel = old_loglevel;
    return 1;
}

int arm_wrapper_slverr (unsigned long addr, unsigned int fsr, struct pt_regs *regs)
{
    if(!arm_wrapper_check(0))
    {
        return 0;
    }
    if(regs)
    {
#ifndef MODULE
        show_regs(regs);
#endif
    }
    return 0;
}

irqreturn_t arm_wrapper_intr (int irq, void *dev_id)
{
    if(!arm_wrapper_check(1))
    {
        return IRQ_NONE;
    }

    if(1 == arm_wrapper_kernel_trap_warning)
    {
        panic("SCPU has trashed itself ... ");
    }
    return IRQ_HANDLED;
}


static int rtk_arm_warpper_irq_num = -1;
#define IRQ_ARM_WRAPPER IRQ_DCSYS
//c0108000 T _text IRQ_DCSYS IRQ_SECURE_SB2

void arm_wrapper_intr_setup (void)
{
    struct irq_data *data=NULL;
    if (rtk_arm_warpper_irq_num <= 0)
    {
        rtd_pr_hw_monitor_err("%s error. invalid irq: %d\n", __func__, rtk_arm_warpper_irq_num);
        return;
    }

    /* Request IRQ */
    if(request_irq(rtk_arm_warpper_irq_num,
                   arm_wrapper_intr,
                   IRQF_SHARED,
                   "ARM_WRAPPER",
                   arm_wrapper_intr))
    {
        ARMWRAP_ERR("%s error. \n", __func__);
        data=irq_get_irq_data(rtk_arm_warpper_irq_num); /*coverity CID 486157, Logically dead code*/
        if(data)
        {
            rtd_pr_hw_monitor_err("cannot register hwirq %lu\n",irqd_to_hwirq(data));
        }
        return ;
    }

    //enable arm wrapper interrupt
    rtd_outl(SCPU_WRAPPER_mem_trash_dbg_int_reg,
    SCPU_WRAPPER_mem_trash_dbg_int_scpu_spi_en_mask|SCPU_WRAPPER_mem_trash_dbg_int_scpu_int_en_m0_mask|SCPU_WRAPPER_mem_trash_dbg_int_write_data_mask);
    rtd_pr_hw_monitor_err("@%s:    enable arm wrapper interrupt [reserved dbus address for SCPU]\n",__func__);
}

volatile int arm_wrapper_flag;
int get_arm_wrapper_flag (void)
{
    return arm_wrapper_flag;
}

void set_arm_wrapper_flag (void)
{
    arm_wrapper_flag=1;
}

static unsigned int wrapper_start[WRAPPER_NUM];
static unsigned int wrapper_end[WRAPPER_NUM];
static unsigned int wrapper_ctrl[WRAPPER_NUM];
void arm_wrapper_suspend (void)
{
    int i;
    for(i=0; i<WRAPPER_NUM; i++)
    {
        wrapper_start[i]=rtd_inl(SCPU_MEM_TRASH_DBG_START_0+4*i);
        wrapper_end[i]=rtd_inl(SCPU_MEM_TRASH_DBG_END_0+4*i);
        wrapper_ctrl[i]=rtd_inl(SCPU_MEM_TRASH_DBG_CTRL_0+4*i);
    }
}
EXPORT_SYMBOL(arm_wrapper_suspend);

void arm_wrapper_resume (void)
{
    int i;
    for(i=0; i<WRAPPER_NUM; i++)
    {
        if (wrapper_ctrl[i]&BIT(0))
        {
            rtd_outl(SCPU_MEM_TRASH_DBG_START_0+4*i,wrapper_start[i]);
            rtd_outl(SCPU_MEM_TRASH_DBG_END_0+4*i,wrapper_end[i]);
            rtd_outl(SCPU_MEM_TRASH_DBG_CTRL_0+4*i,wrapper_ctrl[i]|BIT(1)|BIT(0)|BIT(4));
        }
    }
    //enable arm wrapper interrupt
    rtd_outl(SCPU_WRAPPER_mem_trash_dbg_int_reg,
    SCPU_WRAPPER_mem_trash_dbg_int_scpu_spi_en_mask|SCPU_WRAPPER_mem_trash_dbg_int_scpu_int_en_m0_mask|SCPU_WRAPPER_mem_trash_dbg_int_write_data_mask);
}
EXPORT_SYMBOL(arm_wrapper_resume);

static int rtk_arm_wrapper_probe (struct platform_device *pdev)
{
    int ret = 0;
    //struct device_node *np=of_find_node_by_name(NULL, OF_ARM_WRAPPER_STR);
    struct device_node *np = pdev->dev.of_node;
    if (!np)
    {
        rtd_pr_hw_monitor_err("%s %d error \n",__func__,__LINE__);
        return -1;
    }

    rtk_arm_warpper_irq_num = irq_of_parse_and_map(np, 0);
    if(!rtk_arm_warpper_irq_num)
    {
        rtd_pr_hw_monitor_err("%s there is no arm_wrapper irq in dts\n",__func__);
        of_node_put(np);
        return -2;
    }

    //arm_wrapper_dft_set();
    arm_wrapper_intr_setup();
    return ret;
}

static int rtk_arm_wrapper_probe_remove(struct platform_device *pdev)
{
    return 0;
}

#define OF_ARM_WRAPPER_STR "arm_wrapper"
static const struct of_device_id rtk_arm_wrapper_of_match[] =
{
    {
        .compatible = "realtek,arm_wrapper",
    },
    {},
};

static struct platform_driver rtk_platform_arm_wrapper_driver =
{
    .probe = rtk_arm_wrapper_probe,
    .driver = {
        .name = OF_ARM_WRAPPER_STR,
        .of_match_table = rtk_arm_wrapper_of_match,
    },
    .remove = rtk_arm_wrapper_probe_remove,
};
MODULE_DEVICE_TABLE(of, rtk_arm_wrapper_of_match);
#ifndef MODULE
module_platform_driver(rtk_platform_arm_wrapper_driver);
#else
extern struct kobject *dc_mt_kobj;

static ssize_t arm_wrapper_show (struct kobject *kobj, struct kobj_attribute *attr, char *page)
{
    return arm_wrapper_get_info(page);
}

#define ARM_WRAPPER_TEST_SIZE 1024*1024
void arm_wrapper_set_dbg (const char *page)
{
    char buffer[128];

    static unsigned int addr_s,addr_e;
    static volatile int* arm_wrapper=NULL;
    int arm_wrapper_ = 0;
    if (!arm_wrapper)
    {
        arm_wrapper = kmalloc(ARM_WRAPPER_TEST_SIZE,GFP_KERNEL);
        if (!arm_wrapper)
        {
            rtd_pr_hw_monitor_err("%s %d  memory allocate fail\n",__func__,__LINE__);
            return ;
        }

        addr_s=virt_to_phys(arm_wrapper);
        addr_e =virt_to_phys(arm_wrapper)+ARM_WRAPPER_TEST_SIZE-1;
    }

    memset(buffer, 0, sizeof(buffer));
    if (strncmp(page,"dbg_show",strlen("dbg_show"))==0)
    {
        rtd_pr_hw_monitor_err("dbg start addr=%x, dbg end addr=%x\n",addr_s,addr_e);

        rtd_pr_hw_monitor_err("You can set ARM wrapper cmd like below:\n");

        snprintf(buffer,sizeof(buffer),"echo \"set%d %x-%x %s\">/sys/realtek_boards/arm_wrapper",2,addr_s,addr_e,"r");
        rtd_pr_hw_monitor_err("%s\n",buffer);
        rtd_pr_hw_monitor_err("echo dbg_read >/sys/realtek_boards/arm_wrapper\n\n");

        snprintf(buffer,sizeof(buffer),"echo \"set%d %x-%x %s\">/sys/realtek_boards/arm_wrapper",2,addr_s,addr_e,"w");
        rtd_pr_hw_monitor_err("%s\n",buffer);
        rtd_pr_hw_monitor_err("echo dbg_write >/sys/realtek_boards/arm_wrapper\n\n");

        rtd_pr_hw_monitor_err("echo dbg_flush_cache >/sys/realtek_boards/arm_wrapper   (flush cache by copy lots of data)\n\n");
    }
    else if (strncmp(page,"dbg_read",strlen("dbg_read"))==0)
    {
        rtd_pr_hw_monitor_err("\n\n\nSCPU triggerr arm wrapper interrupt: invalid read access debug address          \e[1;31m0x%08x  \e[0m\n\n\n",&(arm_wrapper[2]));
        arm_wrapper_=arm_wrapper[2];

        rtd_pr_hw_monitor_err("\n\n\n%s %d. SCPU dbg read finish\n",__func__,__LINE__);
    }
    else if (strncmp(page,"dbg_write",strlen("dbg_write"))==0)
    {
        rtd_pr_hw_monitor_err("\n\n\nSCPU triggerr arm wrapper interrupt: invalid write access debug address          \e[1;31m0x%08x\e[0m\n\n\n",&(arm_wrapper[2]));
        arm_wrapper[2]=38;

        rtd_pr_hw_monitor_err("\n\n\n%s %d. SCPU dbg write finish\n",__func__,__LINE__);
    }
    else if (strncmp(page,"dbg_dump",strlen("dbg_dump"))==0)
    {
        arm_wrapper_dump();
    }
    else if (strncmp(page,"dbg_flush_cache",strlen("dbg_flush_cache"))==0)
    {
        int i;
        rtd_pr_hw_monitor_err("%s %d  drop cache \n",__func__,__LINE__);
        for (i=0; i<ARM_WRAPPER_TEST_SIZE; i++)
        {
            arm_wrapper[i]=i*i;
        }
    }
}

static ssize_t arm_wrapper_store (struct kobject *kobj, struct kobj_attribute *attr, const char *page, size_t count)
{
    unsigned char old_loglevel=console_loglevel;
    console_loglevel=7;
    if (strncmp(page,"dbg_",strlen("dbg_"))==0)
    {
        arm_wrapper_set_dbg(page);
    }
    else
    {
        arm_wrapper_set(page);
    }
    console_loglevel=old_loglevel;
    return count;
}

#define ATTR_PERMISSION_RO      (S_IRUSR | S_IRGRP| S_IROTH)
#define ATTR_PERMISSION_RW      (S_IRUSR | S_IRGRP| S_IROTH | S_IWUSR | S_IWGRP)

static struct kobj_attribute arm_wrapper_attribute =
    __ATTR(arm_wrapper, ATTR_PERMISSION_RW, arm_wrapper_show, arm_wrapper_store);
static struct attribute *attrs[] =
{
    &arm_wrapper_attribute.attr,
    NULL,   /* need to NULL terminate the list of attributes */
};

static struct attribute_group attr_group =
{
    .attrs = attrs,
};


int rtk_armwrapper_init(void)
{
    int retval,result;

#ifdef MODULE
    result = platform_driver_register(&rtk_platform_arm_wrapper_driver);
    if (result) 
    {   
        rtd_pr_hw_monitor_err("%s: can not register platform driver, ret=%d\n", __func__,result);
        return -1;
    }
#endif

    if (dc_mt_kobj) 
    {   
        retval = sysfs_create_group(dc_mt_kobj, &attr_group);
        if(retval)
        {
            rtd_pr_hw_monitor_err("%s: can not create /sys/mt/arm_wrapper, ret=%d\n", __func__,retval);
            return -2;
        }
    }

    return result;
}
EXPORT_SYMBOL(rtk_armwrapper_init);
void rtk_armwrapper_exit(void)
{
    platform_driver_unregister(&rtk_platform_arm_wrapper_driver);
}
EXPORT_SYMBOL(rtk_armwrapper_exit);
#endif

void arm_wrapper_monitor_init(void)
{
    arm_wrapper_dft_set();
}

#if 1
#define ARM_WRAPPER_ENV_UNSET             "arm_wrapper_unset="

extern int get_arguments(char* str, int argc, char** argv);

//hwmonitor arm_wrapper unset "4"
#ifndef MODULE
static int __init bootparam_arm_wrapper_unset (char *str)
#else
static int bootparam_arm_wrapper_unset (char *str)
#endif
{
    char* argv[WRAPPER_NUM];
    int argc;
    int i,j;
    int entry;

    if(str == NULL)
    {
        return -1;
    }
    
    if (strcmp(str, "all")==0)
    {
        for(j = 0; j < WRAPPER_NUM; ++j)
        {
            arm_wrapper_skip_init[j] = 1;
        }
        return 0;//all is unset,so don't check other param
    }

    argc = get_arguments((char*)str, WRAPPER_NUM, argv);

    for(i = 0; i < argc; ++i)
    {
        if(sscanf(argv[i], "%d", &entry) != 1)
        {
            entry = -1;
        }

        if((entry >= 0)&&(entry < WRAPPER_NUM))
        {
            arm_wrapper_skip_init[entry] = 1;
        }
        else if(entry == WRAPPER_NUM)
        {
            for(j = 0; j < WRAPPER_NUM; ++j)
            {
                arm_wrapper_skip_init[j] = 1;
            }
            break;//all is unset,so don't check other param
        }
    }

    return 0;
}

#ifndef MODULE
__setup(ARM_WRAPPER_ENV_UNSET, bootparam_arm_wrapper_unset);
#else
extern bool rtk_parse_commandline_equal(const char *string, char *output_string, int string_size);
void get_bootparam_arm_wrapper_unset (void)
{
    char para_buff[100];
    if(rtk_parse_commandline_equal("arm_wrapper_unset", para_buff,sizeof(para_buff)) == 0)
    {    
        rtd_pr_hw_monitor_err("Error : can't get hwm_range reclaim from bootargs\n");
        return;
    }    
    bootparam_arm_wrapper_unset(para_buff);
    return;
}
EXPORT_SYMBOL(get_bootparam_arm_wrapper_unset);
#endif

#endif


