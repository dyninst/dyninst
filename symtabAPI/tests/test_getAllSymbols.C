#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "symtabAPI/h/Dyn_Symtab.h"
#include "symtabAPI/h/Dyn_Archive.h"
#define logerror printf

static int mutatorTest(Dyn_Symtab *symtab)
{
	vector <Dyn_Symbol *>syms;
	
	/*********************************************************************************
		Dyn_Symtab::getAllSymbolsByType: ST_UNKNOWN
	*********************************************************************************/	
	if(!symtab->getAllSymbolsByType(syms,Dyn_Symbol::ST_UNKNOWN))
	{
		logerror("unable to get all symbols\n");
		logerror("%s\n", Dyn_Symtab::printError(Dyn_Symtab::getLastSymtabError()).c_str());
		return -1;
	}
	if(syms.size() != 1)
		logerror("%s[%d]:  found %d symbols.\n",
		            __FILE__, __LINE__, syms.size());
	syms.clear();
	
	/*********************************************************************************
		Dyn_Symtab::getAllSymbolsByType: ST_FUNCTION
	*********************************************************************************/	
	if(!symtab->getAllSymbolsByType(syms,Dyn_Symbol::ST_FUNCTION))
	{
		logerror("unable to get all Functions\n");
		logerror("%s\n", Dyn_Symtab::printError(Dyn_Symtab::getLastSymtabError()).c_str());
		return -1;
	}
	if(syms.size() != 1)
		logerror("%s[%d]: found %d functions.\n",
		            __FILE__, __LINE__, syms.size());
	syms.clear();
	
	/*********************************************************************************
		Dyn_Symtab::getAllSymbolsByType: ST_OBJECT
	*********************************************************************************/	
	if(!symtab->getAllSymbolsByType(syms,Dyn_Symbol::ST_OBJECT))
	{
		logerror("unable to get all symbols\n");
		logerror("%s\n", Dyn_Symtab::printError(Dyn_Symtab::getLastSymtabError()).c_str());
		return -1;
	}
	if(syms.size() != 1)
		logerror("%s[%d]: found %d variables\n",
		            __FILE__, __LINE__, syms.size());
	syms.clear();
}

//extern "C" TEST_DLL_EXPORT int test1__mutatorMAIN(ParameterDict &param)
int main()
{
	// dprintf("Entered test1_1 mutatorMAIN()\n");
	string s = "/p/paradyn/development/giri/core/testsuite/i386-unknown-linux2.4/test1.mutatee_gcc";
	//string s = "/p/paradyn/development/giri/core/testsuite/rs6000-ibm-aix5.1/test1.mutatee_gcc";
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
