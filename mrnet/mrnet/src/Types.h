#if !defined(Types_h)
#define Types_h


#define __STDC_LIMIT_MACROS
#define __STDC_CONSTANT_MACROS

#include <sys/types.h>

#if defined (linux)
#include <stdint.h>
#elif defined(solaris)
#include <inttypes.h>
#endif

#if !defined (bool_t)
#define bool_t int32_t
#endif

#if !defined (enum_t)
#define enum_t int32_t
#endif

#if !defined (uchar_t)
#define uchar_t unsigned char
#endif

#ifndef FALSE
#define FALSE   (0)
#endif

#ifndef TRUE
#define TRUE    (1)
#endif

#endif /* !defined(Types_h) */
