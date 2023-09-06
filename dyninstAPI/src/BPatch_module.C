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

#include <stdio.h>
#include <ctype.h>
#include <string>
#include <sstream>

#define BPATCH_FILE

#include "function.h"
#include "debug.h"
#include "addressSpace.h"
#include "dynProcess.h"
#include "BPatch.h"
#include "BPatch_module.h"
#include "BPatch_libInfo.h"
#include "BPatch_function.h"
#include "BPatch_point.h"
#include "BPatch_statement.h"
#include "BPatch_collections.h"
#include "symtabAPI/h/Type.h"    // For BPatch_type related stuff

#include "mapped_module.h"
#include "mapped_object.h"
#include "instPoint.h"

using namespace SymtabAPI;

std::string current_func_name;
std::string current_mangled_func_name;
BPatch_function *current_func = NULL;

/*
 * BPatch_module::getSourceObj()
 *
 * Return the contained source objects (e.g. functions).
 *
 */

bool BPatch_module::getSourceObj(BPatch_Vector<BPatch_sourceObj *> &vect)
{
   if (!mod) return false;

   BPatch_Vector<BPatch_function *> temp;
   bool result = getProcedures(temp);
   if (!result)
      return false;

   for (unsigned int i = 0; i < temp.size(); ++i) {
      vect.push_back((BPatch_sourceObj*) temp[i]);
   }
   return true;
}

/*
 * BPatch_function::getObjParent()
 *
 * Return the parent of the function (i.e. the image)
 *
 */
BPatch_sourceObj *BPatch_module::getObjParent()
{
   return (BPatch_sourceObj *) img;
}

BPatch_object *BPatch_module::getObject() {
   if (!isValid()) return NULL;
   return img->findOrCreateObject(mod->obj());
}

char *BPatch_module::getName(char *buffer, int length)
{
   if (!mod)
      return NULL;

   string str = mod->fileName();

   strncpy(buffer, str.c_str(), length);

   return buffer;
}

const char *BPatch_module::libraryName()
{
   if (!mod)
      return NULL;

   if (isSharedLib())      
      return mod->fileName().c_str();

   return NULL;
}

char *BPatch_module::getFullName(char *buffer, int length)
{
   if (!mod)
      return NULL;
   string str = mod->fileName();

   strncpy(buffer, str.c_str(), length);

   return buffer;
}


BPatch_module::BPatch_module(BPatch_addressSpace *_addSpace, 
                             AddressSpace *as,
                             mapped_module *_mod,
                             BPatch_image *_img ) :
   addSpace(_addSpace), 
   lladdSpace(as),
   mod(_mod),
   img(_img), 
   moduleTypes(NULL),
   full_func_parse(false),
   full_var_parse(false)
{
   _srcType = BPatch_sourceModule;

   switch(mod->language()) {
      case lang_C:
         setLanguage( BPatch_c );
         break;

      case lang_CPlusPlus:
      case lang_GnuCPlusPlus:
         setLanguage( BPatch_cPlusPlus );
         break;

      case lang_Fortran:
      case lang_CMFortran:
         setLanguage( BPatch_fortran );
         break;

      case lang_Assembly:
         setLanguage( BPatch_assembly );
         break;

      case lang_Unknown: 
      default:
         setLanguage( BPatch_unknownLanguage );
         break;
   }; /* end language switch */

} /* end BPatch_module() */

// Public 'destructor' function...
void BPatch_module::handleUnload() 
{
   // Hrm... what to do. For now, mark us as "deleted" so that
   // any other calls return an error.

   // Brainstorm: we can set mod to NULL (as it really is, 
   // having been deleted...) and key off that. Saves a boolean.

   mod = NULL;
}

bool BPatch_module::isValid() 
{
   return mod != NULL;
}

BPatch_module::~BPatch_module()
{
   if (moduleTypes) {
      BPatch_typeCollection::freeTypeCollection(moduleTypes);
   }

   // XXX The odd while-loop structures allow the
   //     destructors for map objects to remove themselves
   //     from the maps; otherwise the iterators will
   //     become invalidated

   BPatch_funcMap::iterator fi = func_map.begin();
   BPatch_funcMap::iterator tmp_f;
   while(fi != func_map.end()) {
      tmp_f = fi;
      ++fi;
      delete (*tmp_f).second;
   }

   BPatch_instpMap::iterator ii = instp_map.begin();
   BPatch_instpMap::iterator tmp_i;
   while(ii != instp_map.end()) {
      tmp_i = ii;
      ++ii; 
      delete (*tmp_i).second;
   }

   BPatch_varMap::iterator vi = var_map.begin();
   BPatch_varMap::iterator tmp_v;
   while(vi != var_map.end()) {
      tmp_v = vi;
      ++vi;
      delete (*tmp_v).second;
   }

   func_map.clear();
   instp_map.clear();
   var_map.clear();
}

