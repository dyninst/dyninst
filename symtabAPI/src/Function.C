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

// $Id: Object.C,v 1.31 2008/11/03 15:19:25 jaw Exp $

#include "Symtab.h"
#include "symutil.h"
#include "Module.h"
#include "Collections.h"
#include "Function.h"
#include "VariableLocation.h"
#include "Object.h"
#include "registers/abstract_regs.h"

#if !defined(os_windows)
#include "dwarfFrameParser.h"
#endif

#include <iterator>
#include <algorithm>

#include <iostream>

using namespace std;
using namespace Dyninst;
using namespace Dyninst::SymtabAPI;

FunctionBase::FunctionBase() :
   locals(NULL),
   params(NULL),
   functionSize_(0),
   retType_(NULL),
   inline_parent(NULL),
   frameBaseExpanded_(false),
   data(NULL)
{
}

boost::shared_ptr<Type> FunctionBase::getReturnType(Type::do_share_t) const
{
    getModule()->exec()->parseTypesNow();
    return retType_;
}

bool FunctionBase::setReturnType(boost::shared_ptr<Type> newType)
{
   retType_ = newType;
   return true;
}

bool FunctionBase::findLocalVariable(std::vector<localVar *> &vars, std::string name)
{
    getModule()->exec()->parseTypesNow();

   unsigned origSize = vars.size();

   if (locals) {
      localVar *var = locals->findLocalVar(name);
      if (var)
         vars.push_back(var);
   }

   if (params) {
      localVar *var = params->findLocalVar(name);
      if (var)
         vars.push_back(var);
   }

   if (vars.size() > origSize)
      return true;

   return false;
}

const FuncRangeCollection &FunctionBase::getRanges()
{
   return ranges;
}

bool FunctionBase::getLocalVariables(std::vector<localVar *> &vars)
{
    getModule()->exec()->parseTypesNow();
   if (!locals)
      return false;

   auto p = locals->getAllVars();
   std::copy(p.begin(), p.end(), back_inserter(vars));

   if (p.empty())
      return false;
   return true;
}

bool FunctionBase::getParams(std::vector<localVar *> &params_)
{
    getModule()->exec()->parseTypesNow();
   if (!params)
      return false;

   auto p = params->getAllVars();
   std::copy(p.begin(), p.end(), back_inserter(params_));

   if (p.empty())
      return false;
   return true;
}

bool FunctionBase::addLocalVar(localVar *locVar)
{
   if (!locals) {
      locals = new localVarCollection();
   }
   locals->addLocalVar(locVar);
   return true;
}

bool FunctionBase::addParam(localVar *param)
{
   if (!params) {
      params = new localVarCollection();
   }
   params->addLocalVar(param);
	return true;
}

FunctionBase *FunctionBase::getInlinedParent()
{
    getModule()->exec()->parseTypesNow();
   return inline_parent;
}

const InlineCollection &FunctionBase::getInlines()
{
    getModule()->exec()->parseTypesNow();
   return inlines;
}

FunctionBase::~FunctionBase()
{
   if (locals) {
      delete locals;
      locals = NULL;
   }
   if (params) {
      delete params;
      params = NULL;
   }
}

std::vector<Dyninst::VariableLocation> &FunctionBase::getFramePtrRefForInit() {
   if (inline_parent)
      return inline_parent->getFramePtr();

   return frameBase_;
}

dyn_mutex &FunctionBase::getFramePtrLock()
{
   if (inline_parent)
      return inline_parent->getFramePtrLock();

   return frameBaseLock_;
}

std::vector<Dyninst::VariableLocation> &FunctionBase::getFramePtr()
{
   if (inline_parent)
      return inline_parent->getFramePtr();

   if (frameBaseExpanded_)
      return frameBase_;

   frameBaseExpanded_ = true;

   std::vector<VariableLocation> orig;
   orig.swap(frameBase_);

   for (unsigned i = 0; i < orig.size(); ++i) {
      expandLocation(orig[i], frameBase_);
   }

   return frameBase_;
}

