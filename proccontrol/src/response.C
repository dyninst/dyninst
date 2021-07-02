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

#include "Event.h"

#include "response.h"
#include "int_process.h"
#include "int_handler.h"
#include "procpool.h"

#include <cstring>

using namespace Dyninst;
using namespace ProcControlAPI;
using namespace std;

unsigned int response::next_id = 1;

static Mutex<> id_lock;

unsigned newResponseID()
{
  unsigned id;
  id_lock.lock();
  id = response::next_id++;
  id_lock.unlock();
  return id;
}

unsigned newResponseID(unsigned size)
{
  unsigned id;
  id_lock.lock();
  id = response::next_id;
  response::next_id += size;
  id_lock.unlock();
  return id;
}

response::response() :
   event(Event::ptr()),
   state(unset),
   checked_ready(false),
   isSyncHandled(false),
   error(false),
   errorcode(0),
   proc(NULL),
   aio(NULL),
   resp_type((resp_type_t)-1),
   decoder_event(NULL),
   multi_resp_size(0),
   multi_resp_recvd(0)
{
   id = newResponseID();
}

response::~response()
{
   assert(error || state != ready || checked_ready);
}

bool response::isReady() const
{
   checked_ready = true;
   return (state == ready);
}

bool response::testReady() const
{
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
   if (multi_resp_size && multi_resp_size < multi_resp_recvd)
      return;

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

int_eventAsyncIO *response::getAsyncIOEvent()
{
   return aio;
}

void response::setAsyncIOEvent(int_eventAsyncIO *aio_)
{
   aio = aio_;
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
      case rt_set:
         return "Set Response";
      case rt_stack:
         return "Stack Response";
      case rt_data:
         return "Data Response";
   }
   assert(0);
   return "";
}

unsigned int response::markAsMultiResponse(int num_resps)
{
   assert(num_resps);
   assert(state == unset);
   id_lock.lock();
   id = next_id;
   next_id += num_resps;
   id_lock.unlock();

   multi_resp_size = num_resps;

   return id;
}

bool response::isMultiResponse()
{
   return multi_resp_size != 0;
}

unsigned int response::multiResponseSize()
{
   return multi_resp_size;
}

bool response::isMultiResponseComplete()
{
   return (multi_resp_size == multi_resp_recvd);
}

void response::setDecoderEvent(ArchEvent *ae)
{
   decoder_event = ae;
}

ArchEvent *response::getDecoderEvent()
{
   return decoder_event;
}

int_process *response::getProcess() const
{
  return proc;
}

void response::setProcess(int_process *p)
{
  proc = p;
}

result_response::ptr response::getResultResponse()
{
   return resp_type == rt_result ? 
      boost::static_pointer_cast<result_response>(shared_from_this()) :
      result_response::ptr();
}

mem_response::ptr response::getMemResponse()
{
   return resp_type == rt_mem ? 
      boost::static_pointer_cast<mem_response>(shared_from_this()) :
      mem_response::ptr();
}

reg_response::ptr response::getRegResponse()
{
   return resp_type == rt_reg ? 
      boost::static_pointer_cast<reg_response>(shared_from_this()) :
      reg_response::ptr();
}

allreg_response::ptr response::getAllRegResponse()
{
   return resp_type == rt_allreg ? 
      boost::static_pointer_cast<allreg_response>(shared_from_this()) :
      allreg_response::ptr();
}

stack_response::ptr response::getStackResponse()
{
   return resp_type == rt_stack ?
      boost::static_pointer_cast<stack_response>(shared_from_this()) :
      stack_response::ptr();
}

data_response::ptr response::getDataResponse()
{
   return resp_type == rt_data ?
      boost::static_pointer_cast<data_response>(shared_from_this()) :
      data_response::ptr();
}

response::ptr responses_pending::rmResponse(unsigned int id)
{
   //cvar lock should already be held.
   std::map<unsigned int, response::ptr>::iterator i = pending.find(id);
   if (i == pending.end()) {
      pthrd_printf("Unknown response.\n");
      return response::ptr();
   }
   response::ptr result = (*i).second;

   pending.erase(i);

   return result;
}
 
bool responses_pending::waitFor(response::ptr resp)
{
   cvar.lock();

   if (resp->isReady()) {
      cvar.unlock();
      pthrd_printf("Waiting for async event %u, complete\n", resp->getID());
      return true;
   }

   unsigned int iter = 0;
   map<unsigned int, response::ptr>::iterator i = pending.find(resp->getID());
   assert(i != pending.end());

   while (!resp->isReady()) {
      pthrd_printf("Waiting for async event %u, iter = %u\n", resp->getID(), iter);
      cvar.wait();
      iter++;
   }
   cvar.unlock();

   pthrd_printf("Waiting for async event %u, complete\n", resp->getID());

   return true;
}

