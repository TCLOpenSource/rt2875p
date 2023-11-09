/******************************************************************************
 *
 *   Copyright(c) 2014 Realtek Semiconductor Corp. All rights reserved.
 *
 *   @author yungjui.lee@realtek.com
 *
 *****************************************************************************/

#ifndef _RTK_COMMON_
#define _RTK_COMMON_

#include <linux/string.h>
#include "AudioRPC_System.h"
#include "AudioRPCBaseDS_data.h"
#include "AudioRPC_Agent_data.h"
#include "audio_common.h"
#include "AudioInbandAPI.h"
#include <rtk_kdriver/rtkaudio_debug.h>
#include "ioctrl/audio/audio_cmd_id.h"

extern int64_t CLOCK_GetPTS(void);
#define pli_getPTS() CLOCK_GetPTS()

#if 1
#define AUDIO_INFO    rtd_pr_adsp_info
#define AUDIO_ERROR   rtd_pr_adsp_err
#define AUDIO_FATAL   rtd_pr_adsp_emerg
#define AUDIO_VERBOSE rtd_pr_adsp_debug
#define AUDIO_DEBUG   rtd_pr_adsp_debug
#define DEBUG         rtd_pr_adsp_debug
#define ERROR         rtd_pr_adsp_err
#define INFO          rtd_pr_adsp_info
#else //for debug only
#define AUDIO_INFO    rtd_pr_adsp_emerg
#define AUDIO_ERROR   rtd_pr_adsp_emerg
#define AUDIO_FATAL   rtd_pr_adsp_emerg
#define AUDIO_VERBOSE rtd_pr_adsp_emerg
#define AUDIO_DEBUG   rtd_pr_adsp_emerg
#define DEBUG         rtd_pr_adsp_emerg
#define ERROR         rtd_pr_adsp_emerg
#define INFO          rtd_pr_adsp_emerg
#endif

#define H_FLAG_SUCCESS                  (0x10000000)
#define S_OK                            ((H_FLAG_SUCCESS) |     (0x00000000))
#define S_FALSE                         ((H_FLAG_SUCCESS) |     (0x00000001))


#ifndef TRUE
#define TRUE					(1)
#endif

#ifndef FALSE
#define FALSE					(0)
#endif

#ifndef BOOLEAN
#ifndef _EMUL_WIN
typedef unsigned int            __BOOLEAN;
#define BOOLEAN __BOOLEAN
#else
typedef unsigned char           __BOOLEAN;
#define BOOLEAN __BOOLEAN
#endif
#endif

#ifndef BOOL
#define BOOL                        bool
#endif

#ifndef UINT8
#define UINT8 unsigned char
#endif
#ifndef SINT8
#define SINT8 signed char
#endif
#ifndef UINT16
#define UINT16 unsigned short
#endif
#ifndef SINT16
#define SINT16 signed short
#endif
#ifndef UINT32
#define UINT32 unsigned int
#endif
#ifndef SINT32
#define SINT32 signed int
#endif
#ifndef UINT64
#define UINT64 unsigned long long
#endif
#ifndef SINT64
#define SINT64 signed long long
#endif

#define UNDEFINED_AGENT_ID (0xffffffff)
#define UNINIT_PINID       (0xffffffff)

typedef enum {
    STATE_STOP=0,
    STATE_PAUSE=1,
    STATE_RUN=2,
    STATE_MAX=3
}STATE;

typedef enum {
    EVENT_EOS=1,
}EVENT;

#define NO_ERR        (0)
#define ERR_UNDERFLOW (0x1)
#define ERR_OVERFLOW  (0x2)

