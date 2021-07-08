/*
 * See the dyninst/COPYRIGHT file for copyright information.
 * 
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
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
#include <iostream>
#include <map>
#include <list>
#include <vector>
#include <string>
#include <algorithm>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

#include "unistd.h"
#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"
#include "BPatch_function.h"
#include "BPatch_flowGraph.h"
#include "BPatch_basicBlock.h"
#include "BPatch_point.h"

#include "dyninstCore.h"
#include "config.h"
#include "ipc.h"
#include "log.h"
#include "utils.h"

/* These are missing on some platforms -- just use O_SYNC 
 * This is the approach Linux uses 
 */
#ifndef O_RSYNC
#define O_RSYNC O_SYNC
#endif

#ifndef O_DSYNC
#define O_DSYNC O_SYNC
#endif

using namespace std;

/*******************************************************************************
 * Monitor functions
 */
static int numInstsAllowed = 0;
bool shouldInsert()
{
   static int numCalled = 0;

   numCalled++;

   if (!config.hunt_crashes || config.hunt_low == -1 || config.hunt_high == -1) {
      numInstsAllowed++;
      return true;
   }

   if (numCalled-1 < config.hunt_low) {
      return false;
   }
   if (numCalled-1 >= config.hunt_high) {
      return false;
   }

   numInstsAllowed++;
   return true;
}

void stateBasedPrint(message *msgData, statusID prevStat, statusID currStat, int *tabDepth, logLevel priority)
{
   switch (currStat) {
      case ID_PASS:
         if (priority <= config.verbose) --(*tabDepth);
         if (prevStat == ID_TEST) {
            switch (getMsgID(msgData->id_data)) {
               case ID_INIT_CREATE_PROCESS: dlog(priority, "Mutatee launched as process %d.\n", msgData->int_data); break;
               case ID_PARSE_MODULE: dlog(priority, "Found %d module(s).\n", msgData->int_data); break;
               case ID_PARSE_FUNC: dlog(priority, "Found %d function(s).\n", msgData->int_data); break;
               case ID_INST_GET_FUNCS: dlog(priority, "Retrieved %d function(s).\n", msgData->int_data); break;
               default: dlog(priority, "done.\n"); break;
            }
            return;
         }
         break;

      case ID_WARN:
         if (priority <= config.verbose) --(*tabDepth);
         if (prevStat == ID_TEST) {
            dlog(priority, "%s\n", msgData->str_data);
            return;
         }
         break;

      case ID_FAIL:
         if (priority <= config.verbose) --(*tabDepth);
         if (prevStat == ID_TEST) {
            dlog(priority, "failed.\n");
         }
         break;

      case ID_INFO:
      case ID_TEST:
         if (prevStat == ID_TEST)
            dlog(priority, "\n");
         break;

      default:
         break;
   }

   if (*tabDepth < 0) dlog(ERR, "%d ", *tabDepth);
   for (int i = 0; i < *tabDepth; ++i) dlog(priority, "    ");

   switch (currStat) {
      case ID_FAIL: dlog(priority, "*** Failure: %s\n", msgData->str_data); break;
      case ID_PASS: dlog(priority, "done.\n"); break;
      case ID_WARN: dlog(priority, "Warning: %s\n", msgData->str_data); break;
      case ID_INFO:
         switch (getMsgID(msgData->id_data)) {
            case ID_EXIT_CODE: dlog(priority, "Mutatee exited normally and returned %d\n", msgData->int_data); break;
            case ID_EXIT_SIGNAL: dlog(priority, "Mutatee exited via signal %d\n", msgData->int_data); break;
            case ID_DATA_STRING: dlog(priority, "%s\n", msgData->str_data); break;
            case ID_TRACE_POINT: dlog(priority, "TRACE_POINT: %s\n", msgData->str_data); break;
            case ID_CRASH_HUNT_NUM_ACTIONS: dlog(priority, "Crash hunting checked %d points\n", msgData->int_data); break;
            default: dlog(ERR, "Internal error.  Invalid MessageID used with ID_INFO status.\n");
         }
         break;
      case ID_TEST:
         switch (getMsgID(msgData->id_data)) {
            case ID_PARSE_FUNC: dlog(priority, "Parsing function data from %s module... ", msgData->str_data); break;
            case ID_PARSE_MODULE_CFG: dlog(priority, "Parsing CFG data from functions in %s module... ", msgData->str_data); break;
            case ID_PARSE_FUNC_CFG: dlog(priority, "Parsing CFG data from %s function... ", msgData->str_data); break;
            case ID_INST_MODULE: dlog(priority, "Instrumenting functions in module %s... ", msgData->str_data); break;
            case ID_INST_FUNC: dlog(priority, "Instrumenting function %s... ", msgData->str_data); break;
            default: dlog(priority, "%s... ", msgStr(getMsgID(msgData->id_data))); break;
         }
         if (priority <= config.verbose) ++(*tabDepth);
         break;
   }
}

int launch_monitor(FILE *infd)
{
   statusID currStat = ID_FAIL;
   statusID prevStat = ID_FAIL, prevStatLog = ID_FAIL;
   int tabDepth = 0, tabDepthLog = 0;
   logLevel priority = INFO;
   message msgData;

   if (config.time_limit > 0)
      alarm(config.time_limit);

   while (config.state == NORMAL && readMsg(infd, &msgData)) {
      prevStatLog = currStat;
      if (priority <= config.verbose)
         prevStat = currStat;
      currStat = getStatID(msgData.id_data);
      priority = getPriID(msgData.id_data);

      if (getMsgID(msgData.id_data) == ID_POST_FORK ||
          (getMsgID(msgData.id_data) == ID_INIT_CREATE_PROCESS && currStat == ID_PASS)) {
         config.grandchildren.insert(msgData.int_data);
      }

      if (getMsgID(msgData.id_data) == ID_DETACH_CHILD && currStat == ID_PASS) {
         config.state = DETACHED;
      }

      if (getMsgID(msgData.id_data) == ID_TRACE_POINT && config.trace_count) {
         config.trace_history.push_back( msgData.str_data );
         if (config.trace_history.size() > config.trace_count)
            config.trace_history.pop_front();
         continue;
      }

      if (getMsgID(msgData.id_data) == ID_CRASH_HUNT_NUM_ACTIONS)
      {
         config.hunt_high = msgData.int_data;
      }

      if (priority <= config.verbose) {
         stateBasedPrint(&msgData, prevStat, currStat, &tabDepth, priority);
         tabDepthLog = tabDepth;
      } else {
         stateBasedPrint(&msgData, prevStatLog, currStat, &tabDepthLog, LOG_ONLY);
      }
      free(msgData.str_data);
   }

   // Unset alarm
   alarm(0);

   if (config.state == NORMAL) {
      // Give everybody a chance to catch up.
      config.state = SIGCHLD_WAIT;
      dlog(INFO, "Waiting for mutator to exit...\n");
      sleep(10);
   }

   // Report trace history, if needed.
   if (config.trace_inst && config.trace_count) {
      while (config.trace_history.size()) {
         dlog(INFO, "TRACE_POINT: %s\n", config.trace_history[0].c_str());
         config.trace_history.pop_front();
      }
   }
   return 0;
}

