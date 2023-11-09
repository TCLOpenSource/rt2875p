#include <linux/kernel.h>
#include <linux/module.h>
#include <crypto/public_key.h>
#include <crypto/algapi.h>
#include <rtk_kdriver/mcp/rsa/rtk_rsa_lib.h>
#include <rtk_kdriver/mcp/rsa/rtk_rsa_util.h>
#include <rtk_kdriver/mcp/rsa/rtk_rsa_hw_fun.h>

static int _rtk_rsa_crypt(MPI *_out, MPI in, MPI mod, MPI r2, MPI key)
{
    int ret = -1;
    MPI out = mpi_alloc(0);
    if (out == NULL)
        return -ENOMEM;
    if (mpi_cmp_ui(in, 0) < 0) {
        RSA_DBG(" = -EBADMSG [input < 0]");
        return -EBADMSG;
    }
    if (mpi_cmp(in, mod) >= 0) {
        RSA_DBG(" = -EBADMSG [input >= n]");
        return -EBADMSG;
    }

    ret = hwsu_mont_exp_fast(out, in, mod, r2, key);
    if (ret < 0) {
        mpi_free(out);
        return ret;
    }

    *_out = out;
    return 0;
}

/*
 * Perform the verification step [RFC3447 sec 8.2.2].
 */
static int _rtk_rsa_key_crypt(RTK_RSA_T *rsa, UINT8 *input, UINT32 input_len,
                              UINT8 *output, UINT32 output_len, RSA_CRYPT_TYPE crypt_type)
{
    size_t tsize, k;
    int ret = -1;
    MPI out = NULL;
    MPI in = NULL;

    if(input == NULL || input_len == 0 || !output || output_len == 0) {
        ret = -EBADMSG;
        goto error;
    }

    in = mpi_read_raw_data(input, input_len);
    if(in == NULL){
        ret = -1;
        goto error;
    }
    if(rsa->n == NULL || rsa->r2 == NULL) {
        ret = -EINVAL;
        goto error;
    }
    k = mpi_get_nbits(rsa->n);
    tsize = mpi_get_nbits(in);

    if (k < tsize) {
        ret = -EBADMSG;
        goto error;
    }

    switch(crypt_type) {
        case PUB_KEY_EN:
        case PUB_KEY_DE:
            if(rsa->e == NULL) {
                ret = -1;
                goto error;
            }
            ret = _rtk_rsa_crypt(&out, in, rsa->n, rsa->r2, rsa->e);
            break;
        case PRI_KEY_EN:
        case PRI_KEY_DE:
            if(rsa->d == NULL) {
                ret = -1;
                goto error;
            }
            ret = _rtk_rsa_crypt(&out, in, rsa->n, rsa->r2, rsa->d);
            break;
        default:
            ret = -1;
            break;
    }

    if(ret == 0)
        ret = convert_mpi_to_char_array(out, output, output_len);
error:
    if(in != NULL)
        mpi_free(in);
    if(out != NULL)
        mpi_free(out);
    return ret;
}

static int rtk_rsa_default_verify(const u8 *H, const u8 *EM, size_t k, size_t hash_size,
                                  const u8 *asn1_template, size_t asn1_size)
{
    unsigned PS_end, T_offset, i;
    if (k < 2 + 1 + asn1_size + hash_size)
        return -EBADMSG;
    /* Decode the EMSA-PKCS1-v1_5 */
    if (EM[1] != 0x01) {
        RSA_DBG(" = -EBADMSG [EM[1] == %02u]", EM[1]);
        return -EBADMSG;
    }

    T_offset = k - (asn1_size + hash_size);
    PS_end = T_offset - 1;
    if (EM[PS_end] != 0x00) {
        RSA_DBG(" = -EBADMSG [EM[T-1] == %02u]", EM[PS_end]);
        return -EBADMSG;
    }

    for (i = 2; i < PS_end; i++) {
        if (EM[i] != 0xff) {
            RSA_DBG(" = -EBADMSG [EM[PS%x] == %02u]", i - 2, EM[i]);
            return -EBADMSG;
        }
    }

    if (crypto_memneq(asn1_template, EM + T_offset, asn1_size) != 0) {
        RSA_DBG(" = -EBADMSG [EM[T] ASN.1 mismatch]");
        return -EBADMSG;
    }

    if (crypto_memneq(H, EM + T_offset + asn1_size, hash_size) != 0) {
        RSA_DBG(" = -EKEYREJECTED [EM[T] hash mismatch]");
        return -EKEYREJECTED;
    }
    return 0;
}

