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

    mapped_module* lowlevel_mod() { return mod; }

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
    bool isExploratoryModeOn();
    bool setAnalyzedCodeWriteable(bool writeable);
    bool isSystemLib();
    bool remove(BPatch_function*);
    bool remove(instPoint*);
  
    char * getName(char *buffer, int length);

    char * getFullName(char *buffer, int length);

    const char * libraryName();

    BPatch_object *  getObject();

    size_t getAddressWidth();

    bool getVariables(BPatch_Vector<BPatch_variableExpr *> &vars);

    BPatch_variableExpr* findVariable(const char* name);

    BPatch_Vector<BPatch_function *> * getProcedures(bool incUninstrumentable = false);

               bool  getProcedures(BPatch_Vector<BPatch_function*> &procs, bool incUninstrumentable = false);

    BPatch_Vector<BPatch_function *> * findFunction(const char *name,
                          BPatch_Vector<BPatch_function *> &funcs,
                          bool notify_on_failure =true,
                          bool regex_case_sensitive =true,
                          bool incUninstrumentable =false,
                          bool dont_use_regex = false);


    BPatch_function * findFunctionByEntry(Dyninst::Address entry);


    BPatch_Vector<BPatch_function *> * 
    findFunctionByAddress(void *addr,
                           BPatch_Vector<BPatch_function *> &funcs,
                           bool notify_on_failure = true,
                           bool incUninstrumentable = false);

    BPatch_typeCollection *getModuleTypes();
    
 
    BPatch_function * findFunctionByMangled(const char * mangled_name,
                                             bool incUninstrumentable=false);


    bool  findPoints(Dyninst::Address addr, std::vector<BPatch_point *> &points);


    bool dumpMangled(char *prefix);

    bool isSharedLib();


    bool getAddressRanges(const char *fileName, unsigned int lineNo,
                          std::vector<Dyninst::SymtabAPI::AddressRange > &ranges);
    
    bool getSourceLines( unsigned long addr, BPatch_Vector<BPatch_statement> &lines);


    bool getStatements(BPatch_Vector<BPatch_statement> &statements);

    void * getBaseAddr(void);

    Dyninst::Address getLoadAddr(void);

    unsigned long getSize(void);

    bool isValid();

    BPatch_hybridMode getHybridMode();

    void enableDefensiveMode(bool on);
    
private:
    bool parseTypesIfNecessary();
    BPatch_typeCollection *moduleTypes;

    void parseDwarfTypes();

   BPatch_funcMap func_map;
   BPatch_instpMap instp_map;
   BPatch_varMap var_map;
   
   bool full_func_parse;
   bool full_var_parse;
};

#endif /* _BPatch_module_h_ */
