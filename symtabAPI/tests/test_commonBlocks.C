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

#include "symtabAPI/h/Symtab.h"
#include "symtabAPI/h/Archive.h"
#define logerror printf

using namespace Dyninst;
using namespace Dyninst::SymtabAPI;

//Currently only on solaris (test1.mutatee_f90) & AIX (test1.mutatee_xlf90)
static int mutatorTest(Symtab *symtab)
{
	/*********************************************************************************
		Dyn_Symtab::parseTypes
	*********************************************************************************/	

	Type *type;
	symtab->findVariableType(type, "globals");
	assert(type != NULL);
	
	//Test that it is indeed of type common
	typeCommon *ctyp = type->getCommonType();
	assert(ctyp != NULL);

	/* DEBUG */
	//cout << styp->getName() << ":"  << styp->getID() << ":" <<  styp->getSize() << ":" << styp->getDataClass() << endl;

	vector<Field *> *fields = ctyp->getComponents();

	/* DEBUG */
	for(unsigned i=0; i<fields->size(); i++)
	    cout << (*fields)[i]->getName() << ":" << (*fields)[i]->getType()->getDataClass() << endl;
}

//extern "C" TEST_DLL_EXPORT int test1__mutatorMAIN(ParameterDict &param)
int main(int argc, char **argv)
{
	// dprintf("Entered test1_1 mutatorMAIN()\n");
	//string s = "/p/paradyn/development/giri/core/testsuite/i386-unknown-linux2.4/test1.mutatee_g++";
	//string s = "test1.mutatee_gcc";
	//string s = "stripped_g++";
	string s = argv[1]; 	
	Symtab *symtab;
	bool err = Symtab::openFile(symtab,s);
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
