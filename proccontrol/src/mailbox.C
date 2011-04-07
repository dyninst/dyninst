#include "proccontrol/h/Mailbox.h"
#include "proccontrol/h/Event.h"
#include "proccontrol/h/PCErrors.h"
#include "proccontrol/src/int_process.h"

#include "common/h/dthread.h"

#include <queue>

using namespace std;
using namespace Dyninst;
using namespace ProcControlAPI;

class MailboxMT : public Mailbox
{
private:
   queue<Event::ptr> message_queue;
   queue<Event::ptr> priority_message_queue; //Mostly used for async responses
   CondVar message_cond;
   unsigned numUserEvents;
public:
   MailboxMT();
   ~MailboxMT();

   virtual void enqueue(Event::ptr ev, bool priority = false);
   virtual Event::ptr dequeue(bool block);
   virtual Event::ptr peek();
   virtual unsigned int size();
   virtual bool hasPriorityEvent();
   virtual bool hasUserEvent();
};

Mailbox::Mailbox()
{
}

Mailbox::~Mailbox()
{
}

MailboxMT::MailboxMT()
    : numUserEvents(0)
{
}

MailboxMT::~MailboxMT()
{
}

void MailboxMT::enqueue(Event::ptr ev, bool priority)
{
   message_cond.lock();

   if (priority)
      priority_message_queue.push(ev);
   else
      message_queue.push(ev);

   message_cond.broadcast();
   pthrd_printf("Added event %s to mailbox, size = %lu + %lu = %lu\n", 
                ev->name().c_str(), 
                (unsigned long) message_queue.size(),
                (unsigned long) priority_message_queue.size(),
                (unsigned long) (message_queue.size() + priority_message_queue.size()));

   if( ev->userEvent() ) {
      numUserEvents++;
   }
     
   message_cond.unlock();

   if( ev->userEvent() ) {
       MTManager::eventqueue_cb_wrapper();
   }
}

Event::ptr MailboxMT::peek()
{
   message_cond.lock();
   queue<Event::ptr> &q = !priority_message_queue.empty() ? priority_message_queue : message_queue;
   if (q.empty())
   {
      message_cond.unlock();
      return Event::ptr();
   }
   Event::ptr ret = q.front();
   message_cond.unlock();
   return ret;
}

Event::ptr MailboxMT::dequeue(bool block)
{
   message_cond.lock();
   queue<Event::ptr> &q = !priority_message_queue.empty() ? priority_message_queue : message_queue;

   if (q.empty() && !block) {
      message_cond.unlock();
      return Event::ptr();
   }
   while (priority_message_queue.empty() && message_queue.empty()) {
      pthrd_printf("Blocking for events from mailbox, queue size = %lu\n", 
                   (unsigned long) message_queue.size());
      message_cond.wait();
   }

   queue<Event::ptr> &r = !priority_message_queue.empty() ? priority_message_queue : message_queue;
   Event::ptr ret = r.front();
   r.pop();

   if( ret->userEvent() ) {
       numUserEvents--;
   }

   message_cond.unlock();
   pthrd_printf("Returning event %s from mailbox\n", ret->name().c_str());
   return ret;
}

unsigned int MailboxMT::size()
{
   message_cond.lock();
   unsigned int result = (unsigned int) (message_queue.size() + priority_message_queue.size());
   message_cond.unlock();
   return result;
}

bool MailboxMT::hasPriorityEvent()
{
   message_cond.lock();
   bool result = !priority_message_queue.empty();
   message_cond.unlock();
   return result;
}

bool MailboxMT::hasUserEvent()
{
    message_cond.lock();
    bool result = ( numUserEvents != 0 );
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