bool FunctionBase::setFramePtr(vector<VariableLocation> *locs)
{
   frameBase_.clear();
   std::copy(locs->begin(), locs->end(), std::back_inserter(frameBase_));
   return true;
}

std::pair<std::string, Dyninst::Offset> InlinedFunction::getCallsite()
{
    std::string callsite_file = "<unknown>";
    if(callsite_file_number > 0 && callsite_file_number < module_->getStrings()->size()) {
        callsite_file = (*module_->getStrings())[callsite_file_number].str;
    }
    return make_pair(callsite_file, callsite_line);
}

void FunctionBase::expandLocation(const VariableLocation &loc,
                              std::vector<VariableLocation> &ret) {
   // We are the frame base, so... WTF?

   assert(loc.mr_reg != Dyninst::FrameBase);

#if defined(os_windows)
   ret.push_back(loc);
   return;
#else
   if (loc.mr_reg != Dyninst::CFA) {
      ret.push_back(loc);
      return;
   }

   using namespace Dyninst::DwarfDyninst;
   auto obj = getModule()->exec()->getObject();
   DwarfFrameParser::Ptr frameParser =
		   DwarfFrameParser::create(*obj->dwarf->frame_dbg(), obj->dwarf->origFile()->e_elfp(), obj->getArch());

   std::vector<VariableLocation> FDEs;
   Dyninst::DwarfDyninst::FrameErrors_t err;
   if(!frameParser) return;
   frameParser->getRegsForFunction(std::make_pair(loc.lowPC, loc.hiPC), Dyninst::CFA, FDEs, err);

   if (FDEs.empty()) {
      // Odd, but happens
      return;
   }

   // This looks surprisingly similar to localVar's version...
   // Perhaps we should unify.

   std::vector<VariableLocation>::iterator i;
   for (i = FDEs.begin(); i != FDEs.end(); i++)
   {
      Offset fdelowPC = i->lowPC;
      Offset fdehiPC = i->hiPC;

      Offset frame_lowPC = loc.lowPC;
      Offset frame_hiPC = loc.hiPC;

      if (frame_hiPC < fdehiPC) {
         // We're done since the variable proceeds the frame
         // base
         break;
      }

      // low is MAX(frame_lowPC, fdelowPC)
      Offset low = (frame_lowPC < fdelowPC) ? fdelowPC : frame_lowPC;

      // high is MIN(frame_hiPC, fdehiPC)
      Offset high = (frame_hiPC < fdehiPC) ? frame_hiPC : fdehiPC;

      VariableLocation newloc;

      newloc.stClass = loc.stClass;
      newloc.refClass = loc.refClass;
      newloc.mr_reg = i->mr_reg;
      newloc.frameOffset = loc.frameOffset + i->frameOffset;
      newloc.lowPC = low;
      newloc.hiPC = high;

/*
      cerr << "Created frame pointer ["
           << hex << newloc.lowPC << ".." << newloc.hiPC
           << "], reg " << newloc.mr_reg.name()
           << " /w/ offset " << newloc.frameOffset
           << " = (" << loc.frameOffset
           << "+" << i->frameOffset << ")"
           << ", "
           << storageClass2Str(newloc.stClass)
           << ", "
           << storageRefClass2Str(newloc.refClass) << endl;
*/


      ret.push_back(newloc);
   }
   return;
#endif
}

void *FunctionBase::getData()
{
   return data;
}

void FunctionBase::setData(void *d)
{
   data = d;
}

Function::Function(Symbol *sym)
    : FunctionBase(), Aggregate(sym)
{}

Function::Function()
    : FunctionBase()
{}

int Function::getFramePtrRegnum() const
{
   return 0;
}

bool Function::setFramePtrRegnum(int)
{
   return false;
}

Offset Function::getPtrOffset() const
{
    Offset retval = 0;
    for (unsigned i = 0; i < symbols_.size(); ++i) {
        Offset tmp_off = symbols_[i]->getPtrOffset();
        if (tmp_off) {
           if (retval == 0) retval = tmp_off;
           assert(retval == tmp_off);
        }
    }
    return retval;
}