bool BPatch_module::parseTypesIfNecessary() 
{
	if ( moduleTypes != NULL ) 
		return false;

	if (!isValid())
		return false;

	bool is64 = (mod->pmod()->imExec()->codeObject()->cs()->getAddressWidth() == 8);

	if (sizeof(void *) == 8 && !is64) 
	{
		// Terrible Hack:
		//   If mutatee and mutator address width is different,
		//   we need to patch up certain standard types.
		BPatch_type *typePtr;

		typePtr = BPatch::bpatch->builtInTypes->findBuiltInType(-10);
		typePtr->getSymtabType(Dyninst::SymtabAPI::Type::share)->setSize(4);

		typePtr = BPatch::bpatch->builtInTypes->findBuiltInType(-19);
		typePtr->getSymtabType(Dyninst::SymtabAPI::Type::share)->setSize(4);
	}

	mod->pmod()->mod()->exec()->parseTypesNow();
	moduleTypes = BPatch_typeCollection::getModTypeCollection(this);

	vector<boost::shared_ptr<Type>> modtypes;
    mod->pmod()->mod()->getAllTypes(modtypes);

	if (modtypes.empty())
		return false;

	for (unsigned i=0; i<modtypes.size(); i++) 
	{
		auto typ = modtypes[i];
		BPatch_type *type = new BPatch_type(typ);
		moduleTypes->addType(type);
	}

	vector<pair<string, boost::shared_ptr<Type> > > globalVars;
    mod->pmod()->mod()->getAllGlobalVars(globalVars);

	if (globalVars.empty())
		return false;

	for (unsigned i=0; i<globalVars.size(); i++)
	{
		BPatch_type *var_type = NULL;
		extern AnnotationClass<BPatch_type> TypeUpPtrAnno;

		auto ll_var_type = globalVars[i].second;
		std::string &var_name = globalVars[i].first;

		assert(ll_var_type);

		if (!ll_var_type->getAnnotation(var_type, TypeUpPtrAnno))
		{
			var_type = new BPatch_type(globalVars[i].second);
		}
		else
		{
			assert(var_type);
		}

		moduleTypes->addGlobalVariable(var_name.c_str(), var_type);
#if 0
		if (!globalVars[i].second->getUpPtr())
		{
			new BPatch_type(globalVars[i].second);
		}

		moduleTypes->addGlobalVariable(globalVars[i].first.c_str(), 
				(BPatch_type *)globalVars[i].second->getUpPtr());
#endif
	}
	return true; 
}

BPatch_typeCollection *BPatch_module::getModuleTypes() 
{
	parseTypesIfNecessary();
	return moduleTypes;
}

/*
 * BPatch_module::getProcedures
 *
 * Returns a list of all procedures in the module upon success, and NULL
 * upon failure.
 */
BPatch_Vector<BPatch_function *> *
	BPatch_module::getProcedures(bool incUninstrumentable) {
		if (!isValid())
			return NULL;

   BPatch_Vector<BPatch_function*> *funcs = new BPatch_Vector<BPatch_function*>();
   bool result = getProcedures(*funcs, incUninstrumentable);
   if (!result) {
      delete funcs;
      return NULL;
   }

   return funcs;
}

bool BPatch_module::getProcedures(BPatch_Vector<BPatch_function*> &funcs,
                                     bool incUninstrumentable)
{
   if (!isValid())
      return false;

   if (!full_func_parse || 
       func_map.size() != mod->getFuncVectorSize() || 
       mod->obj()->isExploratoryModeOn())
   {
      const std::vector<func_instance*> &funcs_ = mod->getAllFunctions();
      for (unsigned i=0; i<funcs_.size(); i++) {
         if (!func_map.count(funcs_[i])) {
            addSpace->findOrCreateBPFunc(funcs_[i], this); // adds func to func_map
         }
      }
      full_func_parse = true;
   }      

   BPatch_funcMap::iterator i = func_map.begin();
   for (; i != func_map.end(); i++) {
      func_instance *fi = static_cast<func_instance *>(i->first);
      if (incUninstrumentable || fi->isInstrumentable()) {
         funcs.push_back((*i).second);
      }
   }
   return true;
}

