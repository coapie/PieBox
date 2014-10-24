/*
 * Copyright (c) 2013, Konghan. All rights reserved.
 * Distributed under the BSD license, see the LICENSE file.
 */

#ifndef __OSAPI_H__
#define __OSAPI_H__

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
void *malloc(size_t size);
void free(void *ptr);
void *calloc(size_t nmemb, size_t size);                            
void *realloc(void *ptr, size_t size);
*/

#define osapi_malloc    malloc
#define osapi_free      free
#define osapi_calloc    calloc
#define osapi_realloc   realloc



#ifdef __cplusplus
}
#endif

#endif // __OSAPI_H__

