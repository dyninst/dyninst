/*
 * Copyright (c) 1996-2007 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

// $Id: Object.C,v 1.31 2008/11/03 15:19:25 jaw Exp $

#include "Annotatable.h"
#include "common/h/serialize.h"

#include "Symtab.h"
#include "symutil.h"
#include "Module.h"
#include "Collections.h"
#include "Function.h"

#include "symtabAPI/src/Object.h"

#include "annotations.h"

#include <iostream>

using namespace std;
using namespace Dyninst;
using namespace Dyninst::SymtabAPI;


Function *Function::createFunction(Symbol *sym) {
    Function *func = new Function();
    func->addSymbol(sym);
    return func;
}

Function::Function()
    : retType_(NULL), framePtrRegNum_(-1), locs_(NULL) {}

Type * Function::getReturnType() const
{
    return retType_;
}

bool Function::setReturnType(Type *newType)
{
    retType_ = newType;
    return true;
}

int Function::getFramePtrRegnum() const
{
    return framePtrRegNum_;
}

bool Function::setFramePtrRegnum(int regnum)
{
    framePtrRegNum_ = regnum;
    return true;
}

std::vector<Dyninst::SymtabAPI::loc_t *> *Function::getFramePtr() const 
{
    return locs_;
}

bool Function::setFramePtr(vector<loc_t *> *locs) 
{
    if(locs_) 
        return false;
    
    locs_ = locs;
    return true;
}


bool Function::findLocalVariable(std::vector<localVar *> &vars, std::string name)
{
   module_->exec()->parseTypesNow();	

   localVarCollection *lvs = NULL, *lps = NULL;
   bool res1 = false, res2 = false;
   res1 = getAnnotation(lvs, FunctionLocalVariablesAnno);
   res2 = getAnnotation(lps, FunctionParametersAnno);

   if (!res1 && !res2)
      return false;

   unsigned origSize = vars.size();	

   if (lvs)
   {
      localVar *var = lvs->findLocalVar(name);
      if (var) 
         vars.push_back(var);
   }

   if (lps)
   {
      localVar *var = lps->findLocalVar(name);
      if (var) 
         vars.push_back(var);
   }

   if (vars.size() > origSize)
      return true;

   return false;
}

bool Function::getLocalVariables(std::vector<localVar *> &vars)
{
   module_->exec()->parseTypesNow();	

   localVarCollection *lvs = NULL;
   if (!getAnnotation(lvs, FunctionLocalVariablesAnno))
   {
      return false;
   }
   if (!lvs)
   {
      fprintf(stderr, "%s[%d]:  FIXME:  NULL ptr for annotation\n", FILE__, __LINE__);
      return false;
   }

   vars = *(lvs->getAllVars());

   if (vars.size())
      return true;
   return false;
}

bool Function::getParams(std::vector<localVar *> &params)
{
   module_->exec()->parseTypesNow();	

   localVarCollection *lvs = NULL;
   if (!getAnnotation(lvs, FunctionParametersAnno))
   {
      return false;
   }

   if (!lvs)
   {
      fprintf(stderr, "%s[%d]:  FIXME:  NULL ptr for annotation\n", FILE__, __LINE__);
      return false;
   }

   params = *(lvs->getAllVars());

   if (params.size())
      return true;
   return false;
}

bool Function::addLocalVar(localVar *locVar)
{
   localVarCollection *lvs = NULL;

   if (!getAnnotation(lvs, FunctionLocalVariablesAnno))
   {
      lvs = new localVarCollection();

      if (!addAnnotation(lvs, FunctionLocalVariablesAnno))
      {
         fprintf(stderr, "%s[%d]:  failed to add local var collecton anno\n", 
               FILE__, __LINE__);
         return false;
      }
   }

   lvs->addLocalVar(locVar);
   return true;
}

bool Function::addParam(localVar *param)
{
   localVarCollection *ps = NULL;

   if (!getAnnotation(ps, FunctionParametersAnno))
   {
      ps = new localVarCollection();

      if (!addAnnotation(ps, FunctionParametersAnno))
      {
         fprintf(stderr, "%s[%d]:  failed to add local var collecton anno\n", 
               FILE__, __LINE__);
         return false;
      }
   }

   ps->addLocalVar(param);

   return true;
}
