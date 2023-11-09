#ifndef __ATSC3_ALP_LIB_H__
#define __ATSC3_ALP_LIB_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <rtd_log/rtd_module_log.h>

//#pragma pack(1)

#define ATSC3_ALP_HDR_SIZE        188

unsigned char* atsc3_alp_find_starter(unsigned char *alp, unsigned long alp_len, unsigned long sb_len);

//////////////////////////////////////////////

#define ATSC3_ALP_WARNING(fmt, args...)       rtd_pr_atsc3alp_warn( fmt, ## args)
#define ATSC3_ALP_INFO(fmt, args...)          rtd_pr_atsc3alp_info( fmt, ## args)
#define ATSC3_ALP_DBG(fmt, args...)           rtd_pr_atsc3alp_debug( fmt, ## args)


#ifdef __cplusplus
}
#endif

#endif
