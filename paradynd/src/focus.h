/*
 * Copyright (c) 1996 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */

#ifndef __FOCUS__
#define __FOCUS__


#include "common/h/Vector.h"
#include "common/h/String.h"


class Hierarchy {  
 public:
  virtual ~Hierarchy() { };
  virtual string getName() const = 0;
  virtual pdvector<string> tokenized() const = 0;
  virtual bool focus_matches(const pdvector<string> &match_path) const = 0;
};


// --------------------------------------------------------------
// DIFFERENT TYPES OF HIERARCHIES

class machineHierarchy : public Hierarchy {
  string machine;
  string process;
  string thread;
 public:
  machineHierarchy(const pdvector<string> &setupInfo);
  int getPid() const;
  void setPid(int pid);
  int getThreadID() const;
  string getName() const;
  bool allMachines() const { 
    return (!machine_defined() && !process_defined() && !thread_defined());
  }
  string get_machine() const { return machine; }
  string get_process() const { return process; }
  string get_thread()  const { return thread; }
  bool machine_defined() const { return (machine.length()>0); }
  bool process_defined() const { return (process.length()>0); }
  bool thread_defined()  const { return (thread.length()>0);  }
  void set_machine(const string &machine_) {  machine = machine_;  }
  void set_process(const string &process_) {  process = process_; }
  void set_thread(const string &thread_)   {  thread  = thread_;  }

  bool focus_matches(const pdvector<string> &match_path) const;
  pdvector<string> tokenized() const;
};


class codeHierarchy : public Hierarchy {
  string module;
  string function;
 public:
  codeHierarchy(const pdvector<string> &setupInfo);
  string getName() const;  
  bool allCode() const { 
    return (!module_defined() && !function_defined());
  }
  string get_module()   const { return module;   }
  string get_function() const { return function; }
  bool module_defined()   const { return (module.length()>0);   }
  bool function_defined() const { return (function.length()>0); }

  bool focus_matches(const pdvector<string> &match_path) const;
  pdvector<string> tokenized() const;
};

struct message_data_t {
  string communicator;
  string tag;
};
  
struct barrier_data_t {
  string barrierStr;    // this may or may not be used
};

struct semaphore_data_t {
  string semaphoreStr;  // this may or may not be used
};
  
struct spinlock_data_t {
  string spinlockStr;   // this may or may not be used
};


class syncObjHierarchy : public Hierarchy {
  typedef enum { NoSyncObjT, MessageT, BarrierT, 
		 SemaphoreT, SpinlockT }  sync_obj_type;

  sync_obj_type syncObjType;
  message_data_t   messageData;
  barrier_data_t   barrierData;
  semaphore_data_t semaphoreData;
  spinlock_data_t  spinlockData;

 public:
  syncObjHierarchy(const pdvector<string> &setupInfo);
  string getName() const;
  bool allSync() const { return (syncObjType == NoSyncObjT); }
  bool allMessages() const { 
    return (syncObjType == MessageT  && 
	    !communicator_defined()  &&
	    !tag_defined());
  }
  bool allBarriers() const {
    return (syncObjType == BarrierT  &&
	    barrierData.barrierStr.length() == 0);
  }
  bool allSemaphores() const {
    return (syncObjType == SemaphoreT  &&
	    semaphoreData.semaphoreStr.length() == 0);
  }
  bool allSpinlocks() const {
    return (syncObjType == SpinlockT  &&
	    spinlockData.spinlockStr.length() == 0);
  }

  string get_communicator() const { 
    if(syncObjType == MessageT)
      return messageData.communicator;
    else
      return string("");
  }
  string get_tag() const { 
    if(syncObjType == MessageT)
      return messageData.tag; 
    else
      return string("");
  }
  bool communicator_defined() const { 
    return (messageData.communicator.length()>0);
  }
  bool tag_defined() const { return (messageData.tag.length()>0); }

  bool focus_matches(const pdvector<string> &match_path) const;
  pdvector<string> tokenized() const;
};


class memoryHierarchy : public Hierarchy {
 public:
  memoryHierarchy() { }
  memoryHierarchy(const pdvector<string> &setupInfo);
  string getName() const ;
  bool allMem() const { return true; }
  bool focus_matches(const pdvector<string> &match_path) const;
  pdvector<string> tokenized() const;
};

// --------------------------------------------------------------
// FOCUS

class Focus {
 private:
  machineHierarchy *machineInfo;
  codeHierarchy    *codeInfo;
  syncObjHierarchy *syncObjInfo;
  memoryHierarchy  *memoryInfo;

  void operator=(const Focus &);

 public:
  Focus(const pdvector<u_int>& ids, bool *errorFlag);
  ~Focus();
  
  Focus(const Focus &f);
  
  const machineHierarchy *getMachineHierarchy() const { return machineInfo; }
  const codeHierarchy    *getCodeHierarchy()    const { return codeInfo;    }
  const syncObjHierarchy *getSyncObjHierarchy() const { return syncObjInfo; }
  const memoryHierarchy  *getMemoryHierarchy()  const { return memoryInfo;  }

  string getName() const;
  bool allMachines() const { return machineInfo->allMachines(); }
  bool allCode()     const { return codeInfo->allCode();        }
  bool allSync()     const { return syncObjInfo->allSync();     }
  bool allMem()      const { return allMem();                  }

  string get_machine() const { return machineInfo->get_machine(); }
  string get_process() const { return machineInfo->get_process(); }
  string get_thread()  const { return machineInfo->get_thread();  }
  int getPid() const    { return machineInfo->getPid(); }
  void setPid(int pid)  { machineInfo->setPid(pid); }
  int getThreadID() const    { return machineInfo->getThreadID(); }
  bool machine_defined() const { return machineInfo->machine_defined(); }
  bool process_defined() const { return machineInfo->process_defined(); }
  bool thread_defined()  const { return machineInfo->thread_defined();  }
  void set_machine(const string &mStr) {  machineInfo->set_machine(mStr); }
  void set_process(const string &pStr) {  machineInfo->set_process(pStr); }
  void set_thread(const string &tStr)  {  machineInfo->set_thread(tStr);  }

  string get_module()   const { return codeInfo->get_module();   }
  string get_function() const { return codeInfo->get_function(); }
  bool module_defined() const   { return codeInfo->module_defined();   }
  bool function_defined() const { return codeInfo->function_defined(); }

  string get_communicator() const { return syncObjInfo->get_communicator(); }
  string get_tag()          const { return syncObjInfo->get_tag();          }
  bool communicator_defined() const { 
    return syncObjInfo->communicator_defined();
  }
  bool tag_defined() const { return syncObjInfo->tag_defined(); }

  bool allMessages()   const { return syncObjInfo->allMessages(); }
  bool allBarriers()   const { return syncObjInfo->allBarriers(); }
  bool allSemaphores() const { return syncObjInfo->allSemaphores(); }
  bool allSpinlocks()  const { return syncObjInfo->allSpinlocks(); }  

  pdvector< pdvector<string> > tokenized() const;
};

class ostream;

ostream& operator<<(ostream &s, const Focus &f);

// returns a focus, the same as the given focus, only with the given pid
Focus adjustFocusForPid(const Focus &foc, int pid);


#endif