/*******************************************************************************
 * Mutator functions
 */

void printSummary(BPatch_thread *, BPatch_exitType);
void reportNewProcess(BPatch_thread *, BPatch_thread *);
bool insertSummary(BPatch_function *, BPatch_variableExpr *);
bool generateInstrumentation(dynHandle *, BPatch_function *, BPatch_snippet *);
BPatch_variableExpr *allocateIntegerInMutatee(dynHandle *, BPatch_function *);
bool instrumentFunctionEntry(dynHandle *, BPatch_function *);
bool instrumentFunctionExit(dynHandle *, BPatch_function *);
bool instrumentBasicBlocks(dynHandle *, BPatch_function *);
bool instrumentMemoryReads(dynHandle *, BPatch_function *);
bool instrumentMemoryWrites(dynHandle *, BPatch_function *);

// Helpful global variables for instrumentation tracing.
static int trace_msglen = sizeof(void *) * 2;
static BPatch_variableExpr *trace_fd = NULL;
static BPatch_function *trace_write = NULL;
static map< void *, BPatch_function * > trace_points;

bool initTraceInMutatee(dynHandle *);
bool generateTraceSnippet(dynHandle *, BPatch_function *);
void readTracePipe();

int launch_mutator()
{
   unsigned i = 0, j = 0;
   const char *reason = NULL;
   char buf[STRING_MAX] = {0};
   BPatch_Vector< BPatch_module * > *appModules = NULL;
   BPatch_Vector< BPatch_function * > *appFunctions = NULL;
   BPatch_flowGraph *appCFG = NULL;

   dynHandle *dh = mutatorInit();
   if (!dh) return(-1);

   /**************************************************************
    * Parsing Phase
    */
   // Is this if redundant?  Will anybody run this without parsing the modules?
   if (config.parse_level >= PARSE_MODULE) {
      sendMsg(config.outfd, ID_PARSE_MODULE, INFO);
      appModules = dh->image->getModules();
      if (!appModules) {
         sendMsg(config.outfd, ID_PARSE_MODULE, INFO, ID_FAIL,
                 "Failure in BPatch_image::getModules()");
         return(-1);
      } else
         sendMsg(config.outfd, ID_PARSE_MODULE, INFO, ID_PASS,
                 appModules->size());
   }
   if (config.parse_level >= PARSE_FUNC) {
      for (i = 0; i < appModules->size(); ++i) {
         (*appModules)[i]->getName(buf, sizeof(buf));

         // Check module inclusion/exclusion regular expression rules.
         reason = config.mod_rules.getReason(buf);
         if (reason) sendMsg(config.outfd, ID_DATA_STRING, INFO, ID_INFO, reason);
         if (!config.mod_rules.isValid(buf)) continue;

         sendMsg(config.outfd, ID_PARSE_FUNC, INFO, ID_TEST, buf);
         appFunctions = (*appModules)[i]->getProcedures();
         if (!appFunctions) {
            sendMsg(config.outfd, ID_PARSE_FUNC, INFO, ID_FAIL,
                    "Failure in BPatch_module::getProcedures()");
            return(-1);
         } else
            sendMsg(config.outfd, ID_PARSE_FUNC, INFO, ID_PASS,
                    appFunctions->size());

         if (config.parse_level >= PARSE_CFG) {
            sendMsg(config.outfd, ID_PARSE_MODULE_CFG, INFO, ID_TEST,
                    (*appModules)[i]->getName(buf, sizeof(buf)));

            int cfg_warn_cnt = 0, cfg_pass_cnt = 0;
            for (j = 0; j < appFunctions->size(); ++j) {
               (*appFunctions)[j]->getName(buf, sizeof(buf));

               // Check function inclusion/exclusion regular expression rules.
               reason = config.func_rules.getReason(buf);
               if (reason) sendMsg(config.outfd, ID_DATA_STRING, INFO, ID_INFO, reason);
               if (!config.func_rules.isValid(buf)) continue;

               sendMsg(config.outfd, ID_PARSE_FUNC_CFG, VERB1, ID_TEST, buf);
               appCFG = (*appFunctions)[j]->getCFG();
               if (!appCFG) {
                  sendMsg(config.outfd, ID_PARSE_FUNC_CFG, VERB1, ID_WARN,
                          "Failure in BPatch_function::getCFG()");
                  ++cfg_warn_cnt;
               } else {
                  sendMsg(config.outfd, ID_PARSE_FUNC_CFG, VERB1, ID_PASS);
                  ++cfg_pass_cnt;
               }
            }
            if (cfg_warn_cnt)
               sendMsg(config.outfd, ID_PARSE_MODULE_CFG, INFO, ID_WARN,
                       sprintf_static("%d warning(s), %d passed.", cfg_warn_cnt, cfg_pass_cnt));
            else
               sendMsg(config.outfd, ID_PARSE_MODULE_CFG, INFO, ID_PASS);
         }
      }
   }

   /**************************************************************
    * Instrumentation Phase
    */

   /* Load the library, if specified */
   if (config.instType == USER_FUNC) {
       char instLibrary[1024];
       int offset = strcspn(config.inst_function, ":");
       strncpy (instLibrary, config.inst_function, offset);
       instLibrary[offset] = '\0';
       if(! dh->addSpace->loadLibrary(instLibrary)) {
          sendMsg(config.outfd, ID_INST_FIND_POINTS, VERB3, ID_FAIL,
                  "Failure in loading library");
          return false;
       }
   }

   if (config.trace_inst) 
   {
      errno = 0;
      sendMsg(config.outfd, ID_TRACE_INIT, INFO);
      sendMsg(config.outfd, ID_TRACE_OPEN_READER, VERB1);
	
      if (config.use_process)
      {
         config.pipefd = open(config.pipe_filename, O_RDONLY | O_RSYNC | O_NONBLOCK);
         if (config.pipefd < 0) {
            sendMsg(config.outfd, ID_TRACE_OPEN_READER, VERB1, ID_FAIL,
                    sprintf_static("Mutator could not open trace pipe (%s) for read: %s\n",
                                   config.pipe_filename, strerror(errno)));
            config.pipefd = -1;

         } else {
            sendMsg(config.outfd, ID_TRACE_OPEN_READER, VERB1, ID_PASS);
	      
            // Run mutatee side of trace initialization.
            sendMsg(config.outfd, ID_TRACE_INIT_MUTATEE, VERB1);
	      
            if (!initTraceInMutatee(dh)) {
               sendMsg(config.outfd, ID_TRACE_INIT_MUTATEE, VERB1, ID_FAIL);
               config.pipefd = -1;
            } else
               sendMsg(config.outfd, ID_TRACE_INIT_MUTATEE, VERB1, ID_PASS);
         }

         if (config.pipefd == -1) {
            sendMsg(config.outfd, ID_TRACE_INIT, INFO, ID_FAIL,
                    "Disabling instrumentation tracing.");
         } else
            sendMsg(config.outfd, ID_TRACE_INIT, INFO, ID_PASS);

         // We should remove the named pipe now, pass or fail.
         unlink(config.pipe_filename);
      }
   }

   if (config.inst_level >= INST_FUNC_ENTRY) {
      
      if (config.transMode == TRANS_PROCESS)
         if (!dynStartTransaction(dh)) return(-1);

      for (i = 0; i < appModules->size(); ++i) {
         int mod_warn_cnt = 0, mod_pass_cnt = 0;
         (*appModules)[i]->getName(buf, sizeof(buf));
         // Check module inclusion/exclusion regular expression rules.
         reason = config.mod_rules.getReason(buf);
         if (reason) 
            sendMsg(config.outfd, ID_DATA_STRING, INFO, ID_INFO, reason);
         if (!config.mod_rules.isValid(buf)) {
            continue;
         }

         if (config.transMode == TRANS_MODULE)
            if (!dynStartTransaction(dh)) {
               continue;
            }

         sendMsg(config.outfd, ID_INST_MODULE, INFO, ID_TEST, buf);

         sendMsg(config.outfd, ID_INST_GET_FUNCS, VERB1);
         appFunctions = (*appModules)[i]->getProcedures();
         if (!appFunctions) {
            sendMsg(config.outfd, ID_INST_GET_FUNCS, VERB1, ID_FAIL,
                    "Failure in BPatch_module::getProcedures()");
            return(-1);
         } else
            sendMsg(config.outfd, ID_INST_GET_FUNCS, VERB1, ID_PASS,
                    appFunctions->size());

         for (j = 0; j < appFunctions->size(); ++j) {
            int func_warn_cnt = 0, func_pass_cnt = 0;
            (*appFunctions)[j]->getName(buf, sizeof(buf));

            // Check function inclusion/exclusion regular expression rules.
            reason = config.func_rules.getReason(buf);
            if (reason) sendMsg(config.outfd, ID_DATA_STRING, INFO, ID_INFO, reason);
            if (!config.func_rules.isValid(buf)) continue;

            if (config.transMode == TRANS_FUNCTION)
               if (!dynStartTransaction(dh)) continue;

            sendMsg(config.outfd, ID_INST_FUNC, VERB1, ID_TEST, buf);

            if (config.inst_level == INST_FUNC_ENTRY) {
               if (!instrumentFunctionEntry(dh, (*appFunctions)[j]))
                  ++func_warn_cnt;
               else
                  ++func_pass_cnt;
            }

            if (config.inst_level == INST_FUNC_EXIT) {
               if (!instrumentFunctionExit(dh, (*appFunctions)[j]))
                  ++func_warn_cnt;
               else
                  ++func_pass_cnt;
            }

            if (config.inst_level == INST_BASIC_BLOCK) {
               if (!instrumentBasicBlocks(dh, (*appFunctions)[j]))
                  ++func_warn_cnt;
               else
                  ++func_pass_cnt;
            }

            if (config.inst_level == INST_MEMORY_READ) {
               if (!instrumentMemoryReads(dh, (*appFunctions)[j]))
                  ++func_warn_cnt;
               else
                  ++func_pass_cnt;
            }

            if (config.inst_level == INST_MEMORY_WRITE) {
               if (!instrumentMemoryWrites(dh, (*appFunctions)[j]))
                  ++func_warn_cnt;
               else
                  ++func_pass_cnt;
            }

            if (config.transMode == TRANS_FUNCTION)
               if (!dynEndTransaction(dh)) {
                  func_warn_cnt = func_pass_cnt;
                  func_pass_cnt = 0;
               }

            if (func_warn_cnt) {
               sendMsg(config.outfd, ID_INST_FUNC, VERB1, ID_WARN,
                       sprintf_static("%d warning(s), %d passed.", func_warn_cnt, func_pass_cnt));
               ++mod_warn_cnt;
            } else {
               sendMsg(config.outfd, ID_INST_FUNC, VERB1, ID_PASS);
               ++mod_pass_cnt;
            }
         }

         if (config.transMode == TRANS_MODULE)
            if (!dynEndTransaction(dh)) {
               mod_warn_cnt = mod_pass_cnt;
               mod_pass_cnt = 0;
            }

         if (mod_warn_cnt)
            sendMsg(config.outfd, ID_INST_MODULE, INFO, ID_WARN,
                    sprintf_static("%d warning(s), %d passed.", mod_warn_cnt, mod_pass_cnt));
         else
            sendMsg(config.outfd, ID_INST_MODULE, INFO, ID_PASS);
      }

      if (config.transMode == TRANS_PROCESS)
         dynEndTransaction(dh);
   }

   if (config.hunt_crashes) {
      sendMsg(config.outfd, ID_CRASH_HUNT_NUM_ACTIONS, INFO, ID_INFO, numInstsAllowed);
   }

   if (!config.use_process)
   {
      BPatch_binaryEdit *writeBE = dynamic_cast<BPatch_binaryEdit *>(dh->addSpace);

      // Load symbol libraries (if necessary)
      if( writeBE != NULL ) {
          deque<string>::iterator symLib_iter;
          for(symLib_iter = config.symbol_libraries.begin();
              symLib_iter != config.symbol_libraries.end();
              ++symLib_iter)
          {
              if( !writeBE->loadLibrary((*symLib_iter).c_str()) ) {
                  sendMsg(config.outfd, ID_INST_FIND_POINTS, VERB3, ID_FAIL,
                          "Unable to load symbol library");
                  return false;
              }
          }
      }

      writeBE->writeFile(config.writeFilePath);
   }

   if (config.use_process)
   {
      sendMsg(config.outfd, ID_RUN_CHILD, INFO);
      if (!dh->proc->continueExecution()) {
         sendMsg(config.outfd, ID_RUN_CHILD, INFO, ID_FAIL,
                 "Failure in BPatch_thread::continueExecution()");
         return false;
      }
      sendMsg(config.outfd, ID_RUN_CHILD, INFO, ID_PASS);
      
    
      //
      // Child continued.  Start reading from trace pipe, if enabled.
      //
      if (config.trace_inst) {
         while (config.pipefd != -1) {
            sendMsg(config.outfd, ID_POLL_STATUS_CHANGE, DEBUG);
            bool change = dh->bpatch->pollForStatusChange();
            sendMsg(config.outfd, ID_POLL_STATUS_CHANGE, DEBUG, ID_PASS);

            // Recheck conditional if a change was detected.
            if (change) continue;

            readTracePipe();

            // Eeeew.  I know.  But how else do you wait on a file descriptor,
            // AND BPatch::pollforStatusChange() == true at the same time?

            // We should probably have BPatch::registerStatusChangeCallback()
            // or something similar.
            sleep(1);
         }
      }
      

      //
      // All processing complete.  Loop indefinitly until exit handler called.
      //
      sendMsg(config.outfd, ID_WAIT_TERMINATION, INFO);
      while (1) {
         sendMsg(config.outfd, ID_CHECK_TERMINATION, INFO);
         bool dead = dh->proc->isTerminated();
         sendMsg(config.outfd, ID_CHECK_TERMINATION, INFO, ID_PASS);
         if (dead) break;

         sendMsg(config.outfd, ID_WAIT_STATUS_CHANGE, DEBUG);
         if (!dh->bpatch->waitForStatusChange())
            sendMsg(config.outfd, ID_WAIT_STATUS_CHANGE, DEBUG, ID_FAIL);
         else
            sendMsg(config.outfd, ID_WAIT_STATUS_CHANGE, DEBUG, ID_PASS);
      }
      sendMsg(config.outfd, ID_WAIT_TERMINATION, INFO, ID_PASS);
   }
   return 0;
}

