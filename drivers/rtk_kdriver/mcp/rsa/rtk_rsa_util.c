#include <linux/slab.h>
#include <linux/mpi.h>
#include <rtk_kdriver/mcp/mcp.h>
#include <rtk_kdriver/mcp/rsa/rtk_rsa_lib.h>
#include <rtk_kdriver/mcp/rsa/rtk_rsa_hw_fun.h>

void change_byte_array_endian(UINT8 *buffer, UINT32 len)
{
        int i = 0, j = 0;
        int loop = 0;
        loop = len / sizeof(int);
        for(i = 0; i < loop; i++)
                for(j = 0; j < sizeof(int) / 2; j++) {
                        unsigned char tmp = buffer[sizeof(int) * i + j];
                        buffer[sizeof(int) * i + j] =
                                buffer[sizeof(int) * i + sizeof(int) - j - 1];
                        buffer[sizeof(int) * i + sizeof(int) - j - 1] = tmp;
                }
}

int convert_mpi_to_char_array(MPI x, unsigned char *array, unsigned int len)
{
        int ret = -1;
        unsigned int x_size = 0;
        unsigned char *x_buf = NULL;
        if(x && array && len > 0) {
                x_buf = mpi_get_buffer(x, &x_size, NULL);
                if(!x_buf) {
                        ret = -ENOMEM;
                        goto exit;
                }
                if(len >= x_size)
                        memcpy(array + len - x_size, x_buf, x_size);
                else
                        memcpy(array, x_buf + x_size - len, len);
                kfree(x_buf);
                ret = 0;
        }
exit:
        return ret;
}

unsigned char hex_to_dec(unsigned char hex)
{
        if(hex >= '0' && hex <= '9')
                return hex - '0';
        else if(hex >= 'a' && hex <= 'f')
                return hex - 'a' + 0xa;
        else if(hex >= 'A' && hex <= 'F')
                return hex - 'A' + 0xa;
        return 0;
}

void dump_mem_buffer(char *name, unsigned char *octet_str, unsigned int len)
{
        int i = 0;
        RSA_INFO("****************dump_mem_buffer[%s]*******************\n", name);
        for(i = 0; i < len; i+=8) {
                   RSA_INFO("%02x %02x %02x %02x %02x %02x %02x %02x", octet_str[i],
                    octet_str[i+1],
                    octet_str[i+2],
                    octet_str[i+3],
                    octet_str[i+4],
                    octet_str[i+5],
                    octet_str[i+6],
                    octet_str[i+7]);
        }
        RSA_INFO("\n**************************************************\n");
}

void dump_mpi_value(char *name, MPI x, unsigned int len)
{
        int res = 0;
        unsigned char *buffer = NULL;
        if(x == NULL || len == 0)
                return;
        buffer = kzalloc(len, GFP_ATOMIC);
        if(!buffer)
                return;
        res = convert_mpi_to_char_array(x, buffer, len);
        //if(res == 0)
        RSA_INFO("%s nlimbs %d, nbis %d len = %d, res = %d", name, x->nlimbs, x->nbits,len, res);
        dump_mem_buffer(name, buffer, len);
        kfree(buffer);
}


/*
*Function:Convert a dec array to a mpi.This dec array should be Little indian 
*and msb int first.
*Note: the ret value should be free by caller.
*/
MPI get_mpi_from_char_array(unsigned char *buffer, unsigned int len, UINT8 convert)
{
        MPI res = NULL;
        int space = 0;
        int total = len;
        unsigned char *tmp_buffer = NULL;
        if(len == 0)
                return res;
        space = len % BYTES_PER_MPI_LIMB;
        if(space)
                total = len + BYTES_PER_MPI_LIMB - space;
        
        tmp_buffer = kzalloc(total, GFP_KERNEL);
        if(!tmp_buffer)
                return res;
        if(space)
                memcpy(tmp_buffer + BYTES_PER_MPI_LIMB - space, buffer, len);
        else
                memcpy(tmp_buffer, buffer, len);
        if(convert)
                change_byte_array_endian(tmp_buffer, total);
        res = mpi_read_raw_data(tmp_buffer, total);
        kfree(tmp_buffer);
        return res;
        
}