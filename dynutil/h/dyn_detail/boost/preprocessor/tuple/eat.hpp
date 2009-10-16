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
# ifndef DYN_DETAIL_BOOST_PREPROCESSOR_TUPLE_EAT_HPP
# define DYN_DETAIL_BOOST_PREPROCESSOR_TUPLE_EAT_HPP
#
# include <dyn_detail/boost/preprocessor/config/config.hpp>
#
# /* DYN_DETAIL_BOOST_PP_TUPLE_EAT */
#
# if ~DYN_DETAIL_BOOST_PP_CONFIG_FLAGS() & DYN_DETAIL_BOOST_PP_CONFIG_MWCC()
#    define DYN_DETAIL_BOOST_PP_TUPLE_EAT(size) DYN_DETAIL_BOOST_PP_TUPLE_EAT_I(size)
# else
#    define DYN_DETAIL_BOOST_PP_TUPLE_EAT(size) DYN_DETAIL_BOOST_PP_TUPLE_EAT_OO((size))
#    define DYN_DETAIL_BOOST_PP_TUPLE_EAT_OO(par) DYN_DETAIL_BOOST_PP_TUPLE_EAT_I ## par
# endif
#
# define DYN_DETAIL_BOOST_PP_TUPLE_EAT_I(size) DYN_DETAIL_BOOST_PP_TUPLE_EAT_ ## size
#
# define DYN_DETAIL_BOOST_PP_TUPLE_EAT_0()
# define DYN_DETAIL_BOOST_PP_TUPLE_EAT_1(a)
# define DYN_DETAIL_BOOST_PP_TUPLE_EAT_2(a, b)
# define DYN_DETAIL_BOOST_PP_TUPLE_EAT_3(a, b, c)
# define DYN_DETAIL_BOOST_PP_TUPLE_EAT_4(a, b, c, d)
# define DYN_DETAIL_BOOST_PP_TUPLE_EAT_5(a, b, c, d, e)
# define DYN_DETAIL_BOOST_PP_TUPLE_EAT_6(a, b, c, d, e, f)
# define DYN_DETAIL_BOOST_PP_TUPLE_EAT_7(a, b, c, d, e, f, g)
# define DYN_DETAIL_BOOST_PP_TUPLE_EAT_8(a, b, c, d, e, f, g, h)
# define DYN_DETAIL_BOOST_PP_TUPLE_EAT_9(a, b, c, d, e, f, g, h, i)
# define DYN_DETAIL_BOOST_PP_TUPLE_EAT_10(a, b, c, d, e, f, g, h, i, j)
# define DYN_DETAIL_BOOST_PP_TUPLE_EAT_11(a, b, c, d, e, f, g, h, i, j, k)
# define DYN_DETAIL_BOOST_PP_TUPLE_EAT_12(a, b, c, d, e, f, g, h, i, j, k, l)
# define DYN_DETAIL_BOOST_PP_TUPLE_EAT_13(a, b, c, d, e, f, g, h, i, j, k, l, m)
# define DYN_DETAIL_BOOST_PP_TUPLE_EAT_14(a, b, c, d, e, f, g, h, i, j, k, l, m, n)
# define DYN_DETAIL_BOOST_PP_TUPLE_EAT_15(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o)
# define DYN_DETAIL_BOOST_PP_TUPLE_EAT_16(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p)
# define DYN_DETAIL_BOOST_PP_TUPLE_EAT_17(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q)
# define DYN_DETAIL_BOOST_PP_TUPLE_EAT_18(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r)
# define DYN_DETAIL_BOOST_PP_TUPLE_EAT_19(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s)
# define DYN_DETAIL_BOOST_PP_TUPLE_EAT_20(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t)
# define DYN_DETAIL_BOOST_PP_TUPLE_EAT_21(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u)
# define DYN_DETAIL_BOOST_PP_TUPLE_EAT_22(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v)
# define DYN_DETAIL_BOOST_PP_TUPLE_EAT_23(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w)
# define DYN_DETAIL_BOOST_PP_TUPLE_EAT_24(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x)
# define DYN_DETAIL_BOOST_PP_TUPLE_EAT_25(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y)
#
# endif
