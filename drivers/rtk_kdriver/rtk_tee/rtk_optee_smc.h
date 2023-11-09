#ifndef RTK_OPTEE_SMC_H
#define RTK_OPTEE_SMC_H

// #include <linux/arm-smccc.h>
// #include <linux/bitops.h>

// #define OPTEE_SMC_STD_CALL_VAL(func_num) \
	// ARM_SMCCC_CALL_VAL(ARM_SMCCC_STD_CALL, ARM_SMCCC_SMC_32, \
			   // ARM_SMCCC_OWNER_TRUSTED_OS, (func_num))
// #define OPTEE_SMC_FAST_CALL_VAL(func_num) \
	// ARM_SMCCC_CALL_VAL(ARM_SMCCC_FAST_CALL, ARM_SMCCC_SMC_32, \
			   // ARM_SMCCC_OWNER_TRUSTED_OS, (func_num))
#include "optee_smc.h"

/*
 * Release of secondary cores
 *
 * OP-TEE in secure world is in charge of the release process of secondary
 * cores. The Rich OS issue the this request to ask OP-TEE to boot up the
 * secondary cores, go through the OP-TEE per-core initialization, and then
 * switch to the Non-seCure world with the Rich OS provided entry address.
 * The secondary cores enter Non-Secure world in SVC mode, with Thumb, FIQ,
 * IRQ and Abort bits disabled.
 *
 * Call register usage:
 * a0	SMC Function ID, OPTEE_SMC_BOOT_SECONDARY
 * a1	Index of secondary core to boot
 * a2	Upper 32 bits of a 64-bit Non-Secure world entry physical address
 * a3	Lower 32 bits of a 64-bit Non-Secure world entry physical address
 * a4-7	Not used
 *
 * Normal return register usage:
 * a0	OPTEE_SMC_RETURN_OK
 * a1-7	Preserved
 *
 * Error return:
 * a0	OPTEE_SMC_RETURN_EBADCMD		Core index out of range
 * a1-7	Preserved
 *
 * Not idle return register usage:
 * a0	OPTEE_SMC_RETURN_EBUSY
 * a1-7	Preserved
 */
#define OPTEE_SMC_FUNCID_BOOT_SECONDARY  12
#define OPTEE_SMC_BOOT_SECONDARY \
	OPTEE_SMC_FAST_CALL_VAL(OPTEE_SMC_FUNCID_BOOT_SECONDARY)

/*
 * Inform OP-TEE about a new virtual machine
 *
 * Hypervisor issues this call during virtual machine (guest) creation.
 * OP-TEE records client id of new virtual machine and prepares
 * to receive requests from it. This call is available only if OP-TEE
 * was built with virtualization support.
 *
 * Call requests usage:
 * a0	SMC Function ID, OPTEE_SMC_VM_CREATED
 * a1	Hypervisor Client ID of newly created virtual machine
 * a2-6 Not used
 * a7	Hypervisor Client ID register. Must be 0, because only hypervisor
 *      can issue this call
 *
 * Normal return register usage:
 * a0	OPTEE_SMC_RETURN_OK
 * a1-7	Preserved
 *
 * Error return:
 * a0	OPTEE_SMC_RETURN_ENOTAVAIL	OP-TEE have no resources for
 *					another VM
 * a1-7	Preserved
 *
 */
#define OPTEE_SMC_FUNCID_VM_CREATED	13
#define OPTEE_SMC_VM_CREATED \
	OPTEE_SMC_FAST_CALL_VAL(OPTEE_SMC_FUNCID_VM_CREATED)

/*
 * Inform OP-TEE about shutdown of a virtual machine
 *
 * Hypervisor issues this call during virtual machine (guest) destruction.
 * OP-TEE will clean up all resources associated with this VM. This call is
 * available only if OP-TEE was built with virtualization support.
 *
 * Call requests usage:
 * a0	SMC Function ID, OPTEE_SMC_VM_DESTROYED
 * a1	Hypervisor Client ID of virtual machine being shut down
 * a2-6 Not used
 * a7	Hypervisor Client ID register. Must be 0, because only hypervisor
 *      can issue this call
 *
 * Normal return register usage:
 * a0	OPTEE_SMC_RETURN_OK
 * a1-7	Preserved
 *
 */
#define OPTEE_SMC_FUNCID_VM_DESTROYED	14
#define OPTEE_SMC_VM_DESTROYED \
	OPTEE_SMC_FAST_CALL_VAL(OPTEE_SMC_FUNCID_VM_DESTROYED)

/*
 * Query OP-TEE about number of supported threads
 *
 * Normal World OS or Hypervisor issues this call to find out how many
 * threads OP-TEE supports. That is how many standard calls can be issued
 * in parallel before OP-TEE will return OPTEE_SMC_RETURN_ETHREAD_LIMIT.
 *
 * Call requests usage:
 * a0	SMC Function ID, OPTEE_SMC_GET_THREAD_COUNT
 * a1-6 Not used
 * a7	Hypervisor Client ID register
 *
 * Normal return register usage:
 * a0	OPTEE_SMC_RETURN_OK
 * a1	Number of threads
 * a2-7 Preserved
 *
 * Error return:
 * a0	OPTEE_SMC_RETURN_UNKNOWN_FUNCTION   Requested call is not implemented
 * a1-7	Preserved
 */
