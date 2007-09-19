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
