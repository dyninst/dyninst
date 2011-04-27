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

#include "proccontrol/h/Event.h"

#include "proccontrol/src/response.h"
#include "proccontrol/src/int_process.h"
#include "proccontrol/src/int_handler.h"

#include <cstring>

using namespace Dyninst;
using namespace ProcControlAPI;
using namespace std;

unsigned int response::next_id = 0;

response::response() :
   event(Event::ptr()),
   state(unset),
   checked_ready(false),
   isSyncHandled(false),
   error(false),
   errorcode(0)
{
   static Mutex id_lock;
   id_lock.lock();
   id = next_id++;
   id_lock.unlock();
}

response::~response()
{
   assert(state != ready || checked_ready);
}

bool response::isReady() const
{
   checked_ready = true;
   return (state == ready);
}

Event::ptr response::getEvent() const
{
   return event;
}

bool response::hasError() const
{
   return error;
}

int response::errorCode() const
{
   assert(error);
   return errorcode;
}

bool response::isPosted() const 
{
   return state != unset;
}

void response::markReady()
{
   assert(state != ready);
   state = ready;
}

void response::markPosted()
{
   assert(state == unset);
   state = posted;
}

void response::markError(int code)
{
   assert(!error);
   error = true;
   errorcode = code;
}

void response::setEvent(Event::ptr ev)
{
   assert(!event || !ev || event == ev);
   event = ev;
}

void response::markSyncHandled()
{
   isSyncHandled = true;
}

unsigned int response::getID() const 
{
   return id;
}

string response::name() const
{
   switch (resp_type) {
      case rt_result:
         return "Result Response";
      case rt_reg:
         return "Reg Response";
      case rt_allreg:
         return "AllReg Response";
      case rt_mem:
         return "Mem Response";
   }
   assert(0);
   return "";
}

result_response::ptr response::getResultResponse()
{
   return resp_type == rt_result ? 
      dyn_detail::boost::static_pointer_cast<result_response>(shared_from_this()) :
      result_response::ptr();
}

mem_response::ptr response::getMemResponse()
{
   return resp_type == rt_mem ? 
      dyn_detail::boost::static_pointer_cast<mem_response>(shared_from_this()) :
      mem_response::ptr();
}

reg_response::ptr response::getRegResponse()
{
   return resp_type == rt_reg ? 
      dyn_detail::boost::static_pointer_cast<reg_response>(shared_from_this()) :
      reg_response::ptr();
}

allreg_response::ptr response::getAllRegResponse()
{
   return resp_type == rt_allreg ? 
      dyn_detail::boost::static_pointer_cast<allreg_response>(shared_from_this()) :
      allreg_response::ptr();
}

response::ptr responses_pending::rmResponse(unsigned int id)
{
   //cvar lock should already be held.
   std::map<unsigned int, response::ptr>::iterator i = pending.find(id);
   assert(i != pending.end());
   response::ptr result = (*i).second;

   pending.erase(i);

   return result;
}
 
bool responses_pending::waitFor(response::ptr resp)
{
   cvar.lock();

   if (resp->isReady()) {
      cvar.unlock();
      pthrd_printf("Waiting for async event %d, complete\n", resp->getID());
      return true;
   }

   unsigned int iter = 0;
   map<unsigned int, response::ptr>::iterator i = pending.find(resp->getID());
   assert(i != pending.end());

   while (!resp->isReady()) {
      pthrd_printf("Waiting for async event %d, iter = %d\n", resp->getID(), iter);
      cvar.wait();
      iter++;
   }
   cvar.unlock();

   pthrd_printf("Waiting for async event %d, complete\n", resp->getID());

   return true;
}

bool responses_pending::hasAsyncPending()
{
   bool ret = false;
   cvar.lock();
   map<unsigned, response::ptr>::const_iterator i;
   for (i = pending.begin(); i != pending.end(); i++) {
      if (i->second->getEvent()) {
         ret = true;
         break;
      }
   }
   cvar.unlock();
   return ret;
}

response::ptr responses_pending::getResponse(unsigned int id)
{
   cvar.lock();
   map<unsigned int, response::ptr>::iterator i = pending.find(id);
   cvar.unlock();
   assert(i != pending.end());
   return (*i).second;
}

