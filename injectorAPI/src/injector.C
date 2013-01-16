// A simple library injector

#include "Injector.h"
#include "codegen.h"
#include "Symtab.h"
#include <iostream>

using namespace Dyninst;
using namespace ProcControlAPI;
using namespace InjectorAPI;
using namespace SymtabAPI;
using namespace std;

Injector::Injector(ProcControlAPI::Process::ptr proc) :
   proc_(proc) {}

Injector::~Injector() {}

bool Injector::inject(std::string libname) {
   if (!checkIfExists(libname)) return false;

   Codegen codegen(proc_, libname);
   if (!codegen.generate()) return false;

   IRPC::ptr irpc = IRPC::createIRPC(codegen.buffer().start_ptr(),
                                     codegen.buffer().size(),
                                     codegen.buffer().startAddr());
   
   // Don't try to execute a library name...
   irpc->setStartOffset(codegen.startOffset());

   proc_->runIRPCSync(irpc);

   // We used to check if the library was loaded, but it's too risky with
   // symlinks etc. 
   return true;
}

bool Injector::checkIfExists(std::string name) {
   // Let's use Symtab
   Symtab *obj = NULL;
   bool ret = Symtab::openFile(obj, name);
   if (!ret || !obj) return false;
   // This would be nice but causes faults..
   //Symtab::closeSymtab(obj);
   return true;
}

bool Injector::libraryLoaded(std::string name) {
   if (proc_->isTerminated()) return false;

   LibraryPool& libs = proc_->libraries();
   
   for (auto li = libs.begin(); li != libs.end(); li++) {
      if ((*li)->getName() == name) return true;
   }
   return false;
}
