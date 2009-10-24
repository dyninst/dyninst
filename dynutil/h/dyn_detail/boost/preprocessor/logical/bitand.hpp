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
# ifndef DYN_DETAIL_BOOST_PREPROCESSOR_LOGICAL_BITAND_HPP
# define DYN_DETAIL_BOOST_PREPROCESSOR_LOGICAL_BITAND_HPP
#
# include <dyn_detail/boost/preprocessor/config/config.hpp>
#
# /* DYN_DETAIL_BOOST_PP_BITAND */
#
# if ~DYN_DETAIL_BOOST_PP_CONFIG_FLAGS() & DYN_DETAIL_BOOST_PP_CONFIG_MWCC()
#    define DYN_DETAIL_BOOST_PP_BITAND(x, y) DYN_DETAIL_BOOST_PP_BITAND_I(x, y)
# else
#    define DYN_DETAIL_BOOST_PP_BITAND(x, y) DYN_DETAIL_BOOST_PP_BITAND_OO((x, y))
#    define DYN_DETAIL_BOOST_PP_BITAND_OO(par) DYN_DETAIL_BOOST_PP_BITAND_I ## par
# endif
#
# if ~DYN_DETAIL_BOOST_PP_CONFIG_FLAGS() & DYN_DETAIL_BOOST_PP_CONFIG_MSVC()
#    define DYN_DETAIL_BOOST_PP_BITAND_I(x, y) DYN_DETAIL_BOOST_PP_BITAND_ ## x ## y
# else
#    define DYN_DETAIL_BOOST_PP_BITAND_I(x, y) DYN_DETAIL_BOOST_PP_BITAND_ID(DYN_DETAIL_BOOST_PP_BITAND_ ## x ## y)
#    define DYN_DETAIL_BOOST_PP_BITAND_ID(res) res
# endif
#
# define DYN_DETAIL_BOOST_PP_BITAND_00 0
# define DYN_DETAIL_BOOST_PP_BITAND_01 0
# define DYN_DETAIL_BOOST_PP_BITAND_10 0
# define DYN_DETAIL_BOOST_PP_BITAND_11 1
#
# endif
