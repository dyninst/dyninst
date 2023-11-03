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

#include "stackwalk/src/analysis_stepper.h"
#include "dataflowAPI/h/stackanalysis.h"
#include "stackwalk/h/swk_errors.h"
#include "stackwalk/h/frame.h"
#include "stackwalk/src/sw.h"

#include "parseAPI/h/CodeSource.h"
#include "parseAPI/h/SymLiteCodeSource.h"
#include "parseAPI/h/CodeObject.h"

#include "instructionAPI/h/InstructionDecoder.h"

#if defined(WITH_SYMLITE)
#include "symlite/h/SymLite-elf.h"
#elif defined(WITH_SYMTAB_API)
#include "symtabAPI/h/Symtab.h"
#include "symtabAPI/h/SymtabReader.h"
using namespace Dyninst::SymtabAPI;
#else
#error "No defined symbol reader"
#endif

using namespace Dyninst;
using namespace Stackwalker;
using namespace ParseAPI;
using namespace std;

std::map<string, CodeObject *> AnalysisStepperImpl::objs;
const AnalysisStepperImpl::height_pair_t AnalysisStepperImpl::err_height_pair;
std::map<string, CodeSource*> AnalysisStepperImpl::srcs;
std::map<string, SymReader*> AnalysisStepperImpl::readers;



AnalysisStepperImpl::AnalysisStepperImpl(Walker *w, AnalysisStepper *p) :
   FrameStepper(w),
   parent(p)
{
    callchecker = new CallChecker(getProcessState());
}

AnalysisStepperImpl::~AnalysisStepperImpl()
{
    delete callchecker;
}


#if defined(WITH_SYMLITE)
CodeSource *AnalysisStepperImpl::getCodeSource(std::string name)
{
  map<string, CodeSource*>::iterator found = srcs.find(name);
  if(found != srcs.end()) return found->second;
  
  static SymElfFactory factory;
  
  SymReader* r = factory.openSymbolReader(name);
  if(!r) return NULL;
  
  
  SymReaderCodeSource *cs = new SymReaderCodeSource(r);
  srcs[name] = cs;
  readers[name] = r;
  
  return static_cast<CodeSource *>(cs);
}
#elif defined(WITH_SYMTAB_API)
CodeSource* AnalysisStepperImpl::getCodeSource(std::string name)
{
  map<string, CodeSource*>::iterator found = srcs.find(name);
  if(found != srcs.end()) return found->second;
  Symtab* st;
  if(!Symtab::openFile(st, name)) return NULL;
  
  SymtabCodeSource *cs = new SymtabCodeSource(st);
  srcs[name] = cs;
  readers[name] = new SymtabReader(st);
  
  return static_cast<CodeSource *>(cs);  
}
#else
#error "Do symbol reader implementation"

#endif

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

   return code_object;
}

