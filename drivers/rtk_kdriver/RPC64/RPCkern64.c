/*
 * $Id: RPCkern.c,v 1.10 2004/8/4 09:25 Jacky Exp $
 */
#include <generated/autoconf.h>
/*
#if defined(CONFIG_MODVERSIONS) && !defined(MODVERSIONS)
#define MODVERSIONS
#endif

#ifdef MODVERSIONS
#include <linux/modversions.h>
#endif

#ifndef MODULE
#define MODULE
#endif
*/
#include <linux/module.h>
#include <rtd_log/rtd_module_log.h>	/* rtd_pr_rpc_info() */
#include <linux/slab.h>		/* kmalloc() */
#include <linux/fs.h>		/* everything... */
#include <linux/errno.h>	/* error codes */
#include <linux/types.h>	/* size_t */
#include <linux/proc_fs.h>
#include <linux/fcntl.h>	/* O_ACCMODE */
#include <linux/ioctl.h>	/* needed for the _IOW etc stuff used later */
#include <linux/sched.h>
#include <linux/version.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
#include <linux/sched/debug.h>
#endif
#include <linux/freezer.h>
#include <linux/delay.h>
#include <rtk_kdriver/RPCDriver.h>
#include <linux/kthread.h>
#include <asm/cacheflush.h>

#include <asm/io.h>
//#include <asm/system.h>	/* cli(), *_flags */
#include <linux/uaccess.h>	/* copy_to_user() copy_from_user() */
#include <rbus/sb2_reg.h>
#include <mach/platform.h>

//#define PDEBUG(fmt, args...) rtd_pr_rpc_alert("RPC: " fmt, ## args)
#define TIMEOUT 30*HZ
#define	LOOPCOUNT 50

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
static RADIX_TREE(kernel_rpc_tree, GFP_ATOMIC);
#else
static struct radix_tree_root kernel_rpc_tree = RADIX_TREE_INIT(GFP_ATOMIC);
#endif
static DEFINE_SEMAPHORE(kernel_rpc_sem);
static DEFINE_SEMAPHORE(kernel_rpc_read_sem);
//extern unsigned int debugMode; // declaire in mach-rtdxxxx/init.c

#ifndef CONFIG_REALTEK_RPC_MULTIPROCESS
typedef struct RPC_STRUCT {
	u32 programID;	/* program ID defined in IDL file */
	u32 versionID;	/* version ID defined in IDL file */
	u32 procedureID;	/* function ID defined in IDL file */

	u32 taskID;	/* the caller's task ID, assign 0 if NONBLOCK_MODE */
	u32 parameterSize;	/* packet's body size */
	u32 mycontext;	/* return address of reply value */
} RPC_STRUCT;
#endif

extern void smp_send_rtk_rpc(int cpu);

typedef struct KRPC_INFO
{
	wait_queue_head_t *wq;
	int reply;
	unsigned int reply_value;
} KRPC_INFO;

RPC_KERN_Dev *rpc_kern_devices;

struct task_struct *rpc_kthread[RPC_NR_KERN_DEVS/RPC_NR_PAIR] = {0};
static wait_queue_head_t rpc_wq[RPC_NR_KERN_DEVS/RPC_NR_PAIR];
static struct semaphore send_kernel_rpc_sem[RPC_NR_KERN_DEVS/RPC_NR_PAIR];
static char *rpc_kthread_name[RPC_NR_KERN_DEVS/RPC_NR_PAIR] = {"rpc-A", "rpc-V1"
#ifdef CONFIG_REALTEK_RPC_VCPU2 																					
																						, "rpc-V2"
#endif
#ifdef CONFIG_REALTEK_RPC_VCPU3
																						, "rpc-V3"
#endif
};

KRPC_INFO rpc_info[RPC_NR_KERN_DEVS / RPC_NR_PAIR];

#ifdef CONFIG_DEBUG_KERNEL_RPC
#define DEBUG_KERNEL_RPC_MAX_LEN 15
#define DEBUG_KERNEL_RPC_MAX_TIME 50
#define KERNEL_RPC_DUMMY_REGISTER	0xb8060130

int kernel_rpc_debug_flag[RPC_NR_KERN_DEVS/RPC_NR_PAIR] = {0};
int kernel_rpc_video_dummy = 0;

typedef struct DEBUG_KERNEL_RPC {
	unsigned long ringIn;
	unsigned long ringOut;
	unsigned long programID;
	unsigned long versionID;
	unsigned long command;
	unsigned long para1;
	unsigned long  timestamp;
} DEBUG_KERNEL_RPC;


DEBUG_KERNEL_RPC debug_kernel_rpc[RPC_NR_KERN_DEVS/RPC_NR_PAIR][DEBUG_KERNEL_RPC_MAX_LEN];
unsigned long krpc_time_before_handle_command[RPC_NR_KERN_DEVS/RPC_NR_PAIR];
unsigned long krpc_time_after_handle_command[RPC_NR_KERN_DEVS/RPC_NR_PAIR];
unsigned long krpc_handle_command_id[RPC_NR_KERN_DEVS/RPC_NR_PAIR];
unsigned long krpc_handle_command_count[RPC_NR_KERN_DEVS/RPC_NR_PAIR];

RPC_COMMON_Dev record_timeout_readbuf[RPC_NR_KERN_DEVS/RPC_NR_PAIR];
RPC_COMMON_Dev record_timeout_writebuf[RPC_NR_KERN_DEVS/RPC_NR_PAIR];