/*
 * BPatch_module::findFunction
 *
 * Returns a vector of BPatch_function* with the same name that is provided or
 * NULL if no function with that name is in the module.  This function
 * searches the BPatch_function vector of the module followed by
 * the func_instance of the module.  If a func_instance is found
 * a BPatch_function is created and added to the BPatch_function vector of
 * the module.
 * name The name of function to look up.
 */

   BPatch_Vector<BPatch_function *> *
BPatch_module::findFunction(const char *name, 
      BPatch_Vector<BPatch_function *> & funcs,
      bool notify_on_failure, bool regex_case_sensitive,
      bool incUninstrumentable, bool dont_use_regex)
{
   if (!isValid())
      return NULL;

   unsigned size = funcs.size();

   if (!name) {
      char msg[512];
      sprintf(msg, "%s[%d]:  Module %s: findFunction(NULL)...  failing",
            __FILE__, __LINE__, mod->fileName().c_str());
      BPatch_reportError(BPatchSerious, 100, msg);
      return NULL;
   }

   // Do we want regex?
   if (dont_use_regex 
         ||  (NULL == strpbrk(name, REGEX_CHARSET))) {
      std::vector<func_instance *> int_funcs;
      if (mod->findFuncVectorByPretty(name, int_funcs)) {
         for (unsigned piter = 0; piter < int_funcs.size(); piter++) {
            if (incUninstrumentable || int_funcs[piter]->isInstrumentable()) 
            {
               BPatch_function * bpfunc = addSpace->findOrCreateBPFunc(int_funcs[piter], this);
               funcs.push_back(bpfunc);
            }
         }
      }
      else {
         if (mod->findFuncVectorByMangled(name,
                  int_funcs)) {
            for (unsigned miter = 0; miter < int_funcs.size(); miter++) {
               if (incUninstrumentable || int_funcs[miter]->isInstrumentable()) 
               {
                  BPatch_function * bpfunc = addSpace->findOrCreateBPFunc(int_funcs[miter], this);
                  //						BPatch_function * bpfunc = proc->findOrCreateBPFunc(int_funcs[miter], this);
                  funcs.push_back(bpfunc);
               }
            }
         }
      }
      if (size != funcs.size())
         return &funcs;
   }
   else {
      // Regular expression search. As with BPatch_image, we handle it here

#if !defined(os_windows)
      // REGEX falls through:
      regex_t comp_pat;
      int err, cflags = REG_NOSUB | REG_EXTENDED;

      if ( !regex_case_sensitive )
         cflags |= REG_ICASE;

      //cerr << "compiling regex: " <<name<<endl;

      if (0 != (err = regcomp( &comp_pat, name, cflags ))) {
         char errbuf[80];
         regerror( err, &comp_pat, errbuf, 80 );
         if (notify_on_failure) {
            cerr << __FILE__ << ":" << __LINE__ << ":  REGEXEC ERROR: "<< errbuf << endl;
            std::string msg = std::string("Image: Unable to find function pattern: ") 
               + std::string(name) + ": regex error --" + std::string(errbuf);
            BPatch_reportError(BPatchSerious, 100, msg.c_str());
         }
         return NULL;
      }

      // Regular expression search. This used to be handled at the image
      // class level, but was moved up here to simplify semantics. We
      // have to iterate over every function known to the process at some
      // point, so it might as well be top-level. This is also an
      // excellent candidate for a "value-added" library.

      const std::vector<func_instance *> &int_funcs = mod->getAllFunctions();

      for (unsigned ai = 0; ai < int_funcs.size(); ai++) {
         func_instance *func = int_funcs[ai];
         // If it matches, push onto the vector
         // Check all pretty names (and then all mangled names if there is no match)
         bool found_match = false;
         for (auto piter = func->pretty_names_begin(); 
	      piter != func->pretty_names_end();
	      ++piter) {
	   const string &pName = *piter;
            int err_code;
            if (0 == (err_code = regexec(&comp_pat, pName.c_str(), 1, NULL, 0 ))){
               if (func->isInstrumentable() || incUninstrumentable) {
                  BPatch_function *foo = addSpace->findOrCreateBPFunc(func, NULL);
                  //	   BPatch_function *foo = proc->findOrCreateBPFunc(func, NULL);
                  funcs.push_back(foo);
               }
               found_match = true;
               break;
            }
         }
         if (found_match) continue; // Don't check mangled names

         for (auto miter = func->symtab_names_begin(); 
	      miter != func->symtab_names_end();
	      ++miter) {
	   const string &mName = *miter;
            int err_code;

            if (0 == (err_code = regexec(&comp_pat, mName.c_str(), 1, NULL, 0 ))){
               if (func->isInstrumentable() || incUninstrumentable) {
                  BPatch_function *foo = addSpace->findOrCreateBPFunc(func, NULL);
                  //	   BPatch_function *foo = proc->findOrCreateBPFunc(func, NULL);
                  funcs.push_back(foo);
               }
               found_match = true;
               break;
            }
         }
      }

      regfree(&comp_pat);

      if (funcs.size() != size) {
         return &funcs;
      } 

      if (notify_on_failure) {
         std::string msg = std::string("Unable to find pattern: ") + std::string(name);
         BPatch_reportError(BPatchSerious, 100, msg.c_str());
      }
#endif
   }

   if (notify_on_failure) {
      char msg[1024];
      sprintf(msg, "%s[%d]:  Module %s: unable to find function %s",
            __FILE__, __LINE__, mod->fileName().c_str(), name);
      BPatch_reportError(BPatchSerious, 100, msg);

   }
   return &funcs;
}

   BPatch_Vector<BPatch_function *> *