gcframe_ret_t AnalysisStepperImpl::getCallerFrameArch(set<height_pair_t> heights,
        const Frame &in, Frame &out)
{
    ProcessState *proc = getProcessState();
    
    bool result = false;

    set<height_pair_t>::iterator heightIter;
    for (heightIter = heights.begin(); heightIter != heights.end(); ++heightIter) {

        height_pair_t height = *heightIter;

        Address in_sp = in.getSP(),
                in_fp = in.getFP(),
                out_sp = 0,
                out_ra = 0,
                out_fp = 0,
                out_fp_addr = 0;
        StackAnalysis::Height sp_height = height.first;
        StackAnalysis::Height fp_height = height.second;
        location_t out_ra_loc, out_fp_loc;

        if (sp_height == StackAnalysis::Height::bottom) {
            sw_printf("[%s:%d] - Analysis didn't find a stack height\n", 
                    FILE__, __LINE__);
            continue;
        } else {
            
            // SP height is the distance from the last SP of the previous frame
            // to the SP in this frame at the current offset.
            // Since we are walking to the previous frame,
            // we subtract this height to get the outgoing SP
            out_sp = in_sp - sp_height.height();
        }

        // Since we know the outgoing SP,
        // the outgoing RA must be located just below it
	if(!getOutRA(out_sp, out_ra, out_ra_loc, proc)) continue;


        // If we have multiple potential heights (due to overlapping functions), 
        // check if potential stack height is valid (verify that calculated RA follows a call instr)
        if (heights.size() > 1) {
	  if(!validateRA(out_ra)) continue;
        }
        if (fp_height == StackAnalysis::Height::bottom)
	{
	  if(sp_height.height() == -1 * (long)(proc->getAddressWidth()))
	  {
	    //out.setFP(out_sp);
	  }
	}
	else
	{
            // FP height is the distance from the last SP of the previous frame
            // to the FP in this frame at the current offset.
            // If analysis finds this height,
            // then out SP + FP height should equal in FP.
            // We then assume that in FP points to out FP.
            out_fp_addr = out_sp + fp_height.height();

	}
	
	if(out_fp_addr)
	{
	  if (out_fp_addr != in_fp) {
	    sw_printf(
		      "[%s:%d] - Warning - current FP %lx does not point to next FP located at %lx\n",
		      FILE__, __LINE__, in_fp, out_fp_addr);
	  }
	  bool resultMem = proc->readMem(&out_fp, out_fp_addr, proc->getAddressWidth());
	  if (resultMem) {
	    out_fp_loc.location = loc_address;
	    out_fp_loc.val.addr = out_fp_addr;
	    
	    out.setFPLocation(out_fp_loc);
	    out.setFP(out_fp);
	  }
	  else {
	    sw_printf("[%s:%d] - Failed to read FP value\n", FILE__, __LINE__);
	  }
	}
	else
	{
	  sw_printf("[%s:%d] - Failed to find FP\n", FILE__, __LINE__);
	}
	
	
	out.setSP(out_sp);
	out.setRALocation(out_ra_loc);
	out.setRA(out_ra);

        if (result) {
            sw_printf("[%s:%d] - Warning - found multiple valid frames.\n", 
                    FILE__, __LINE__);
        } else {
            sw_printf("[%s:%d] - Found a valid frame\n", 
                    FILE__, __LINE__);
            result = true;
        }
    }
    return checkResult(result);
}

CodeRegion* AnalysisStepperImpl::getCodeRegion(std::string name, Offset off)
{
   CodeObject *obj = getCodeObject(name);
   if (!obj) {
      return NULL;
   }
   set<CodeRegion *> regions;
   obj->cs()->findRegions(off, regions);
   
   if (regions.empty()) {
      return NULL;
   }
   //We shouldn't be dealing with overlapping regions in a live process
   assert(regions.size() == 1);
   CodeRegion *region = *(regions.begin());
   return region;
}




std::set<AnalysisStepperImpl::height_pair_t> AnalysisStepperImpl::analyzeFunction(string name,
                                                                                  Offset callSite)
{
    set<height_pair_t> err_heights_pair;
    err_heights_pair.insert(err_height_pair);
    CodeRegion* region = getCodeRegion(name, callSite);
    CodeObject* obj = getCodeObject(name);
    
    if(!obj || !region) return err_heights_pair;
    
    Symbol_t sym = readers[name]->getContainingSymbol(callSite);
    if (!readers[name]->isValidSymbol(sym)) {
       sw_printf("[%s:%d] - Could not find symbol at offset %lx\n", FILE__,
                 __LINE__, callSite);
       return err_heights_pair;
    }
    Address entry_addr = readers[name]->getSymbolOffset(sym);
    
    
    obj->parse(entry_addr, false);
    ParseAPI::Function* func = obj->findFuncByEntry(region, entry_addr);

    if(!func)
    {
      sw_printf("[%s:%d] - Could not find function at offset %lx\n", FILE__,
                __LINE__, callSite);
      return err_heights_pair;
    }

   //Since there is only one region, there is only one block with the offset
    // Not actually true; overlapping code is always possible.
   set<ParseAPI::Block*> blocks;
   for(auto i = func->blocks().begin();
       i != func->blocks().end();
       ++i)
   {
     if((*i)->start() <= callSite && (*i)->end() > callSite)
     {
       blocks.insert(*i);
     }
   }
   //obj->findBlocks(region, callSite, blocks);
   if(blocks.size() == 0) {
      sw_printf("[%s:%d] - Function at entry point %lx did not contain call site %lx\n", FILE__,
                __LINE__, entry_addr, callSite);
     return err_heights_pair;
   }
   
   ParseAPI::Block *block = *(blocks.begin());

   set<height_pair_t> heights;
   StackAnalysis analysis(func);
   heights.insert(height_pair_t(analysis.findSP(block, callSite), analysis.findFP(block, callSite)));
 
   sw_printf("[%s:%d] - Have %lu possible stack heights in %s at %lx:\n", FILE__, __LINE__, heights.size(), name.c_str(), callSite);
   for (set<height_pair_t>::iterator i = heights.begin(); 
        i != heights.end(); i++)
   {
      sw_printf("\tsp = %s, fp = %s\n", i->first.format().c_str(), i->second.format().c_str());
   }

   // Return set of possible heights
   return heights;
}

