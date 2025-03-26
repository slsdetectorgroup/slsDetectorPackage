// SPDX-License-Identifier: Apache-2.0
/*
 * Copyright 1995-2016 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the OpenSSL license (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

/*
 * Modifications 2021 Paul Scherrer Institut
 * Removed most of the code that is not relevant for our scope.
 * Snippets copied from md5_local.h or md32_common.h has been marked
 */

#ifndef HEADER_MD5_H
#define HEADER_MD5_H

#include <stddef.h>

#ifdef __cplusplus
/*
 * Modifications 2021 Paul Scherrer Institut
 * namespace sls added
 */
namespace sls {
extern "C" {
#endif

/*
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 * ! MD5_LONG has to be at least 32 bits wide.                     !
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 */
#define MD5_LONG          unsigned int
#define MD5_CBLOCK        64
#define MD5_LBLOCK        (MD5_CBLOCK / 4)
#define MD5_DIGEST_LENGTH 16

/**
 * Modification 2021 Paul Scherrer Institut
 * Comment from md32_common.h
 */
/*-
 * This is a generic 32 bit "collector" for message digest algorithms.
 * Whenever needed it collects input character stream into chunks of
 * 32 bit values and invokes a block function that performs actual hash
 * calculations.
 *
 * Porting guide.
 *
 * Obligatory macros:
 *
 * DATA_ORDER_IS_BIG_ENDIAN or DATA_ORDER_IS_LITTLE_ENDIAN
 *      this macro defines byte order of input stream.
 * HASH_CBLOCK
 *      size of a unit chunk HASH_BLOCK operates on.
 * HASH_LONG
 *      has to be at least 32 bit wide.
 * HASH_CTX
 *      context structure that at least contains following
 *      members:
 *              typedef struct {
 *                      ...
 *                      HASH_LONG       Nl,Nh;
 *                      either {
 *                      HASH_LONG       data[HASH_LBLOCK];
 *                      unsigned char   data[HASH_CBLOCK];
 *                      };
 *                      unsigned int    num;
 *                      ...
 *                      } HASH_CTX;
 *      data[] vector is expected to be zeroed upon first call to
 *      HASH_UPDATE.
 * HASH_UPDATE
 *      name of "Update" function, implemented here.
 * HASH_TRANSFORM
 *      name of "Transform" function, implemented here.
 * HASH_FINAL
 *      name of "Final" function, implemented here.
 * HASH_BLOCK_DATA_ORDER
 *      name of "block" function capable of treating *unaligned* input
 *      message in original (data) byte order, implemented externally.
 * HASH_MAKE_STRING
 *      macro converting context variables to an ASCII hash string.
 *
 * MD5 example:
 *
 *      #define DATA_ORDER_IS_LITTLE_ENDIAN
 *
 *      #define HASH_LONG               MD5_LONG
 *      #define HASH_CTX                MD5_CTX
 *      #define HASH_CBLOCK             MD5_CBLOCK
 *      #define HASH_UPDATE             MD5_Update_SLS
 *      #define HASH_TRANSFORM          MD5_Transform
 *      #define HASH_FINAL              MD5_Final_SLS
 *      #define HASH_BLOCK_DATA_ORDER   md5_block_data_order
 */
#define MD32_REG_T int

/**
 * Modification 2021 Paul Scherrer Institut
 * Made default little endian if big endian not defined
 */
#ifndef DATA_ORDER_IS_BIG_ENDIAN
#define DATA_ORDER_IS_LITTLE_ENDIAN
#endif

/**
 * Modification 2021 Paul Scherrer Institut
 * Macros exported from md32_common.h
 */
#define HASH_LONG             MD5_LONG
#define HASH_CTX              MD5_CTX
#define HASH_CBLOCK           MD5_CBLOCK
#define HASH_UPDATE           MD5_Update_SLS
#define HASH_TRANSFORM        MD5_Transform
#define HASH_FINAL            MD5_Final_SLS
#define HASH_BLOCK_DATA_ORDER md5_block_data_order
#define HASH_MAKE_STRING(c, s)                                                 \
    do {                                                                       \
        unsigned long ll;                                                      \
        ll = (c)->A;                                                           \
        (void)HOST_l2c(ll, (s));                                               \
        ll = (c)->B;                                                           \
        (void)HOST_l2c(ll, (s));                                               \
        ll = (c)->C;                                                           \
        (void)HOST_l2c(ll, (s));                                               \
        ll = (c)->D;                                                           \
        (void)HOST_l2c(ll, (s));                                               \
    } while (0)
#define ROTATE(a, n) (((a) << (n)) | (((a) & 0xffffffff) >> (32 - (n))))
#if defined(DATA_ORDER_IS_BIG_ENDIAN)

#define HOST_c2l(c, l)                                                         \
    (l = (((unsigned long)(*((c)++))) << 24),                                  \
     l |= (((unsigned long)(*((c)++))) << 16),                                 \
     l |= (((unsigned long)(*((c)++))) << 8),                                  \
     l |= (((unsigned long)(*((c)++)))))
#define HOST_l2c(l, c)                                                         \
    (*((c)++) = (unsigned char)(((l) >> 24) & 0xff),                           \
     *((c)++) = (unsigned char)(((l) >> 16) & 0xff),                           \
     *((c)++) = (unsigned char)(((l) >> 8) & 0xff),                            \
     *((c)++) = (unsigned char)(((l)) & 0xff), l)

#elif defined(DATA_ORDER_IS_LITTLE_ENDIAN)

#define HOST_c2l(c, l)                                                         \
    (l = (((unsigned long)(*((c)++)))),                                        \
     l |= (((unsigned long)(*((c)++))) << 8),                                  \
     l |= (((unsigned long)(*((c)++))) << 16),                                 \
     l |= (((unsigned long)(*((c)++))) << 24))
#define HOST_l2c(l, c)                                                         \
    (*((c)++) = (unsigned char)(((l)) & 0xff),                                 \
     *((c)++) = (unsigned char)(((l) >> 8) & 0xff),                            \
     *((c)++) = (unsigned char)(((l) >> 16) & 0xff),                           \
     *((c)++) = (unsigned char)(((l) >> 24) & 0xff), l)

#endif

typedef struct MD5state_st {
    MD5_LONG A, B, C, D;
    MD5_LONG Nl, Nh;
    MD5_LONG data[MD5_LBLOCK];
    unsigned int num;
} MD5_CTX;

int MD5_Init_SLS(MD5_CTX *c);
int MD5_Update_SLS(MD5_CTX *c, const void *data, size_t len);
int MD5_Final_SLS(unsigned char *md, MD5_CTX *c);

/**
 * Modification 2021 Paul Scherrer Institut
 * from md32_common.h
 */
void md5_block_data_order(MD5_CTX *c, const void *p, size_t num);
#ifdef __cplusplus
}

} // namespace sls
#endif
#endif
