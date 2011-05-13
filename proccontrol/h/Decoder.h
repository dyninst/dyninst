#if !defined(DECODER_H_)
#define DECODER_H_

#include <vector>
#include "Event.h"
#include "util.h"

namespace Dyninst {
namespace ProcControlAPI {

class PC_EXPORT Decoder
{
 public:
   Decoder();
   virtual ~Decoder();

   static const unsigned int default_priority = 0x1000;

   virtual unsigned getPriority() const = 0;
   virtual bool decode(ArchEvent *archE, std::vector<Event::ptr> &events) = 0;
};

struct decoder_cmp
{
   bool operator()(const Decoder* a, const Decoder* b)
   {
      return a->getPriority() < b->getPriority();
   }
};

}
}

#endif
