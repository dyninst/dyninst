/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
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
  virtual pdstring getName() const = 0;
  virtual pdvector<pdstring> tokenized() const = 0;
  virtual bool focus_matches(const pdvector<pdstring> &match_path) const = 0;
};


// --------------------------------------------------------------
// DIFFERENT TYPES OF HIERARCHIES

class machineHierarchy : public Hierarchy {
  pdstring machine;
  pdstring process;
  pdstring thread;
 public:
  machineHierarchy(const pdvector<pdstring> &setupInfo);
  int getPid() const;
  void setPid(int pid);
  int getThreadID() const;
  pdstring getName() const;
  bool allMachines() const { 
    return (!machine_defined() && !process_defined() && !thread_defined());
  }
  pdstring get_machine() const { return machine; }
  pdstring get_process() const { return process; }
  pdstring get_thread()  const { return thread; }
  bool machine_defined() const { return (machine.length()>0); }
  bool process_defined() const { return (process.length()>0); }
  bool thread_defined()  const { return (thread.length()>0);  }
  void set_machine(const pdstring &machine_) {  machine = machine_;  }
  void set_process(const pdstring &process_) {  process = process_; }
  void set_thread(const pdstring &thread_)   {  thread  = thread_;  }

  bool focus_matches(const pdvector<pdstring> &match_path) const;
  pdvector<pdstring> tokenized() const;
};


class codeHierarchy : public Hierarchy {
  pdstring module;
  pdstring function;
  pdstring loop;
 public:
  codeHierarchy(const pdvector<pdstring> &setupInfo);
  pdstring getName() const;  
  bool allCode() const { 
    return (!module_defined() && !function_defined());
  }
  pdstring get_module()   const { return module;   }
  pdstring get_function() const { return function; }
  bool module_defined()   const { return (module.length()>0);   }
  bool function_defined() const { return (function.length()>0); }

  bool focus_matches(const pdvector<pdstring> &match_path) const;
  pdvector<pdstring> tokenized() const;
};

struct message_data_t {
  pdstring communicator;
  pdstring tag;
};
  
struct barrier_data_t {
  pdstring barrierStr;    // this may or may not be used
};

struct semaphore_data_t {
  pdstring semaphoreStr;  // this may or may not be used
};
  
struct spinlock_data_t {
  pdstring spinlockStr;   // this may or may not be used
};

struct mutex_data_t {
   pdstring mutexStr;
};

struct condvar_data_t {
   pdstring condVarStr;
};

struct rwlock_data_t {
   pdstring rwlockStr;
};

class syncObjHierarchy : public Hierarchy {
  typedef enum { NoSyncObjT, MessageT, BarrierT, 
                 SemaphoreT, SpinlockT, MutexT, CondVarT,
                 RwLockT } sync_obj_type;

  sync_obj_type syncObjType;
  message_data_t   messageData;
  barrier_data_t   barrierData;
  semaphore_data_t semaphoreData;
  spinlock_data_t  spinlockData;
  mutex_data_t     mutexData;
  condvar_data_t   condVarData;
  rwlock_data_t    rwlockData;

 public:
  syncObjHierarchy(const pdvector<pdstring> &setupInfo);
  pdstring getName() const;
  bool allSync() const { return (syncObjType == NoSyncObjT); }
  bool allMessages() const { 
    return (syncObjType == MessageT  && !communicator_defined()  &&
            !tag_defined());
  }
  bool allBarriers() const {
    return (syncObjType == BarrierT  && barrierData.barrierStr.length() == 0);
  }
  bool allSemaphores() const {
    return (syncObjType == SemaphoreT  &&
            semaphoreData.semaphoreStr.length() == 0);
  }
  bool allSpinlocks() const {
    return (syncObjType == SpinlockT  && 
            spinlockData.spinlockStr.length() == 0);
  }
  bool allMutexes() const {
    return (syncObjType == MutexT  && mutexData.mutexStr.length() == 0);
  }
  bool allCondVars() const {
    return (syncObjType == CondVarT  && condVarData.condVarStr.length() == 0);
  }
  bool allRwLocks() const {
    return (syncObjType == RwLockT  && rwlockData.rwlockStr.length() == 0);
  }

