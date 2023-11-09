#ifndef _LINUX_RTK_AVCPU_H
#define _LINUX_RTK_AVCPU_H

#define AVCPU_SUSPEND           100
#define AVCPU_RESUME            101
#define AVCPU_RESET_PREPARE     102
#define AVCPU_RESET_DONE        103

#define ACPU_STATUS_FLAG_ADDRESS	0x000000c8
#ifdef CONFIG_RTK_KDRV_SUPPORT_ACPU2
  #define ACPU2_STATUS_FLAG_ADDRESS       0x000001fc
#endif
#define VCPU_STATUS_FLAG_ADDRESS	0x00000814
#ifdef CONFIG_RTK_KDRV_SUPPORT_VCPU2
  #define VCPU2_STATUS_FLAG_ADDRESS	0x00000818
#endif
#define AVCPU_STATUS_FLAG_STD_ADDR	0x0000081C

#endif