bool responses_pending::hasAsyncPending(bool ev_only)
{
   bool ret = false;
   cvar.lock();
   if (!ev_only) {
      ret = !pending.empty();
   }
   else {
      map<unsigned, response::ptr>::const_iterator i;
      for (i = pending.begin(); i != pending.end(); i++) {
         if (i->second->getEvent()) {
            ret = true;
            break;
         }
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

CondVar<> &responses_pending::condvar()
{
   return cvar;
}

void responses_pending::addResponse(response::ptr r, int_process *proc)
{
   pthrd_printf("Adding response %u of type %s to list of pending responses\n", r->getID(), r->name().c_str());
   Event::ptr ev = proc->handlerPool()->curEvent();
   if (r->isSyncHandled)
      ev = Event::ptr();

   r->setProcess(proc);

   r->setEvent(ev);
   r->markPosted();

   if (!r->isMultiResponse()) {
      pending[r->getID()] = r;
   }
   else {
      unsigned int id = r->getID();
      unsigned int end = r->getID() + r->multiResponseSize();
      for (unsigned int i = id; i < end; i++) {
         pending[i] = r;
      }
   }
}

void responses_pending::noteResponse()
{
   if (isGeneratorThread()) {
      //Signaling the ProcPool is meant to wake the generator.  We
      // obviously don't need to signal ourselves.  In fact,
      // the generator may already be holding the procpool lock, 
      // so we don't want to retake it.
      return;
   }
   ProcPool()->condvar()->lock();
   ProcPool()->condvar()->broadcast();
   ProcPool()->condvar()->unlock();
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
   if (!isMultiResponse()) {
      b = b_;
      return;
   }

   multi_resp_recvd++;
   if (multi_resp_recvd == 1) {
      b = b_;
      return;
   }
   // The 'b = b & b_' logic is so that a multi-response result will 
   // be true iff all RESULT_ACKs returned success
   b = b & b_;
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
   if (isMultiResponse()) {
      multi_resp_recvd++;
   }
   if (isMultiResponseComplete()) {
      thr->updateRegCache(*regpool);
   }
}

void allreg_response::setIndividualRegAccess(reg_response::ptr iacc, Dyninst::MachRegister ireg)
{
   assert(!indiv_access);
   indiv_access = iacc;
   indiv_reg = ireg;
}

Dyninst::MachRegister allreg_response::getIndividualReg()
{
   return indiv_reg;
}

reg_response::ptr allreg_response::getIndividualAcc()
{
   return indiv_access;
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
   buffer_set(false),
   last_base(0)
{
   resp_type = rt_mem;
}

mem_response::mem_response(char *targ, unsigned targ_size) :
   buffer(targ),
   size(targ_size),
   buffer_set(true),
   last_base(0)
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

void mem_response::postResponse(char *src, unsigned src_size, Address src_addr)
{
   assert(buffer_set);

   if (!isMultiResponse()) {
      assert(src_size >= size);
      memcpy(buffer, src, size);
      return;
   }

   multi_resp_recvd++;
   unsigned long offset = src_addr - last_base;
   memcpy(buffer + offset, src, src_size);
}

void mem_response::setLastBase(Address a) 
{
   last_base = a;
}

Address mem_response::lastBase()
{
   return last_base;
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

stack_response::ptr stack_response::createStackResponse(int_thread *t)
{
   return stack_response::ptr(new stack_response(t));
}

stack_response::stack_response(int_thread *t) :
   data(NULL),
   thr(t)
{
   resp_type = rt_stack;
}
 
stack_response::~stack_response()
{
}

void *stack_response::getData()
{
   return data;
}

int_thread *stack_response::getThread()
{
   return thr;
}

void stack_response::postResponse(void *d)
{
   data = d;
}

data_response::data_response() :
   data(NULL)
{
   resp_type = rt_data;
}

data_response::~data_response()
{
   if (data) {
      free(data);
      data = NULL;
   }
}

data_response::ptr data_response::createDataResponse()
{
   return data_response::ptr(new data_response());
}

void *data_response::getData()
{
   return data;
}

void data_response::postResponse(void *d)
{
   data = d;
   markReady();
}

unsigned int ResponseSet::next_id = 1;
Mutex<> ResponseSet::id_lock;
std::map<unsigned int, ResponseSet *> ResponseSet::all_respsets;

ResponseSet::ResponseSet()
{
  id_lock.lock();
  myid = next_id++;
  if (!myid)
    myid = next_id++;
  all_respsets.insert(make_pair(myid, this));
  id_lock.unlock();
}

void ResponseSet::addID(unsigned id, unsigned index)
{
  ids.insert(make_pair(index, id));
}

unsigned ResponseSet::getIDByIndex(unsigned int index, bool &found) const
{
  map<unsigned, unsigned>::const_iterator i = ids.find(index);
  if (i == ids.end()) {
    found = false;
    return 0;
  }
  found = true;
  return i->second;
}

unsigned int ResponseSet::getID() const {
  return myid;
}

ResponseSet *ResponseSet::getResponseSetByID(unsigned int id) {
  map<unsigned int, ResponseSet *>::iterator i;
  ResponseSet *respset = NULL;
  id_lock.lock();
  i = all_respsets.find(id);
  if (i != all_respsets.end()) {
    respset = i->second;
    all_respsets.erase(i);
  }
  id_lock.unlock();
  return respset;
}