  pdstring get_communicator() const { 
     if(syncObjType == MessageT)
        return messageData.communicator;
     else
        return pdstring("");
  }
  pdstring get_tag() const { 
     if(syncObjType == MessageT)
        return messageData.tag; 
     else
        return pdstring("");
  }
  pdstring get_mutex() const {
     if(syncObjType == MutexT)
        return mutexData.mutexStr;
     else
        return pdstring("");
  }
  pdstring get_condVar() const {
     if(syncObjType == CondVarT)
        return condVarData.condVarStr;
     else
        return pdstring("");
  }
  pdstring get_rwlock() const {
     if(syncObjType == RwLockT)
        return rwlockData.rwlockStr;
     else
        return pdstring("");
  }

  bool communicator_defined() const { 
     return (messageData.communicator.length() > 0);
  }
  bool tag_defined() const { return (messageData.tag.length()>0); }
  bool mutex_defined() const { return (mutexData.mutexStr.length() > 0); }
  bool condVar_defined() const { return (condVarData.condVarStr.length() > 0);}
  bool rwlock_defined() const { return (rwlockData.rwlockStr.length() > 0); }

  bool focus_matches(const pdvector<pdstring> &match_path) const;
  pdvector<pdstring> tokenized() const;
};


class memoryHierarchy : public Hierarchy {
 public:
  memoryHierarchy() { }
  memoryHierarchy(const pdvector<pdstring> &setupInfo);
  pdstring getName() const ;
  bool allMem() const { return true; }
  bool focus_matches(const pdvector<pdstring> &match_path) const;
  pdvector<pdstring> tokenized() const;
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

  pdstring getName() const;
  bool allMachines() const { return machineInfo->allMachines(); }
  bool allCode()     const { return codeInfo->allCode();        }
  bool allSync()     const { return syncObjInfo->allSync();     }
  bool allMem()      const { return allMem();                  }

  pdstring get_machine() const { return machineInfo->get_machine(); }
  pdstring get_process() const { return machineInfo->get_process(); }
  pdstring get_thread()  const { return machineInfo->get_thread();  }
  int getPid() const    { return machineInfo->getPid(); }
  void setPid(int pid)  { machineInfo->setPid(pid); }
  int getThreadID() const    { return machineInfo->getThreadID(); }
  bool machine_defined() const { return machineInfo->machine_defined(); }
  bool process_defined() const { return machineInfo->process_defined(); }
  bool thread_defined()  const { return machineInfo->thread_defined();  }
  void set_machine(const pdstring &mStr) {  machineInfo->set_machine(mStr); }
  void set_process(const pdstring &pStr) {  machineInfo->set_process(pStr); }
  void set_thread(const pdstring &tStr)  {  machineInfo->set_thread(tStr);  }

  pdstring get_module()   const { return codeInfo->get_module();   }
  pdstring get_function() const { return codeInfo->get_function(); }
  bool module_defined() const   { return codeInfo->module_defined();   }
  bool function_defined() const { return codeInfo->function_defined(); }

  pdstring get_communicator() const { return syncObjInfo->get_communicator(); }
  pdstring get_tag()          const { return syncObjInfo->get_tag();          }
  bool communicator_defined() const { 
    return syncObjInfo->communicator_defined();
  }
  bool tag_defined() const { return syncObjInfo->tag_defined(); }

  bool allMessages()   const { return syncObjInfo->allMessages(); }
  bool allBarriers()   const { return syncObjInfo->allBarriers(); }
  bool allSemaphores() const { return syncObjInfo->allSemaphores(); }
  bool allSpinlocks()  const { return syncObjInfo->allSpinlocks(); }  

  pdvector< pdvector<pdstring> > tokenized() const;
};


ostream& operator<<(ostream &s, const Focus &f);

// returns a focus, the same as the given focus, only with the given pid
Focus adjustFocusForPid(const Focus &foc, int pid);


#endif

