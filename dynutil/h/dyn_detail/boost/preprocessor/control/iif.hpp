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
# ifndef DYN_DETAIL_BOOST_PREPROCESSOR_CONTROL_IIF_HPP
# define DYN_DETAIL_BOOST_PREPROCESSOR_CONTROL_IIF_HPP
#
# include <dyn_detail/boost/preprocessor/config/config.hpp>
#
# if ~DYN_DETAIL_BOOST_PP_CONFIG_FLAGS() & DYN_DETAIL_BOOST_PP_CONFIG_MWCC()
#    define DYN_DETAIL_BOOST_PP_IIF(bit, t, f) DYN_DETAIL_BOOST_PP_IIF_I(bit, t, f)
# else
#    define DYN_DETAIL_BOOST_PP_IIF(bit, t, f) DYN_DETAIL_BOOST_PP_IIF_OO((bit, t, f))
#    define DYN_DETAIL_BOOST_PP_IIF_OO(par) DYN_DETAIL_BOOST_PP_IIF_I ## par
# endif
#
# if ~DYN_DETAIL_BOOST_PP_CONFIG_FLAGS() & DYN_DETAIL_BOOST_PP_CONFIG_MSVC()
#    define DYN_DETAIL_BOOST_PP_IIF_I(bit, t, f) DYN_DETAIL_BOOST_PP_IIF_ ## bit(t, f)
# else
#    define DYN_DETAIL_BOOST_PP_IIF_I(bit, t, f) DYN_DETAIL_BOOST_PP_IIF_II(DYN_DETAIL_BOOST_PP_IIF_ ## bit(t, f))
#    define DYN_DETAIL_BOOST_PP_IIF_II(id) id
# endif
#
# define DYN_DETAIL_BOOST_PP_IIF_0(t, f) f
# define DYN_DETAIL_BOOST_PP_IIF_1(t, f) t
#
# endif
