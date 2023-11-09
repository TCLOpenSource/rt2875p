#ifndef __RTK_PQ_CMN_UTIL_LIB_TYPES_H__
#define __RTK_PQ_CMN_UTIL_LIB_TYPES_H__

/*---------------------------------------------------------------------------*
 *      Description: This is wrapper to Platform base types
 *      Prefix:
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
 *      External Header Files [#include <filename.h>]
 *---------------------------------------------------------------------------*/
#include <linux/types.h>

/*---------------------------------------------------------------------------*
 *      Internal Header Files [#include "filename.h" ]
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
 *      Global Macros/Constants/Typedefs
 *---------------------------------------------------------------------------*/

typedef unsigned char       UINT8;
typedef unsigned short      UINT16;
typedef unsigned int        UINT32;
typedef unsigned long long  UINT64;
typedef unsigned char       BYTE;
//typedef char                BOOL;
typedef char                CHAR;
typedef char                INT8;
typedef short               INT16;
typedef int                 INT32;
typedef long long           INT64;
typedef void                VOID;
typedef signed char         BOOL8;
typedef signed long         BOOL32;

#ifdef BOOL
#undef BOOL
#define BOOL    UINT8
#else
#define BOOL    UINT8
#endif

#ifndef FALSE
#define FALSE               0
#endif

#ifndef TRUE
#define TRUE                1
#endif


typedef signed char BOOL8;
typedef unsigned char FLAGS8;
typedef unsigned char BITFLD8;

typedef signed short BOOL16;
typedef unsigned short FLAGS16;
typedef unsigned short BITFLD16;

typedef signed long BOOL32;
typedef unsigned long FLAGS32;
typedef unsigned long BITFLD32;

typedef union {
    UINT8 data;
    struct {
        UINT8 bit7:1;
        UINT8 bit6:1;
        UINT8 bit5:1;
        UINT8 bit4:1;
        UINT8 bit3:1;
        UINT8 bit2:1;
        UINT8 bit1:1;
        UINT8 bit0:1;
    } member;
} TYP_BIT8;

typedef union {
    UINT16 data;
    struct {
        UINT8 bit15:1;
        UINT8 bit14:1;
        UINT8 bit13:1;
        UINT8 bit12:1;
        UINT8 bit11:1;
        UINT8 bit10:1;
        UINT8 bit9:1;
        UINT8 bit8:1;
        UINT8 bit7:1;
        UINT8 bit6:1;
        UINT8 bit5:1;
        UINT8 bit4:1;
        UINT8 bit3:1;
        UINT8 bit2:1;
        UINT8 bit1:1;
        UINT8 bit0:1;
    } member;
} TYP_BIT16;

typedef union {
    UINT32 data;
    struct {
        UINT8 bit31:1;
        UINT8 bit30:1;
        UINT8 bit29:1;
        UINT8 bit28:1;
        UINT8 bit27:1;
        UINT8 bit26:1;
        UINT8 bit25:1;
        UINT8 bit24:1;
        UINT8 bit23:1;
        UINT8 bit22:1;
        UINT8 bit21:1;
        UINT8 bit20:1;
        UINT8 bit19:1;
        UINT8 bit18:1;
        UINT8 bit17:1;
        UINT8 bit16:1;
        UINT8 bit15:1;
        UINT8 bit14:1;
        UINT8 bit13:1;
        UINT8 bit12:1;
        UINT8 bit11:1;
        UINT8 bit10:1;
        UINT8 bit9:1;
        UINT8 bit8:1;
        UINT8 bit7:1;
        UINT8 bit6:1;
        UINT8 bit5:1;
        UINT8 bit4:1;
        UINT8 bit3:1;
        UINT8 bit2:1;
        UINT8 bit1:1;
        UINT8 bit0:1;
    } member;
} TYP_BIT32;

typedef union {
    UINT16 data;
    struct {
        UINT8 byte1;
        UINT8 byte0;
    } member;
} TYP_BYTE2;

typedef union {
    struct {
        UINT8 byte2;
        UINT8 byte1;
        UINT8 byte0;
    } member;
} TYP_BYTE3;

typedef union {
    UINT32 data;
    struct {
        UINT8 byte3;
        UINT8 byte2;
        UINT8 byte1;
        UINT8 byte0;
    } member;
} TYP_BYTE4;

typedef struct {
    UINT16 data;
    UINT8 *str_address;
    UINT8 *end_address;
} TYP_DATA_ADR;

typedef union {
    UINT8 data;
    struct {
        UINT8 hbyte1:4;
        UINT8 hbyte0:4;
    } member;
} TYP_HBYTE;

typedef union {
    UINT8 data;
    struct {
        UINT8 qbyte3:2;
        UINT8 qbyte2:2;
        UINT8 qbyte1:2;
        UINT8 qbyte0:2;
    } member;
} TYP_QBYTE;

typedef union {
    UINT16 data;
    struct {
        UINT8 qword3:4;
        UINT8 qword2:4;
        UINT8 qword1:4;
        UINT8 qword0:4;
    } member;
} TYP_QWORD;

typedef union {
    UINT32 data;
    struct {
        UINT8 hqlng7:4;
        UINT8 hqlng6:4;
        UINT8 hqlng5:4;
        UINT8 hqlng4:4;
        UINT8 hqlng3:4;
        UINT8 hqlng2:4;
        UINT8 hqlng1:4;
        UINT8 hqlng0:4;
    } member;
} TYP_HQLNG;

typedef union {
    UINT32 data;
    struct {
        UINT16 hword1;
        UINT16 hword0;
    } member;
} TYP_HWORD;

/*---------------------------------------------------------------------------*
 *      Local Macros/Constants/Typedefs
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
 *      Struct
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
 *      Enum
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
 *      Struct
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
 *      Global Variables
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
 *      Local Variables
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
 *      Global Function Prototypes
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
 *      Local Function Prototypes
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
 *      Global Function
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
 *      Local Function
 *---------------------------------------------------------------------------*/

#endif /* __RTK_PQ_CMN_UTIL_LIB_TYPES_H__ */
