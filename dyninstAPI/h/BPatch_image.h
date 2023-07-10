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

#ifndef _BPatch_image_h_
#define _BPatch_image_h_

#include "BPatch_dll.h"
#include "BPatch_sourceObj.h"
#include "BPatch_Vector.h"
#include "BPatch_module.h"
#include "BPatch_type.h"
#include "BPatch_process.h"
#include "BPatch_binaryEdit.h"
#include "BPatch_parRegion.h"
#include "dyntypes.h"

#include <string>
#include <utility>
#include <vector>
#include <map>

typedef bool (*BPatchFunctionNameSieve)(const char *test,void *data);
class image;
class int_variable;
class BPatch_point;
class BPatch_object;

class BPatch_statement;
class BPatch_image;
class BPatch_object_getMod;

namespace Dyninst {
	namespace SymtabAPI {
		struct AddressRange;
	}
  namespace PatchAPI {
    class PatchMgr;
    typedef boost::shared_ptr<PatchMgr> PatchMgrPtr;
    BPATCH_DLL_EXPORT PatchMgrPtr convert(const BPatch_image *);
  }
}


class BPATCH_DLL_EXPORT BPatch_image: public BPatch_sourceObj {
  friend class BPatch; // registerLoaded... callbacks
  friend class BPatch_module; // access to findOrCreate...
  friend class BPatch_object; // Also access to findOrCreate
  friend class BPatch_object_getMod;
  friend class BPatch_process;
  friend class BPatch_addressSpace;
  friend class BPatch_binaryEdit;
  friend Dyninst::PatchAPI::PatchMgrPtr Dyninst::PatchAPI::convert(const BPatch_image *);

  BPatch_variableExpr *findOrCreateVariable(int_variable *);
 public:

  // The following functions are for internal use by  the library only:
  // As such, these functions are not locked.
  //BPatch_image(BPatch_process *_proc);
  BPatch_image(BPatch_addressSpace *addSpace);
  BPatch_image();
  BPatch_module *findModule(mapped_module *base);
  BPatch_object *findObject(mapped_object *base);
  virtual ~BPatch_image();
  void getNewCodeRegions
  (std::vector<BPatch_function*>&newFuncs, 
   std::vector<BPatch_function*>&modFuncs);
  void clearNewCodeRegions();
  // End functions for internal use only

  //  BPatch_image::getThr
  //  
  //  return the BPatch_thread associated with this image
  BPatch_thread * getThr();


  // BPatch_image::getAddressSpace()
  //
  //  return the BPatch_addressSpace associated with this image
  BPatch_addressSpace * getAddressSpace();
    

  //  BPatch_image::getProcess
  //  
  //  return the BPatch_process associated with this image
  BPatch_process * getProcess();


  //  BPatch_image::getSourceObj
  //  
  //  fill a vector with children source objects (modules)

  bool getSourceObj(BPatch_Vector<BPatch_sourceObj *> &sources);

  //  BPatch_image::getObjParent
  //  
  //  Return the parent of this image (always NULL since this is the top level)

  BPatch_sourceObj * getObjParent();

  //  BPatch_image::getVariables
  //  
  //  Returns the global variables defined in this image

  bool getVariables(BPatch_Vector<BPatch_variableExpr *> &vars);

  //  BPatch_image::getProcedures
  //  
  //  Returns a list of all procedures in the image upon success,
  //  NULL upon failure
  BPatch_Vector<BPatch_function *> * getProcedures(bool incUninstrumentable = false);
    
  bool getProcedures(BPatch_Vector<BPatch_function*> &procs, bool incUninstrumentable = false);

  //  BPatch_image::getParRegions
  //  
  //  Returns a list of all procedures in the image upon success,
  //  NULL upon failure

  BPatch_Vector<BPatch_parRegion *> * 
  getParRegions(bool incUninstrumentable = false);

  //  BPatch_image::getModules
  //  
  //  Returns a vector of all modules in this image
  BPatch_Vector<BPatch_module *> * getModules();

  //  BPatch_image::getObjects
  //  
  //  Returns a vector of all objects in this image
  void getObjects(std::vector<BPatch_object *> &objs);

  bool getModules(BPatch_Vector<BPatch_module*> &mods);

  //  BPatch_image::findModule
  //  
  //  Returns a module matching <name> if present in image, NULL if not found
  //  if <substring_match> is set, the first module that has <name> as a substring
  //  of its name is returned (eg, to find "libpthread.so.1", search for "libpthread" 
  //  with substring_match set to true)

  BPatch_module * findModule(const char *name, bool substring_match = false);

  //  BPatch_image::getGlobalVariables
  //  
  //  Returns the global variables defined in this image

  BPatch_Vector<BPatch_variableExpr *> * getGlobalVariables();

  //  BPatch_image::findFunction
  //  
  //  Returns a vector of functions matching <name>, if <name> is a regular
  //  expression, a (slower) regex search will be performed.  
  //  Returns NULL on failure.

  BPatch_Vector<BPatch_function*> * findFunction(const char *name,
						 BPatch_Vector<BPatch_function*> &funcs, 
						 bool showError=true,
						 bool regex_case_sensitive=true,
						 bool incUninstrumentable = false);
                                                    
