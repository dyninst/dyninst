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


#include <stdlib.h>
#include <iostream.h>
#include <ctype.h>
#include "paradynd/src/focus.h"
#include "paradynd/src/resource.h"


machineHierarchy::machineHierarchy(const pdvector<string> &setupInfo) {
   unsigned setupInfo_size = setupInfo.size();
   assert(setupInfo_size <= 3);
   switch(setupInfo_size) {
      // the breaks for these lines have intentionally been left out
     case 3: thread  = setupInfo[2];
     case 2: process = setupInfo[1];
     case 1: machine = setupInfo[0];
     case 0: break;  // none are set
   }
}

string machineHierarchy::getName() const {
   string name = "/Machine";
   if(machine_defined()) name += ("/" + machine);
   if(process_defined()) name += ("/" + process);
   if(thread_defined())  name += ("/" + thread);
   return name;
}

int machineHierarchy::getPid() const {
  int pid = -1;
  if(process.length() > 2) { // of format executable{pid}
    const char *beg = process.c_str();
    const char *lparen;
    for(lparen = beg; *lparen!='{' && *lparen!=0; lparen++) ;
    char tempBuf[20];
    char *ts = tempBuf;
    for(const char *p = lparen + 1; isdigit(*p); p++)
      *ts = *p;
    pid = atoi(tempBuf);
  }
  return pid;
}

void machineHierarchy::setPid(int pid) {
  if(process.length() > 2) { // of format executable{pid}
    const char *beg = process.c_str();
    char execName[200];  // th
    char *ts = execName;
    for(const char *p = beg; *p!='{' && *p!=0; p++, ts++)
      *ts = *p;
    *ts = 0;
    string new_process = string(execName) + "{" + string(pid) + "}";
    process = new_process;
  }
}

int machineHierarchy::getThreadID() const {
   int thr_id = -1;

   if(thread.length() > 4) {  // need at least "thr_#"
      const char *begNum = thread.c_str();
      begNum += 4;  // bypass "thr_"
      char tempBuf[50];
      char *ts = tempBuf;
      for(const char *fs = begNum; isdigit(*fs); fs++, ts++) 
	 *ts = *fs;
      *ts = 0;
      thr_id = atoi(tempBuf);
   }
   return thr_id;
}

bool machineHierarchy::focus_matches(const pdvector<string> &/*match_path*/) 
   const
{
   return false;
}

pdvector<string> machineHierarchy::tokenized() const {
   pdvector<string> retVec;
   retVec.push_back(string("Machine"));
   if(machine_defined()) retVec.push_back(string(machine));
   if(process_defined()) retVec.push_back(string(process));
   if(thread_defined())  retVec.push_back(string(thread));
   return retVec;
}

codeHierarchy::codeHierarchy(const pdvector<string> &setupInfo) {
   unsigned setupInfo_size = setupInfo.size();
   assert(setupInfo_size <= 2);
   switch(setupInfo_size) {
      // the breaks for these lines have intentionally been left out
     case 2: function = setupInfo[1];
     case 1: module   = setupInfo[0];
     case 0: break;  // none are set
   }
}

string codeHierarchy::getName() const {
   string name = "/Code";
   if(module_defined())   name += ("/" + module);
   if(function_defined()) name += ("/" + function);
   return name;
}

pdvector<string> codeHierarchy::tokenized() const {
   pdvector<string> retVec;
   retVec.push_back(string("Code"));
   if(module_defined())   retVec.push_back(string(module));
   if(function_defined()) retVec.push_back(string(function));
   return retVec;
}

// match_path is a vector of any of the following combinations of input (as
// of 5/6/02).  It is an attribute of a constraint, and when a focus is
// chosen, the match_path allows certain constraints to be grepped out
// (ie. chosen).

// match_path is one of:
// ( "Code" )       - used to select modules
// ( "Code", "*" )  - used to select functions
// ( "SyncObject" ) - used to select a type of synchronization
// ( "SyncObject", "Message" ) - seems to be used with message group constr.s
// ( "SyncObject", "Message", "*" ) - used with message tag constraints
bool codeHierarchy::focus_matches(const pdvector<string> &match_path) const
{
   unsigned mp_size = match_path.size();
   bool ret = false;
   //cerr << "        CODE-heir: " << getName() << "\n";
   //cerr << "        mp_size: " << mp_size << ", contents: ";
   //for(unsigned i=0; i<mp_size; i++) {
   //   cerr << " " << match_path[i] << ",";
   //}
   //cerr << "\n";
   // Currently, only handle constraints on /Code or /SyncObject
   assert(match_path[0] == "Code" || match_path[0] == "SyncObject");

   if(mp_size == 1) {
      if(match_path[0] == "Code") {
	 if(module_defined() && !function_defined())  
	    ret = true;
	 else
	    ret = false;
      } 
   }
   else if(mp_size == 2) {  
      if(match_path[0] == "Code") {
	 if(match_path[1] == "*") {
	    if(function_defined())  ret = true;
	    else    ret = false;
	 } else { 
	    ret = (match_path[1] == get_function());
	 }
      } 
   } else {
     ret = false;  // can't handle match_path of this sort
   }
   //cerr << "        returning " << ret << "\n";
   return ret;
}