Offset Function::getTOCOffset() const
{
    Offset retval = 0;
    for (unsigned i = 0; i < symbols_.size(); ++i) {
        Offset tmp_toc = symbols_[i]->getLocalTOC();
        if (tmp_toc) {
            if (retval == 0) retval = tmp_toc;
            assert(retval == tmp_toc);
        }
    }
    return retval;
}

Function::~Function()
{
}

bool Function::removeSymbol(Symbol *sym)
{
	removeSymbolInt(sym);
	if (symbols_.empty()) {
	    getModule()->exec()->deleteFunction(this);
	}
	return true;
}

std::ostream &operator<<(std::ostream &os, const Dyninst::VariableLocation &l)
{
	const char *stc = storageClass2Str(l.stClass);
	const char *strc = storageRefClass2Str(l.refClass);
	os << "{"
           << "storageClass=" << stc
           << " storageRefClass=" << strc
           << " reg=" << l.mr_reg.name()
           << " frameOffset=" << l.frameOffset
           << " lowPC=" << l.lowPC
           << " hiPC=" << l.hiPC
           << "}";
	return os;
}

std::ostream &operator<<(std::ostream &os, const Dyninst::SymtabAPI::Function &f)
{
	boost::shared_ptr<Type> retType = (const_cast<Function &>(f)).getReturnType(Type::share);

	std::string tname(retType ? retType->getName() : "no_type");
	const Aggregate *ag = dynamic_cast<const Aggregate *>(&f);
	assert(ag);

	os  << "Function{"
		<< " type=" << tname
            << " framePtrRegNum_=" << f.getFramePtrRegnum()
		<< " FramePtrLocationList=[";
#if 0
	for (unsigned int i = 0; i < f.frameBase_.size(); ++i)
	{
		os << f.frameBase_[i];
		if ( (i + 1) < f.frameBase_.size())
			os << ", ";
	}
#endif
	os  << "] ";
	os  <<  *ag;
	os  <<  "}";
	return os;

}

std::string Function::getName() const
{
    return getFirstSymbol()->getMangledName();
}

bool FunctionBase::operator==(const FunctionBase &f)
{
	if (retType_ && !f.retType_)
		return false;
	if (!retType_ && f.retType_)
		return false;
	if (retType_)
		if (retType_->getID() != f.retType_->getID())
		{
			return false;
		}

	return ((Aggregate &)(*this)) == ((const Aggregate &)f);
}

InlinedFunction::InlinedFunction(FunctionBase *parent) :
    FunctionBase(),
    callsite_file_number(0),
    callsite_line(0),
    module_(parent->getModule())
{
    inline_parent = parent;
    offset_ = parent->getOffset();
    boost::unique_lock<dyn_mutex> l(parent->inlines_lock);
    parent->inlines.push_back(this);
}

InlinedFunction::~InlinedFunction()
{
}

bool InlinedFunction::removeSymbol(Symbol *)
{
   return false;
}

bool InlinedFunction::addMangledName(std::string name, bool /*isPrimary*/, bool /*isDebug*/)
{
    name_ = name;
    return true;
}

bool InlinedFunction::addPrettyName(std::string name, bool /*isPrimary*/, bool /*isDebug*/)
{
    name_ = name;
    return true;
}

std::string InlinedFunction::getName() const
{
    return name_;
}

Offset InlinedFunction::getOffset() const
{
    return offset_;
}

unsigned InlinedFunction::getSize() const
{
    return functionSize_;//inline_parent->getSize();
}

void InlinedFunction::setFile(string filename) {
    StringTablePtr strs = module_->getStrings();
    boost::unique_lock<dyn_mutex> l(strs->lock);
    // This looks gross, but here's what it does:
    // Get index 1 (unique by name). Insert the filename on that index (which defaults to push_back if empty).
    // Returns an <iterator, bool>; get the iterator (we don't care if it's new). Project to random access (index 0).
    // Difference from begin == array index in string table.
    callsite_file_number = strs->project<0>(strs->get<1>().insert(StringTableEntry(filename,"")).first) - strs->begin();
}

Module* Function::getModule() const {
    return module_;
}
