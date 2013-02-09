// A simple library injector

#include "Injector.h"
#include "codegen.h"
#include "Symtab.h"
#include <iostream>
#include "InstructionDecoder.h"

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

   // DEBUG

#if 0
   unsigned char *tmp = (unsigned char *)codegen.buffer().start_ptr();
   tmp += codegen.startOffset();

   InstructionAPI::InstructionDecoder d(tmp, codegen.buffer().size(), proc_->getArchitecture());
   Address foo = codegen.buffer().startAddr() + codegen.startOffset();
   InstructionAPI::Instruction::Ptr insn = d.decode();
   while(insn) {
      cerr << "\t" << hex << foo << ": " << insn->format(foo) << dec << endl;
      foo += insn->size();
      insn = d.decode();
   }

   foo = codegen.buffer().startAddr();
   tmp = (unsigned char *)codegen.buffer().start_ptr();
   for (unsigned i = 0; i < codegen.buffer().size(); ++i) {

      cerr << hex << foo + i << ": " << (unsigned int)(tmp[i]) << " " << tmp[i] << endl;
   }

   unsigned long *bar = (unsigned long *)tmp;
   for (unsigned i = 0; i < (codegen.buffer().size() / sizeof(unsigned long)); ++i) {
      cerr << hex << foo + i*sizeof(long) << ": " << bar[i] << dec << endl;
   }
#endif

   bool res = proc_->runIRPCSync(irpc);
   
   if (!res) {
      // ProcControl has an annoying case where an event handler wanted a process
      // to stay paused in the middle of an IRPC, which means ... we'll never finish
      // that IRPC in sync mode. So we continue the poor thing by hand. 
      bool done = false;
      while (!done) {

         if (proc_->isTerminated()) {
            fprintf(stderr, "IRPC on terminated process, ret false!\n");
            return false;
         }
         
         if (ProcControlAPI::getLastError() != ProcControlAPI::err_notrunning) {
            // Something went wrong
            return false;
         }
         else {
            irpc->continueStoppedIRPC();
            
            res = proc_->handleEvents(true);
            
            if (irpc->state() == ProcControlAPI::IRPC::Done) {
               done = true;
            }
         }
      }
   }

   

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