BPatch_module::findFunctionByAddress(void *addr, BPatch_Vector<BPatch_function *> &funcs,
      bool notify_on_failure, 
      bool incUninstrumentable)
{
   if (!isValid()) {
      if (notify_on_failure) {
         BPatch_reportError(BPatchSerious, 100, "Module is not valid");
      }
      return NULL;
   }

   BPatch_function *bpfunc = NULL;
   std::set<func_instance *> ifuncs;
   mod->findFuncsByAddr((Address) addr, ifuncs);

   for (std::set<func_instance *>::iterator iter = ifuncs.begin(); 
       iter != ifuncs.end(); ++iter) {
        func_instance *pdfunc = *iter; 
        if (incUninstrumentable || pdfunc->isInstrumentable()) {
           bpfunc = addSpace->findOrCreateBPFunc(pdfunc, this);
          if (bpfunc) {
               funcs.push_back(bpfunc);
           }
        }
   }
   if (funcs.empty() && notify_on_failure) {
       std::ostringstream msg;
       msg << "No functions at: " << std::hex << (Address)addr
           << " " << mod->fileName();
       BPatch_reportError(BPatchSerious, 100, msg.str().c_str());
   }

   return &funcs;
}

BPatch_function * BPatch_module::findFunctionByMangled(const char *mangled_name,
      bool incUninstrumentable)
{
   if (!isValid()) return NULL;

   BPatch_function *bpfunc = NULL;

   std::vector<func_instance *> int_funcs;
   std::string mangled_str(mangled_name);

   if (!mod->findFuncVectorByMangled(mangled_str,
            int_funcs))
      return NULL;

   if (int_funcs.size() > 1) {
      fprintf(stderr, "%s[%d]: Warning: found multiple name matches for %s, returning first\n",
            FILE__, __LINE__, mangled_name);
   }

   func_instance *pdfunc = int_funcs[0];

   if (incUninstrumentable || pdfunc->isInstrumentable()) {
      bpfunc = addSpace->findOrCreateBPFunc(pdfunc, this);
   }

   return bpfunc;
}

bool BPatch_module::dumpMangled(char * prefix)
{
   mod->dumpMangled(prefix);
   return true;
}

bool BPatch_module::remove(BPatch_function *bpfunc)
{
    func_instance *func = bpfunc->lowlevel_func();

    bool foundIt = false;
    BPatch_funcMap::iterator fmap_iter = func_map.find(func);
    if (func_map.end() != fmap_iter) {
        foundIt = true;
    }

    if (!foundIt) {
        return false;
    }

    func_map.erase(fmap_iter);

    return true;
}

bool BPatch_module::remove(instPoint* point)
{
    BPatch_instpMap::iterator pit = instp_map.find(point);
    if (pit != instp_map.end()) {
        instp_map.erase(pit);
        return true;
    }
    return false;
}