void responses_pending::lock()
{
   cvar.lock();
}

void responses_pending::unlock()
{
   cvar.unlock();
}

void responses_pending::signal()
{
   cvar.broadcast();
}

void responses_pending::addResponse(response::ptr r, int_process *proc)
{
   pthrd_printf("Adding response %d of type %s to list of pending responses\n", r->getID(), r->name().c_str());
   Event::ptr ev = proc->handlerPool()->curEvent();
   if (!ev) {
      ev = proc->getInternalRPCEvent(); //May return NULL
   }
   if (r->isSyncHandled)
      ev = Event::ptr();

   r->setEvent(ev);
   r->markPosted();
   pending[r->getID()] = r;
}

responses_pending &getResponses()
{
   static responses_pending rp;
   return rp;
}

result_response::ptr result_response::createResultResponse()
{
   return result_response::ptr(new result_response());
}

result_response::result_response() :
   b(false)
{
   resp_type = rt_result;
}

result_response::~result_response()
{
}

void result_response::setResponse(bool b_)
{
   postResponse(b_);
   markReady();
}

void result_response::postResponse(bool b_)
{
   b = b_;
}

bool result_response::getResult() const
{
   return b;
}

reg_response::ptr reg_response::createRegResponse()
{
   return reg_response::ptr(new reg_response());
}


reg_response::reg_response() :
   val(0),
   thr(NULL)
{
   resp_type = rt_reg;
}

reg_response::~reg_response()
{
}

void reg_response::setRegThread(Dyninst::MachRegister r, int_thread *t)
{
   reg = r;
   thr = t;
}

void reg_response::setResponse(Dyninst::MachRegisterVal v)
{
   postResponse(v);
   markReady();
}

void reg_response::postResponse(Dyninst::MachRegisterVal v)
{
   assert(reg && thr);
   thr->updateRegCache(reg, v);
   val = v;
}

Dyninst::MachRegisterVal reg_response::getResult() const { 
   return val;
}

allreg_response::ptr allreg_response::createAllRegResponse()
{
   return allreg_response::ptr(new allreg_response());
}

allreg_response::ptr allreg_response::createAllRegResponse(int_registerPool *regpool)
{
   allreg_response::ptr r = allreg_response::ptr(new allreg_response());
   r->regpool = regpool;
   return r;
}

allreg_response::allreg_response() :
   regpool(NULL),
   thr(NULL)
{
   resp_type = rt_allreg;
}

allreg_response::~allreg_response()
{
}

void allreg_response::setThread(int_thread *t)
{
   thr = t;
}

void allreg_response::setRegPool(int_registerPool *p)
{
   regpool = p;
}

void allreg_response::setResponse()
{
   postResponse();
   markReady();
}

void allreg_response::postResponse()
{
   assert(thr);
   thr->updateRegCache(*regpool);
}

int_registerPool *allreg_response::getRegPool() const
{
   return regpool;
}

mem_response::ptr mem_response::createMemResponse()
{
   return mem_response::ptr(new mem_response());
}

mem_response::ptr mem_response::createMemResponse(char *targ, unsigned targ_size)
{
   return mem_response::ptr(new mem_response(targ, targ_size));
}

mem_response::mem_response() :
   buffer(NULL),
   size(0),
   buffer_set(false)
{
   resp_type = rt_mem;
}

mem_response::mem_response(char *targ, unsigned targ_size) :
   buffer(targ),
   size(targ_size),
   buffer_set(true)
{
   resp_type = rt_mem;
}

mem_response::~mem_response()
{
}

void mem_response::setBuffer(char *targ, unsigned targ_size)
{
   assert(!buffer_set);
   buffer = targ;
   size = targ_size;
   buffer_set = true;
}

void mem_response::setResponse(char *src, unsigned src_size)
{
   postResponse(src, src_size);
   markReady();
}

void mem_response::setResponse()
{
   postResponse();
   markReady();
}

void mem_response::postResponse(char *src, unsigned src_size)
{
   assert(buffer_set);
   assert(src_size >= size);

   memcpy(buffer, src, size);
}

void mem_response::postResponse()
{
}

char *mem_response::getBuffer() const
{
   return buffer;
}

unsigned mem_response::getSize() const
{
   return size;
}