//
// Structures and functions used with DyninstAPI callbacks.
//
struct summaryElem {
   BPatch_function *func;
   vector< BPatch_variableExpr * > count;
};
static vector< summaryElem * > summary;

void printSummary(BPatch_thread *proc, BPatch_exitType exit_type)
{
   unsigned int i, j;
   char buf[STRING_MAX];
   char *ptr;
   vector<int> value;

   // Mutatee has closed.  Drain the trace pipe.
   if (config.pipefd) readTracePipe();

   switch (exit_type) {
      case ExitedNormally:
         if (config.summary) {
            for (i = 0; i < summary.size(); ++i) {
               value.resize(summary[i]->count.size());

               bool used = false;
               for (j = 0; j < value.size(); ++j) {
                  summary[i]->count[j]->readValue(&value[j]);
                  if (value[j] > 0) used = true;
               }

               if (used) {
                  summary[i]->func->getModule()->getName(buf, sizeof(buf));
                  ptr = sprintf_static("[%s] ", buf);

                  summary[i]->func->getName(buf, sizeof(buf));
                  ptr = strcat_static(ptr, buf);

                  for (j = 0; j < value.size(); ++j)
                     ptr = strcat_static(ptr, sprintf_static(" %d", value[j]));

                  sendMsg(config.outfd, ID_DATA_STRING, INFO, ID_INFO, ptr);
               }
            }
         }
         sendMsg(config.outfd, ID_EXIT_CODE, INFO, ID_INFO, proc->getProcess()->getExitCode());
         break;

      case ExitedViaSignal:
          sendMsg(config.outfd, ID_EXIT_SIGNAL, INFO, ID_INFO, proc->getProcess()->getExitSignal());
         if (config.hunt_crashes) {
            sendMsg(config.outfd, ID_CRASH_HUNT_NUM_ACTIONS, INFO, ID_INFO, numInstsAllowed);
         }
         exit(-1);  // Nothing left to do.  The mutatee is gone.
         break;

      case NoExit:
         sendMsg(config.outfd, ID_DATA_STRING, INFO, ID_INFO,
                 "Conflicting reports about mutatee status.");
         break;

      default:
         sendMsg(config.outfd, ID_DATA_STRING, INFO, ID_INFO,
                 "Monitor error.  Possibly linked with wrong version of DyninstAPI.\n");
   }
   return;
}