unsigned long record_isr_time[RPC_NR_KERN_DEVS/RPC_NR_PAIR];
unsigned long record_isr_wake_up_time[RPC_NR_KERN_DEVS/RPC_NR_PAIR];
unsigned long record_isr_user_wake_up_time[RPC_NR_KERN_DEVS/RPC_NR_PAIR];
unsigned long record_thread_timeout[RPC_NR_KERN_DEVS/RPC_NR_PAIR];
unsigned long record_send_time[RPC_NR_KERN_DEVS/RPC_NR_PAIR];
#endif

int rpc_kern_is_paused = 0;

static int rpc_kernel_thread(void *p);
extern unsigned char Scaler_GetRPCInitByVCPU(void);

int rpc_kern_init(void)
{
	static int is_init = 0;
	int result = 0, num, i;

	/* Create corresponding structures for each device. */
	rpc_kern_devices = (RPC_KERN_Dev *)get_rpc_addr(RPC_KERN_RECORD_ADDR);

	num = RPC_NR_KERN_DEVS;
	for (i = 0; i < num; i++) {
		PDEBUG("rpc_kern_device %d addr: %px \n", i, &(rpc_kern_devices[i]));

		rpc_kern_devices[i].ringBuf = (RPC_KERN_DEV_ADDR + i * RPC_RING_SIZE) ;

		// Initialize pointers...
		rpc_kern_devices[i].ringStart = rpc_kern_devices[i].ringBuf;
		rpc_kern_devices[i].ringEnd = rpc_kern_devices[i].ringBuf+RPC_RING_SIZE;
		rpc_kern_devices[i].ringIn = rpc_kern_devices[i].ringBuf;
		rpc_kern_devices[i].ringOut = rpc_kern_devices[i].ringBuf;

		PDEBUG("The %dth RPC_KERN_Dev:\n", i);
		PDEBUG("RPC ringStart: 0x%8x\n", (int)rpc_kern_devices[i].ringStart);
		PDEBUG("RPC ringEnd:   0x%8x\n", (int)rpc_kern_devices[i].ringEnd);
		PDEBUG("RPC ringIn:    0x%8x\n", (int)rpc_kern_devices[i].ringIn);
		PDEBUG("RPC ringOut:   0x%8x\n", (int)rpc_kern_devices[i].ringOut);
		PDEBUG("\n");

		if (!is_init) {
			rpc_kern_devices[i].ptrSync = kmalloc(sizeof(RPC_SYNC_Struct), GFP_KERNEL);

			// Initialize wait queue...
			init_waitqueue_head(&(rpc_kern_devices[i].ptrSync->waitQueue));

			// Initialize sempahores...
			sema_init(&rpc_kern_devices[i].ptrSync->readSem, 1);
			sema_init(&rpc_kern_devices[i].ptrSync->writeSem, 1);
		}

		if (i%RPC_NR_PAIR == 1) {
			long ii = (long) i;
			if (rpc_kthread[i / RPC_NR_PAIR] == 0)
				rpc_kthread[i / RPC_NR_PAIR] = kthread_run(rpc_kernel_thread, (void *)ii, "%s", rpc_kthread_name[i/RPC_NR_PAIR]);
		}
	}
	if (!is_init) {
		for (i = 0; i < RPC_NR_KERN_DEVS/RPC_NR_PAIR; i++)
			init_waitqueue_head(&(rpc_wq[i]));
	}

		for (i = 0; i < RPC_NR_KERN_DEVS/RPC_NR_PAIR; i++)
		{
			sema_init(&send_kernel_rpc_sem[i], 1);
		}

	is_init = 1;
	rpc_kern_is_paused = 0;

	return result;
}

int rpc_kern_pause(void)
{
	rpc_kern_is_paused = 1;

	return 0;
}

int rpc_kern_run(void)
{
	int i;
	RPC_KERN_Dev * rpc_dev = (RPC_KERN_Dev *)get_rpc_addr(RPC_KERN_RECORD_ADDR);
	for (i = 0; i < RPC_NR_KERN_DEVS; i++)
	{
		if(!(ring_empty((RPC_COMMON_Dev *)(&rpc_dev[i]))))
		{
			rtd_pr_rpc_notice("rpc_kern_device(%d) not empty:%d\n",
			 i,get_ring_data_size((char *)&(rpc_dev[i].ringStart),(char *)&(rpc_dev[i].ringEnd),(char *)&(rpc_dev[i].ringIn), (char *)&(rpc_dev[i].ringOut)));
			peek_rpc_struct(__func__, (RPC_COMMON_Dev *)(&rpc_dev[i]));
		}
	}
	rpc_kern_is_paused = 0;
	return 0;
}

