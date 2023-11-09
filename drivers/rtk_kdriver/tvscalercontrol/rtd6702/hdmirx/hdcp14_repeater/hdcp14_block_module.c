#include <linux/kernel.h>
#include <linux/module.h>
#include "hdcp14_api.h"
#include "hdcp14_block_module.h"


static u8 s_sk_boxes[7][16] =
{
	{8, 14, 5, 9, 3, 0, 12, 6, 1, 11, 15, 2, 4, 7, 10, 13},
	{1, 6, 4, 15, 8, 3, 11, 5, 10, 0, 9, 12, 7, 13, 14, 2},
	{13, 11, 8, 6, 7, 4, 2, 15, 1, 12, 14, 0, 10, 3, 9, 5},
	{0, 14, 11, 7, 12, 3, 2, 13, 15, 4, 8, 1, 9, 10, 5, 6},
	{12, 7, 15, 8, 11, 14, 1, 4, 6, 10, 3, 5, 0, 9, 13, 2},
	{1, 12, 7, 2, 8, 3, 4, 14, 11, 5, 0, 15, 13, 6, 10, 9},
	{10, 7, 6, 1, 0, 14, 3, 13, 12, 9, 11, 2, 15, 5, 4, 8},
};

static u8 s_sb_boxes[7][16] =
{
	{12, 9, 3, 0, 11, 5, 13, 6, 2, 4, 14, 7, 8, 15, 1, 10},
	{3, 8, 14, 1, 5, 2, 11, 13, 10, 4, 9, 7, 6, 15, 12, 0},
	{7, 4, 1, 10, 11, 13, 14, 3, 12, 15, 6, 0, 2, 8, 9, 5},
	{6, 3, 1, 4, 10, 12, 15, 2, 5, 14, 11, 8, 9, 7, 0, 13},
	{3, 6, 15, 12, 4, 1, 9, 2, 5, 8, 10, 7, 11, 13, 0, 14},
	{11, 14, 6, 8, 5, 2, 12, 7, 1, 4, 15, 3, 10, 13, 9, 0},
	{1, 11, 7, 4, 2, 5, 12, 9, 13, 6, 8, 15, 14, 0, 3, 10},	
};


u32 do_s_k_box(u32 kx)
{
	u32 ret = 0;
	u8 media_value[7];
	int i = 0;
	//rtd_pr_hdcp_info("%x\n", kx);
	for(i = 0; i < 7; i++) {
		u8 tmp  = ((kx >> i) & 0x1) | (((kx >> (7 + i)) & 0x1) << 1) 
					| (((kx >> (14 + i)) & 0x1) << 2) | (((kx >> (21 + i)) & 0x1) << 3);
		media_value[i] = s_sk_boxes[i][tmp];
		//rtd_pr_hdcp_info("%d, %d, %d\n", i, tmp, media_value[i]);		
	}

	ret = ((media_value[0] & 0x1) << 0) | ((media_value[0] & 0x2) << 6) 
					| ((media_value[0] & 0x4) << 12) | ((media_value[0] & 0x8) << 18)
          | ((media_value[1] & 0x1) << 1)  | ((media_value[1] & 0x2) << 7) 
					| ((media_value[1] & 0x4) << 13) | ((media_value[1] & 0x8) << 19)
	      | ((media_value[2] & 0x1) << 2)  | ((media_value[2] & 0x2) << 8) 
					| ((media_value[2] & 0x4) << 14) | ((media_value[2] & 0x8) << 20)
	      | ((media_value[3] & 0x1) << 3)  | ((media_value[3] & 0x2) << 9) 
					| ((media_value[3] & 0x4) << 15) | ((media_value[3] & 0x8) << 21)
	      | ((media_value[4] & 0x1) << 4)  | ((media_value[4] & 0x2) << 10) 
					| ((media_value[4] & 0x4) << 16) | ((media_value[4] & 0x8) << 22)
	      | ((media_value[5] & 0x1) << 5)  | ((media_value[5] & 0x2) << 11) 
					| ((media_value[5] & 0x4) << 17) | ((media_value[5] & 0x8) << 23)
	      | ((media_value[6] & 0x1) << 6)  | ((media_value[6] & 0x2) << 12) 
					| ((media_value[6] & 0x4) << 18) | ((media_value[6] & 0x8) << 24);
		//rtd_pr_hdcp_info("ret %x\n", ret);	//0x7f722dc	
	return ret;
}

