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

#include "Symtab.h"
#include "Archive.h"
#define logerror printf

using namespace Dyninst;
using namespace SymtabAPI;

static int mutatorTest(Symtab *symtab)
{
	vector <Symbol *>syms;
	
	/*********************************************************************************
		Symtab::getAllSymbolsByType: ST_UNKNOWN
	*********************************************************************************/	
	if(!symtab->getAllSymbolsByType(syms,Symbol::ST_UNKNOWN))
	{
		logerror("unable to get all symbols\n");
		logerror("%s\n", Symtab::printError(Symtab::getLastSymtabError()).c_str());
		return -1;
	}
	if(syms.size() != 1)
		logerror("%s[%d]:  found %d symbols.\n",
		            __FILE__, __LINE__, syms.size());
	syms.clear();
	
	/*********************************************************************************
		Symtab::getAllSymbolsByType: ST_FUNCTION
	*********************************************************************************/	
	if(!symtab->getAllSymbolsByType(syms,Symbol::ST_FUNCTION))
	{
		logerror("unable to get all Functions\n");
		logerror("%s\n", Symtab::printError(Symtab::getLastSymtabError()).c_str());
		return -1;
	}
	if(syms.size() != 1)
		logerror("%s[%d]: found %d functions.\n",
		            __FILE__, __LINE__, syms.size());
	syms.clear();
	
	/*********************************************************************************
		Symtab::getAllSymbolsByType: ST_OBJECT
	*********************************************************************************/	
	if(!symtab->getAllSymbolsByType(syms,Symbol::ST_OBJECT))
	{
		logerror("unable to get all symbols\n");
		logerror("%s\n", Symtab::printError(Symtab::getLastSymtabError()).c_str());
		return -1;
	}
	if(syms.size() != 1)
		logerror("%s[%d]: found %d variables\n",
		            __FILE__, __LINE__, syms.size());
	syms.clear();
}

//extern "C" TEST_DLL_EXPORT int test1__mutatorMAIN(ParameterDict &param)
int main(int argc, char *argv[])
{
	// dprintf("Entered test1_1 mutatorMAIN()\n");
	string s = argv[1];
	//string s = "/p/paradyn/development/giri/core/testsuite/rs6000-ibm-aix5.1/test1.mutatee_gcc";
	Symtab *symtab;
	bool err = Symtab::openFile(symtab, s);
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
