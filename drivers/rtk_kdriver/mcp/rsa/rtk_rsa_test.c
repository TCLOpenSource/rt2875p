#include <linux/module.h>
#include <linux/slab.h>
#include "../rtk_mcp_test.h"
#include <rtk_kdriver/mcp/rsa/rtk_rsa_lib.h>
#include <rtk_kdriver/mcp/rsa/rtk_rsa_api.h>
#include <rtk_kdriver/mcp/rsa/rtk_rsa_hw_fun.h>

/************************only for test*********************************/

static unsigned char * convert_hex_to_octet_array
(unsigned char *octet_str, unsigned char *hex_str, unsigned int hex_str_len)
{
    int i = 0;
    for(i = 0; i < hex_str_len / 2; i++) {
        octet_str[i] =  (hex_to_dec(hex_str[2 * i]) << 4) |
                        hex_to_dec(hex_str[2 * i + 1]);
    }
    return octet_str;

}

#if 0
static int verify_callback(UINT8 *digest, UINT32 digest_size, UINT8 *dec_sigbuf, UINT32 dec_sigbuf_size)
{
    dump_mem_buffer("digest", digest, digest_size);
    dump_mem_buffer("dec_sigbuf", dec_sigbuf, dec_sigbuf_size);
    return 0;
}
#endif

