#ifndef _RMM_VENDOR_HOOK_H
#define _RMM_VENDOR_HOOK_H

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 15, 0)) && defined(CONFIG_REALTEK_MEMORY_MANAGEMENT_MODULE)
int rmm_vendor_hook_init(void);
void rmm_vendor_hook_exit(void);
#else
int rmm_vendor_hook_init(void) { return 0; }
void rmm_vendor_hook_exit(void) { }
#endif

#endif /* _RMM_VENDOR_HOOK_H */