syncObjHierarchy::syncObjHierarchy(const pdvector<string> &setupInfo) {
   unsigned setupInfo_size = setupInfo.size();
   
   if(setupInfo_size == 0) {
      syncObjType = NoSyncObjT;
      return;
   }

   if(setupInfo[0] == "Barrier") {
      syncObjType = BarrierT;
      if(setupInfo_size >= 2)
	 barrierData.barrierStr = setupInfo[1];
      assert(setupInfo_size <= 2);  //changes needed if triggered
   } else if(setupInfo[0] == "Message") {
      syncObjType = MessageT;
      if(setupInfo_size >= 2)
	 messageData.communicator = setupInfo[1];

      if(setupInfo_size >= 3) {
	 // I think the [2] element is a tag
	 messageData.tag     = setupInfo[2];
      }
      assert(setupInfo_size <= 3);  //changes needed if triggered
   } else if(setupInfo[0] == "Semaphore") {
      syncObjType = SemaphoreT;
      if(setupInfo_size >= 2)
	 semaphoreData.semaphoreStr = setupInfo[1];
      assert(setupInfo_size <= 2);  //changes needed if triggered
   } else if(setupInfo[0] == "SpinLock") {
      syncObjType = SpinlockT;
      if(setupInfo_size >= 2)
	 spinlockData.spinlockStr = setupInfo[1];
      assert(setupInfo_size <= 2);  //changes needed if triggered
   } else {
      syncObjType = NoSyncObjT;
   }
}

string syncObjHierarchy::getName() const {
   string name = "/SyncObject";
   switch(syncObjType) {
     case NoSyncObjT:
	break;
     case BarrierT:
	name += "/Barrier";
	if(barrierData.barrierStr.length())
	   name += ("/" + barrierData.barrierStr);
	break;
     case MessageT:
	name += "/Message";
	if(messageData.communicator.length())
	   name += ("/" + messageData.communicator);
	if(messageData.tag.length())
	   name += ("/" + messageData.tag);
	break;
     case SemaphoreT:
	name += "/Semaphore";
	if(semaphoreData.semaphoreStr.length())
	   name += ("/" + semaphoreData.semaphoreStr);
	break;
     case SpinlockT:
	name += "/SpinLock";
	if(spinlockData.spinlockStr.length())
	   name += ("/" + spinlockData.spinlockStr);
	break;
   }
   return name;
}

// see description above codeHierarchy::focus_matches
bool syncObjHierarchy::focus_matches(const pdvector<string> &match_path) const
{
   unsigned mp_size = match_path.size();
   bool ret = false;
   //cerr << "        SYNC-heir: " << getName() << "\n";
   //cerr << "        mp_size: " << mp_size << ", contents: ";
   //for(unsigned i=0; i<mp_size; i++) {
   //   cerr << " " << match_path[i] << ",";
   //}
   //cerr << "\n";
   // Currently, only handle constraints on /Code or /SyncObject
   assert(match_path[0] == "Code" || match_path[0] == "SyncObject");

   if(mp_size == 1) {
      if(match_path[0] == "SyncObject") {
	 if(allMessages() || allBarriers() || 
	    allSemaphores() || allSpinlocks())   ret = true;
	 else   ret = false;
      }
   }
   else if(mp_size == 2) {  
      if(match_path[0] == "SyncObject") {
	 assert(match_path[1] == "Message");
	 if(communicator_defined() && !tag_defined())  ret = true;
	 else ret = false;
      }
   }
   else if(mp_size == 3) {
      assert(match_path[0] == "SyncObject");
      assert(match_path[1] == "Message");
      if(match_path[2] == "*") {
	 if(tag_defined()) ret = true;
	 else ret = false;
      } else {
	 ret = (get_tag() == match_path[2]);
      }
   }
   //cerr << "        returning " << ret << "\n";
   return ret;
}

