#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "symtabAPI/h/Dyn_Symtab.h"
#define logerror printf

static int mutatorTest(Dyn_Symtab *symtab)
{

#if defined(x86_64_unknown_linux2_4)
  ||defined(ia64_unknown_linux2_4)

	/*********************************************************************************
		Dyn_Symtab::getAddressWidth
	*********************************************************************************/	
	if(symtab->getAddressWidth() != 8)
	{
		logerror("**** Error:Address width for file %s is %d. It should be 8\n", symtab->file().c_str(), symtab->getAddressWidth());
		return -1;
	}
#else 

	/*********************************************************************************
		Dyn_Symtab::getAddressWidth
	*********************************************************************************/	
	if(symtab->getAddressWidth() != 4)
	{
		logerror("**** Error:Address width for file %s is %d. It should be 4\n", symtab->file().c_str(), symtab->getAddressWidth());
		return -1;
	}
#endif

}

//extern "C" TEST_DLL_EXPORT int test1__mutatorMAIN(ParameterDict &param)
int main()
{
	// dprintf("Entered test1_1 mutatorMAIN()\n");
	string s = "/p/paradyn/development/giri/core/testsuite/i386-unknown-linux2.4/test1.mutatee_g++";
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
