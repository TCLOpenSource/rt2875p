#ifndef __RTK_RSA_LIB_H__
#define __RTK_RSA_LIB_H__
#include <linux/mpi.h>
#include <linux/mutex.h>
#include <rtd_log/rtd_module_log.h>

typedef unsigned char UINT8;
typedef unsigned short UINT16;
typedef unsigned int UINT32;
typedef enum _RSA_INPUT_TYPE
{
	RSA_INPUT_TYPE_CHAR_ARRAY_LITTLE_ENDIAN,
	RSA_INPUT_TYPE_HEX_STRING,
	RSA_INPUT_TYPE_CHAR_ARRAY_BIG_ENDIAN,
}RSA_INPUT_TYPE;

typedef enum hash_algo_id {
        HASH_ALGO_ID_MD4,
        HASH_ALGO_ID_MD5,
        HASH_ALGO_ID_SHA1,
        HASH_ALGO_ID_RIPE_MD_160,
        HASH_ALGO_ID_SHA256,
        HASH_ALGO_ID_SHA384,
        HASH_ALGO_ID_SHA512,
        HASH_ALGO_ID_SHA224,
        HASH_ALGO_ID_LAST
} HASH_ALGO_ID;
typedef enum _RSA_PADDING_MODE {
        RSA_PKCS1_PADDING = 0,
        /*RSA_SSLV23_PADDING = 1,
        RSA_NO_PADDING = 2,
        RSA_PKCS1_OAEP_PADDING = 3,
        RSA_X931_PADDING = 4,*/
} RSA_PADDING_MODE;
typedef enum _RSA_KEY_TYPE {
        RSA_KEY_N,
        RSA_KEY_E,
        RSA_KEY_D,
} RSA_KEY_TYPE;

typedef enum _RSA_CRYPT_TYPE {
        PUB_KEY_EN,
        PUB_KEY_DE,
        PRI_KEY_EN,
        PRI_KEY_DE,
} RSA_CRYPT_TYPE;

/*
 * Hash algorithm OIDs plus ASN.1 DER wrappings [RFC4880 sec 5.2.2].
 */
static const u8 RSA_digest_info_MD5[] = {
        0x30, 0x20, 0x30, 0x0C, 0x06, 0x08,
        0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x02, 0x05, /* OID */
        0x05, 0x00, 0x04, 0x10
};

static const u8 RSA_digest_info_SHA1[] = {
        0x30, 0x21, 0x30, 0x09, 0x06, 0x05,
        0x2B, 0x0E, 0x03, 0x02, 0x1A,
        0x05, 0x00, 0x04, 0x14
};

static const u8 RSA_digest_info_RIPE_MD_160[] = {
        0x30, 0x21, 0x30, 0x09, 0x06, 0x05,
        0x2B, 0x24, 0x03, 0x02, 0x01,
        0x05, 0x00, 0x04, 0x14
};

static const u8 RSA_digest_info_SHA224[] = {
        0x30, 0x2d, 0x30, 0x0d, 0x06, 0x09,
        0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x04,
        0x05, 0x00, 0x04, 0x1C
};

static const u8 RSA_digest_info_SHA256[] = {
        0x30, 0x31, 0x30, 0x0d, 0x06, 0x09,
        0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x01,
        0x05, 0x00, 0x04, 0x20
};

static const u8 RSA_digest_info_SHA384[] = {
        0x30, 0x41, 0x30, 0x0d, 0x06, 0x09,
        0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x02,
        0x05, 0x00, 0x04, 0x30
};

static const u8 RSA_digest_info_SHA512[] = {
        0x30, 0x51, 0x30, 0x0d, 0x06, 0x09,
        0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x03,
        0x05, 0x00, 0x04, 0x40
};

static const struct {
        const u8 *data;
        size_t size;
} RSA_ASN1_templates[HASH_ALGO_ID_LAST] = {
#define _(X) { RSA_digest_info_##X, sizeof(RSA_digest_info_##X) }
        [HASH_ALGO_ID_MD5]         = _(MD5),
        [HASH_ALGO_ID_SHA1]        = _(SHA1),
        [HASH_ALGO_ID_RIPE_MD_160] = _(RIPE_MD_160),
        [HASH_ALGO_ID_SHA256]      = _(SHA256),
        [HASH_ALGO_ID_SHA384]      = _(SHA384),
        [HASH_ALGO_ID_SHA512]      = _(SHA512),
        [HASH_ALGO_ID_SHA224]      = _(SHA224),
#undef _
};

typedef struct _RTK_RSA_T RTK_RSA_T;
struct _RTK_RSA_T {
        struct mutex mutex;
        RSA_PADDING_MODE padding_mode;
        HASH_ALGO_ID hash_algo;
        MPI n;
        MPI e;
        MPI d;
        MPI r2;
        int (*rsa_set_hash_algo)(RTK_RSA_T *rsa, HASH_ALGO_ID hash_algo);
        int (*rsa_set_padding_mode)(RTK_RSA_T *rsa,
                                     RSA_PADDING_MODE padding_mode);
        int (*rsa_set_key)(RTK_RSA_T *rsa, RSA_KEY_TYPE key_type,
                           UINT8 *key, UINT32 len);
        int (*rsa_public_key_encrypt)(RTK_RSA_T *rsa, UINT8 *input, UINT32 input_len,
                                                UINT8 *output, UINT32 output_len);
        int (*rsa_public_key_decrypt)(RTK_RSA_T *rsa, UINT8 *input, UINT32 input_len,
                                                UINT8 *output, UINT32 output_len);
        int (*rsa_private_key_encrypt)(RTK_RSA_T *rsa, UINT8 *input, UINT32 input_len,
                                                UINT8 *output, UINT32 output_len);
        int (*rsa_private_key_decrypt)(RTK_RSA_T *rsa, UINT8 *input, UINT32 input_len,
                                                UINT8 *output, UINT32 output_len);
        int (*rsa_sign)(RTK_RSA_T *rsa,  UINT8 *digest, UINT32 digest_size, UINT8 *sigbuf, UINT32 siglen);
        int (*rsa_verfiy)(RTK_RSA_T *rsa, UINT8 *sig_buf, UINT32 sig_len, UINT8 *digest, UINT32 digest_len);
        int (*rsa_set_rsa_verify_callback)(RTK_RSA_T *rsa,
                int (*rsa_verify_callback)(UINT8 *digest, UINT32 digest_size, UINT8 *dec_sigbuf, UINT32 dec_sigbuf_size));
        int (*rsa_verify_callback)(UINT8 *digest, UINT32 digest_size,
                                   UINT8 *dec_sigbuf, UINT32 dec_sigbuf_size);
};

extern void rtk_rsa_init(RTK_RSA_T *rsa) ;
extern void rtk_rsa_clean(RTK_RSA_T *rsa) ;


#define RSA_DBG(fmt, args...)               rtd_pr_rsa_debug("[DBG] " fmt, ## args)
#define RSA_INFO(fmt, args...)              rtd_pr_rsa_info("[Info] " fmt, ## args)
#define RSA_WARNING(fmt, args...)       rtd_pr_rsa_warn("[Warn] " fmt, ## args)
#define RSA_ERR(fmt, args...)       rtd_pr_rsa_err("[Err] " fmt, ## args)


#endif

