
#include "proccontrol/h/Handler.h"
#include "proccontrol/h/Process.h"
#include <map>

#if !defined(INT_HANDLER_H_)
#define INT_HANDLER_H_

using namespace std;
using namespace Dyninst;
using namespace ProcControlAPI;

struct handler_cmp
{
   bool operator()(const Handler* a, const Handler* b)
   {
      return a->getPriority() < b->getPriority();
   }
};

class HandlerPool
{
 public:
   typedef set<Handler *, handler_cmp> HandlerSet_t;
   typedef map<EventType, HandlerSet_t*, eventtype_cmp> HandlerMap_t;

   HandlerPool();
   ~HandlerPool();

   void addHandler(Handler *handler);
   bool handleEvent(Event::ptr ev);
 private:
   HandlerMap_t handlers;
   void addHandlerInt(EventType etype, Handler *handler);
};

class HandleBootstrap : public Handler
{
 public:
   HandleBootstrap();
   virtual ~HandleBootstrap();

   virtual void getEventTypesHandled(vector<EventType> &etypes);
   virtual bool handleEvent(Event::ptr ev);
};

class HandleCrash : public Handler
{
 public:
  HandleCrash();
  ~HandleCrash();

   virtual void getEventTypesHandled(vector<EventType> &etypes);
   virtual bool handleEvent(Event::ptr ev);  
};

class HandleSignal : public Handler
{
 public:
   HandleSignal();
   ~HandleSignal();
   
   virtual void getEventTypesHandled(vector<EventType> &etypes);
   virtual bool handleEvent(Event::ptr ev);
};

class HandlePostExit : public Handler
{
 public:
   HandlePostExit();
   ~HandlePostExit();

   virtual void getEventTypesHandled(vector<EventType> &etypes);
   virtual bool handleEvent(Event::ptr ev);
};

class HandlePreExit : public Handler
{
 public:
   HandlePreExit();
   ~HandlePreExit();

   virtual void getEventTypesHandled(vector<EventType> &etypes);
   virtual bool handleEvent(Event::ptr ev);
};

class HandleThreadCreate : public Handler
{
 public:
   HandleThreadCreate();
   ~HandleThreadCreate();
   
   virtual void getEventTypesHandled(vector<EventType> &etypes);
   virtual bool handleEvent(Event::ptr ev);   
};

class HandleThreadDestroy : public Handler
{
 public:
   HandleThreadDestroy();
   ~HandleThreadDestroy();
      
   virtual void getEventTypesHandled(vector<EventType> &etypes);
   virtual bool handleEvent(Event::ptr ev);   
};

class HandleThreadStop : public Handler
{
 public:
  HandleThreadStop();
  ~HandleThreadStop();

  virtual void getEventTypesHandled(vector<EventType> &etypes);
  virtual bool handleEvent(Event::ptr ev);
};

class HandlePostFork : public Handler
{
  public:
   HandlePostFork();
   ~HandlePostFork();

  virtual void getEventTypesHandled(vector<EventType> &etypes);
  virtual bool handleEvent(Event::ptr ev);
};

class HandlePostExec : public Handler
{
  public:
   HandlePostExec();
   ~HandlePostExec();

   virtual void getEventTypesHandled(vector<EventType> &etypes);
   virtual bool handleEvent(Event::ptr ev);
};

class HandleSingleStep : public Handler
{
 public:
  HandleSingleStep();
  ~HandleSingleStep();

  virtual void getEventTypesHandled(vector<EventType> &etypes);
  virtual bool handleEvent(Event::ptr ev);
};

class HandleBreakpoint : public Handler
{
 public:
  HandleBreakpoint();
  ~HandleBreakpoint();

  virtual void getEventTypesHandled(vector<EventType> &etypes);
  virtual bool handleEvent(Event::ptr ev);
};

class HandlePostBreakpoint : public Handler
{
 public:
  HandlePostBreakpoint();
  ~HandlePostBreakpoint();

  virtual void getEventTypesHandled(vector<EventType> &etypes);
  virtual bool handleEvent(Event::ptr ev);
  virtual int getPriority() const;
};

class HandleBreakpointClear : public Handler
{
 public:
  HandleBreakpointClear();
  ~HandleBreakpointClear();

  virtual void getEventTypesHandled(vector<EventType> &etypes);
  virtual bool handleEvent(Event::ptr ev);
};

class HandleLibrary : public Handler
{
 public:
   HandleLibrary();
   ~HandleLibrary();

   virtual bool handleEvent(Event::ptr ev);
   virtual void getEventTypesHandled(std::vector<EventType> &etypes);
};

class HandleCallbacks : public Handler
{
  friend class HandlerPool;
 private:
  typedef std::map<EventType, set<Process::cb_func_t>, eventtype_cmp> cbfuncs_t;
  cbfuncs_t cbfuncs;
  set<EventType, eventtype_cmp> alleventtypes;
  bool registerCallback_int(EventType ev, Process::cb_func_t func);
  bool removeCallback_int(EventType et);
  bool removeCallback_int(EventType et, Process::cb_func_t func);
  bool handleCBReturn(Process::const_ptr proc, Thread::const_ptr thrd, 
                      Process::cb_action_t ret);
 public:
  HandleCallbacks();
  ~HandleCallbacks();

  static HandleCallbacks *getCB();
  virtual int getPriority() const;
  virtual void getEventTypesHandled(vector<EventType> &etypes);
  virtual bool handleEvent(Event::ptr ev);
  bool hasCBs(Event::const_ptr ev);
  
  bool registerCallback(EventType ev, Process::cb_func_t func);
  bool removeCallback(EventType et, Process::cb_func_t func);
  bool removeCallback(EventType et);
  bool removeCallback(Process::cb_func_t func);

  bool deliverCallback(Event::ptr ev, const set<Process::cb_func_t> &cbset);
  
  bool requiresCB(Event::const_ptr ev);

};

#endif
