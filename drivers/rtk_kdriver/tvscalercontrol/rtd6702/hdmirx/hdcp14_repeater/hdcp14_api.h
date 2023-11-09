#ifndef __HDCP_CIPHER_API_H__
#define __HDCP_CIPHER_API_H__

#define IS_BIG_ENDIAN 1

void hdcp_dump_char_array(char *name, unsigned char *data, unsigned int len);

/*
*  Parameters:
*      keys[in]:  the hdcp1.4 private key sets, should be 270B, every key should be big endian
*      ksv[in]:   the ksv, should be 7B and big endian 
*      km[out]:  the counted km, 7B, big endian
*  OK return true, NG return false;
*/
bool hdcp14_compute_km(u8 *keys, u8 *ksv, u8 *km);

/*
*  Parameters:
*      km[in]:  should be 7B, every key should be big endian
*      repeater[in]:   device is a hdcp repeater or not 
*      an[in]:   random number, 8B,should be big endian 
*      ks[out]:  the session key, 7B, big endian
*      m[out]:  m key, 8B, big endian
*      r[out] : 2B, big endian
*  OK return true, NG return false;
*/
bool hdcp14_block_cipher(u8 *km, u8 repeater, u8 *an, u8 *ks, u8 *m, uint8_t *r);
/*
*  Parameters:
*      ksv_list[in]:  the ksv list, every ksv should be big endian
*      list_len[in]:   the byte length of the ksv list
*      bstatus[in]:   2B, big endian
*      m[in]:  8B, big endian
*      v[out]:  the hash value
*  OK return true, NG return false;
*/
bool hdcp14_compute_v(u8 *ksv_list, u32 list_len, u8 *bstatus, u8 *m, u8 *v);

void hdcp14_decrypt_key(u8 *enc_key, u8 *plain_key);
#endif 
