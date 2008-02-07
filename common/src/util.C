#include "dynutil/h/util.h"
namespace Dyninst {
DLLEXPORT unsigned addrHashCommon(const Address &addr)
{
   // inspired by hashs of string class

   register unsigned result = 5381;

   register Address accumulator = addr;
   while (accumulator > 0) {
      // We use 3 bits at a time from the address
      result = (result << 4) + result + (accumulator & 0x07);
      accumulator >>= 3;
   }

   return result;
}

DLLEXPORT unsigned addrHash(const Address & iaddr)
{
   return Dyninst::addrHashCommon(iaddr);
}

DLLEXPORT unsigned ptrHash(const void * iaddr)
{
   return Dyninst::addrHashCommon((Address)iaddr);
}

DLLEXPORT unsigned addrHash4(const Address &iaddr)
{
   // call when you know that the low 2 bits are 0 (meaning they contribute
   // nothing to an even hash distribution)
   return Dyninst::addrHashCommon(iaddr >> 2);
}

DLLEXPORT unsigned addrHash16(const Address &iaddr)
{
   // call when you know that the low 4 bits are 0 (meaning they contribute
   // nothing to an even hash distribution)
   return Dyninst::addrHashCommon(iaddr >> 4);
}

}