void BPatch_module::parseTypes() 
{
   mod->pmod()->mod()->exec()->parseTypesNow();
}
// This is done by analogy with BPatch_module::getVariables,
// not BPatch_image::findVariable.  This should result in consistent
// behavior at the module level.
BPatch_variableExpr* BPatch_module::findVariable(const char* name)
{
   parseTypesIfNecessary();
   const std::vector<int_variable *> &allVars = mod->getAllVariables();

   for (unsigned i = 0; i < allVars.size(); i++) {
     if(strcmp(allVars[i]->symTabName().c_str(), name) == 0)
     {
        return addSpace->findOrCreateVariable(allVars[i]);
     }
   }

   return NULL;
}

bool BPatch_module::getVariables(BPatch_Vector<BPatch_variableExpr *> &vars)
{
   if (!isValid())
      return false;
   if (!full_var_parse) {
      const std::vector<int_variable*> &vars_ = mod->getAllVariables();
      for (unsigned i=0; i<vars_.size(); i++) {
         if (!var_map.count(vars_[i])) {
            addSpace->findOrCreateVariable(vars_[i]);
         }
      }
      full_var_parse = true;
   }      

   BPatch_varMap::iterator i = var_map.begin();
   for (; i != var_map.end(); i++) {
      vars.push_back((*i).second);
   }

   return true;
}


bool BPatch_module::getSourceLines(unsigned long addr, 
      BPatch_Vector< BPatch_statement> &lines) 
{
   if (!isValid()) 
   {
      fprintf(stderr, "%s[%d]:  failed to getSourceLines: invalid\n", FILE__, __LINE__);
      return false;
   }

   unsigned int originalSize = lines.size();
   std::vector<Statement::Ptr> lines_ll;

   Module *stmod = mod->pmod()->mod();
   assert(stmod);

   if (!stmod->getSourceLines(lines_ll, addr - mod->obj()->codeBase()))
   {
      return false;
   }

   for (unsigned int j = 0; j < lines_ll.size(); ++j)
   {
      Statement::ConstPtr t = lines_ll[j];
	   lines.push_back(BPatch_statement(this, t));
   }

   return (lines.size() != originalSize);
} /* end getSourceLines() */

bool BPatch_module::getStatements(BPatch_Vector<BPatch_statement> &statements)
{
	// Iterate over each address range in the line information
	SymtabAPI::Module *stmod = mod->pmod()->mod();
	assert(stmod);
	std::vector<SymtabAPI::Statement::Ptr> statements_ll;

	if (!stmod->getStatements(statements_ll))
	{
		return false;
	}

	for (unsigned int i = 0; i < statements_ll.size(); ++i)
	{
		// Form a BPatch_statement object for this entry
		// Note:  Line information stores offsets, so we need to adjust to
		//  addresses
		SymtabAPI::Statement::ConstPtr stm = statements_ll[i];
		BPatch_statement statement(this, stm);

		// Add this statement
		statements.push_back(statement);

	}
	return true;

}

bool BPatch_module::getAddressRanges(const char *fileName,
                                     unsigned int lineNo, std::vector<SymtabAPI::AddressRange > &ranges)
{
	unsigned int starting_size = ranges.size();

   if (!isValid()) 
   {
      fprintf(stderr, "%s[%d]:  module is not valid\n", FILE__, __LINE__);
      return false;
   }

   if ( fileName == NULL )
   {
	   fileName = mod->fileName().c_str();
   }

   if (!mod->pmod()->mod()->getAddressRanges(ranges, std::string(fileName), lineNo))
   {
	   return false;
   }


   //  Iterate over the returned offset ranges to turn them into addresses
   for (unsigned int i = starting_size; i < ranges.size(); ++i)
   {
	   ranges[i].first += mod->obj()->codeBase();
	   ranges[i].second += mod->obj()->codeBase();
   }

   return true;

} /* end getAddressRanges() */

bool BPatch_module::isSharedLib() 
{
	return mod->obj()->isSharedLib();
}

/*
 * BPatch_module::getBaseAddr
 *
 * Returns the starting address of the module.
 */
void *BPatch_module::getBaseAddr()
{
   return (void *)mod->obj()->codeAbs();
}

/*
 * BPatch_module::getSize
 *
 * Returns the size of the module in bytes.
 */
unsigned long BPatch_module::getSize() 
{
   if (!mod) return 0;
   return (unsigned long) mod->obj()->imageSize();
}


Dyninst::SymtabAPI::Module *Dyninst::SymtabAPI::convert(const BPatch_module *m) {
   if (!m->mod) return NULL;
   return m->mod->pmod()->mod();
}

size_t BPatch_module::getAddressWidth()
{
   if (!mod) return 0;
   return mod->obj()->parse_img()->getObject()->getAddressWidth();
}

