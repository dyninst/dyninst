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