u32 do_s_b_box(u32 bx)
{
	u32 ret = 0;
	u8 media_value[7];
	int i = 0;
	for(i = 0; i < 7; i++) {
		u8 tmp  = ((bx >> i) & 0x1) | (((bx >> (7 + i)) & 0x1) << 1) 
					| (((bx >> (14 + i)) & 0x1) << 2) | (((bx >> (21 + i)) & 0x1) << 3);
		media_value[i] = s_sb_boxes[i][tmp];
		//rtd_pr_hdcp_info("%d, %d, %d\n", i, tmp, media_value[i]);		
	}

	ret = ((media_value[0] & 0x1) << 0) | ((media_value[0] & 0x2) << 6) 
					| ((media_value[0] & 0x4) << 12) | ((media_value[0] & 0x8) << 18)
          | ((media_value[1] & 0x1) << 1)  | ((media_value[1] & 0x2) << 7) 
					| ((media_value[1] & 0x4) << 13) | ((media_value[1] & 0x8) << 19)
	      | ((media_value[2] & 0x1) << 2)  | ((media_value[2] & 0x2) << 8) 
					| ((media_value[2] & 0x4) << 14) | ((media_value[2] & 0x8) << 20)
	      | ((media_value[3] & 0x1) << 3)  | ((media_value[3] & 0x2) << 9) 
					| ((media_value[3] & 0x4) << 15) | ((media_value[3] & 0x8) << 21)
	      | ((media_value[4] & 0x1) << 4)  | ((media_value[4] & 0x2) << 10) 
					| ((media_value[4] & 0x4) << 16) | ((media_value[4] & 0x8) << 22)
	      | ((media_value[5] & 0x1) << 5)  | ((media_value[5] & 0x2) << 11) 
					| ((media_value[5] & 0x4) << 17) | ((media_value[5] & 0x8) << 23)
	      | ((media_value[6] & 0x1) << 6)  | ((media_value[6] & 0x2) << 12) 
					| ((media_value[6] & 0x4) << 18) | ((media_value[6] & 0x8) << 24);
		//rtd_pr_hdcp_info("ret %x\n", ret);	//0x7f722dc	
	return ret;
	
}

#define diffusion_network_k(I0, I1, I2, I3, I4, I5, I6, O0, O1, O2, O3, O4, O5, O6) do { \
	O0 = (I1 ^ I2 ^ I3 ^ I4 ^ I5 ^ I6); \
	O1 = (I0 ^ I2 ^ I3 ^ I4 ^ I5 ^ I6); \
	O2 = (I0 ^ I1 ^ I3 ^ I4 ^ I5 ^ I6); \
	O3 = (I0 ^ I1 ^ I2 ^ I4 ^ I5 ^ I6); \
	O4 = (I0 ^ I1 ^ I2 ^ I3 ^ I5 ^ I6); \
	O5 = (I0 ^ I1 ^ I2 ^ I3 ^ I4 ^ I6); \
	O6 = (I0 ^ I1 ^ I2 ^ I3 ^ I4 ^ I5 ^ I6); \
}while(0)

#define diffusion_network_b(I0, I1, I2, I3, I4, I5, I6, K0, K1, K2, K3, K4, K5, K6, O0, O1, O2, O3, O4, O5, O6) do { \
	O0 = (K0 ^ I1 ^ I2 ^ I3 ^ I4 ^ I5 ^ I6); \
	O1 = (K1 ^ I0 ^ I2 ^ I3 ^ I4 ^ I5 ^ I6); \
	O2 = (K2 ^ I0 ^ I1 ^ I3 ^ I4 ^ I5 ^ I6); \
	O3 = (K3 ^ I0 ^ I1 ^ I2 ^ I4 ^ I5 ^ I6); \
	O4 = (K4 ^ I0 ^ I1 ^ I2 ^ I3 ^ I5 ^ I6); \
	O5 = (K5 ^ I0 ^ I1 ^ I2 ^ I3 ^ I4 ^ I6); \
	O6 = (K6 ^ I0 ^ I1 ^ I2 ^ I3 ^ I4 ^ I5 ^ I6); \
}while(0)