ssize_t rpc_kern_read(int opt, char *buf, size_t count)
{
	RPC_KERN_Dev *dev;
	int temp, size;
	ssize_t ret = 0;
	u32 ptmp;

	dev = &rpc_kern_devices[opt * RPC_NR_PAIR + 1];

	PDEBUG("read rpc_kern_device: %lx \n", (unsigned long)dev);
	if (down_interruptible(&dev->ptrSync->readSem))
		return -ERESTARTSYS;

	if (dev->ringIn == dev->ringOut)
		goto out;	/* the ring is empty... */
	else if (dev->ringIn > dev->ringOut)
		size = dev->ringIn - dev->ringOut;
	else
		size = RPC_RING_SIZE + dev->ringIn - dev->ringOut;

	if (count > size){
		ret = 0;
		goto out;
		//count = size;
	}

	temp = dev->ringEnd - dev->ringOut;
	if (temp >= count) {
		if (my_copy_user((int *)buf, (int *)get_rpc_addr(dev->ringOut), count)) {
			ret = -EFAULT;
			goto out;
		}
		ret += count;
		ptmp = dev->ringOut + ((count+3) & 0xfffffffc);
		if (ptmp == dev->ringEnd)
			dev->ringOut = dev->ringStart;
		else
			dev->ringOut = ptmp;

		PDEBUG("RPC Read is in 1st kind...\n");
	} else {
		if (my_copy_user((int *)buf, (int *)get_rpc_addr(dev->ringOut), temp)) {
			ret = -EFAULT;
			goto out;
		}
		count -= temp;

		if (my_copy_user((int *)(buf + temp), (int *)get_rpc_addr(dev->ringStart), count)) {
			ret = -EFAULT;
			goto out;
		}
		ret += (temp + count);
		dev->ringOut = dev->ringStart + ((count + 3) & 0xfffffffc);

		PDEBUG("RPC Read is in 2nd kind...\n");
	}
out:
	PDEBUG("RPC kern ringOut pointer is : 0x%8x\n", (int)dev->ringOut);
	up(&dev->ptrSync->readSem);
	return ret;
}

ssize_t rpc_kern_write(int opt, const char *buf, size_t count)
{
	RPC_KERN_Dev *dev;
	int temp, size;
	ssize_t ret = 0;
	u32 ptmp;

	dev = &rpc_kern_devices[opt * RPC_NR_PAIR];

	PDEBUG("write rpc_kern_device: %lx \n", (unsigned long)dev);
	PDEBUG("[rpc_kern_write] write rpc_kern_device: caller%x, *buf:0x%x \n", (unsigned int)__read_caller_register(), *(unsigned int *)buf);
	if (down_interruptible(&dev->ptrSync->writeSem))
		return -ERESTARTSYS;

	if (dev->ringIn == dev->ringOut)
		size = 0;	/* the ring is empty */
	else if (dev->ringIn > dev->ringOut)
		size = dev->ringIn - dev->ringOut;
	else
		size = RPC_RING_SIZE + dev->ringIn - dev->ringOut;
	if ((count+3) > (RPC_RING_SIZE - size - 1))
		goto out;

	temp = dev->ringEnd - dev->ringIn;
	if (temp >= count) {
		if (my_copy_user((int *)get_rpc_addr(dev->ringIn), (int *)buf, count)) {
			ret = -EFAULT;
			goto out;
		}
		ret += count;
		ptmp = dev->ringIn + ((count + 3) & 0xfffffffc);

		dsb(sy);

		if (ptmp == dev->ringEnd)
			dev->ringIn = dev->ringStart;
		else
			dev->ringIn = ptmp;

		PDEBUG("RPC Write is in 1st kind...\n");
	} else {
		if (my_copy_user((int *)get_rpc_addr(dev->ringIn), (int *)buf, temp)) {
			ret = -EFAULT;
			goto out;
		}
		count -= temp;

		if (my_copy_user((int *)get_rpc_addr(dev->ringStart), (int *)(buf + temp), count)) {
			ret = -EFAULT;
			goto out;
		}
		ret += (temp + count);

		dsb(sy);

		dev->ringIn = dev->ringStart + ((count + 3) & 0xfffffffc);

		PDEBUG("RPC Write is in 2nd kind...\n");
	}

	if (opt == RPC_AUDIO){
		rtd_outl(SB2_CPU_SW_INT_0_reg, (RPC_INT_ACPU_S | RPC_INT_WRITE_1));
#ifdef ACPU_CORE_ID
		smp_send_rtk_rpc(ACPU_CORE_ID);
#endif
	}else if (opt == RPC_VIDEO)
		rtd_outl(SB2_CPU_SW_INT_1_reg, (RPC_INT_VCPU_S | RPC_INT_WRITE_1));
#ifdef CONFIG_REALTEK_RPC_VCPU2
	else if (opt == RPC_VIDEO2)							/*video2*/
		rtd_outl(SB2_CPU_SW_INT_3_reg, (RPC_INT_VCPU2_S | RPC_INT_WRITE_1));
#endif
	else
		rtd_pr_rpc_err("error device number...\n");

out:
	PDEBUG("RPC kern ringIn pointer is : 0x%8x\n", (int)dev->ringIn);
	up(&dev->ptrSync->writeSem);
	return ret;
}

int register_kernel_rpc(unsigned long command, FUNC_PTR ptr)
{
	int error;

	down(&kernel_rpc_sem);
	error = radix_tree_preload(GFP_KERNEL);
	if (error == 0) {
		error = radix_tree_insert(&kernel_rpc_tree, command, (void *)ptr);
		if (error)
			rtd_pr_rpc_err("RPC: register kernel rpc %ld error...\n", command);
		radix_tree_preload_end();
	}
	up(&kernel_rpc_sem);

	return error;
}
EXPORT_SYMBOL(register_kernel_rpc);

unsigned int handle_command(unsigned long command, unsigned long param1, unsigned long param2)
{
	FUNC_PTR ptr;
	int ret = 0;

	//rtd_pr_rpc_err("Handle command %lx, param1: %lx, param2: %lx, time:%lu...\n", command, param1, param2, jiffies);
	down(&kernel_rpc_sem);
	ptr = radix_tree_lookup(&kernel_rpc_tree, command);
	up(&kernel_rpc_sem);
	if (ptr)
		ret = ptr(param1, param2);
	else
		rtd_pr_rpc_err("RPC: lookup kernel rpc %ld error...\n", command);

	//rtd_pr_rpc_err("Finish Handle command: %lx, time:%lu... \n", command, jiffies);
	return ret;
}