void reportNewProcess(BPatch_thread* /*parent*/, BPatch_thread *child)
{
    sendMsg(config.outfd, ID_POST_FORK, INFO, ID_INFO, child->getProcess()->getPid());
}

//
// Helper mutator functions.
//
bool insertSummary(BPatch_function *func, BPatch_variableExpr *expr)
{
   sendMsg(config.outfd, ID_SUMMARY_INSERT, DEBUG);

   summaryElem *elem;
   vector< summaryElem * >::iterator iter = summary.begin();

   while (iter != summary.end()) {
      elem = *iter;
      if (elem->func == func) break;
      ++iter;
   }

   if (iter == summary.end()) {
      elem = new summaryElem;
      if (!elem) {
         sendMsg(config.outfd, ID_SUMMARY_INSERT, DEBUG, ID_FAIL,
                 "Memory allocation of new summary element failed");
         return false;
      }
      elem->func = func;
      summary.push_back(elem);
   }

   elem->count.push_back(expr);
   sendMsg(config.outfd, ID_SUMMARY_INSERT, DEBUG, ID_PASS);

   return true;
}

BPatch_variableExpr *allocateIntegerInMutatee(dynHandle *dh, int defaultValue = 0)
{
   BPatch_variableExpr *countVar;

   sendMsg(config.outfd, ID_ALLOC_COUNTER, VERB3);

   sendMsg(config.outfd, ID_INST_FIND_INT, VERB4);
   BPatch_type *intType = dh->image->findType("int");
   if (!intType) {
      sendMsg(config.outfd, ID_INST_FIND_INT, VERB4, ID_FAIL,
              "BPatch_image::findType(\"int\")");
      goto fail;
   } else
      sendMsg(config.outfd, ID_INST_FIND_INT, VERB4, ID_PASS);

   sendMsg(config.outfd, ID_INST_MALLOC_INT, VERB4);

   countVar = dh->addSpace->malloc(*intType);
   if (!countVar) {
      sendMsg(config.outfd, ID_INST_MALLOC_INT, VERB4, ID_FAIL,
              "Failure in BPatch_process::malloc()");
      goto fail;
      
   } else if (!countVar->writeValue(&defaultValue)) {
      sendMsg(config.outfd, ID_INST_MALLOC_INT, VERB4, ID_FAIL,
              "Failure initializing counter in mutatee [BPatch_variableExpr::writeValue()]");
      goto fail;
      
   } else {
      sendMsg(config.outfd, ID_INST_MALLOC_INT, VERB4, ID_PASS);
   }
    
   sendMsg(config.outfd, ID_ALLOC_COUNTER, VERB3, ID_PASS);
   return(countVar);

 fail:
   sendMsg(config.outfd, ID_ALLOC_COUNTER, VERB3, ID_FAIL);
   return(NULL);
}

