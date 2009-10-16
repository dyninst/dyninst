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
# ifndef DYN_DETAIL_BOOST_PREPROCESSOR_CONTROL_IF_HPP
# define DYN_DETAIL_BOOST_PREPROCESSOR_CONTROL_IF_HPP
#
# include <dyn_detail/boost/preprocessor/config/config.hpp>
# include <dyn_detail/boost/preprocessor/control/iif.hpp>
# include <dyn_detail/boost/preprocessor/logical/bool.hpp>
#
# /* DYN_DETAIL_BOOST_PP_IF */
#
# if ~DYN_DETAIL_BOOST_PP_CONFIG_FLAGS() & DYN_DETAIL_BOOST_PP_CONFIG_EDG()
#    define DYN_DETAIL_BOOST_PP_IF(cond, t, f) DYN_DETAIL_BOOST_PP_IIF(DYN_DETAIL_BOOST_PP_BOOL(cond), t, f)
# else
#    define DYN_DETAIL_BOOST_PP_IF(cond, t, f) DYN_DETAIL_BOOST_PP_IF_I(cond, t, f)
#    define DYN_DETAIL_BOOST_PP_IF_I(cond, t, f) DYN_DETAIL_BOOST_PP_IIF(DYN_DETAIL_BOOST_PP_BOOL(cond), t, f)
# endif
#
# endif
