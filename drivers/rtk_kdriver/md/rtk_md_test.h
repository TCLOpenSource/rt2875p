#ifndef __RTK_MD_TEST_H__
#define __RTK_MD_TEST_H__

extern void rtk_md_unit_test(void);
extern void rtk_md_memcpy_test(int length);
extern void rtk_md_fill_test(void);
extern void rtk_md_stress_test(int length, int loops);
extern int rtk_multi_vs_signal(unsigned int length, unsigned int number);


#endif