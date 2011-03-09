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
   static const int UserThreadCreate    = 5;
   static const int LWPCreate           = 6;
   static const int UserThreadDestroy   = 7;
   static const int LWPDestroy          = 8;
   static const int Stop                = 9;
   static const int Signal              = 10;
   static const int LibraryLoad         = 11;
   static const int LibraryUnload       = 12;
   static const int Bootstrap           = 13;
   static const int Breakpoint          = 14;
   static const int RPC                 = 15;
   static const int SingleStep          = 16;
   static const int Library             = 17;
   static const int ForceTerminate      = 18;

   //These aren't completely real events.  They can have callbacks registered, but won't be delivered.
   // Instead, a real event will be delivered to their callback.  E.g, a callback registered for 
   // Terminate will actually get Exit or Crash events.
   static const int Terminate           = 400;
   static const int ThreadCreate        = 401;
   static const int ThreadDestroy       = 402;

   //Users do not recieve CBs for the below event types--ProcControlAPI internal
   static const int InternalEvents      = 500;
   static const int BreakpointClear     = 500;
   static const int RPCInternal         = 501;
   static const int Async               = 502;
   static const int ChangePCStop        = 503; // Used for bug_freebsd_change_pc
   static const int PrepSingleStep      = 504;

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
