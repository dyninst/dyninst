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
#if !defined(EVENTTYPE_H_)
#define EVENTTYPE_H_

namespace Dyninst {
namespace ProcControlAPI {

class EventType
{
 public:
   static const int Error               = -1;
   static const int Unset               = 0;
   static const int Exit                = 1;
   static const int Crash               = 2;
   static const int Fork                = 3;
   static const int Exec                = 4;
   static const int ThreadCreate        = 5;
   static const int ThreadDestroy       = 6;
   static const int Stop                = 7;
   static const int Signal              = 8;
   static const int LibraryLoad         = 9;
   static const int LibraryUnload       = 10;
   static const int Bootstrap           = 11;
   static const int Breakpoint          = 12;
   static const int RPC                 = 13;
   static const int SingleStep          = 14;
   static const int Library             = 15;

   //Users do not recieve CBs for the below event types--ProcControlAPI internal
   static const int InternalEvents      = 500;
   static const int BreakpointClear     = 500;
   static const int RPCInternal         = 501;
   static const int Async               = 502;
   static const int ChangePCStop        = 503; // Used for bug_freebsd_change_pc

   //Users should define their own events at this value or higher.
   static const int MaxProcCtrlEvent    = 1000;
   typedef int Code;

   typedef enum {
      Pre = 0,
      Post,
      None,
      Any
   } Time;

   Code code() const { return ecode; }
   Time time() const { return etime; }

   EventType(Code e) : ecode(e), etime(Any) {}
   EventType(Time t, Code e) : ecode(e), etime(t) {}
   EventType() : ecode(Unset), etime(None) {}
   
   std::string name() const;
 protected:
   Code ecode;
   Time etime;
};

struct eventtype_cmp
{
   bool operator()(const EventType &a, const EventType &b)
   {
      if (a.code() < b.code()) 
         return true;
      if (a.code() > b.code())
         return false;
      return ((int) a.time() < (int) b.time());
   }
};

}
}

#endif