/** Define the base pin **/
typedef struct _Pin {
    // member variables
    void* pDerivedObj;
    void* allocator;
    SINT32 listSize;

    // member functions
    UINT32  (*GetReadSize)(struct _Pin*, UINT32);
    UINT32  (*GetWriteSize)(struct _Pin*);
    UINT8*  (*GetReadPtr)(struct _Pin*, UINT32);
    UINT32  (*SetReadPtr)(struct _Pin*, UINT8*, UINT32);
    UINT32  (*SetNumOfReadPtr)(struct _Pin*, UINT32);
    UINT8*  (*GetWritePtr)(struct _Pin*);
    UINT32  (*SetWritePtr)(struct _Pin*, UINT8*);
    UINT32  (*GetBufHeader)(struct _Pin*, void**, UINT32*);
    UINT32  (*GetBufAddress)(struct _Pin*, UINT8**, UINT32*);
    UINT32  (*GetListSize)(struct _Pin*);
    UINT32  (*GetBufferSize)(struct _Pin*);
    void    (*SetBufferSize)(struct _Pin*, UINT32);
    UINT32  (*GetBuffer)(struct _Pin*, UINT8**, UINT32*);
    UINT32  (*GetPhysicalWriteAddress)(struct _Pin*);
    UINT32  (*ResetPin)(struct _Pin*);
    void    (*Delete)(struct _Pin*);
    /* Ring Buffer Usage */

    UINT32  (*MemCopyFromReadPtr)(struct _Pin*, UINT8*, UINT32, UINT32);

//    UINT32  (*MemCopyFromReadPtr)(struct _Pin*, UINT8*, UINT32);
//    UINT32  (*MemCopyFromReadPtrNonInverse)(struct _Pin*, UINT8*, UINT32);

    UINT32  (*MemCopyToWritePtr)(struct _Pin*, UINT8*, UINT32);
    UINT32  (*MemCopyToWritePtrNoInverse)(struct _Pin*, UINT8*, UINT32);
    UINT32  (*AddReadPtr)(struct _Pin*, UINT32);
    UINT32  (*AddWritePtr)(struct _Pin*, UINT32);
    UINT32  (*CheckReadableSize)(struct _Pin*, UINT32);
    UINT32  (*CheckWritableSize)(struct _Pin*, UINT32);
} Pin;
Pin* new_pin(UINT32 bufSize);
Pin* new_pin_Cached(UINT32 bufSize);
/** End of base pin **/

/** Define the base module**/
typedef struct _Base {
    // member variables
    void* pDerivedObj;
    UINT32 instanceID;
    char name[16];
    Pin *inPin;
    Pin *outPin;
    UINT32 state;
    UINT32 inPinID;
    UINT32 outPinID;
    struct list_head list;
    struct list_head flowList;
    void* flowEventQ;
    void* flowUserQ;

    // member functions
    void    (*Delete)(struct _Base*);
    UINT32  (*Run)(struct _Base*);
    UINT32  (*Pause)(struct _Base*);
    UINT32  (*Stop)(struct _Base*);
    UINT32  (*Flush)(struct _Base*);
    UINT32  (*SetRefClock)(struct _Base*, UINT32);
    Pin*    (*GetOutPin)(struct _Base*);
    UINT32  (*GetInPinID)(struct _Base*);
    UINT32  (*GetOutPinID)(struct _Base*);
    UINT32  (*GetState)(struct _Base*);
    UINT32  (*GetAgentID)(struct _Base*);
    UINT32  (*InitOutRingBuf)(struct _Base*);
    UINT32  (*ResetOutRingBuf)(struct _Base*);
    UINT32  (*PrivateInfo)(struct _Base*, UINT32, UINT8*, UINT32);
    UINT32  (*SwitchFocus)(struct _Base*, void*);
    UINT32  (*SetSeekingInfo)(struct _Base*, SINT32, SINT32);
    UINT32  (*Connect)(struct _Base*, struct _Base*);
    UINT32  (*Disconnect)(struct _Base*, struct _Base*);
    UINT32  (*Remove)(struct _Base*);
    UINT32  (*SetFlowEventQ)(struct _Base*,void*,void*);
} Base;

Base* new_base(void);
void delete_base(Base* pBaseObj);
UINT32 Remove(Base*);
/** End of base module **/

/** ReferenceClock **/
typedef struct _REFERENCECLOCK {
    REFCLOCK    *m_core;
    UINT8       *m_virtualAddr;
    UINT32      m_phyAddr;

    void   (*Delete)(struct _REFERENCECLOCK*);
    UINT32 (*GetCoreAddress)(struct _REFERENCECLOCK*, UINT32*, UINT8**, UINT8**);
} ReferenceClock;
ReferenceClock* new_ReferenceClock(void);
/** End of Referenceclock **/

typedef enum {
    RTKAUDIO_QS_HDMI,
    RTKAUDIO_QS_DP
} RTKAUDIO_QS_TYPE;

RTKAUDIO_QS_TYPE rtkaudio_quickshow_init_table(AUDIO_QUICK_SHOW_PARAM *param);

/**** common functions ****/
int64_t getpts(void);
UINT32 CreateAgent(UINT32 type);
UINT32 DestroyAgent(UINT32 *instanceID);

unsigned int memory_align(unsigned int size, unsigned int alignsz);
void add_to_module_list(Base* module_handle);
void delete_from_module_list(Base* module_handle);

void add_id_to_map(Base* module_handle, UINT32 instanceID);
void del_id_from_map(UINT32 instanceID);
Base* map_to_module(UINT32 instanceID);

void IPC_WriteU32(BYTE* des, unsigned long data);
void IPC_WriteU64(BYTE* des, unsigned long long data);
uint32_t IPC_ReadU32(BYTE* src);
unsigned long long IPC_ReadU64(BYTE* src);

void IPC_memcpy(int *des, int *src, int nSample);

#endif /*_RTK_COMMON_*/
