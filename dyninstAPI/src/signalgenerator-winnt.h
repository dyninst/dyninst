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

/* $Id: signalgenerator-winnt.h,v 1.10 2008/04/15 16:43:32 roundy Exp $
 */

#ifndef _SIGNAL_GENERATOR_WINNT_H
#define _SIGNAL_GENERATOR_WINNT_H

#include <string>

// Normally included automatically in signalgenerator.h

///////////////////////////////////////////////////////////////////
////////// Handler section
///////////////////////////////////////////////////////////////////

// These are the prototypes for the NT-style signal handler
// NT has a two-level continue mechanism which we have to handle.
// There is a process-wide debug pause/resume, and a per-thread flag
// to pause or resume. We use the per-thread for pause() and continueProc(),
// and the process-wide for debug events. The process is stopped when
// the signal handler executes, and is re-run at the end. If the process
// should stay stopped the handlers must pause it explicitly.


class SignalGenerator : public SignalGeneratorCommon
{
  friend class SignalHandler;
  friend class SignalGeneratorCommon;
  friend class process;

  public:
   virtual ~SignalGenerator() {}

   HANDLE getProcessHandle() {return (HANDLE)procHandle;}
   HANDLE getThreadHandle() {return (HANDLE)thrHandle;}
  SignalGenerator(char *idstr, std::string file, std::string dir,
                         pdvector<std::string> *argv,
                         pdvector<std::string> *envp,
                         std::string inputFile,
                         std::string outputFile,
                         int stdin_fd, int stdout_fd,
                         int stderr_fd);

  SignalGenerator(char *idstr, std::string file, int pid);

   bool waitingForStop() {return false;}
   void setWaitingForStop(bool flag) {;}

  bool SuspendThreadFromEvent(LPDEBUG_EVENT ev, dyn_lwp *lwp);
  private:
  SignalHandler *newSignalHandler(char *name, int id);
  virtual bool forkNewProcess();
  virtual bool attachProcess();
  virtual bool waitForStopInline();
  bool waitNextEventInternal(EventRecord &);
  bool waitForEventsInternal(pdvector<EventRecord> &events);
  virtual bool decodeEvents(pdvector<EventRecord> &events);

  bool decodeEvent(EventRecord &event);
  bool decodeBreakpoint(EventRecord &);
  bool decodeException(EventRecord &);
  virtual bool decodeSyscall(EventRecord &ev);

  int procHandle;
  int thrHandle;
  bool waiting_for_stop;
};

#endif
