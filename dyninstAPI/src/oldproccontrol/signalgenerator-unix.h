/*
 * Copyright (c) 1996-2009 Barton P. Miller
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

/* $Id: signalgenerator-unix.h,v 1.17 2008/04/15 16:43:31 roundy Exp $
 */


#ifndef _SIGNAL_GENERATOR_UNIX_H_
#define _SIGNAL_GENERATOR_UNIX_H_

#include <string>
#include "dyninstAPI/src/os.h"

class Frame;

class SignalGenerator : public SignalGeneratorCommon
{
  friend class SignalHandler;
  friend class SignalGeneratorCommon;
  friend class process;
  
  public:
  virtual ~SignalGenerator();

   bool checkForExit(EventRecord &ev, bool block =false);

   bool waitingForStop() {return waiting_for_stop;}
   void setWaitingForStop(bool flag) {waiting_for_stop = flag;}
   void expectFakeSignal() {expect_fake_signal = true;}
  private:
  //  SignalGenerator should only be constructed by process
  SignalGenerator(char *idstr, std::string file, std::string dir,
                  pdvector<std::string> *argv,
                  pdvector<std::string> *envp,
                  std::string inputFile,
                  std::string outputFile,
                  int stdin_fd, int stdout_fd,
                  int stderr_fd);

  SignalGenerator(char *idstr, std::string file, int pid);

  virtual SignalHandler *newSignalHandler(char *name, int id);

  virtual bool forkNewProcess();
  virtual bool attachProcess();
  virtual bool waitForStopInline();
  virtual bool decodeEvents(pdvector<EventRecord> &events);
  virtual bool waitForEventsInternal(pdvector<EventRecord> &events);

  //  decodeSignal_NP is called by decodeSignal before decodeSignal does any
  //  decoding.  It allows platform specific actions to be taken for signal
  //  decoding.  If it returns true, decodeSignal assumes that a valid decode 
  //  has taken place and it  will not do any further decoding.
  bool decodeSignal_NP(EventRecord &ev);
  bool decodeSignal(EventRecord &ev);
  bool decodeSigTrap(EventRecord &ev);
  bool decodeSigStopNInt(EventRecord &ev);
  bool decodeSigIll(EventRecord &ev);
  bool isInstTrap(const EventRecord &ev, const Frame &af);

  //  decodeSyscall changes the field ev.what from a platform specific
  //  syscall representation, eg, SYS_fork, to a platform indep. one,
  //  eg. procSysFork.  returns false if there is no available mapping.
  virtual bool decodeSyscall(EventRecord &ev);

#if !defined (os_linux) && !defined(os_freebsd)
   bool decodeProcStatus(procProcStatus_t status, EventRecord &ev);
#endif

   bool waiting_for_stop;

   bool expect_fake_signal;

#if defined (os_linux)
   public:
   bool attachToChild(int pid);
   bool registerLWP(int lwpid) { return waitpid_mux.registerLWP(lwpid, this);}
   bool unregisterLWP(int lwpid) { return waitpid_mux.unregisterLWP(lwpid, this);}
   bool add_lwp_to_poll_list(dyn_lwp *lwp);
   bool remove_lwp_from_poll_list(int lwp_id);
   bool resendSuppressedSignals();
   bool exists_dead_lwp();
   bool forceWaitpidReturn() {waitpid_mux.forceWaitpidReturn(); return true;}
   bool pauseAllWaitpid() { return waitpid_mux.suppressWaitpidActivity();}
   bool resumeWaitpid() { return waitpid_mux.resumeWaitpidActivity();}


   pdvector<waitpid_ret_pair> event_queue;
 private:
   static WaitpidMux waitpid_mux;

   pdvector<int> suppressed_sigs;
   pdvector<dyn_lwp *> suppressed_lwps;
   //  SignalHandler::suppressSignalWhenStopping
   //  needed on linux platforms.  Allows the signal handler function
   //  to ignore most non SIGSTOP signals when waiting for a process to stop
   //  Returns true if signal is to be suppressed.
   bool suppressSignalWhenStopping(EventRecord &ev);
   //  SignalHandler::resendSuppressedSignals
   //  called upon receipt of a SIGSTOP.  Sends all deferred signals to the stopped process.
   int find_dead_lwp();
   pdvector<int> attached_lwp_ids;
 public:
   pid_t waitpid_kludge(pid_t, int *, int, int *);
#endif

};

#endif