int rtk_rsa_public_key_encrypt(RTK_RSA_T *rsa, UINT8 *input, UINT32 input_len,
                               UINT8 *output, UINT32 output_len)
{
    int ret = -1;
    mutex_lock(&rsa->mutex);
    ret = _rtk_rsa_key_crypt(rsa, input, input_len, output, output_len, PUB_KEY_EN);
    mutex_unlock(&rsa->mutex);
    return ret;
}

int rtk_rsa_public_key_decrypt(RTK_RSA_T *rsa, UINT8 *input, UINT32 input_len,
                               UINT8 *output, UINT32 output_len)
{
    int ret = -1;
    mutex_lock(&rsa->mutex);
    ret = _rtk_rsa_key_crypt(rsa, input, input_len, output, output_len, PUB_KEY_DE);
    mutex_unlock(&rsa->mutex);
    return ret;
}
int rtk_rsa_private_key_encrypt(RTK_RSA_T *rsa, UINT8 *input, UINT32 input_len,
                                UINT8 *output, UINT32 output_len)
{
    int ret = -1;
    mutex_lock(&rsa->mutex);
    ret = _rtk_rsa_key_crypt(rsa, input, input_len, output, output_len, PRI_KEY_EN);
    mutex_unlock(&rsa->mutex);
    return ret;
}

int rtk_rsa_private_key_decrypt(RTK_RSA_T *rsa, UINT8 *input, UINT32 input_len,
                                UINT8 *output, UINT32 output_len)
{
    int ret = -1;
    mutex_lock(&rsa->mutex);
    ret = _rtk_rsa_key_crypt(rsa, input, input_len, output, output_len, PRI_KEY_DE);
    mutex_unlock(&rsa->mutex);
    return ret;
}
#if 0
static int rtk_rsa_i2osp(MPI x, size_t xLen, u8 **pX)
{
    unsigned X_size, x_size;
    int X_sign;
    u8 *X;

    /* Make sure the string is the right length.  The number should begin
     * with { 0x00, 0x01, ... } so we have to account for 15 leading zero
     * bits not being reported by MPI.
     */
    x_size = mpi_get_nbits(x);
    RSA_DBG("size(x)=%u xLen*8=%zu\n", x_size, xLen * 8);
    if (x_size != xLen * 8 - 15)
        return -ERANGE;

    X = mpi_get_buffer(x, &X_size, &X_sign);
    if (!X)
        return -ENOMEM;
    if (X_sign < 0) {
        kfree(X);
        return -EBADMSG;
    }
    if (X_size != xLen - 1) {
        kfree(X);
        return -EBADMSG;
    }

    *pX = X;
    return 0;
}
#endif
static int rtk_rsa_set_hash_algo(RTK_RSA_T *rsa, HASH_ALGO_ID hash_algo)
{
    if(hash_algo >= HASH_ALGO_ID_MD4
       && hash_algo < HASH_ALGO_ID_LAST) {
        mutex_lock(&rsa->mutex);
        rsa->hash_algo = hash_algo;
        mutex_unlock(&rsa->mutex);
        return 0;
    }
    return -1;
}
static int rtk_rsa_set_padding_mode(RTK_RSA_T *rsa,
                                    RSA_PADDING_MODE padding_mode)
{
    if(padding_mode == RSA_PKCS1_PADDING) {
        mutex_lock(&rsa->mutex);
        rsa->padding_mode = padding_mode;
        mutex_unlock(&rsa->mutex);
        return 0;
    }
    return -1;
}

static int rtk_rsa_set_rsa_verify_callback(RTK_RSA_T *rsa,
        int (*rsa_verify_callback)(UINT8 *digest, UINT32 digest_size, UINT8 *dec_sigbuf, UINT32 dec_sigbuf_size))
{
    mutex_lock(&rsa->mutex);
    rsa->rsa_verify_callback = rsa_verify_callback;
    mutex_unlock(&rsa->mutex);
    return 0;
}

