// A simple library injector

#include "loadLibrary/injector.h"
#include "loadLibrary/codegen.h"
#include <iostream>
#include "int_process.h"
#include "int_handler.h"
#include "Event.h"
#include <algorithm>
#include <vector>
#include "irpc.h"

using namespace Dyninst;
using namespace ProcControlAPI;
using namespace std;


#if defined(DEBUG_DISASSEMBLE)
#include "instructionAPI/h/Instruction.h"
#include "instructionAPI/h/InstructionDecoder.h"
using namespace InstructionAPI;
#endif

Injector::Injector(ProcControlAPI::Process *proc) :
   proc_(proc) {}

Injector::~Injector() {}

bool Injector::inject(std::string libname) {
   int_process *proc = proc_->llproc();
   pthrd_printf("Injecting %s into process %d\n", libname.c_str(), proc->getPid());
   if (!checkIfExists(libname)) {
      perr_printf("Library %s doesn't exist\n", libname.c_str());
      proc->setLastError(err_nofile, "File doesn't exist\n");
      return false;
   }

   Codegen codegen(proc_, libname);
   if (!codegen.generate()) {
      perr_printf("Could not generate code\n");
      proc->setLastError(err_internal, "Error in code generation");
      return false;
   }

   int_iRPC::ptr irpc = int_iRPC::ptr(new int_iRPC(codegen.buffer().start_ptr(),
                                                   codegen.buffer().size(),
                                                   false,
                                                   true,
                                                   codegen.buffer().startAddr()));
   // Don't try to execute a library name...
   irpc->setStartOffset(codegen.startOffset());

#if defined(DEBUG_DISASSEMBLE)
   cerr << "Setting starting offset to " << hex << codegen.startOffset() << endl;
   cerr << "And starting address is " << codegen.buffer().startAddr() << dec << endl;

   unsigned char *ptr = codegen.buffer().start_ptr();
   ptr += codegen.startOffset();
   Offset size = codegen.buffer().size() - codegen.startOffset();

   InstructionDecoder d(ptr, size, proc_->getArchitecture());

   Offset off = 0;
   while (off < size) {
     Instruction::Ptr insn = d.decode();
     cerr << hex << off + codegen.startOffset() + codegen.buffer().startAddr() << " : " << insn->format() << endl;
     off += insn->size();
   }

   off = 0;
   while (off < size) {
     cerr << hex <<  off + codegen.startOffset() + codegen.buffer().startAddr() << ": " << (int) ptr[off] << dec << endl;
     off++;
   }

#endif

   //Post, but doesn't start running yet.
   bool result = rpcMgr()->postRPCToProc(proc, irpc);
   if (!result) {
      pthrd_printf("Error posting RPC to process %d\n", proc->getPid());
      return false;
   }

   //Set the internal state so that this iRPC runs.
   int_thread *thr = irpc->thread();
   thr->getInternalState().desyncState(int_thread::running);
   irpc->setRestoreInternal(true);
   
   //Run the IRPC and wait for completion.
   proc->throwNopEvent();
   result = int_process::waitAndHandleEvents(true);
   if (!result) {
      perr_printf("Error waiting for and handling events\n");
      return false;
   }

   //TODO: Any mechanism for error checks?
   return true;
}                                                   

bool Injector::checkIfExists(std::string name) {
   SymReader *objSymReader = proc_->llproc()->getSymReader()->openSymbolReader(name);
   return (objSymReader != NULL);
}

bool Injector::libraryLoaded(std::string name) {
   if (proc_->isTerminated()) return false;

   LibraryPool& libs = proc_->libraries();
   
   for (auto li = libs.begin(); li != libs.end(); li++) {
      if ((*li)->getName() == name) return true;
   }
   return false;
}
