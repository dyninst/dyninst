/***********************************************************************
 * Copyright © 2003 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.     *
 **********************************************************************/

#if !defined(Types_h)
#define Types_h


#if !defined (__STDC_LIMIT_MACROS)
#define __STDC_LIMIT_MACROS
#endif
#if !defined (__STDC_CONSTANT_MACROS)
#define __STDC_CONSTANT_MACROS
#endif

#include <sys/types.h>

#if defined (linux)
#include <stdint.h>
#elif defined(solaris)
#include <inttypes.h>
#elif defined(WIN32)
#include "xplat/Types.h"
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