void linear_transformation_k(u32 ky, u32 kz, u32 *kxo, u32 *kyo)
{
	u8 ky_bits[28] = {0};
	u8 kz_bits[28] = {0};	
	u8 kx_out_bits[28] = {0};
	u8 ky_out_bits[28] = {0};
	int i = 0;
	for(i = 0; i < 28; i++) {
		ky_bits[i] = (ky >> i) & 0x1;
		kz_bits[i] = (kz >> i) & 0x1;
	}

	/*K1*/
	diffusion_network_k(kz_bits[0],kz_bits[1],kz_bits[2],kz_bits[3],
 	        kz_bits[4],kz_bits[5],kz_bits[6],
	        kx_out_bits[0], kx_out_bits[4], kx_out_bits[8], kx_out_bits[12],
			kx_out_bits[16], kx_out_bits[20], kx_out_bits[24]);
		
	/*K2*/		
	diffusion_network_k(kz_bits[7],kz_bits[8],kz_bits[9],ky_bits[0],
 	        ky_bits[1],ky_bits[2],ky_bits[12],
	        ky_out_bits[0], ky_out_bits[4], ky_out_bits[8], ky_out_bits[12],
			ky_out_bits[16], ky_out_bits[20], ky_out_bits[24]);	
	/*K3*/		
	diffusion_network_k(kz_bits[10],kz_bits[11],kz_bits[12],ky_bits[3],
 	        ky_bits[4],ky_bits[5],ky_bits[13],
	        ky_out_bits[1], ky_out_bits[5], ky_out_bits[9], ky_out_bits[13],
			ky_out_bits[17], ky_out_bits[21], ky_out_bits[25]);	
	/*K4*/		
	diffusion_network_k(kz_bits[13],kz_bits[14],kz_bits[15],ky_bits[6],
 	        ky_bits[7],ky_bits[8],ky_bits[14],
	        ky_out_bits[2], ky_out_bits[6], ky_out_bits[10], ky_out_bits[14],
			ky_out_bits[18], ky_out_bits[22], ky_out_bits[26]);	
	/*K5*/		
	diffusion_network_k(kz_bits[16],kz_bits[17],kz_bits[18],ky_bits[9],
 	        ky_bits[10],ky_bits[11],ky_bits[15],
	        ky_out_bits[3], ky_out_bits[7], ky_out_bits[11], ky_out_bits[15],
			ky_out_bits[19], ky_out_bits[23], ky_out_bits[27]);	
	/*K6*/		
	diffusion_network_k(ky_bits[16],ky_bits[17],ky_bits[18],ky_bits[19],
 	        kz_bits[19],kz_bits[20],kz_bits[21],
	        kx_out_bits[1], kx_out_bits[5], kx_out_bits[9], kx_out_bits[13],
			kx_out_bits[17], kx_out_bits[21], kx_out_bits[25]);	
	/*K7*/		
	diffusion_network_k(ky_bits[20],ky_bits[21],ky_bits[22],ky_bits[23],
 	        kz_bits[22],kz_bits[23],kz_bits[24],
	        kx_out_bits[2], kx_out_bits[6], kx_out_bits[10], kx_out_bits[14],
			kx_out_bits[18], kx_out_bits[22], kx_out_bits[26]);	
	/*K8*/		
	diffusion_network_k(ky_bits[24],ky_bits[25],ky_bits[26],ky_bits[27],
 	        kz_bits[25],kz_bits[26],kz_bits[27],
	        kx_out_bits[3], kx_out_bits[7], kx_out_bits[11], kx_out_bits[15],
			kx_out_bits[19], kx_out_bits[23], kx_out_bits[27]);	
	*kxo = 0;
	*kyo = 0;
	for(i = 0; i < 28; i++) {
		*kxo = *kxo | ((kx_out_bits[i] & 0x1) << i);
		*kyo = *kyo | ((ky_out_bits[i] & 0x1) << i);
	}			
																				
} 


