//
//  boost/assert.hpp - DYN_DETAIL_BOOST_ASSERT(expr)
//
//  Copyright (c) 2001, 2002 Peter Dimov and Multi Media Ltd.
//  Copyright (c) 2007 Peter Dimov
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
//  Note: There are no include guards. This is intentional.
//
//  See http://www.boost.org/libs/utility/assert.html for documentation.
//

#undef DYN_DETAIL_BOOST_ASSERT

#if defined(DYN_DETAIL_BOOST_DISABLE_ASSERTS)

# define DYN_DETAIL_BOOST_ASSERT(expr) ((void)0)

#elif defined(DYN_DETAIL_BOOST_ENABLE_ASSERT_HANDLER)

#include <dyn_detail/boost/current_function.hpp>

namespace dyn_detail
{
  
namespace boost
{

void assertion_failed(char const * expr, char const * function, char const * file, long line); // user defined

} // namespace boost
 
} // namespace dyn_detail

#define DYN_DETAIL_BOOST_ASSERT(expr) ((expr)? ((void)0): ::boost::assertion_failed(#expr, DYN_DETAIL_BOOST_CURRENT_FUNCTION, __FILE__, __LINE__))

#else
# include <assert.h> // .h to support old libraries w/o <cassert> - effect is the same
# define DYN_DETAIL_BOOST_ASSERT(expr) assert(expr)
#endif

#undef DYN_DETAIL_BOOST_VERIFY

#if defined(DYN_DETAIL_BOOST_DISABLE_ASSERTS) || ( !defined(DYN_DETAIL_BOOST_ENABLE_ASSERT_HANDLER) && defined(NDEBUG) )

# define DYN_DETAIL_BOOST_VERIFY(expr) ((void)(expr))

#else

# define DYN_DETAIL_BOOST_VERIFY(expr) DYN_DETAIL_BOOST_ASSERT(expr)

#endif
