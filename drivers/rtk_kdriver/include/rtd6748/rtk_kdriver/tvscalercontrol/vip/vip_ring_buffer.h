#ifndef _VIP_RINGBUFFER_H_
#define _VIP_RINGBUFFER_H_

/*******************************************************************************
 * Header files
*******************************************************************************/

/*******************************************************************************
 * Definition
*******************************************************************************/
#define VIP_RINGBUF_NAME_STRLEN (32)

/*******************************************************************************
 * Structure
*******************************************************************************/
enum {
	/* 0 */ VIP_RINGBUF_PTR_READ = 0,
	/* 1 */ VIP_RINGBUF_PTR_WRITE,
	/* 2 */ VIP_RINGBUF_PTR_MAX
};

typedef struct {
	unsigned char Length;
	const char *Name;
} VIP_EASY_RING_BUF_INIT_T;

typedef struct {
	// lock
	unsigned long _RINGBUFFlags;
	spinlock_t Lock;
	// ring-buffer members
	unsigned char BufLength;
	unsigned char ReadPtr;
	unsigned char WritePtr;
	// info
	unsigned int Num;
	char Name[VIP_RINGBUF_NAME_STRLEN];
	// methods
	char (*FuncInit)(void *, void *);	// initial
	char (*FuncReset)(void *, void *);	// reset status
	unsigned char (*FuncGetIdx)(void *, char);	// get specific index
	char (*FuncSetIdx)(void *, char);	// try to move specific index forward
	unsigned char (*FuncPeekIdx)(void *, char);	// peek the index if move the specific index
	unsigned char (*FuncSetIdx_Force)(void *, char);	// force move the sepecific pointer forward
} VIP_EASY_RING_BUF_T;

/*******************************************************************************
 * Template Functions
*******************************************************************************/
char VipEasyRingBuf_Init(void *pSelf, void *ExParams);
char VipEasyRingBuf_Reset(void *pSelf, void *ExParams);
unsigned char VipEasyRingBuf_GetIdx(void *pSelf, char Type);
char VipEasyRingBuf_SetIdx(void *pSelf, char Type);
unsigned char VipEasyRingBuf_PeekIdx(void *pSelf, char Type);
unsigned char VipEasyRingBuf_SetIdx_Force(void *pSelf, char Type);

#endif