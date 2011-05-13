#include "proccontrol_comp.h"
#include "communication.h"

using namespace std;

class pc_libraryMutator : public ProcControlMutator {
public:
  virtual test_results_t executeTest();
};

extern "C" DLLEXPORT TestMutator* pc_library_factory()
{
  return new pc_libraryMutator();
}

struct proc_info {
   int loaded_libtesta;
   int loaded_libtestb;
   int unloaded_libtesta;
   int unloaded_libtestb;
   int order;
   bool found_exec;
   bool found_libc;
   proc_info() :
      loaded_libtesta(-1),
      loaded_libtestb(-1),
      unloaded_libtesta(-1),
      unloaded_libtestb(-1),
      order(0),
      found_exec(false),
      found_libc(false)
   {
   }
};

static std::map<Process::const_ptr, proc_info> proclibs;
static bool got_breakpoint;
static bool myerror;

Process::cb_ret_t on_breakpoint(Event::const_ptr ev)
{
   got_breakpoint = true;
   return Process::cbDefault;
}

Process::cb_ret_t on_library(Event::const_ptr ev)
{
   EventLibrary::const_ptr evlib = ev->getEventLibrary();
   if (!evlib) {
      logerror("error, recieved non library event\n");
      myerror = true;
      return Process::cbDefault;
   }
   proc_info &pi = proclibs[ev->getProcess()];

   std::set<Library::ptr>::const_iterator i;
   for (i = evlib->libsAdded().begin(); i != evlib->libsAdded().end(); i++) {
      Library::ptr lib = *i;
      if (lib->getName().find("libtestA") != string::npos) {
         pi.loaded_libtesta = pi.order++;
      }
      if (lib->getName().find("libtestB") != string::npos) {
         pi.loaded_libtestb = pi.order++;
      }

      bool found_lib = false;
      for (LibraryPool::const_iterator j = ev->getProcess()->libraries().begin();
           j != ev->getProcess()->libraries().end(); j++)
      {
         if (*j == lib) {
            found_lib = true;
            break;
         }
      }
      if (!found_lib) {
         logerror("New library was not in library list\n");
         myerror = true;
      }
   }

   for (i = evlib->libsRemoved().begin(); i != evlib->libsRemoved().end(); i++) {
      Library::ptr lib = *i;
      if (lib->getName().find("libtestA") != string::npos) {
         pi.unloaded_libtesta = pi.order++;
      }
      if (lib->getName().find("libtestB") != string::npos) {
         pi.unloaded_libtestb = pi.order++;
      }

      for (LibraryPool::const_iterator j = ev->getProcess()->libraries().begin();
           j != ev->getProcess()->libraries().end(); j++)
      {
         if (*j == lib) {
            logerror("Removed library was still in library list\n");
            myerror = true;
         }
      }
   }

   return Process::cbDefault;
}

test_results_t pc_libraryMutator::executeTest()
{
   proclibs.clear();
   got_breakpoint = false;
   myerror = false;

   Process::registerEventCallback(EventType::Breakpoint, on_breakpoint);
   Process::registerEventCallback(EventType::Library, on_library);

   std::vector<Process::ptr>::iterator i;

   for (i = comp->procs.begin(); i != comp->procs.end(); i++) {
      std::string libc_fullname;
      Library::ptr libc_lib;

      Process::ptr proc = *i;
      Process::const_ptr cproc = proc;
      proc_info &pi = proclibs[cproc];
      
      for (LibraryPool::iterator j = proc->libraries().begin();
           j != proc->libraries().end(); j++)
      {
         Library::ptr lib = *j;
         if (lib->getName().find("libc")) {
            pi.found_libc = true;
            libc_fullname = lib->getName();
            libc_lib = lib;
         }
         if (lib->getName().find("pc_library_mutatee")) {
            pi.found_exec = true;
         }
      }
      
      Library::ptr libc_lib2 = proc->libraries().getLibraryByName(libc_fullname);
      if (libc_lib != libc_lib2) {
         logerror("Failed to find libc in getLibraryByName\n");
         myerror = true;
      }

      bool result = proc->continueProc();
      if (!result) {
         logerror("Failed to continue process\n");
         myerror = true;
      }
   }

   syncloc loc[NUM_PARALLEL_PROCS];
   bool result = comp->recv_broadcast((unsigned char *) loc, sizeof(syncloc));
   if (!result) {
      logerror("Failed to recieve sync broadcast\n");
      myerror = true;
   }
   for (unsigned j=0; j<comp->procs.size(); j++) {
      if (loc[j].code != SYNCLOC_CODE) {
         logerror("Recieved unexpected message code\n");
         myerror = true;
      }
   }

   result = comp->send_broadcast((unsigned char *) loc, sizeof(syncloc));
   if (!result) {
      logerror("Failed to send sync broadcast\n");
      myerror = true;
   }

   if (got_breakpoint) {
      logerror("Recieved breakpoint, shouldn't have\n");
      myerror = true;
   }
   if (comp->procs.size() != proclibs.size()) {
      logerror("Didn't get library events from enough processes\n");
      myerror = true;
   }
   for (std::map<Process::const_ptr, proc_info>::iterator j = proclibs.begin();
        j != proclibs.end(); j++)
   {
      const proc_info &pi = j->second;
      if (pi.loaded_libtesta == -1) {
         logerror("Didn't load libtestA\n");
         myerror = true;
      }
      if (pi.loaded_libtestb == -1) {
         logerror("Didn't load libtestB\n");
         myerror = true;
      }
      if (pi.unloaded_libtesta == -1) {
         logerror("Didn't unload libtestA\n");
         myerror = true;
      }
      if (pi.unloaded_libtestb == -1) {
         logerror("Didn't unload libtestA\n");
         myerror = true;
      }
      if (pi.loaded_libtesta != 0 ||
          pi.loaded_libtestb != 1 ||
          pi.unloaded_libtestb != 2 ||
          pi.unloaded_libtesta != 3)
      {
         logerror("Unexpected library load order\n");
         myerror = true;
      }
      if (!pi.found_exec) {
         logerror("Failed to find executable\n");
         myerror = true;
      }
      if (!pi.found_libc) {
         logerror("Failed to find libc\n");
         myerror = true;
      }
   }
   
   Process::removeEventCallback(on_library);
   Process::removeEventCallback(on_breakpoint);

   return myerror ? FAILED : PASSED;
}
