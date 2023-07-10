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

#ifndef RESPONSE_H_
#define RESPONSE_H_

#include "Event.h"
#include "common/src/dthread.h"
#include <string>
#include <map>
#include <vector>

using namespace Dyninst;
using namespace ProcControlAPI;

class mem_response;
class result_response;
class reg_response;
class allreg_response;
class stack_response;
class data_response;
class HandlerPool;

class response : public boost::enable_shared_from_this<response> {
   friend void boost::checked_delete<response>(response *) CHECKED_DELETE_NOEXCEPT;
   friend void boost::checked_delete<const response>(const response *) CHECKED_DELETE_NOEXCEPT;
   friend class responses_pending;
   friend unsigned newResponseID();
   friend unsigned newResponseID(unsigned);
  protected:
   Dyninst::ProcControlAPI::Event::ptr event;

   typedef enum {
      unset,
      posted,
      ready
   } state_t;

   unsigned int id;
   static unsigned int next_id;
   state_t state;
   mutable bool checked_ready;
   bool isSyncHandled;

   bool error;
   int errorcode;
   int_process *proc;
   int_eventAsyncIO *aio;

  protected:
   response();
   typedef enum {
      rt_result,
      rt_reg,
      rt_allreg,
      rt_mem,
      rt_stack,
      rt_data,
      rt_set
   } resp_type_t;
   resp_type_t resp_type;

   ArchEvent *decoder_event;

   int multi_resp_size;
   int multi_resp_recvd;

  public:
   typedef boost::shared_ptr<response> ptr;
   typedef boost::shared_ptr<const response> const_ptr;

   virtual ~response();

   unsigned int getID() const;

   boost::shared_ptr<result_response> getResultResponse();
   boost::shared_ptr<mem_response> getMemResponse();
   boost::shared_ptr<reg_response> getRegResponse();
   boost::shared_ptr<allreg_response> getAllRegResponse();
   boost::shared_ptr<stack_response> getStackResponse();
   boost::shared_ptr<data_response> getDataResponse();

   bool isReady() const;
   bool testReady() const;
   bool isPosted() const;
   bool hasError() const;
   int errorCode() const;

   void markPosted();
   void markReady();
   void markError(int code = 0);
   void markSyncHandled();

   void setEvent(Event::ptr ev);
   Event::ptr getEvent() const;

   unsigned int markAsMultiResponse(int num_resps);
   bool isMultiResponse();
   unsigned int multiResponseSize();
   bool isMultiResponseComplete();

   void setDecoderEvent(ArchEvent *ae);
   ArchEvent *getDecoderEvent();

   void setProcess(int_process *p);
   int_process *getProcess() const;

   int_eventAsyncIO *getAsyncIOEvent();
   void setAsyncIOEvent(int_eventAsyncIO *aoi_);

   std::string name() const;
};

class responses_pending {
  private:
   std::map<unsigned int, response::ptr> pending;
   CondVar<Mutex <false> > cvar;

  public:
   response::ptr rmResponse(unsigned int id);
   response::ptr getResponse(unsigned int id);
   bool waitFor(response::ptr resp);
   void addResponse(response::ptr r, int_process *proc);
   void noteResponse();
   bool hasAsyncPending(bool ev_only = true);

   CondVar<Mutex <false> > &condvar();
   void lock();
   void unlock();
   void signal();
};

responses_pending &getResponses();

class result_response : public response
{
   friend class linux_process;
   friend class linux_thread;
   friend void boost::checked_delete<result_response>(result_response *) CHECKED_DELETE_NOEXCEPT;
   friend void boost::checked_delete<const result_response>(const result_response *) CHECKED_DELETE_NOEXCEPT;
  private:
   bool b;
   result_response();

  public:
   typedef boost::shared_ptr<result_response> ptr;
   typedef boost::shared_ptr<const result_response> const_ptr;

   static result_response::ptr createResultResponse();

   virtual ~result_response();

   void setResponse(bool b);
   void postResponse(bool b);

   bool getResult() const;
};

class reg_response : public response
{
   friend class linux_thread;
   friend void boost::checked_delete<reg_response>(reg_response *) CHECKED_DELETE_NOEXCEPT;
   friend void boost::checked_delete<const reg_response>(const reg_response *) CHECKED_DELETE_NOEXCEPT;
  private:
   Dyninst::MachRegisterVal val;
   reg_response();

   Dyninst::MachRegister reg;
   int_thread *thr;

