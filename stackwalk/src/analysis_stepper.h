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

#if !defined(ANALYSIS_STEPPER_H_)
#define ANALYSIS_STEPPER_H_

#include "stackwalk/h/framestepper.h"
#include "dataflowAPI/h/stackanalysis.h"
#include "dataflowAPI/h/Absloc.h"
#include "SymReader.h"

#include <string>
#include <map>
#include <set>
#include <utility>
#include <vector>

namespace Dyninst {
namespace ParseAPI {
class CodeObject;
class CodeSource;
 class CodeRegion;
 
}
}

namespace Dyninst {
namespace Stackwalker {

class CallChecker;
class AnalysisStepperImpl : public FrameStepper
{
  private:
   AnalysisStepper *parent;
   CallChecker * callchecker;
  public:
   AnalysisStepperImpl(Walker *w, AnalysisStepper *p);
   virtual ~AnalysisStepperImpl();

   typedef std::pair<StackAnalysis::Height, StackAnalysis::Height> height_pair_t;
   typedef std::pair<Absloc, StackAnalysis::Height> registerState_t;
   
   static const height_pair_t err_height_pair;

   virtual gcframe_ret_t getCallerFrame(const Frame &in, Frame &out);
   virtual unsigned getPriority() const;  
   
   virtual const char *getName() const;
   
  protected:
   
   static std::map<std::string, ParseAPI::CodeObject *> objs;
   static std::map<std::string, ParseAPI::CodeSource*> srcs;
   static std::map<std::string, SymReader*> readers;
   
   static ParseAPI::CodeObject *getCodeObject(std::string name);
   static ParseAPI::CodeSource *getCodeSource(std::string name);

   std::set<height_pair_t> analyzeFunction(std::string name, Offset off);
   std::vector<registerState_t> fullAnalyzeFunction(std::string name, Offset off);
   
   virtual bool isPrevInstrACall(Address addr, Address & target);
   virtual gcframe_ret_t getCallerFrameArch(std::set<height_pair_t> height, const Frame &in, Frame &out);
   gcframe_ret_t getFirstCallerFrameArch(const std::vector<registerState_t>& heights, const Frame& in, Frame& out);
   gcframe_ret_t checkResult(bool result);
   bool validateRA(Address candidateRA);
   ParseAPI::CodeRegion* getCodeRegion(std::string name, Offset off);
   bool getOutRA(Address out_sp, Address& out_ra, location_t& out_ra_loc, ProcessState* proc);
   
   
};

}
}
#endif
