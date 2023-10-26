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

#include "Annotatable.h"

#include "Symtab.h"
#include "symutil.h"
#include "Module.h"
#include "Collections.h"
#include "Variable.h"
#include "Aggregate.h"
#include "Function.h"
#include <iterator>
#include "registers/abstract_regs.h"
#include "symtabAPI/src/Object.h"

#include <iostream>

using namespace std;
using namespace Dyninst;
using namespace Dyninst::SymtabAPI;


Variable::Variable(Symbol *sym) :
	Aggregate(sym),
	type_(NULL)
{
}

Variable::Variable() :
	Aggregate(),
	type_(NULL)
{
}
void Variable::setType(boost::shared_ptr<Type> type)
{
	type_ = type;
}

boost::shared_ptr<Type> Variable::getType(Type::do_share_t)
{
	module_->exec()->parseTypesNow();
	return type_;
}

void Variable::print(std::ostream &os) const {
	boost::shared_ptr<Type> var_t = const_cast<Variable *>(this)->getType(Type::share);
	std::string tname(var_t ? var_t->getName() : "no_type");

	os  << "Variable{"        
		<< " type=" 
		<< tname
	    << " ";
	Aggregate::print(os);
	os  << 	"}";
}

bool Variable::operator==(const Variable &v)
{
	if (type_ && !v.type_)
		return false;
	if (!type_ && v.type_)
		return false;
	if (type_)
		if (type_->getID() != v.type_->getID())
		{
			return false;
		}
	return ((Aggregate &)(*this)) == ((const Aggregate &)v);
}

bool Variable::removeSymbol(Symbol *sym) 
{
    removeSymbolInt(sym);
    if (symbols_.empty()) {
        module_->exec()->deleteVariable(this);
    }
    return true;
}

localVar::localVar(std::string name,  boost::shared_ptr<Type> typ, std::string fileName, 
		int lineNum, FunctionBase *f, std::vector<VariableLocation> *locs) :
	name_(name), 
	type_(typ), 
	fileName_(fileName), 
	lineNum_(lineNum),
        func_(f),
        locsExpanded_(false)
{
	if (locs)
	{
           std::copy(locs->begin(), locs->end(), std::back_inserter(locs_));
	}
}

localVar::localVar(localVar &lvar)
:
	AnnotatableSparse(),
	name_(lvar.name_),
	type_(lvar.type_),
	fileName_(lvar.fileName_),
	lineNum_(lvar.lineNum_),
	func_(lvar.func_),
	locsExpanded_(lvar.locsExpanded_)
{
        std::copy(lvar.locs_.begin(), lvar.locs_.end(),
                  std::back_inserter(locs_));
}

bool localVar::addLocation(const VariableLocation &location)
{
   if (!locsExpanded_) {
      locs_.push_back(location);
      return true;
   }

   expandLocation(location, locs_);
   return true;
}

void localVar::expandLocation(
        const VariableLocation &loc,
        std::vector<VariableLocation> &ret)
{
    if (loc.mr_reg != Dyninst::FrameBase) {
        ret.push_back(loc);
        return;
    }

    // We're referencing a frame base; must have a function or this
    // is corrupted data. 
    assert(func_);

    std::vector<VariableLocation> &func_fp = func_->getFramePtr();

   //#define DEBUG

#ifdef DEBUG
   cerr << "Expanding location for variable " << name_ 
	<< " / " << hex << this << dec << endl;
#endif

   /*
   if (func_fp.empty()) {
      cerr << "Error: function " << hex << func_
           << " / " << func_->getAllMangledNames()[0] << " has no frame pointer!" << endl;
   }
   */
   if (func_fp.empty())
      return;

   // We need to break loc into a list matching the address
   // ranges of func_fp. 
   
   // Also, combine our frame offset with func_fp[...]'s frame
   // offset and use its register.

   std::vector<VariableLocation>::iterator i;
   for (i = func_fp.begin(); i != func_fp.end(); i++) 
   {
      Offset fplowPC = i->lowPC;
      Offset fphiPC = i->hiPC;

      Offset varlowPC = loc.lowPC;
      Offset varhiPC = loc.hiPC;
#ifdef DEBUG
      cerr << "var range: " << hex
           << varlowPC << ".." << varhiPC << endl;
      cerr << "frame range: " << hex
           << fplowPC << ".." << fphiPC << dec << endl;
#endif
      if (fplowPC > varhiPC) {
         // Done, the frame base is after the variable
         break;
      }

      if (fphiPC < varlowPC) {
         // No overlap yet, continue
         continue;
      }


      // low is MAX(varlowPC, fplowPC)
      Offset low = (varlowPC < fplowPC) ? fplowPC : varlowPC;

      // high is MIN(varhiPC, fphiPC)
      Offset high = (varhiPC < fphiPC) ? varhiPC : fphiPC;

      VariableLocation newloc;

      newloc.stClass = loc.stClass;
      newloc.refClass = loc.refClass;
      newloc.mr_reg = i->mr_reg;
      newloc.frameOffset = loc.frameOffset + i->frameOffset;
      newloc.frameOffsetAbs = loc.frameOffset;
      newloc.lowPC = low;
      newloc.hiPC = high;

#ifdef DEBUG
      cerr << "Created variable location ["
           << hex << newloc.lowPC << ".." << newloc.hiPC
           << "], reg " << newloc.mr_reg.name()
           << " /w/ offset " << newloc.frameOffset
           << " = (" << loc.frameOffset 
           << "+" << i->frameOffset << ")" 
           << ", " 
           << storageClass2Str(newloc.stClass)
           << ", " 
           << storageRefClass2Str(newloc.refClass) << endl;
#endif
      ret.push_back(newloc);
   }
   return;
}

localVar::~localVar() {}

void localVar::fixupUnknown(Module *module) 
{
	if (type_->getDataClass() == dataUnknownType) 
	{
		typeCollection *tc = typeCollection::getModTypeCollection(module);
		assert(tc);
        auto t = tc->findType(type_->getID(), Type::share);
        if(t) type_ = t;
	}
}

std::string &localVar::getName() 
{
	return name_;
}

boost::shared_ptr<Type> localVar::getType(Type::do_share_t)
{
	return type_;
}

bool localVar::setType(boost::shared_ptr<Type> newType) 
{
	type_ = newType;
	return true;
}

int localVar::getLineNum() 
{
	return lineNum_;
}

std::string &localVar::getFileName() 
{
	return fileName_;
}

std::vector<Dyninst::VariableLocation> &localVar::getLocationLists() 
{
   if (!locsExpanded_) {
      // Here we get clever
      std::vector<VariableLocation> orig;
      locs_.swap(orig);
      for (unsigned i = 0; i < orig.size(); ++i) {
         expandLocation(orig[i], locs_);
      }

      locsExpanded_ = true;
   }

   return locs_;
}

bool localVar::operator==(const localVar &l)
{
	if (type_ && !l.type_) return false;
	if (!type_ && l.type_) return false;

	if (type_)
	{
		if (type_->getID() != l.type_->getID())
			return false;
	}

	if (name_ != l.name_) return false;
	if (fileName_ != l.fileName_) return false;
	if (lineNum_ != l.lineNum_) return false;

        if (locs_ != l.locs_) return false;

	return true;
}