//
// Instrumentation code.
//

bool initTraceInMutatee(dynHandle *dh)
{
   // Chicken: Can't use goto for error handling because it would branch past
   // variable initialization.

   // Egg: Can't define variables at top of function (with dummy constructors)
   // because BPatch_funcCallExpr() needs a valid BPatch_function object.

   // Temporary solution: Deviate from similar functions and rely on external
   // error handling.

   // Permanent solution: BPatch_snippet and derivitives should have default
   // constructors.

   int value;
   BPatch_function *openFunc;
   BPatch_Vector< BPatch_function * > funcs;

   sendMsg(config.outfd, ID_TRACE_FIND_OPEN, VERB2, ID_TEST);
   if (!dh->image->findFunction("^(__)?open(64)?$", funcs)) {
      sendMsg(config.outfd, ID_TRACE_FIND_OPEN, VERB2, ID_FAIL,
              "Failure in BPatch_image::findFunction()");
      return false;

   } else if (funcs.size() == 0) {
      sendMsg(config.outfd, ID_TRACE_FIND_OPEN, VERB2, ID_FAIL,
              "Could not find any functions named 'open' in mutatee");
      return false;
   }
   sendMsg(config.outfd, ID_TRACE_FIND_OPEN, VERB2, ID_PASS);
   openFunc = funcs[0];

   //
   // Initialize global variables
   //
   trace_fd = allocateIntegerInMutatee(dh, -1);
   funcs.clear();
   sendMsg(config.outfd, ID_TRACE_FIND_WRITE, VERB2);
   if (!dh->image->findFunction("^(__)?write$", funcs)) {
      sendMsg(config.outfd, ID_TRACE_FIND_WRITE, VERB2, ID_FAIL,
              "Failure in BPatch_image::findFunction()");
      return false;

   } else if (funcs.size() == 0) {
      sendMsg(config.outfd, ID_TRACE_FIND_WRITE, VERB2, ID_FAIL,
              "Could not find any functions named 'write' in mutatee");
      return false;
   }
   sendMsg(config.outfd, ID_TRACE_FIND_WRITE, VERB2, ID_PASS);
   trace_write = funcs[0];

   // "open(config.pipe_filename, O_WRONLY | O_DSYNC)"
   BPatch_constExpr path(config.pipe_filename);
   BPatch_constExpr flags(O_WRONLY | O_DSYNC);

   BPatch_Vector< BPatch_snippet * > param;
   param.push_back( &path );
   param.push_back( &flags );
   BPatch_funcCallExpr openCall(*openFunc, param); // Problem child. See above.

   // "fd = open(config.pipe_filename, O_WRONLY)"
   BPatch_arithExpr assign(BPatch_assign, *trace_fd, openCall);

   // Run the snippet.
   sendMsg(config.outfd, ID_TRACE_OPEN_WRITER, VERB2);
   dh->proc->oneTimeCode(assign);
   trace_fd->readValue(&value);
   if (value < 0) {
      sendMsg(config.outfd, ID_TRACE_OPEN_WRITER, VERB2, ID_FAIL,
              "Error detected in mutatee's call to open()");
      return false;
   }
   sendMsg(config.outfd, ID_TRACE_OPEN_WRITER, VERB2, ID_PASS);

   return true;
}

bool insertTraceSnippet(dynHandle *dh, BPatch_function *func, BPatch_Vector<BPatch_point *> *points)
{
   char *buf;
   BPatch_point *point;
   BPatch_callWhen when;
   BPatch_snippetOrder order;

   int warn_cnt = 0, pass_cnt = 0;

   sendMsg(config.outfd, ID_TRACE_INSERT, VERB3);

   for (unsigned int i = 0; i < points->size(); ++i) {
      if (!shouldInsert())
         continue;
      sendMsg(config.outfd, ID_TRACE_INSERT_ONE, VERB4);

      point = (*points)[i];
      buf = sprintf_static("%*p", trace_msglen, (void *)point);

      BPatch_constExpr data(buf);
      BPatch_constExpr len(trace_msglen);

      BPatch_Vector< BPatch_snippet * > param;
      param.push_back( trace_fd );
      param.push_back( &data );
      param.push_back( &len );

      // write(trace_fd, buf, trace_msglen);
      BPatch_funcCallExpr writeCall(*trace_write, param);

      // if (trace_fd > 0)
      BPatch_boolExpr checkFd(BPatch_gt, *trace_fd, BPatch_constExpr( 0 ));

      BPatch_ifExpr traceSnippet(checkFd, writeCall);

      switch (point->getPointType()) {
         case BPatch_entry:
            when = BPatch_callBefore;
            order = BPatch_firstSnippet;
            break;

         case BPatch_exit:
            when = BPatch_callAfter;
            order = BPatch_lastSnippet;
            break;

         default:
            sendMsg(config.outfd, ID_TRACE_INSERT_ONE, VERB4, ID_FAIL,
                    "Internal error.  Attempting to trace non entry/exit point.");
            ++warn_cnt;
            continue;
      }

      BPatchSnippetHandle *handle = dh->addSpace->insertSnippet(traceSnippet, *point, when, order);
      if (!handle) {
         sendMsg(config.outfd, ID_TRACE_INSERT_ONE, VERB4, ID_FAIL,
                 "Error detected in BPatch_process::insertSnippet().");
         ++warn_cnt;
         continue;
      }
      sendMsg(config.outfd, ID_TRACE_INSERT_ONE, VERB4, ID_PASS);
      ++pass_cnt;
      trace_points[(void *)point] = func;
   }

   if (warn_cnt) {
      sendMsg(config.outfd, ID_TRACE_INSERT, VERB3, ID_WARN,
              sprintf_static("%d warning(s), %d passed.", warn_cnt, pass_cnt));
   } else {
      sendMsg(config.outfd, ID_TRACE_INSERT, VERB3, ID_PASS);
   }
   return true;
}

