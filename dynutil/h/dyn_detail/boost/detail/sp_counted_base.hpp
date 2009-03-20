#ifndef DYN_DETAIL_BOOST_DETAIL_SP_COUNTED_BASE_HPP_INCLUDED
#define DYN_DETAIL_BOOST_DETAIL_SP_COUNTED_BASE_HPP_INCLUDED

// MS compatible compilers support #pragma once

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

//
//  detail/sp_counted_base.hpp
//
//  Copyright 2005, 2006 Peter Dimov
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include <dyn_detail/boost/config.hpp>

#if defined( DYN_DETAIL_BOOST_SP_DISABLE_THREADS )
# include <dyn_detail/boost/detail/sp_counted_base_nt.hpp>

#elif defined( DYN_DETAIL_BOOST_SP_USE_SPINLOCK )
# include <dyn_detail/boost/detail/sp_counted_base_spin.hpp>

#elif defined( DYN_DETAIL_BOOST_SP_USE_PTHREADS )
# include <dyn_detail/boost/detail/sp_counted_base_pt.hpp>

#elif defined( DYN_DETAIL_BOOST_DISABLE_THREADS ) && !defined( DYN_DETAIL_BOOST_SP_ENABLE_THREADS ) && !defined( DYN_DETAIL_BOOST_DISABLE_WIN32 )
# include <dyn_detail/boost/detail/sp_counted_base_nt.hpp>

#elif defined( __GNUC__ ) && ( defined( __i386__ ) || defined( __x86_64__ ) )
# include <dyn_detail/boost/detail/sp_counted_base_gcc_x86.hpp>

#elif defined( __GNUC__ ) && defined( __ia64__ ) && !defined( __INTEL_COMPILER )
# include <dyn_detail/boost/detail/sp_counted_base_gcc_ia64.hpp>

#elif defined(__HP_aCC) && defined(__ia64)
# include <dyn_detail/boost/detail/sp_counted_base_acc_ia64.hpp>

#elif defined( __MWERKS__ ) && defined( __POWERPC__ )
# include <dyn_detail/boost/detail/sp_counted_base_cw_ppc.hpp>

#elif defined( __GNUC__ ) && ( defined( __powerpc__ ) || defined( __ppc__ ) || defined( __ppc ) )
# include <dyn_detail/boost/detail/sp_counted_base_gcc_ppc.hpp>

#elif defined(__GNUC__) && ( __GNUC__ * 100 + __GNUC_MINOR__ >= 401 ) && !defined( __arm__ ) && !defined( __hppa ) && ( !defined( __INTEL_COMPILER ) || defined( __ia64__ ) )
# include <dyn_detail/boost/detail/sp_counted_base_sync.hpp>

#elif defined(__GNUC__) && ( defined( __sparcv9 ) || ( defined( __sparcv8 ) && ( __GNUC__ * 100 + __GNUC_MINOR__ >= 402 ) ) )
# include <dyn_detail/boost/detail/sp_counted_base_gcc_sparc.hpp>

#elif defined( WIN32 ) || defined( _WIN32 ) || defined( __WIN32__ )
# include <dyn_detail/boost/detail/sp_counted_base_w32.hpp>

#elif !defined( DYN_DETAIL_BOOST_HAS_THREADS )
# include <dyn_detail/boost/detail/sp_counted_base_nt.hpp>

#else
# include <dyn_detail/boost/detail/sp_counted_base_spin.hpp>

#endif

#endif  // #ifndef DYN_DETAIL_BOOST_DETAIL_SP_COUNTED_BASE_HPP_INCLUDED