#define OPTEE_SMC_FUNCID_GET_THREAD_COUNT	15
#define OPTEE_SMC_GET_THREAD_COUNT \
	OPTEE_SMC_FAST_CALL_VAL(OPTEE_SMC_FUNCID_GET_THREAD_COUNT)
/*
 * Set drtc register
 *
 * Call register usage:
 * a0	Function ID
 * a1-6	drtc set info
 * a7	Preserved
 *
 * Normal return register usage:
 * a0	OPTEE_SMC_RETURN_OK
 * a1-7	Preserved
 *
 * Not idle return register usage:
 * a0	OPTEE_SMC_RETURN_EBADCMD
 * a1-7	Preserved
 */
#define OPTEE_SMC_FUNCID_SET_DRTC	21
#define OPTEE_SMC_SET_DRTC \
	OPTEE_SMC_FAST_CALL_VAL(OPTEE_SMC_FUNCID_SET_DRTC)

/*
 * Get git version
 *
 * Call register usage:
 * a0	Function ID
 * a1-6	Preserved
 * a7	Preserved
 *
 * Normal return register usage:
 * a0	OPTEE_SMC_RETURN_OK
 * a1-2	return git version(example: a1 = 0x01234567 a2=0x89abcdef,so git version is:"0123456789abcdef")
 * a3-7
 *
 * Not idle return register usage:
 * a0	OPTEE_SMC_RETURN_EBADCMD
 * a1-7	Preserved
 */
#define OPTEE_SMC_FUNCID_GET_GIT_VERSION    22
#define OPTEE_SMC_GET_GIT_VERSION \
	OPTEE_SMC_FAST_CALL_VAL(OPTEE_SMC_FUNCID_GET_GIT_VERSION)

struct optee_smc_calls_get_git_version_result {
	unsigned long a0;
	unsigned char git_version[12];
};

/*
 * RTK LOGBUF setup
 *
 * Call register usage:
 * a0	Function ID
 * a1-7	Preserved
 *
 * Normal return register usage:
 * a0	OPTEE_SMC_RETURN_OK
 * a1-7	Preserved
 *
 * Not idle return register usage:
 * a0	OPTEE_SMC_RETURN_EBADCMD
 * a1-7	Preserved
 */
#define OPTEE_SMC_FUNCID_SET_LOGBUF	25
#define OPTEE_SMC_SET_LOGBUF \
	OPTEE_SMC_FAST_CALL_VAL(OPTEE_SMC_FUNCID_SET_LOGBUF)

/*
 * RTK HDCP2 OPTEE
 *
 * Call register usage:
 * a0	Function ID
 * a1-7	Preserved
 *
 * Normal return register usage:
 * a0	OPTEE_SMC_RETURN_OK
 * a1-7	Preserved
 *
 * Not idle return register usage:
 * a0	OPTEE_SMC_RETURN_EBADCMD
 * a1-7	Preserved
 */
#define OPTEE_SMC_FUNCID_HDCP2	26
#define OPTEE_SMC_HDCP2 \
	OPTEE_SMC_FAST_CALL_VAL(OPTEE_SMC_FUNCID_HDCP2)

/*
 * Set cw write enable bit
 *
 * Call register usage:
 * a0	Function ID
 * a1	CW_OPS_REQ_MAGIC
 * a2-7	Preserved
 *
 * Normal return register usage:
 * a0	OPTEE_SMC_RETURN_OK
 * a1-7	Preserved
 *
 * Not idle return register usage:
 * a0	OPTEE_SMC_RETURN_EBADCMD
 * a1-7	Preserved
*/
#define OPTEE_SMC_FUNCID_SET_TP_CW_WEN	27
#define OPTEE_SMC_SET_TP_CW_WEN \
	OPTEE_SMC_FAST_CALL_VAL(OPTEE_SMC_FUNCID_SET_TP_CW_WEN)

/*
 * Set shutdown flag
 *
 * Call register usage:
 * a0	Function ID
 * a1	SHUTDOWN_MAGIC
 * a2-7	Preserved
 *
 * Normal return register usage:
 * a0	OPTEE_SMC_RETURN_OK
 * a1-7	Preserved
 *
 * Not idle return register usage:
 * a0	OPTEE_SMC_RETURN_EBADCMD
 * a1-7	Preserved
*/
#define OPTEE_SMC_FUNCID_SHUTDOWN	28
#define OPTEE_SMC_SHUTDOWN \
	OPTEE_SMC_FAST_CALL_VAL(OPTEE_SMC_FUNCID_SHUTDOWN)
#endif