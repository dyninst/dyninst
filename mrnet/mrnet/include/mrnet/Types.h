/****************************************************************************
 * Copyright © 2003-2007 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.          *
 ****************************************************************************/

#if !defined(Types_h)
#define Types_h


#if !defined (__STDC_LIMIT_MACROS)
#define __STDC_LIMIT_MACROS
#endif
#if !defined (__STDC_CONSTANT_MACROS)
#define __STDC_CONSTANT_MACROS
#endif

#include <sys/types.h>

#if defined (os_linux)
#include <stdint.h>
#elif defined(os_solaris)
#include <inttypes.h>
#elif defined(os_windows)
#include "xplat/Types.h"
#endif

#if !defined (bool_t)
#define bool_t int32_t
#endif

#if !defined (enum_t)
#define enum_t int32_t
#endif

#if !defined (char_t)
#define char_t char
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

/* pretty names for MRNet port and rank types. */
namespace MRN
{

typedef uint16_t Port;
typedef uint32_t Rank;

extern const Port UnknownPort;
extern const Rank UnknownRank;

}

#define FirstSystemTag 0
#define FirstApplicationTag 100

#endif /* !defined(Types_h) */
