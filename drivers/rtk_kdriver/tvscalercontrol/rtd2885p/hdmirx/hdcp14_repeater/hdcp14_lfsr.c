#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include "hdcp14_lfsr.h"
#include <rtd_log/rtd_module_log.h>

void swap_ll(unsigned long long * in)
{
	u8 *ptr = (uint8_t*)in;
	u8 tmp;
	u8 i = 0;
	for(i = 0; i < 4; i++) {
		tmp = *(ptr + i);
		*(ptr + i) = *(ptr+7-i);
		*(ptr+7-i) = tmp;
	}
}


void do_lfsr0(u32 *lfsr0)
{
	int tmp = *lfsr0;
	/* x13 + x11 + x9 + x5 + 1 */
	int fb = GETBIT(tmp, 12) ^ GETBIT(tmp, 10) ^ GETBIT(tmp, 8) ^ GETBIT(tmp, 4);
	*lfsr0 = ((tmp << 1) | fb) & 0x1fff;
}

void do_lfsr1(u32 *lfsr1)
{
	int tmp = *lfsr1;
	/* x14 + x11 + x10 + x7 + x6 + x4 + 1 */
	int fb = GETBIT(tmp, 13) ^ GETBIT(tmp, 10) ^ GETBIT(tmp, 9) ^ GETBIT(tmp, 6) ^ GETBIT(tmp, 5) ^ GETBIT(tmp, 3);
	*lfsr1 = (tmp << 1 | fb) & 0x3fff;
}

void do_lfsr2(u32 *lfsr2)
{
	int tmp = *lfsr2;
	/* x16 + x15 + x12 + x8 + x7 + x5 + 1 */
	int fb = GETBIT(tmp, 15) ^ GETBIT(tmp, 14) ^ GETBIT(tmp, 11) ^ GETBIT(tmp, 7) ^ GETBIT(tmp, 6) ^ GETBIT(tmp, 4);
	*lfsr2 = (tmp << 1 | fb) & 0xffff;
}

void do_lfsr3(u32 *lfsr3)
{
	u32 tmp = *lfsr3;
	/* x17 + x15 + x11 + x5 + 1 */
	u32 fb = GETBIT(tmp, 16) ^ GETBIT(tmp, 14) ^ GETBIT(tmp, 10) ^ GETBIT(tmp, 4);
	*lfsr3 = (tmp << 1 | fb) & 0x1ffff;
}

u32 do_shuffle(u32 *sh, u32 in, u32 sel)
{
	int A = GETBIT(*sh, 1);
	int B = GETBIT(*sh, 0);
	if (sel == 0) {
		*sh = (GETBIT(B, 0) ? BIT(1) : 0) | in;
		return A;
	}
	else {
		*sh = (GETBIT(in, 0) ? BIT(1) : 0) | A;
		return B;
	}
}

LFSR* lfsr_init(u8 *input)
{
	LFSR* lfsr_handle = NULL;
	unsigned long long in = 0;
	lfsr_handle = (LFSR *)kmalloc(sizeof(LFSR), GFP_ATOMIC);
	if(lfsr_handle == NULL)
		return NULL;
	memcpy(((unsigned char *)&in) + 1, input, 7);
	/* lfsr0 = input bit 6 || input bit [11:0] */
	//rtd_pr_hdcp_info("0x%llx\n", in);
	swap_ll(&in);
	//rtd_pr_hdcp_info("swap 0x%llx\n", in);
	//rtd_pr_hdcp_info("in32 %d\n",GETBIT(in, 32));
	lfsr_handle->lfsr0 = (GETBIT(in, 6) ? 0 : BIT(12)) | (in & 0xfff);
	/* lfsr1 = input bit 18 || input bit [24:12] */
	lfsr_handle->lfsr1 = (GETBIT(in, 18) ? 0 : BIT(13)) | ((in >> 12) & 0x1fff);
	/* lfsr2 = input bit 32 || input bit [39:25] */
	lfsr_handle->lfsr2 = (GETBIT(in, 32) ? 0 : BIT(15)) | ((in >> 25) & 0x7fff);
	/* lfsr3 = input bit 47 || input bit [55:40] */
	lfsr_handle->lfsr3 = (GETBIT(in, 47) ? 0 : BIT(16)) | ((in >> 40) & 0xffff);
	lfsr_handle->sh0 = 0x01;
	lfsr_handle->sh1 = 0x01;
	lfsr_handle->sh2 = 0x01;
	lfsr_handle->sh3 = 0x01;
	lfsr_handle->inited = 1;
	lfsr_handle->clocks = 0;
	return lfsr_handle;
}

