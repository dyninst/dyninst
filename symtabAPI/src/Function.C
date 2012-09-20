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
#include "common/h/serialize.h"

#include "Symtab.h"
#include "symutil.h"
#include "Module.h"
#include "Collections.h"
#include "Function.h"
#include "dynutil/h/VariableLocation.h"
#include "symtabAPI/src/Object.h"

//#include "dwarf/h/dwarfFrameParser.h"

#include "annotations.h"
#include <iterator>


#include <iostream>

using namespace std;
using namespace Dyninst;
using namespace Dyninst::SymtabAPI;

Function::Function(Symbol *sym)
    : Aggregate(sym),
      retType_(NULL), 
      framePtrRegNum_(-1),
      frameBaseExpanded_(false),
      functionSize_(0)
{}

Function::Function()
    : Aggregate(),
      retType_(NULL), 
      framePtrRegNum_(-1),
      frameBaseExpanded_(false),
      functionSize_(0)
{}

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

std::vector<Dyninst::VariableLocation> &Function::getFramePtrRefForInit() {
   return frameBase_;
}

std::vector<Dyninst::VariableLocation> &Function::getFramePtr() 
{
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

bool Function::setFramePtr(vector<VariableLocation> *locs) 
{
   frameBase_.clear();
   std::copy(locs->begin(), locs->end(), std::back_inserter(frameBase_));
   return true;
}

void Function::expandLocation(const VariableLocation &loc,
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

   Dyninst::Dwarf::DwarfFrameParser::Ptr frameParser =
      Dyninst::Dwarf::DwarfFrameParser::create(module_->exec()->getObject()->dwarf_dbg(),
                                               module_->exec()->getObject()->getArch());
   
   std::vector<VariableLocation> FDEs;
   Dyninst::Dwarf::FrameErrors_t err;
   frameParser->getRegsForFunction(getOffset(),
                                   Dyninst::CFA,
                                   FDEs,
                                   err);


   assert(!FDEs.empty());

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

#if 0 
   fprintf(stderr, "%s[%d]:  FIXME here: localVarCollection = %p\n", FILE__, __LINE__, lvs);
   std::vector<localVar *> * v  = new std::vector<localVar *>();
   vars = *v;
   return true;
#else
   vars = *(lvs->getAllVars());

   if (vars.size())
      return true;
#endif

   fprintf(stderr, "%s[%d]:  NO LOCAL VARS\n", FILE__, __LINE__);
   return false;
}

bool Function::getParams(std::vector<localVar *> &params)
{
   module_->exec()->parseTypesNow();

   localVarCollection *lvs = NULL;
   if (!getAnnotation(lvs, FunctionParametersAnno))
   {
      if (!setupParams())
      {
         return false;
      }

      if (!getAnnotation(lvs, FunctionParametersAnno))
      {
         return false;
      }
   }

   if (!lvs)
   {
      fprintf(stderr, "%s[%d]:  FIXME:  NULL ptr for annotation\n", FILE__, __LINE__);
      return false;
   }

   params = *(lvs->getAllVars());
   
   return true;
}

bool Function::addLocalVar(localVar *locVar)
{
   localVarCollection *lvs = NULL;

   if (!getAnnotation(lvs, FunctionLocalVariablesAnno))
   {
      lvs = new localVarCollection();

      if (!addAnnotation(lvs, FunctionLocalVariablesAnno))
      {
         return false;
      }
   }
   if (!lvs)
      return false;

   lvs->addLocalVar(locVar);
   return true;
}

bool Function::addParam(localVar *param)
{
	localVarCollection *ps = NULL;
   if (!setupParams())
   {
      return false;
   }

	if (!getAnnotation(ps, FunctionParametersAnno))
	{
      return false;
	}
   
	ps->addLocalVar(param);

	return true;
}

bool Function::setupParams()
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
        
   return true;
}

Function::~Function()
{
   localVarCollection *lvs = NULL;
   if (getAnnotation(lvs, FunctionLocalVariablesAnno) && (NULL != lvs))
   {
	   if (!removeAnnotation(FunctionLocalVariablesAnno))
	   {
		   fprintf(stderr, "%s[%d]:  ERROR removing local vars\n", FILE__, __LINE__);
	   }
	   delete lvs;
   }

   localVarCollection *lps = NULL;
   if (getAnnotation(lps, FunctionParametersAnno) && (NULL != lps))
   {
	   if (!removeAnnotation(FunctionParametersAnno))
	   {
		   fprintf(stderr, "%s[%d]:  ERROR removing params\n", FILE__, __LINE__);
	   }
	   delete lps;
   }

}

#if !defined(SERIALIZATION_DISABLED)
Serializable *Function::serialize_impl(SerializerBase *sb, const char *tag) THROW_SPEC (SerializerError)
{
	if (!sb) SER_ERR("bad paramater sb");



	//  Use typeID as unique identifier
	unsigned int t_id = retType_ ? retType_->getID() : (unsigned int) 0xdeadbeef;

		ifxml_start_element(sb, tag);
		gtranslate(sb, t_id, "typeID");
		gtranslate(sb, framePtrRegNum_, "framePointerRegister");
#if 0
		gtranslate(sb, frameBase_, "framePointerLocationList");
#endif
		Aggregate::serialize_aggregate(sb);
		ifxml_end_element(sb, tag);
		if (sb->isInput())
		{
			if (t_id == 0xdeadbeef)
				retType_ = NULL;
			else
				restore_type_by_id(sb, retType_, t_id);
#if 0
			for (unsigned long i = 0; i < symbols_.size(); ++i)
			{
				symbols_[i]->setFunction(this);
				assert(symbols_[i]->isFunction());
			}
#endif
		}

	serialize_printf("%s[%d]:  Function(%p--%s)::%s\n", FILE__, __LINE__, this,
			getAllPrettyNames().size() ? getAllPrettyNames()[0].c_str() : "UNNAMED_FUNCTION",
			sb->isInput() ? "deserialize" : "serialize");
	return NULL;
}
#else
Serializable *Function::serialize_impl(SerializerBase *, const char *) THROW_SPEC (SerializerError)
{
   return NULL;
}
#endif

bool Function::removeSymbol(Symbol *sym) 
{
	removeSymbolInt(sym);
	if (symbols_.empty()) {
		module_->exec()->deleteFunction(this);
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
	Type *retType = (const_cast<Function &>(f)).getReturnType();

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

bool Function::operator==(const Function &f)
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

	if (framePtrRegNum_ != f.framePtrRegNum_)
		return false;

#if 0
	if (frameBase_.size() != f.frameBase_.size())
		return false;

	for (unsigned int i = 0; i < frameBase_.size(); ++i)
	{
		if (frameBase_[i] == frameBase_[i])
			return false;
	}
#endif

	return ((Aggregate &)(*this)) == ((Aggregate &)f);
}

