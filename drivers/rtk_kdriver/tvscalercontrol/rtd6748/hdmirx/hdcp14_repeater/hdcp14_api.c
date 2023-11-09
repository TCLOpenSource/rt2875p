#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include "hdcp14_api.h"
#include "hdcp14_block_module.h"
#include "hdcp14_lfsr.h"
#include "sha1.h"
#include <rtd_log/rtd_module_log.h>

/*
* Parameters:
*  ksv:key select verctors, should 5Bytes
*  keys: hdcp1.4 key, should be 320Bytes
* Output:
*  km:the result of km, should be 7Bytes
*   
*/
void hdcp_dump_char_array(char *name, unsigned char *data, unsigned int len)
{
	int i = 0;
	int buf_len = len * 6;
	char *buf = (char *)kmalloc(buf_len, GFP_ATOMIC);
       if(!buf)
           return;
	if( !data || !len) 
	    return;
    
	buf[0] = 0;
	for(i = 0; i < len; i++) {
		snprintf(buf + strlen(buf), buf_len - strlen(buf), "%02x", data[i]);
		if(((i + 1) % 32) == 0) {
			snprintf(buf + strlen(buf), buf_len - strlen(buf), "%s", "\n");
		}
	}
	if(name)
            rtd_pr_hdcp_info("%s-%d: %s", name, len, buf);
      else 
            rtd_pr_hdcp_info("%s\n", buf);
	
}

static void uint64_to_char_array(u64 value, u8 *array, u32 len, u8 is_big_endian)
{
	int i = 0;
	if(!array || !len || len > 8)
		return;
	memset(array, 0, len);
//	rtd_pr_hdcp_info("%llx\n", value);
	if(is_big_endian) {
		for(i = len - 1; i >= 0; i--) {
			
			array[i] = (value & 0xFF);
			value = (value >> 8);
//			rtd_pr_hdcp_info("%d, %02x, %llx\n", i, array[i], value);
		}	
	} else {
		for(i = 0; i < len; i++) {
			array[i] = (value & 0xFF);
			value = (value >> 8);
		}
	}	
}

static u64 char_array_to_uint64(u8 *array, u32 len, u8 is_big_endian)
{
	u64 value = 0;
	int i = 0;
	if(!array || !len || len > 8)
		return 0;
	if(is_big_endian) {
		for(i = 0; i < len; i++) {
			value = (value << 8);
			value = (value | array[i]); 
			
		}	
	} else {
		for(i = len - 1; i >= 0; i--) {
			value = (value << 8);
			value = (value | array[i]); 
			
		}
	}	
	return value;
}

static void uint32_to_char_array(u32 value, u8 *array, u32 len, u8 is_big_endian)
{
	int i = 0;
	if(!array || !len || len > 4)
		return;
	memset(array, 0, len);
	
	if(is_big_endian) {
		for(i = len - 1; i >= 0; i--) {
			array[i] = (value & 0xFF);
			value = (value >> 8);
		}	
	} else {
		for(i = 0; i < len; i++) {
			array[i] = (value & 0xFF);
			value = (value >> 8);
		}
	}	
}

static u32 char_array_to_uint32(u8 *array, u32 len, u8 is_big_endian)
{
	u32 value = 0;
	int i = 0;
	if(!array || !len || len > 4)
		return 0;
	if(is_big_endian) {
		for(i = 0; i < len; i++) {
			value = (value << 8);
			value = (value | array[i]); 
			
		}	
	} else {
		for(i = len - 1; i >= 0; i--) {
			value = (value << 8);
			value = (value | array[i]); 
			
		}
	}	
	return value;
}

