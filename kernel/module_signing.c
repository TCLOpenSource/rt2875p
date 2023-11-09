// SPDX-License-Identifier: GPL-2.0-or-later
/* Module signature checker
 *
 * Copyright (C) 2012 Red Hat, Inc. All Rights Reserved.
 * Written by David Howells (dhowells@redhat.com)
 */

#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/module.h>
#include <linux/module_signature.h>
#include <linux/string.h>
#include <linux/verification.h>
#include <crypto/public_key.h>
#include <crypto/rng.h>
#include <crypto/drbg.h>
#include <crypto/akcipher.h>
#include <crypto/kpp.h>
#include <crypto/acompress.h>
#include "module-internal.h"
#include "module_enc_key.h"

/*
 * Verify the signature on a module.
 */
int mod_verify_sig(const void *mod, struct load_info *info)
{
	struct module_signature ms;
	size_t sig_len, modlen = info->len;
	int ret;

	pr_devel("==>%s(,%zu)\n", __func__, modlen);

	if (modlen <= sizeof(ms))
		return -EBADMSG;

	memcpy(&ms, mod + (modlen - sizeof(ms)), sizeof(ms));

	ret = mod_check_sig(&ms, modlen, "module");
	if (ret)
		return ret;

	sig_len = be32_to_cpu(ms.sig_len);
	modlen -= sig_len + sizeof(ms);
	info->len = modlen;

	return verify_pkcs7_signature(mod, modlen, mod + modlen, sig_len,
				      VERIFY_USE_SECONDARY_KEYRING,
				      VERIFYING_MODULE_SIGNATURE,
				      NULL, NULL);
}



struct module_signature_new_version {
	u8	algo;		/* Public-key crypto algorithm [0] */
	u8	hash;		/* Digest algorithm [0] */
	u8	id_type;	/* Key identifier type [PKEY_ID_PKCS7] */
	u8	signer_len;	/* Length of signer's name [0] */
	u8	key_id_len;	/* Length of key identifier [0] */
	u8	__pad[3];
	__be32	sig_len;	/* Length of signature data */
	u8  common_key_index;
	u8  common_iv_index;
	u8  encrypted;
	u8  __pad1;
	u8  key[16];
	u8  iv[16];
};

struct module_decrypt_result {
	struct completion completion;
	int err;
};

static void module_decrypt_complete(struct crypto_async_request *req, int err)
{
	struct module_decrypt_result *res = req->data;

	if (err == -EINPROGRESS)
		return;

	res->err = err;
	complete(&res->completion);
}

/*
unsigned char buf[32768] = {0};
void dump_data(unsigned char *func, unsigned int line, unsigned char *data, unsigned int len)
{
    int i = 0;

    printk(KERN_ERR "%s %d: ", func, line);
    buf[0] = 0;
    for(i  = 0; i < len; i++)
        sprintf(buf+ strlen(buf), "%02x", data[i]);
    printk(KERN_ERR "%s\n", buf);
}
*/

static unsigned char hex_to_dec(unsigned char hex)
{
	if(hex >= '0' && hex <= '9')
		return hex - '0';
	else if(hex >= 'a' && hex <= 'f')
		return hex - 'a' + 0xa;
	else if(hex >= 'A' && hex <= 'F')
		return hex - 'A' + 0xa;
	return 0;
}

static unsigned char * convert_hex_array_to_octet_array(unsigned char *octet_str,
		unsigned char *hex_str, unsigned int hex_str_len)
{
	unsigned int i = 0;

	for (i = 0; i < hex_str_len / 2; i++) {
		octet_str[i] =  (hex_to_dec(hex_str[2 * i]) << 4) | hex_to_dec(hex_str[2 * i + 1]);
	}

	return octet_str;
}

static int get_common_key_table(unsigned char *key_table, unsigned int key_table_len)
{
	char *hex_str = CONFIG_MODULE_ENC_KEY;

	if (strlen(hex_str) < 2 * key_table_len)
		return -EINVAL;

	convert_hex_array_to_octet_array(key_table, hex_str, 2 * key_table_len);

	return 0;
}