#ifdef CONFIG_DEBUG_KERNEL_RPC
static void dump_trash_kernel_rpc_buffer(int opt)
{
	RPC_KERN_Dev *devread;
	u32 *data;
	u32 i;

	devread = &rpc_kern_devices[opt*RPC_NR_PAIR+1];

	rtd_pr_rpc_err("kernel rpc -->dump trash buffer\n");
	rtd_pr_rpc_err("previous read buffer:\n");  

	rtd_pr_rpc_err("show_buffer(0x%8x, 0x%8x) InOut(0x%8x, 0x%8x)\n",
		(u32)devread->ringStart,(u32)devread->ringEnd, (u32)devread->ringIn, (u32)devread->ringOut);

	data = (u32 *)get_rpc_addr(devread->ringStart);

	for ( i = 0 ; i < RPC_RING_SIZE ; i+=16)
	{
		rtd_pr_rpc_err("%p: %08x %08x %08x %08x\n", data, ntohl(data[0]), ntohl(data[1]), ntohl(data[2]), ntohl(data[3]));
		data += 4;
	}

	rtd_pr_rpc_err("previous read buffer:\n");
	for(i=0; i<DEBUG_KERNEL_RPC_MAX_LEN; i++ )
	{
		rtd_pr_rpc_err("\n ringIn :0x%8lx,  ringOut:0x%8lx,  programId: %lu, versionID: %lu, command: %lx, para1: %lx,time:%lu \n",debug_kernel_rpc[opt][i].ringIn,debug_kernel_rpc[opt][i].ringOut,
			debug_kernel_rpc[opt][i].programID, debug_kernel_rpc[opt][i].versionID, debug_kernel_rpc[opt][i].command, debug_kernel_rpc[opt][i].para1, debug_kernel_rpc[opt][i].timestamp);
		//rtd_pr_rpc_err("the %d ==>ringIn :0x%8x,  ringOut:0x%8x \n",debug_kernel_rpc[opt][j].ringIn,debug_kernel_rpc[opt][j].ringOut);
	}

	kernel_rpc_debug_flag[opt] = 1;	
	kthread_stop(rpc_kthread[opt]);
	rpc_kthread[opt] = 0;	
	
}
#endif

