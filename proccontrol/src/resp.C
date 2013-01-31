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

#include <utility>
#include "response.h"
#include "resp.h"
#include "proccontrol/h/Handler.h"
#include "int_handler.h"


using namespace std;
using namespace Dyninst;
using namespace ProcControlAPI;

resp_process::resp_process(Dyninst::PID p, string e, vector<string> a, 
                           vector<string> envp, map<int,int> f) :
   int_process(p, e, a, envp, f),
   waitingForPost(false)
{
}

resp_process::resp_process(Dyninst::PID pid_, int_process *p) :
   int_process(pid_, p),
   waitingForPost(false)
{
}

resp_process::~resp_process()
{
}

void resp_process::addResp(Resp::ptr resp, unsigned id_start, unsigned id_end)
{
   active_resps_lock.lock();
   for (unsigned i=id_start; i<id_end; i++) {
      active_resps.insert(make_pair(i, resp));
   }
   active_resps_lock.unlock();
}

Resp::ptr resp_process::recvResp(unsigned int id, bool &is_complete) {
   Resp::ptr resp;

   active_resps_lock.lock();
   map<int, Resp::ptr>::iterator i = active_resps.find(id);
   assert(i != active_resps.end());
   resp = i->second;

   if (resp->state == Resp::Setup) {
      //Race.  We received the event before marking as posted.
      // wait for the post, which should come soon.  Recollect
      // the Resp each time in-case it's been removed.
      waitingForPost = true;
      while (resp->state != Resp::Setup) {
         active_resps_lock.wait();
         i = active_resps.find(id);
         if (i == active_resps.end()) {
            active_resps_lock.unlock();
            return Resp_ptr_NULL;
         }
         resp = i->second;
      }
      waitingForPost = false;
   }

   //Normal case.  Check whether we've received everything in a multi-response.
   // If so, mark it as received and clean it out of the pending list.
   resp->num_recvd++;
   is_complete = (resp->num_recvd == resp->id_end - resp->id_start);
   
   if (is_complete) {
      rmResponse(resp, true);
      resp->state = Resp::Received;
      if (resp->isWaitedOn)
         active_resps_lock.broadcast();
   }
   active_resps_lock.unlock();
   
   return resp;
}

void resp_process::markRespPosted(Resp::ptr resp)
{
   active_resps_lock.lock();
   resp->state = Resp::Posted;
   if (waitingForPost) {
      active_resps_lock.broadcast();
   }
   active_resps_lock.unlock();
}

void resp_process::waitForEvent(Resp::ptr resp)
{
   if (resp->state == Resp::Posted)
   {
      active_resps_lock.lock();
      resp->isWaitedOn = true;
      while (resp->state == Resp::Posted) {
         active_resps_lock.wait();
      }
      resp->isWaitedOn = false;
      active_resps_lock.unlock();      
   }
}

void resp_process::rmResponse(Resp::ptr resp, bool lock_held)
{
   if (resp->isCleaned)
      return;
   resp->isCleaned = true;

   if (!lock_held) 
      active_resps_lock.lock();

   map<int, Resp::ptr>::iterator start = active_resps.find(resp->id_start);
   assert(start != active_resps.end());
   if (resp->id_end == resp->id_start + 1) {
      active_resps.erase(start);
   }
   else {
      map<int, Resp::ptr>::iterator end = active_resps.find(resp->id_end-1);
      assert(end != active_resps.end());
      end++;
      active_resps.erase(start, end);
   }
   
   if (resp->state != Resp::Received) {
      resp->state = Resp::Error;
      if (waitingForPost)
         active_resps_lock.broadcast();
   }
   
   if (!lock_held) 
      active_resps_lock.unlock();
}

void Resp::init()
{
   proc->addResp(this, id_start, id_end);
   state = Setup;
   num_recvd = 0;
   isWaitedOn = false;
   isCleaned = false;
   event = proc->handlerPool()->curEvent();
}

Resp::Resp(resp_process *proc_) :
   id_start(newResponseID()),
   id_end(id_start+1),
   proc(proc_)
{
   init();
}

Resp::Resp(unsigned int multi_size, resp_process *proc_) :
   id_start(newResponseID()),
   id_end(id_start + multi_size),
   proc(proc_)
{
   init();
}

unsigned int Resp::getID()
{
   return id_start;
}

unsigned int Resp::getIDEnd()
{
   return id_end;
}

Resp::~Resp()
{
   proc->rmResponse(this);
}

void Resp::post()
{
   proc->markRespPosted(this);
}
