#ifndef DYN_DETAIL_BOOST_DETAIL_INTERLOCKED_HPP_INCLUDED
#define DYN_DETAIL_BOOST_DETAIL_INTERLOCKED_HPP_INCLUDED

// MS compatible compilers support #pragma once

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

//
//  boost/detail/interlocked.hpp
//
//  Copyright 2005 Peter Dimov
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//

#include <dyn_detail/boost/config.hpp>

#if defined( DYN_DETAIL_BOOST_USE_WINDOWS_H )

# include <windows.h>

# define DYN_DETAIL_BOOST_INTERLOCKED_INCREMENT InterlockedIncrement
# define DYN_DETAIL_BOOST_INTERLOCKED_DECREMENT InterlockedDecrement
# define DYN_DETAIL_BOOST_INTERLOCKED_COMPARE_EXCHANGE InterlockedCompareExchange
# define DYN_DETAIL_BOOST_INTERLOCKED_EXCHANGE InterlockedExchange
# define DYN_DETAIL_BOOST_INTERLOCKED_EXCHANGE_ADD InterlockedExchangeAdd
# define DYN_DETAIL_BOOST_INTERLOCKED_COMPARE_EXCHANGE_POINTER InterlockedCompareExchangePointer
# define DYN_DETAIL_BOOST_INTERLOCKED_EXCHANGE_POINTER InterlockedExchangePointer

#elif defined(_WIN32_WCE)

// under Windows CE we still have old-style Interlocked* functions

extern "C" long __cdecl InterlockedIncrement( long* );
extern "C" long __cdecl InterlockedDecrement( long* );
extern "C" long __cdecl InterlockedCompareExchange( long*, long, long );
extern "C" long __cdecl InterlockedExchange( long*, long );
extern "C" long __cdecl InterlockedExchangeAdd( long*, long );

# define DYN_DETAIL_BOOST_INTERLOCKED_INCREMENT InterlockedIncrement
# define DYN_DETAIL_BOOST_INTERLOCKED_DECREMENT InterlockedDecrement
# define DYN_DETAIL_BOOST_INTERLOCKED_COMPARE_EXCHANGE InterlockedCompareExchange
# define DYN_DETAIL_BOOST_INTERLOCKED_EXCHANGE InterlockedExchange
# define DYN_DETAIL_BOOST_INTERLOCKED_EXCHANGE_ADD InterlockedExchangeAdd

# define DYN_DETAIL_BOOST_INTERLOCKED_COMPARE_EXCHANGE_POINTER(dest,exchange,compare) \
    ((void*)DYN_DETAIL_BOOST_INTERLOCKED_COMPARE_EXCHANGE((long*)(dest),(long)(exchange),(long)(compare)))
# define DYN_DETAIL_BOOST_INTERLOCKED_EXCHANGE_POINTER(dest,exchange) \
    ((void*)DYN_DETAIL_BOOST_INTERLOCKED_EXCHANGE((long*)(dest),(long)(exchange)))

#elif defined( BOOST_MSVC ) || defined( BOOST_INTEL_WIN )

extern "C" long __cdecl _InterlockedIncrement( long volatile * );
extern "C" long __cdecl _InterlockedDecrement( long volatile * );
extern "C" long __cdecl _InterlockedCompareExchange( long volatile *, long, long );
extern "C" long __cdecl _InterlockedExchange( long volatile *, long);
extern "C" long __cdecl _InterlockedExchangeAdd( long volatile *, long);

# pragma intrinsic( _InterlockedIncrement )
# pragma intrinsic( _InterlockedDecrement )
# pragma intrinsic( _InterlockedCompareExchange )
# pragma intrinsic( _InterlockedExchange )
# pragma intrinsic( _InterlockedExchangeAdd )

# if defined(_M_IA64) || defined(_M_AMD64)

extern "C" void* __cdecl _InterlockedCompareExchangePointer( void* volatile *, void*, void* );
extern "C" void* __cdecl _InterlockedExchangePointer( void* volatile *, void* );

