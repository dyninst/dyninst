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
#include <iomanip>
#include <iostream>

#include "Symtab.h"
#include "Archive.h"
#include "Module.h"

#define logerror printf

using namespace Dyninst;
using namespace Dyninst::SymtabAPI;

static int mutatorTest(Symtab *symtab)
{
	/*********************************************************************************
		Dyn_Symtab::parseLineInfo
	*********************************************************************************/	
	//Tests only test1.mutatee_gcc
	vector<Module *>mods;
	symtab->getAllModules(mods);
	//Iterating over the line information for all modules
	for(unsigned i = 0; i< mods.size();i++){
	    LineInformation *lineInformation = mods[i]->getLineInformation();
	    if(lineInformation) {
          LineInformation::const_iterator iter = lineInformation->begin();
          for(;iter!=lineInformation->end();iter++)
          {
             const std::pair<Offset, Offset> range = iter->first;
	    	    LineNoTuple line = iter->second;
	    	    cout << "0x" << setbase(16) << range.first << ":" << "0x" << range.second << setbase(10) << ":" << line.first << ":" << line.second << ":" << endl;
          }
	    }	
	}

	std::vector< LineNoTuple > lines;
	cout << lines.size() << endl;
   
   return 0;
}

//extern "C" TEST_DLL_EXPORT int test1__mutatorMAIN(ParameterDict &param)
int main(int argc, char **argv)
{
	string s = argv[1]; 	
	Symtab *symtab;
	bool err = Symtab::openFile(symtab,s);

	return mutatorTest(symtab);
}