gcframe_ret_t AnalysisStepperImpl::getCallerFrame(const Frame &in, Frame &out)
{
   // For now, do not walk frames created by the Dyninst stepper
   // as the stack pointer may not be correct
   if (dynamic_cast<DyninstDynamicStepper*>(in.getStepper()))
   {
     return gcf_not_me;
   }

   LibAddrPair libaddr;
   LibraryState *ls = getProcessState()->getLibraryTracker();
   if (!ls) {
      sw_printf("[%s:%d] - Failed to get library tracker\n", FILE__, __LINE__);
      return gcf_not_me;
   }

   bool result = ls->getLibraryAtAddr(in.getRA(), libaddr);
   if (!result) {
      sw_printf("[%s:%d] - Failed to get library at %lx\n", FILE__, __LINE__, in.getRA());
      return gcf_not_me;
   }
   
   string name = libaddr.first;
   Offset offset = in.getRA() - libaddr.second;
   Offset function_offset = offset;
   if (in.getRALocation().location != loc_register && !in.nonCall()) {
      /* Look up by callsite, rather than return address */
      function_offset = function_offset - 1;
   }

   set<height_pair_t> heights = analyzeFunction(name, function_offset);
   gcframe_ret_t ret = gcf_not_me;
   if (*(heights.begin()) == err_height_pair) {
     sw_printf("[%s:%d] - Analysis failed on %s at %lx\n", FILE__, __LINE__, name.c_str(), offset);
     return ret;
   }

   ret = getCallerFrameArch(heights, in, out);
   
   if((ret == gcf_not_me) && in.isTopFrame())
   {
     vector<registerState_t> all_defined_heights = fullAnalyzeFunction(name, function_offset);
     if(!all_defined_heights.empty())
     {
	 ret = getFirstCallerFrameArch(all_defined_heights, in, out);
     }
   }
   // PGCC can confuse our analysis by popping the RA into a GPR and then storing it somewhere completely
   // different. This shows up as a stack height of zero in our analysis.
   // Fixing the infinite loop here.
   if(in.getRA() == out.getRA()) return gcf_not_me;
   
   return ret;
   
}

unsigned AnalysisStepperImpl::getPriority() const
{
   return analysis_priority;
}

bool AnalysisStepperImpl::isPrevInstrACall(Address addr, Address & target)
{
    return callchecker->isPrevInstrACall(addr, target);
} 

std::vector<AnalysisStepperImpl::registerState_t> AnalysisStepperImpl::fullAnalyzeFunction(std::string name, Offset callSite)
{
   std::vector<registerState_t> heights;
  
   CodeObject *obj = getCodeObject(name);
   if (!obj) {
     return heights;
   }

   set<CodeRegion *> regions;
   obj->cs()->findRegions(callSite, regions);
   
   if (regions.empty()) {
      sw_printf("[%s:%d] - Could not find region at %lx\n", FILE__, __LINE__, callSite);
      return heights;
   }
   //We shouldn't be dealing with overlapping regions in a live process
   assert(regions.size() == 1);
   CodeRegion *region = *(regions.begin());
   
   set<ParseAPI::Function*> funcs;
   obj->findFuncs(region, callSite, funcs);
   if (funcs.empty()) {
      sw_printf("[%s:%d] - Could not find function at offset %lx\n", FILE__,
                __LINE__, callSite);
      return heights;
   }

   //Since there is only one region, there is only one block with the offset
   set<ParseAPI::Block*> blocks;
   obj->findBlocks(region, callSite, blocks);
   if(blocks.empty()) return heights;
   
   ParseAPI::Block *block = *(blocks.begin());

   for (set<ParseAPI::Function *>::iterator i = funcs.begin(); i != funcs.end(); i++)
   {
      StackAnalysis analysis(*i);
      analysis.findDefinedHeights(block, callSite, heights);
      
   }

   sw_printf("[%s:%d] - Have %lu possible stack heights in %s at %lx:\n", FILE__, __LINE__, heights.size(), name.c_str(), callSite);

   // Return set of possible heights
   return heights;  
  
}


