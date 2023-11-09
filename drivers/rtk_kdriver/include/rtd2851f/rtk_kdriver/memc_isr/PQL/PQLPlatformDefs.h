#ifndef _PQLDEFINES_H
#define _PQLDEFINES_H

#include "memc_isr/include/kw_type.h"
#include "memc_isr/Common/kw_common.h"
#include "memc_isr/Common/kw_debug.h"
#include "memc_isr/Driver/regio.h"
//#include "memc_isr/Driver/KI7416_reg.h"
//#include "memc_isr/Driver/KI7426_reg.h"
#ifdef CONFIG_KDRIVER_USE_NEW_COMMON
//#include <scaler/scalerDrvCommon.h>
#else
#include <scalercommon/scalerDrvCommon.h>
#endif
//platform definitions
#ifdef _WIN32
#include "Win32DebugDefs.h"
#else
#define DBGDATA_DEFS(T)
#define DEBUGMODULE_BEGIN(TPARAM,TOUTPUT)
#define ADD_PARAM_DES(T,X,V,D)
#define ADD_PARAM(T,X,V)
#define ADD_OUTPUT_DES(T,X,D)
#define ADD_OUTPUT(T,X)
#define ADD_PARAM_ARRAY_DES(T,X,DIM,V,D)
#define ADD_PARAM_ARRAY(T,X,DIM,V)
#define ADD_OUTPUT_ARRAY_DES(T,X,DIM,D)
#define ADD_OUTPUT_ARRAY(T,X,DIM)
#define ADD_DUMMY_PARAM()
#define ADD_DUMMY_OUTPUT()
#define DEBUGMODULE_END
#define DEBUGSERVER_BEGIN(TPARAM,TOUTPUT)
#define ADD_MODULE_DES(P,p,O,o,D)
#define ADD_MODULE_NAME_DES(P,p,O,o,N,D)
#define ADD_MODULE(P,p,O,o)
#define DEBUGSERVER_END

#define VOID void
/*#define _NULL ((VOID*)0)*/
#define IC_K8LP 1
#define IC_K7LP 0
#define IC_K6LP 0
#define IC_K5LP 0
#define IC_K4LP 0
#define IC_K3LP 0

#define CONFIG_MC_8_BUFFER	1  //YE Test Jerry 8 buffer

#define Pro_tv030 0
#define Pro_tv002 0
#define Pro_tv010 0
#define Pro_tv011 0

#define MIX_MODE_REGION_17 1
#define PQL_DYNAMIC_ADJUST 0
#define PQL_DYNAMIC_ADJUST_FIX_ISSUE 1
#define RTK_ADJUST_HDF 1

#endif

#endif
