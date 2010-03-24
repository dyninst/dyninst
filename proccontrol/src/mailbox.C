#include "proccontrol/h/Mailbox.h"
#include "proccontrol/h/Event.h"
#include "proccontrol/h/PCErrors.h"

#include "common/h/dthread.h"

#include <queue>

using namespace std;
using namespace Dyninst;
using namespace ProcControlAPI;

class MailboxMT : public Mailbox
{
private:
   queue<Event::ptr> message_queue;
   CondVar message_cond;
public:
   MailboxMT();
   ~MailboxMT();

   virtual void enqueue(Event::ptr ev);
   virtual Event::ptr dequeue(bool block);
   virtual Event::ptr peek();
   virtual unsigned int size();
};

Mailbox::Mailbox()
{
}

Mailbox::~Mailbox()
{
}

MailboxMT::MailboxMT()
{
}

MailboxMT::~MailboxMT()
{
}

void MailboxMT::enqueue(Event::ptr ev)
{
   message_cond.lock();
   message_queue.push(ev);
   message_cond.broadcast();
   pthrd_printf("Added event %s to mailbox, size = %lu\n", ev->name().c_str(), 
                (unsigned long) message_queue.size());
   message_cond.unlock();
}

Event::ptr MailboxMT::peek()
{
   message_cond.lock();
   if (message_queue.empty())
   {
      message_cond.unlock();
      return Event::ptr();
   }
   Event::ptr ret = message_queue.front();
   message_cond.unlock();
   return ret;
}

Event::ptr MailboxMT::dequeue(bool block)
{
   message_cond.lock();
   if (message_queue.empty() && !block) {
      message_cond.unlock();
      return Event::ptr();
   }
   while (message_queue.empty()) {
      pthrd_printf("Blocking for events from mailbox, queue size = %lu\n", 
                   (unsigned long) message_queue.size());
      message_cond.wait();
   }
   Event::ptr ret = message_queue.front();
   message_queue.pop();
   message_cond.unlock();
   pthrd_printf("Returning event %s from mailbox\n", ret->name().c_str());
   return ret;
}

unsigned int MailboxMT::size()
{
   message_cond.lock();
   unsigned int result = (unsigned int) message_queue.size();
   message_cond.unlock();
   return result;
}

Mailbox* Dyninst::ProcControlAPI::mbox() 
{
   static Mailbox *mbox_int = NULL;
   if (!mbox_int) {
      mbox_int = static_cast<Mailbox *>(new MailboxMT());
   }
   return mbox_int;
}
