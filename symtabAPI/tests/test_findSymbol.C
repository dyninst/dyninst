/*
 * Test program: tests to see if a symbol retrieval works
 * Usage : ./test_findSymbol <object file>
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
	
	/*********************************************************************************
		findSymbolByType: function with ST_UNKNOWN
	*********************************************************************************/	
	if(!symtab->findSymbolByType(syms,"func1_1",Dyn_Symbol::ST_UNKNOWN))
	{
		logerror("unable to find symbol %s\n","func1_1");
		logerror("%s\n", Dyn_Symtab::printError(Dyn_Symtab::getLastSymtabError()).c_str());
		return -1;
	}	
	if(syms.size() != 1)
		logerror("%s[%d]:  WARNING  : found %d symbols named %s.  Using the first.\n",
		            __FILE__, __LINE__, syms.size(), "func1_1");
	if(syms[0]->getPrettyName() != "func1_1")
	{
		logerror("Pretty Name for symbol %s does not match the pretty Name for the found Symbol(%s)\n","func1_1",syms[0]->getPrettyName().c_str());
		return -1;
	}
	syms.clear();
	
	/*********************************************************************************
		findSymbolByType: function with ST_FUNCTION
	*********************************************************************************/	
	if(!symtab->findSymbolByType(syms,"func1_1",Dyn_Symbol::ST_FUNCTION))
	{
		logerror("unable to find Function %s\n","func1_1");
		logerror("%s\n", Dyn_Symtab::printError(Dyn_Symtab::getLastSymtabError()).c_str());
		return -1;
	}	
	if(syms.size() != 1)
		logerror("%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n",
		            __FILE__, __LINE__, syms.size(), "func1_1");
	if(syms[0]->getPrettyName() != "func1_1")
	{
		logerror("Pretty Name for function %s does not match the pretty Name for the found Symbol(%s)\n","func1_1",syms[0]->getPrettyName().c_str());
		return -1;
	}
	syms.clear();
	
	/*********************************************************************************
		findSymbolByType: Variable with ST_UNKNOWN
	*********************************************************************************/	
	if(!symtab->findSymbolByType(syms,"globalVariable1_1",Dyn_Symbol::ST_UNKNOWN))
	{
		logerror("unable to find symbol %s\n","globalVariable1_1");
		logerror("%s\n", Dyn_Symtab::printError(Dyn_Symtab::getLastSymtabError()).c_str());
		return -1;
	}
	if(syms.size() != 1)
		logerror("%s[%d]:  WARNING  : found %d symbols named %s.  Using the first.\n",
		            __FILE__, __LINE__, syms.size(), "globalVariable1_1");
	if(syms[0]->getPrettyName() != "globalVariable1_1")
	{
		logerror("Pretty Name for symbol %s does not match the pretty Name for the found Symbol(%s)\n","globalVariable1_1",syms[0]->getPrettyName().c_str());
		return -1;
	}
	syms.clear();
	/*********************************************************************************
		findSymbolByType: Variable with ST_OBJECT
	*********************************************************************************/	
	if(!symtab->findSymbolByType(syms,"globalVariable1_1",Dyn_Symbol::ST_OBJECT))
	{
		logerror("unable to find variable %s\n","globalVariable1_1");
		logerror("%s\n", Dyn_Symtab::printError(Dyn_Symtab::getLastSymtabError()).c_str());
		return -1;
	}
	if(syms.size() != 1)
		logerror("%s[%d]:  WARNING  : found %d variables named %s.  Using the first.\n",
		            __FILE__, __LINE__, syms.size(), "globalVariable1_1");
	if(syms[0]->getPrettyName() != "globalVariable1_1")
	{
		logerror("Pretty Name for variable %s does not match the pretty Name for the found Symbol(%s)\n","globalVariable1_1",syms[0]->getPrettyName().c_str());
		return -1;
	}
	syms.clear();

