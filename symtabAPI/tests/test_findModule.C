#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "symtabAPI/h/Dyn_Symtab.h"
#include "symtabAPI/h/Dyn_Archive.h"
#define logerror printf

static int mutatorTest(Dyn_Symtab *symtab)
{
	Dyn_Module mod;
	
	/*********************************************************************************
		Dyn_Symtab::findModule
	*********************************************************************************/	
	if(!symtab->findModule("test1.mutatee_gcc", mod))
	{
		logerror("Finding Module %s failed\n","test1.mutatee.c");
		logerror("%s\n", symtab->printError(symtab->getLastSymtabError()).c_str());
		return -1;
	}
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
