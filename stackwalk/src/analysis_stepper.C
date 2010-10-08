/*
 * Copyright (c) 1996-2009 Barton P. Miller
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "stackwalk/src/analysis_stepper.h"
#include "symtabAPI/h/Symtab.h"
#include "symEval/h/stackanalysis.h"
#include "stackwalk/h/swk_errors.h"
#include "stackwalk/h/frame.h"

#include "parseAPI/h/CodeSource.h"
#include "parseAPI/h/CodeObject.h"

using namespace Dyninst;
using namespace Stackwalker;
using namespace ParseAPI;
using namespace SymtabAPI;
using namespace std;

std::map<string, CodeObject *> AnalysisStepperImpl::objs;
const AnalysisStepperImpl::height_pair_t AnalysisStepperImpl::err_height_pair;

AnalysisStepperImpl::AnalysisStepperImpl(Walker *w, AnalysisStepper *p) :
   FrameStepper(w),
   parent(p)
{
}

AnalysisStepperImpl::~AnalysisStepperImpl()
{
}

CodeSource *AnalysisStepperImpl::getCodeSource(std::string name)
{
   Symtab *symtab = NULL;
   bool result = Symtab::openFile(symtab, name);
   if (!result) {
      sw_printf("[%s:%u] - SymtabAPI failed to open file %s\n", __FILE__, __LINE__, 
                name.c_str());
      return NULL;
   }
   SymtabCodeSource *cs = new SymtabCodeSource(symtab);
   return static_cast<CodeSource *>(cs);
}

CodeObject *AnalysisStepperImpl::getCodeObject(string name)
{
   map<string, CodeObject *>::iterator i = objs.find(name);
   if (i != objs.end()) {
      return i->second;
   }
   
   CodeSource *code_source = getCodeSource(name);
   if (!code_source)
      return NULL;
   CodeObject *code_object = new CodeObject(code_source);
   objs[name] = code_object;

   code_object->parse();
   return code_object;
}

AnalysisStepperImpl::height_pair_t AnalysisStepperImpl::analyzeFunction(string name,
                                                                        Offset off)
{
   CodeObject *obj = getCodeObject(name);
   if (!obj) {
      return err_height_pair;
   }

   set<CodeRegion *> regions;
   obj->cs()->findRegions(off, regions);
   
   if (regions.empty()) {
      sw_printf("[%s:%u] - Could not find region at %lx\n", __FILE__, __LINE__, off);
      return err_height_pair;
   }
   //We shouldn't be dealing with overlapping regions in a live process
   assert(regions.size() == 1);
   CodeRegion *region = *(regions.begin());
   
   set<ParseAPI::Function*> funcs;
   obj->findFuncs(region, off, funcs);
   if (funcs.empty()) {
      sw_printf("[%s:%u] - Could not find function at offset %lx\n", __FILE__,
                __LINE__, off);
      return err_height_pair;
   }

   set<height_pair_t> heights;
   for (set<ParseAPI::Function *>::iterator i = funcs.begin(); i != funcs.end(); i++)
   {
      StackAnalysis analysis(*i);
      heights.insert(height_pair_t(analysis.findSP(off), analysis.findFP(off)));
   }

   sw_printf("[%s:%u] - Have %lu possible stack heights in %s at %lx:\n", __FILE__, __LINE__, heights.size(), name.c_str(), off);
   for (set<height_pair_t>::iterator i = heights.begin(); 
        i != heights.end(); i++)
   {
      sw_printf("\tsp = %s, fp = %s\n", i->first.format().c_str(), i->second.format().c_str());
   }

   //Return the first pair found, until we work out something more sensible
   return *(heights.begin());
}

gcframe_ret_t AnalysisStepperImpl::getCallerFrame(const Frame &in, Frame &out)
{
   LibAddrPair libaddr;
   LibraryState *ls = getProcessState()->getLibraryTracker();
   if (!ls) {
      sw_printf("[%s:%u] - Failed to get library tracker\n", __FILE__, __LINE__);
      return gcf_not_me;
   }

   bool result = ls->getLibraryAtAddr(in.getRA(), libaddr);
   if (!result) {
      sw_printf("[%s:%u] - Failed to get library at %lx\n", in.getRA());
      return gcf_not_me;
   }
   
   string name = libaddr.first;
   Offset offset = in.getRA() - libaddr.second;

   height_pair_t height_pair = analyzeFunction(name, offset);
   if (height_pair == err_height_pair) {
      sw_printf("Analysis failed on %s at %lx\n", name.c_str(), offset);
      return gcf_not_me;
   }

   return getCallerFrameArch(height_pair, in, out);
}

unsigned AnalysisStepperImpl::getPriority() const
{
   return analysis_priority;
}


