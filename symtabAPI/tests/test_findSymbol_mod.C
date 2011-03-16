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
#include "symtabAPI/h/Dyn_Archive.h"
#define logerror printf

static int mutatorTest(Dyn_Symtab *symtab)
{
	vector <Dyn_Symbol *>syms;
	Dyn_Module *mod;
        if(!symtab->findModule("test1.mutatee.c", mod))
        {
                logerror("Finding Module %s failed\n","test1.mutatee.c");
                logerror("%s\n", Dyn_Symtab::printError(Dyn_Symtab::getLastSymtabError()).c_str());
                return -1;
        }
	/*********************************************************************************
		findSymbolByType: function with ST_UNKNOWN
	*********************************************************************************/	
	if(!mod->findSymbolByType(syms,"func1_1",Dyn_Symbol::ST_UNKNOWN))
	{
		logerror("unable to find symbol %s\n","func1_1");
                logerror("%s\n", Dyn_Symtab::printError(Dyn_Symtab::getLastSymtabError()).c_str());
		return -1;
	}	
	if(syms.size() != 1)
		logerror("%s[%d]:  WARNING  : found %d symbols named %s.  Using the first.\n",
		            __FILE__, __LINE__, syms.size(), "func1_1");
	if(syms[0]->getPrettyName() != "func1_1")
	{
		logerror("Pretty Name for symbol %s does not match the pretty Name for the found Symbol(%s)\n","func1_1",syms[0]->getPrettyName().c_str());
		return -1;
	}
	syms.clear();
	
	/*********************************************************************************
		findSymbolByType: function with ST_FUNCTION
	*********************************************************************************/	
	if(!mod->findSymbolByType(syms,"func1_1",Dyn_Symbol::ST_FUNCTION))
	{
		logerror("unable to find Function %s\n","func1_1");
                logerror("%s\n", Dyn_Symtab::printError(Dyn_Symtab::getLastSymtabError()).c_str());
		return -1;
	}	
	if(syms.size() != 1)
		logerror("%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n",
		            __FILE__, __LINE__, syms.size(), "func1_1");
	if(syms[0]->getPrettyName() != "func1_1")
	{
		logerror("Pretty Name for function %s does not match the pretty Name for the found Symbol(%s)\n","func1_1",syms[0]->getPrettyName().c_str());
		return -1;
	}
	syms.clear();
	
	/*********************************************************************************
		findSymbolByType: Variable with ST_UNKNOWN
	*********************************************************************************/	
	if(!mod->findSymbolByType(syms,"globalVariable1_1",Dyn_Symbol::ST_UNKNOWN))
	{
		logerror("unable to find symbol %s\n","globalVariable1_1");
                logerror("%s\n", Dyn_Symtab::printError(Dyn_Symtab::getLastSymtabError()).c_str());
		return -1;
	}
	if(syms.size() != 1)
		logerror("%s[%d]:  WARNING  : found %d symbols named %s.  Using the first.\n",
		            __FILE__, __LINE__, syms.size(), "globalVariable1_1");
	if(syms[0]->getPrettyName() != "globalVariable1_1")
	{
		logerror("Pretty Name for symbol %s does not match the pretty Name for the found Symbol(%s)\n","globalVariable1_1",syms[0]->getPrettyName().c_str());
		return -1;
	}
	syms.clear();
	/*********************************************************************************
		findSymbolByType: Variable with ST_OBJECT
	*********************************************************************************/	
	if(!mod->findSymbolByType(syms,"globalVariable1_1",Dyn_Symbol::ST_OBJECT))
	{
		logerror("unable to find variable %s\n","globalVariable1_1");
                logerror("%s\n", Dyn_Symtab::printError(Dyn_Symtab::getLastSymtabError()).c_str());
		return -1;
	}
	if(syms.size() != 1)
		logerror("%s[%d]:  WARNING  : found %d variables named %s.  Using the first.\n",
		            __FILE__, __LINE__, syms.size(), "globalVariable1_1");
	if(syms[0]->getPrettyName() != "globalVariable1_1")
	{
		logerror("Pretty Name for variable %s does not match the pretty Name for the found Symbol(%s)\n","globalVariable1_1",syms[0]->getPrettyName().c_str());
		return -1;
	}
	syms.clear();
	/*********************************************************************************
		findSymbolByType: MODULE with ST_UNKNOWN
	*********************************************************************************/	
	if(!mod->findSymbolByType(syms,"test1.mutatee.c",Dyn_Symbol::ST_UNKNOWN))
	{
		logerror("unable to find symbol %s\n","test1.mutatee.c");
                logerror("%s\n", Dyn_Symtab::printError(Dyn_Symtab::getLastSymtabError()).c_str());
		return -1;
	}
	if(syms[0]->getPrettyName() != "test1.mutatee.c")
	{
		logerror("Pretty Name for symbol %s does not match the pretty Name for the found Symbol(%s)\n","test1.mutatee.c",syms[0]->getPrettyName().c_str());
		return -1;
	}
	syms.clear();

	/*********************************************************************************
		findSymbolByType: Variables with ST_UNKNOWN, isRegex - true
	*********************************************************************************/	
	if(!mod->findSymbolByType(syms,"globalVariable*",Dyn_Symbol::ST_UNKNOWN, false, true))
	{
		logerror("unable to find any variable of the form %s\n","globalVariable*");
                logerror("%s\n", Dyn_Symtab::printError(Dyn_Symtab::getLastSymtabError()).c_str());
		return -1;
	}
	if(syms.size() != 164)
		logerror("%s[%d]:  WARNING  : found %d symbols named %s.\n",
		            __FILE__, __LINE__, syms.size(), "globalVariable*");
	syms.clear();
	
	/*********************************************************************************
		findSymbolByType: Variables with ST_OBJECT, isRegex - true
	*********************************************************************************/	
	if(!mod->findSymbolByType(syms,"globalVariable*",Dyn_Symbol::ST_OBJECT, false, true))
	{
		logerror("unable to find any variable of the form %s\n","globalVariable*");
                logerror("%s\n", Dyn_Symtab::printError(Dyn_Symtab::getLastSymtabError()).c_str());
		return -1;
	}
	if(syms.size() != 164)
		logerror("%s[%d]:  WARNING  : found %d variable named %s.\n",
		            __FILE__, __LINE__, syms.size(), "globalVariable*");
	syms.clear();
	
	/*********************************************************************************
		findSymbolByType: Variables with ST_UNKNOWN, isRegex - true, check case - true
	*********************************************************************************/	
	//should return false. no variables of starting with globalvariable
	if(mod->findSymbolByType(syms,"globalvariable*",Dyn_Symbol::ST_UNKNOWN, false, true, true))
	{
		logerror("found variables of the form %s when nothing should have been found\n","globalVariable*");
		return -1;
	}
	syms.clear();
}

//extern "C" TEST_DLL_EXPORT int test1__mutatorMAIN(ParameterDict &param)
int main()
{
	// dprintf("Entered test1_1 mutatorMAIN()\n");
	string s = "/p/paradyn/development/giri/core/testsuite/i386-unknown-linux2.4/test1.mutatee_gcc";
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