bool hdcp14_compute_km(u8 *keys, u8 *ksv, u8 *km)
{
	u64 km_value = 0;
	int i = 0;
	int j = 0;
	bool ret = false;
	u8 is_big_endian = IS_BIG_ENDIAN;

	if(!ksv || !keys || !km)
		return ret;

	if (is_big_endian)
    {
		for(i = 4; i >= 0; i--) 
		{
			u8 ks = ksv[i];
			
			for(j = 0; j < 8; j++) 
			{
				if((ks & 0x1)) 
                {
					u8 *ptr = keys + 7 * ((4 - i) * 8 + j);
					u64 key = 	char_array_to_uint64(ptr, 7, is_big_endian);
					//rtd_pr_hdcp_info("%02x%02x%02x%02x%02x%02x%02x%02x\n", ptr[0], ptr[1], ptr[2], ptr[3],
					//ptr[4], ptr[5], ptr[6], ptr[7]);
					//rtd_pr_hdcp_info("key[%d] == %llx\n", ((4 - i) * 8 + j), key);
					//key = (key & 0x00FFFFFFFFFFFFFF);
					km_value += key;
					//km_value = (km_value & 0x00FFFFFFFFFFFFFF);
				}
				ks = (ks >> 1);
			}
		}
	} else {
		for(i = 0; i < 5; i++) {
			u8 ks = ksv[i];
			for(j = 0; j < 8; j++) {
				if((ks & 0x1)) {
					u8 *ptr = keys + 7 * (i * 8 + j);
					u64 key = 	char_array_to_uint64(ptr, 7, is_big_endian);
					rtd_pr_hdcp_info("[HDCP] key[%03d] == %llx\n", 8 * (i * 8 + j), key);
					//key = (key & 0x00FFFFFFFFFFFFFF);
					km_value += key;
					//km_value = (km_value & 0x00FFFFFFFFFFFFFF);
				}
				ks = (ks >> 1);
			}
		}
	}
	
	uint64_to_char_array(km_value, km, 7, is_big_endian);
	rtd_pr_hdcp_info("[HDCP] km_value == %llx\n", km_value);
	ret = true;
	return ret;
}

static bool do_sha1(unsigned char *input, unsigned int len, unsigned char *hash)
{
	SHA1Context sha1ctx;
	SHA1Reset(&sha1ctx);
	SHA1Input(&sha1ctx, input, len);
	SHA1Result(&sha1ctx, hash);
	return true;
}

bool hdcp14_compute_v(u8 *ksv_list, u32 list_len, 
				u8 *bstatus, u8 *m0, u8 *v)
{
	bool ret = false;
	u8 *msg = NULL;
	u32 msg_len = 0;
	int ksv_cnt = list_len / 5; 
	int i = 0;
	int j = 0;
	u32 index = 0;
	u8 is_big_endian = IS_BIG_ENDIAN;
	
	if(!ksv_list && list_len)
		return false;
	if((list_len % 5) != 0)
		return false;
		
	if(!bstatus || !m0 || !v)
		return false;
	msg_len = list_len + 2 + 8;
	msg = (u8 *)kmalloc(msg_len, GFP_KERNEL);
	if(!msg)
		return false;

      if(ksv_list) {
        	for(i = 0; i < ksv_cnt; i++) {
        		u8 *ptr = ksv_list + i * 5;
        		if(is_big_endian) {
        			for(j = 4; j >= 0; j--) {
        				//rtd_pr_hdcp_info("%02x\n", ptr[j]);
        				msg[index++] = ptr[j]; 
        			} 		
        		} else {
        			memcpy(msg + index, ptr, 5);
        			index += 5;
        		}	
        	}	
       }
	
	if(is_big_endian) {
		msg[index++] = bstatus[1];
		msg[index++] = bstatus[0];
	} else {
		memcpy(msg + index, bstatus, 2);
		index += 2;
	}

	if(is_big_endian) {
		for(j = 7; j >= 0; j--) {
			msg[index++] = m0[j]; 
		} 
	} else {
		memcpy(msg + index, m0, 8);
		index += 8;
	}	
	
	hdcp_dump_char_array("[HDCP]hdcp_1_4_compute_v sha1 input", msg, index);
	
	ret = do_sha1(msg, msg_len, v);
	hdcp_dump_char_array("[HDCP] v", v, 20);
	kfree(msg);
	return ret;
}