  public:
   typedef boost::shared_ptr<reg_response> ptr;
   typedef boost::shared_ptr<const reg_response> const_ptr;

   static reg_response::ptr createRegResponse();

   virtual ~reg_response();

   void setRegThread(Dyninst::MachRegister r, int_thread *t);
   void setResponse(Dyninst::MachRegisterVal v);
   void postResponse(Dyninst::MachRegisterVal v);
   Dyninst::MachRegisterVal getResult() const;
};

class allreg_response : public response
{
   friend void boost::checked_delete<allreg_response>(allreg_response *) CHECKED_DELETE_NOEXCEPT;
   friend void boost::checked_delete<const allreg_response>(const allreg_response *) CHECKED_DELETE_NOEXCEPT;
  private:
   int_registerPool *regpool;
   int_thread *thr;
   reg_response::ptr indiv_access;
   Dyninst::MachRegister indiv_reg;
   allreg_response();

  public:
   typedef boost::shared_ptr<allreg_response> ptr;
   typedef boost::shared_ptr<const allreg_response> const_ptr;

   static allreg_response::ptr createAllRegResponse(int_registerPool *regpool);
   static allreg_response::ptr createAllRegResponse();

   virtual ~allreg_response();

   void setThread(int_thread *t);
   void setRegPool(int_registerPool *p);
   void setResponse();
   void postResponse();

   void setIndividualRegAccess(reg_response::ptr iacc, Dyninst::MachRegister ireg);
   Dyninst::MachRegister getIndividualReg();
   reg_response::ptr getIndividualAcc();

   int_registerPool *getRegPool() const;
};

class mem_response : public response
{
   friend void boost::checked_delete<mem_response>(mem_response *) CHECKED_DELETE_NOEXCEPT;
   friend void boost::checked_delete<const mem_response>(const mem_response *) CHECKED_DELETE_NOEXCEPT;
  private:
   char *buffer;
   unsigned size;
   bool buffer_set;
   Address last_base;
   mem_response();
   mem_response(char *targ, unsigned targ_size);

  public:
   typedef boost::shared_ptr<mem_response> ptr;
   typedef boost::shared_ptr<const mem_response> const_ptr;

   static mem_response::ptr createMemResponse();
   static mem_response::ptr createMemResponse(char *targ, unsigned targ_size);

   virtual ~mem_response();

   char *getBuffer() const;
   unsigned getSize() const;

   void setBuffer(char *targ, unsigned targ_size);
   void setResponse(char *src, unsigned src_size);
   void setResponse();
   void postResponse(char *src, unsigned src_size, Address src_addr = 0);
   void postResponse();
   void setLastBase(Address a);
   Address lastBase();
};

class stack_response : public response
{
   friend void boost::checked_delete<stack_response>(stack_response *) CHECKED_DELETE_NOEXCEPT;
   friend void boost::checked_delete<const stack_response>(const stack_response *) CHECKED_DELETE_NOEXCEPT;
  private:
   void *data;
   int_thread *thr;
   stack_response(int_thread *t);

  public:
   typedef boost::shared_ptr<stack_response> ptr;
   typedef boost::shared_ptr<const stack_response> const_ptr;

   static stack_response::ptr createStackResponse(int_thread *t);

   virtual ~stack_response();

   void *getData();
   int_thread *getThread();
   void postResponse(void *d);
};

class data_response : public response
{
   friend void boost::checked_delete<data_response>(data_response *) CHECKED_DELETE_NOEXCEPT;
   friend void boost::checked_delete<const data_response>(const data_response *) CHECKED_DELETE_NOEXCEPT;
  private:
   void *data;
   data_response();
  public:
   typedef boost::shared_ptr<data_response> ptr;
   typedef boost::shared_ptr<const data_response> const_ptr;

   static data_response::ptr createDataResponse();

   virtual ~data_response();

   void *getData();
   void postResponse(void *d);
};

class ResponseSet {
 private:
  std::map<unsigned, unsigned> ids;
  unsigned myid;
  static unsigned next_id;
  static Mutex<false> id_lock;
  static std::map<unsigned, ResponseSet *> all_respsets;
 public:
  ResponseSet();
  void addID(unsigned resp_id, unsigned index);
  unsigned getID() const;
  unsigned getIDByIndex(unsigned int index, bool &found) const;
  static ResponseSet *getResponseSetByID(unsigned);
};

unsigned newResponseID();
unsigned newResponseID(unsigned size);

#endif