void readTracePipe()
{
   int read_len;
   char buf[ STRING_MAX ] = { '\0' };

   if (config.pipefd < 0) return;

   do {
      errno = 0;
      sendMsg(config.outfd, ID_TRACE_READ, DEBUG);
      read_len = read(config.pipefd, buf, trace_msglen);
      buf[trace_msglen] = '\0';

      if (read_len < trace_msglen) {
         if (read_len == -1 && errno == EAGAIN) {
            // No data on pipe.  Break out of read loop
            // and re-poll for status change.
            sendMsg(config.outfd, ID_TRACE_READ, DEBUG, ID_PASS);
            break;

         } else if (read_len == 0 && errno == 0) {
            // Read EOF from pipefd.  Close pipe and break.
            sendMsg(config.outfd, ID_TRACE_READ, DEBUG, ID_PASS);
            close(config.pipefd);
            config.pipefd = -1;
            break;

         } else if (read_len > 0) {
            // Partial data written to trace pipe.  Report to monitor.
            sendMsg(config.outfd, ID_TRACE_READ, DEBUG, ID_FAIL,
                    sprintf_static("Read partial message from trace pipe.  Discarding message '%s'.", buf));
            break;

         } else if (errno) {
            // Send error message to monitor.
            sendMsg(config.outfd, ID_TRACE_READ, DEBUG, ID_FAIL,
                    sprintf_static("Mutator encountered error on trace pipe read(): %s", strerror(errno)));
            close(config.pipefd);
            config.pipefd = -1;
            break;
         }
      }

      void *traceMsg = (void *)strtol(buf, NULL, 16);
      map< void *, BPatch_function * >::iterator iter = trace_points.find(traceMsg);
      if (iter == trace_points.end()) {
         sendMsg(config.outfd, ID_TRACE_READ, DEBUG, ID_FAIL,
                 sprintf_static("Read invalid message from trace pipe.  0x%s does not refer to a valid BPatch_point.", buf));
         break;
      }
      sendMsg(config.outfd, ID_TRACE_READ, DEBUG, ID_PASS);

      BPatch_point *point = (BPatch_point *)traceMsg;

      const char *pType = "Unknown ";
      if (point->getPointType() == BPatch_entry) pType = "Entering ";
      if (point->getPointType() == BPatch_exit)  pType = "Exiting ";

      const char *pName = "anonymous function";
      BPatch_function *pFunc = (*iter).second;
      if (pFunc) {
         if (pFunc->getName(buf, sizeof(buf)))
            pName = sprintf_static("function %s", buf);
         else
            pName = sprintf_static("anonymous function at 0x%*p", trace_msglen, pFunc->getBaseAddr());
      }

      if (config.pipefd > 0) {
         // Could have been interrupted by mutatee exit.
	sendMsg(config.outfd, ID_TRACE_POINT, INFO, ID_INFO, strcat_static(pType, pName));
      }
   } while (errno == 0);
}

void closeTracePipe()
{
   int negOne = -1;

   // Should we acutally create snippets to call close()?
   if (trace_fd) trace_fd->writeValue(&negOne);
}

bool generateInstrumentation(dynHandle *dh, BPatch_function *func, BPatch_snippet* incSnippet) {
   if (config.instType == USER_FUNC) {
      // Instrument user's function call
      
      // config.inst_function is of the format library:function_name
      const char *instFunction;

      instFunction = strchr(config.inst_function, ':');
      if (!instFunction) {
         sendMsg(config.outfd, ID_INST_FIND_POINTS, VERB3, ID_FAIL,
                 "Failure in loading instrumentation function");
         return false;
      }
      instFunction++;

      BPatch_Vector<BPatch_function *> funcs;
      if( NULL ==  dh->image->findFunction(instFunction, funcs, false, true, true) || !funcs.size() || NULL == funcs[0]){
         sendMsg(config.outfd, ID_INST_FIND_POINTS, VERB3, ID_FAIL,
                 "Unable to find function");
         return false;;
      }
      BPatch_function *instCallFunc = funcs[0];
      BPatch_Vector<BPatch_snippet *> instCallArgs;
      /* How to push function's argument */
      BPatch_funcCallExpr instCallExpr(*instCallFunc, instCallArgs);
      *incSnippet = instCallExpr;

   } else {

      BPatch_variableExpr *countVar = allocateIntegerInMutatee(dh);
      if (!countVar) return false;
      if (!insertSummary(func, countVar)) return false;

      // Should we test for errors on this?
      BPatch_arithExpr instArithExpr( BPatch_assign, *countVar,
                                      BPatch_arithExpr(BPatch_plus, *countVar,
                                                       BPatch_constExpr(1)));
      *incSnippet = instArithExpr; 
   }
	
   return true;
}

bool instrumentFunctionEntry(dynHandle *dh, BPatch_function *func)
{
   BPatch_Vector<BPatch_point *> *points;
   BPatchSnippetHandle *handle;
   BPatch_snippet incSnippet; 

   sendMsg(config.outfd, ID_INST_FUNC_ENTRY, VERB2);

   if (! generateInstrumentation(dh, func, &incSnippet) )
    	goto fail;

   sendMsg(config.outfd, ID_INST_FIND_POINTS, VERB3);
   points = func->findPoint(BPatch_entry);
   if (!points) {
      sendMsg(config.outfd, ID_INST_FIND_POINTS, VERB3, ID_FAIL,
              "Failure in BPatch_function::findPoint(BPatch_entry)");
      goto fail;

   } else if (points->size() == 0) {
      sendMsg(config.outfd, ID_INST_FIND_POINTS, VERB3, ID_WARN,
              "No instrumentation points found in function");
      goto fail;

   } else {
      sendMsg(config.outfd, ID_INST_FIND_POINTS, VERB3, ID_PASS);
   }

   if (shouldInsert())
   {
      sendMsg(config.outfd, ID_INST_INSERT_CODE, VERB3);
      handle = dh->addSpace->insertSnippet(incSnippet, *points);
      if (!handle) {
         sendMsg(config.outfd, ID_INST_INSERT_CODE, VERB3, ID_FAIL,
                 "Failure in BPatch_process::insertSnippet()");
         goto fail;
      } else
         sendMsg(config.outfd, ID_INST_INSERT_CODE, VERB3, ID_PASS);
   }

   if (trace_fd) insertTraceSnippet(dh, func, points);

   sendMsg(config.outfd, ID_INST_FUNC_ENTRY, VERB2, ID_PASS);
   return true;

 fail:
   sendMsg(config.outfd, ID_INST_FUNC_ENTRY, VERB2, ID_WARN,
           "Failure while instrumenting function entry.");
   return false;
}

