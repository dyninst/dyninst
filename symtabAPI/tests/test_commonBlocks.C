#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "symtabAPI/h/Symtab.h"
#include "symtabAPI/h/Archive.h"
#define logerror printf

using namespace Dyninst;
using namespace Dyninst::SymtabAPI;

//Currently only on solaris (test1.mutatee_f90) & AIX (test1.mutatee_xlf90)
static int mutatorTest(Symtab *symtab)
{
	/*********************************************************************************
		Dyn_Symtab::parseTypes
	*********************************************************************************/	

	Type *type;
	symtab->findVariableType(type, "globals");
	assert(type != NULL);
	
	//Test that it is indeed of type common
	typeCommon *ctyp = type->getCommonType();
	assert(ctyp != NULL);

	/* DEBUG */
	//cout << styp->getName() << ":"  << styp->getID() << ":" <<  styp->getSize() << ":" << styp->getDataClass() << endl;

	vector<Field *> *fields = ctyp->getComponents();

	/* DEBUG */
	for(unsigned i=0; i<fields->size(); i++)
	    cout << (*fields)[i]->getName() << ":" << (*fields)[i]->getType()->getDataClass() << endl;
}

//extern "C" TEST_DLL_EXPORT int test1__mutatorMAIN(ParameterDict &param)
int main(int argc, char **argv)
{
	// dprintf("Entered test1_1 mutatorMAIN()\n");
	//string s = "/p/paradyn/development/giri/core/testsuite/i386-unknown-linux2.4/test1.mutatee_g++";
	//string s = "test1.mutatee_gcc";
	//string s = "stripped_g++";
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