  //  BPatch_image::findFunction
  //  
  //  Returns a vector of functions matching criterion specified by user defined
  //  callback function bpsieve.

  BPatch_Vector<BPatch_function *> * 
  findFunction(BPatch_Vector<BPatch_function *> &funcs,
	       BPatchFunctionNameSieve bpsieve,
	       void *user_data=NULL,
	       int showError=0,
	       bool incUninstrumentable = false);

  //  BPatch_image::findFunction(Address)
  //
  //  Returns a function at a specified address
  BPatch_function *  findFunction(unsigned long addr);

  bool  findFunction(Dyninst::Address addr, 
		     BPatch_Vector<BPatch_function *> &funcs);

  //  BPatch_image::findVariable
  //  
  //  Returns global variable matching <name> in the image.  NULL if not found.

  BPatch_variableExpr * findVariable(const char *name, bool showError=true);

  //  BPatch_image::findVariable
  //  
  //  Returns local variable matching name <nm> in function scope of 
  //  provided BPatch_point.

  BPatch_variableExpr * findVariable(BPatch_point &scp, const char *nm, bool showError=true);

  //  BPatch_image::findType
  //  
  //  Returns a BPatch_type corresponding to <name>, if exists, NULL if not found

  BPatch_type * findType(const char *name);

  //  BPatch_image::findPoints
  //
  //  Returns a vector of BPatch_points that correspond with the provided address, one
  //  per function that includes an instruction at that address. Will have one element
  //  if there is not overlapping code. 
  bool  findPoints(Dyninst::Address addr, std::vector<BPatch_point *> &points);

  //  BPatch_image::getAddressRanges
  //  
  //  method to retrieve addresses corresponding to a line in a file

  bool getAddressRanges( const char * fileName, unsigned int lineNo, 
			 std::vector<Dyninst::SymtabAPI::AddressRange > & ranges );
    
  bool getSourceLines( unsigned long addr, BPatch_Vector<BPatch_statement> & lines );

  typedef std::vector<std::pair<unsigned long, unsigned long> >::iterator arange_iter;
  typedef BPatch_Vector<BPatch_statement>::iterator statement_iter;
  arange_iter getAddressRanges_begin(const char* fileName, unsigned int lineNo);
  arange_iter getAddressRanges_end(const char* fileName, unsigned int lineNo);
  statement_iter getSourceLines_begin(unsigned long addr);
  statement_iter getSourceLines_end(unsigned long addr);

  //  BPatch_image::getProgramName
  //  
  //  fills provided buffer <name> with the program's name, up to <len> chars

  char * getProgramName(char *name, unsigned int len);

  //  BPatch_image::getProgramFileName
  //  
  //  fills provided buffer <name> with the program's file name, 
  //  which may include path information.

  char * getProgramFileName(char *name, unsigned int len);

  /* BPatch_image::parseNewFunctions
   *
   * This function uses function entry addresses to find and parse
   * new functions using our control-flow traversal parsing. 
   *
   * funcEntryAddrs: this is a vector of function start addresses
   * that seed the control-flow-traversal parsing.  If they lie in
   * an existing module they are parsed in that module, otherwise a
   * new module is created.  In both cases the modules are added to
   * affectedModules
   *
   * affectedModules: BPatch_modules will be added to this vector if no
   * existing modules bounded the specified function entry points.
   * Unfortunately, new modules will also sometimes have to be created
   * for dynamically created code in memory that does not map to the
   * file version of the binary.  
   *
   * Return value: This value is true if a new module was created or if
   * new code was parsed in an existing module
   */
  bool parseNewFunctions(BPatch_Vector<BPatch_module*> &affectedModules, 
			 const BPatch_Vector<Dyninst::Address> &funcEntryAddrs);

  //
  //  Reads a string from the target process
  bool  readString(Dyninst::Address addr, std::string &str, 
		   unsigned size_limit = 0);

  bool  readString(BPatch_variableExpr *expr, std::string &str, 
		   unsigned size_limit = 0);

 private:
  BPatch_addressSpace *addSpace;
  BPatch_module *defaultModule;

  BPatch_module *findOrCreateModule(mapped_module *base);
  BPatch_object *findOrCreateObject(mapped_object *base);
  void removeModule(BPatch_module *mod);
  void removeObject(BPatch_object *obj);
  void removeAllModules();

  typedef std::map<mapped_module *,  BPatch_module *> ModMap;
  typedef std::map<mapped_object *,  BPatch_object *> ObjMap;

  ModMap modmap;
  ObjMap objmap;

  // Annoying backwards-compatible return type
  std::vector<BPatch_module *> modlist;

  BPatch_Vector<BPatch_module *> removed_list;
  BPatch_Vector<BPatch_point *> unresolvedCF;

  // These private "find" functions convert from internal func_instance
  // representation to the exported BPatch_Function type
  void findFunctionInImage(const char *name, image *img,
			   BPatch_Vector<BPatch_function*> *funcs,
			   bool incUninstrumentable = false);
  void sieveFunctionsInImage(image *img, 
			     BPatch_Vector<BPatch_function *> *funcs,
			     BPatchFunctionNameSieve bpsieve, 
			     void *user_data,
			     bool incUninstrumentable = false);

  static bool setFuncModulesCallback(BPatch_function *bpf, void *data);
};

#endif /* _BPatch_image_h_ */
