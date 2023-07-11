// A library for injecting other libraries into other processes.

#if !defined(_INJECTOR_H_)
#define _INJECTOR_H_

#include <string>
#include "PCProcess.h"

namespace Dyninst {
namespace ProcControlAPI {

class Injector {
  public:
   Injector(ProcControlAPI::Process *proc);
   ~Injector();

   bool inject(std::string libname);

  private:
   bool libraryLoaded(std::string libname);
   bool checkIfExists(std::string libname);

   ProcControlAPI::Process *proc_;
};

}
}

#endif
