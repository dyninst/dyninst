/*
 * See the dyninst/COPYRIGHT file for copyright information.
 * 
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <stdio.h>
#include <stdexcept>
#include <string>
#include "solo_mutatee_boilerplate.h"

int excp_func2()
{
	throw std::runtime_error(std::string("exception2"));
   return 4;
}

int func2()
{
	try
	{
		excp_func2();
	}
	catch (const std::runtime_error &err)
	{
		fprintf(stderr, "%s\n", err.what());
	}
	return 4;
}

int excp_func1()
{
	throw std::runtime_error(std::string("exception1"));
   return 4;
}

int test_exception_mutatee() 
{
	try 
	{
		excp_func1();
	}

	catch (const std::runtime_error &err)
	{
		fprintf(stderr, "%s\n", err.what());
	}

   /*If mutatee should run, things go here.*/
   return 0;
}

