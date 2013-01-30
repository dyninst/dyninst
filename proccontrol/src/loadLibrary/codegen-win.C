#include "loadLibrary/codegen.h"

using namespace Dyninst;
using namespace std;

bool Codegen::generateInt() {
   // Windows is utterly unlike Linux. 

   // Well, mostly. 

   Address loadLibraryA = findSymbolAddr("_LoadLibraryA@4", true, false);
   if (!loadLibraryA) {
      loadLibraryA = findSymbolAddr("_LoadLibraryA", true, false);
   }
   if (!loadLibraryA) {
      loadLibraryA = findSymbolAddr("LoadLibraryA", true, false);
   }
   if (!loadLibraryA) return false;

   std::vector<Address> arguments;
   Address libbase = copyString(libname_);
   
   arguments.push_back(libbase);

   // No noops needed on a real OS
   codeStart_ = buffer_.curAddr();
   generatePreamble();
   if (!generateCall(loadLibraryA, arguments)) return false;
   
   return true;
}
