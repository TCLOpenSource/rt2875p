#ifndef __RTICE_AUX_H__
#define __RTICE_AUX_H__

#define B4TL(b0, b1, b2, b3)          \
	(((unsigned long)b0<<24) + ((unsigned long)b1<<16) \
	 + ((unsigned long)b2<<8) + ((unsigned long)b3))
	 
#define B2TS(b0,b1) (((unsigned short)b0<<8) + ((unsigned short)b1) )

#define LTB4(L, b0, b1, b2, b3)        do { b0 = (L >> 24) & 0xFF; \
	b1 = (L >> 16) & 0xFF; \
	b2 = (L >> 8) & 0xFF; \
	b3 = (L & 0xFF); \
	}while (0);

#define LTB8(L, b0, b1, b2, b3, b4, b5, b6, b7)\
	do { b0 = (L >> 56) & 0xFF; \
	b1 = (L >> 48) & 0xFF; \
	b2 = (L >> 40) & 0xFF; \
	b3 = (L >> 32) & 0xFF; \
	b4 = (L >> 24) & 0xFF; \
	b5 = (L >> 16) & 0xFF; \
	b6 = (L >> 8) & 0xFF; \
	b7 = (L & 0xFF); \
	}while (0);

#define STB2(S, b0, b1)        do { b0 = (S >> 8) & 0xFF; \
	b1 = (S & 0xFF); \
	}while (0);

extern void rtice_print_array(unsigned char*fmt,unsigned char*pData,\
							unsigned int len);
extern void rtice_print_string(unsigned char *format, ...);

#define rtice_print_array_with_text(pData, len, fmt, args...)    \
	do { rtd_pr_rtice_debug(fmt, ##args);  \
	rtice_print_array(pData, len); } while(0)
	
extern unsigned char rtice_checksum(unsigned char *pData, unsigned int len);
extern void uart1_enable(void);

#endif	/*  */