bool hdcp14_block_cipher(u8 *km, u8 repeater, u8 *an, 
						u8 *ks, u8 *m, u8 *r)
{
	int i = 0;
	LFSR* lfsr_ctx = NULL; 
	u32 kx = 0, ky = 0, kz =0;
	u32 kxo = 0, kyo = 0, kzo =0;
	u32 bx = 0, by = 0, bz =0;
	u32 bxo = 0, byo = 0, bzo =0;	
	u64 tmp_v = 0;
	u8 is_big_endian = IS_BIG_ENDIAN;
	
	if(!ks || !m || !r)
		return false; 
			
	kx = char_array_to_uint32(km + 3, 4, is_big_endian);
	kx =  kx & 0x0FFFFFFF;
	
	ky = char_array_to_uint32(km, 4, is_big_endian);
	ky =  ky >> 4;
	
	bx = char_array_to_uint32(an + 4, 4, is_big_endian);
	bx =  bx & 0x0FFFFFFF;
	
	by = char_array_to_uint32(an + 1, 4, is_big_endian);
	by =  by >> 4;
	
	bz = char_array_to_uint32(an, 1, is_big_endian);
	if(repeater) 
		bz |= (1 << 8);
		
	//rtd_pr_hdcp_info("load : kx=%08x,ky=%08x,kz=%08x,bx=%08x,by=%08x,bz=%08x\nstart do 48 clocks\n",
    //			kx, ky, kz, bx, by, bz);
	
	
	
	/*Apply 48 clocks to the block module registers*/
	for(i = 0; i < 48; i++) {
		
		hdcp_1_4_round_function_b(bx, by, bz, ky, &bxo, &byo, &bzo);
		hdcp_1_4_round_function_k(kx, ky, kz, &kxo, &kyo, &kzo);
		bx = bxo;
		by = byo;
		bz = bzo;
		kx = kxo;
		ky = kyo;
		kz = kzo;
		
	}
	//rtd_pr_hdcp_info("48 clock result kx=%08x,ky=%08x,kz=%08x,bx=%08x,by=%08x,bz=%08x\n",
	//		kx, ky, kz, bx, by, bz);
	tmp_v = by;
	tmp_v = (tmp_v  << 28) | bx;
	uint64_to_char_array(tmp_v, ks, 7, is_big_endian);	
	//hdcp_dump_char_array("ks", ks, 7);
	
	kx = bx;
	ky = by;
	kz = bz;

	bx = char_array_to_uint32(an + 4, 4, is_big_endian);
	bx =  bx & 0x0FFFFFFF;
	
	by = char_array_to_uint32(an + 1, 4, is_big_endian);
	by =  by >> 4;
	
	bz = char_array_to_uint32(an, 1, is_big_endian);
	if(repeater) 
		bz |= (1 << 8);	
	
	//rtd_pr_hdcp_info("Reload : kx=%08x,ky=%08x,kz=%08x,bx=%08x,by=%08x,bz=%08x\nstart do 48 clocks\n",
	//		kx, ky, kz, bx, by, bz);
	
	lfsr_ctx = lfsr_init(ks);
	if(!lfsr_ctx)
		return false;
	//lfsr_print(lfsr_ctx);
	for(i  = 0; i < 56; i++) {
		u32 lfsr_v = do_lfsr(lfsr_ctx);
		u32 output = 0;	
		output = hdcp_1_4_output_function(ky, kz, by, bz);	
		
		if(i == 52) {
			uint32_to_char_array(output & 0xFFFF, m, 2, is_big_endian);
		} else if(i == 53) {
			uint32_to_char_array(output & 0xFFFF, m + 2, 2, is_big_endian);
		} else if(i == 54) {
			uint32_to_char_array(output & 0xFFFF, m + 4, 2, is_big_endian);
			uint32_to_char_array((output >> 16) & 0xFF, r, 1, is_big_endian);
		} else if(i == 55) {
			uint32_to_char_array(output & 0xFFFF, m + 6, 2, is_big_endian);
			uint32_to_char_array((output >> 16) & 0xFF, r + 1, 1, is_big_endian);
		}

		hdcp_1_4_round_function_b(bx, by, bz, ky, &bxo, &byo, &bzo);
		hdcp_1_4_round_function_k(kx, ky, kz, &kxo, &kyo, &kzo);
		
		bx = bxo;
		by = byo;
		bz = bzo;
		kx = kxo;
		ky = kyo;
		kz = kzo;	
		
		if(lfsr_v)
			ky = ky | (1 << 13);
		else
			ky = ky & (~(1 << 13));
			
		//rtd_pr_hdcp_info("loop %d clock result kx=%08x,ky=%08x,kz=%08x,bx=%08x,by=%08x,bz=%08x, %08x\n",
		//	i, kx, ky, kz, bx, by, bz, output);
	}
	lfsr_uninit(lfsr_ctx);
	//hdcp_dump_char_array("m", m, 8);
	//hdcp_dump_char_array("m", r, 2);	
	return true;				
}