static int rtk_rsa_set_key(RTK_RSA_T *rsa, RSA_KEY_TYPE key_type,
                           UINT8 *key, UINT32 len)
{
    int ret = -EINVAL;
    MPI tmp = NULL;
    MPI prev = NULL;
    if(key && len)
        tmp = mpi_read_raw_data(key, len);
    if(tmp) {
        ret = 0;
        mutex_lock(&rsa->mutex);
        if(key_type == RSA_KEY_N) {
            prev = rsa->n;
            rsa->n = tmp;
            tmp = NULL;
            /*dump_mpi_value("RSA_KEY_N:", rsa->n, 256);*/
            rsa->r2 = mpi_alloc(0);
            ret = rtk_rsa_get_r2(rsa->n, rsa->r2);
        } else if(key_type == RSA_KEY_E) {
            prev = rsa->e;
            rsa->e = tmp;
            tmp = NULL;
            /*dump_mpi_value("RSA_KEY_E:", rsa->e, 4);*/
        } else if(key_type == RSA_KEY_D) {
            prev = rsa->d;
            rsa->d = tmp;
            tmp = NULL;
        } else {
            ret = -EINVAL;
        }
        mutex_unlock(&rsa->mutex);
    }
    if(tmp != NULL)
        mpi_free(tmp);
    if(prev != NULL)
        mpi_free(prev);
    return ret;
}

/*
 * Perform the sign step [RFC3447 sec 8.2.2].
 */
static int rtk_rsa_sign(RTK_RSA_T *rsa,  UINT8 *digest, UINT32 digest_len, UINT8 *sigbuf, UINT32 siglen)
{
    return -1;
}

/*
 * Perform the verification step [RFC3447 sec 8.2.2].
 */
static int rtk_rsa_verfiy(RTK_RSA_T *rsa, UINT8 *sig_buff, UINT32 sig_len, UINT8 *digest, UINT32 digest_len)
{
    int ret = -1;
    UINT8 *digest_buff = NULL;

    if((rsa->n == NULL) || (digest_len == 0) || (digest == NULL))
        return -1;
    digest_buff = kzalloc(sig_len, GFP_KERNEL);

    mutex_lock(&rsa->mutex);
    if (!RSA_ASN1_templates[rsa->hash_algo].data && !rsa->rsa_verify_callback) {
        ret = -ENOTSUPP;
        goto error;
    }

    if((ret = _rtk_rsa_key_crypt(rsa, sig_buff, sig_len, digest_buff, sig_len, PUB_KEY_EN)) != 0)
        goto error;

    if(rsa->rsa_verify_callback) {
        ret = rsa->rsa_verify_callback(digest, digest_len, digest_buff, sig_len);
    } else {
        ret = rtk_rsa_default_verify(digest, digest_buff, sig_len, digest_len,
                                     RSA_ASN1_templates[rsa->hash_algo].data,
                                     RSA_ASN1_templates[rsa->hash_algo].size);
    }

error:
    mutex_unlock(&rsa->mutex);
    kfree(digest_buff);
    digest_buff = NULL;
    return ret;
}

void rtk_rsa_init(RTK_RSA_T *rsa)
{
    if(rsa) {
        mutex_init(&rsa->mutex);
        rsa->hash_algo = HASH_ALGO_ID_SHA256;
        rsa->padding_mode = RSA_PKCS1_PADDING;
        rsa->rsa_set_hash_algo = rtk_rsa_set_hash_algo;
        rsa->rsa_set_padding_mode = rtk_rsa_set_padding_mode;
        rsa->rsa_set_key = rtk_rsa_set_key;
        rsa->rsa_public_key_encrypt = rtk_rsa_public_key_encrypt;
        rsa->rsa_public_key_decrypt = rtk_rsa_public_key_decrypt;
        rsa->rsa_private_key_encrypt = rtk_rsa_private_key_encrypt;
        rsa->rsa_private_key_decrypt = rtk_rsa_private_key_decrypt;
        rsa->rsa_sign = rtk_rsa_sign;
        rsa->rsa_verfiy = rtk_rsa_verfiy;
        rsa->rsa_set_rsa_verify_callback = rtk_rsa_set_rsa_verify_callback;
        rsa->rsa_verify_callback = NULL;
    }
}

void rtk_rsa_clean(RTK_RSA_T *rsa)
{
    if(rsa) {
        if(rsa->d) {
            mpi_free(rsa->d);
            rsa->d = NULL;
        }
        if(rsa->e) {
            mpi_free(rsa->e);
            rsa->e = NULL;
        }
        if(rsa->n) {
            mpi_free(rsa->n);
            rsa->n = NULL;
        }
        if(rsa->r2) {
            mpi_free(rsa->r2);
            rsa->r2 = NULL;
        }
        memset(rsa, 0, sizeof(RTK_RSA_T));
    }
}