static int rpc_kernel_thread(void *pp)
{
	char readbuf[sizeof(RPC_STRUCT) + 3 * sizeof(unsigned int)] = {0};
	RPC_KERN_Dev *dev;
	RPC_STRUCT *rpc;
	unsigned int *tmp;
	long p = (long) pp;
	int i=0, f = ((int)p/RPC_NR_PAIR);
	unsigned long previousprint = 0;

/*	daemonize(current->comm);*/

	dev = &rpc_kern_devices[(int)p];
	while (1) {
//		if (current->flags & PF_FREEZE)
//			refrigerator(PF_FREEZE);
		try_to_freeze();

//		rtd_pr_rpc_info(" #@# wait %s %x %x \n", current->comm, dev, dev->waitQueue);
		//if (wait_event_interruptible(dev->ptrSync->waitQueue, (kthread_should_stop()) | (dev->ringIn != dev->ringOut))) {
		if (wait_event_interruptible_timeout(dev->ptrSync->waitQueue, (kthread_should_stop()) | (dev->ringIn != dev->ringOut), HZ)<0) {
			rtd_pr_rpc_err("%s got signal or should stop...\n", current->comm);
			continue;
		}
#ifdef CONFIG_DEBUG_KERNEL_RPC
		if(kernel_rpc_debug_flag[f] ==0)
			record_thread_timeout[f] = jiffies;
#endif
//		rtd_pr_rpc_info(" #@# wakeup %s \n", current->comm);

		if (kthread_should_stop()) {
			rtd_pr_rpc_err("%s exit...\n", current->comm);
			break;
		}

		if(dev->ringIn == dev->ringOut)
			continue;

#ifdef CONFIG_DEBUG_KERNEL_RPC
		//record buffer information
		if(kernel_rpc_debug_flag[f] ==0){
			for(i = 0; i< (DEBUG_KERNEL_RPC_MAX_LEN-1) ; i++){
				debug_kernel_rpc[f][i].ringIn = debug_kernel_rpc[f][i+1].ringIn;
				debug_kernel_rpc[f][i].ringOut = debug_kernel_rpc[f][i+1].ringOut;
	 			debug_kernel_rpc[f][i].timestamp = debug_kernel_rpc[f][i+1].timestamp;
				debug_kernel_rpc[f][i].programID = debug_kernel_rpc[f][i+1].programID;
				debug_kernel_rpc[f][i].versionID= debug_kernel_rpc[f][i+1].versionID;
				debug_kernel_rpc[f][i].command = debug_kernel_rpc[f][i+1].command;
				debug_kernel_rpc[f][i].para1= debug_kernel_rpc[f][i+1].para1;

			}
			debug_kernel_rpc[f][DEBUG_KERNEL_RPC_MAX_LEN-1].ringIn = (int)dev->ringIn;
			debug_kernel_rpc[f][DEBUG_KERNEL_RPC_MAX_LEN-1].ringOut = (int)dev ->ringOut;
			debug_kernel_rpc[f][DEBUG_KERNEL_RPC_MAX_LEN-1].timestamp = jiffies;

			debug_kernel_rpc[f][DEBUG_KERNEL_RPC_MAX_LEN-1].programID = 0; //have not get this message
			debug_kernel_rpc[f][DEBUG_KERNEL_RPC_MAX_LEN-1].versionID = 0;
			debug_kernel_rpc[f][DEBUG_KERNEL_RPC_MAX_LEN-1].command = 0;
			debug_kernel_rpc[f][DEBUG_KERNEL_RPC_MAX_LEN-1].para1 =0;
		}
#endif

		// read the reply data...
		if (rpc_kern_read(((int)p)/RPC_NR_PAIR, readbuf, sizeof(RPC_STRUCT)) != sizeof(RPC_STRUCT)) {
			if(time_after(jiffies, previousprint)){
				rtd_pr_rpc_err("ERROR in read kernel RPC struct in %s ...\n", current->comm);
				previousprint = jiffies+HZ;
			}
			continue;
		}

		rpc = (RPC_STRUCT *) readbuf;
		tmp = (unsigned int *)(readbuf + sizeof(RPC_STRUCT));

#ifdef CONFIG_DEBUG_KERNEL_RPC
		//update programID & versionID
		if(kernel_rpc_debug_flag[f] ==0){
			debug_kernel_rpc[f][DEBUG_KERNEL_RPC_MAX_LEN-1].programID = htonl(rpc->programID);
			debug_kernel_rpc[f][DEBUG_KERNEL_RPC_MAX_LEN-1].versionID = htonl(rpc->versionID);
		}
#endif

		if ((htonl(rpc->programID) == R_PROGRAM) || (htonl(rpc->programID) == AUDIO_AGENT)
			||(htonl(rpc->programID) == VIDEO_AGENT) || (htonl(rpc->programID)== KERNELID)) //RPC data
		{
			/* handle the request... */
			char replybuf[sizeof(RPC_STRUCT) + 2 * sizeof(unsigned int)];
			unsigned int ret;
			RPC_STRUCT *rrpc = (RPC_STRUCT *)replybuf;
			int count = 0;

			/* read the payload... */
			while ((rpc_kern_read(((int)p) / RPC_NR_PAIR, readbuf + sizeof(RPC_STRUCT), 3 * sizeof(unsigned int)) 
			!= 3 * sizeof(unsigned int)) && (count<LOOPCOUNT)) {/*loop 500 ms*/
				//rtd_pr_rpc_err("ERROR in read para payload in %s...\n", current->comm);
				count ++;
				msleep(10);
				//continue;
			}

			if(count == LOOPCOUNT){
				rtd_pr_rpc_err("ERROR in read para payload in %s...\n", current->comm);
				//drop data
				if (down_interruptible(&dev->ptrSync->readSem))
						return -ERESTARTSYS;
				dev->ringOut = dev->ringIn;
				up(&dev->ptrSync->readSem);
				if(rpc->taskID)  //block mode, send reply
				{
					// fill the RPC_STRUCT...
					rrpc->programID = (unsigned int)htonl(REPLYID);
					rrpc->versionID = (unsigned int)htonl(REPLYID);
					rrpc->procedureID = 0;
					rrpc->taskID = 0;
#ifdef CONFIG_REALTEK_RPC_MULTIPROCESS
					rrpc->sysPID = 0;
#endif
					rrpc->parameterSize = (unsigned int)htonl(2*sizeof(unsigned int));
					rrpc->mycontext = rpc->mycontext;

					// fill the parameters...
					tmp = (unsigned int *)(replybuf+sizeof(RPC_STRUCT));
					*(tmp+0) = rpc->taskID;
					*(tmp+1) = (unsigned int)htonl(0);


					if (rpc_kern_write(((int)p)/RPC_NR_PAIR, replybuf, sizeof(replybuf)) != sizeof(replybuf)) {
						rtd_pr_rpc_err("ERROR in send ack from kernel to %s ...\n", current->comm);
						return RPC_FAIL;
					}
				}
				else{
					rtd_pr_rpc_notice("kernel RPC: discard RPC data from %s ...\n", current->comm);

				}
				rtd_pr_rpc_err("kernel rpc payload: program:%u version:%u procedure:%u taskID:%u sysPID:%x size:%u context:%x",
					htonl(rpc->programID), htonl(rpc->versionID), htonl(rpc->procedureID), htonl(rpc->taskID), htonl(rpc->sysPID), htonl(rpc->parameterSize), htonl(rpc->mycontext));
				continue;
			}

#ifdef CONFIG_DEBUG_KERNEL_RPC
			if(kernel_rpc_debug_flag[f] ==0){
				krpc_time_before_handle_command[f] = jiffies;
				krpc_handle_command_id[f] = ntohl(*tmp);
				krpc_handle_command_count[f] ++;
			}
#endif
			ret = handle_command(ntohl(*tmp), ntohl(*(tmp+1)), ntohl(*(tmp+2)));
#ifdef CONFIG_DEBUG_KERNEL_RPC
			if(kernel_rpc_debug_flag[f] ==0){
				krpc_time_after_handle_command[f] = jiffies;
				krpc_handle_command_count[f]--;
				debug_kernel_rpc[f][DEBUG_KERNEL_RPC_MAX_LEN-1].command = ntohl(*tmp);
				debug_kernel_rpc[f][DEBUG_KERNEL_RPC_MAX_LEN-1].para1 =ntohl(*(tmp+1));
			}
#endif
			if (rpc->taskID) {	/*for blocking mode RPC request */
				/* fill the RPC_STRUCT... */
				rrpc->programID = (unsigned int)htonl(REPLYID);
				rrpc->versionID = (unsigned int)htonl(REPLYID);
				rrpc->procedureID = 0;
				rrpc->taskID = 0;
#ifdef CONFIG_REALTEK_RPC_MULTIPROCESS
				rrpc->sysPID = 0;
#endif
				rrpc->parameterSize = (unsigned int)htonl(2 * sizeof(unsigned int));
				rrpc->mycontext = rpc->mycontext;

				/* fill the parameters... */
				tmp = (unsigned int *)(replybuf + sizeof(RPC_STRUCT));
				*(tmp + 0) = rpc->taskID;
				*(tmp + 1) = (unsigned int)htonl(ret);

				if (rpc_kern_write(((int)p) / RPC_NR_PAIR, replybuf, sizeof(replybuf)) != sizeof(replybuf)) {
					rtd_pr_rpc_err("ERROR in send ack from kernel to %s ...\n", current->comm);
					return RPC_FAIL;
				}
			}
		}
		else if((htonl(rpc->programID)== REPLYID) && (htonl(rpc->versionID) == REPLYID)) {	// reply data
			int rpc_opt = 0;
			KRPC_INFO *info;
			int countack = 0;
			/* read the payload... */
			while ((rpc_kern_read(((int)p) / RPC_NR_PAIR,readbuf + sizeof(RPC_STRUCT),2 * sizeof(unsigned int))
			!= 2 * sizeof(unsigned int)) && (countack < LOOPCOUNT)) {
				//rtd_pr_rpc_err("ERROR in read reply payload in %s...\n", current->comm);
				countack++;
				msleep(10);
				//continue;
			}

			if(countack == LOOPCOUNT){
				rtd_pr_rpc_err("ERROR in read reply payload in %s...\n", current->comm);
				//rtd_pr_rpc_notice("kernel RPC: ack data not correct from %s...\n", current->comm);
				//drop the data
				if (down_interruptible(&dev->ptrSync->readSem))
						return -ERESTARTSYS;
				dev->ringOut = dev->ringIn;
				up(&dev->ptrSync->readSem);
				rtd_pr_rpc_err("kernel rpc ack: program:%u version:%u procedure:%u taskID:%u sysPID:%x size:%u context:%x",
					rpc->programID, rpc->versionID, rpc->procedureID, rpc->taskID, rpc->sysPID, rpc->parameterSize, (unsigned long)rpc->mycontext);
				continue;
			}

			/* parse the reply data... */
			rpc_opt = (int)(*tmp) - 0x100;
			info = &rpc_info[rpc_opt];
			
			info->reply = 1;
			info->reply_value = ntohl(*(tmp + 1));
			wake_up(info->wq);	/* ack the sync... */

#ifdef CONFIG_DEBUG_KERNEL_RPC
			if(kernel_rpc_debug_flag[f] ==0){
				debug_kernel_rpc[f][DEBUG_KERNEL_RPC_MAX_LEN-1].command = ntohl(*tmp);
				debug_kernel_rpc[f][DEBUG_KERNEL_RPC_MAX_LEN-1].para1 =ntohl(*(tmp+1));
			}
#endif
		}
		else{
			rtd_pr_rpc_err("ERROR kernel rpc:%s send a RPC with error programID : %u to kernel...\n",rpc_kthread_name[((int)p)/RPC_NR_PAIR], htonl(rpc->programID));
#ifdef CONFIG_DEBUG_KERNEL_RPC
			dump_trash_kernel_rpc_buffer(((int)p)/RPC_NR_PAIR);
#endif
		}
	}

	return 0;
}

