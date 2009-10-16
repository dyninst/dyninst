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
# ifndef DYN_DETAIL_BOOST_PREPROCESSOR_CAT_HPP
# define DYN_DETAIL_BOOST_PREPROCESSOR_CAT_HPP
#
# include <dyn_detail/boost/preprocessor/config/config.hpp>
#
# /* DYN_DETAIL_BOOST_PP_CAT */
#
# if ~DYN_DETAIL_BOOST_PP_CONFIG_FLAGS() & DYN_DETAIL_BOOST_PP_CONFIG_MWCC()
#    define DYN_DETAIL_BOOST_PP_CAT(a, b) DYN_DETAIL_BOOST_PP_CAT_I(a, b)
# else
#    define DYN_DETAIL_BOOST_PP_CAT(a, b) DYN_DETAIL_BOOST_PP_CAT_OO((a, b))
#    define DYN_DETAIL_BOOST_PP_CAT_OO(par) DYN_DETAIL_BOOST_PP_CAT_I ## par
# endif
#
# if ~DYN_DETAIL_BOOST_PP_CONFIG_FLAGS() & DYN_DETAIL_BOOST_PP_CONFIG_MSVC()
#    define DYN_DETAIL_BOOST_PP_CAT_I(a, b) a ## b
# else
#    define DYN_DETAIL_BOOST_PP_CAT_I(a, b) DYN_DETAIL_BOOST_PP_CAT_II(a ## b)
#    define DYN_DETAIL_BOOST_PP_CAT_II(res) res
# endif
#
# endif