static int do_module_decrypt(struct module_signature_new_version *pms,
        unsigned char *mbuf, unsigned int len)
{
	unsigned char key[16] = {0};
	unsigned char iv[16] = {0};
	unsigned char common_key_table[128];
	unsigned int common_key_index = pms->common_key_index %(sizeof(common_key_table) - 16);
	unsigned int common_iv_index = pms->common_iv_index %(sizeof(common_key_table) - 16);
	unsigned char common_iv[16] = {0};
	unsigned char *buf =  NULL;
	struct crypto_skcipher *tfm = NULL;
	struct skcipher_request *req = NULL;
	struct module_decrypt_result result;
	struct scatterlist sg[1];
	unsigned int decrypt_unit_len = PAGE_SIZE;
	int ret = 0;
	unsigned int cur_pos = 0;

	memcpy(key, pms->key, 16);
	memcpy(iv, pms->iv, 16);

	ret = get_common_key_table(common_key_table, 128);
	if(ret != 0)
		return ret;

	buf = (unsigned char *)get_zeroed_page(GFP_KERNEL);
	if(!buf)
		return -ENOMEM;

	tfm = crypto_alloc_skcipher("cbc(aes)", 0, 0);
	if (IS_ERR(tfm)) {
		pr_err("%s: Failed to load transform for "
				"%s: %ld\n", __FUNCTION__, "cbc(aes)", PTR_ERR(tfm));
		free_pages((unsigned long)buf, 0);
		return PTR_ERR(tfm);
	}

	req = skcipher_request_alloc(tfm, GFP_KERNEL);
	if (!req) {
		pr_err("%s: Failed to allocate request for %s\n", __FUNCTION__, "cbc(aes)");
		crypto_free_skcipher(tfm);
		free_pages((unsigned long)buf, 0);
		return -ENOMEM;
	}

	skcipher_request_set_callback(req, CRYPTO_TFM_REQ_MAY_BACKLOG,
			module_decrypt_complete, &result);

	crypto_skcipher_clear_flags(tfm, ~0);
	ret = crypto_skcipher_setkey(tfm, common_key_table + common_key_index, 16);
	if(ret != 0) {
		pr_err("%s: Failed to set key  %s\n", __FUNCTION__, "cbc(aes)");
		skcipher_request_free(req);
		crypto_free_skcipher(tfm);
		free_pages((unsigned long)buf, 0);
		return ret;
	}
	memcpy(buf, key, 16);
	sg_init_one(&sg[0], buf, 16);
	crypto_skcipher_clear_flags(tfm, ~0);
	memcpy(common_iv, common_key_table + common_iv_index, 16);
	skcipher_request_set_crypt(req, sg, sg, 16, common_iv);
	result.err = 0;
	init_completion(&result.completion);
	ret = crypto_skcipher_decrypt(req);
	switch (ret) {
	case 0:
		break;
	case -EINPROGRESS:
	case -EBUSY:
		wait_for_completion(&result.completion);
		reinit_completion(&result.completion);
		ret = result.err;
		if (!ret)
			break;
	default:
		ret = -EIO;
	}

	if(ret != 0) {
		skcipher_request_free(req);
		crypto_free_skcipher(tfm);
		free_pages((unsigned long)buf, 0);
		pr_err("%s: Failed to decrypt key  %s\n", __FUNCTION__, "cbc(aes)");
		return ret;
	}
	memcpy(key, buf, 16);

	memcpy(buf, iv, 16);
	sg_init_one(&sg[0], buf, 16);
	crypto_skcipher_clear_flags(tfm, ~0);
	memcpy(common_iv, common_key_table + common_iv_index, 16);
	skcipher_request_set_crypt(req, sg, sg, 16, common_iv);
	result.err = 0;
	init_completion(&result.completion);
	ret = crypto_skcipher_decrypt(req);
	switch (ret) {
	case 0:
		break;
	case -EINPROGRESS:
	case -EBUSY:
		wait_for_completion(&result.completion);
		reinit_completion(&result.completion);
		ret = result.err;
		if (!ret)
			break;
	default:
		ret = -EIO;
	}

	if (ret != 0) {
		skcipher_request_free(req);
		crypto_free_skcipher(tfm);
		free_pages((unsigned long)buf, 0);
		pr_err("%s: Failed to decrypt key  %s\n", __FUNCTION__, "cbc(aes)");
		return ret;
	}
	memcpy(iv, buf, 16);

	crypto_skcipher_clear_flags(tfm, ~0);

	ret = crypto_skcipher_setkey(tfm, key, 16);
	if (ret != 0) {
		pr_err("%s: Failed to set key  %s\n", __FUNCTION__, "cbc(aes)");
		skcipher_request_free(req);
		crypto_free_skcipher(tfm);
		free_pages((unsigned long)buf, 0);
		return ret;
	}

	while (len > 0) {
		unsigned int tmp_len = (len > decrypt_unit_len) ? decrypt_unit_len : len;
		memcpy(buf, mbuf + cur_pos, tmp_len);
		sg_init_one(&sg[0], buf, tmp_len);

		/*dump_data("key:", __LINE__, key, 16);
		dump_data("iv:", __LINE__, last_iv, 16);
		dump_data(__FUNCTION__, __LINE__, buf, 32);*/

		crypto_skcipher_clear_flags(tfm, ~0);
		skcipher_request_set_crypt(req, sg, sg, tmp_len, iv);

		result.err = 0;
		init_completion(&result.completion);

		ret = crypto_skcipher_decrypt(req);
		switch (ret) {
		case 0:
			break;
		case -EINPROGRESS:
		case -EBUSY:
			wait_for_completion(&result.completion);
			reinit_completion(&result.completion);
			ret = result.err;
			if (!ret)
				break;
		default:
			ret = -EIO;
		}

		if(ret != 0) {
			pr_err("%s: Failed to decrypt  %s\n", __FUNCTION__, "cbc(aes)");
			break;
		}

		memcpy(iv, mbuf + cur_pos + tmp_len - 16, 16);
		memcpy(mbuf + cur_pos, buf, tmp_len);
		/*dump_data(__FUNCTION__, __LINE__, mbuf + cur_pos, 32);*/
		cur_pos += tmp_len;
		len -= tmp_len;
	}

	skcipher_request_free(req);
	crypto_free_skcipher(tfm);
	free_pages((unsigned long)buf, 0);

	return ret;
}


/*
 * Verify the signature on a module.
 */
int mod_verify_sig_new(const void *mod, struct load_info *info)
{
	struct module_signature_new_version ms;
	size_t sig_len, modlen = info->len;
	int ret;

	pr_devel("==>%s(,%zu)\n", __func__, modlen);

	if (modlen <= sizeof(ms))
		return -EBADMSG;

	memcpy(&ms, mod + (modlen - sizeof(ms)), sizeof(ms));

	ret = mod_check_sig((const struct module_signature *)&ms, modlen, "module");
	if (ret)
		return ret;

	sig_len = be32_to_cpu(ms.sig_len);
	modlen -= sig_len + sizeof(ms);
	info->len = modlen;

	if (ms.encrypted == 1) {
		unsigned int module_content_len = modlen;
		int ret = 0;

		ret = do_module_decrypt(&ms, (unsigned char *)mod, (module_content_len - (module_content_len & 0xf)));
		if (ret != 0)
			return ret;
	}

	return verify_pkcs7_signature(mod, modlen, mod + modlen, sig_len,
				      VERIFY_USE_SECONDARY_KEYRING,
				      VERIFYING_MODULE_SIGNATURE,
				      NULL, NULL);
}
