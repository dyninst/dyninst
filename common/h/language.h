/*
 * Copyright (c) 1996-2007 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */



// $Id: language.h,v

// Put C++ language specific code in here

#ifndef __LANGUAGE__
#define __LANGUAGE__

#if ! defined( TYPENAME )

#if defined( __GNUC__ )
#  define TYPENAME typename
#elif defined(__SUNPRO_CC)
#  define TYPENAME typename
#elif defined (__XLC__) || defined(__xlC__)
#define TYPENAME typename
#elif defined(mips_sgi_irix6_4)  // not sure what MACRO the IRIX compiler uses
#  define TYPENAME typename
#elif defined(_MSC_VER) && (_MSC_VER >= 1310)
    // Visual Studio .NET or greater
#  define TYPENAME typename
#else  // other compilers may not support the typename keyword yet
#define TYPENAME
#endif

#endif



#endif

#if !defined(DLLEXPORT)
#if defined (_MSC_VER)
/* If we're on Windows, we need to explicetely export these functions: */
	#if defined(DLL_BUILD)
		#define DLLEXPORT __declspec(dllexport)
	#else
		#define DLLEXPORT __declspec(dllimport)	
	#endif
#else
	#define DLLEXPORT
#endif
#endif

