/****************************************************************************
 * Copyright © 2003-2005 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.          *
 ****************************************************************************/

// $Id: Types-win.h,v 1.4 2005/03/24 04:59:20 darnold Exp $
#ifndef XPLAT_TYPES_WIN_H
#define XPLAT_TYPES_WIN_H

#include <windows.h>
#include <winsock2.h>
#include <limits.h>

// Microsoft's compiler does not provide typedefs for specific-sized integers
// in <stdint.h>, or any other header.
typedef __int8 int8_t;
typedef __int16 int16_t;
typedef __int32 int32_t;
typedef __int64 int64_t;

typedef unsigned __int8 uint8_t;
typedef unsigned __int16 uint16_t;
typedef unsigned __int32 uint32_t;
typedef unsigned __int64 uint64_t;

#if !defined(INT8_MAX)
#  define INT8_MAX _I8_MAX
#  define UINT8_MAX _UI8_MAX
#endif // !defined(INT8_MAX)

#if !defined(INT16_MAX)
#  define INT16_MAX _I16_MAX
#  define UINT16_MAX _UI16_MAX
#endif // !defined(INT16_MAX)

#if !defined(INT32_MAX)
#  define INT32_MAX _I32_MAX
#  define UINT32_MAX _UI32_MAX
#endif // !defined(INT32_MAX)

#if !defined(INT64_MAX)
#  define INT64_MAX _I64_MAX
#  define UINT64_MAX _UI64_MAX
#endif // !defined(INT64_MAX)

// "address" type
typedef char* caddr_t;

// length of socket address struct
typedef int socklen_t;


// IPv4 address type
typedef uint32_t in_addr_t;

#endif // XPLAT_TYPES_WIN_H
