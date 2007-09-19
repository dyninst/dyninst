/*
* Test program: Adds a new symbol and checks for it 
*
* Usage: ./test_addSymbol <object file>
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
	Dyn_Section *ret;
	symtab->findSection(".text",ret);
	Dyn_Symbol *newSym = new Dyn_Symbol("func300_3","DEFAULT_MODULE",Dyn_Symbol::ST_FUNCTION,Dyn_Symbol::SL_GLOBAL,123456, ret, 4);
        if(!symtab->addSymbol(newSym))
	{
		logerror("Adding a new symbol failed %s\n","func1_1");
		logerror("%s\n", Dyn_Symtab::printError(Dyn_Symtab::getLastSymtabError()).c_str());
		return -1;
	}	
	/*********************************************************************************
		findSymbolByType: function with ST_UNKNOWN
	*********************************************************************************/	
	if(!symtab->findSymbolByType(syms,"func300_3",Dyn_Symbol::ST_UNKNOWN))
	{
		logerror("unable to find the added symbol %s\n","func300_3");
		logerror("%s\n", Dyn_Symtab::printError(Dyn_Symtab::getLastSymtabError()).c_str());
		return -1;
	}	
	if(syms[0]->getPrettyName() != "func300_3")
	{
		logerror("Pretty Name for the added symbol %s does not match the pretty Name for the found Symbol(%s)\n","func300_3",syms[0]->getPrettyName().c_str());
		return -1;
	}
	
	/*********************************************************************************
		findSymbolByType: function with ST_FUNCTION
	*********************************************************************************/	
	if(!symtab->findSymbolByType(syms,"func1_1",Dyn_Symbol::ST_FUNCTION))
	{
		logerror("unable to find the added Function %s\n","func300_3");
		logerror("%s\n", Dyn_Symtab::printError(Dyn_Symtab::getLastSymtabError()).c_str());
		return -1;
	}	
	if(syms[0]->getPrettyName() != "func300_3")
	{
		logerror("Pretty Name for the added function %s does not match the pretty Name for the found Symbol(%s)\n","func300_3",syms[0]->getPrettyName().c_str());
		return -1;
	}
	

	/*********************************************************************************
		findSymbolByType: Variables with ST_UNKNOWN, isRegex - true
	*********************************************************************************/	
	if(!symtab->findSymbolByType(syms,"func300*",Dyn_Symbol::ST_UNKNOWN, false, true))
	{
		logerror("unable to find the added symbol of the form %s\n","func300*");
		logerror("%s\n", Dyn_Symtab::printError(Dyn_Symtab::getLastSymtabError()).c_str());
		return -1;
	}
	syms.clear();
	
	/*********************************************************************************
		findSymbolByType: Variables with ST_OBJECT, isRegex - true
	*********************************************************************************/	
	if(!symtab->findSymbolByType(syms,"func300*",Dyn_Symbol::ST_FUNCTION, false, true))
	{
		logerror("unable to find the added function of the form %s\n","func300*");
		logerror("%s\n", Dyn_Symtab::printError(Dyn_Symtab::getLastSymtabError()).c_str());
		return -1;
	}
	syms.clear();

	//symtab->delSymbol(newSym);
}

//extern "C" TEST_DLL_EXPORT int test1__mutatorMAIN(ParameterDict &param)
int main(int argc, char **argv)
{
	// dprintf("Entered test1_1 mutatorMAIN()\n");
	string s = "/p/paradyn/development/giri/core/testsuite/i386-unknown-linux2.4/test1.mutatee_gcc";
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
