/*
 * Test program: nm equivalent (prints all symbol data)
 *
 * Usage: ./dynNM <object file>
 */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <iomanip>

#include "Symtab.h"
#include "Archive.h"
#define logerror printf

using namespace DynSymtab;
static int mutatorTest(Symtab *symtab)
{
  vector <Symbol *>syms;
#if 0  
  /*********************************************************************************
		Symtab::getAllSymbolsByType: ST_UNKNOWN
  *********************************************************************************/	
  if(!symtab->getAllSymbolsByType(syms,Symbol::ST_UNKNOWN))
    {
      logerror("unable to get all symbols\n");
      logerror("%s\n", Dyn_Symtab::printError(Symtab::getLastSymtabError()).c_str());
      return -1;
    }
  if(syms.size() != 1)
    logerror("%s[%d]:  found %d symbols.\n",
	     __FILE__, __LINE__, syms.size());

  for (unsigned i = 0; i < syms.size(); i++) {
    cerr << *(syms[i]) << endl;
  }

  syms.clear();
#endif  
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

  for (unsigned i = 0; i < syms.size(); i++) {
    //cerr << (*syms[i]) << endl;
    cout << setbase(16) << syms[i]->getAddr() << endl;
  }
  cout << "size of symbols: " << setbase(10) << syms.size() << endl;

  syms.clear();
#if 0  
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

  for (unsigned i = 0; i < syms.size(); i++) {
    cerr << (*syms[i]) << endl;
  }
#endif  


  syms.clear();
}

int main(int argc, char **argv)
{
  // string s = "/p/paradyn/development/giri/core/testsuite/i386-unknown-linux2.4/test1.mutatee_gcc";
  //string s = "/p/paradyn/development/giri/core/testsuite/rs6000-ibm-aix5.1/test1.mutatee_gcc";
  string s = argv[1];
  cerr << "Checking file " << s << endl;
  Symtab *symtab = NULL;
  bool err = Symtab::openFile(symtab,s);
  if (!err) {
    cerr << "Error: problem with opening file: " << Symtab::printError(Symtab::getLastSymtabError()) << endl;
    cerr << s << "/" << symtab << endl;
    exit(1);
  }
  //symtab = param["symtab"]->getPtr();
  // Get log file pointers
  //FILE *outlog = (FILE *)(param["outlog"]->getPtr());
  //FILE *errlog = (FILE *)(param["errlog"]->getPtr());
  //setOutputLog(outlog);
  //setErrorLog(errlog);
  // Run mutator code
  return mutatorTest(symtab);
}
