#ifndef _HDCP2_OPTEE_H
#define _HDCP2_OPTEE_H

#define HDCP2_CMD_LOAD_KEY 						0x0
#define HDCP2_CMD_ENCRYPTO_KEY					0x1
#define HDCP2_CMD_DECRYPTO_KEY					0x2
#define HDCP2_CMD_HANDLE_MESSAGE				0x3
#define HDCP2_CMD_POLLING_MESSAGE				0x4
#define HDCP2_CMD_HDMI_ENCRYPTO_KEY     		0x5
#define HDCP2_CMD_HDMI_DECRYPTO_KEY     		0x6
#define HDCP2_CMD_HDMI_WRITE_KEY     			0x7

#define HDCP2_CMD_WRITE_KEYSET					0x8				
#define HDCP2_CMD_GET_ENCHDCP2KEY				0x9				
#define HDCP2_CMD_SET_ENCHDCP2KEY				0xa				
#define HDCP2_CMD_GET_ROOTPUBLICKEY				0xb				
#define HDCP2_CMD_GET_CERTINFO					0xc			
#define HDCP2_CMD_GET_CERTINFO2					0xd
#define HDCP2_CMD_DECRYPT_RSAES_OAEP    		0xe
#define HDCP2_CMD_KD_KEY_DERIVATION				0xf
#define HDCP2_CMD_COMPUTE_H_PRIME				0x10
#define HDCP2_CMD_COMPUTE_L_PRIME				0x11
#define HDCP2_CMD_COMPUTE_KH					0x12
#define HDCP2_CMD_Encrypt_Km_using_Kh			0x13
#define HDCP2_CMD_Decrypt_Km_using_Kh			0x14
#define HDCP2_CMD_Decrypt_EKS					0x15
#define HDCP2_CMD_XOR_Ks_with_LC128				0x16
#define HDCP2_CMD_Generate_Km		    		0x17
#define HDCP2_CMD_Generate_KS		    		0x18
#define HDCP2_CMD_SHA1							0x19
#define HDCP2_CMD_SHA256						0x1a
#define HDCP2_CMD_HMAC_SHA256					0x1b
#define HDCP2_CMD_AES_CTR						0x1c
#define HDCP2_CMD_GET_PROTECTED_DECRYPTIONKEY	0x1d
#define HDCP2_CMD_SET_PROTECTED_DECRYPTIONKEY	0x1e
#define HDCP2_CMD_Compute_V_Prime				0x1f
#define HDCP2_CMD_Compute_M_Prime				0x20
#define HDCP2_CMD_UseTestVector					0x21
#define HDCP2_CMD_Encrypt_RSAES_OAEP			0x22
#define HDCP2_CMD_HMAC_SHA256_with_kd			0x23
#define HDCP2_CMD_Get_KsXorLc128				0x24
#define HDCP2_CMD_DATA_DECRYPT					0x25

#define HDCP2_CMD_TV002_LOAD_KEY				0x26
#define HDCP2_CMD_DEMOBOARD_LOAD_KEY			0x27

#define HDCP2_CMD_TV002_Burn_Keybox				2005
#define HDCP2_CMD_TV002_Certificate_Keybox		2006
#define HDCP2_CMD_DemoBoard_Burn_Keybox			2001
#define HDCP2_CMD_DemoBoard_Certificate_Keybox	2003


