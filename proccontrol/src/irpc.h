#if !defined(IRPC_H_)
#define IRPC_H_

#include <map>
#include <list>

#include "dynutil/h/dyntypes.h"
#include "proccontrol/h/Handler.h"
#include "int_process.h"
#include <stdlib.h>

using namespace Dyninst;
using namespace ProcControlAPI;

class int_process;
class int_thread;
class iRPCAllocation;
class int_iRPC;

class iRPCAllocation
{
   friend void dyn_detail::boost::checked_delete<iRPCAllocation>(iRPCAllocation *);
  public:
   typedef dyn_detail::boost::shared_ptr<iRPCAllocation> ptr;
  iRPCAllocation() :
      addr(0),
      size(0),
      start_offset(0),
      orig_data(NULL),
      needs_datasave(true),
      have_saved_regs(false),
      ref_count(0)
      {
      }
   ~iRPCAllocation() 
      {
         if (orig_data)
            free(orig_data);
      }

   Dyninst::Address addr;
   unsigned long size;
   unsigned start_offset;
   void *orig_data;
   bool needs_datasave;
   bool have_saved_regs;
   int ref_count;

   //These are NULL if the user handed us memory to run the iRPC in.
   dyn_detail::boost::weak_ptr<int_iRPC> creation_irpc;
   dyn_detail::boost::weak_ptr<int_iRPC> deletion_irpc;
};

class int_iRPC : public dyn_detail::boost::enable_shared_from_this<int_iRPC>
{
   friend void dyn_detail::boost::checked_delete<int_iRPC>(int_iRPC *);   
 public:
   typedef dyn_detail::boost::shared_ptr<int_iRPC> ptr;


   typedef enum {
      Unassigned = 0,
      Posted = 1,     //RPC is in queue to run
      Prepping = 2,   //Thread/Process is being stopped to setup RPC
      Prepped = 3,    //Thread/Process has been stopped to setup RPC
      Ready = 4,      //RPC is setup on thread and needs continue
      Running = 5,    //RPC is running
      Cleaning = 6,   //RPC is complete and is being remove
      Finished = 7    //RPC ran
   } State;
   typedef enum {
      NoType,
      Allocation,
      Deallocation,
      User,
      InfMalloc,
      InfFree
   } Type;

   int_iRPC(void *binary_blob_, 
            unsigned long binary_size_,
            bool async_, 
            bool alreadyAllocated = false, 
            Dyninst::Address addr = 0);

   ~int_iRPC();

   unsigned long id() const;
   State getState() const;
   Type getType() const;
   const char *getStrType() const;
   const char *getStrState() const;
   void *binaryBlob() const;
   unsigned long binarySize() const;
   unsigned long startOffset() const;
   bool isAsync() const;
   int_thread *thread() const;
   IRPC::weak_ptr getIRPC() const;
   iRPCAllocation::ptr allocation() const;
   iRPCAllocation::ptr targetAllocation() const;
   bool isProcStopRPC() const;
   bool isInternalRPC() const;

   unsigned long allocSize() const;
   Dyninst::Address addr() const;
   bool hasSavedRegs() const;
   bool userAllocated() const;
   bool shouldSaveData() const;
   bool isMemManagementRPC() const;
   int_iRPC::ptr allocationRPC() const;
   int_iRPC::ptr deletionRPC() const; 
   bool needsToDesync() const; 
   bool isRPCPrepped();

   void setNeedsDesync(bool b);
   void setState(State s);
   void setType(Type t);
   void setBinaryBlob(void *b);
   void setBinarySize(unsigned long s);
   void copyBinaryBlob(void *b, unsigned long s);
   void setStartOffset(unsigned long o);
   void setAsync(bool a);
   void setThread(int_thread *t);
   void setIRPC(IRPC::weak_ptr o);
   void setAllocation(iRPCAllocation::ptr i);
   void setTargetAllocation(iRPCAllocation::ptr i);
   void setAllocSize(unsigned long size);
   void setShouldSaveData(bool b);
   bool fillInAllocation();

   void markReady();
   void markRunning();
   int_iRPC::ptr newAllocationRPC();
   int_iRPC::ptr newDeallocationRPC();
   
   Dyninst::Address infMallocResult();
   void setMallocResult(Dyninst::Address addr);
   rpc_wrapper *getWrapperForDecode();
 private:
   static unsigned long next_id;
   unsigned long my_id;
   State state;
   Type type;
   void *binary_blob;
   unsigned long binary_size;
   unsigned long start_offset;
   bool async;
   int_thread *thrd;
   iRPCAllocation::ptr cur_allocation;
   iRPCAllocation::ptr target_allocation;
   IRPC::weak_ptr hl_irpc;
   bool freeBinaryBlob;
   bool needsDesync;
   int lock_live;
   bool needs_clean;
   Dyninst::Address malloc_result;
};

//Singleton class, only one of these across all processes.
class iRPCMgr
{
   friend class iRPC;
   friend class iRPCHandler;
 public:
   iRPCMgr();
   ~iRPCMgr();

   unsigned numActiveRPCs(int_thread *thr);
   iRPCAllocation::ptr findAllocationForRPC(int_thread *thread, int_iRPC::ptr rpc);
   
   bool postRPCToProc(int_process *proc, int_iRPC::ptr rpc);
   bool postRPCToThread(int_thread *thread, int_iRPC::ptr rpc);
   bool prepNextRPC(int_thread *thr, bool sync_prep, bool &user_error);
   bool runNextRPC(int_thread *thread, bool block);

   bool createAllocationSnippet(int_process *proc, Dyninst::Address addr,
                                bool use_addr,
                                unsigned long size, void* &buffer, 
                                unsigned long &buffer_size,
                                unsigned long &start_offset);
   bool createDeallocationSnippet(int_process *proc, Dyninst::Address addr, 
                                  unsigned long size, void* &buffer, 
                                  unsigned long &buffer_size, unsigned long &start_offset);
   bool collectAllocationResult(int_thread *thr, Dyninst::Address &addr, bool &err);

   int_iRPC::ptr createInfMallocRPC(int_process *proc, unsigned long size, bool use_addr, Dyninst::Address addr);
   int_iRPC::ptr createInfFreeRPC(int_process *proc, unsigned long size, Dyninst::Address addr);

   bool checkIfNeedsProcStop(int_process *p);
   bool stopNeededThreads(int_process *p, bool sync);

   bool handleThreadContinue(int_thread *thr, bool user_cont);
   bool isRPCTrap(int_thread *thr, Dyninst::Address addr);
};

iRPCMgr *rpcMgr();

//Runs after user callback
class iRPCHandler : public Handler
{
 public:
   iRPCHandler();
   virtual ~iRPCHandler();
   virtual bool handleEvent(Event::ptr ev);
   virtual void getEventTypesHandled(std::vector<EventType> &etypes);
   virtual int getPriority() const;
};

//Wraps an int_iRPC::ptr so that the user level class IRPC doesn't
//need to directly maintain a shared pointer into internal code.
class rpc_wrapper
{
  public:
   rpc_wrapper(int_iRPC::ptr rpc_) :
     rpc(rpc_)
   {
   }
    
   ~rpc_wrapper()
   {
   }
   
   int_iRPC::ptr rpc;
};
#endif
