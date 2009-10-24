# /* Copyright (C) 2001
#  * Housemarque Oy
#  * http://www.housemarque.com
#  *
#  * Distributed under the Boost Software License, Version 1.0. (See
#  * accompanying file LICENSE_1_0.txt or copy at
#  * http://www.boost.org/LICENSE_1_0.txt)
#  *
#  * See http://www.boost.org for most recent version.
#  */
#
# /* Revised by Paul Mensonides (2002) */
#
# ifndef DYN_DETAIL_BOOST_PREPROCESSOR_LIST_ADT_HPP
# define DYN_DETAIL_BOOST_PREPROCESSOR_LIST_ADT_HPP
#
# include <dyn_detail/boost/preprocessor/config/config.hpp>
# include <dyn_detail/boost/preprocessor/detail/is_binary.hpp>
# include <dyn_detail/boost/preprocessor/logical/compl.hpp>
# include <dyn_detail/boost/preprocessor/tuple/eat.hpp>
#
# /* DYN_DETAIL_BOOST_PP_LIST_CONS */
#
# define DYN_DETAIL_BOOST_PP_LIST_CONS(head, tail) (head, tail)
#
# /* DYN_DETAIL_BOOST_PP_LIST_NIL */
#
# define DYN_DETAIL_BOOST_PP_LIST_NIL DYN_DETAIL_BOOST_PP_NIL
#
# /* DYN_DETAIL_BOOST_PP_LIST_FIRST */
#
# define DYN_DETAIL_BOOST_PP_LIST_FIRST(list) DYN_DETAIL_BOOST_PP_LIST_FIRST_D(list)
#
# if ~DYN_DETAIL_BOOST_PP_CONFIG_FLAGS() & DYN_DETAIL_BOOST_PP_CONFIG_MWCC()
#    define DYN_DETAIL_BOOST_PP_LIST_FIRST_D(list) DYN_DETAIL_BOOST_PP_LIST_FIRST_I list
# else
#    define DYN_DETAIL_BOOST_PP_LIST_FIRST_D(list) DYN_DETAIL_BOOST_PP_LIST_FIRST_I ## list
# endif
#
# define DYN_DETAIL_BOOST_PP_LIST_FIRST_I(head, tail) head
#
# /* DYN_DETAIL_BOOST_PP_LIST_REST */
#
# define DYN_DETAIL_BOOST_PP_LIST_REST(list) DYN_DETAIL_BOOST_PP_LIST_REST_D(list)
#
# if ~DYN_DETAIL_BOOST_PP_CONFIG_FLAGS() & DYN_DETAIL_BOOST_PP_CONFIG_MWCC()
#    define DYN_DETAIL_BOOST_PP_LIST_REST_D(list) DYN_DETAIL_BOOST_PP_LIST_REST_I list
# else
#    define DYN_DETAIL_BOOST_PP_LIST_REST_D(list) DYN_DETAIL_BOOST_PP_LIST_REST_I ## list
# endif
#
# define DYN_DETAIL_BOOST_PP_LIST_REST_I(head, tail) tail
#
# /* DYN_DETAIL_BOOST_PP_LIST_IS_CONS */
#
# if DYN_DETAIL_BOOST_PP_CONFIG_FLAGS() & DYN_DETAIL_BOOST_PP_CONFIG_BCC()
#    define DYN_DETAIL_BOOST_PP_LIST_IS_CONS(list) DYN_DETAIL_BOOST_PP_LIST_IS_CONS_D(list)
#    define DYN_DETAIL_BOOST_PP_LIST_IS_CONS_D(list) DYN_DETAIL_BOOST_PP_LIST_IS_CONS_ ## list
#    define DYN_DETAIL_BOOST_PP_LIST_IS_CONS_(head, tail) 1
#    define DYN_DETAIL_BOOST_PP_LIST_IS_CONS_DYN_DETAIL_BOOST_PP_NIL 0
# else
#    define DYN_DETAIL_BOOST_PP_LIST_IS_CONS(list) DYN_DETAIL_BOOST_PP_IS_BINARY(list)
# endif
#
# /* DYN_DETAIL_BOOST_PP_LIST_IS_NIL */
#
# if ~DYN_DETAIL_BOOST_PP_CONFIG_FLAGS() & DYN_DETAIL_BOOST_PP_CONFIG_BCC()
#    define DYN_DETAIL_BOOST_PP_LIST_IS_NIL(list) DYN_DETAIL_BOOST_PP_COMPL(DYN_DETAIL_BOOST_PP_IS_BINARY(list))
# else
#    define DYN_DETAIL_BOOST_PP_LIST_IS_NIL(list) DYN_DETAIL_BOOST_PP_COMPL(DYN_DETAIL_BOOST_PP_LIST_IS_CONS(list))
# endif
#
# endif
