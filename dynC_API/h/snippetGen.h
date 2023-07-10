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
#ifndef SNIPPET_GEN_H
#define SNIPPET_GEN_H

#include "BPatch_snippet.h"
#include "BPatch_addressSpace.h"
#include "BPatch_point.h"
#include "BPatch_function.h"
#include "BPatch_module.h"
#include "BPatch.h"
#include <sstream>
#include <string>
#include <vector>



class SnippetGenerator{
  public:

   enum SGErrorType{
      SG_LookUpFailure,
      SG_TypeError,
      SG_ScopeViolation,
      SG_SyntaxError,
      SG_InternalError
   };

   struct SGError{
      SGErrorType type;
      bool fatal;
   };   

   enum SGContext{
      SG_FunctionName,
      SG_ModuleName,
      SG_TID
   };

  private:
   std::stringstream lastError;
   SGError lastErrorInfo{};
   BPatch_point *point;
   BPatch_addressSpace *addSpace;
   BPatch_image *image;
   
   std::vector<BPatch_register> registers;
  
  public:
   std::string getError() {return lastError.str();}
   SGError getErrorInfo() {return lastErrorInfo;}
  
  public:

  SnippetGenerator() :  point(NULL), addSpace(NULL), image(NULL) {}
  SnippetGenerator(BPatch_point &pt) : point(&pt)
   { 
      addSpace = point->getAddressSpace();
      image = addSpace->getImage();
   }
   
  SnippetGenerator(BPatch_addressSpace &aSpace) : addSpace(&aSpace)
   { 
      point = NULL;
      image = addSpace->getImage();
   }

   ~SnippetGenerator(){}

   BPatch_snippet *findOrCreateVariable(const char * name, const char * type, const void * initialValue = NULL);
   BPatch_snippet *findOrCreateArray(const char * name, const char * elementType, long size);
   BPatch_snippet *findRegister(const char *name);
   BPatch_snippet *findAppVariable(const char * name, bool global = false, bool local = false);
   BPatch_snippet *findParameter(const char * name);
   BPatch_snippet *findParameter(int index);
   BPatch_snippet *findInstVariable(const char *mangledStub, const char * name);
   BPatch_snippet *generateArrayRef(BPatch_snippet *base, BPatch_snippet *index);
   BPatch_function *findFunction(const char * name, std::vector<BPatch_snippet *> params);
   BPatch_snippet *getContextInfo(SGContext context);

   BPatch_point *getPoint() {return point;}
};

#endif