bool instrumentFunctionExit(dynHandle *dh, BPatch_function *func)
{
   BPatch_Vector<BPatch_point *> *points;
   BPatchSnippetHandle *handle;
   BPatch_snippet incSnippet; 

   sendMsg(config.outfd, ID_INST_FUNC_EXIT, VERB2);

   if (! generateInstrumentation (dh, func, &incSnippet))
    	goto fail;

   sendMsg(config.outfd, ID_INST_FIND_POINTS, VERB3);
   points = func->findPoint(BPatch_exit);
   if (!points) {
      sendMsg(config.outfd, ID_INST_FIND_POINTS, VERB3, ID_FAIL,
              "Failure in BPatch_function::findPoint(BPatch_exit)");
      goto fail;

   } else if (points->size() == 0) {
      sendMsg(config.outfd, ID_INST_FIND_POINTS, VERB3, ID_WARN,
              "No instrumentation points found in function");
      goto fail;

   } else {
      sendMsg(config.outfd, ID_INST_FIND_POINTS, VERB3, ID_PASS);
   }

   sendMsg(config.outfd, ID_INST_INSERT_CODE, VERB3);
   if (shouldInsert()) {
      handle = dh->addSpace->insertSnippet(incSnippet, *points);
      if (!handle) {
         sendMsg(config.outfd, ID_INST_INSERT_CODE, VERB3, ID_FAIL,
                 "Failure in BPatch_process::insertSnippet()");
         goto fail;

      } else
         sendMsg(config.outfd, ID_INST_INSERT_CODE, VERB3, ID_PASS);
   }

   if (trace_fd) insertTraceSnippet(dh, func, points);

   sendMsg(config.outfd, ID_INST_FUNC_EXIT, VERB2, ID_PASS);
   return true;

 fail:
   sendMsg(config.outfd, ID_INST_FUNC_EXIT, VERB2, ID_WARN,
           "Failure while instrumenting function exit.");
   return false;
}

bool instrumentBasicBlocks(dynHandle *dh, BPatch_function *func)
{
   BPatch_Set<BPatch_basicBlock*> allBlocks;
   BPatch_snippet incSnippet; 
   BPatch_Set<BPatch_basicBlock*>::iterator iter;
   int bb_warn_cnt = 0, bb_pass_cnt = 0;

   sendMsg(config.outfd, ID_INST_BASIC_BLOCK, VERB2);

   sendMsg(config.outfd, ID_GET_CFG, VERB3);
   BPatch_flowGraph *appCFG = func->getCFG();
   if (!appCFG) {
      sendMsg(config.outfd, ID_GET_CFG, VERB3, ID_FAIL,
              "Failure in BPatch_function::getCFG()");
      goto fail;

   } else {
      sendMsg(config.outfd, ID_GET_CFG, VERB3, ID_PASS);
   }

   sendMsg(config.outfd, ID_INST_GET_BB, VERB3);
   if (!appCFG->getAllBasicBlocks(allBlocks)) {
      sendMsg(config.outfd, ID_INST_GET_BB, VERB3, ID_FAIL,
              "Failure in BPatch_flowGraph::getAllBasicBlocks()");
      goto fail;

   } else if (allBlocks.size() == 0) {
      sendMsg(config.outfd, ID_INST_GET_BB, VERB3, ID_WARN,
              "No basic blocks found in function");
      goto fail;

   } else {
      sendMsg(config.outfd, ID_INST_GET_BB, VERB3, ID_PASS);
   }

   if (! generateInstrumentation (dh, func, &incSnippet))
    	goto fail;


   sendMsg(config.outfd, ID_INST_BB_LIST, VERB3);
   for (iter = allBlocks.begin(); iter != allBlocks.end(); iter++) {
      if (!shouldInsert())
         continue;

      sendMsg(config.outfd, ID_INST_GET_BB_POINTS, VERB4);
      BPatch_point *entry = (*iter)->findEntryPoint();
      if (!entry) {
         sendMsg(config.outfd, ID_INST_GET_BB_POINTS, VERB4, ID_WARN,
                 "Failure in BPatch_basicBlock::findEntryPoint()");
         ++bb_warn_cnt;
         continue;

      } else {
         sendMsg(config.outfd, ID_INST_GET_BB_POINTS, VERB4, ID_PASS);
      }

      sendMsg(config.outfd, ID_INST_INSERT_CODE, VERB4);
      BPatchSnippetHandle *handle = dh->addSpace->insertSnippet(incSnippet, *entry);
      if (!handle) {
         sendMsg(config.outfd, ID_INST_INSERT_CODE, VERB4, ID_FAIL,
                 "Failure in BPatch_process::insertSnippet()");
         ++bb_warn_cnt;
         continue;

      } else {
         sendMsg(config.outfd, ID_INST_INSERT_CODE, VERB4, ID_PASS);
         ++bb_pass_cnt;
      }
   }
   if (bb_warn_cnt)
      sendMsg(config.outfd, ID_INST_BB_LIST, VERB3, ID_WARN,
              sprintf_static("%d warning(s), %d passed.", bb_warn_cnt, bb_pass_cnt));
   else
      sendMsg(config.outfd, ID_INST_BB_LIST, VERB3, ID_PASS);

   sendMsg(config.outfd, ID_INST_BASIC_BLOCK, VERB2, ID_PASS);
   return true;

 fail:
   sendMsg(config.outfd, ID_INST_BASIC_BLOCK, VERB2, ID_WARN,
           "Failure while instrumenting basic blocks.");
   return false;
}

