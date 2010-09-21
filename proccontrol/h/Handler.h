#if !defined(HANDLER_H_)
#define HANDLER_H_

#include "Event.h"

#include <set>
#include <vector>

namespace Dyninst {
namespace ProcControlAPI {

class Handler 
{
 protected:
   std::string name;
 public:
   Handler(std::string name_ = std::string(""));
   virtual ~Handler();

   typedef enum {
      ret_success,
      ret_async,
      ret_error
   } handler_ret_t;

   virtual handler_ret_t handleEvent(Event::ptr ev) = 0;
   virtual void getEventTypesHandled(std::vector<EventType> &etypes) = 0;
   virtual int getPriority() const;
   virtual Event::ptr convertEventForCB(Event::ptr orig);

   std::string getName() const;
   static const int PrePlatformPriority = 0x1000;
   static const int DefaultPriority = 0x1008;
   static const int PostPlatformPriority = 0x1010;
   static const int CallbackPriority = 0x1018;
   static const int PostCallbackPriority = 0x1020;
};

}
}

#endif
