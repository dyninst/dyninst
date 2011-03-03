#if !defined (GENERATOR_H_)
#define GENERATOR_H_

#include "dyntypes.h"
#include "Decoder.h"
#include "PCErrors.h"

#include "common/h/dthread.h"

#include <set>
#include <map>
#include <string>

struct GeneratorMTInternals;

namespace Dyninst {
namespace ProcControlAPI {

class Process;
class ArchEvent;
class Event;
class Mailbox;

class Generator
{
 public:
   static Generator *getDefaultGenerator();
   static void stopDefaultGenerator();
   virtual bool getAndQueueEvent(bool block) = 0;
   virtual ~Generator();
   
   typedef void (*gen_cb_func_t)();
   static void registerNewEventCB(gen_cb_func_t func);
   static void removeNewEventCB(gen_cb_func_t);
 protected:

   //State tracking
   typedef enum {
      none,
      initializing,
      process_blocked,
      system_blocked,
      decoding,
      handling,
      queueing,
      error,
      exiting
   } state_t;
   state_t state;
   bool isExitingState();
   void setState(state_t newstate);

   //Event handling
   static std::map<Dyninst::PID, Process *> procs;
   typedef std::set<Decoder *, decoder_cmp> decoder_set_t;
   decoder_set_t decoders;

   //Misc
   bool hasLiveProc();
   std::string name;
   Generator(std::string name_);
   bool getAndQueueEventInt(bool block);

   static std::set<gen_cb_func_t> CBs;
   static Mutex *cb_lock;

   //Public interface
   //  Implemented by architectures
   virtual bool initialize() = 0;
   virtual bool canFastHandle() = 0;
   virtual ArchEvent *getEvent(bool block) = 0;
   virtual bool plat_skipGeneratorBlock();
   //  Implemented by MT or ST
   virtual bool processWait(bool block) = 0;
};

class GeneratorMT : public Generator
{
 private:
   GeneratorMTInternals *sync;
   void main();

 public:
   void launch(); //Launch thread
   void start(); //Startup function for new thread
   GeneratorMT(std::string name_);
   virtual ~GeneratorMT();
   
   virtual bool processWait(bool block);
   virtual bool getAndQueueEvent(bool block);
};

class GeneratorST : public Generator
{
 public:
   GeneratorST(std::string name_);
   virtual ~GeneratorST();
   
   virtual bool processWait(bool block);
   virtual bool getAndQueueEvent(bool block);
};

}
}

#endif
