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

#if !defined(RESP_H_)
#define RESP_H_

#include "response.h"
#include "int_process.h"
#include <map>
#include <string>
#include <vector>

/**
 * The Resp class is a second implementation of the response
 * class.  On async systems, it tracks
 * command messages we've sent to the OS, and associates them
 * with the responses we receive from the OS.
 *
 * Resp has several improvements over response:
 * - Integrated support for multi-responses, where you
 *   get multiple responses from one message.
 * - Replacement of response's global lock with a 
 *   per-process condition variable, which should have
 *   significantly less contention
 * - Templated parameters to hold command specific data.
 *
 * Over the long-term, it would be nice to completely replace
 * response with Resp, but for the moment Resp will just be used
 * for new code.
 **/

#include "int_process.h"

class Resp;
typedef Resp* Resp_ptr;
#define Resp_ptr_NULL NULL; 


class resp_process : virtual public int_process {
   friend class Resp;
  private:

   void addResp(Resp_ptr resp, unsigned id_start, unsigned id_end); 
   void markRespPosted(Resp_ptr resp);
   void rmResponse(Resp_ptr resp, bool lock_held = false);
   void markRespDone(Resp_ptr resp);

   std::map<int, Resp_ptr> active_resps;
   CondVar<Mutex<false> > active_resps_lock;
public:
   Resp_ptr recvResp(unsigned int id, bool &is_complete);

   resp_process(Dyninst::PID p, std::string e, std::vector<std::string> a, 
                std::vector<std::string> envp, std::map<int,int> f);
   resp_process(Dyninst::PID pid_, int_process *p);
   ~resp_process();

   void waitForEvent(Resp_ptr resp);
};

class Resp {
   friend class resp_process;
protected:
   enum {
      Setup,
      Posted,
      Received,
      Done,
      Error
   } state;
   unsigned int id_start;
   unsigned int id_end;
   unsigned int num_recvd;
   bool isWaitedOn;
   bool isCleaned;
   resp_process *proc;
   Dyninst::ProcControlAPI::Event::ptr event;
   void init();
public:
   typedef Resp_ptr ptr;

   Resp(resp_process *proc_);
   Resp(unsigned int multi_size, resp_process *proc_);
   virtual ~Resp();

   resp_process *getProc();
   unsigned int getID();
   unsigned int getIDEnd();
   void post();
   void done();
   bool hadError() const;
};

template<class T>
class RespItem : public Resp
{
protected:
   T *obj;
public:
   RespItem(T *obj_, resp_process *proc_) :
      Resp(proc_),
      obj(obj_)
   {
   }

   RespItem(T *obj_, resp_process *proc_, unsigned multi_size) :
      Resp(multi_size, proc_),
      obj(obj_)
   {
   }

   virtual ~RespItem()
   {
   }

   T *get() {
      return obj;
   }
};

namespace Dyninst {
namespace ProcControlAPI {
class int_eventAsyncFileRead;
}
}

typedef RespItem<Dyninst::ProcControlAPI::stat64_ptr> StatResp_t;
typedef RespItem<Dyninst::ProcControlAPI::int_eventAsyncFileRead> FileReadResp_t;
typedef RespItem<Dyninst::ProcControlAPI::FileSet> FileSetResp_t;
typedef RespItem<unsigned long> MemUsageResp_t;

#endif