void BPatch_module::setDefaultNamespacePrefix(char * /*name*/) 
{ 
}

bool BPatch_module::isSystemLib() 
{
   if (!mod) return false;
   return mod->obj()->isSystemLib(mod->obj()->fullName());
}

AddressSpace *BPatch_module::getAS()
{
   return lladdSpace;
}

BPatch_hybridMode BPatch_module::getHybridMode()
{
    if (!mod || !getAS()->proc()) {
        return BPatch_normalMode;
    }
    return mod->obj()->hybridMode();
}

void BPatch_module::enableDefensiveMode(bool on) {
    mod->obj()->enableDefensiveMode(on);
}

bool BPatch_module::isExploratoryModeOn()
{ 
    if (!mod || !getAS()->proc()) {
        return false;
    }

    BPatch_hybridMode mode = mod->obj()->hybridMode();
    if (BPatch_exploratoryMode == mode || BPatch_defensiveMode == mode) 
        return true;

    return false;
}

/* Protect analyzed code in the module that has been loaded into the 
 * process's address space.  Returns false if failure, true even 
 * if there's no analyzed code in the module and it doesn't
 * actually wind up protecting anything, doesn't trigger analysis
 * in the module
 */ 
bool BPatch_module::setAnalyzedCodeWriteable(bool writeable)
{
    // only implemented for processes and only needed for defensive 
    // BPatch_modules
    if ( !getAS()->proc() || BPatch_defensiveMode != getHybridMode() ) {
        return false;
    }

    // see if we've analyzed code in the module without triggering analysis
    if ( ! lowlevel_mod()->getFuncVectorSize() ) {
        return true;
    }

    // build up list of memory pages that contain analyzed code
    std::set<Address> pageAddrs;
    lowlevel_mod()->getAnalyzedCodePages(pageAddrs);

    // get proc from which we can call changeMemoryProtections
    PCProcess *proc = ((BPatch_process*)addSpace)->lowlevel_process();
    assert(proc);
    if (!proc->isStopped()) {
        if (!proc->stopProcess())
            return false;
    }

    // add protected pages to the mapped_object's hash table, and
    // aggregate adjacent pages into regions and apply protection
    std::set<Address>::iterator piter = pageAddrs.begin();
    int pageSize = getAS()->proc()->getMemoryPageSize();
    while (piter != pageAddrs.end()) {
        Address start, end;
        start = (*piter);
        end = start + pageSize;

        while(1) // extend region if possible
        {
            // add the current page addr to mapped_object's hash table
            // of protected code pages
            if (writeable) {
                mod->obj()->removeProtectedPage( *piter );
            } else {
                mod->obj()->addProtectedPage( *piter );
            }

            piter++;

            if (pageAddrs.end() == piter) {
                break; // last region
            }
            if ( end != (*piter) ) {
                break; // there's a gap, add new region
            }
            // extend current region
            end += pageSize;
        } 

        PCProcess::PCMemPerm newRights(true, false, true);  // PAGE_EXECUTE_READ;
        if (writeable)
            newRights.setW();  // PAGE_EXECUTE_READWRITE;
        
        proc->changeMemoryProtections(start, end - start, newRights, true);
    }
    return true;
}

Address BPatch_module::getLoadAddr()
{
   return mod->obj()->codeBase();
}

BPatch_function *BPatch_module::findFunctionByEntry(Dyninst::Address entry)
{
    BPatch_function* func = addSpace->findFunctionByEntry(entry);
    if (func && func->getModule() == this) {
        return func;
    }

    return NULL;
}

bool BPatch_module::findPoints(Dyninst::Address addr,
                                          std::vector<BPatch_point *> &points) {
   mapped_object *obj = mod->obj();
   block_instance *blk = obj->findOneBlockByAddr(addr);
   if (!blk) return false;

   std::vector<func_instance *> funcs;
   blk->getFuncs(std::back_inserter(funcs));
   for (unsigned i = 0; i < funcs.size(); ++i) {
      // Check module ownership
      if (funcs[i]->mod() != mod) continue;
      BPatch_function *bpfunc = addSpace->findOrCreateBPFunc(funcs[i], this);
      if (!bpfunc) continue;
      instPoint *p = instPoint::preInsn(funcs[i], blk, addr);
      if (!p) continue;
      BPatch_point *pbp = addSpace->findOrCreateBPPoint(bpfunc, p, BPatch_locInstruction);
      if (pbp) points.push_back(pbp);
   }
   return true;
}
