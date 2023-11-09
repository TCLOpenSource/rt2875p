#ifndef __RTK_RSA_UTIL_H__
#define __RTK_RSA_UTIL_H__

#include "rtk_rsa_lib.h"

extern void change_byte_array_endian(UINT8 *buffer, UINT32 len);
extern MPI get_mpi_from_char_array(unsigned char *buffer, unsigned int len, UINT8 convert);
extern void dump_mem_buffer(char *name, unsigned char *octet_str, unsigned int len);
extern void dump_mpi_value(char *name, MPI x, unsigned int len);
extern unsigned char hex_to_dec(unsigned char hex);
extern int convert_mpi_to_char_array(MPI x, unsigned char *array, unsigned int len);
extern int print_check_reuslt(unsigned int enc, unsigned char* pRefData, unsigned char* pOutData, 
                                    unsigned int OutDataLen, const char *msg, const char *fun);
#endif
