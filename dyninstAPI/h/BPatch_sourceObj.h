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

#ifndef _BPatch_sourceObj_h_
#define _BPatch_sourceObj_h_

#include "BPatch_dll.h"
#include "BPatch_Vector.h"

class BPatch_type;
class BPatch_variableExpr;
class BPatch_point;

typedef enum BPatch_language {
    BPatch_c, 
    BPatch_cPlusPlus, 
    BPatch_fortran, 
    BPatch_fortran77, 
    BPatch_fortran90,
    BPatch_fortran95,
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


class BPATCH_DLL_EXPORT BPatch_sourceObj {
 public:
    virtual ~BPatch_sourceObj() { }

      BPatch_sourceType getSrcType() { return _srcType; }
      virtual bool getSourceObj(BPatch_Vector<BPatch_sourceObj *> &) = 0;
      virtual bool getVariables(BPatch_Vector<BPatch_variableExpr *> &) = 0;
      virtual BPatch_sourceObj *getObjParent() = 0;

      //BPatch_Vector<BPatch_variableExpr *> *findVariable(const char *name);
      BPatch_language getLanguage() { return _srcLanguage; }
      const char *getLanguageStr() {return strLanguage(_srcLanguage);}
      //BPatch_type *getType(char *name);
      //BPatch_Vector<char *> *getLoadedFileNames();
      //char *getName(char *buf, unsigned int len);
      //int getNameLen();

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
        case BPatch_fortran95: return "BPatch_fortran95";
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
