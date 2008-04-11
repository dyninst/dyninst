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
