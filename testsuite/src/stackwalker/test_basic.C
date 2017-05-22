#include "walker.h"
#include "procstate.h"
#include "frame.h"
#include "swk_errors.h"

#include <assert.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <stdio.h>
#include <errno.h>
#include <vector>
#include <string.h>
#include <signal.h>

#if defined(use_symtab)
#include "Variable.h"
#include "Function.h"
#include "Type.h"

#include "local_var.h"
using namespace Dyninst::SymtabAPI;
#endif

using namespace Dyninst;
using namespace Dyninst::Stackwalker;
using namespace std;

bool getVariables(std::vector<Frame> &swalk, unsigned frame);
bool isSignTst;

void walk_stack1(Walker *walker);
void parse_args(std::vector<Walker *> &walkers, int argc, char *argv[]);
void usage();

void *a = 0;

bool isSelfSW = false;

/**
 * An example and test showing how to build a stackwalker object, how to
 * handle the debugging aspects of the stackwalker, and finally how to collect
 * a stackwalk.
 *
 * This tool takes a collection of processess on the command line that may include
 * attached processes, created processes, or the current process.  Once every
 * five seconds, or when input is available on stdin, a stackwalk is taken.
 **/
int main(int argc, char *argv[])
{
  std::vector<Walker *> walkers;
  parse_args(walkers, argc, argv);
  if (!walkers.size())
     usage();

    printf("walker size %d\n",walkers.size());


  /**
   * The notification FD is used by third party stackwalkers (debuggers) to tell
   * the user tool when control needs to be given to StackwalkerAPI to handle a
   * debug event.  If data is available on the notificationFD then the user
   * should call handleDebugEvent.
   **/
  int notification_fd = -1;
  for (unsigned i=0; i<walkers.size(); i++)
  {
     ProcDebug *pdebug = dynamic_cast<ProcDebug *>(walkers[i]->getProcessState());
     if (pdebug)
     {
        //We using at least one debugger walker.  Use the notification fd.
        notification_fd = ProcDebug::getNotificationFD();
        break;
     }
  }

  struct timeval timeout;
  timeout.tv_sec = 5;
  timeout.tv_usec = 0;

  while (walkers.size())
  {
    //Set up the select call.  Wait for input on stdin or the notificationFD.
    int max = 1;
    fd_set readfds, writefds, exceptfds;
    FD_ZERO(&readfds);
    FD_ZERO(&writefds);
    FD_ZERO(&exceptfds);
    FD_SET(0, &readfds);
    if (notification_fd != -1) {
       FD_SET(notification_fd, &readfds);
       max = notification_fd + 1;
    }

    /**
     * Wait for one of:
     *  5 seconds                     - Then do a stackwalk
     *  input on stdin                - Then do a stackwalk
     *  input on the notificationFD   - Call handleDebugEvent to let the
     *                                  debugger process any events.
     **/
    int result = select(max, &readfds, &writefds, &exceptfds, &timeout);
    if (result == -1 && errno != EINTR)
    {
      perror("select failure");
      return 0;
    }

    if (notification_fd != -1 && FD_ISSET(notification_fd, &readfds))
    {
       //Handle the event.  Should check for process exit here.
       ProcDebug::handleDebugEvent(false); //false = non-blocking
       continue;
    }

    bool has_live_walker = false;
    for (unsigned i=0; i<walkers.size(); i++) {
       if (!walkers[i])
          continue;
       ProcDebug *dbg = dynamic_cast<ProcDebug*>(walkers[i]->getProcessState());
       if (!dbg) {
          has_live_walker = true;
          continue;
       }
       if (dbg->isTerminated())
          walkers[i] = NULL;
       else
          has_live_walker = true;
    }
    if (!has_live_walker) {
       printf("Last walker exited.  Terminating\n");
       exit(0);
    }

    if (FD_ISSET(0, &readfds))
    {
      //Input was available on stdin.  If it's 'q', then quit.
      char c;
      int result_i;
      result_i = read(0, &c, 1);
      if (result_i == -1) {
         perror("Couldn't read from 0");
         return -1;
      }
      if (c == 'q')
         break;
    }
    if (result >= 0)
    {
      //Either the five seconds timed out (result == 0) or we have input
      // on stdin (result == 1).  Perform a stackwalk.
      for (unsigned i=0; i<walkers.size(); i++)
      {
         if (!walkers[i])
            continue;
         printf("\n%d:\n", walkers[i]->getProcessState()->getProcessId());
         walk_stack1(walkers[i]);
      }
      timeout.tv_sec = 5;
    }
  }
  return 0;
}

void usage()
{
  fprintf(stderr, "usage: test_basic [ -self | -attach <pid> | -create " \
          "<exec> <args> ]\n\n");
  fprintf(stderr, "You may specify multiple processes at once e.g:\n");
  fprintf(stderr, "\t test_basic -self -create foo arg1 -attach 5000 -attach 5002\n");
  exit(-1);
}

