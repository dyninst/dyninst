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
/*
 * Test Program : check for exceptions
 * Usage : ./test_Excps <object file>
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "symtabAPI/h/Dyn_Symtab.h"
#define logerror printf

static int mutatorTest(Dyn_Symtab *symtab)
{

#if defined(i386_unknown_linux2_0) \
|| defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */ \
|| defined(i386_unknown_solaris2_5)

	vector <Dyn_ExceptionBlock *>excps;
	
	/*********************************************************************************
		Dyn_Symtab::getAllModules
	*********************************************************************************/	
	if(!symtab->getAllExceptions(excps))
	{
		logerror("unable to get all exceptions\n");
		logerror("%s\n", Dyn_Symtab::printError(Dyn_Symtab::getLastSymtabError()).c_str());
		return -1;
	}
	if(excps.size() !=3)
	{
		logerror("%s[%d]:  **Error: found %d execptions instead of 3.\n",
		            __FILE__, __LINE__, excps.size());
	}
	Dyn_ExceptionBlock excp;
	OFFSET addr = excps[0]->tryStart();
	if(!symtab->findException(addr, excp))
	{
		logerror("unable to find exception\n");
		logerror("%s\n", Dyn_Symtab::printError(Dyn_Symtab::getLastSymtabError()).c_str());
		return -1;
	}
	addr = excps[0]->catchStart();
	if(!symtab->findCatchBlock(excp, addr);
	{
		logerror("unable to find catchBlock\n");
		logerror("%s\n", Dyn_Symtab::printError(Dyn_Symtab::getLastSymtabError()).c_str());
		return -1;
	}
	excps.clear();
	
#endif

}

int main(int argc, char **argv)
{
	// dprintf("Entered test1_1 mutatorMAIN()\n");
	//string s = "/p/paradyn/development/giri/core/testsuite/i386-unknown-linux2.4/test5.mutatee_g++";
	string s = argv[1];
	Dyn_Symtab *symtab = NULL;
	bool err = Dyn_Symtab::openFile(s,symtab);
	if (!err) {
	   cerr << "Error: problem with opening file: " << Dyn_Symtab::printError(Dyn_Symtab::getLastSymtabError()) << endl;
	   cerr << s << "/" << symtab << endl;
	   exit(1);
	}
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
