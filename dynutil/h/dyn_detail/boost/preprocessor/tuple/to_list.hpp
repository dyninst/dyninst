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
# ifndef DYN_DETAIL_BOOST_PREPROCESSOR_TUPLE_TO_LIST_HPP
# define DYN_DETAIL_BOOST_PREPROCESSOR_TUPLE_TO_LIST_HPP
#
# include <dyn_detail/boost/preprocessor/config/config.hpp>
#
# /* DYN_DETAIL_BOOST_PP_TUPLE_TO_LIST */
#
# if ~DYN_DETAIL_BOOST_PP_CONFIG_FLAGS() & DYN_DETAIL_BOOST_PP_CONFIG_MWCC()
#    define DYN_DETAIL_BOOST_PP_TUPLE_TO_LIST(size, tuple) DYN_DETAIL_BOOST_PP_TUPLE_TO_LIST_I(size, tuple)
#    if ~DYN_DETAIL_BOOST_PP_CONFIG_FLAGS() & DYN_DETAIL_BOOST_PP_CONFIG_MSVC()
#        define DYN_DETAIL_BOOST_PP_TUPLE_TO_LIST_I(s, t) DYN_DETAIL_BOOST_PP_TUPLE_TO_LIST_ ## s t
#    else
#        define DYN_DETAIL_BOOST_PP_TUPLE_TO_LIST_I(s, t) DYN_DETAIL_BOOST_PP_TUPLE_TO_LIST_II(DYN_DETAIL_BOOST_PP_TUPLE_TO_LIST_ ## s t)
#        define DYN_DETAIL_BOOST_PP_TUPLE_TO_LIST_II(res) res
#    endif
# else
#    define DYN_DETAIL_BOOST_PP_TUPLE_TO_LIST(size, tuple) DYN_DETAIL_BOOST_PP_TUPLE_TO_LIST_OO((size, tuple))
#    define DYN_DETAIL_BOOST_PP_TUPLE_TO_LIST_OO(par) DYN_DETAIL_BOOST_PP_TUPLE_TO_LIST_I ## par
#    define DYN_DETAIL_BOOST_PP_TUPLE_TO_LIST_I(s, t) DYN_DETAIL_BOOST_PP_TUPLE_TO_LIST_ ## s ## t
# endif
#
# define DYN_DETAIL_BOOST_PP_TUPLE_TO_LIST_0() DYN_DETAIL_BOOST_PP_NIL
# define DYN_DETAIL_BOOST_PP_TUPLE_TO_LIST_1(a) (a, DYN_DETAIL_BOOST_PP_NIL)
# define DYN_DETAIL_BOOST_PP_TUPLE_TO_LIST_2(a, b) (a, (b, DYN_DETAIL_BOOST_PP_NIL))
# define DYN_DETAIL_BOOST_PP_TUPLE_TO_LIST_3(a, b, c) (a, (b, (c, DYN_DETAIL_BOOST_PP_NIL)))
# define DYN_DETAIL_BOOST_PP_TUPLE_TO_LIST_4(a, b, c, d) (a, (b, (c, (d, DYN_DETAIL_BOOST_PP_NIL))))
# define DYN_DETAIL_BOOST_PP_TUPLE_TO_LIST_5(a, b, c, d, e) (a, (b, (c, (d, (e, DYN_DETAIL_BOOST_PP_NIL)))))
# define DYN_DETAIL_BOOST_PP_TUPLE_TO_LIST_6(a, b, c, d, e, f) (a, (b, (c, (d, (e, (f, DYN_DETAIL_BOOST_PP_NIL))))))
# define DYN_DETAIL_BOOST_PP_TUPLE_TO_LIST_7(a, b, c, d, e, f, g) (a, (b, (c, (d, (e, (f, (g, DYN_DETAIL_BOOST_PP_NIL)))))))
# define DYN_DETAIL_BOOST_PP_TUPLE_TO_LIST_8(a, b, c, d, e, f, g, h) (a, (b, (c, (d, (e, (f, (g, (h, DYN_DETAIL_BOOST_PP_NIL))))))))
# define DYN_DETAIL_BOOST_PP_TUPLE_TO_LIST_9(a, b, c, d, e, f, g, h, i) (a, (b, (c, (d, (e, (f, (g, (h, (i, DYN_DETAIL_BOOST_PP_NIL)))))))))
# define DYN_DETAIL_BOOST_PP_TUPLE_TO_LIST_10(a, b, c, d, e, f, g, h, i, j) (a, (b, (c, (d, (e, (f, (g, (h, (i, (j, DYN_DETAIL_BOOST_PP_NIL))))))))))
# define DYN_DETAIL_BOOST_PP_TUPLE_TO_LIST_11(a, b, c, d, e, f, g, h, i, j, k) (a, (b, (c, (d, (e, (f, (g, (h, (i, (j, (k, DYN_DETAIL_BOOST_PP_NIL)))))))))))
# define DYN_DETAIL_BOOST_PP_TUPLE_TO_LIST_12(a, b, c, d, e, f, g, h, i, j, k, l) (a, (b, (c, (d, (e, (f, (g, (h, (i, (j, (k, (l, DYN_DETAIL_BOOST_PP_NIL))))))))))))
# define DYN_DETAIL_BOOST_PP_TUPLE_TO_LIST_13(a, b, c, d, e, f, g, h, i, j, k, l, m) (a, (b, (c, (d, (e, (f, (g, (h, (i, (j, (k, (l, (m, DYN_DETAIL_BOOST_PP_NIL)))))))))))))
# define DYN_DETAIL_BOOST_PP_TUPLE_TO_LIST_14(a, b, c, d, e, f, g, h, i, j, k, l, m, n) (a, (b, (c, (d, (e, (f, (g, (h, (i, (j, (k, (l, (m, (n, DYN_DETAIL_BOOST_PP_NIL))))))))))))))
# define DYN_DETAIL_BOOST_PP_TUPLE_TO_LIST_15(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o) (a, (b, (c, (d, (e, (f, (g, (h, (i, (j, (k, (l, (m, (n, (o, DYN_DETAIL_BOOST_PP_NIL)))))))))))))))
# define DYN_DETAIL_BOOST_PP_TUPLE_TO_LIST_16(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p) (a, (b, (c, (d, (e, (f, (g, (h, (i, (j, (k, (l, (m, (n, (o, (p, DYN_DETAIL_BOOST_PP_NIL))))))))))))))))
# define DYN_DETAIL_BOOST_PP_TUPLE_TO_LIST_17(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q) (a, (b, (c, (d, (e, (f, (g, (h, (i, (j, (k, (l, (m, (n, (o, (p, (q, DYN_DETAIL_BOOST_PP_NIL)))))))))))))))))
# define DYN_DETAIL_BOOST_PP_TUPLE_TO_LIST_18(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r) (a, (b, (c, (d, (e, (f, (g, (h, (i, (j, (k, (l, (m, (n, (o, (p, (q, (r, DYN_DETAIL_BOOST_PP_NIL))))))))))))))))))
# define DYN_DETAIL_BOOST_PP_TUPLE_TO_LIST_19(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s) (a, (b, (c, (d, (e, (f, (g, (h, (i, (j, (k, (l, (m, (n, (o, (p, (q, (r, (s, DYN_DETAIL_BOOST_PP_NIL)))))))))))))))))))
# define DYN_DETAIL_BOOST_PP_TUPLE_TO_LIST_20(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t) (a, (b, (c, (d, (e, (f, (g, (h, (i, (j, (k, (l, (m, (n, (o, (p, (q, (r, (s, (t, DYN_DETAIL_BOOST_PP_NIL))))))))))))))))))))
# define DYN_DETAIL_BOOST_PP_TUPLE_TO_LIST_21(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u) (a, (b, (c, (d, (e, (f, (g, (h, (i, (j, (k, (l, (m, (n, (o, (p, (q, (r, (s, (t, (u, DYN_DETAIL_BOOST_PP_NIL)))))))))))))))))))))
# define DYN_DETAIL_BOOST_PP_TUPLE_TO_LIST_22(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v) (a, (b, (c, (d, (e, (f, (g, (h, (i, (j, (k, (l, (m, (n, (o, (p, (q, (r, (s, (t, (u, (v, DYN_DETAIL_BOOST_PP_NIL))))))))))))))))))))))
# define DYN_DETAIL_BOOST_PP_TUPLE_TO_LIST_23(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w) (a, (b, (c, (d, (e, (f, (g, (h, (i, (j, (k, (l, (m, (n, (o, (p, (q, (r, (s, (t, (u, (v, (w, DYN_DETAIL_BOOST_PP_NIL)))))))))))))))))))))))
# define DYN_DETAIL_BOOST_PP_TUPLE_TO_LIST_24(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x) (a, (b, (c, (d, (e, (f, (g, (h, (i, (j, (k, (l, (m, (n, (o, (p, (q, (r, (s, (t, (u, (v, (w, (x, DYN_DETAIL_BOOST_PP_NIL))))))))))))))))))))))))
# define DYN_DETAIL_BOOST_PP_TUPLE_TO_LIST_25(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y) (a, (b, (c, (d, (e, (f, (g, (h, (i, (j, (k, (l, (m, (n, (o, (p, (q, (r, (s, (t, (u, (v, (w, (x, (y, DYN_DETAIL_BOOST_PP_NIL)))))))))))))))))))))))))
#
# endif
