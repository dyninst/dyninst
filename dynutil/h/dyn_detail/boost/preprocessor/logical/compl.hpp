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
# ifndef DYN_DETAIL_BOOST_PREPROCESSOR_LOGICAL_COMPL_HPP
# define DYN_DETAIL_BOOST_PREPROCESSOR_LOGICAL_COMPL_HPP
#
# include <dyn_detail/boost/preprocessor/config/config.hpp>
#
# /* DYN_DETAIL_BOOST_PP_COMPL */
#
# if ~DYN_DETAIL_BOOST_PP_CONFIG_FLAGS() & DYN_DETAIL_BOOST_PP_CONFIG_MWCC()
#    define DYN_DETAIL_BOOST_PP_COMPL(x) DYN_DETAIL_BOOST_PP_COMPL_I(x)
# else
#    define DYN_DETAIL_BOOST_PP_COMPL(x) DYN_DETAIL_BOOST_PP_COMPL_OO((x))
#    define DYN_DETAIL_BOOST_PP_COMPL_OO(par) DYN_DETAIL_BOOST_PP_COMPL_I ## par
# endif
#
# if ~DYN_DETAIL_BOOST_PP_CONFIG_FLAGS() & DYN_DETAIL_BOOST_PP_CONFIG_MSVC()
#    define DYN_DETAIL_BOOST_PP_COMPL_I(x) DYN_DETAIL_BOOST_PP_COMPL_ ## x
# else
#    define DYN_DETAIL_BOOST_PP_COMPL_I(x) DYN_DETAIL_BOOST_PP_COMPL_ID(DYN_DETAIL_BOOST_PP_COMPL_ ## x)
#    define DYN_DETAIL_BOOST_PP_COMPL_ID(id) id
# endif
#
# define DYN_DETAIL_BOOST_PP_COMPL_0 1
# define DYN_DETAIL_BOOST_PP_COMPL_1 0
#
# endif
