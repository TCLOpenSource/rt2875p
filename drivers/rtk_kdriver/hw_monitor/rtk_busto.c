#include <linux/uaccess.h>
#include <linux/uaccess.h>

#ifndef CONFIG_ARM64
#include <asm/system_info.h>
#endif
#include <rtd_log/rtd_module_log.h>
#include <linux/version.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 11, 0))
#include <linux/sched/signal.h>
#endif
/* The following code comes from:
 * 32-bit misaligned trap handler (c) 1998 San Mehat (CCC) -July 1998
 * /proc/sys/debug/alignment, modified and integrated into
 * Linux 2.1 by Russell King
 *
 * Speed optimisations and better fault handling by Russell King.
 *
 * *** NOTE ***
 * This code is not portable to processors with late data abort handling.
 */
#define CODING_BITS(i)	(i & 0x0e000000)
#define COND_BITS(i)	(i & 0xf0000000)

#define LDST_I_BIT(i)	(i & (1 << 26))		/* Immediate constant	*/
#define LDST_P_BIT(i)	(i & (1 << 24))		/* Preindex		*/
#define LDST_U_BIT(i)	(i & (1 << 23))		/* Add offset		*/
#define LDST_W_BIT(i)	(i & (1 << 21))		/* Writeback		*/
#define LDST_L_BIT(i)	(i & (1 << 20))		/* Load			*/

#define LDST_P_EQ_U(i)	((((i) ^ ((i) >> 1)) & (1 << 23)) == 0)

#define LDSTHD_I_BIT(i)	(i & (1 << 22))		/* double/half-word immed */
#define LDM_S_BIT(i)	(i & (1 << 22))		/* write CPSR from SPSR	*/

#define RN_BITS(i)	((i >> 16) & 15)	/* Rn			*/
#define RD_BITS(i)	((i >> 12) & 15)	/* Rd			*/
#define RM_BITS(i)	(i & 15)		/* Rm			*/

#define REGMASK_BITS(i)	(i & 0xffff)
#define OFFSET_BITS(i)	(i & 0x0fff)

#define IS_SHIFT(i)	(i & 0x0ff0)
#define SHIFT_BITS(i)	((i >> 7) & 0x1f)
#define SHIFT_TYPE(i)	(i & 0x60)
#define SHIFT_LSL	0x00
#define SHIFT_LSR	0x20
#define SHIFT_ASR	0x40
#define SHIFT_RORRRX	0x60

#define DEADCODE 	0xDEADDEAD

/* Thumb-2 32 bit format per ARMv7 DDI0406A A6.3, either f800h,e800h,f800h */
#define IS_T32(hi16) \
	(((hi16) & 0xe000) == 0xe000 && ((hi16) & 0x1800))

union offset_union {
	unsigned long un;
	  signed long sn;
};

#define TYPE_ERROR	0
#define TYPE_FAULT	1
#define TYPE_LDST	2
#define TYPE_DONE	3


static int __maybe_unused
do_busto_fake_ldrstr(unsigned long addr, unsigned long instr, struct pt_regs *regs)
{
#ifndef CONFIG_ARM64
	unsigned int rd = RD_BITS(instr);
#endif

	if (LDST_L_BIT(instr)) {
#ifndef CONFIG_ARM64
		regs->uregs[rd] = DEADCODE;
#endif
	} else {
		rtd_pr_hw_monitor_crit("fake_ldtsrt, inst(0x%lx \n", instr);
		return TYPE_ERROR;
	}
	return TYPE_LDST;
}

void do_busto_finish_ldst(unsigned long addr, unsigned long instr, struct pt_regs *regs, union offset_union offset)
{
	if (!LDST_U_BIT(instr))
		offset.un = -offset.un;

	if (!LDST_P_BIT(instr))
		addr += offset.un;

#ifndef CONFIG_ARM64
	if (!LDST_P_BIT(instr) || LDST_W_BIT(instr))
		regs->uregs[RN_BITS(instr)] = addr;
#endif
}

extern int send_sig_mceerr(int code, void __user *addr, short lsb, struct task_struct *t);
#include <linux/version.h>
int do_rbusto_fake_read(unsigned long addr, unsigned int fsr, struct pt_regs *regs)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 8, 0))
	union offset_union offset={0};
#else
	union offset_union uninitialized_var(offset);
#endif
	unsigned long instr = 0, instrptr;
	int (*handler)(unsigned long addr, unsigned long instr, struct pt_regs *regs);
	unsigned int type;
	//unsigned int fault=0;
	//u16 tinstr = 0;
	int isize = 4;

	instrptr = instruction_pointer(regs);

#ifndef CONFIG_ARM64
	if (thumb_mode(regs)) {
		/// thumb code
		rtd_pr_hw_monitor_crit("busto_exclusive not support thumb code kernel.\n");
		return -1;
	} else {
		fault = probe_kernel_address((void *)instrptr, instr);
		instr = __mem_to_opcode_arm(instr);
	}
#endif

/*coverity CID 485858, Logically dead code*/
#if 0
	if (fault) {
		type = TYPE_FAULT;
		goto bad_or_fault;
	}
#endif

	if (user_mode(regs))
		goto user;
	
#if 0
fixup:
#endif

#ifndef CONFIG_ARM64
	regs->ARM_pc += isize;
