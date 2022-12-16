#ifndef __CC_H__
#define __CC_H__

#include "cpu.h"
#include <stdlib.h>
#include <stdio.h>
#include "app_debug.h"
typedef int32_t sys_prot_t;

#if defined (__GNUC__) & !defined (__CC_ARM)
#define LWIP_TIMEVAL_PRIVATE 0
#include <sys/time.h>

#endif

/* define compiler specific symbols */
#if defined (__ICCARM__)
#define PACK_STRUCT_BEGIN
#define PACK_STRUCT_STRUCT
#define PACK_STRUCT_END
#define PACK_STRUCT_FIELD(x) x
#define PACK_STRUCT_USE_INCLUDES

#elif defined (__GNUC__)

#define PACK_STRUCT_BEGIN
#define PACK_STRUCT_STRUCT __attribute__ ((__packed__))
#define PACK_STRUCT_END
#define PACK_STRUCT_FIELD(x) x

#elif defined (__CC_ARM)

#define PACK_STRUCT_BEGIN __packed
#define PACK_STRUCT_STRUCT
#define PACK_STRUCT_END
#define PACK_STRUCT_FIELD(x) x

#elif defined (__TASKING__)

#define PACK_STRUCT_BEGIN
#define PACK_STRUCT_STRUCT
#define PACK_STRUCT_END
#define PACK_STRUCT_FIELD(x) x

#endif

#ifndef LWIP_PLATFORM_ASSERT
#define LWIP_PLATFORM_ASSERT(x) do {ESP32NET_LOG("Assertion \"%s\" failed at line %d in %s\r\n", \
                                     x, __LINE__, __FILE__); } while(0)

#define LWIP_PLATFORM_DIAG(x) do {ESP32NET_LOG x;} while(0)

#endif /* LWIP_PLATFORM_ASSERT */
/* Define random number generator function */
extern uint32_t sys_rand(void);
#define LWIP_RAND() ((u32_t)sys_rand())

#endif /* __CC_H__ */
