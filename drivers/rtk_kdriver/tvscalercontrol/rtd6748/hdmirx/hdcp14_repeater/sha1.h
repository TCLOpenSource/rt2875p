/*
* sha1.h
*
* Description:
* This is the header file for code which implements the Secure
* Hashing Algorithm 1 as defined in FIPS PUB 180-1 published
* April 17, 1995.
*
* Many of the variable names in this code, especially the
* single character names, were used because those were the names
* used in the publication.
*
* Please read the file sha1.c for more information.
*
*/

#ifndef _SHA1_H_
#define _SHA1_H_

/*
* If you do not have the ISO standard stdint.h header file, then you
* must typdef the following:
* name meaning
* u32 unsigned 32 bit integer
* u8 unsigned 8 bit integer (i.e., unsigned char)
* int_least16_t integer of >= 16 bits
*
*/
#ifndef _SHA_enum_
#define _SHA_enum_
enum
{
    shaSuccess = 0,
    shaNull,         /* Null pointer parameter */
    shaInputTooLong, /* input data too long */
    shaStateError    /* called Input after Result */
};
#endif
#define SHA1HashSize 20
/*
* This structure will hold context information for the SHA-1
* hashing operation
*/
typedef struct SHA1Context
{
    u32 Intermediate_Hash[SHA1HashSize / 4]; /* Message Digest */
    u32 Length_Low; /* Message length in bits */
    u32 Length_High; /* Message length in bits */
    /* Index into message block array */
    u32 Message_Block_Index;
    u8 Message_Block[64]; /* 512-bit message blocks */
    int Computed; /* Is the digest computed? */
    int Corrupted; /* Is the message digest corrupted? */
} SHA1Context;


/*
* Function Prototypes
*/

int SHA1Reset(SHA1Context *);
int SHA1Input(SHA1Context *, const u8 *, unsigned int);
int SHA1Result(SHA1Context *, u8 Message_Digest[SHA1HashSize]);

#endif