u32 do_lfsr(LFSR* lfsr_handle)
{
	u32 combiner_out = 0;
	u32 LFSR_TAP[4][3];
	u32 sh_in = 0;
	u32 sh_out = 0;
	if (lfsr_handle == NULL || lfsr_handle->inited != 1)
		return -1;
	
	//do_lfsr0(&lfsr_handle->lfsr0);
	//do_lfsr1(&lfsr_handle->lfsr1);
	//do_lfsr2(&lfsr_handle->lfsr2);
	//do_lfsr3(&lfsr_handle->lfsr3);
	
	LFSR_TAP[0][0] = GETBIT(lfsr_handle->lfsr0, 3);
	LFSR_TAP[0][1] = GETBIT(lfsr_handle->lfsr0, 7);
	LFSR_TAP[0][2] = GETBIT(lfsr_handle->lfsr0, 12);

	LFSR_TAP[1][0] = GETBIT(lfsr_handle->lfsr1, 4);
	LFSR_TAP[1][1] = GETBIT(lfsr_handle->lfsr1, 8);
	LFSR_TAP[1][2] = GETBIT(lfsr_handle->lfsr1, 13);

	LFSR_TAP[2][0] = GETBIT(lfsr_handle->lfsr2, 5);
	LFSR_TAP[2][1] = GETBIT(lfsr_handle->lfsr2, 9);
	LFSR_TAP[2][2] = GETBIT(lfsr_handle->lfsr2, 15);

	LFSR_TAP[3][0] = GETBIT(lfsr_handle->lfsr3, 5);
	LFSR_TAP[3][1] = GETBIT(lfsr_handle->lfsr3, 11);
	LFSR_TAP[3][2] = GETBIT(lfsr_handle->lfsr3, 16);

	sh_in = LFSR_TAP[0][0] ^ LFSR_TAP[1][0] ^ LFSR_TAP[2][0] ^ LFSR_TAP[3][0];
	if(lfsr_handle->clocks == 6) {
	/*	int i = 0,j = 0;
		for (i = 0; i < 4; i++)
			for (j = 0; j < 3; j++)
				rtd_pr_hdmi_info("LFSR_TAD[%d][%d] = %d\n",i,j,LFSR_TAP[i][j]);
		rtd_pr_hdmi_info("sh_in %d\n",sh_in);*/ 
	}
	sh_out = do_shuffle(&lfsr_handle->sh0, sh_in, LFSR_TAP[0][1]);
	sh_out = do_shuffle(&lfsr_handle->sh1, sh_out, LFSR_TAP[1][1]);
	sh_out = do_shuffle(&lfsr_handle->sh2, sh_out, LFSR_TAP[2][1]);
	sh_out = do_shuffle(&lfsr_handle->sh3, sh_out, LFSR_TAP[3][1]);

	combiner_out = LFSR_TAP[0][2] ^ LFSR_TAP[1][2] ^ LFSR_TAP[2][2] ^ LFSR_TAP[3][2] ^ sh_out;
	
	do_lfsr0(&lfsr_handle->lfsr0);
	do_lfsr1(&lfsr_handle->lfsr1);
	do_lfsr2(&lfsr_handle->lfsr2);
	do_lfsr3(&lfsr_handle->lfsr3);
	
	lfsr_handle->clocks++;
	
	return combiner_out;
}

void lfsr_print(LFSR* lfsr_handle)
{
	rtd_pr_hdcp_info("lfsr0 0x%x\n",lfsr_handle->lfsr0);
	rtd_pr_hdcp_info("lfsr1 0x%x\n",lfsr_handle->lfsr1);
	rtd_pr_hdcp_info("lfsr2 0x%x\n",lfsr_handle->lfsr2);
	rtd_pr_hdcp_info("lfsr3 0x%x\n",lfsr_handle->lfsr3);
	rtd_pr_hdcp_info("sh0 0x%x\n",lfsr_handle->sh0);
	rtd_pr_hdcp_info("sh1 0x%x\n",lfsr_handle->sh1);
	rtd_pr_hdcp_info("sh2 0x%x\n",lfsr_handle->sh2);
	rtd_pr_hdcp_info("sh3 0x%x\n",lfsr_handle->sh3);
}

void lfsr_uninit(LFSR* lfsr_handle)
{
	if (lfsr_handle != NULL)
		kfree(lfsr_handle);
}