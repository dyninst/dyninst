# /* Copyright (C) 2001
#  * Housemarque Oy
#  * http://www.housemarque.com
#  *
#  * Distributed under the Boost Software License, Version 1.0. (See
#  * accompanying file LICENSE_1_0.txt or copy at
#  * http://www.boost.org/LICENSE_1_0.txt)
#  */
#
# /* Revised by Paul Mensonides (2002) */
#
# /* See http://www.boost.org for most recent version. */
#
# ifndef DYN_DETAIL_BOOST_PREPROCESSOR_LOGICAL_AND_HPP
# define DYN_DETAIL_BOOST_PREPROCESSOR_LOGICAL_AND_HPP
#
# include <dyn_detail/boost/preprocessor/config/config.hpp>
# include <dyn_detail/boost/preprocessor/logical/bool.hpp>
# include <dyn_detail/boost/preprocessor/logical/bitand.hpp>
#
# /* DYN_DETAIL_BOOST_PP_AND */
#
# if ~DYN_DETAIL_BOOST_PP_CONFIG_FLAGS() & DYN_DETAIL_BOOST_PP_CONFIG_EDG()
#    define DYN_DETAIL_BOOST_PP_AND(p, q) DYN_DETAIL_BOOST_PP_BITAND(DYN_DETAIL_BOOST_PP_BOOL(p), DYN_DETAIL_BOOST_PP_BOOL(q))
# else
#    define DYN_DETAIL_BOOST_PP_AND(p, q) DYN_DETAIL_BOOST_PP_AND_I(p, q)
#    define DYN_DETAIL_BOOST_PP_AND_I(p, q) DYN_DETAIL_BOOST_PP_BITAND(DYN_DETAIL_BOOST_PP_BOOL(p), DYN_DETAIL_BOOST_PP_BOOL(q))
# endif
#
# endif
