#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <rtk_kdriver/mcp/rsa/rtk_rsa_lib.h>
#include <rtk_kdriver/mcp/rsa/rtk_rsa_api.h>

RTK_RSA_T *  RTK_RSA_new(void)
{
        RTK_RSA_T *rsa = (RTK_RSA_T *)kzalloc(sizeof(RTK_RSA_T), GFP_KERNEL);
        if(rsa)
                rtk_rsa_init(rsa);
        return rsa;
}

void  RTK_RSA_free(RTK_RSA_T *rsa)
{
        if(rsa) {
                rtk_rsa_clean(rsa);
                kfree(rsa);
        }
}

int RTK_RSA_set_key(RTK_RSA_T *rsa, RSA_KEY_TYPE key_type,
                RSA_INPUT_TYPE key_input_type, UINT8 *key, UINT32 len)
{
        int ret = 0;
        unsigned char *tmp_buffer = NULL;
        int total = 0;
        if(!rsa || !key || !len)
                return -EINVAL;
        if(key_input_type == RSA_INPUT_TYPE_CHAR_ARRAY_LITTLE_ENDIAN) {
                int space = 0;
                total = len;
                space = len % BYTES_PER_MPI_LIMB;
                if(space)
                        total = len + BYTES_PER_MPI_LIMB - space;

                tmp_buffer = kzalloc(total, GFP_KERNEL);
                if(!tmp_buffer)
                        return -ENOMEM;
                if(space)
                        memcpy(tmp_buffer + BYTES_PER_MPI_LIMB - space, key, len);
                else
                        memcpy(tmp_buffer, key, len);
                change_byte_array_endian(tmp_buffer, total);
        } else if(key_input_type == RSA_INPUT_TYPE_HEX_STRING) {
                int i = 0;
                if(len < 2)
                        return -EINVAL;
                total = len / 2;
                tmp_buffer = kzalloc(total, GFP_KERNEL);
                if(!tmp_buffer)
                        return -ENOMEM;
                for(i = 0; i < total; i++)
                        tmp_buffer[i] = (hex_to_dec(key[2 * i]) << 4) | 
                                                        hex_to_dec(key[2 * i + 1]);
        } else if(key_input_type == RSA_INPUT_TYPE_CHAR_ARRAY_BIG_ENDIAN) {
                total = len ;
                tmp_buffer = key;
        } else {
                return -EINVAL;
        }
        ret = rsa->rsa_set_key(rsa, key_type, tmp_buffer, total);
        if(key_input_type == RSA_INPUT_TYPE_CHAR_ARRAY_LITTLE_ENDIAN
            || key_input_type == RSA_INPUT_TYPE_HEX_STRING)
                kfree(tmp_buffer);
        return ret;
}

int RTK_RSA_set_hash_algo(RTK_RSA_T *rsa, HASH_ALGO_ID hash_algo)
{
        if(rsa)
                return rsa->rsa_set_hash_algo(rsa, hash_algo);
        return -EINVAL;
}

int RTK_RSA_set_padding_mode(RTK_RSA_T *rsa,
                          RSA_PADDING_MODE padding_mode)
{
        if(rsa)
                return rsa->rsa_set_padding_mode(rsa, padding_mode);
        return -EINVAL;        
}

int RTK_RSA_set_verify_callback(RTK_RSA_T *rsa,
                int (*rsa_verify_callback)(UINT8 *digest, UINT32 digest_len, UINT8 *dec_sigbuf, UINT32 dec_sigbuf_len))
{
        if(rsa)
                return rsa->rsa_set_rsa_verify_callback(rsa, rsa_verify_callback);
        return -EINVAL; 
}

int RTK_RSA_public_key_encrypt(RTK_RSA_T *rsa, UINT8 *input_buff, UINT32 input_len,  
                                UINT8 *output_buff, UINT32 output_len)
{
        if(rsa && input_buff && input_len && output_len)
                return rsa->rsa_public_key_encrypt(rsa, input_buff, input_len, output_buff, output_len);
        return -EINVAL;
}

int RTK_RSA_public_key_decrypt(RTK_RSA_T *rsa, UINT8 *input_buff, UINT32 input_len,  
                                UINT8 *output_buff, UINT32 output_len)
{
        if(rsa && input_buff && input_len && output_len)
                return rsa->rsa_public_key_decrypt(rsa, input_buff, input_len, output_buff, output_len);
        return -EINVAL;
}

int RTK_RSA_private_key_encrypt(RTK_RSA_T *rsa, UINT8 *input_buff, UINT32 input_len,  
                                UINT8 *output_buff, UINT32 output_len)
{
        if(rsa && input_buff && input_len && output_len)
                return rsa->rsa_private_key_encrypt(rsa, input_buff, input_len, output_buff, output_len);
        return -EINVAL;
}

int RTK_RSA_private_key_decrypt(RTK_RSA_T *rsa, UINT8 *input_buff, UINT32 input_len,  
                                UINT8 *output_buff, UINT32 output_len)
{
        if(rsa && input_buff && input_len && output_len)
                return rsa->rsa_private_key_decrypt(rsa, input_buff, input_len, output_buff, output_len);
        return -EINVAL;
}

int RTK_RSA_sign(RTK_RSA_T *rsa, UINT8 *digest, UINT32 digest_len, UINT8 *sigbuf, UINT32 siglen)
{
        if(rsa && digest && digest_len && sigbuf && siglen)
                return rsa->rsa_sign(rsa, digest, digest_len, sigbuf, siglen);
        return -EINVAL;
}

int RTK_RSA_verfiy(RTK_RSA_T *rsa, UINT8 *sigbuf, UINT32 siglen, UINT8 *digest, UINT32 digest_len)
{
        if(rsa && digest && digest_len && sigbuf && siglen)
                return rsa->rsa_verfiy(rsa, sigbuf, siglen, digest, digest_len);
        return -EINVAL;
}
