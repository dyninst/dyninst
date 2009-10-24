# /* **************************************************************************
#  *                                                                          *
#  *     (C) Copyright Paul Mensonides 2002.
#  *     Distributed under the Boost Software License, Version 1.0. (See
#  *     accompanying file LICENSE_1_0.txt or copy at
#  *     http://www.boost.org/LICENSE_1_0.txt)
#  *                                                                          *
#  ************************************************************************** */
#
# /* See http://www.boost.org for most recent version. */
#
# ifndef DYN_DETAIL_BOOST_PREPROCESSOR_CONTROL_EXPR_IIF_HPP
# define DYN_DETAIL_BOOST_PREPROCESSOR_CONTROL_EXPR_IIF_HPP
#
# include <dyn_detail/boost/preprocessor/config/config.hpp>
#
# /* DYN_DETAIL_BOOST_PP_EXPR_IIF */
#
# if ~DYN_DETAIL_BOOST_PP_CONFIG_FLAGS() & DYN_DETAIL_BOOST_PP_CONFIG_MWCC()
#    define DYN_DETAIL_BOOST_PP_EXPR_IIF(bit, expr) DYN_DETAIL_BOOST_PP_EXPR_IIF_I(bit, expr)
# else
#    define DYN_DETAIL_BOOST_PP_EXPR_IIF(bit, expr) DYN_DETAIL_BOOST_PP_EXPR_IIF_OO((bit, expr))
#    define DYN_DETAIL_BOOST_PP_EXPR_IIF_OO(par) DYN_DETAIL_BOOST_PP_EXPR_IIF_I ## par
# endif
#
# define DYN_DETAIL_BOOST_PP_EXPR_IIF_I(bit, expr) DYN_DETAIL_BOOST_PP_EXPR_IIF_ ## bit(expr)
#
# define DYN_DETAIL_BOOST_PP_EXPR_IIF_0(expr)
# define DYN_DETAIL_BOOST_PP_EXPR_IIF_1(expr) expr
#
# endif
