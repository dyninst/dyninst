#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <iomanip>

#include "symtabAPI/h/Symtab.h"
#include "symtabAPI/h/Archive.h"
#define logerror printf

using namespace Dyninst;
using namespace Dyninst::SymtabAPI;

static int mutatorTest(Symtab *symtab)
{
	/*********************************************************************************
		Dyn_Symtab::parseLineInfo
	*********************************************************************************/	
	//Tests only test1.mutatee_gcc

	Module *mod;
	symtab->findModule(mod, "test1.mutatee.c");

	std::vector< std::pair< unsigned long, unsigned long > > ranges;
	symtab->getAddressRanges(ranges, "test1.mutatee.c", 597 );
	cout << ranges.size() << endl;

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
	symtab->getSourceLines( lines, ranges[0].first);
	
	cout << lines.size() << endl;
}

//extern "C" TEST_DLL_EXPORT int test1__mutatorMAIN(ParameterDict &param)
int main(int argc, char **argv)
{
	// dprintf("Entered test1_1 mutatorMAIN()\n");
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
