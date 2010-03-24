#if !defined(MAILBOX_H_)
#define MAILBOX_H_

#include "Event.h"

namespace Dyninst {
namespace ProcControlAPI {

class Mailbox
{
public:
   Mailbox();
   virtual ~Mailbox();

   virtual void enqueue(Event::ptr ev) = 0;
   virtual Event::ptr dequeue(bool block) = 0;
   virtual Event::ptr peek() = 0;
   virtual unsigned int size() = 0;
};

extern Mailbox* mbox();

}
}

#endif
