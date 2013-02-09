// A library for injecting other libraries into other processes.

#if !defined(_INJECTOR_H_)
#define _INJECTOR_H_

#include "PCProcess.h"

namespace Dyninst {
namespace InjectorAPI {

class INJECTOR_EXPORT Injector {
  public:
   Injector(ProcControlAPI::Process::ptr proc);
   ~Injector();

   bool inject(std::string libname);

  private:
   bool libraryLoaded(std::string libname);
   bool checkIfExists(std::string libname);

   ProcControlAPI::Process::ptr proc_;
};

};
};

#endif