extern int optee_hdcp2_init(void);
extern void optee_hdcp2_deinit(void);
extern spinlock_t* get_hdcp2_optee_spinlock(void);
extern void optee_hdcp2_main(unsigned char id,unsigned char nport,unsigned char *send_buf, unsigned int send_size,unsigned char *rev_buf, unsigned int rev_size);
extern void optee_hdcp2_wid_WriteKeySet(unsigned char id, unsigned char *data, unsigned int size);
extern void optee_hdcp2_widi_GET_CERTINFO2(unsigned char id, unsigned char *data1, unsigned char *data2,unsigned char *data3,unsigned char *data4);
extern void optee_hdcp2_widi_DECRYPT_RSAES_OAEP(unsigned char id, unsigned char *data, unsigned int size);
extern void optee_hdcp2_widi_KD_KEY_DERIVATION(unsigned char id, unsigned char *rtx, unsigned char *rrx, unsigned int version);
extern void optee_hdcp2_widi_Compute_H_Prime(unsigned char id,unsigned char repeater, unsigned int version, unsigned char *rtx, unsigned char *deviceOptionInfo, unsigned char *pHPrime);
extern void optee_hdcp2_widi_Compute_L_Prime(unsigned char id, unsigned int version, unsigned char *rn, unsigned char *rrx, unsigned char *pLPrime);
extern void optee_hdcp2_widi_Compute_KH(unsigned char id);
extern void optee_hdcp2_widi_Encrypt_Km_using_Kh(unsigned char id, unsigned char *m, unsigned char *ekhkm);
extern void optee_hdcp2_widi_Decrypt_Km_using_Kh(unsigned char id, unsigned char *m, unsigned char *ekhkm);
extern void optee_hdcp2_widi_Decrypt_EKS(unsigned char id, unsigned int version, unsigned char *rtx, unsigned char *rrx, unsigned char *rn, unsigned char *eks);
extern void optee_hdcp2_widi_XOR_Ks_with_LC128(unsigned char id);
extern void optee_hdcp2_widi_Generate_Km(unsigned char id, unsigned char *db, unsigned char *dbmask, unsigned char *maskedDB);
extern void optee_hdcp2_widi_Generate_KS(unsigned char id, unsigned int version, unsigned char *rtx, unsigned char *rrx, unsigned char *rn, unsigned char *eks);
extern void optee_hdcp2_widi_SHA1(unsigned char id, unsigned char *in_ptr,  unsigned int in_len,  unsigned char *out_ptr, unsigned int out_len);
extern void optee_hdcp2_widi_SHA256(unsigned char id, unsigned char *in_ptr,  unsigned int in_len,  unsigned char *out_ptr, unsigned int out_len);
extern void optee_hdcp2_widi_HMAC_SHA256(unsigned char id, unsigned char *in_ptr,  unsigned int in_len, unsigned char *key_ptr, unsigned int key_len, unsigned char *out_ptr, unsigned int out_len);
extern void optee_hdcp2_widi_AES_CTR(unsigned char id, unsigned char *in_ptr,  unsigned int in_len, unsigned char *key_ptr, unsigned int key_len, unsigned char *counter_ptr, unsigned int counter_len,unsigned char *out_ptr, unsigned int out_len);
extern void optee_hdcp2_widi_GET_PROTECTED_DECRYPTIONKEY(unsigned char id, unsigned char *data, unsigned int len);
extern void optee_hdcp2_widi_SET_PROTECTED_DECRYPTIONKEY(unsigned char id, unsigned char *data, unsigned int len);
extern void optee_hdcp2_widi_Compute_V_Prime(unsigned char id, unsigned char *in_ptr,  unsigned int in_len,  unsigned char *out_ptr, unsigned int out_len);
extern void optee_hdcp2_widi_Compute_M_Prime(unsigned char id, unsigned char *in_ptr,  unsigned int in_len,  unsigned char *out_ptr, unsigned int out_len);
extern void optee_hdcp2_widi_UseTestVector(unsigned char id, unsigned int argc, unsigned char *testvector, unsigned int size);
extern void optee_hdcp2_widi_Encrypt_RSAES_OAEP(unsigned char id, unsigned char *kpubrx, unsigned char *ekpub_km);
extern void optee_hdcp2_widi_HMAC_SHA256_with_kd(unsigned char id, unsigned char *in_ptr,  unsigned int in_len, unsigned char *out_ptr, unsigned int out_len);
extern void optee_hdcp2_widi_Get_KsXorLc128(unsigned char id, unsigned char *data, unsigned int size);
extern void optee_hdcp2_widi_DATA_DECRYPT(unsigned char id, unsigned char *counter, unsigned char *input, unsigned char *output, int size);
#endif
