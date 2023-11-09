// SPDX-License-Identifier: GPL-2.0-only
/*
 * sha2-ce-glue.c - SHA-224/SHA-256 using ARMv8 Crypto Extensions
 *
 * Copyright (C) 2014 - 2017 Linaro Ltd <ard.biesheuvel@linaro.org>
 */

#include <asm/neon.h>
#include <asm/simd.h>
#include <asm/unaligned.h>
#include <crypto/internal/hash.h>
#include <crypto/internal/simd.h>
#include <linux/version.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 15, 0))
#include <crypto/sha2.h>
#else
#include <crypto/sha.h>
#endif
#include <crypto/sha256_base.h>
#include <linux/cpufeature.h>
#include <linux/crypto.h>
#include <linux/module.h>

struct sha256_ce_state {
	struct sha256_state	sst;
	u32			finalize;
};

asmlinkage void rtk_sha2_ce_transform(struct sha256_ce_state *sst, u8 const *src,
				  int blocks);

static void __sha2_ce_transform(struct sha256_state *sst, u8 const *src,
				int blocks)
{
	return rtk_sha2_ce_transform(container_of(sst, struct sha256_ce_state, sst),
				 src, blocks);
}

const u32 rtk_sha256_ce_offsetof_count = offsetof(struct sha256_ce_state,
					      sst.count);
const u32 rtk_sha256_ce_offsetof_finalize = offsetof(struct sha256_ce_state,
						 finalize);

static inline int sw_sha256_base_do_update(struct sha256_state *sctx,
					const u8 *data,
					unsigned int len,
					sha256_block_fn *block_fn)
{
	unsigned int partial = sctx->count % SHA256_BLOCK_SIZE;

	sctx->count += len;

	if (unlikely((partial + len) >= SHA256_BLOCK_SIZE)) {
		int blocks;

		if (partial) {
			int p = SHA256_BLOCK_SIZE - partial;

			memcpy(sctx->buf + partial, data, p);
			data += p;
			len -= p;

			block_fn(sctx, sctx->buf, 1);
		}

		blocks = len / SHA256_BLOCK_SIZE;
		len %= SHA256_BLOCK_SIZE;

		if (blocks) {
			block_fn(sctx, data, blocks);
			data += blocks * SHA256_BLOCK_SIZE;
		}
		partial = 0;
	}
	if (len)
		memcpy(sctx->buf + partial, data, len);

	return 0;
}

static int sw_sha256_ce_update(struct sha256_ce_state *ce_state, const u8 *data,
			    unsigned int len)
{
	struct sha256_state *sctx = &ce_state->sst;
	if (!crypto_simd_usable())
		return -1;
	
	ce_state->finalize = 0;
	kernel_neon_begin();
	sw_sha256_base_do_update(sctx, data, len, __sha2_ce_transform);
	kernel_neon_end();

	return 0;
}

static inline int sw_sha256_base_do_finalize(struct sha256_state *sctx,
					  sha256_block_fn *block_fn)
{
	const int bit_offset = SHA256_BLOCK_SIZE - sizeof(__be64);
	__be64 *bits = (__be64 *)(sctx->buf + bit_offset);
	unsigned int partial = sctx->count % SHA256_BLOCK_SIZE;

	sctx->buf[partial++] = 0x80;
	if (partial > bit_offset) {
		if(partial != SHA256_BLOCK_SIZE)
			memset(sctx->buf + partial, 0x0, SHA256_BLOCK_SIZE - partial);
		partial = 0;

		block_fn(sctx, sctx->buf, 1);
	}

	memset(sctx->buf + partial, 0x0, bit_offset - partial);
	*bits = cpu_to_be64(sctx->count << 3);
	block_fn(sctx, sctx->buf, 1);

	return 0;
}

static inline int sw_sha256_base_finish(struct sha256_state *sctx, u8 *out)
{
	int digest_size = 32;
	__be32 *digest = (__be32 *)out;
	int i;

	for (i = 0; digest_size > 0; i++, digest_size -= sizeof(__be32))
		put_unaligned_be32(sctx->state[i], digest++);

	*sctx = (struct sha256_state){};
	return 0;
}

static int sw_sha256_ce_final(struct sha256_ce_state *ce_state, 
							bool need_padding, u8 *out)
{
	struct sha256_state *sctx = &ce_state->sst;
	if (!crypto_simd_usable()) {
		return -1;
	}
	ce_state->finalize = 0;
 	if(need_padding) {
		kernel_neon_begin();
		sw_sha256_base_do_finalize(sctx, __sha2_ce_transform);
		kernel_neon_end();
 	}
	return sw_sha256_base_finish(sctx, out);
}

int sw_sha256_ce(bool need_padding, unsigned int *iv, 
		unsigned char *msg, unsigned int msg_len, unsigned char *hash)
{
	struct sha256_ce_state ce_state CRYPTO_MINALIGN_ATTR;
	struct sha256_state *sctx = &ce_state.sst;
	if(!need_padding && (msg_len & 0x3F))
		return -1;
	if(!msg || !msg_len || !hash)
		return -1;
	
	if(!iv) {
		sctx->state[0] = SHA256_H0;
		sctx->state[1] = SHA256_H1;
		sctx->state[2] = SHA256_H2;
		sctx->state[3] = SHA256_H3;
		sctx->state[4] = SHA256_H4;
		sctx->state[5] = SHA256_H5;
		sctx->state[6] = SHA256_H6;
		sctx->state[7] = SHA256_H7;
	} else {
		sctx->state[0] = iv[0];
		sctx->state[1] = iv[1];
		sctx->state[2] = iv[2];
		sctx->state[3] = iv[3];
		sctx->state[4] = iv[4];
		sctx->state[5] = iv[5];
		sctx->state[6] = iv[6];
		sctx->state[7] = iv[7];
	}
	sctx->count = 0;

	if(sw_sha256_ce_update(&ce_state, msg, msg_len) != 0)
		return -1;
	return sw_sha256_ce_final(&ce_state, need_padding, hash);
}

