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

#ifndef _BPatch_sourceObj_h_
#define _BPatch_sourceObj_h_

#include <BPatch_Vector.h>

class BPatch_type;
class BPatch_variableExpr;

typedef enum BPatch_language {
    BPatch_c, 
    BPatch_cPlusPlus, 
    BPatch_fortran, 
    BPatch_fortran77, 
    BPatch_fortran90, 
    BPatch_assembly, 
    BPatch_mixed, 
    BPatch_hpf, 
    BPatch_java, 
    BPatch_unknownLanguage 
} BPatch_language;

typedef enum BPatch_sourceType {
    BPatch_sourceUnknown_type,
    BPatch_sourceProgram,
    BPatch_sourceModule,
    BPatch_sourceFunction,
    BPatch_sourceOuterLoop,
    BPatch_sourceLoop,
    BPatch_srcBlock,
    BPatch_sourceStatement
} BPatch_sourceType;

class BPatch_sourceObj {
  public:
      BPatch_sourceType getSrcType() { return _srcType; }
      virtual BPatch_Vector<BPatch_sourceObj *> *getSourceObj() = 0;
      virtual BPatch_sourceObj *getObjParent() = 0;

      BPatch_Vector<BPatch_variableExpr *> *findVariable(const char *name);
      BPatch_language getLanguage();
      BPatch_type *getType(char *name);
      BPatch_Vector<BPatch_variableExpr *> *getVariables();
      BPatch_Vector<char *> *getLoadedFileNames();
      char *programName(char *buf, unsigned int len);
      int programNameLen();

  protected:
      enum BPatch_sourceType _srcType;
};

#endif