static int hw_rsa_test(void)
{
    unsigned int sig[64] = {
        0xC9CCBCF2, 0xE9D8E63A, 0x0FD57F7F, 0x903EC809, 0x0CB99A5D, 0xAAE36549, 0xE760D762, 0x847572AF,
        0x18F7D4CC, 0xBB5B4804, 0xEBEBB950, 0x3FD5912B, 0x2BE9DB67, 0xAC13D184, 0x85782BC0, 0xFF8EEE80,
        0x28996AF1, 0xC16D4656, 0x530C35CE, 0x159267E6, 0xAF0FA319, 0xA13E273A, 0x553B9255, 0x0CF62209,
        0xCD73FFD0, 0x370602EB, 0x73921DFD, 0x2588DCE0, 0x10D844E4, 0x172689FB, 0x6E68D723, 0x8BCF985A,
        0x8AE2A425, 0xE0D6931D, 0x6625D84B, 0x3FBEEE88, 0x33D01CCE, 0x35A773BE, 0xB644B146, 0x448CF0F0,
        0x92D7D73C, 0xB2226D03, 0xD6D9EDE8, 0x1647920F, 0xDD6522D9, 0xEECB5A63, 0xA1395237, 0xFD109616,
        0x1389B115, 0x73F9806D, 0xBDBBFAB9, 0x72ADE96B, 0xC2898BA0, 0x2D628D97, 0x576B8E88, 0x42986E8D,
        0x6BDFBADD, 0xC641CA08, 0x35B79B0E, 0x2D5D1AC1, 0x024137FF, 0x36781A49, 0x9E3B9586, 0xF701F48A,
    };
    unsigned int mod[64] = {
        0xF3793BCA, 0xA51612A5, 0xC3B7C7A9, 0x526D9037, 0xA5C9FFB6, 0x1B1968F4, 0x8E098943, 0xCF7E8CBC,
        0xD22A77E4, 0xF30021E9, 0x1918944D, 0x40A75529, 0x846E0D12, 0xD7B4A4E6, 0x15B43FAD, 0xFAFBFD3A,
        0x53A34215, 0xD891DB6F, 0xDCD94266, 0x8AEABA16, 0xA510C52D, 0xD7001239, 0xEF0090EF, 0x42E68992,
        0x71F1BE11, 0x90E343BA, 0x13D2AD6F, 0x3A6BBC68, 0xAA7A8124, 0x96AD00EF, 0x06A46C26, 0x5DF211AE,
        0xA3B0D923, 0x92E9D1BD, 0x89A307CD, 0x3FA0E590, 0xECDDBDDC, 0x3D22C32D, 0x37096E3E, 0x1C23B017,
        0x1E4A2987, 0xAE4568FB, 0x8C190B07, 0xA1BCFC0C, 0x1838CDB4, 0x4174B847, 0xD3B52082, 0x83CA3FF2,
        0x8F88B3A9, 0x6E9EEDCF, 0xA7E5921C, 0x95D4C000, 0x0E1D5B1D, 0xBCD413A0, 0xC12C0EBC, 0xC045F70A,
        0x74EF8A01, 0xDEE10A49, 0xF0A41685, 0x8354C77F, 0x4EEB3C75, 0xBF5E99D2, 0x8916D375, 0xFBB3FCF9,
    };

    unsigned char out_ref[32] = {
        0xba, 0x78, 0x16, 0xbf, 0x8f, 0x01, 0xcf, 0xea, 0x41, 0x41, 0x40, 0xde, 0x5d, 0xae, 0x22, 0x23,
        0xb0, 0x03, 0x61, 0xa3, 0x96, 0x17, 0x7a, 0x9c, 0xb4, 0x10, 0xff, 0x61, 0xf2, 0x00, 0x15, 0xad,
    };

    RTK_RSA_T *rsa = RTK_RSA_new();
    int ret = 0;
    unsigned char out_data[32] = {0};
    MPI mpi_sig = NULL;
    MPI out = mpi_alloc(0);
    unsigned int pub_key_value = 65537; /*public key is always 65537*/

    RSA_INFO("========================== HW RSA Test==========================\n");
    if(rsa == NULL || out == NULL){
        if(out)
            mpi_free(out);
        if(rsa)
            RTK_RSA_free(rsa);
        return -1;
    }
    RTK_RSA_set_key(rsa, RSA_KEY_N, RSA_INPUT_TYPE_CHAR_ARRAY_LITTLE_ENDIAN, (UINT8 *)mod, sizeof(mod));
    RTK_RSA_set_key(rsa, RSA_KEY_E, RSA_INPUT_TYPE_CHAR_ARRAY_LITTLE_ENDIAN, (UINT8 *)(&pub_key_value), sizeof(pub_key_value));

    mpi_sig = get_mpi_from_char_array((unsigned char *)sig, 64 * 4, 1);
    //dump_mpi_value("hw test sig:", mpi_sig, mpi_get_size(mpi_sig));
    //dump_mpi_value("hw test rsa->n:", rsa->n, mpi_get_size(rsa->n));
    //dump_mpi_value("hw test rsa->r2:", rsa->r2, mpi_get_size(rsa->r2));
    //dump_mpi_value("hw test rsa->e:", rsa->e, mpi_get_size(rsa->e));
    hwsu_mont_exp_fast(out, mpi_sig, rsa->n, rsa->r2, rsa->e);
    //dump_mpi_value("hw test out:", out, mpi_get_size(out));
    convert_mpi_to_char_array(out, out_data, 32);

    ret += PrintCheckReuslt(1, out_ref, out_data, 32, "[7.2]", __FUNCTION__);
    mpi_free(mpi_sig);
    mpi_free(out);
    RTK_RSA_free(rsa);
    return ret;
}

