/*
 * Test program: nm equivalent (prints all symbol data)
 *
 * Usage: ./dynNM <object file>
 */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <iomanip>
#include <iostream>

#include "Symtab.h"
#include "Archive.h"
#define logerror printf

using namespace Dyninst;
using namespace SymtabAPI;
static int mutatorTest(Symtab *symtab)
{
  vector <Symbol *>syms;
  /*********************************************************************************
		Symtab::getAllSymbolsByType: ST_FUNCTION
  *********************************************************************************/	
  if(!symtab->getAllSymbolsByType(syms,Symbol::ST_FUNCTION))
  {
     logerror("unable to get all Functions\n");
     logerror("%s\n", Symtab::printError(Symtab::getLastSymtabError()).c_str());
     return -1;
  }
  printf("%s[%d]: found %lu functions.\n", __FILE__, __LINE__, syms.size());
  
  for (unsigned i = 0; i < syms.size(); i++) {
     if (syms[i]->getPrettyName() != std::string(""))
        cout << syms[i]->getPrettyName() << ": " << setbase(16) << syms[i]->getAddr() << endl;
     else
        cout << syms[i]->getName() << ": " << setbase(16) << syms[i]->getAddr() << endl;
  }

  syms.clear();


  syms.clear();
  return 0;
}

int main(int argc, char **argv)
{
  string s = argv[1];
  cerr << "Checking file " << s << endl;
  Symtab *symtab = NULL;
  bool result = Symtab::openFile(symtab,s);
  if (!result) {
    cerr << "Error: problem with opening file: " << 
       Symtab::printError(Symtab::getLastSymtabError()) << endl;
    cerr << s << "/" << symtab << endl;
    exit(-1);
  }
  return mutatorTest(symtab);
}
