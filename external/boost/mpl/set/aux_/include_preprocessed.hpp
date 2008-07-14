
// NO INCLUDE GUARDS, THE HEADER IS INTENDED FOR MULTIPLE INCLUSION

// Copyright Aleksey Gurtovoy 2001-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Source: /home/jaw/CVSROOT_20081103/CVSROOT/core/external/boost/mpl/set/aux_/include_preprocessed.hpp,v $
// $Date: 2008/07/14 21:54:57 $
// $Revision: 1.1 $

#include <boost/mpl/aux_/config/preprocessor.hpp>

#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/stringize.hpp>

#if !defined(BOOST_NEEDS_TOKEN_PASTING_OP_FOR_TOKENS_JUXTAPOSING)
#   define AUX778076_HEADER \
    plain/BOOST_MPL_PREPROCESSED_HEADER \
/**/
#else
#   define AUX778076_HEADER \
    BOOST_PP_CAT(plain,/)##BOOST_MPL_PREPROCESSED_HEADER \
/**/
#endif


#   include BOOST_PP_STRINGIZE(boost/mpl/set/aux_/preprocessed/AUX778076_HEADER)

#   undef AUX778076_HEADER

#undef BOOST_MPL_PREPROCESSED_HEADER