gcframe_ret_t AnalysisStepperImpl::getFirstCallerFrameArch(const std::vector<registerState_t>& heights,
							   const Frame& in,
							   Frame& out)
{
  ProcessState *proc = getProcessState();

  bool result = false;

  vector<registerState_t>::const_iterator heightIter;
  for (heightIter = heights.begin(); heightIter != heights.end(); ++heightIter) {

    Address
    out_sp = 0,
    out_ra = 0;
    location_t out_ra_loc;

    StackAnalysis::Height sp_height = heightIter->second;


    // SP height is the distance from the last SP of the previous frame
    // to the SP in this frame at the current offset.
    // Since we are walking to the previous frame,
    // we subtract this height to get the outgoing SP
    MachRegisterVal sp_base;

    if (heightIter->first.type() != Absloc::Register) continue;
    proc->getRegValue(heightIter->first.reg(), in.getThread(), sp_base);
    out_sp = sp_base - sp_height.height();

    if(heightIter->second.height() == -1 * (long)proc->getAddressWidth())
    {
      // FP candidate: register pointing to entry SP
       sw_printf("[%s:%d] - Found candidate FP %s, height 0x%lx\n", __FILE__, __LINE__,
                 heightIter->first.format().c_str(), (unsigned long) heightIter->second.height());
    }

    // Since we know the outgoing SP,
    // the outgoing RA must be located just below it
    if(!getOutRA(out_sp, out_ra, out_ra_loc, proc)) continue;

    // If we have multiple potential heights (due to overlapping functions), 
    // check if potential stack height is valid (verify that calculated RA follows a call instr)
    if (heights.size() > 1) {
      if(!validateRA(out_ra)) continue;
    }
      
    out.setSP(out_sp);
    out.setRALocation(out_ra_loc);
    out.setRA(out_ra);
    
    if (result) {
      sw_printf("[%s:%d] - Warning - found multiple valid frames.\n", 
		FILE__, __LINE__);
    } else {
      sw_printf("[%s:%d] - Found a valid frame\n", 
		FILE__, __LINE__);
      result = true;
    }
  }
  return checkResult(result);

}

bool AnalysisStepperImpl::validateRA(Address candidateRA)
{
  sw_printf("[%s:%d] - Calling isPrevInstrACall\n", FILE__, __LINE__);
  Address target;
  if (!isPrevInstrACall(candidateRA, target)) {
    sw_printf("[%s:%d] - Return location %lx does not follow a call instruction\n",
	      FILE__, __LINE__, candidateRA);
    return false;
  }
  return true;
}

gcframe_ret_t AnalysisStepperImpl::checkResult(bool result)
{
  if (result) {
    sw_printf("[%s:%d] - success\n", FILE__, __LINE__); 
    return gcf_success;
  } else {
    sw_printf("[%s:%d] - failed\n", FILE__, __LINE__); 
    return gcf_not_me;
  }
}

bool AnalysisStepperImpl::getOutRA(Address out_sp, Address& out_ra, location_t& out_ra_loc, ProcessState* proc)
{
  // Since we know the outgoing SP,
  // the outgoing RA must be located just below it
  size_t addr_width = proc->getAddressWidth();
  Address out_ra_addr = out_sp - addr_width;
  out_ra_loc.location = loc_address;
  out_ra_loc.val.addr = out_ra_addr;
  
  bool resultMem = proc->readMem(&out_ra, out_ra_addr, addr_width);
  if (!resultMem) {
    sw_printf("[%s:%d] - Error reading from return location %lx on stack\n",
	      FILE__, __LINE__, out_ra_addr);
    return false;
  }
  return true;
}