#  pragma intrinsic( _InterlockedCompareExchangePointer )
#  pragma intrinsic( _InterlockedExchangePointer )

#  define DYN_DETAIL_BOOST_INTERLOCKED_COMPARE_EXCHANGE_POINTER _InterlockedCompareExchangePointer
#  define DYN_DETAIL_BOOST_INTERLOCKED_EXCHANGE_POINTER _InterlockedExchangePointer

# else

#  define DYN_DETAIL_BOOST_INTERLOCKED_COMPARE_EXCHANGE_POINTER(dest,exchange,compare) \
    ((void*)BOOST_INTERLOCKED_COMPARE_EXCHANGE((long volatile*)(dest),(long)(exchange),(long)(compare)))
#  define DYN_DETAIL_BOOST_INTERLOCKED_EXCHANGE_POINTER(dest,exchange) \
    ((void*)BOOST_INTERLOCKED_EXCHANGE((long volatile*)(dest),(long)(exchange)))

# endif

# define DYN_DETAIL_BOOST_INTERLOCKED_INCREMENT _InterlockedIncrement
# define DYN_DETAIL_BOOST_INTERLOCKED_DECREMENT _InterlockedDecrement
# define DYN_DETAIL_BOOST_INTERLOCKED_COMPARE_EXCHANGE _InterlockedCompareExchange
# define DYN_DETAIL_BOOST_INTERLOCKED_EXCHANGE _InterlockedExchange
# define BDYN_DETAIL_OOST_INTERLOCKED_EXCHANGE_ADD _InterlockedExchangeAdd

#elif defined( WIN32 ) || defined( _WIN32 ) || defined( __WIN32__ ) || defined( __CYGWIN__ )
namespace dyn_detail
{
  
namespace boost
{

namespace detail
{

extern "C" __declspec(dllimport) long __stdcall InterlockedIncrement( long volatile * );
extern "C" __declspec(dllimport) long __stdcall InterlockedDecrement( long volatile * );
extern "C" __declspec(dllimport) long __stdcall InterlockedCompareExchange( long volatile *, long, long );
extern "C" __declspec(dllimport) long __stdcall InterlockedExchange( long volatile *, long );
extern "C" __declspec(dllimport) long __stdcall InterlockedExchangeAdd( long volatile *, long );

} // namespace detail

} // namespace boost

} // namespace dyn_detail

# define DYN_DETAIL_BOOST_INTERLOCKED_INCREMENT ::dyn_detail::boost::detail::InterlockedIncrement
# define DYN_DETAIL_BOOST_INTERLOCKED_DECREMENT ::dyn_detail::boost::detail::InterlockedDecrement
# define DYN_DETAIL_BOOST_INTERLOCKED_COMPARE_EXCHANGE ::dyn_detail::boost::detail::InterlockedCompareExchange
# define DYN_DETAIL_BOOST_INTERLOCKED_EXCHANGE ::dyn_detail::boost::detail::InterlockedExchange
# define DYN_DETAIL_BOOST_INTERLOCKED_EXCHANGE_ADD ::dyn_detail::boost::detail::InterlockedExchangeAdd

# define DYN_DETAIL_BOOST_INTERLOCKED_COMPARE_EXCHANGE_POINTER(dest,exchange,compare) \
    ((void*)DYN_DETAIL_BOOST_INTERLOCKED_COMPARE_EXCHANGE((long volatile*)(dest),(long)(exchange),(long)(compare)))
# define DYN_DETAIL_BOOST_INTERLOCKED_EXCHANGE_POINTER(dest,exchange) \
    ((void*)DYN_DETAIL_BOOST_INTERLOCKED_EXCHANGE((long volatile*)(dest),(long)(exchange)))

#else

# error "Interlocked intrinsics not available"

#endif

#endif // #ifndef DYN_DETAIL_BOOST_DETAIL_INTERLOCKED_HPP_INCLUDED
