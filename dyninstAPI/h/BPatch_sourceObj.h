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

#ifndef _BPatch_sourceObj_h_
#define _BPatch_sourceObj_h_

#include "BPatch_dll.h"
#include <BPatch_Vector.h>
#ifdef IBM_BPATCH_COMPAT
#include "BPatch_point.h"
#endif

class BPatch_type;
class BPatch_variableExpr;

typedef enum BPatch_language {
    BPatch_c, 
    BPatch_cPlusPlus, 
    BPatch_fortran, 
    BPatch_fortran77, 
    BPatch_fortran90, 
    BPatch_f90_demangled_stabstr, 
    BPatch_assembly, 
    BPatch_mixed, 
    BPatch_hpf, 
    BPatch_java, 
    BPatch_unknownLanguage 
} BPatch_language;

typedef enum BPatch_sourceType {
#ifdef IBM_BPATCH_COMPAT
    BPatch_sourceUnknown,
#else
    BPatch_sourceUnknown_type,
#endif
    BPatch_sourceProgram,
    BPatch_sourceModule,
    BPatch_sourceFunction,
    BPatch_sourceOuterLoop,
    BPatch_sourceLoop,
#ifdef IBM_BPATCH_COMPAT
    BPatch_sourceTypeBlock,
#else
    BPatch_srcBlock,
#endif
    BPatch_sourceStatement
} BPatch_sourceType;

class BPATCH_DLL_EXPORT BPatch_sourceObj {
  public:
      BPatch_sourceType getSrcType() { return _srcType; }
      virtual bool getSourceObj(BPatch_Vector<BPatch_sourceObj *> &) = 0;
      virtual bool getVariables(BPatch_Vector<BPatch_variableExpr *> &) = 0;
      virtual BPatch_sourceObj *getObjParent() = 0;

      BPatch_Vector<BPatch_variableExpr *> *findVariable(const char *name);
      BPatch_language getLanguage() { return _srcLanguage; }
      const char *getLanguageStr() {return strLanguage(_srcLanguage);}
      BPatch_type *getType(char *name);
      BPatch_Vector<char *> *getLoadedFileNames();
      //char *getName(char *buf, unsigned int len);
      int getNameLen();

#ifdef IBM_BPATCH_COMPAT
 virtual bool getAddressRange(void*& _startAddress, void*& _endAddress) {return false;}
 virtual bool getLineNumbers(unsigned int _startLine, unsigned int  _endLine) {return false;}
 virtual void getIncPoints(BPatch_Vector<BPatch_point *> &vect) {return;}
 virtual void getExcPoints(BPatch_Vector<BPatch_point *> &vect) {return;}
#endif
  protected:
      enum BPatch_sourceType _srcType;
      BPatch_language _srcLanguage;
      void setLanguage(BPatch_language lang) { _srcLanguage = lang; }

    const char *strLanguage(BPatch_language l) {
	switch(l) {
	case BPatch_c: return "BPatch_c";
	case BPatch_cPlusPlus: return "BPatch_cPlusPlus";
	case BPatch_fortran: return "BPatch_fortran";
	case BPatch_fortran77: return "BPatch_fortran77";
	case BPatch_fortran90: return "BPatch_fortran90";
	case BPatch_f90_demangled_stabstr: return "BPatch_fortran90_demangled_stabstr";
	case BPatch_assembly: return "BPatch_assembly";
	case BPatch_mixed: return "BPatch_mixed";
	case BPatch_hpf: return "BPatch_hpf";
	case BPatch_java: return "BPatch_java";
	case BPatch_unknownLanguage: return "BPatch_unknownLanguage";
	default: return "strLanguage:  bad conversion -- FIXME";
	}
	return "strLanguage:  bad conversion -- FIXME";
      }

};

#endif