#if defined(rs6000_ibm_aix5_1) || defined(rs6000_ibm_aix4_1)
	/*********************************************************************************
		findSymbolByType: MODULE with ST_UNKNOWN
	*********************************************************************************/	
	if(!symtab->findSymbolByType(syms,"../src/test1.mutatee.c",Dyn_Symbol::ST_UNKNOWN))
	{
		logerror("unable to find symbol %s\n","test1.mutatee.c");
		logerror("%s\n", Dyn_Symtab::printError(Dyn_Symtab::getLastSymtabError()).c_str());
		return -1;
	}
	if(syms[0]->getPrettyName() != "../src/test1.mutatee.c")
	{
		logerror("Pretty Name for symbol %s does not match the pretty Name for the found Symbol(%s)\n","test1.mutatee.c",syms[0]->getPrettyName().c_str());
		return -1;
	}
	syms.clear();

	/*********************************************************************************
		findSymbolByType: MODULE with ST_MODULE
	*********************************************************************************/	
	if(!symtab->findSymbolByType(syms,"../src/test1.mutatee.c",Dyn_Symbol::ST_MODULE))
	{
		logerror("unable to find module %s\n","test1.mutatee.c");
		logerror("%s\n", Dyn_Symtab::printError(Dyn_Symtab::getLastSymtabError()).c_str());
		return -1;
	}
	if(syms[0]->getPrettyName() != "../src/test1.mutatee.c")
	{
		logerror("Pretty Name for module %s does not match the pretty Name for the found Symbol(%s)\n","test1.mutatee.c",syms[0]->getPrettyName().c_str());
		return -1;
	}
	syms.clear();
#else
	/*********************************************************************************
		findSymbolByType: MODULE with ST_UNKNOWN
	*********************************************************************************/	
	if(!symtab->findSymbolByType(syms,"test1.mutatee.c",Dyn_Symbol::ST_UNKNOWN))
	{
		logerror("unable to find symbol %s\n","test1.mutatee.c");
		logerror("%s\n", Dyn_Symtab::printError(Dyn_Symtab::getLastSymtabError()).c_str());
		return -1;
	}
	if(syms[0]->getPrettyName() != "test1.mutatee.c")
	{
		logerror("Pretty Name for symbol %s does not match the pretty Name for the found Symbol(%s)\n","test1.mutatee.c",syms[0]->getPrettyName().c_str());
		return -1;
	}
	syms.clear();
	/*********************************************************************************
		findSymbolByType: MODULE with ST_MODULE
	*********************************************************************************/	
	if(!symtab->findSymbolByType(syms,"test1.mutatee.c",Dyn_Symbol::ST_MODULE))
	{
		logerror("unable to find module %s\n","test1.mutatee.c");
		logerror("%s\n", Dyn_Symtab::printError(Dyn_Symtab::getLastSymtabError()).c_str());
		return -1;
	}
	if(syms[0]->getPrettyName() != "test1.mutatee.c")
	{
		logerror("Pretty Name for module %s does not match the pretty Name for the found Symbol(%s)\n","test1.mutatee.c",syms[0]->getPrettyName().c_str());
		return -1;
	}
	syms.clear();
#endif
	/*********************************************************************************
		findSymbolByType: Variables with ST_UNKNOWN, isRegex - true
	*********************************************************************************/	
	if(!symtab->findSymbolByType(syms,"globalVariable*",Dyn_Symbol::ST_UNKNOWN, false, true))
	{
		logerror("unable to find any variable of the form %s\n","globalVariable*");
		logerror("%s\n", Dyn_Symtab::printError(Dyn_Symtab::getLastSymtabError()).c_str());
		return -1;
	}
	if(syms.size() != 164)
		logerror("%s[%d]:  WARNING  : found %d symbols named %s.\n",
		            __FILE__, __LINE__, syms.size(), "globalVariable*");
	syms.clear();

	/*********************************************************************************
		findSymbolByType: Variables with ST_OBJECT, isRegex - true
	*********************************************************************************/	
	if(!symtab->findSymbolByType(syms,"globalVariable*",Dyn_Symbol::ST_OBJECT, false, true))
	{
		logerror("unable to find any variable of the form %s\n","globalVariable*");
		logerror("%s\n", Dyn_Symtab::printError(Dyn_Symtab::getLastSymtabError()).c_str());
		return -1;
	}
	if(syms.size() != 164)
		logerror("%s[%d]:  WARNING  : found %d variable named %s.\n",
		            __FILE__, __LINE__, syms.size(), "globalVariable*");
	syms.clear();
	
	/*********************************************************************************
		findSymbolByType: Variables with ST_UNKNOWN, isRegex - true, check case - true
	*********************************************************************************/	
	//should return false. no variables of starting with globalvariable
	if(symtab->findSymbolByType(syms,"globalvariable*",Dyn_Symbol::ST_UNKNOWN, false, true, true))
	{
		logerror("found variables of the form %s when nothing should have been found\n","globalVariable*");
		return -1;
	}
	syms.clear();
	delete symtab;
}

int main(int argc, char **argv)
{
	// dprintf("Entered test1_1 mutatorMAIN()\n");
	//string s = "/p/paradyn/development/giri/core/testsuite/i386-unknown-linux2.4/test1.mutatee_gcc";
	//string s = "/p/paradyn/development/giri/core/testsuite/rs6000-ibm-aix5.1/test1.mutatee_gcc";
	string s = argv[1];
	cerr << "Checking file " << s << endl;
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
