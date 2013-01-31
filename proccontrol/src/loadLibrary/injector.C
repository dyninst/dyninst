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

Injector::Injector(ProcControlAPI::Process *proc) :
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


   bool oldSilent = proc_->llproc()->isRunningSilent();
   proc_->llproc()->setRunningSilent(true);

   std::set<Library::ptr> oldLibs;
   for (auto iter = proc_->libraries().begin(); iter != proc_->libraries().end(); ++iter) {
	   oldLibs.insert((*iter));
   }

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

   proc_->llproc()->setRunningSilent(oldSilent);

   std::set<Library::ptr> addedLibs;
   std::set<Library::ptr> removedLibs;
   
   for (auto iter = proc_->libraries().begin(); iter != proc_->libraries().end(); ++iter) {
	   if (oldLibs.find(*iter) == oldLibs.end()) {
		   addedLibs.insert(*iter);
	   }
   }

   EventLibrary::ptr lib_event = EventLibrary::ptr(new EventLibrary(addedLibs, removedLibs));
   lib_event->setThread(irpc->llrpc()->rpc->thread()->thread());
   lib_event->setProcess(proc_->llproc()->proc());
   lib_event->setSyncType(Event::sync_thread);
   
   HandleCallbacks *cbhandler = HandleCallbacks::getCB();
   cbhandler->handleEvent(lib_event);

   // We used to check if the library was loaded, but it's too risky with
   // symlinks etc. 
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
