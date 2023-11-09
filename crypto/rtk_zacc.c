#include <linux/module.h>

#include <linux/kernel.h>

#include <linux/crypto.h>
#include <linux/init.h>
#include <linux/zacc.h>

static int zacc_alg_init(struct crypto_tfm *tfm)
{
	return 0;
}

static void zacc_alg_exit(struct crypto_tfm *tfm)
{
}

static int
zacc_alg_compress(struct crypto_tfm *tfm, u8 const *src, unsigned int slen,
                  u8 *dst, unsigned int *dlen)
{
	ssize_t rv = zacc_encode(dst, src);
	/*
	 * On any error-return from zacc_encode, just pretend
	 * not to be compressed at all. This may invoke zcomp
	 * to memcpy the page.
	 */
	*dlen = (rv < 0) ? PAGE_SIZE : (unsigned int)rv;
	return 0;
}

static int
zacc_alg_decompress(struct crypto_tfm *tfm, u8 const *src, unsigned int slen,
                    u8 *dst, unsigned int *dlen)
{
	ssize_t rv = zacc_decode(dst, src, slen);
	/* Return 0 on success, or pass any error-return. */
	return (rv < 0) ? (int)rv : 0;
}

static struct crypto_alg zacc_alg = {
	.cra_list	= LIST_HEAD_INIT(zacc_alg.cra_list),
	.cra_name	= "zacc",
	.cra_driver_name = "rtk-zacc",
	.cra_flags	= CRYPTO_ALG_TYPE_COMPRESS,
	.cra_compress.coa_compress	= zacc_alg_compress,
	.cra_compress.coa_decompress	= zacc_alg_decompress,
	.cra_init	= zacc_alg_init,
	.cra_exit	= zacc_alg_exit,
	.cra_module	= THIS_MODULE,
};

static int __init zacc_module_init(void)
{
	if (!zacc_ready()) {
		printk("[ERR] rtk-zacc is not ready !!\n");
		return -ENODEV;
	}
	return crypto_register_alg(&zacc_alg);
}
#ifdef MODULE
module_init(zacc_module_init);
#else
late_initcall(zacc_module_init);
#endif

static void __exit zacc_module_exit(void)
{
	crypto_unregister_alg(&zacc_alg);
}
module_exit(zacc_module_exit);

MODULE_ALIAS_CRYPTO("zacc");

MODULE_AUTHOR("Alex-KC Yuan <alexkc.yuan@realtek.com");
MODULE_DESCRIPTION("Crypto wrapper for ZACC HW compression accelerator");
MODULE_LICENSE("GPL");