bool instrumentMemoryReads(dynHandle *dh, BPatch_function *func)
{
   BPatch_Set<BPatch_basicBlock*> allBlocks;
   BPatch_snippet incSnippet; 
   BPatch_Set<BPatch_opCode> ops;
   BPatch_Set<BPatch_basicBlock*>::iterator iter;
   int bb_warn_cnt = 0, bb_pass_cnt = 0;

   sendMsg(config.outfd, ID_INST_MEM_READ, VERB2);

   sendMsg(config.outfd, ID_GET_CFG, VERB3);
   BPatch_flowGraph *appCFG = func->getCFG();
   if (!appCFG) {
      sendMsg(config.outfd, ID_GET_CFG, VERB3, ID_FAIL,
              "Failure in BPatch_function::getCFG()");
      goto fail;

   } else {
      sendMsg(config.outfd, ID_GET_CFG, VERB3, ID_PASS);
   }

   sendMsg(config.outfd, ID_INST_GET_BB, VERB3);
   if (!appCFG->getAllBasicBlocks(allBlocks)) {
      sendMsg(config.outfd, ID_INST_GET_BB, VERB3, ID_FAIL,
              "Failure in BPatch_flowGraph::getAllBasicBlocks()");
      goto fail;

   } else if (allBlocks.size() == 0) {
      sendMsg(config.outfd, ID_INST_GET_BB, VERB3, ID_WARN,
              "No basic blocks found in function");
      goto fail;

   } else {
      sendMsg(config.outfd, ID_INST_GET_BB, VERB3, ID_PASS);
   }

   if (! generateInstrumentation (dh, func, &incSnippet))
    	goto fail;

   ops.insert(BPatch_opLoad);

   sendMsg(config.outfd, ID_INST_BB_LIST, VERB3);
   for (iter = allBlocks.begin(); iter != allBlocks.end(); iter++) {
      if (!shouldInsert())
         continue;
      sendMsg(config.outfd, ID_INST_GET_BB_POINTS, VERB4);
      BPatch_Vector<BPatch_point*> *points = (*iter)->findPoint(ops);
      if (!points) {
         sendMsg(config.outfd, ID_INST_GET_BB_POINTS, VERB4, ID_WARN,
                 "Failure in BPatch_basicBlock::findPoint()");
         ++bb_warn_cnt;
         continue;

      } else if (points->size() == 0) {
         sendMsg(config.outfd,  ID_INST_GET_BB_POINTS, VERB4, ID_WARN,
                 "No instrumentation points found in basic block");
         ++bb_warn_cnt;
         continue;

      } else {
         sendMsg(config.outfd, ID_INST_GET_BB_POINTS, VERB4, ID_PASS);
      }

      sendMsg(config.outfd, ID_INST_INSERT_CODE, VERB4);
      BPatchSnippetHandle *handle = dh->addSpace->insertSnippet(incSnippet, *points);
      if (!handle) {
         sendMsg(config.outfd, ID_INST_INSERT_CODE, VERB4, ID_FAIL,
                 "Failure in BPatch_process::insertSnippet()");
         ++bb_warn_cnt;
         continue;

      } else {
         sendMsg(config.outfd, ID_INST_INSERT_CODE, VERB4, ID_PASS);
         ++bb_pass_cnt;
      }
   }
   if (bb_warn_cnt)
      sendMsg(config.outfd, ID_INST_BB_LIST, VERB3, ID_WARN,
              sprintf_static("%d warning(s), %d passed.", bb_warn_cnt, bb_pass_cnt));
   else
      sendMsg(config.outfd, ID_INST_BB_LIST, VERB3, ID_PASS);

   sendMsg(config.outfd, ID_INST_MEM_READ, VERB2, ID_PASS);
   return true;

 fail:
   sendMsg(config.outfd, ID_INST_MEM_READ, VERB2, ID_WARN,
           "Failure while instrumenting memory reads.");
   return false;
}

bool instrumentMemoryWrites(dynHandle *dh, BPatch_function *func)
{
   BPatch_Set<BPatch_basicBlock*> allBlocks;
   BPatch_snippet incSnippet; 
   BPatch_Set<BPatch_opCode> ops;
   BPatch_Set<BPatch_basicBlock*>::iterator iter;
   int bb_warn_cnt = 0, bb_pass_cnt = 0;

   sendMsg(config.outfd, ID_INST_MEM_WRITE, VERB2);

   sendMsg(config.outfd, ID_GET_CFG, VERB3);
   BPatch_flowGraph *appCFG = func->getCFG();
   if (!appCFG) {
      sendMsg(config.outfd, ID_GET_CFG, VERB3, ID_FAIL,
              "Failure in BPatch_function::getCFG()");
      goto fail;

   } else {
      sendMsg(config.outfd, ID_GET_CFG, VERB3, ID_PASS);
   }

   sendMsg(config.outfd, ID_INST_GET_BB, VERB3);
   if (!appCFG->getAllBasicBlocks(allBlocks)) {
      sendMsg(config.outfd, ID_INST_GET_BB, VERB3, ID_FAIL,
              "Failure in BPatch_flowGraph::getAllBasicBlocks()");
      goto fail;

   } else if (allBlocks.size() == 0) {
      sendMsg(config.outfd, ID_INST_GET_BB, VERB3, ID_WARN,
              "No basic blocks found in function");
      goto fail;

   } else {
      sendMsg(config.outfd, ID_INST_GET_BB, VERB3, ID_PASS);
   }

   if (! generateInstrumentation (dh, func, &incSnippet))
    	goto fail;

   ops.insert(BPatch_opStore);

   sendMsg(config.outfd, ID_INST_BB_LIST, VERB3);
   for (iter = allBlocks.begin(); iter != allBlocks.end(); iter++) {
      if (!shouldInsert())
         continue;
      sendMsg(config.outfd, ID_INST_GET_BB_POINTS, VERB4);
      BPatch_Vector<BPatch_point*> *points = (*iter)->findPoint(ops);
      if (!points) {
         sendMsg(config.outfd, ID_INST_GET_BB_POINTS, VERB4, ID_WARN,
                 "Failure in BPatch_basicBlock::findPoint()");
         ++bb_warn_cnt;
         continue;

      } else if (points->size() == 0) {
         sendMsg(config.outfd,  ID_INST_GET_BB_POINTS, VERB4, ID_WARN,
                 "No instrumentation points found in basic block");
         ++bb_warn_cnt;
         continue;

      } else {
         sendMsg(config.outfd, ID_INST_GET_BB_POINTS, VERB4, ID_PASS);
      }

      sendMsg(config.outfd, ID_INST_INSERT_CODE, VERB4);
      BPatchSnippetHandle *handle = dh->addSpace->insertSnippet(incSnippet, *points);
      if (!handle) {
         sendMsg(config.outfd, ID_INST_INSERT_CODE, VERB4, ID_FAIL,
                 "Failure in BPatch_process::insertSnippet()");
         ++bb_warn_cnt;
         continue;

      } else {
         sendMsg(config.outfd, ID_INST_INSERT_CODE, VERB4, ID_PASS);
         ++bb_pass_cnt;
      }
   }
   if (bb_warn_cnt)
      sendMsg(config.outfd, ID_INST_BB_LIST, VERB3, ID_WARN,
              sprintf_static("%d warning(s), %d passed.", bb_warn_cnt, bb_pass_cnt));
   else
      sendMsg(config.outfd, ID_INST_BB_LIST, VERB3, ID_PASS);

   sendMsg(config.outfd, ID_INST_MEM_WRITE, VERB2, ID_PASS);
   return true;

 fail:
   sendMsg(config.outfd, ID_INST_MEM_WRITE, VERB2, ID_WARN,
           "Failure while instrumenting memory writes.");
   return false;

}
