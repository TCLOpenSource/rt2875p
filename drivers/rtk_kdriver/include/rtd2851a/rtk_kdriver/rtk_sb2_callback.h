#ifndef __SB2_CALLBACK_H__
#define __SB2_CALLBACK_H__

#define MODULE_NAME_LEN_MAX     32

typedef struct sb2_timeout_info {
        unsigned long  timeout_reg;
        int  isTimeout ;
} sb2_timeout_info;

typedef struct {
    unsigned long addr_start;   //register start addr
    unsigned long addr_end;     //register end addr
    const char      name[MODULE_NAME_LEN_MAX];  //moudle name
} t_sb2_timout_moudle;

#define _ANY_   (0x8240add2)  //a address of magic number to ignore sb2 error, for debugging purpose
#define _UNSET_ (~(_ANY_))

#ifdef CONFIG_RTK_KDRV_SB2
#include <linux/notifier.h>
#include <rtd_log/rtd_module_log.h>
extern struct atomic_notifier_head sb2_notifier_list;
extern struct atomic_notifier_head sb2_notifier_list_cat;       //show register moudles

int isSb2Timeout(char * module_name);
int get_sb2_timeout_info(sb2_timeout_info *info);

#define SB2_DEBUG_INFO_DECLARE(name,func)  \
        static int SB2_notifier_##name (struct notifier_block *this, unsigned long ev, void *ptr)  \
        {  \
                func();    \
                return NOTIFY_OK;     \
        } \
        static struct notifier_block SB2_##name = {    \
            .notifier_call = SB2_notifier_##name ,    \
        };\
        static int SB2_notifier_cat_##name (struct notifier_block *this, unsigned long ev, void *ptr)  \
        {  \
                rtd_pr_hw_monitor_err(  "\t\t%s\n",#name);    \
                return NOTIFY_OK;     \
        } \
        static struct notifier_block SB2_cat_##name = {    \
            .notifier_call = SB2_notifier_cat_##name ,    \
        }

#define SB2_DEBUG_INFO_REGISTER(name,func)  \
        do{     \
                atomic_notifier_chain_register(&sb2_notifier_list, &SB2_##name );\
                atomic_notifier_chain_register(&sb2_notifier_list_cat, &SB2_cat_##name );\
        }while(0)

#define SB2_DEBUG_INFO_UNREGISTER(name,func)  \
        do{     \
                atomic_notifier_chain_unregister(&sb2_notifier_list, &SB2_##name );\
                atomic_notifier_chain_unregister(&sb2_notifier_list_cat, &SB2_cat_##name );\
        }while(0)

extern unsigned long buserr_ignored_addr;
#define BUSERR_IGNORE_SET(addr) ({ buserr_ignored_addr = addr; })
#define BUSERR_IGNORE_UNSET(addr) ({ buserr_ignored_addr = _UNSET_; })

#else

#define SB2_DEBUG_INFO_DECLARE(name,func)
#define SB2_DEBUG_INFO_REGISTER(name,func)
#define SB2_DEBUG_INFO_UNREGISTER(name,func)

static inline int isSb2Timeout(char * module_name)
{
        return 0;
}
static inline int get_sb2_timeout_info(sb2_timeout_info *info)
{
        return 0;
}

#define BUSERR_IGNORE_SET(addr)
#define BUSERR_IGNORE_UNSET(addr)

#endif

#endif  //__SB2_CALLBACK_H__