#if defined(CONFIG_REALTEK_VDEC) || defined(CONFIG_RTK_KDRV_VDEC)
extern void rtkvdec_show_sharedmemory(void) ;
#endif

#ifdef CONFIG_DEBUG_KERNEL_RPC
void dump_kernel_rpc_data(int opt)
{
	RPC_KERN_Dev *devread, *devwrite;
	int j=0;

	devread = &rpc_kern_devices[opt*RPC_NR_PAIR+1];
	devwrite = &rpc_kern_devices[opt*RPC_NR_PAIR];
	if(kernel_rpc_debug_flag[RPC_KERN_DEV_V1S_ID3/RPC_NR_PAIR] == 0){
		kernel_rpc_video_dummy = (unsigned int)rtd_inl(KERNEL_RPC_DUMMY_REGISTER);
		my_memcpy((int *)&record_timeout_readbuf[opt], (int *)devread, sizeof(RPC_COMMON_Dev));
		my_memcpy((int *)&record_timeout_writebuf[opt], (int *)devwrite, sizeof(RPC_COMMON_Dev));
	}

	rtd_pr_rpc_err("current time: %lu, send time: %lu, record_isr_time: %lu , wake_up_time: %lu, user_wake_up_time: %lu, enter thread time:%lu \n", jiffies,
		record_send_time[opt], record_isr_time[opt], record_isr_wake_up_time[opt], record_isr_user_wake_up_time[opt], record_thread_timeout[opt]);
	rtd_pr_rpc_err("last time rpc thread handle rpc command (%lu) --> enter time: %lu, out time: %lu, count : %lu\n", krpc_handle_command_id[opt], krpc_time_before_handle_command[opt],
		krpc_time_after_handle_command[opt], krpc_handle_command_count[opt]);

	rtd_pr_rpc_err("kernel rpc timeout! current read buffer status ringIn: 0x%8x, ringOut:0x%8x\n", (int)record_timeout_readbuf[opt].ringIn, (int)record_timeout_readbuf[opt].ringOut);
	if(devread->ringIn != devread->ringOut)
		peek_rpc_struct("kern_read_buffer", (RPC_COMMON_Dev *)&record_timeout_readbuf[opt]);

	rtd_pr_rpc_err("kernel rpc timeout! current write buffer status ringIn: 0x%8x, ringOut:0x%8x\n", (int)record_timeout_writebuf[opt].ringIn, (int)record_timeout_writebuf[opt].ringOut);
	if(devwrite->ringIn != devwrite->ringOut)
		peek_rpc_struct("kern_write_buffer", (RPC_COMMON_Dev *)&record_timeout_writebuf[opt]);

	if(opt == RPC_VIDEO)
		rtd_pr_rpc_err("kernel rpc timeout! current %s dummy register info:0x%08x\n", rpc_kthread_name[opt], kernel_rpc_video_dummy);

	rtd_pr_rpc_err("previous read buffer:\n");
	for(j=0; j<DEBUG_KERNEL_RPC_MAX_LEN; j++ )
	{
		rtd_pr_rpc_err("\n ringIn :0x%8lx,  ringOut:0x%8lx,  programId: %lu, versionID: %lu, command: %lx, para1: %lx,time:%lu \n",debug_kernel_rpc[opt][j].ringIn,debug_kernel_rpc[opt][j].ringOut,
			debug_kernel_rpc[opt][j].programID, debug_kernel_rpc[opt][j].versionID, debug_kernel_rpc[opt][j].command, debug_kernel_rpc[opt][j].para1, debug_kernel_rpc[opt][j].timestamp);
		//rtd_pr_rpc_err("the %d ==>ringIn :0x%8x,  ringOut:0x%8x \n",debug_kernel_rpc[opt][j].ringIn,debug_kernel_rpc[opt][j].ringOut);
	}

#if defined(CONFIG_REALTEK_VDEC) || defined(CONFIG_RTK_KDRV_VDEC)
	rtkvdec_show_sharedmemory();
#endif

	if(opt == RPC_AUDIO)
		rtd_outl(DUMP_AUDIO_BUFFER_DUMMY_REGISTER, 0xC0DE0002);

}
#endif

