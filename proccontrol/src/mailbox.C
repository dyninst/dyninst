/*
 * See the dyninst/COPYRIGHT file for copyright information.
 * 
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */
#include "Mailbox.h"
#include "Event.h"
#include "PCErrors.h"
#include "int_process.h"

#include "common/src/dthread.h"

#include <queue>

using namespace std;
using namespace Dyninst;
using namespace ProcControlAPI;

class MailboxMT : public Mailbox
{
private:
   queue<Event::ptr> message_queue;
   queue<Event::ptr> priority_message_queue; //Mostly used for async responses
   queue<Event::ptr> user_message_queue;
   CondVar<> message_cond;
public:
   MailboxMT();
   ~MailboxMT();

   virtual void enqueue(Event::ptr ev, bool priority = false);
   virtual void enqueue_user(Event::ptr ev);
   virtual Event::ptr dequeue(bool block);
   virtual Event::ptr peek();
   virtual unsigned int size();
   virtual bool hasPriorityEvent();
   virtual void lock_queue();
   virtual void unlock_queue();
private:
   void enqueue_worker(Event::ptr ev, bool priority, bool user);
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

void MailboxMT::lock_queue()
{
	message_cond.lock();
}

void MailboxMT::unlock_queue()
{
	message_cond.unlock();
}


void MailboxMT::enqueue(Event::ptr ev, bool priority)
{
   enqueue_worker(ev, priority, false);
}

void MailboxMT::enqueue_user(Event::ptr ev)
{
   enqueue_worker(ev, false, true);
}

void MailboxMT::enqueue_worker(Event::ptr ev, bool priority, bool user)
{
   message_cond.lock();

   if (priority)
      priority_message_queue.push(ev);
   else if (user)
      user_message_queue.push(ev);
   else
      message_queue.push(ev);

   message_cond.broadcast();
   pthrd_printf("Added event %s to mailbox, size = %lu + %lu + %lu = %lu\n", 
                ev->name().c_str(), 
                (unsigned long) message_queue.size(),
                (unsigned long) priority_message_queue.size(),
                (unsigned long) user_message_queue.size(),
                (unsigned long) (message_queue.size() + priority_message_queue.size() + user_message_queue.size()));

   message_cond.unlock();

   MTManager::eventqueue_cb_wrapper();
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

   bool user_thread = isUserThread();

   while (priority_message_queue.empty() && message_queue.empty() && (!user_thread || user_message_queue.empty())) {
      if (!block) {
         pthrd_printf("Polled mailbox for messages, but none found\n");
         message_cond.unlock();
         return Event::ptr();
      }

      pthrd_printf("Blocking for events from mailbox, queue size = %lu\n", 
                   (unsigned long) message_queue.size());
      message_cond.wait();
   }

   queue<Event::ptr> *q;
   if (!priority_message_queue.empty())
      q = &priority_message_queue;
   else if (!user_message_queue.empty() && user_thread)
      q = &user_message_queue;
   else
      q = &message_queue;

   Event::ptr ret = q->front();
   q->pop();

   message_cond.unlock();
   pthrd_printf("Returning event %s from mailbox\n", ret->name().c_str());
   return ret;
}

unsigned int MailboxMT::size()
{
   message_cond.lock();
   unsigned int result = (unsigned int) (message_queue.size() + priority_message_queue.size() + user_message_queue.size());
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

Mailbox* Dyninst::ProcControlAPI::mbox() 
{
   static Mailbox *mbox_int = NULL;
   if (!mbox_int) {
      mbox_int = static_cast<Mailbox *>(new MailboxMT());
   }
   return mbox_int;
}