static void linear_transformation_b(u32 by, u32 bz,u32 ky, u32 *bxo, u32 *byo)
{
	u8 by_bits[28] = {0};
	u8 bz_bits[28] = {0};	
	u8 ky_bits[28] = {0};	
	u8 bx_out_bits[28] = {0};
	u8 by_out_bits[28] = {0};
	int i = 0;
	for(i = 0; i < 28; i++) {
		by_bits[i] = (by >> i) & 0x1;
		bz_bits[i] = (bz >> i) & 0x1;
		ky_bits[i] = (ky >> i) & 0x1;
	}

	/*B1*/
	diffusion_network_b(bz_bits[0],bz_bits[1],bz_bits[2],bz_bits[3],
 	        bz_bits[4],bz_bits[5],bz_bits[6],
 	        ky_bits[0], ky_bits[1], ky_bits[2], ky_bits[3],
 	        ky_bits[4], ky_bits[5], ky_bits[6],
	        bx_out_bits[0], bx_out_bits[4], bx_out_bits[8], bx_out_bits[12],
			bx_out_bits[16], bx_out_bits[20], bx_out_bits[24]);
		
	/*B2*/		
	diffusion_network_k(bz_bits[7],bz_bits[8],bz_bits[9],by_bits[0],
 	        by_bits[1],by_bits[2],by_bits[12],
	        by_out_bits[0], by_out_bits[4], by_out_bits[8], by_out_bits[12],
			by_out_bits[16], by_out_bits[20], by_out_bits[24]);	
	/*B3*/		
	diffusion_network_k(bz_bits[10],bz_bits[11],bz_bits[12],by_bits[3],
 	        by_bits[4],by_bits[5],by_bits[13],
	        by_out_bits[1], by_out_bits[5], by_out_bits[9], by_out_bits[13],
			by_out_bits[17], by_out_bits[21], by_out_bits[25]);	
	/*B4*/		
	diffusion_network_k(bz_bits[13],bz_bits[14],bz_bits[15],by_bits[6],
 	        by_bits[7],by_bits[8],by_bits[14],
	        by_out_bits[2], by_out_bits[6], by_out_bits[10], by_out_bits[14],
			by_out_bits[18], by_out_bits[22], by_out_bits[26]);	
	/*B5*/		
	diffusion_network_k(bz_bits[16],bz_bits[17],bz_bits[18],by_bits[9],
 	        by_bits[10],by_bits[11],by_bits[15],
	        by_out_bits[3], by_out_bits[7], by_out_bits[11], by_out_bits[15],
			by_out_bits[19], by_out_bits[23], by_out_bits[27]);	
	/*B6*/		
	diffusion_network_b(by_bits[16],by_bits[17],by_bits[18],by_bits[19],
 	        bz_bits[19],bz_bits[20],bz_bits[21],
 	        
 	        ky_bits[7], ky_bits[8], ky_bits[9], ky_bits[10],
 	        ky_bits[11], ky_bits[12], ky_bits[13],
 	        
	        bx_out_bits[1], bx_out_bits[5], bx_out_bits[9], bx_out_bits[13],
			bx_out_bits[17], bx_out_bits[21], bx_out_bits[25]);	
	/*B7*/		
	diffusion_network_b(by_bits[20],by_bits[21],by_bits[22],by_bits[23],
 	        bz_bits[22],bz_bits[23],bz_bits[24],
 	        
 	        ky_bits[14], ky_bits[15], ky_bits[16], ky_bits[17],
 	        ky_bits[18], ky_bits[19], ky_bits[20],
 	        
	        bx_out_bits[2], bx_out_bits[6], bx_out_bits[10], bx_out_bits[14],
			bx_out_bits[18], bx_out_bits[22], bx_out_bits[26]);	
	/*K8*/		
	diffusion_network_b(by_bits[24],by_bits[25],by_bits[26],by_bits[27],
 	        bz_bits[25],bz_bits[26],bz_bits[27],
 	        
 	        ky_bits[21], ky_bits[22], ky_bits[23], ky_bits[24],
 	        ky_bits[25], ky_bits[26], ky_bits[27],
 	        
	        bx_out_bits[3], bx_out_bits[7], bx_out_bits[11], bx_out_bits[15],
			bx_out_bits[19], bx_out_bits[23], bx_out_bits[27]);	
	*bxo = 0;
	*byo = 0;
	for(i = 0; i < 28; i++) {
		*bxo = *bxo | ((bx_out_bits[i] & 0x1) << i);
		*byo = *byo | ((by_out_bits[i] & 0x1) << i);
	}			
																				
}

