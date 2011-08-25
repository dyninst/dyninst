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

#ifndef RESPONSE_H_
#define RESPONSE_H_

#include "dyn_detail/boost/shared_ptr.hpp"
#include "dyn_detail/boost/weak_ptr.hpp"

#include "proccontrol/h/Event.h"
#include "common/h/dthread.h"
#include <map>

using namespace Dyninst;
using namespace ProcControlAPI;

class mem_response;
class result_response;
class reg_response;
class allreg_response;
class HandlerPool;

class response : public dyn_detail::boost::enable_shared_from_this<response> {
   friend void dyn_detail::boost::checked_delete<response>(response *);
   friend void dyn_detail::boost::checked_delete<const response>(const response *);
   friend class responses_pending;
  private:
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

  protected:
   response();
   typedef enum {
      rt_result,
      rt_reg,
      rt_allreg,
      rt_mem
   } resp_type_t;
   resp_type_t resp_type;

   ArchEvent *decoder_event;

   int multi_resp_size;
   int multi_resp_recvd;

  public:
   typedef dyn_detail::boost::shared_ptr<response> ptr;
   typedef dyn_detail::boost::shared_ptr<const response> const_ptr;

   virtual ~response();

   unsigned int getID() const;

   dyn_detail::boost::shared_ptr<result_response> getResultResponse();
   dyn_detail::boost::shared_ptr<mem_response> getMemResponse();
   dyn_detail::boost::shared_ptr<reg_response> getRegResponse();
   dyn_detail::boost::shared_ptr<allreg_response> getAllRegResponse();
   
   bool isReady() const;
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

   std::string name() const;
};

class responses_pending {
  private:
   std::map<unsigned int, response::ptr> pending;
   CondVar cvar;

  public:
   response::ptr rmResponse(unsigned int id);
   response::ptr getResponse(unsigned int id);
   bool waitFor(response::ptr resp);
   void addResponse(response::ptr r, int_process *proc);
   void noteResponse();
   bool hasAsyncPending(bool ev_only = true);

   void lock();
   void unlock();
   void signal();
};

responses_pending &getResponses();

class result_response : public response
{
   friend class linux_process;
   friend class linux_thread;
   friend void dyn_detail::boost::checked_delete<result_response>(result_response *);
   friend void dyn_detail::boost::checked_delete<const result_response>(const result_response *);
  private:
   bool b;
   result_response();

  public:
   typedef dyn_detail::boost::shared_ptr<result_response> ptr;
   typedef dyn_detail::boost::shared_ptr<const result_response> const_ptr;

   static result_response::ptr createResultResponse();

   virtual ~result_response();

   void setResponse(bool b);
   void postResponse(bool b);

   bool getResult() const;
};

class reg_response : public response
{
   friend class linux_thread;
   friend void dyn_detail::boost::checked_delete<reg_response>(reg_response *);
   friend void dyn_detail::boost::checked_delete<const reg_response>(const reg_response *);
  private:
   Dyninst::MachRegisterVal val;
   reg_response();

   Dyninst::MachRegister reg;
   int_thread *thr;   
   
  public:
   typedef dyn_detail::boost::shared_ptr<reg_response> ptr;
   typedef dyn_detail::boost::shared_ptr<const reg_response> const_ptr;

   static reg_response::ptr createRegResponse();

   virtual ~reg_response();

   void setRegThread(Dyninst::MachRegister r, int_thread *t);
   void setResponse(Dyninst::MachRegisterVal v);
   void postResponse(Dyninst::MachRegisterVal v);
   Dyninst::MachRegisterVal getResult() const;
};

class allreg_response : public response
{
   friend void dyn_detail::boost::checked_delete<allreg_response>(allreg_response *);
   friend void dyn_detail::boost::checked_delete<const allreg_response>(const allreg_response *);
  private:
   int_registerPool *regpool;
   int_thread *thr;
   allreg_response();

  public:
   typedef dyn_detail::boost::shared_ptr<allreg_response> ptr;
   typedef dyn_detail::boost::shared_ptr<const allreg_response> const_ptr;

   static allreg_response::ptr createAllRegResponse(int_registerPool *regpool);
   static allreg_response::ptr createAllRegResponse();

   virtual ~allreg_response();

   void setThread(int_thread *t);
   void setRegPool(int_registerPool *p);
   void setResponse();
   void postResponse();
   int_registerPool *getRegPool() const;
};

class mem_response : public response
{
   friend void dyn_detail::boost::checked_delete<mem_response>(mem_response *);
   friend void dyn_detail::boost::checked_delete<const mem_response>(const mem_response *);
  private:
   char *buffer;
   unsigned size;
   bool buffer_set;
   Address last_base;
   mem_response();
   mem_response(char *targ, unsigned targ_size);

  public:
   typedef dyn_detail::boost::shared_ptr<mem_response> ptr;
   typedef dyn_detail::boost::shared_ptr<const mem_response> const_ptr;

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
};


#endif
