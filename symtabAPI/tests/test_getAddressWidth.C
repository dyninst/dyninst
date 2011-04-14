/*
 * Copyright (c) 1996-2011 Barton P. Miller
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "symtabAPI/h/Dyn_Symtab.h"
#define logerror printf

static int mutatorTest(Dyn_Symtab *symtab)
{

#if defined(x86_64_unknown_linux2_4)
  ||defined(ia64_unknown_linux2_4)

	/*********************************************************************************
		Dyn_Symtab::getAddressWidth
	*********************************************************************************/	
	if(symtab->getAddressWidth() != 8)
	{
		logerror("**** Error:Address width for file %s is %d. It should be 8\n", symtab->file().c_str(), symtab->getAddressWidth());
		return -1;
	}
#else 

	/*********************************************************************************
		Dyn_Symtab::getAddressWidth
	*********************************************************************************/	
	if(symtab->getAddressWidth() != 4)
	{
		logerror("**** Error:Address width for file %s is %d. It should be 4\n", symtab->file().c_str(), symtab->getAddressWidth());
		return -1;
	}
#endif

}

//extern "C" TEST_DLL_EXPORT int test1__mutatorMAIN(ParameterDict &param)
int main()
{
	// dprintf("Entered test1_1 mutatorMAIN()\n");
	string s = "/p/paradyn/development/giri/core/testsuite/i386-unknown-linux2.4/test1.mutatee_g++";
	Dyn_Symtab *symtab;
	bool err = Dyn_Symtab::openFile(s,symtab);
        //symtab = param["symtab"]->getPtr();
	// Get log file pointers
	//FILE *outlog = (FILE *)(param["outlog"]->getPtr());
	//FILE *errlog = (FILE *)(param["errlog"]->getPtr());
	//setOutputLog(outlog);
	//setErrorLog(errlog);
	// Read the program's image and get an associated image object
	// Run mutator code
	return mutatorTest(symtab);
}
