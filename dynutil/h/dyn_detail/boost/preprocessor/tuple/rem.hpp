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
# ifndef DYN_DETAIL_BOOST_PREPROCESSOR_TUPLE_REM_HPP
# define DYN_DETAIL_BOOST_PREPROCESSOR_TUPLE_REM_HPP
#
# include <dyn_detail/boost/preprocessor/config/config.hpp>
#
# /* DYN_DETAIL_BOOST_PP_TUPLE_REM */
#
# if ~DYN_DETAIL_BOOST_PP_CONFIG_FLAGS() & DYN_DETAIL_BOOST_PP_CONFIG_MWCC()
#    define DYN_DETAIL_BOOST_PP_TUPLE_REM(size) DYN_DETAIL_BOOST_PP_TUPLE_REM_I(size)
# else
#    define DYN_DETAIL_BOOST_PP_TUPLE_REM(size) DYN_DETAIL_BOOST_PP_TUPLE_REM_OO((size))
#    define DYN_DETAIL_BOOST_PP_TUPLE_REM_OO(par) DYN_DETAIL_BOOST_PP_TUPLE_REM_I ## par
# endif
#
# define DYN_DETAIL_BOOST_PP_TUPLE_REM_I(size) DYN_DETAIL_BOOST_PP_TUPLE_REM_ ## size
#
# define DYN_DETAIL_BOOST_PP_TUPLE_REM_0()
# define DYN_DETAIL_BOOST_PP_TUPLE_REM_1(a) a
# define DYN_DETAIL_BOOST_PP_TUPLE_REM_2(a, b) a, b
# define DYN_DETAIL_BOOST_PP_TUPLE_REM_3(a, b, c) a, b, c
# define DYN_DETAIL_BOOST_PP_TUPLE_REM_4(a, b, c, d) a, b, c, d
# define DYN_DETAIL_BOOST_PP_TUPLE_REM_5(a, b, c, d, e) a, b, c, d, e
# define DYN_DETAIL_BOOST_PP_TUPLE_REM_6(a, b, c, d, e, f) a, b, c, d, e, f
# define DYN_DETAIL_BOOST_PP_TUPLE_REM_7(a, b, c, d, e, f, g) a, b, c, d, e, f, g
# define DYN_DETAIL_BOOST_PP_TUPLE_REM_8(a, b, c, d, e, f, g, h) a, b, c, d, e, f, g, h
# define DYN_DETAIL_BOOST_PP_TUPLE_REM_9(a, b, c, d, e, f, g, h, i) a, b, c, d, e, f, g, h, i
# define DYN_DETAIL_BOOST_PP_TUPLE_REM_10(a, b, c, d, e, f, g, h, i, j) a, b, c, d, e, f, g, h, i, j
# define DYN_DETAIL_BOOST_PP_TUPLE_REM_11(a, b, c, d, e, f, g, h, i, j, k) a, b, c, d, e, f, g, h, i, j, k
# define DYN_DETAIL_BOOST_PP_TUPLE_REM_12(a, b, c, d, e, f, g, h, i, j, k, l) a, b, c, d, e, f, g, h, i, j, k, l
# define DYN_DETAIL_BOOST_PP_TUPLE_REM_13(a, b, c, d, e, f, g, h, i, j, k, l, m) a, b, c, d, e, f, g, h, i, j, k, l, m
# define DYN_DETAIL_BOOST_PP_TUPLE_REM_14(a, b, c, d, e, f, g, h, i, j, k, l, m, n) a, b, c, d, e, f, g, h, i, j, k, l, m, n
# define DYN_DETAIL_BOOST_PP_TUPLE_REM_15(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o) a, b, c, d, e, f, g, h, i, j, k, l, m, n, o
# define DYN_DETAIL_BOOST_PP_TUPLE_REM_16(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p) a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p
# define DYN_DETAIL_BOOST_PP_TUPLE_REM_17(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q) a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q
# define DYN_DETAIL_BOOST_PP_TUPLE_REM_18(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r) a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r
# define DYN_DETAIL_BOOST_PP_TUPLE_REM_19(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s) a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s
# define DYN_DETAIL_BOOST_PP_TUPLE_REM_20(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t) a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t
# define DYN_DETAIL_BOOST_PP_TUPLE_REM_21(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u) a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u
# define DYN_DETAIL_BOOST_PP_TUPLE_REM_22(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v) a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v
# define DYN_DETAIL_BOOST_PP_TUPLE_REM_23(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w) a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w
# define DYN_DETAIL_BOOST_PP_TUPLE_REM_24(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x) a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x
# define DYN_DETAIL_BOOST_PP_TUPLE_REM_25(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y) a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y
#
# /* DYN_DETAIL_BOOST_PP_TUPLE_REM_CTOR */
#
# if ~DYN_DETAIL_BOOST_PP_CONFIG_FLAGS() & DYN_DETAIL_BOOST_PP_CONFIG_EDG()
#    define DYN_DETAIL_BOOST_PP_TUPLE_REM_CTOR(size, tuple) DYN_DETAIL_BOOST_PP_TUPLE_REM_CTOR_I(DYN_DETAIL_BOOST_PP_TUPLE_REM(size), tuple)
# else
#    define DYN_DETAIL_BOOST_PP_TUPLE_REM_CTOR(size, tuple) DYN_DETAIL_BOOST_PP_TUPLE_REM_CTOR_D(size, tuple)
#    define DYN_DETAIL_BOOST_PP_TUPLE_REM_CTOR_D(size, tuple) DYN_DETAIL_BOOST_PP_TUPLE_REM_CTOR_I(DYN_DETAIL_BOOST_PP_TUPLE_REM(size), tuple)
# endif
#
# if ~DYN_DETAIL_BOOST_PP_CONFIG_FLAGS() & DYN_DETAIL_BOOST_PP_CONFIG_MWCC()
#    define DYN_DETAIL_BOOST_PP_TUPLE_REM_CTOR_I(ext, tuple) ext tuple
# else
#    define DYN_DETAIL_BOOST_PP_TUPLE_REM_CTOR_I(ext, tuple) DYN_DETAIL_BOOST_PP_TUPLE_REM_CTOR_OO((ext, tuple))
#    define DYN_DETAIL_BOOST_PP_TUPLE_REM_CTOR_OO(par) DYN_DETAIL_BOOST_PP_TUPLE_REM_CTOR_II ## par
#    define DYN_DETAIL_BOOST_PP_TUPLE_REM_CTOR_II(ext, tuple) ext ## tuple
# endif
#
# endif