// temporary
pdvector<string> syncObjHierarchy::tokenized() const {
   pdvector<string> retVec;
   retVec.push_back(string("SyncObject"));
   switch(syncObjType) {
     case NoSyncObjT:
	break;
     case BarrierT:
	retVec.push_back(string("Barrier"));
	if(barrierData.barrierStr.length())
	   retVec.push_back(barrierData.barrierStr);
	break;
     case MessageT:
	retVec.push_back(string("Message"));
	if(messageData.communicator.length())
	   retVec.push_back(messageData.communicator);
	if(messageData.tag.length())
	   retVec.push_back(messageData.tag);
	break;
     case SemaphoreT:
	retVec.push_back(string("Semaphore"));
	if(semaphoreData.semaphoreStr.length())
	   retVec.push_back(semaphoreData.semaphoreStr);
	break;
     case SpinlockT:
	retVec.push_back(string("SpinLock"));
	if(spinlockData.spinlockStr.length())
	   retVec.push_back(spinlockData.spinlockStr);
	break;
   }
   return retVec;
}

memoryHierarchy::memoryHierarchy(const pdvector<string> & /* setupInfo */) {
}

string memoryHierarchy::getName() const {
   string name = "/Memory";
   return name;
}

bool memoryHierarchy::focus_matches(const pdvector<string> &/*match_path*/) 
   const 
{
   return false;
}

pdvector<string> memoryHierarchy::tokenized() const {
   pdvector<string> retVec;
   retVec.push_back(string("Memory"));
   return retVec;
}


Focus::Focus(const pdvector<u_int>& ids, bool *errorFlag) : 
   machineInfo(NULL), codeInfo(NULL), syncObjInfo(NULL),
   memoryInfo(NULL)
{
   pdvector< pdvector<string> > focusVec;
   *errorFlag = false;
  
   for (unsigned i=0; i<ids.size(); i++) {
      resource *r = resource::findResource(ids[i]);
      if(r == NULL) {
	 (*errorFlag) = true;
	 return;
      }
      focusVec += r->names();
   }

   // assign to the object
   for (unsigned j=0; j<focusVec.size(); j++) {
      pdvector<string> &curFocusVec = focusVec[j];
      assert(curFocusVec.size() > 0);  // there must be a heirarchy name
      if(curFocusVec[0] == "Machine") {
	 curFocusVec.erase(0, 0);
	 machineInfo = new machineHierarchy(curFocusVec);
      } else if(curFocusVec[0] == "Code") {      
	 curFocusVec.erase(0, 0);
	 codeInfo = new codeHierarchy(curFocusVec);	 
      } else if(curFocusVec[0] == "SyncObject") {
	 curFocusVec.erase(0, 0);
	 syncObjInfo = new syncObjHierarchy(curFocusVec);
      } else if(curFocusVec[1] == "Memory") {
	 curFocusVec.erase(0, 0);
	 memoryInfo = new memoryHierarchy(curFocusVec);
      } else {
	 assert(false);  // add a new heirarchy type if this is triggered
      }
   }

   // "Memory tag currently isn't actually ever passed in.
   if(memoryInfo==NULL) {
      memoryInfo = new memoryHierarchy();
   }
}

Focus::Focus(const Focus &f) {
   machineInfo = new machineHierarchy(*(f.machineInfo));
   codeInfo    = new    codeHierarchy(*(f.codeInfo));
   syncObjInfo = new syncObjHierarchy(*(f.syncObjInfo));
   memoryInfo  = new  memoryHierarchy(*(f.memoryInfo));
}

Focus::~Focus() {
   delete machineInfo;
   delete codeInfo;
   delete syncObjInfo;
   delete memoryInfo;
}

string Focus::getName() const {
   string name = "";
   name += machineInfo->getName() + ",";
   name += codeInfo->getName() + ",";   
   name += syncObjInfo->getName() + ",";   
   name += memoryInfo->getName();   
   return name;
}

pdvector< pdvector<string> > Focus::tokenized() const {
   pdvector< pdvector<string> > ret;
   ret.push_back(machineInfo->tokenized());
   ret.push_back(codeInfo->tokenized());
   ret.push_back(syncObjInfo->tokenized());
   ret.push_back(memoryInfo->tokenized());
   return ret;
}

ostream& operator<<(ostream &s, const Focus &f) {
  return s << f.getName();
}

Focus adjustFocusForPid(const Focus &foc, int pid) {
  Focus tempFocus = foc;
  tempFocus.setPid(pid);
  return tempFocus;  // object copy
}

