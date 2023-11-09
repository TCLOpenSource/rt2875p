#ifndef __RTK_RSA_API_H__
#define __RTK_RSA_API_H__
#include "rtk_rsa_lib.h"
#include "rtk_rsa_util.h"

extern RTK_RSA_T * RTK_RSA_new(void);
extern void  RTK_RSA_free(RTK_RSA_T *rsa);
extern int RTK_RSA_set_hash_algo(RTK_RSA_T *rsa, HASH_ALGO_ID hash_algo);
extern int RTK_RSA_set_padding_mode(RTK_RSA_T *rsa, RSA_PADDING_MODE padding_mode);
extern int RTK_RSA_set_key(RTK_RSA_T *rsa, RSA_KEY_TYPE key_type,
                           RSA_INPUT_TYPE key_input_type, UINT8 *key, UINT32 len);
extern int RTK_RSA_public_key_encrypt(RTK_RSA_T *rsa, UINT8 *input_buff, UINT32 input_len,
                                      UINT8 *output_buff, UINT32 output_len);
extern int RTK_RSA_public_key_decrypt(RTK_RSA_T *rsa, UINT8 *input_buff, UINT32 input_len,
                                      UINT8 *output_buff, UINT32 output_len);
extern int RTK_RSA_private_key_encrypt(RTK_RSA_T *rsa, UINT8 *input_buff, UINT32 input_len,
                                       UINT8 *output_buff, UINT32 output_len);
extern int RTK_RSA_private_key_decrypt(RTK_RSA_T *rsa, UINT8 *input_buff, UINT32 input_len,
                                       UINT8 *output_buff, UINT32 output_len);
extern int RTK_RSA_verfiy(RTK_RSA_T *rsa, UINT8 *sigbuf, UINT32 siglen,  UINT8 *digest, UINT32 digest_len);
extern int RTK_RSA_set_verify_callback(RTK_RSA_T *rsa,
                                        int (*rsa_verify_callback)(UINT8 *digest, UINT32 digest_len, UINT8 *dec_sigbuf, UINT32 dec_sigbuf_len));
#endif