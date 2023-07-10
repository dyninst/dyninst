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

#ifndef _BPatch_module_h_
#define _BPatch_module_h_
#include "BPatch_dll.h"
#include "BPatch_Vector.h"
#include "BPatch_sourceObj.h"
#include "BPatch_enums.h"
#include "dyntypes.h"
#include <stddef.h>
#include <vector>
#include <map>

class mapped_module;

class BPatch_image;
class BPatch_function;
class BPatch_typeCollection;
class BPatch_builtInTypeCollection;
class BPatch_addressSpace;
class BPatch_process;
class BPatch_statement;
class func_instance;
class int_variable;
class instPoint;
class AddressSpace;
class BPatch_snippet;
class BPatchSnippetHandle;
class BPatch_module;
class BPatch_object;

namespace Dyninst { 
   namespace SymtabAPI {
      class Module;
       struct AddressRange;
      BPATCH_DLL_EXPORT Module *convert(const BPatch_module *);
   }
   namespace PatchAPI {
	   class PatchFunction;
	   class Point;
   }
}

extern BPatch_builtInTypeCollection * builtInTypes;


class BPATCH_DLL_EXPORT BPatch_module: public BPatch_sourceObj{

    friend class BPatch_function;
    friend class BPatch_flowGraph;
    friend class BPatch_image;
    friend class BPatch_thread;
    friend class BPatch_process;
    friend class BPatch_binaryEdit;
    friend class BPatch_addressSpace;
    friend class BPatch_statement;
    friend Dyninst::SymtabAPI::Module *Dyninst::SymtabAPI::convert(const BPatch_module *);


    typedef std::map<Dyninst::PatchAPI::PatchFunction*, 
       BPatch_function*> BPatch_funcMap;
    typedef std::map<int_variable*, BPatch_variableExpr*> BPatch_varMap;
    typedef std::map<Dyninst::PatchAPI::Point *, 
       BPatch_point*> BPatch_instpMap;


    
    BPatch_addressSpace *addSpace;
    AddressSpace *lladdSpace;
    mapped_module      	 *mod;
    BPatch_image	 *img;
    AddressSpace *getAS();

public:

    //  This function should go away when paradyn is on top of dyninst
    mapped_module* lowlevel_mod() { return mod; }

    // The following functions are for internal use by  the library only:
    BPatch_module(BPatch_addressSpace *_addSpace,
                  AddressSpace *as,
                  mapped_module *_mod, 
                  BPatch_image *img);
    virtual ~BPatch_module();
    bool getSourceObj(BPatch_Vector<BPatch_sourceObj *>&);
    BPatch_sourceObj *getObjParent();
    void parseTypes();
    void setDefaultNamespacePrefix(char *name);    
    void handleUnload();
    bool isExploratoryModeOn();// true if exploratory or defensive mode is on
    bool setAnalyzedCodeWriteable(bool writeable);//sets write perm's analyzed code pages
    bool isSystemLib();
    bool remove(BPatch_function*);
    bool remove(instPoint*);
    // End functions for internal use only
  
    // BPatch_module::getName
    // Returns file name associated with module

    char * getName(char *buffer, int length);

    // BPatch_module::getFullName
    // Returns full path name of module, when available

    char * getFullName(char *buffer, int length);

    // BPatch_module::libraryName
    // Returns name if library, if this module is a shared object

    const char * libraryName();

    // BPatch_module::getObject
    // Returns BPatch_object containing this file
    BPatch_object *  getObject();

    // BPatch_module::getAddressWidth
    // Returns the width (in bytes) of an address in this module

    size_t getAddressWidth();

    // BPatch_module::getVariables
    // Fills a vector with the global variables that are specified in this module

    bool getVariables(BPatch_Vector<BPatch_variableExpr *> &vars);

    // BPatch_module::findVariable
    // Find and return a global variable (NULL if not found)

    BPatch_variableExpr* findVariable(const char* name);

	// BPatch_module::getProcedures
    // Returns a vector of all functions in this module
    BPatch_Vector<BPatch_function *> * getProcedures(bool incUninstrumentable = false);

               bool  getProcedures(BPatch_Vector<BPatch_function*> &procs, bool incUninstrumentable = false);

    // BPatch_module::findFunction
    // Returns a vector of BPatch_function *  matching specified <name>

    BPatch_Vector<BPatch_function *> * findFunction(const char *name,
                          BPatch_Vector<BPatch_function *> &funcs,
                          bool notify_on_failure =true,
                          bool regex_case_sensitive =true,
                          bool incUninstrumentable =false,
                          bool dont_use_regex = false);


    //  BPatch_addressSpace::findFunctionByEntry
    //  Returns the function starting at the given address
    BPatch_function * findFunctionByEntry(Dyninst::Address entry);


    // FIXME: This method is (undocumented) 

    BPatch_Vector<BPatch_function *> * 
    findFunctionByAddress(void *addr,
                           BPatch_Vector<BPatch_function *> &funcs,
                           bool notify_on_failure = true,
                           bool incUninstrumentable = false);

    // get the module types member (instead of directly accessing)
    BPatch_typeCollection *getModuleTypes();
    
 
    // BPatch_module::findFunctionByMangled
    // Returns a function, if it exits, that matches the provided mangled name

    BPatch_function * findFunctionByMangled(const char * mangled_name,
                                             bool incUninstrumentable=false);


    //  BPatch_module::findPoints
    //
    //  Returns a vector of BPatch_points that correspond with the provided address, one
    //  per function that includes an instruction at that address. Will have one element
    //  if there is not overlapping code. 
    bool  findPoints(Dyninst::Address addr, std::vector<BPatch_point *> &points);


    // BPatch_module::dumpMangled
    // Prints all <mangled> function names in this module

    bool dumpMangled(char *prefix);

    // BPatch_module::isSharedLib
    // Returns true if this module represents a shared library

    bool isSharedLib();


    // BPatch_module::getAddressRanges
    // 
    // function to get addresses for a line of the module
    // if fileName is NULL, uses the name of the module

    bool getAddressRanges(const char *fileName, unsigned int lineNo,
                          std::vector<Dyninst::SymtabAPI::AddressRange > &ranges);
    
    // BPatch_module::getSourceLines
    //
    // function to get source file names and lines
    // for an address in the module
    
    bool getSourceLines( unsigned long addr, BPatch_Vector<BPatch_statement> &lines);


    // BPatch_mode::getStatements
    //
    // Fill supplied vector with all BPatch_statements from this module
  
    bool getStatements(BPatch_Vector<BPatch_statement> &statements);

    // BPatch_module::wgetBaseAddr
    // Returns a base address of the module; defined as the start
    // of the first function.
    void * getBaseAddr(void);

    Dyninst::Address getLoadAddr(void);

    // BPatch_module::getSize
    // Returns the size of the module; defined as the end of the last
    // function minus the start of the first function.
    unsigned long getSize(void);

    bool isValid();

    // BPastch_module::getHybridMode
    // returns the hybrid Analysis mode: normal, exploratory, defensive
    BPatch_hybridMode getHybridMode();

    void enableDefensiveMode(bool on);
    
private:
    // Parse wrapper
    bool parseTypesIfNecessary();
    BPatch_typeCollection *moduleTypes;

    // We understand the type information in DWARF format.
    void parseDwarfTypes();

   BPatch_funcMap func_map;
   BPatch_instpMap instp_map;
   BPatch_varMap var_map;
   
   bool full_func_parse;
   bool full_var_parse;
};

#endif /* _BPatch_module_h_ */
