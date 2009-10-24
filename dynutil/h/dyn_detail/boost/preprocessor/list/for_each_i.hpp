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
# ifndef DYN_DETAIL_BOOST_PREPROCESSOR_LIST_LIST_FOR_EACH_I_HPP
# define DYN_DETAIL_BOOST_PREPROCESSOR_LIST_LIST_FOR_EACH_I_HPP
#
# include <dyn_detail/boost/preprocessor/arithmetic/inc.hpp>
# include <dyn_detail/boost/preprocessor/config/config.hpp>
# include <dyn_detail/boost/preprocessor/list/adt.hpp>
# include <dyn_detail/boost/preprocessor/repetition/for.hpp>
# include <dyn_detail/boost/preprocessor/tuple/elem.hpp>
# include <dyn_detail/boost/preprocessor/tuple/rem.hpp>
#
# /* DYN_DETAIL_BOOST_PP_LIST_FOR_EACH_I */
#
# if ~DYN_DETAIL_BOOST_PP_CONFIG_FLAGS() & DYN_DETAIL_BOOST_PP_CONFIG_EDG() && ~DYN_DETAIL_BOOST_PP_CONFIG_FLAGS() & DYN_DETAIL_BOOST_PP_CONFIG_MSVC()
#    define DYN_DETAIL_BOOST_PP_LIST_FOR_EACH_I(macro, data, list) DYN_DETAIL_BOOST_PP_FOR((macro, data, list, 0), DYN_DETAIL_BOOST_PP_LIST_FOR_EACH_I_P, DYN_DETAIL_BOOST_PP_LIST_FOR_EACH_I_O, DYN_DETAIL_BOOST_PP_LIST_FOR_EACH_I_M)
# else
#    define DYN_DETAIL_BOOST_PP_LIST_FOR_EACH_I(macro, data, list) DYN_DETAIL_BOOST_PP_LIST_FOR_EACH_I_I(macro, data, list)
#    define DYN_DETAIL_BOOST_PP_LIST_FOR_EACH_I_I(macro, data, list) DYN_DETAIL_BOOST_PP_FOR((macro, data, list, 0), DYN_DETAIL_BOOST_PP_LIST_FOR_EACH_I_P, DYN_DETAIL_BOOST_PP_LIST_FOR_EACH_I_O, DYN_DETAIL_BOOST_PP_LIST_FOR_EACH_I_M)
# endif
#
# if DYN_DETAIL_BOOST_PP_CONFIG_FLAGS() & DYN_DETAIL_BOOST_PP_CONFIG_STRICT()
#    define DYN_DETAIL_BOOST_PP_LIST_FOR_EACH_I_P(r, x) DYN_DETAIL_BOOST_PP_LIST_FOR_EACH_I_P_D x
#    define DYN_DETAIL_BOOST_PP_LIST_FOR_EACH_I_P_D(m, d, l, i) DYN_DETAIL_BOOST_PP_LIST_IS_CONS(l)
# else
#    define DYN_DETAIL_BOOST_PP_LIST_FOR_EACH_I_P(r, x) DYN_DETAIL_BOOST_PP_LIST_IS_CONS(DYN_DETAIL_BOOST_PP_TUPLE_ELEM(4, 2, x))
# endif
#
# if ~DYN_DETAIL_BOOST_PP_CONFIG_FLAGS() & DYN_DETAIL_BOOST_PP_CONFIG_MWCC()
#    define DYN_DETAIL_BOOST_PP_LIST_FOR_EACH_I_O(r, x) DYN_DETAIL_BOOST_PP_LIST_FOR_EACH_I_O_D x
#    define DYN_DETAIL_BOOST_PP_LIST_FOR_EACH_I_O_D(m, d, l, i) (m, d, DYN_DETAIL_BOOST_PP_LIST_REST(l), DYN_DETAIL_BOOST_PP_INC(i))
# else
#    define DYN_DETAIL_BOOST_PP_LIST_FOR_EACH_I_O(r, x) (DYN_DETAIL_BOOST_PP_TUPLE_ELEM(4, 0, x), DYN_DETAIL_BOOST_PP_TUPLE_ELEM(4, 1, x), DYN_DETAIL_BOOST_PP_LIST_REST(DYN_DETAIL_BOOST_PP_TUPLE_ELEM(4, 2, x)), DYN_DETAIL_BOOST_PP_INC(DYN_DETAIL_BOOST_PP_TUPLE_ELEM(4, 3, x)))
# endif
#
# if ~DYN_DETAIL_BOOST_PP_CONFIG_FLAGS() & DYN_DETAIL_BOOST_PP_CONFIG_EDG()
#    define DYN_DETAIL_BOOST_PP_LIST_FOR_EACH_I_M(r, x) DYN_DETAIL_BOOST_PP_LIST_FOR_EACH_I_M_D(r, DYN_DETAIL_BOOST_PP_TUPLE_ELEM(4, 0, x), DYN_DETAIL_BOOST_PP_TUPLE_ELEM(4, 1, x), DYN_DETAIL_BOOST_PP_TUPLE_ELEM(4, 2, x), DYN_DETAIL_BOOST_PP_TUPLE_ELEM(4, 3, x))
# else
#    define DYN_DETAIL_BOOST_PP_LIST_FOR_EACH_I_M(r, x) DYN_DETAIL_BOOST_PP_LIST_FOR_EACH_I_M_I(r, DYN_DETAIL_BOOST_PP_TUPLE_REM_4 x)
#    define DYN_DETAIL_BOOST_PP_LIST_FOR_EACH_I_M_I(r, x_e) DYN_DETAIL_BOOST_PP_LIST_FOR_EACH_I_M_D(r, x_e)
# endif
#
# define DYN_DETAIL_BOOST_PP_LIST_FOR_EACH_I_M_D(r, m, d, l, i) m(r, d, i, DYN_DETAIL_BOOST_PP_LIST_FIRST(l))
#
# /* DYN_DETAIL_BOOST_PP_LIST_FOR_EACH_I_R */
#
# if ~DYN_DETAIL_BOOST_PP_CONFIG_FLAGS() & DYN_DETAIL_BOOST_PP_CONFIG_EDG()
#    define DYN_DETAIL_BOOST_PP_LIST_FOR_EACH_I_R(r, macro, data, list) DYN_DETAIL_BOOST_PP_FOR_ ## r((macro, data, list, 0), DYN_DETAIL_BOOST_PP_LIST_FOR_EACH_I_P, DYN_DETAIL_BOOST_PP_LIST_FOR_EACH_I_O, DYN_DETAIL_BOOST_PP_LIST_FOR_EACH_I_M)
# else
#    define DYN_DETAIL_BOOST_PP_LIST_FOR_EACH_I_R(r, macro, data, list) DYN_DETAIL_BOOST_PP_LIST_FOR_EACH_I_R_I(r, macro, data, list)
#    define DYN_DETAIL_BOOST_PP_LIST_FOR_EACH_I_R_I(r, macro, data, list) DYN_DETAIL_BOOST_PP_FOR_ ## r((macro, data, list, 0), DYN_DETAIL_BOOST_PP_LIST_FOR_EACH_I_P, DYN_DETAIL_BOOST_PP_LIST_FOR_EACH_I_O, DYN_DETAIL_BOOST_PP_LIST_FOR_EACH_I_M)
# endif
#
# endif