#endif
	
	switch (CODING_BITS(instr)) {
	case 0x00000000:	/* 3.13.4 load/store instruction extensions */
		if (LDSTHD_I_BIT(instr))
			offset.un = (instr & 0xf00) >> 4 | (instr & 15);
#ifndef CONFIG_ARM64
		else
			offset.un = regs->uregs[RM_BITS(instr)];
#endif

		if ((instr & 0x000000f0) == 0x000000b0 || /* LDRH, STRH */
		    (instr & 0x001000f0) == 0x001000f0)   /* LDRSH */
			panic("half-word LDR ");
		else if ((instr & 0x001000f0) == 0x000000d0 || /* LDRD */
			 (instr & 0x001000f0) == 0x000000f0)   /* STRD */
			panic("double-word LDR ");
		else if ((instr & 0x01f00ff0) == 0x01000090) /* SWP */
			goto swp;
		else
			goto bad;
		break;

/*coverity CID 485790, Logically dead code*/
#if 0
	case 0x04000000:	/* ldr or str immediate */
		if (COND_BITS(instr) == 0xf0000000) /* NEON VLDn, VSTn */
			goto bad;
		offset.un = OFFSET_BITS(instr);
		handler = do_busto_fake_ldrstr;
		break;

	case 0x06000000:	/* ldr or str register */
#ifndef CONFIG_ARM64
		offset.un = regs->uregs[RM_BITS(instr)];
#endif

		if (IS_SHIFT(instr)) {
			unsigned int shiftval = SHIFT_BITS(instr);

			switch(SHIFT_TYPE(instr)) {
			case SHIFT_LSL:
				offset.un <<= shiftval;
				break;

			case SHIFT_LSR:
				offset.un >>= shiftval;
				break;

			case SHIFT_ASR:
				offset.sn >>= shiftval;
				break;

			case SHIFT_RORRRX:
				if (shiftval == 0) {
					offset.un >>= 1;
#ifndef CONFIG_ARM64
					if (regs->ARM_cpsr & PSR_C_BIT)
						offset.un |= 1 << 31;
#endif
				} else
					offset.un = offset.un >> shiftval |
							  offset.un << (32 - shiftval);
				break;
			}
		}
		handler = do_busto_fake_ldrstr;
		break;
#endif
	default:
		goto bad;
	}

	if (!handler)
		goto bad;
	type = handler(addr, instr, regs);

	if (type == TYPE_ERROR || type == TYPE_FAULT) {
#ifndef CONFIG_ARM64
		regs->ARM_pc -= isize;
#endif
		goto bad_or_fault;
	}

	if (type == TYPE_LDST)
		do_busto_finish_ldst(addr, instr, regs, offset);

	return 0;

 bad_or_fault:
	if (type == TYPE_ERROR)
		goto bad;
	/*
	 * We got a fault - fix it up, or die.
	 */
	///do_bad_area(addr, fsr, regs);
	rtd_pr_hw_monitor_crit( "rbusto_fake_read -4 \n");
	return -4;

 swp:
	rtd_pr_hw_monitor_crit( "busto_exclusive: not handling swp instruction\n");

 bad:
	/*
	 * Oops, we didn't handle the instruction.
	 */
/*coverity CID 21249, Logically dead code*/
#if 0
	rtd_pr_hw_monitor_crit( "busto_exclusive: not handling instruction "
	       "%0*lx at [<%08lx>]\n",
	       isize << 1,
	       isize == 2 ? tinstr : instr, instrptr);
#else
	rtd_pr_hw_monitor_crit( "busto_exclusive: not handling instruction "
	       "%0*lx at [<%08lx>]\n",
	       isize << 1, instr, instrptr);
#endif
	return -3;

 user:
/*coverity CID 21249, Logically dead code*/
#if 0
	rtd_pr_hw_monitor_crit( "busto_exclusive: %s (%d) PC=0x%08lx Instr=0x%0*lx "
	       "Address=0x%08lx FSR 0x%03x\n", current->comm,
	       task_pid_nr(current), instrptr,
	       isize << 1,
	       isize == 2 ? tinstr : instr,
	       addr, fsr);
#else
	rtd_pr_hw_monitor_crit( "busto_exclusive: %s (%d) PC=0x%08lx Instr=0x%0*lx "
	       "Address=0x%08lx FSR 0x%03x\n", current->comm,
	       task_pid_nr(current), instrptr,
	       isize << 1,instr, addr, fsr);
#endif
	
	if (1) {//ai_usermode & UM_SIGNAL) {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
		// plz check commit a89e9b8abf82725e4a in linux5.4/
        
#ifndef MODULE
		struct kernel_siginfo si;

		clear_siginfo(&si);
		si.si_signo = SIGBUS;
		si.si_errno = 0;
		si.si_code = BUS_OBJERR;
		si.si_addr = (void __user *)addr;
		force_sig_info(&si);
#else
		send_sig_mceerr(BUS_OBJERR, (void __user *)addr, 0, NULL);
#endif

#else
		siginfo_t si;

		si.si_signo = SIGBUS;
		si.si_errno = 0;
		si.si_code = BUS_OBJERR;
		si.si_addr = (void __user *)addr;
		force_sig_info(si.si_signo, &si, current);
#endif
	} 

	return -5;
	
}

