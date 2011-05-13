#if !defined(MAILBOX_H_)
#define MAILBOX_H_

#include "Event.h"
#include "util.h"

namespace Dyninst {
namespace ProcControlAPI {

class PC_EXPORT Mailbox
{
public:
   Mailbox();
   virtual ~Mailbox();

   virtual void enqueue(Event::ptr ev, bool priority = false) = 0;
   virtual bool hasPriorityEvent() = 0;
   virtual Event::ptr dequeue(bool block) = 0;
   virtual Event::ptr peek() = 0;
   virtual unsigned int size() = 0;
   virtual bool hasUserEvent() = 0;
};

extern PC_EXPORT Mailbox* mbox();

}
}

#endif