/*int send_rpc_command(int opt, unsigned long command, unsigned long param1, unsigned long param2, unsigned long *retvalue)*/
int send_krpc(int opt,
			KRPC_PROCEDUREID procedureID,
			KRPC_SENDMODE send_mode,
			unsigned long command,
			unsigned long param1,
			unsigned long param2,
			unsigned long *retvalue)
{
	char sendbuf[sizeof(RPC_STRUCT) + 3 * sizeof(unsigned int)];
	RPC_STRUCT *rpc = (RPC_STRUCT *) sendbuf;
	unsigned int *tmp;
#ifdef CONFIG_DEBUG_KERNEL_RPC
	unsigned long previousprint = 0;
#endif

	if (rpc_kern_is_paused) {
		rtd_pr_rpc_err("RPCkern: someone access rpc kern during the pause...\n");
		BUG();
		return RPC_FAIL;
	}
	
	if(opt >= RPC_AUDIO2){
		rtd_pr_rpc_err("RPCkern: no this RPC channel(%d), command: %lx...\n", opt, command);
		return RPC_FAIL;
	}

	down(&send_kernel_rpc_sem[opt]);
	if (rpc_kthread[opt] == 0) {
		rtd_pr_rpc_err("RPCkern: %s is disabled...\n", rpc_kthread_name[opt]);
#ifdef CONFIG_DEBUG_KERNEL_RPC
		if(time_after(jiffies, previousprint)){
			dump_kernel_rpc_data(opt);
			previousprint = jiffies+5*HZ;
		}	
#endif
#if defined(CONFIG_REALTEK_VDEC) || defined(CONFIG_RTK_KDRV_VDEC)
		if ( opt == RPC_VIDEO)
			rtkvdec_show_sharedmemory();
#endif
		up(&send_kernel_rpc_sem[opt]);
		return RPC_FAIL;
	}

/*
	if (opt == RPC_AUDIO) {
		//invalidate sync flag before read
		dmac_flush_range(phys_to_virt(0x000000d0), phys_to_virt(0x000000d4));
		outer_inv_range(0x000000d0, 0x000000d4);

		   // Make sure ACPU should init RPC ready before send RPC
		   // ACPU should set sync flag value not same with 0xFFFFFFFF after init RPC ready.
		if (*((int *)phys_to_virt(0x000000d0)) == (0xffffffff)) {
			rtd_pr_rpc_err(KERN_INFO "RPCkern: ACPU not finished init RPC ...\n");
			up(&send_kernel_rpc_sem[opt]);
			return RPC_ACPU_NOT_READY;
		}
	}
*/

	if( opt == RPC_VIDEO)
	{
		//invalidate sync flag before read
		dmac_flush_range(phys_to_virt(0x000000d4), phys_to_virt(0x000000d8));
		outer_inv_range(0x000000d4,0x000000d8);

		// Make sure VCPU should init RPC ready before send RPC
		// VCPU should set sync flag value not same with 0xFFFFFFFF after init RPC ready.
		if(*((int *)phys_to_virt(0x000000d4)) == (0xffffffff))
		{
			rtd_pr_rpc_err("RPCkern: VCPU not finished init RPC ...\n");
			up(&send_kernel_rpc_sem[opt]);
			return RPC_VCPU_NOT_READY;
		}
	}

#ifdef CONFIG_REALTEK_RPC_VCPU2
	if (opt == RPC_VIDEO2) {
		/*invalidate sync flag before read */
		dmac_flush_range(phys_to_virt(SYNC_FLAG_V2CPU),
				 phys_to_virt(0x000000dc));
		outer_inv_range(SYNC_FLAG_V2CPU, 0x000000dc);

		/*
		   // Make sure VCPU2 should init RPC ready before send RPC
		   // VCPU2 should set sync flag value not same with 0xFFFFFFFF after init RPC ready.
		 */
		if (*((int *)phys_to_virt(SYNC_FLAG_V2CPU)) == (0xffffffff)) {
			rtd_pr_rpc_info(
			       "RPCkern: VCPU2 not finished init RPC ...\n");
			up(&send_kernel_rpc_sem[opt]);
			return RPC_VCPU_NOT_READY;
		}
	}
#endif

#ifdef CONFIG_REALTEK_RPC_VCPU3
	if (opt == RPC_VIDEO3) {
		/*invalidate sync flag before read */
		dmac_flush_range(phys_to_virt(SYNC_FLAG_V3CPU),
				 phys_to_virt(0x00000068));
		outer_inv_range(SYNC_FLAG_V3CPU, 0x00000068);

		/*
		   // Make sure VCPU3 should init RPC ready before send RPC
		   // VCPU3 should set sync flag value not same with 0xFFFFFFFF after init RPC ready.
		 */
		if (*((int *)phys_to_virt(SYNC_FLAG_V3CPU)) == (0xffffffff)) {
			rtd_pr_rpc_info(
			       "RPCkern: VCPU3 not finished init RPC ...\n");
			up(&send_kernel_rpc_sem[opt]);
			return RPC_VCPU_NOT_READY;
		}
	}
#endif

/*	rtd_pr_rpc_info(" #@# sendbuf: %d \n", sizeof(sendbuf));*/
	rpc_info[opt].wq = &rpc_wq[opt];
	rpc_info[opt].reply = 0;
	/* fill the RPC_STRUCT... */
	rpc->programID = htonl(KERNELID);
	rpc->versionID = htonl(KERNELID);
	rpc->procedureID = htonl(procedureID);
	rpc->taskID = (send_mode & KRPC_BLOCK_MODE) ? (opt + 0x100) : 0;
#ifdef CONFIG_REALTEK_RPC_MULTIPROCESS
	rpc->sysPID = 0;
#endif
	rpc->parameterSize = htonl(3 * sizeof(unsigned int));
	rpc->mycontext = 0x1000;//no longer need in 64bit, reserve

	/* fill the parameters... */
	tmp = (unsigned int *)(sendbuf + sizeof(RPC_STRUCT));
/*	rtd_pr_rpc_info(" aaa: %x bbb: %x \n", sendbuf, tmp);*/
	*tmp = (unsigned int)htonl(command);
	*(tmp + 1) = (unsigned int)htonl(param1);
	*(tmp + 2) = (unsigned int)htonl(param2);

	if (rpc_kern_write(opt, sendbuf, sizeof(sendbuf)) != sizeof(sendbuf)) {
		rtd_pr_rpc_err("ERROR in send kernel RPC to %s : command:%x, parm1: %x, parm2: %x ...\n", rpc_kthread_name[opt],
			command, param1, param2);
		up(&send_kernel_rpc_sem[opt]);
		return RPC_FAIL;
	}

#ifdef CONFIG_DEBUG_KERNEL_RPC
	if(opt == RPC_VIDEO)
		rtd_maskl(KERNEL_RPC_DUMMY_REGISTER, 0x1e000007, 0x00000000);

	record_send_time[opt] = jiffies;
#endif

	if(send_mode & KRPC_BLOCK_MODE) //for block mode
   	{
		// wait the result...
		if (!wait_event_timeout(rpc_wq[opt], rpc_info[opt].reply, TIMEOUT)) {
			rtd_pr_rpc_err("[krpc timeout v1.2]kernel rpc to %s wait ack timeout ->disable %s thread:  command:%x, parm1: %x, parm2: %x ...\n", rpc_kthread_name[opt],
				rpc_kthread_name[opt], command, param1, param2);
#ifdef CONFIG_DEBUG_KERNEL_RPC
			dump_kernel_rpc_data(opt);
			kernel_rpc_debug_flag[opt] = 1;
			if(krpc_handle_command_count[opt] != 0 )
				dump_stack();
#endif
			kthread_stop(rpc_kthread[opt]);
			rpc_kthread[opt] = 0;
			up(&send_kernel_rpc_sem[opt]);
			//if (debugMode == RELEASE_LEVEL)
			//	panic("RPC timeout in release mode!!! need micom reset \n");

			return RPC_FAIL;
		} else {
			*retvalue = rpc_info[opt].reply_value; 
			//rtd_pr_rpc_err(" #@# ret: %lx \n", *retvalue); 
			up(&send_kernel_rpc_sem[opt]);
			return RPC_OK;
		}
	}
	else
	{
		up(&send_kernel_rpc_sem[opt]);
		return RPC_OK;
	}
}

EXPORT_SYMBOL(send_krpc);

int send_rpc_command(int opt, unsigned long command, unsigned long param1, unsigned long param2, unsigned long *retvalue)
{
	return send_krpc(opt, KRPC_PROCEDUREID_KERNEL,KRPC_BLOCK_MODE, command, param1, param2, retvalue);
}



EXPORT_SYMBOL(send_rpc_command);

