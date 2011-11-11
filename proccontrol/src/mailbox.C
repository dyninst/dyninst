/*
 * Copyright (c) 1996-2011 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
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
#include "proccontrol/h/Mailbox.h"
#include "proccontrol/h/Event.h"
#include "proccontrol/h/PCErrors.h"
#include "proccontrol/src/int_process.h"

#include "common/h/dthread.h"

#include <iostream>
#include <queue>

using namespace std;
using namespace Dyninst;
using namespace ProcControlAPI;

class MailboxMT : public Mailbox
{
public:

private:
	std::map<priority_t, queue<Event::ptr> > queues;
	Event::ptr peek_int();
	Event::ptr pop_int();
	bool empty_int();
	unsigned int size_int();

   CondVar message_cond;
   unsigned numUserEvents;
   bool valid(priority_t p) {
	   return (p == low || p == med || p == high);
   }
public:


   MailboxMT();
   ~MailboxMT();

   virtual void enqueue(Event::ptr ev, priority_t priority = med);
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

void MailboxMT::enqueue(Event::ptr ev, priority_t priority)
{
	message_cond.lock();

	assert(valid(priority));
	queues[priority].push(ev);

   message_cond.broadcast();
   pthrd_printf("Added event %s to mailbox, size = %lu + %lu + %lu\n", 
	   ev ? ev->name().c_str() : "(NULL)", 
                (unsigned long) queues[low].size(),
                (unsigned long) queues[med].size(),
				(unsigned long) queues[high].size());
	//cerr << "Added event " << *ev << " to mailbox\n";

   if( ev && ev->userEvent() ) {
      numUserEvents++;
   }
     
   message_cond.unlock();

   if( ev && ev->userEvent() ) {
       MTManager::eventqueue_cb_wrapper();
   }
}

Event::ptr MailboxMT::peek_int()
{
   for (std::map<priority_t, queue<Event::ptr> >::reverse_iterator iter = queues.rbegin(); 
	   iter != queues.rend(); ++iter) { 
		   if (!iter->second.empty()) {
				return iter->second.front();
		   }
   }
   return Event::ptr();
}

Event::ptr MailboxMT::pop_int()
{
   for (std::map<priority_t, queue<Event::ptr> >::reverse_iterator iter = queues.rbegin(); 
	   iter != queues.rend(); ++iter) { 
		   if (!iter->second.empty()) {
			   Event::ptr ret = iter->second.front();
			   iter->second.pop();
			   return ret;
		   }
   }
   return Event::ptr();
}

bool MailboxMT::empty_int()
{
   for (std::map<priority_t, queue<Event::ptr> >::reverse_iterator iter = queues.rbegin(); 
	   iter != queues.rend(); ++iter) { 
		   if (!iter->second.empty()) {
			   return false;
		   }
   }
	return true;
}

unsigned int MailboxMT::size_int()
{
	unsigned int size = 0;
   for (std::map<priority_t, queue<Event::ptr> >::reverse_iterator iter = queues.rbegin(); 
	   iter != queues.rend(); ++iter) { 
		   size += iter->second.size();
   }
	return size;
}


Event::ptr MailboxMT::peek()
{
   message_cond.lock();
   Event::ptr ev = peek_int();

   message_cond.unlock();
   return ev;
}

Event::ptr MailboxMT::dequeue(bool block)
{
   message_cond.lock();

   while (empty_int()) {
	   if (!block) {
		   message_cond.unlock();
		   return Event::ptr();
	   }
	   pthrd_printf("Blocking for events from mailbox\n");
       message_cond.wait();
   }
   Event::ptr ret = pop_int();

   if( ret && ret->userEvent() ) {
       numUserEvents--;
   }
   pthrd_printf("Returning event %s from mailbox\n", ret ? ret->name().c_str() : "(NULL)");
	//cerr << "Removing event " << *ret << " from mailbox\n";

   message_cond.unlock();
   return ret;
}

unsigned int MailboxMT::size()
{
   message_cond.lock();
	unsigned int result = size_int();
   message_cond.unlock();
   return result;
}

bool MailboxMT::hasPriorityEvent()
{
   message_cond.lock();
   bool result = !queues[high].empty();
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
