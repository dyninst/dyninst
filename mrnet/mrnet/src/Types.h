#if !defined(_Types_h_)
#define _Types_h_

#define __STDC_LIMIT_MACROS
#define __STDC_CONSTANT_MACROS

#include <sys/types.h>

#if defined (linux)
#include <stdint.h>
#elif defined(solaris)
#include <inttypes.h>
#endif


#define uchar_t unsigned char

#if !defined (bool_t)
#define bool_t int32_t
#endif

#if !defined (enum_t)
#define enum_t int32_t
#endif

#ifndef FALSE
#define FALSE   (0)
#endif
#ifndef TRUE
#define TRUE    (1)
#endif

#endif /* !defined(_Types_h_) */
