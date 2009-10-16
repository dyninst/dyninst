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
# ifndef DYN_DETAIL_BOOST_PREPROCESSOR_PUNCTUATION_COMMA_IF_HPP
# define DYN_DETAIL_BOOST_PREPROCESSOR_PUNCTUATION_COMMA_IF_HPP
#
# include <dyn_detail/boost/preprocessor/config/config.hpp>
# include <dyn_detail/boost/preprocessor/control/if.hpp>
# include <dyn_detail/boost/preprocessor/facilities/empty.hpp>
# include <dyn_detail/boost/preprocessor/punctuation/comma.hpp>
#
# /* DYN_DETAIL_BOOST_PP_COMMA_IF */
#
# if ~DYN_DETAIL_BOOST_PP_CONFIG_FLAGS() & DYN_DETAIL_BOOST_PP_CONFIG_EDG()
#    define DYN_DETAIL_BOOST_PP_COMMA_IF(cond) DYN_DETAIL_BOOST_PP_IF(cond, DYN_DETAIL_BOOST_PP_COMMA, DYN_DETAIL_BOOST_PP_EMPTY)()
# else
#    define DYN_DETAIL_BOOST_PP_COMMA_IF(cond) DYN_DETAIL_BOOST_PP_COMMA_IF_I(cond)
#    define DYN_DETAIL_BOOST_PP_COMMA_IF_I(cond) DYN_DETAIL_BOOST_PP_IF(cond, DYN_DETAIL_BOOST_PP_COMMA, DYN_DETAIL_BOOST_PP_EMPTY)()
# endif
#
# endif