void hdcp_1_4_round_function_b(u32 bx, u32 by, u32 bz, u32 ky,
				u32 *bxo, u32 *byo, u32 *bzo)
{
	*bzo = do_s_b_box(bx);
	linear_transformation_b(by, bz, ky, bxo, byo);		
} 

void hdcp_1_4_round_function_k(u32 kx, u32 ky, u32 kz,
				u32 *kxo, u32 *kyo, u32 *kzo)
{
	*kzo = do_s_k_box(kx);
	linear_transformation_k(ky, kz, kxo, kyo);		
} 


u32 hdcp_1_4_output_function(u32 ky, u32 kz, u32 by, u32 bz)
{
	u8 ky_bits[28] = {0};
	u8 kz_bits[28] = {0};	
	u8 by_bits[28] = {0};	
	u8 bz_bits[28] = {0};
	u8 out_bits[24] = {0};
	u32 output = 0; 
	int i = 0;
	
	for(i = 0; i < 28; i++) {
		ky_bits[i] = (ky >> i) & 0x1;
		kz_bits[i] = (kz >> i) & 0x1;
		by_bits[i] = (by >> i) & 0x1;
		bz_bits[i] = (bz >> i) & 0x1;
	}
	
	out_bits[0] = (bz_bits[17] & kz_bits[3])^(bz_bits[26] & kz_bits[6]) ^ (bz_bits[22] & kz_bits[0]) ^ (bz_bits[27] & kz_bits[9]) ^ (bz_bits[21] & kz_bits[4]) ^ (bz_bits[18] & kz_bits[22]) ^ (bz_bits[2] & kz_bits[5]) ^ by_bits[5] ^ ky_bits[10];
	out_bits[1] = (bz_bits[5] & kz_bits[20])^(bz_bits[20] & kz_bits[18]) ^ (bz_bits[15] & kz_bits[7]) ^ (bz_bits[24] & kz_bits[23]) ^ (bz_bits[2] & kz_bits[15]) ^ (bz_bits[25] & kz_bits[5]) ^ (bz_bits[0] & kz_bits[3]) ^ by_bits[16] ^ ky_bits[25];
	out_bits[2] = (bz_bits[22] & kz_bits[7])^(bz_bits[5] & kz_bits[19]) ^ (bz_bits[14] & kz_bits[2]) ^ (bz_bits[16] & kz_bits[10]) ^ (bz_bits[25] & kz_bits[22]) ^ (bz_bits[17] & kz_bits[4]) ^ (bz_bits[20] & kz_bits[13]) ^ by_bits[11] ^ ky_bits[21];
	out_bits[3] = (bz_bits[19] & kz_bits[6])^(bz_bits[3] & kz_bits[14]) ^ (bz_bits[15] & kz_bits[9]) ^ (bz_bits[11] & kz_bits[8]) ^ (bz_bits[21] & kz_bits[17]) ^ (bz_bits[16] & kz_bits[18]) ^ (bz_bits[27] & kz_bits[12]) ^ by_bits[1] ^ ky_bits[24];
	out_bits[4] = (bz_bits[19] & kz_bits[25])^(bz_bits[6] & kz_bits[6]) ^ (bz_bits[17] & kz_bits[5]) ^ (bz_bits[18] & kz_bits[2]) ^ (bz_bits[22] & kz_bits[10]) ^ (bz_bits[7] & kz_bits[15]) ^ (bz_bits[9] & kz_bits[21]) ^ by_bits[12] ^ ky_bits[8];
	out_bits[5] = (bz_bits[3] & kz_bits[27])^(bz_bits[7] & kz_bits[14]) ^ (bz_bits[4] & kz_bits[2]) ^ (bz_bits[8] & kz_bits[4]) ^ (bz_bits[16] & kz_bits[24]) ^ (bz_bits[6] & kz_bits[19]) ^ (bz_bits[5] & kz_bits[1]) ^ by_bits[17] ^ ky_bits[12];
	out_bits[6] = (bz_bits[8] & kz_bits[17])^(bz_bits[21] & kz_bits[26]) ^ (bz_bits[27] & kz_bits[4]) ^ (bz_bits[2] & kz_bits[16]) ^ (bz_bits[11] & kz_bits[27]) ^ (bz_bits[24] & kz_bits[7]) ^ (bz_bits[12] & kz_bits[22]) ^ by_bits[3] ^ ky_bits[11];
	out_bits[7] = (bz_bits[9] & kz_bits[9])^(bz_bits[5] & kz_bits[10]) ^ (bz_bits[7] & kz_bits[19]) ^ (bz_bits[4] & kz_bits[11]) ^ (bz_bits[8] & kz_bits[7]) ^ (bz_bits[13] & kz_bits[6]) ^ (bz_bits[3] & kz_bits[8]) ^ by_bits[15] ^ ky_bits[23];
	out_bits[8] = (bz_bits[26] & kz_bits[13])^(bz_bits[13] & kz_bits[12]) ^ (bz_bits[23] & kz_bits[18]) ^ (bz_bits[10] & kz_bits[24]) ^ (bz_bits[11] & kz_bits[15]) ^ (bz_bits[7] & kz_bits[23]) ^ (bz_bits[15] & kz_bits[7]) ^ by_bits[19] ^ ky_bits[16];
	out_bits[9] = (bz_bits[1] & kz_bits[0])^(bz_bits[0] & kz_bits[5]) ^ (bz_bits[19] & kz_bits[20]) ^ (bz_bits[11] & kz_bits[25]) ^ (bz_bits[13] & kz_bits[1]) ^ (bz_bits[16] & kz_bits[24]) ^ (bz_bits[24] & kz_bits[9]) ^ by_bits[18] ^ ky_bits[27];
	out_bits[10] = (bz_bits[26] & kz_bits[14])^(bz_bits[13] & kz_bits[23]) ^ (bz_bits[9] & kz_bits[27]) ^ (bz_bits[14] & kz_bits[25]) ^ (bz_bits[10] & kz_bits[17]) ^ (bz_bits[4] & kz_bits[19]) ^ (bz_bits[1] & kz_bits[1]) ^ by_bits[2] ^ ky_bits[22];
	out_bits[11] = (bz_bits[21] & kz_bits[6])^(bz_bits[15] & kz_bits[21]) ^ (bz_bits[5] & kz_bits[17]) ^ (bz_bits[3] & kz_bits[15]) ^ (bz_bits[13] & kz_bits[26]) ^ (bz_bits[25] & kz_bits[11]) ^ (bz_bits[16] & kz_bits[16]) ^ by_bits[27] ^ ky_bits[7];
	out_bits[12] = (bz_bits[20] & kz_bits[11])^(bz_bits[7] & kz_bits[22]) ^ (bz_bits[18] & kz_bits[20]) ^ (bz_bits[12] & kz_bits[0]) ^ (bz_bits[17] & kz_bits[26]) ^ (bz_bits[1] & kz_bits[23]) ^ (bz_bits[16] & kz_bits[17]) ^ by_bits[0] ^ ky_bits[2];
	out_bits[13] = (bz_bits[14] & kz_bits[8])^(bz_bits[23] & kz_bits[4]) ^ (bz_bits[1] & kz_bits[3]) ^ (bz_bits[12] & kz_bits[14]) ^ (bz_bits[24] & kz_bits[20]) ^ (bz_bits[6] & kz_bits[26]) ^ (bz_bits[18] & kz_bits[23]) ^ by_bits[9] ^ ky_bits[15];
	out_bits[14] = (bz_bits[19] & kz_bits[19])^(bz_bits[6] & kz_bits[0]) ^ (bz_bits[21] & kz_bits[18]) ^ (bz_bits[25] & kz_bits[2]) ^ (bz_bits[23] & kz_bits[13]) ^ (bz_bits[1] & kz_bits[8]) ^ (bz_bits[10] & kz_bits[24]) ^ by_bits[8] ^ ky_bits[14];
	out_bits[15] = (bz_bits[3] & kz_bits[16])^(bz_bits[0] & kz_bits[21]) ^ (bz_bits[27] & kz_bits[24]) ^ (bz_bits[23] & kz_bits[25]) ^ (bz_bits[19] & kz_bits[12]) ^ (bz_bits[8] & kz_bits[27]) ^ (bz_bits[4] & kz_bits[15]) ^ by_bits[7] ^ ky_bits[18];
	out_bits[16] = (bz_bits[6] & kz_bits[3])^(bz_bits[5] & kz_bits[5]) ^ (bz_bits[14] & kz_bits[8]) ^ (bz_bits[22] & kz_bits[25]) ^ (bz_bits[24] & kz_bits[7]) ^ (bz_bits[18] & kz_bits[27]) ^ (bz_bits[2] & kz_bits[2]) ^ by_bits[21] ^ ky_bits[26];
	out_bits[17] = (bz_bits[3] & kz_bits[11])^(bz_bits[4] & kz_bits[14]) ^ (bz_bits[2] & kz_bits[23]) ^ (bz_bits[6] & kz_bits[17]) ^ (bz_bits[22] & kz_bits[22]) ^ (bz_bits[14] & kz_bits[13]) ^ (bz_bits[12] & kz_bits[19]) ^ by_bits[26] ^ ky_bits[4];
	out_bits[18] = (bz_bits[25] & kz_bits[1])^(bz_bits[21] & kz_bits[16]) ^ (bz_bits[19] & kz_bits[14]) ^ (bz_bits[9] & kz_bits[11]) ^ (bz_bits[10] & kz_bits[12]) ^ (bz_bits[15] & kz_bits[6]) ^ (bz_bits[13] & kz_bits[10]) ^ by_bits[22] ^ ky_bits[19];
	out_bits[19] = (bz_bits[23] & kz_bits[21])^(bz_bits[11] & kz_bits[1]) ^ (bz_bits[10] & kz_bits[10]) ^ (bz_bits[20] & kz_bits[20]) ^ (bz_bits[1] & kz_bits[18]) ^ (bz_bits[12] & kz_bits[26]) ^ (bz_bits[14] & kz_bits[9]) ^ by_bits[4] ^ ky_bits[13];
	out_bits[20] = (bz_bits[11] & kz_bits[20])^(bz_bits[26] & kz_bits[21]) ^ (bz_bits[20] & kz_bits[9]) ^ (bz_bits[17] & kz_bits[25]) ^ (bz_bits[8] & kz_bits[12]) ^ (bz_bits[23] & kz_bits[3]) ^ (bz_bits[0] & kz_bits[15]) ^ by_bits[24] ^ ky_bits[0];
	out_bits[21] = (bz_bits[9] & kz_bits[18])^(bz_bits[17] & kz_bits[12]) ^ (bz_bits[26] & kz_bits[21]) ^ (bz_bits[4] & kz_bits[27]) ^ (bz_bits[27] & kz_bits[1]) ^ (bz_bits[0] & kz_bits[16]) ^ (bz_bits[15] & kz_bits[24]) ^ by_bits[6] ^ ky_bits[20];
	out_bits[22] = (bz_bits[22] & kz_bits[13])^(bz_bits[12] & kz_bits[0]) ^ (bz_bits[2] & kz_bits[3]) ^ (bz_bits[10] & kz_bits[16]) ^ (bz_bits[7] & kz_bits[22]) ^ (bz_bits[20] & kz_bits[11]) ^ (bz_bits[25] & kz_bits[26]) ^ by_bits[13] ^ ky_bits[9];
	out_bits[23] = (bz_bits[27] & kz_bits[2])^(bz_bits[24] & kz_bits[0]) ^ (bz_bits[26] & kz_bits[13]) ^ (bz_bits[8] & kz_bits[5]) ^ (bz_bits[0] & kz_bits[4]) ^ (bz_bits[9] & kz_bits[8]) ^ (bz_bits[18] & kz_bits[10]) ^ by_bits[23] ^ ky_bits[3];
	for(i = 0; i < 24; i++) {
		output = output | ((out_bits[i] & 0x1) << i);
	}
	return output;
	
} 