#define RSA_CRYPT_MODUEL_STR "9899d4925b224dcae06c2f9e07ccb0a7255252dacfcfdad31b528d8a6329f261c0cda60fed6fba5dc03088f2f4acef7aec3b18ef996561dc637d4bb64227a283f4c195aa8bc7306d37b40eeefd6b3a448aa511ec7e610fe967448e9fe40eb89dd55e28c36f2c8c4aa079f7ea1842d9dbf36dbc91649883e0535751b79f88a85823a8795fc538688a7ece5fa218e11483f1d8c60cfd8cab38b3ad1925c5ca7db1d96ec74df8fb13087bcf62318cc61d3768bdfb72981bf5ae7c347d5f6201aac16c4b3db44ca35c90eca3745e7643315d211826fc4db2a7d3f2be7fe170c4f2c206fae9a5f3964a9fd6de08c58faa800c4c1c7f67fe0ffe6a7b56d34bc435a213"
#define RSA_CRYPT_PRIVATE_KEY_STR "4c24ade1dd616f89a493b6ead69f7721dfc88519338a5678f1f02e39f8230d1363bdf977871531dbe7b3101292addbcced24cbdb7a22114383cd1843381d48c52912931887287bec7357ffff9629b3e85179ee48a002d48ab5018b5d12d199a2ffbd3cffcf705e0c3aa8c160687caaf24a33de5c79e2742331d2a72ec586cde06f9ee22f7d98159d7837a9b70a003abcd066960e22ebd7f4f637a9111cc0d3d81e8233f58318ac1119079615c2977aa453e50384c4338af0be06ca3a9e659e378573f7038d98869ed336ef166bd58d7dd3247e83f813796cf4d8af8a23ff4e378b8361ae7c337fb65b4d2c5a9171abad3e0eee94d61f9472d94f81785091f0c9"
static int rsa_crypt_test(void)
{
    int ret;
    unsigned char clear_str[32] = {
        0xc0, 0x78, 0x16, 0xbf, 0x8f, 0x01, 0xcf, 0xea, 0x41, 0xad, 0x03, 0x61, 0xa3, 0x96, 0x17, 0x23,
        0xb0, 0x9c, 0xb4, 0x10, 0xff, 0x61, 0xf2, 0x00, 0x15, 0x7a, 0x41, 0x40, 0xde, 0x5d, 0xae, 0x22
    };
    char *rsa_module_str = kmalloc(strlen(RSA_CRYPT_MODUEL_STR), GFP_KERNEL);
    char *private_key_str = kmalloc(strlen(RSA_CRYPT_PRIVATE_KEY_STR), GFP_KERNEL);
    unsigned char *chipher = kmalloc(256, GFP_KERNEL);
    unsigned char *clear_result = kmalloc(256, GFP_KERNEL);
    unsigned int exp = 65537;
    /*expect:0001ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff003031300d0609608648016503040201050004203636363636363636363636363636363636363636363636363636363636363636*/
    RTK_RSA_T * rsa = NULL;
    rsa = RTK_RSA_new();
    if(!chipher || !clear_result || !rsa_module_str || !private_key_str  || !rsa) {
        if(chipher)
            kfree(chipher);
        if(clear_result)
            kfree(clear_result);
        if(rsa_module_str)
            kfree(rsa_module_str);
        if(private_key_str)
            kfree(private_key_str);    
        if(rsa)
            RTK_RSA_free(rsa);
        return -1;
    }
    RSA_INFO("==================== RSA crypt test start ================================\n");
    memcpy(rsa_module_str, RSA_CRYPT_MODUEL_STR, strlen(RSA_CRYPT_MODUEL_STR));
    memcpy(private_key_str, RSA_CRYPT_PRIVATE_KEY_STR, strlen(RSA_CRYPT_PRIVATE_KEY_STR));
    RTK_RSA_set_key(rsa, RSA_KEY_N, RSA_INPUT_TYPE_HEX_STRING,
                    rsa_module_str, strlen(RSA_CRYPT_MODUEL_STR));

    RTK_RSA_set_key(rsa, RSA_KEY_E, RSA_INPUT_TYPE_CHAR_ARRAY_LITTLE_ENDIAN,
                    (UINT8 *)&exp, sizeof(int));
    RTK_RSA_set_key(rsa, RSA_KEY_D, RSA_INPUT_TYPE_HEX_STRING,
                    private_key_str, strlen(RSA_CRYPT_PRIVATE_KEY_STR));

    ret = RTK_RSA_public_key_encrypt(rsa, clear_str, 32, chipher, 256);
    RSA_INFO("RSA public key encrypt %s\n", (ret == 0) ? "sucess" : "fail");

    ret = RTK_RSA_private_key_decrypt(rsa, chipher, 256, clear_result, 32);
    RSA_INFO("RSA private key decrypt %s\n", (ret == 0) ? "sucess" : "fail");

    ret = PrintCheckReuslt(1, clear_str, clear_result, 32, "[7.2]", __FUNCTION__);
    if(chipher)
        kfree(chipher);
    if(clear_result)
        kfree(clear_result);
    if(rsa_module_str)
        kfree(rsa_module_str);
    if(private_key_str)
        kfree(private_key_str); 
    RTK_RSA_free(rsa);
    RSA_INFO("RSA crypt test %s\n", (ret == 0) ? "sucess" : "fail");
    RSA_INFO("====================  RSA crypt test end ================================\n");
    return ret;
}
#define RSA_VERIFY_SIGN_STR "4770653416bb3a0bc936748b87df62f7db6f5a11937e71abc7ac4c419ed3971dde2f9e198680b0e004528d9290e028efc3a7750a3846ac27be2b6e4ca23e70a1850262743a5880699f112ad20340359095dbfe096d3c43f90af1467972bfea5ae30d29341ecae263e1be2b22c007e68269f00e18bf595f4ea106bea102d24f4d6b4ae1834195f82250e4b47b5b10b450da8df3f952f4d220e002f3b9df94d19172702d4f19f071fade3c282fdb291f846a95d9e677580bc462b63657d22692eb7edddf1f72283785e6a873c3db838fb68298840cd7e19ded1ec0e0e860c42dbf1a53715a291c1128421461a8f6e3ad71e478003db618421576b35bdc72f3acea"
#define RSA_VERIFY_KEY_STR "f122bb19ebf350ed424e339a27b5017cb36eafc6f13cf95cccba7251b3d881f88163f9dcf9337401a61c98d623bc08d0327ea1b16e49838c8d9234961500d10b4edb245381182d06dc70ae56bfb363fcc7855a8b85e63152493bcc263ee8428230b4da1e3e1799c669a5df5f01f0c4dd82706055ad500389fd8573bbecd267ffe592144bb1c4499fac14065f1a1dd09de051de456dafe18e1c64495888d3d380a279cc1a0bf610993b08b497882c476925a31a335d859e5ef0ba131ea7090f3d7c548ebaa01b9a9ac19c25d69974645521852c2c49920c6477e9e9c19220f7f4ae3d9c8903b834e89f13b85bf71d83dee805caeadf86f9a5533e0a9915026ff3"
static int rsa_verify_test(void)
{
    int ret;
    char *signature_str = kmalloc(strlen(RSA_VERIFY_SIGN_STR), GFP_KERNEL);
    char *rsa_key_str =     kmalloc(strlen(RSA_VERIFY_KEY_STR), GFP_KERNEL);
    unsigned char hash_value[] = "3636363636363636363636363636363636363636363636363636363636363636";
    unsigned int exp = 65537;
    /*expect:0001ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff003031300d0609608648016503040201050004203636363636363636363636363636363636363636363636363636363636363636*/
    RTK_RSA_T * rsa = NULL;
    rsa = RTK_RSA_new();
    if(!signature_str || !rsa_key_str  || !rsa) {
        if(signature_str)
            kfree(signature_str);
        if(rsa_key_str)
            kfree(rsa_key_str);
        if(rsa)
            RTK_RSA_free(rsa);
        return -1;
    }
    memcpy(signature_str, RSA_VERIFY_SIGN_STR, strlen(RSA_VERIFY_SIGN_STR));
    memcpy(rsa_key_str, RSA_VERIFY_KEY_STR, strlen(RSA_VERIFY_KEY_STR));
    RSA_INFO("==================== RSA verify test start ================================\n");
    RTK_RSA_set_key(rsa, RSA_KEY_N, RSA_INPUT_TYPE_HEX_STRING,
                    rsa_key_str, strlen(RSA_VERIFY_KEY_STR));

    RTK_RSA_set_key(rsa, RSA_KEY_E, RSA_INPUT_TYPE_CHAR_ARRAY_LITTLE_ENDIAN,
                    (UINT8 *)&exp, sizeof(int));

    //RSA_set_verify_callback(rsa, verify_callback);
    ret = RTK_RSA_verfiy(rsa,
                         convert_hex_to_octet_array(signature_str, signature_str, 512), 256,
                         convert_hex_to_octet_array(hash_value, hash_value, 64), 32);
    if(signature_str)
        kfree(signature_str);
    if(rsa_key_str)
        kfree(rsa_key_str);
    RTK_RSA_free(rsa);
    RSA_INFO("RSA verfiy %s\n", (ret == 0) ? "sucess" : "fail");
    RSA_INFO("====================  RSA verify test end ================================\n");
    return ret;
}

void rsa_module_test(void)
{
    hw_rsa_test();
    rsa_crypt_test();
    rsa_verify_test();
}

