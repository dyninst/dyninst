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

#include <BPatch.h>
#include <BPatch_process.h>
#include <BPatch_thread.h>
#include <BPatch_image.h>
#include <BPatch_function.h>
#include <assert.h>
#include "test_util.h"

BPatch bpatch;
BPatch_process *proc;

bool debug_flag = false;
#define dprintf if (debug_flag) fprintf

#define MAX_ARGS 32
char *filename = "test14.mutatee_gcc";
bool should_exec = true;
char *args[MAX_ARGS];
char *create_arg = "-create";
unsigned num_args = 0; 

void instr_func(BPatch_function *func, BPatch_function *lvl1func)
{
   BPatch_Vector<BPatch_point *> *points;
   points = func->findPoint(BPatch_entry);
   for (unsigned j=0; j < points->size(); j++)
   {
      BPatch_point *point = (*points)[j];
      BPatch_Vector<BPatch_snippet *> args;
      BPatch_funcCallExpr callToLevel1(*lvl1func, args);
      BPatchSnippetHandle *hndl;
      hndl = proc->insertSnippet(callToLevel1, *point, BPatch_firstSnippet);
      assert(hndl);
   }
}

static BPatch_process *getProcess()
{
   args[0] = filename;

   BPatch_process *proc;
   if (should_exec) {
      args[1] = create_arg;
      proc = bpatch.processCreate(filename, (const char **) args);
      if(proc == NULL) {
         fprintf(stderr, "%s[%d]: processCreate(%s) failed\n", 
                 __FILE__, __LINE__, filename);
         return NULL;
      }
      
   }
   else
   {
      int pid = startNewProcessForAttach(filename, (const char **) args);
      if (pid < 0)
      {
         fprintf(stderr, "%s ", filename);
         perror("couldn't be started");
         return NULL;
      }
      proc = bpatch.processAttach(filename, pid);  
      if(proc == NULL) {
         fprintf(stderr, "%s[%d]: processAttach(%s, %d) failed\n", 
                 __FILE__, __LINE__, filename, pid);
         return NULL;
      }
      BPatch_image *appimg = proc->getImage();
      signalAttached(NULL, appimg);    
   }
   return proc;
}

int main(int argc, char *argv[])
{
   int i;
   char libRTname[256];

   libRTname[0]='\0';
   if (!getenv("DYNINSTAPI_RT_LIB")) {
      fprintf(stderr,"Environment variable DYNINSTAPI_RT_LIB undefined:\n"
#if defined(i386_unknown_nt4_0)
		 "    using standard search strategy for libdyninstAPI_RT.dll\n");
#else
	         "    set it to the full pathname of libdyninstAPI_RT\n");
      exit(-1);
#endif
   } else
      strcpy((char *)libRTname, (char *)getenv("DYNINSTAPI_RT_LIB"));

   for (i = 1; i < argc; ++i) {
       if (strcmp(argv[i], "-attach") == 0) {
          should_exec = false;
       }
       else if (strcmp(argv[i], "-mutatee") == 0) {
          if (++i >= argc) {
             fprintf(stderr, "ERROR: -mutatee flag requires an argument\n");
             exit(-1);
          }
          filename = argv[i];
       }
       else if (strcmp(argv[i], "-verbose") == 0)
          debug_flag = true;
       else if (!strcmp(argv[i], "-V")) {
          if (libRTname[0]) {
             fprintf (stdout, "DYNINSTAPI_RT_LIB=%s\n", libRTname);
             fflush(stdout);
          }
       }
       else {
          fprintf(stderr, "Usage: test14 [-V] [-verbose] [-attach]|[-mutatee <file>]\n");
          exit(-1);
       }
   }

   proc = getProcess();
      return -1;

   BPatch_image *image = proc->getImage();
   BPatch_Vector<BPatch_function *> lvl1funcs;
   image->findFunction("level1", lvl1funcs);
   if (lvl1funcs.size() != 1)
   {
      fprintf(stderr, "[%s:%u] - Found %d level1 functions.  Expected 1\n",
              __FILE__, __LINE__, lvl1funcs.size());
      return -1;
   }
   BPatch_function *lvl1func = lvl1funcs[0];

   // Instrument level[0-3] entry points to call level1()
   BPatch_Vector<BPatch_function *> funcs;
   image->findFunction("level0", funcs);
   instr_func(funcs[0], lvl1func);
   funcs.clear();
   
   instr_func(lvl1func, lvl1func);
   
   image->findFunction("level2", funcs);
   instr_func(funcs[0], lvl1func);
   funcs.clear();
   
   image->findFunction("level3", funcs);
   instr_func(funcs[0], lvl1func);
   funcs.clear();

   proc->continueExecution();

   do {
      bpatch.waitForStatusChange();
   } while (!proc->isTerminated());

   int exitCode = proc->getExitCode();
   if (exitCode)
   {
       fprintf(stdout, "*** Failed test #1 (Multithreaded tramp guards)\n");
       return -1;
   }
   else
   {
       fprintf(stdout, "Passed test #1 (Multithreaded tramp guards)\n");
       fprintf(stdout, "All tests passed.\n");
   }
   return 0;
}
