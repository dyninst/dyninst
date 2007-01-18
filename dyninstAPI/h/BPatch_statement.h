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

#ifndef _BPATCH_STATEMENT_H_
#define _BPATCH_STATEMENT_H_

#include "BPatch_dll.h"
#include "BPatch_eventLock.h"

class BPatch_module;

#ifdef DYNINST_CLASS_NAME
#undef DYNINST_CLASS_NAME
#endif
#define DYNINST_CLASS_NAME BPatch_statement

class BPATCH_DLL_EXPORT BPatch_statement : public BPatch_eventLock
{
  friend class BPatch_module;
  friend class BPatch_image;

  public:
    //  BPatch_module * getModule()
    //  Return the BPatch_module that contains this statement
    API_EXPORT(Int, (),
    BPatch_module *,module,()); 
    
    //  int getLineNumber()
    //  return the line number of this statement
    API_EXPORT(Int, (),
    int,lineNumber,());

    //  int getLineOffset()
    //  return the line offset of this statement (its start column in the source file)
    //  This may not be supported on all platforms.
    //  Returns -1 if not supported.
    API_EXPORT(Int, (),
    int,lineOffset,());

    //  const char * fileName()
    //  return the name of the file that contains this statement
    API_EXPORT(Int, (),
    const char *,fileName,());

    //  void * startAddr()
    //  return the starting address of this statement
    API_EXPORT(Int, (),
    void *,startAddr,());

    //  void * endAddr()
    //  return the last address associated with this statement
    //  (do we guarantee contiguity of addresses here?  not sure)
    API_EXPORT(Int, (),
    void *,endAddr,());

    ~BPatch_statement() {}

  private:

    //  Full parameter ctor -- can only built by friend classes
    BPatch_statement(BPatch_module *mod, const char *filename, 
                     int lineno, int lineoff = -1,
                     void *startAddr_ = NULL, void *endAddr = NULL); 

    //  same deal -- fields only set by friend classes (not the dyninst user, duh)
    void setModule(BPatch_module *mod) {module_ = mod;}
    void setLineNo(int lineno) {lineno_ = lineno;}
    void setLineoff(int lineoff) {lineoff_ = lineoff;}
    void setFileName(const char *filename) {file_ = filename;}
    void setStartAddr(void *addr) {start_addr = addr;}
    void setEndAddr(void *addr) {end_addr = addr;}

    BPatch_module *module_;
    int lineno_;
    int lineoff_;
    const char *file_;
    void *start_addr;
    void *end_addr;
};

#endif 