void parse_args(std::vector<Walker *> &walkers, int argc, char *argv[])
{
  for (int i=1; i<argc; i++)
  {
    Walker *walker;
    if (strcmp(argv[i], "-attach") == 0 && i+1 < argc) {
       //Third party stack walker, attached to a pid
      walker = Walker::newWalker(atoi(argv[++i]));
       if (walker) {
          ProcDebug *pd = dynamic_cast<ProcDebug *>(walker->getProcessState());
          assert(pd);
          pd->resume();
       }
    }
    else if (strcmp(argv[i], "-self") == 0) {
       //First party stack walker
      walker = Walker::newWalker();
      isSelfSW = true;
    }
    else if (strcmp(argv[i], "-create") == 0 && i+1 < argc) {
        isSelfSW = false;
       //Thread party stackwalker, creates an executable
       string exec_name(argv[++i]);
       if( strcmp(exec_name.c_str(), "stack_signal") ){
            isSignTst = true;
       }
       vector<string> args;
       while ((i < argc) && (strcmp(argv[i], "--") != 0))
       {
          args.push_back(argv[i]);
          i++;
       }
       walker = Walker::newWalker(exec_name, args);
       if (walker) {
          ProcDebug *pd = dynamic_cast<ProcDebug *>(walker->getProcessState());
          assert(pd);
          pd->resume();
       }
    }
    else {
       usage();
    }
    if (!walker)
    {
      fprintf(stderr, "Couldn't creating walker: %s\n",
              getLastErrorMsg());
      continue;
    }
    walkers.push_back(walker);
  }
}

void print_loc(location_t loc)
{
   switch (loc.location) {
      case loc_address:
        isSelfSW?
         printf("Addr: 0x%lx (0x%lx)", loc.val.addr, *((unsigned long *) loc.val.addr)) :
         printf("Addr: 0x%lx ", loc.val.addr);
         break;
      case loc_register:
         printf("Register: %d", (int) loc.val.reg);
         break;
      case loc_unknown:
         printf("Unknown");
         break;
   }
}

/**
 * Walk the stack for the given walker object.  Each walker object is
 * associated with one process.
 **/
void walk_stack3(Walker *walker)
{
  std::vector<Frame> swalk;

  std::vector<Dyninst::THR_ID> threads;

  ProcDebug *dbg = dynamic_cast<ProcDebug *>(walker->getProcessState());
  if (dbg) {
     dbg->pause();
     if (dbg->isTerminated())
        return;
  }
  walker->getAvailableThreads(threads);

  printf("Stack for process %d\n",
            walker->getProcessState()->getProcessId());
  for (unsigned j=0; j<threads.size(); j++) {
     swalk.clear();
     walker->walkStack(swalk, threads[j]);
     printf("   Stack for thread %d\n", threads[j]);

     for (unsigned i=0; i<swalk.size(); i++) {
        std::string name;
        swalk[i].getName(name);
        printf("   %s @ 0x%lx, FP = 0x%lx    ",
               name.c_str(),
               (unsigned long) swalk[i].getRA(),
               (unsigned long) swalk[i].getFP());
#if defined(use_symtab)
        getVariables(swalk, i);
#endif
        printf("\n");
        printf("      RALoc: ");
        print_loc(swalk[i].getRALocation());
        printf(", FPLoc: ");
        print_loc(swalk[i].getFPLocation());
        printf(", SPLoc: ");
        print_loc(swalk[i].getSPLocation());
        printf("\n");
     }
  }
  if (dbg)
     dbg->resume();
}

#if defined(use_symtab)
bool getVariables(std::vector<Frame> &swalk, unsigned frame)
{
   Function *f = getFunctionForFrame(swalk[frame]);
   if (!f) {
      printf("<no func>\n", swalk[frame].getRA());
      return false;
   }
   std::vector<localVar *> vars;
   bool result = f->getLocalVariables(vars);
   if (!result) {
      printf("<no vars>\n",
              f->getAllMangledNames()[0].c_str());
      return false;
   }

   std::vector<localVar *>::iterator i;
   int count = 0;
   for (i = vars.begin(); i != vars.end(); i++, count++) {
      char buffer[1024];
      localVar *var = *i;
      int result = getLocalVariableValue(*i, swalk, frame, buffer, 1024);
      if (count % 4 == 0 && count != 0)
         printf("\n      ");
      else
         printf(" ");

      if (result != glvv_Success) {
         printf("<%s = no value>", var->getName().c_str());
      }
      else if (var->getType()->getSize() == 8 && sizeof(unsigned long) == 8) {
         printf("<%s = %lu>", var->getName().c_str(), *((unsigned long*) buffer));
      }
      else if (var->getType()->getSize() == 8) {
         printf("<%s = %llu>", var->getName().c_str(), *((unsigned long long*) buffer));
      }
      else if (var->getType()->getSize() == 4) {
         printf("<%s = %u>", var->getName().c_str(), *((unsigned int *) buffer));
      }
      else {
         printf("<%s = Unprintable Size>", var->getName().c_str());
      }
   }
   return true;
}
#endif

void walk_stack2(Walker *walker)
{
   if (a) {
      printf("I'm an a\n");
   }
   walk_stack3(walker);
   if (a) {
      printf("I'm an\n");
   }
}

void walk_stack1(Walker *walker)
{
   if (a) {
      printf("I'm no walker\n");
   }
   walk_stack2(walker);
   if (a) {
      printf("I'm no walker\n");
   }
}

