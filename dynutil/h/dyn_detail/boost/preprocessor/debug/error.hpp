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
# ifndef DYN_DETAIL_BOOST_PREPROCESSOR_DEBUG_ERROR_HPP
# define DYN_DETAIL_BOOST_PREPROCESSOR_DEBUG_ERROR_HPP
#
# include <dyn_detail/boost/preprocessor/cat.hpp>
# include <dyn_detail/boost/preprocessor/config/config.hpp>
#
# /* DYN_DETAIL_BOOST_PP_ERROR */
#
# if DYN_DETAIL_BOOST_PP_CONFIG_ERRORS
#    define DYN_DETAIL_BOOST_PP_ERROR(code) DYN_DETAIL_BOOST_PP_CAT(DYN_DETAIL_BOOST_PP_ERROR_, code)
# endif
#
# define DYN_DETAIL_BOOST_PP_ERROR_0x0000 DYN_DETAIL_BOOST_PP_ERROR(0x0000, DYN_DETAIL_BOOST_PP_INDEX_OUT_OF_BOUNDS)
# define DYN_DETAIL_BOOST_PP_ERROR_0x0001 DYN_DETAIL_BOOST_PP_ERROR(0x0001, DYN_DETAIL_BOOST_PP_WHILE_OVERFLOW)
# define DYN_DETAIL_BOOST_PP_ERROR_0x0002 DYN_DETAIL_BOOST_PP_ERROR(0x0002, DYN_DETAIL_BOOST_PP_FOR_OVERFLOW)
# define DYN_DETAIL_BOOST_PP_ERROR_0x0003 DYN_DETAIL_BOOST_PP_ERROR(0x0003, DYN_DETAIL_BOOST_PP_REPEAT_OVERFLOW)
# define DYN_DETAIL_BOOST_PP_ERROR_0x0004 DYN_DETAIL_BOOST_PP_ERROR(0x0004, DYN_DETAIL_BOOST_PP_LIST_FOLD_OVERFLOW)
# define DYN_DETAIL_BOOST_PP_ERROR_0x0005 DYN_DETAIL_BOOST_PP_ERROR(0x0005, DYN_DETAIL_BOOST_PP_SEQ_FOLD_OVERFLOW)
# define DYN_DETAIL_BOOST_PP_ERROR_0x0006 DYN_DETAIL_BOOST_PP_ERROR(0x0006, DYN_DETAIL_BOOST_PP_ARITHMETIC_OVERFLOW)
# define DYN_DETAIL_BOOST_PP_ERROR_0x0007 DYN_DETAIL_BOOST_PP_ERROR(0x0007, DYN_DETAIL_BOOST_PP_DIVISION_BY_ZERO)
#
# endif
