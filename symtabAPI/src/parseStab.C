/*
 * Copyright (c) 1996-2007 Barton P. Miller
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <ctype.h>
// from libiberty's demangle.h
#define DMGL_PARAMS   (1 << 0) 
#define DMGL_ANSI     (1 << 1) 
#define DMGL_VERBOSE  (1 << 3) 

#include "util.h"
#include "Symtab.h" // For looking up compiler type
#include "Symbol.h" 
#include "symtabAPI/src/Collections.h"
#include "common/h/String.h"

using namespace Dyninst;
using namespace Dyninst::SymtabAPI;

/*
#include "BPatch.h"
#include "debug.h"
*/

extern std::string current_func_name;
extern std::string current_mangled_func_name;
extern Symbol *current_func;

// Forward references for parsing routines
static int parseSymDesc(char *stabstr, int &cnt);
static Type *parseConstantUse(Module *, char *stabstr, int &cnt);
static char *parseTypeDef(Module *, char *stabstr, 
                          const char *name, int ID, unsigned int sizeHint = 0);
static int parseTypeUse(Module*, char *&stabstr, int &cnt,
                        const char *name);
static inline bool isSymId(char ch);
static std::string getIdentifier(char *stabstr, int &cnt, bool stopOnSpace=false);

static std::string currentRawSymbolName;

std::string convertCharToString(const char *ptr){
    if(ptr)
        return ptr;
    else
        return "";
}

//
// Start of code to parse Stab information.
//    The structure of this code is a recursive decent parser that parses
//    information in stab records and builds up the corresponding BPatch_types.
//    
//    Each non-terminal in the grammer has a function of the form parse<NT>.
//	
//    The grammar for a non-terminal appears in the comments just before
//    the non-terminal parsing function
//	

void vectorNameMatchKLUDGE(Module *mod, char *demangled_sym, std::vector<Symbol *> &bpfv, std::vector<int> &matches)
{
  // iterate through all matches and demangle names with extra parameters, compare
  for (unsigned int i = 0; i < bpfv.size(); ++i) {
    std::string l_mangled;
    l_mangled = bpfv[i]->getName();
    
    char * l_demangled_raw = P_cplus_demangle(l_mangled.c_str(), mod->exec()->isNativeCompiler());
    if( l_demangled_raw == NULL ) {
    	l_demangled_raw = strdup(l_mangled.c_str());
    }
    
    if (!strcmp(l_demangled_raw, demangled_sym)) {
       matches.push_back(i);
    }
    free(l_demangled_raw);
  } /* end iteration over function vector */
}

Symbol *mangledNameMatchKLUDGE(const char *pretty, const char *mangled, 
					Module *mod)
{

  std::vector<Symbol *> bpfv;
  if (!mod->findSymbolByType(bpfv, pretty, Symbol::ST_FUNCTION)) {
     //cerr << __FILE__ << __LINE__ << ":  KLUDGE Cannot find " << pretty << endl;
     return NULL;  // no pretty name hits, expecting multiple
  }

  //cerr << __FILE__ << __LINE__ << ":  mangledNameMatchKLUDGE: language = " 
  //<< mod->getLanguageStr() << endl;

  if (lang_Fortran_with_pretty_debug == mod->language()) {
      // debug function symbols are presented in "demangled" style.
      if (bpfv.size() == 1)
	return bpfv[0];
      else {
	cerr << __FILE__ << __LINE__ << ":  FIXME!" << endl;
	return NULL;
      }
    }

  // demangle name with extra parameters
  char * demangled_sym = P_cplus_demangle( mangled, mod->exec()->isNativeCompiler(), true );
  if( demangled_sym == NULL ) {
  	demangled_sym = strdup( mangled );
  	assert( demangled_sym != NULL );
  }

  std::vector<int> matches;

  vectorNameMatchKLUDGE(mod, demangled_sym, bpfv, matches);

  Symbol *ret = NULL;

  if (matches.size() == 1) {ret = bpfv[matches[0]]; goto clean_up;}
  if (matches.size() > 1) goto clean_up;
  
  // check in the uninstrumentable pile
  bpfv.clear();
  matches.clear();

  vectorNameMatchKLUDGE(mod, demangled_sym, bpfv, matches);
  if (matches.size() == 1) {ret = bpfv[matches[0]]; goto clean_up;}
  if (matches.size() > 1) goto clean_up;

 clean_up:
  free( demangled_sym );
  return ret;
}

// This function takes the stab stabstring and parses it to create a new 
// type or variable object.  This function only defines the type/variable 
// name and ID.
//
// <stabString> = <ident>:<symDesc> 			|
//		  <ident>:c<constantUse>		|
//		  <ident>:f<symDesc> 			|
//		  <ident>:f<syymDesc>,<ident>,<ident> 	|
//		  <ident>:F<typeUse><paramList> 	|
//		  <ident>:G<typeUse> 			|
//		  <ident>:r<int> 			|
//		  <ident>:S<typeUse>			|
//		  <ident>:[pPr]<typeUse>		|
//		  <ident>::T<typeUse>			|
//		  <ident>:t<typeUse>			|
//		  <ident>:T<typeUse>			|
//		  <ident>:v<typeUse>			|
//		  <ident>:V<symDesc>			|
//		  <indet>:Y[Tc|Ts]
//
// <paramList> = | <typeUse>;<paramList> 
//
std::string SymtabAPI::parseStabString(Module *mod, int linenum, char *stabstr, 
		      int framePtr, typeCommon *commonBlock = NULL)
{
    
    int cnt;
    int ID = 0;
    int symdescID = 0;
    int funcReturnID = 0;
    Symbol  *fp = NULL;
    Type * ptrType = NULL;
    Type * newType = NULL; // For new types to add to the collection
    localVar *locVar = NULL;
    cnt= 0;
    std::string fName = mod->fileName();

    /* get type or variable name */
    std::string mangledname = getIdentifier( stabstr, cnt );

    currentRawSymbolName = mangledname;
    char * demangled = P_cplus_demangle( mangledname.c_str(), mod->exec()->isNativeCompiler() );
    std::string name;
    if( demangled == NULL ) {
    	name = mangledname;
    } else {
       name = demangled;
    }

    if( name[0] != '\0' && stabstr[cnt] != ':' ) {
		return name;
		}
    
    if (stabstr[cnt] == ':') {
       // skip to type part
       cnt++;
    }

    if (isSymId(stabstr[cnt])) {
	/* instance of a predefined type */

	ID = parseSymDesc(stabstr, cnt);
	
	////bperr("Variable: %s  Type ID: %d, file ID %d \n",name, ID, file_ID);
	if (stabstr[cnt] == '=') {
	  /* More Stuff to parse, call parseTypeDef */
	  stabstr = parseTypeDef(mod, (&stabstr[cnt+1]), name.c_str(), ID);
	  cnt = 0;
	  ptrType = mod->getModuleTypes()->findOrCreateType(ID);
	  if (!current_func) {
	      // XXX-may want to use N_LBRAC and N_RBRAC to set function scope 
	      // -- jdd 5/13/99
	      // Still need to add to local variable list if in a function

	      std::string modName = mod->fileName();
	      //bperr("%s[%d] Can't find function %s in module %s\n", __FILE__, __LINE__,
		//     current_mangled_func_name.c_str(), modName);
	      //bperr("Unable to add %s to local variable list in %s\n",
		//     name.c_str(),current_func_name.c_str());
	  } else {
	      locVar = new localVar(name, ptrType, fName, linenum);
	      loc_t *loc = (loc_t *)malloc(sizeof(loc_t));
	      loc->stClass = storageRegOffset;
	      loc->refClass = storageNoRef;
	      loc->frameOffset = framePtr;
	      loc->reg = -1;
	      locVar->addLocation(loc);
	      if (!ptrType) {
		//bperr("adding local var with missing type %s, type = %d\n",
		//      name, ID);
	      }
	      if(!current_func->vars_)
	          current_func->vars_ = new localVarCollection();
	      current_func->vars_->addLocalVar( locVar);
	  }
	} else if (current_func) {
	  // Try to find the BPatch_Function
	  ptrType = mod->getModuleTypes()->findOrCreateType( ID);
	  
	  locVar = new localVar(name, ptrType, fName, linenum);
	  loc_t *loc = (loc_t *)malloc(sizeof(loc_t));
	  loc->stClass = storageRegOffset;
	  loc->refClass = storageNoRef;
	  loc->frameOffset = framePtr;
	  loc->reg = -1;
	  locVar->addLocation(loc);
	  
	  if (!ptrType) {
	    ////bperr("adding local var with missing type %s, type = %d\n",
	    //	     name, ID);
	  }
	  if(!current_func->vars_)
	      current_func->vars_ = new localVarCollection();
	  current_func->vars_->addLocalVar( locVar);
	}
    } else if (stabstr[cnt]) {
      std::vector<Symbol *> bpfv;
      switch (stabstr[cnt]) {
	    case 'f': /*Local Function*/ {
	      std::string scopeName;
	      std::string lfuncName;
	      cnt++;

	      current_func_name = name;
	      current_mangled_func_name = mangledname;

	      funcReturnID = parseTypeUse(mod, stabstr, cnt, name.c_str());
      
	      if (stabstr[cnt]==',') {
		  cnt++; 	/*skip the comma*/

		  /* Local Function Name */
		  lfuncName = getIdentifier(stabstr, cnt);

		  assert(stabstr[cnt] == ',');
		  cnt++; 	/*skip the comma*/

		  /* Scope Name of Local Function */
		  scopeName = getIdentifier(stabstr, cnt);

		  if (stabstr[cnt]) {
		      //bperr("Extra: %s\n", &stabstr[cnt]);
		  }
	      }

	      if (!scopeName.length()) { // Not an embeded function
		  ptrType = mod->getModuleTypes()->findOrCreateType(funcReturnID);
		  if( !ptrType) ptrType = mod->exec()->type_Untyped;

		  if (!(mod->findSymbolByType( bpfv, name, Symbol::ST_FUNCTION ))) {
		    if(!(mod->findSymbolByType( bpfv, name, Symbol::ST_FUNCTION, true ))) {
		       //showInfoCallback(string("missing local function ") +
		 	//					     name + "\n");
                       // It's very possible that we might not find a function
                       // that's a weak reference, and defined in multiple places
                       // as we only store an object from the last definition
                      fp = NULL;
		   }   
		 } else {
		    if (bpfv.size() > 1) {
		      // warn if we find more than one function with current_func_name
                      char msg[1024];
                      sprintf(msg, "%s[%d]:  found %d functions with name %s, using the first",
                              __FILE__, __LINE__, (int)bpfv.size(), name.c_str());
                     // BPatch::bpatch->reportError(BPatchWarning, 0, msg);

                    }else if (!bpfv.size()) {
                      //bperr("%s[%d]:  SERIOUS: found 0 functions with name %s",
                      //       __FILE__, __LINE__, name.c_str());
                      break;
                    }
		    fp = bpfv[0];
		    // set return type.
		    fp->setReturnType(ptrType);
		  }
	      } else {
		  //bperr("%s is an embedded function in %s\n",name.c_str(), scopeName.c_str());
	      }

              current_func = fp;
	      // skip to end - SunPro Compilers output extra info here - jkh 6/9/3
	      cnt = strlen(stabstr);

	      break;
	  }  

	  case 'F':/* Global Function */
	  {
	      cnt++; /*skipping 'F' */

	      funcReturnID = parseTypeUse(mod, stabstr, cnt, name.c_str());

	      current_func_name = name;
	      current_mangled_func_name = mangledname;

	      //
	      // For SunPro compilers there may be a parameter list after 
	      //   the return
	      //
	      while (stabstr[cnt] == ';') {
		  cnt++;	// skip ';'
		  (void) parseTypeUse(mod, stabstr, cnt, "");
	      }

	      // skip to end - SunPro Compilers output extra info here - jkh 6/9/3
	      cnt = strlen(stabstr);

	      ptrType = mod->getModuleTypes()->findOrCreateType(funcReturnID);
	      if (!ptrType) ptrType = mod->exec()->type_Untyped;
		
	      std::vector<Symbol *>fpv;
	      if(!mod->findSymbolByType(fpv, current_mangled_func_name, Symbol::ST_FUNCTION, true)) {
		std::string modName = mod->fileName();
		
		if (NULL == (fp = mangledNameMatchKLUDGE(current_func_name.c_str(), 
							 current_mangled_func_name.c_str(), mod))){
                   //bpwarn("%s L%d - Cannot find global function with mangled name '%s' or pretty name '%s' with return type '%s' in module '%s', possibly extern\n",
                   //       __FILE__, __LINE__,
                   //       current_mangled_func_name.c_str(), current_func_name.c_str(),
                   //       ((ptrType->getName() == NULL) ? "" : ptrType->getName()), 
                   //       modName);
                   //char prefix[5];
                   //strncpy(prefix, current_mangled_func_name, 4);
                   //prefix[4] = '\0';
                   // mod->dumpMangled(prefix);
                   break;
		}
	      }
	      fp = fpv[0];

	      fp->setReturnType(ptrType);
	      current_func = fp;
	      fpv.clear();
	  }    
	      break;
	
	  case 'U':/* Class Declaration - for Sun Compilers - jkh 6/6/03 */
	  case 'E':/* Extern'd Global ??? - undocumented type for Sun Compilers - jkh 6/6/03 */
	  case 'G':/* Global Varaible */
	      cnt++; /* skip the 'G' */

	      /* Get variable type number */
	      symdescID = parseTypeUse(mod, stabstr, cnt, name.c_str());
	      if (stabstr[cnt]) {
		  //bperr( "\tMore to parse - global var %s\n", &stabstr[cnt]);
		  //bperr( "\tFull String: %s\n", stabstr);
	      }
	      // lookup symbol and set type
	      Type *BPtype;
      
	      BPtype = mod->getModuleTypes()->findOrCreateType(symdescID);
	      if (!BPtype) {
		      //bperr("ERROR: unable to find type #%d for variable %s\n", 
		       //symdescID, name.c_str());
	      } else {
		  /** XXXX - should add local too here **/
                  //fprintf(stderr, "%s[%d]:  found global variable: %s\n", FILE__, __LINE__, name.c_str());
		  mod->getModuleTypes()->addGlobalVariable(name, BPtype);
	      }
	      break;

	  case 'P':	// function parameter passed in a register (GNU/Solaris)
	  case 'R':	// function parameter passed in a register (AIX style)
	  case 'v':	// Fortran Local Variable
	  case 'X':	// Fortran function return Variable (e.g. function name)
          case 'p': {	// Function Parameter
	      cnt++; /* skip the 'p' */

	      /* Get variable type number */
	      symdescID = parseTypeUse(mod, stabstr, cnt, name.c_str());

	      if (stabstr[cnt] == ';') {
		  // parameter type information, not used for now
		  cnt = strlen(stabstr);
	      } else if (stabstr[cnt]) {
		  //bperr( "\tMore to parse func param %s\n", &stabstr[cnt]);
		  //bperr( "\tFull String: %s\n", stabstr);
	      }

	      ptrType = mod->getModuleTypes()->findOrCreateType(symdescID);
	      if (!ptrType) ptrType = mod->exec()->type_Untyped;

	      localVar *param;
	      
	      param = new localVar(name, ptrType, fName, linenum);
	      loc_t *loc = (loc_t *)malloc(sizeof(loc_t));
	      loc->stClass = storageRegOffset;
	      loc->refClass = storageNoRef;
	      loc->frameOffset = framePtr;
	      loc->reg = -1;
	      param->addLocation(loc);
      
	      if (current_func) {
	  	  if(!current_func->params_)
	      	      current_func->params_ = new localVarCollection();
		  current_func->params_->addLocalVar(param);
	      } else {
		//showInfoCallback(string("parameter without local function ") 
		//	 + string(stabstr) + "\n");
	      }
	      break;
	  }

	  case 'c': /* constant */
	  {
	      cnt++; /*move past the 'c' */

	      ptrType = parseConstantUse(mod, stabstr, cnt);
	      if (stabstr[cnt])
		//bperr("Parsing Error More constant info to Parse!!: %s\n",
		//    &(stabstr[cnt]));

	      if (!ptrType) ptrType = mod->exec()->type_Untyped;

	      localVar *var;
	      var = new localVar(name, ptrType, fName, linenum);
	      loc_t *loc = (loc_t *)malloc(sizeof(loc_t));
	      loc->stClass = storageRegOffset;
	      loc->refClass = storageNoRef;
	      loc->frameOffset = 0;
	      loc->reg = -1;
	      var->addLocation(loc);
      
	      if (current_mangled_func_name.length()) {
	        std::vector<Symbol *>fpv;
		if(!mod->findSymbolByType(fpv, current_mangled_func_name, Symbol::ST_FUNCTION, true)){
		  //showInfoCallback(string("missing local function ") + 
		//		   current_func_name + "\n");
		} else { // found function, add parameter
	          fp = fpv[0];	
		  current_func = fp;
		  if(!fp->params_)
		  	fp->params_= new localVarCollection();
		  fp->params_->addLocalVar(var);
		}
		fpv.clear();
	      } else {
		//showInfoCallback(std::string("parameter without local function ") 
		//+ std::string(stabstr));
	      }
	  }
	      break;

	  case 'r':/* Register Variable */
	      cnt++; /*move past the 'r'*/
	      /* get type reference */

	      symdescID = parseSymDesc(stabstr, cnt);

#ifdef notdef
	      /* often have rNN=*yy - what is this ? jkh 11/30/00 */
	      if (stabstr[cnt])
		//bperr("Parsing Error More register info to Parse!!: %s\n",
		    &(stabstr[cnt]));
#endif
	      break;

	  case 'S':/* Global Static Variable */ {
	      cnt++; /*move past the 'S'*/

	      /* get type reference */
	      symdescID = parseTypeUse(mod, stabstr, cnt, name.c_str());

	      // lookup symbol and set type
	      Type *BPtype;

              std::string nameTrailer;
	      if (name.find(".") < name.length()) {
		  std::string defaultNameSpace;

                  defaultNameSpace = name.substr(0,name.find("."));
                  nameTrailer = name.substr(name.find(".")+1,name.length()-name.find(".")-1);
		  //TODO?? set it in symtabAPI and use it for finding symbols(findSymbolByType)
//		  mod->setDefaultNamespacePrefix(strdup(defaultNameSpace.c_str()));
	      } else
                 nameTrailer = name;

	      BPtype = mod->getModuleTypes()->findOrCreateType(symdescID);
	      if (!BPtype) {
		      //bperr("ERROR: unable to find type #%d for variable %s\n", 
		      // symdescID, nameTrailer.c_str());
	      } else {
		  Symtab *img = mod->exec();
		  std::vector<Symbol *>syms;
		  if(img->findSymbolByType(syms, nameTrailer, Symbol::ST_OBJECT) || img->findSymbolByType(syms, nameTrailer, Symbol::ST_OBJECT, true)) {
		      syms.clear();	
		      mod->getModuleTypes()->addGlobalVariable(nameTrailer, BPtype);
		  }
	      }

	      if (stabstr[cnt]);
		//bperr("Parsing Error More Global Static info to Parse!!: %s\n",
		//       &(stabstr[cnt]));
	      break;
	  }

	  case 't':	// Type Name 
	      cnt++; /*move past the 't'*/

	      /* get type reference */
	      symdescID = parseSymDesc(stabstr, cnt);

	      //Create Type.
	      if (stabstr[cnt] == '=') {
		/* More Stuff to parse, call parseTypeDef */
            //char *oldStr = stabstr;
		stabstr = parseTypeDef(mod, (&stabstr[cnt+1]), name.c_str(), symdescID);
		cnt = 0;
		// AIX seems to append an semi at the end of these
		if (stabstr[0] && strcmp(stabstr, ";")) {
		    //bperr("\tMore to parse creating type %s\n", stabstr);
		  //bperr( "\tFull String: %s\n", oldStr);
		}
	      } else {
		//Create Type defined as a pre-exisitng type.
		ptrType = mod->getModuleTypes()->findOrCreateType(symdescID);
		if (!ptrType)
		  ptrType = mod->exec()->type_Untyped;
                newType = new typeTypedef(symdescID, ptrType, name);
		if (newType) {
		    mod->getModuleTypes()->addOrUpdateType(newType);
		}
                newType->decrRefCount();
	      }
	      break;
    
	  case ':':	// :T... - skip ":" and parse 'T'
	      if ((stabstr[cnt+1] == 't') || (stabstr[cnt+1] == 'T')) {
		  // parse as a normal typedef
		  parseStabString(mod, linenum, &stabstr[cnt+1], framePtr);
	      } else {
	          //bperr("Unknown type seen %s\n", stabstr);
	      }
	      break;

          case 'T':/* Aggregate type tag -struct, union, enum */
	      cnt++; /*move past the 'T'*/

	      if (stabstr[cnt] == 't') {
		  //C++ struct  tag "T" and type def "t"
		  ////bperr("SKipping C++ Identifier t of Tt\n");
		  cnt++;  //skip it
	      }

	      /* get type reference */
	      symdescID = parseSymDesc(stabstr, cnt);

	      //Create Type.
	      if (stabstr[cnt] == '=') {
		  /* More Stuff to parse, call parseTypeDef */
		  stabstr = parseTypeDef(mod,(&stabstr[cnt+1]),name.c_str(),symdescID);
		  cnt = 0;
		  if (stabstr[0]) {
		      //bperr( "\tMore to parse aggregate type %s\n", (&stabstr[cnt]));
		  //bperr("\tFull String: %s\n", stabstr);
		  }
	      } else {
		  //Create Type defined as a pre-exisitng type.
		  newType = Type::createPlaceholder(symdescID, name);
		  if (newType) { newType = mod->getModuleTypes()->addOrUpdateType(newType); }
	      }

	      break;

	  case 'V':/* Local Static Variable (common block vars too) */
	      cnt++; /*move past the 'V'*/

	      // //bperr("parsing 'v' type of %s\n", stabstr);
	      /* Get variable type number */
	      symdescID = parseTypeUse(mod, stabstr, cnt, name.c_str());
	      if (stabstr[cnt]) {
		  //bperr( "\tMore to parse local static %s\n", &stabstr[cnt]);
		  //bperr( "\tFull String: %s\n", stabstr);
	      }
	      // lookup symbol and set type
	      BPtype = mod->getModuleTypes()->findOrCreateType(symdescID);
	      if (!BPtype) {
		      //bperr("ERROR: unable to find type #%d for variable %s\n", 
		      // symdescID, name.c_str());
		  break;
	      }
	      if (commonBlock) {
		  /* This variable is in a common block */
		  /* add it only if not already there, common block
		     are re-defined for each subroutine but subroutines
		     define only the member they care about
		  */
		  bool found = false;
		  const std::vector<Field *> *fields;
		  fields = commonBlock->getFields();
		  if (fields) {
		      for (unsigned int i=0; i < fields->size(); i++) {
			  if (name == (*fields)[i]->getName()) {
			      found = true;
			      break;
			  }
			  int start1, start2, end1, end2;
			  start1 = (*fields)[i]->getOffset();
			  end1 = start1 + (*fields)[i]->getSize();
			  start2 = framePtr;
			  end2 = framePtr + BPtype->getSize();
			  if (((start2 >= start1) && (start2 < end1)) ||
			      ((start1 >= start2) && (start1 < end2))) {
			      /* common block aliasing detected */
			      //bpwarn("WARN: EQUIVALENCE used in %s: %s and %s\n",
				//  current_func_name.c_str(), name.c_str(), (*fields)[i]->getName());
			      found = true;
			      break;
			  }
		      }
		  }
		  if (!found) {
		      commonBlock->addField(name, BPtype, framePtr);
		  }
	      } else {
		  // put it into the local variable scope
		if (!current_func) {
		  //bperr("Unable to add %s to local variable list in %s\n",
		//	 name.c_str(),current_func_name.c_str());
		} else {
		  locVar = new localVar(name, BPtype, fName, linenum);
	  	  loc_t *loc = (loc_t *)malloc(sizeof(loc_t));
		  loc->stClass = storageAddr;
		  loc->refClass = storageNoRef;
		  loc->frameOffset = framePtr;
		  loc->reg = -1;
		  locVar->addLocation(loc);
		  
		  if(!current_func->vars_)
		  	current_func->vars_ = new localVarCollection();
		  current_func->vars_->addLocalVar( locVar);
		}
	      }
	      break;
          case 'l':
	    /* These are string literals, of the form 
	       name:l(type);value
	       where type must be predefined, and value of of type type.
	       It should be safe to ignore these. */
	    cnt = strlen(stabstr);
	    break;

	  case 'Y':	// C++ specific stuff
             cnt++; // Skip past the 'Y'
             if (stabstr[cnt] == 'I') {
               /* Template instantiation */
                cnt++; // skip past the I;
                if (stabstr[cnt] == 'f') /* Template function */ {
                   while (stabstr[cnt] != '@') cnt++;
                   cnt++; // Skip past '@'
                   cnt++; // Skip past ';'
                   cnt++; // Skip past ';'
                   while (stabstr[cnt] != ':') cnt++;
                   // Create fake stab string that cuts out template garbage
                   char *dupstring = strdup(stabstr);
                   strcpy(dupstring, mangledname.c_str());
                   strcat(dupstring, stabstr+cnt);
                   parseStabString(mod, linenum, dupstring, framePtr, commonBlock);
                   free(dupstring);
                }
             } 
	    cnt = strlen(stabstr);
	    break;

	  default:
	      //bperr( "Unknown symbol descriptor: %c\n", stabstr[cnt]);
	      //bperr( " : %s\n", stabstr);
	      break;
      }   
    }
    return(&stabstr[cnt]);
} /* end of parseStabString */


//
// Is the current character a valid prefix for a symDesc non-terminal?
//
inline bool isSymId(char ch)
{
    return ((ch == '(') || isdigit(ch) || (ch == '-'));
}

//
// parse a Symbol Descriptor ID
//	symDesc = <int> | (<int>,<int>)
//
int parseSymDesc(char *stabstr, int &cnt)
{
    int id;
    int lid;
    int hid;
    int sign = 1;
    bool newForm = false;

    hid = 0; //file-number
    // parse both an int and (int,int) format (file-number, type ID)
    if (stabstr[cnt] == '(') {
	cnt++;
	while (isdigit(stabstr[cnt])) {
	    hid = hid * 10 + stabstr[cnt] - '0';
	    cnt++;
	}

	// skip ","
	if (stabstr[cnt] == ',') cnt++;
	newForm = true;
    }
       
    if (stabstr[cnt] == '-') {
	sign = -1;
	cnt++;
    }

    lid = 0; //type ID
    while (isdigit(stabstr[cnt])) {
        lid = lid * 10 + stabstr[cnt] - '0';
        cnt++;
    }
    if( hid != 0 )
      assert(lid < 65536);
    
    // skip closing ')'
    if (newForm) cnt++;

    id = hid * 65536 + lid;
    id = id * sign;
    return id;
}

//
// parse an identifier up to a ":" or "," or ";"
//
std::string getIdentifier( char *stabstr, int &cnt, bool stopOnSpace ) {
	int i = 0;
	int brCnt = 0;
	bool idChar = true;

	while( idChar ) {
		switch( stabstr[ cnt + i ] ) {
			case '<':
			case '(':
				brCnt++;
				i++;
				break;

			case '>':
			case ')':
				brCnt--;
				i++;
				break;

	        case ' ':
				if ( !stopOnSpace ) {
				    i++;
				    break;
					} // else fall through
			case '\0':
			case ':':
			case ',':
			case ';':
                                /* Handle case of '::' */
                                if ( stabstr[ cnt + i ] == ':' && stabstr[ cnt + i + 1 ] == ':' &&
                                     (stabstr[ cnt + i + 2 ] == '_' || isalpha(stabstr[ cnt + i + 2 ])) ) {
                                   i+=3;
                                   break;
                                }
				/* If we're inside a bracket and we haven't reached
				   the end of the string, continue. */
				if( brCnt != 0 && stabstr[ cnt + i ] != '\0' ) {
					i++;
					}
				else if( brCnt ) {
					//bperr( "Failed to find identifier in stabstring '%s;\n", stabstr );
					idChar = false;
					}
				else {
					idChar = false;
					}
				break;
				
			default:
				i++;
				break;
			} /* end switch */
    } /* end while */

	char * identifier = (char *)malloc( i + 1 );
	assert( identifier );
	
	strncpy( identifier, & stabstr[cnt], i );
	identifier[i] = '\0';
	cnt += i;
	
        std::string pd_identifier = identifier;
        free(identifier);
	return pd_identifier;
	} /* end getIdentifier() */

//
// getFieldName
//
// A simplified version of getIdentifier, it only cares about finding a ':'
//

char * getFieldName( char *stabstr, int &cnt) {
   int i = 0;
   bool idChar = true;

   while ( idChar ) {
      switch( stabstr[ cnt + i ] ) {
      case ':':
         idChar = false;
         break;
      default:
         i++;
      }
   }

   char * identifier = (char *) malloc(i + 1);
   assert(identifier);

   strncpy(identifier, &stabstr[cnt], i);
   identifier[i] = '\0';
   cnt += i;

   return identifier;
}

//
// Parse a use of a type.  
//
//	<typeUse> = <symDesc> | <symDesc>=<typeDef>
//
static int parseTypeUse(Module *mod,char *&stabstr, int &cnt, 
                        const char *name)
{
    int ret = parseSymDesc(stabstr, cnt);

    if (stabstr[cnt] == '=') {
	/* More Stuff to parse, call parseTypeDef */
	stabstr = parseTypeDef(mod, (&stabstr[cnt+1]), name, ret);
	cnt = 0;
    }
    return ret;
}

//
// parseCrossRef - internal struct/union pointer
//
//	<crossRef>	= [s|u|e]<ident>
//
static char *parseCrossRef(typeCollection *moduleTypes,const char * /*name*/,
                           int ID, char *stabstr, int &cnt)
{
    std::string temp;
    Type *newType = NULL, *newType2 = NULL;
    char xreftype;
    cnt++; /* skip 'x'*/

    if ((stabstr[cnt] == 's') || 	// struct 
	(stabstr[cnt] == 'u') ||	// union
        (stabstr[cnt] == 'e')) {	// enum
      xreftype = stabstr[cnt++];

      temp = getIdentifier(stabstr, cnt);
      cnt++; /*skip ':' */

      // Find type that this one points to.
      Type *ptrType = moduleTypes->findType(temp.c_str());
      if (!ptrType) {
         // This type name hasn't been seen before.  Create the
         // skeleton for it, and we'll update it later when we actually see
         // it
         if (xreftype == 'e') {
            newType = new typeEnum(ID, temp);
         } else if (xreftype == 'u') {
            newType = new typeUnion(ID, temp);
         } else {
            newType = new typeStruct(ID, temp);
         }
	// Add to typeCollection
	if(newType) { newType2 = moduleTypes->addOrUpdateType(newType); }
	if(!newType2) {
	  //bperr(" Can't Allocate new type ");
	  exit(-1);
	} else
          newType->decrRefCount();
      }         
    } else {
      /* don't know what it is?? */
      
      temp = getIdentifier(stabstr, cnt);
      cnt++; /*skip ':' */
    }

    return( &(stabstr[cnt]));
}
	    
//
// parse the definition of an array.
// 	arrayDef = ar<symDesc>;<symDesc>;<symDesc>;<symDesc> |
// 		   ar<symDesc>;<symDesc>;<symDesc>;<arrayDef> |
//                 A<arrayDef>
//
static Type *parseArrayDef(Module *mod, const char *name,
		     int ID, char *&stabstr, int &cnt, unsigned int sizeHint)
{
    char *symdesc;
    int symdescID;
    int elementType;
    Type *newType = NULL, *newType2 = NULL;
    Type *ptrType = NULL;
    int lowbound, hibound;

    // format is ar<indexType>;<lowBound>;<highBound>;<elementType>

    assert(stabstr[cnt] == 'a' || stabstr[cnt] == 'A');

    if (stabstr[cnt ++] == 'A') {
       // Open array
       lowbound = 1;
       hibound = 0;
       elementType = parseSymDesc(stabstr, cnt);
       ptrType = mod->getModuleTypes()->findOrCreateType(elementType);
    } else {
       // Regular (maybe) array

       if (stabstr[cnt] != 'r') {
          //bperr("unknown array definition seen %s\n", &stabstr[cnt]);
          return(NULL);
       }
       
       /* array with range */
       symdesc = &(stabstr[cnt]);
       
       cnt++;	/* skip 'r' */
       
       symdescID = parseTypeUse(mod, stabstr, cnt, name);
       
       cnt++; /* skip semicolon */
       lowbound = parseSymDesc(stabstr, cnt);
       
       cnt++; /* skip semicolon */
       if (stabstr[cnt] == 'J') {
          /* Fortran unbounded array */
          hibound = 0;
          cnt++;
       } else if (stabstr[cnt] == 'T') {
          /* Fortran runtime bound array - Txx is the form (xx=digits)*/
          hibound = 0;
          cnt++;
          while (isdigit(stabstr[cnt])) cnt++;
       } else {
          hibound = parseSymDesc(stabstr, cnt);
       }
       
       cnt++; /* skip semicolon */
       elementType = parseSymDesc(stabstr, cnt);
       
       if (stabstr[cnt] == 'a') {
          /* multi dimensional array - Fortran style */
          /* it has no valid id, so we give it a known duplicate */
          ptrType = parseArrayDef(mod, name, 0, stabstr, cnt, sizeHint);
       } else { 
          if (stabstr[cnt] == '=') {
             /* multi dimensional array */
             char *temp;
             temp = parseTypeDef(mod, &(stabstr[cnt+1]), NULL, elementType);
             /* parseTypeDef uses old style of returning updated stabstr,
                but parseArrayDef function needs to return an updated cnt.  
                This simple hack updates cnt based on how far parseTypDef 
                advances it.  jkh 12/4/00 */
             cnt = temp-stabstr;
             if (stabstr[cnt] == ':') {
		//C++ stuff
		////bperr("Skipping C++ rest of array def:  %s\n",name );
		while (stabstr[cnt] != ';') cnt++;
             }
          }
          ptrType = mod->getModuleTypes()->findOrCreateType(elementType);
       }
    }

    //  //bperr("Symbol Desriptor: %s Descriptor ID: %d Type: %d, Low Bound: %d, Hi Bound: %d,\n", symdesc, symdescID, elementType, lowbound, hibound);


    if (ptrType) {
	// Create new type - field in a struct or union
	std::string tName = convertCharToString(name);
        newType = new typeArray(ID, ptrType, lowbound, hibound, tName, sizeHint);
	if (newType) {
	    // Add to Collection
	    newType2 = mod->getModuleTypes()->addOrUpdateType(newType);
            newType->decrRefCount();
	} else {
	    //bperr( " Could not create newType Array\n");
	    exit(-1);
	}
    }
	    
    // //bperr( "parsed array def to %d, remaining %s\n", cnt, &stabstr[cnt]);
    return (newType2);
}

int guessSize(const char *low, const char *hi) {
#ifdef i386_unknown_nt4_0
   LONGLONG l, h;
#else
   long long l, h;
#endif

   if (low[0] == '0')
      sscanf(low, "%llo", &l);
   else
      sscanf(low, "%lld", &l);
   if (hi[0] == '0')
      sscanf(hi, "%llo", &h);
   else
      sscanf(hi, "%lld", &h);

   /*   
   if (( low[0]=='-' && l < -2147483648LL )
       || ( l > || ( h > 2147483647LL))
      return 8;
   else if (( l < -32768 ) || ( h > 32767 ))
      return 4;
   else if (( l < -128 ) || ( h > 127 ))
      return 2;
   else
      return 1;
   */
   if (l < 0) { // Must be signed
      if (l < -2147483648LL || h > 0x7fffffffLL)
         return 8;
      else if (l < 0xffff8000 || h > 0x7fff)
         return 4;
      else if (l < 0xffffff80 || h > 0x7f)
         return 2;
      else
         return 1;
   } else {
      if (h > 0xffffffffLL)
         return 8;
      else if (h > 0xffff)
         return 4;
      else if (h > 0xff)
         return 2;
      else
         return 1;
   }
}

#if defined(i386_unknown_linux2_0) \
 || defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */ \
 || defined(ia64_unknown_linux2_4) /* temporary duplication - TLM */
//
// parse range type of the form:	
//
//	<rangeType> = r<typeNum>;<low>;<high>;
//
static char *parseRangeType(Module *mod, const char *name, int ID, 
                            char *stabstr, unsigned int sizeHint = 0)
{
   int cnt, i, symdescID;
   //int sign = 1;
   Type *baseType;

   cnt = i = 0;

   assert(stabstr[0] == 'r');
   cnt++;

   // range index type - not used
   symdescID = parseSymDesc(stabstr, cnt);

    baseType = mod->getModuleTypes()->findType(symdescID);

   // //bperr("\tSymbol Descriptor: %c and Value: %d\n",tmpchar, symdescID);

   cnt++; /* Discarding the ';' */
   i=0;
   if (stabstr[cnt] == '-' ) {
      i++;
   }

   /* Getting type range or size */

   while (isdigit(stabstr[cnt+i])) i++;

   char *low = (char *)malloc(sizeof(char)*(i+1));
   if(!strncpy(low, &(stabstr[cnt]), i))
      /* Error copying size/range*/
      exit(1);
   low[i] = '\0';

   cnt = cnt + i + 1; /* Discard other Semicolon */
   i = 0;
   if((stabstr[cnt]) == '-') {
      i++; /* discard '-' for (long) unsigned int */
   }
   //Find high bound
   while (isdigit(stabstr[cnt+i])) i++;
   char *hi = (char *)malloc(sizeof(char)*(i+1));
   if(!strncpy(hi, &(stabstr[cnt]), i))
      /* Error copying upper range */
      exit(1);
   hi[i] = '\0';

   int j = atol(hi);
    
   if (j == 0) {
      //Size
      int size = atol(low);

	//Create new type
	Type *newType = new typeScalar(ID, size, name);
	//Add to Collection
	mod->getModuleTypes()->addOrUpdateType(newType);
        newType->decrRefCount();
    }
    else {
	//Range
        //Create new type
        Type *newType;
	std::string tName = convertCharToString(name);
        if (baseType == NULL)
           newType = new typeSubrange(ID, sizeHint ? sizeHint / 8 : guessSize(low,hi), atoi(low), atoi(hi), tName);
        else
           newType = new typeSubrange(ID, sizeHint ? sizeHint / 8 : baseType->getSize(), atoi(low), atoi(hi), tName);
        //Add to Collection
        mod->getModuleTypes()->addOrUpdateType(newType);
        newType->decrRefCount();
    }
   free(low);
   free(hi);
   hi=low=NULL;

   cnt = cnt + i;
   if( stabstr[cnt] == ';')
      cnt++;

   return(&(stabstr[cnt]));
}

#else
//
// parse range type of the form:	
//
//	<rangeType> = r<typeNum>;<low>;<high>;
//
static char *parseRangeType(Module *mod, const char *name, int ID,
                            char *stabstr, unsigned int sizeHint = 0)
{
    int cnt, i, symdescID;
    Type *baseType;
    Type *newType, *newType2;

    cnt = i = 0;

    assert(stabstr[0] == 'r');
    cnt++;

    // range index type
    symdescID = parseSymDesc(stabstr, cnt);

    baseType = mod->getModuleTypes()->findType(symdescID);

    // //bperr("\tSymbol Descriptor: %c and Value: %d\n",tmpchar, symdescID);

    cnt++; /* Discarding the ';' */
    i=0;
    if (stabstr[cnt] == '-' ) {
       i++;
    }

    /* Getting type range or size */
    while (isdigit(stabstr[cnt+i])) i++;

    char *temp = (char *)malloc(sizeof(char)*(i+1));
    if(!strncpy(temp, &(stabstr[cnt]), i))
      /* Error copying size/range*/
      exit(1);
    temp[i] = '\0';
    int j = atol(temp);
    
    char *low = temp;
    cnt = cnt + i + 1; /* Discard other Semicolon */
    i = 0;
    if((stabstr[cnt]) == '-') {
       i++; /* discard '-' for (long) unsigned int */
    }
    
    while(isdigit(stabstr[cnt+i]))
       i++;
    
    char *hi = (char *)malloc(sizeof(char)*(i+1));
    if(!strncpy(hi, &(stabstr[cnt]), i))
       /* Error copying upper range */
       exit(1);
    hi[i] = '\0';

    std::string tname = convertCharToString(name);
    if( j <= 0 ){
      /* range */
      
      // //bperr("\tLower limit: %s and Upper limit: %s\n", low, hi);
      //Create new type
    	
      if (baseType == NULL)
         newType = new typeSubrange(ID, sizeHint ? sizeHint / 8 : guessSize(low,hi), atoi(low), atoi(hi), tname);
      else
         newType = new typeSubrange(ID, sizeHint ? sizeHint / 8 : baseType->getSize(), atoi(low), atoi(hi), tname);
      //Add to Collection
      newType2 = mod->getModuleTypes()->addOrUpdateType(newType);
      newType->decrRefCount();

    } else if( j > 0){
      j = atol(hi);
      if(j == 0){
        /*size */
        int size = (int)j;

	// //bperr("\tSize of Type : %d bytes\n",size);
	//Create new type

        newType = new typeScalar(ID, size, name);
	//Add to Collection
	newType2 = mod->getModuleTypes()->addOrUpdateType(newType);
        newType->decrRefCount();
      } else {
        /* range */
	// //bperr("Type RANGE: ERROR!!\n");
        if (baseType == NULL)
           newType = new typeSubrange(ID, sizeHint ? sizeHint / 8 : sizeof(long), atoi(low), atoi(hi), tname);
	else
	   newType = new typeSubrange(ID, sizeHint ? sizeHint / 8 : baseType->getSize(), atoi(low), atoi(hi),tname);
	newType2 = mod->getModuleTypes()->addOrUpdateType(newType);
        newType->decrRefCount();
      }	
    }
    free(low);
    free(hi);

    cnt = cnt + i;
    if( stabstr[cnt] == ';')
      cnt++;
#ifdef notdef
    // ranges can now be used as part of an inline typdef
    if( stabstr[cnt] ) {
      //bperr( "ERROR: More to parse in type-r- %s\n", &(stabstr[cnt]));
    }
#endif
    
    return(&(stabstr[cnt]));
}

#endif

//
//  <attrType> = @s<int>;<int>
//  <attrType> = @s<int>;(<int>,<int>)
//  <attrType> = @s<int>;r(<int>,<int>);<int>;<int>;
//

//
//   This may in fact be much simpler than first anticipated
//   AIX stabs use attributes only as hints, and dbx only
//   understands @s (size) and @P (packed) types.  We only 
//   parse the size attribute, and should be able to get away
//   with simply passing the remainder to the rest of our parser
//
static char *parseAttrType(Module *mod, const char *name,
			 int ID, char *stabstr, int &cnt)
{
    assert(stabstr[cnt] == '@');
    cnt++; // skip the @
   
    if (stabstr[cnt] == 's') {
      cnt++;
      
      int size = parseSymDesc(stabstr, cnt);
      cnt++;  // skip ';'

      char *newstr =  parseTypeDef(mod, stabstr+cnt, name, ID, size);
      if (newstr[0] == ';')
         return newstr+1;
      else
         return newstr;
    } else {
	////bperr(" Unable to parse Type Attribute: %s ID %d : %s\n", 
	// name,ID, &(stabstr[cnt]));
       while (stabstr[cnt] != ';') cnt++;
       cnt++;
       return parseTypeDef(mod, stabstr+cnt, name, ID);
    }
}
/*
static void parseAttrType(Module *mod, const char *name,
			 int ID, char *stabstr, int &cnt)
{
    bool includesRange = false;
    char *low = NULL, *high = NULL;

    // format @s(size in bits); negative type number;
    dataClass typdescr = dataTypeAttrib;

    assert(stabstr[cnt] == '@');
    cnt++; // skip the @

    if (stabstr[cnt] == 's') {
      cnt++;
      
      int size = parseSymDesc(stabstr, cnt);
      cnt++;  // skip ';'

      if (stabstr[cnt] == 'r') {
	  // include range at end
	  cnt++;
	  includesRange = true;
      }

      int type = parseSymDesc(stabstr, cnt);
      // skip ';' end of stab record ??? (at least for bool)
      cnt++;

      if (includesRange) {
	  int len;

	  // Parse out low range string.
	  len = 0;
	  if (stabstr[cnt] == '-' ) cnt++, len++;
	  while (isdigit(stabstr[cnt])) cnt++, len++;
	  cnt++;    // skip ';'

	  // Store the low range string.
	  low = (char *)malloc(sizeof(char) * (len + 1));
	  assert(low);
	  strncpy(low, &stabstr[cnt - (len + 1)], len);
	  low[len] = '\0';

	  // Parse out high range string.
	  len = 0;
	  if (stabstr[cnt] == '-' ) cnt++, len++;
	  while (isdigit(stabstr[cnt])) cnt++, len++;
	  cnt++;    // skip ';'

	  // Store the high range string.
	  high = (char *)malloc(sizeof(char) * (len + 1));
          assert(high);
          strncpy(high, &stabstr[cnt - (len + 1)], len);
          high[len] = '\0';
      }

      // Create a new B_type that points to a builtInTypes
      Type *ptrType =BPatch::bpatch->builtInTypes->findBuiltInType(type);
      
      if (!ptrType) ptrType = BPatch::bpatch->type_Untyped;
      
      Type *newType = new Type(name, ID, typdescr, size/8, ptrType);
      if(!newType) {
	    //bperr(" Can't Allocate new type ");
	    exit(-1);
      }

      if (includesRange) {
	  newType->setLow(low);
	  newType->setHigh(high);
	  free(low);
	  free(high);
      }

      // Add type to collection
      newType2 = mod->getModuleTypes()->addOrUpdateType(newType);
      newType->decrRefCount();

      if (stabstr[cnt]) {
	  //bperr("More Type Attribute to Parse: %s ID %d : %s\n", name,
	       ID, &(stabstr[cnt]));
	  //bperr("got type = %d\n", type);
	  //bperr("full string = %s\n", stabstr);
      }
    } else {
	////bperr(" Unable to parse Type Attribute: %s ID %d : %s\n", 
	// name,ID, &(stabstr[cnt]));
    }
}
*/
//
//  <refType> = &<typeUse>
//
static char *parseRefType(Module *mod, const char *name,
		   int ID, char *stabstr, int &cnt)
{
    /* reference to another type */
    assert(stabstr[cnt] == '&');
    cnt++;
    
    int refID = parseTypeUse(mod, stabstr, cnt, name);
    
    // Create a new B_type that points to a structure
    Type *ptrType = mod->getModuleTypes()->findOrCreateType(refID);
    if (!ptrType) ptrType = mod->exec()->type_Untyped;
    std::string tName = convertCharToString(name); 
    Type *newType = new typeRef(ID, ptrType, tName);

    // Add to typeCollection
    if(newType) {
	mod->getModuleTypes()->addOrUpdateType(newType);
        newType->decrRefCount();
    } else {
        //bperr(" Can't Allocate new type ");
        exit(-1);
    }
    
    return(&(stabstr[cnt]));
}

//
// Given a base class and a new type, add all visible fields to the new class
//
void addBaseClassToClass(Module *mod, int baseID, 
                         fieldListType *newType, int /*offset*/)
{

    //Find base class
    fieldListType *baseCl = dynamic_cast<fieldListType *>(mod->getModuleTypes()->findType(baseID));
    if( ! baseCl ) {
	std::string modName = mod->fileName();
        //bpwarn( "can't find base class id %d in module %s\n", baseID, modName);
        baseCl = new typeStruct(baseID);
        fieldListType *baseCl2 = dynamic_cast<typeStruct *>(mod->getModuleTypes()->addOrUpdateType( baseCl ));
	std::string fName = "{superclass}";
	newType->addField( fName, baseCl2, -1, visUnknown );
        baseCl->decrRefCount();
	return;
    }
    std::string fName = "{superclass}";
    newType->addField( fName, baseCl, -1, visUnknown );

    //Get field descriptions of the base type
    /*
    const std::vector<Field *> *baseClFields = baseCl->getComponents();
    for (unsigned int fieldNum=0; fieldNum < baseClFields->size(); fieldNum++) {
	Field *field = (*baseClFields)[fieldNum];

	if (field->getVisibility() == visPrivate)
	    continue; //Can not add this member

	newType->addField(field->getName(), field->getTypeDesc(), field->getType(), field->getOffset()+offset, field->getVisibility());
    }
    */
}

//
// parse a list of fields.
//    Format is [A|B|C-M|N|O][c][G]<fieldName>:<type-desc>;offset;size;
//
static char *parseFieldList(Module *mod, fieldListType *newType, 
			    char *stabstr, bool sunCPlusPlus)
{
    int cnt = 0;
    int size = 0;
    char *compname;
    int comptype= 0;
    int beg_offset=0;
    visibility_t _vis = visUnknown;
    dataClass typedescr;
    bool hasVirtuals = false;

#ifdef IBM_BPATCH_COMPAT_STAB_DEBUG
      //bperr( "%s[%d]:  inside parseFieldList\n", __FILE__, __LINE__);
#endif

    if (stabstr[cnt] == '!') {
	//Inheritance definition, Add base class field list to the current one
	//according to visibility rules.

	cnt++; //Skip '!'

	//Get # of base classes
	int baseClNum = atoi(getIdentifier(stabstr, cnt).c_str());
	cnt++; //Skip ','

        typeStruct *newStructType = dynamic_cast<typeStruct *>(newType);
	//Skip information for each base class
	for(int i=0; i<baseClNum; ++i) {
		//Skip virtual inheritance flag, base visibility flag and base offset
		getIdentifier(stabstr, cnt);
		cnt++; //Skip ','

		//Find base class type identifier
		int baseID = parseSymDesc(stabstr, cnt);

		cnt++; //Skip ';'

		addBaseClassToClass(mod, baseID, newStructType, 0);
	}
     }

     while (stabstr[cnt] && (stabstr[cnt] != ';')) {
	typedescr = dataScalar;

	if (stabstr[cnt] == '~') {
		//End of virtual class
		while(stabstr[cnt] != ';') cnt++;
		break; //End of class is reached
	}

	// skip <letter>cG
	if (sunCPlusPlus) cnt += 3;

	if ((stabstr[cnt] == 'u') && (stabstr[cnt+1] == ':') && (!isdigit(stabstr[cnt+2]))) {
	    cnt += 2;
	}

	compname = getFieldName(stabstr, cnt);
/*
	if (strlen(compname) == 0) {
		//Something wrong! Most probably unhandled C++ type
		//Skip the rest of the structure
		while(stabstr[cnt]) cnt++;
		return(&stabstr[cnt]);
	}
*/
	cnt++;	// Skip ":"

	if ((stabstr[cnt]) == ':') {
	  //Method definition
	  typedescr = dataFunction;
	  cnt++;
	}

	if ((stabstr[cnt]) == '/') { // visibility C++
	  cnt++; /* get '/' */
	  switch (stabstr[cnt]) {
	  case '0':
	    _vis = visPrivate;
	    break;
	  case '1':
	    _vis = visProtected;
	    break;
	  case '2':
	    _vis = visPublic;
	    break;
	  default:
	    _vis = visUnknown;
	  }
	  cnt++; // get visibility value
	}

	// should be a typeDescriptor
	comptype = parseTypeUse(mod, stabstr, cnt, "");

        if (stabstr[cnt] == ':') {
           while (stabstr[cnt] == ':') {
              cnt++; //Discard ':'
              beg_offset = 0;
              size = 0;
              std::string varName = getIdentifier(stabstr, cnt);
              if (typedescr == dataFunction) {
                 // Additional qualifiers for methods
                 cnt++; //Skip ';'
                 cnt++; //Skip visibility
                 cnt++; //Skip method modifier
                 if (stabstr[cnt] == '*') {
                    //Virtual fcn definition
                    hasVirtuals = true;
                    cnt++; //Skip '*'
                    while(stabstr[cnt] != ';') cnt++; //Skip vtable index
                    cnt++; //Skip ';'
                    if (stabstr[cnt] != ';') {
                       parseTypeUse(mod, stabstr, cnt, ""); //Skip type number to the base class
                    }
                    cnt++; //Skip ';'
                    if (isSymId(stabstr[cnt])) {
                       parseTypeUse(mod, stabstr, cnt, "");
                    }
                 } else if ( (stabstr[cnt] == '.') || 
                             (stabstr[cnt] == '?') ) {
                    cnt++; //Skip '.' or '?'
                    if (isSymId(stabstr[cnt])) {
                       parseTypeUse(mod, stabstr, cnt, "");
                    }
                 }
              }
              if (stabstr[cnt] == ';')
                 cnt++; //Skip ';'
           }
        } else if (stabstr[cnt] == ',') {
           cnt++;	// skip ','
           beg_offset = parseSymDesc(stabstr, cnt);
           
           if (stabstr[cnt] == ',') {
              cnt++;	// skip ','
              size = parseSymDesc(stabstr, cnt);
           }
           else
              size = 0;
        }
           
#ifdef IBM_BPATCH_COMPAT_STAB_DEBUG
	//bperr( "%s[%d]:  asserting last char of stabstr == ';':  stabstr = %s\n", 
		__FILE__, __LINE__, stabstr);
#endif
	
	if (stabstr[cnt] == ';') // jaw 03/15/02-- major kludge here for DPCL compat
	  cnt++;  // needs further examination
	// //bperr("\tType: %d, Starting Offset: %d (bits), Size: %d (bits)\n", comptype, beg_offset, size);
	// Add struct field to type
	Type *fieldType = mod->getModuleTypes()->findOrCreateType( comptype );
	if (fieldType == NULL) {
		//C++ compilers may add extra fields whose types might not available.
		//Assign void type to these kind of fields. --Mehmet
		fieldType = mod->getModuleTypes()->findType("void");
	}
	std::string fName = convertCharToString(compname);
	if (_vis == visUnknown) {
	    newType->addField(fName, fieldType, beg_offset);
	} else {
	    // //bperr( "Adding field '%s' to type '%s' @ 0x%x\n", compname, newType->getName(), newType );
	    newType->addField(fName, fieldType, beg_offset, _vis);
	    ////bperr("Adding Component with VISIBILITY STRUCT\n");
	}
        free(compname);
     }

     if (hasVirtuals && 
         stabstr[cnt] == ';' &&
         stabstr[cnt+1] == '~' &&
         stabstr[cnt+2] == '%') {
        cnt+=3;
        while (stabstr[cnt] != ';') cnt++;
     }         

    // should end with a ';'
    if (stabstr[cnt] == ';') {
	return &stabstr[cnt+1];
    } else if (stabstr[cnt] == '\0') {
	return &stabstr[cnt];
    } else {
	//bperr("invalid stab record: %s\n", &stabstr[cnt]);
	abort();
	return NULL; // should not get here
    }
}


//
//	Y<type><size><className>;<Bases>;<DataMembers>;<MemberFunctions>;<StaticDataMembers>;
//		<Friends>;<VirtualFunctionInfo>;<NestedClassList>;<AccessAdjustments>;
//		<VirtualBaseClassOffsets>;<TemplatmentMembers>;<PassMethod>;
//
static char *parseCPlusPlusInfo(Module *mod,
	     char *stabstr, const char *mangledName, int ID)
{
    int cnt;
    char *name;
    int structsize;
    bool sunStyle = true;
    bool nestedType = false;
    dataClass typdescr;
    fieldListType * newType = NULL, *newType2 = NULL;

    assert(stabstr[0] == 'Y');
    cnt = 1;

    // size on AIX 
    if (isdigit(stabstr[cnt])) {
	structsize = parseSymDesc(stabstr, cnt);
	sunStyle = false;
    }

    switch(stabstr[cnt]) {
	case 'C':
	case 'c':
	    typdescr = dataTypeClass;
	    break;

	case 'S':
	    nestedType = true;
	case 's':
	    typdescr = dataStructure;
	    break;

	case 'U':
	    nestedType = true;
	case 'u':
	    typdescr = dataUnion;
	    break;

        case 'n':	// namespace - ignored
	    cnt = strlen(stabstr);
	    return(&(stabstr[cnt]));
	    break;

	default:
	    //bperr( "ERROR: Unrecognized C++ str = %s\n", stabstr);
	    cnt = strlen(stabstr);
	    return(&(stabstr[cnt]));
	    break;
    }

    cnt++;		// skip to size
    if (isdigit(stabstr[cnt])) {
	structsize = parseSymDesc(stabstr, cnt);
    }
    
    if (stabstr[cnt] == 'V') cnt++;
    if (stabstr[cnt] == '(') cnt++;

    if (sunStyle && (stabstr[cnt] != ';')) {
	int len;
	char *n;

	// Class or Type Name
	n = &stabstr[cnt];
	while (stabstr[cnt] != ';') cnt++;
	len = &stabstr[cnt] - n;
	name = (char *) calloc(len + 1, sizeof(char));
	strncpy(name, n, len);
    } else {
	name = const_cast< char * >( mangledName );
    }

    std::string tName = convertCharToString(name);
    //Create new type
    switch (typdescr) {
    case dataTypeClass:
       newType = new typeStruct(ID, tName);
       break;
    case dataStructure:
       newType = new typeStruct(ID, tName);
       break;
    case dataUnion:
       newType = new typeUnion(ID, tName);
       break;
    default:
       assert(0);
    }
    //add to type collection
    newType2 = dynamic_cast<fieldListType *>(mod->getModuleTypes()->addOrUpdateType(newType));
    newType->decrRefCount();

    if (sunStyle) {
	cnt++;
	// base class(es) 
	while (stabstr[cnt] != ';') {
	    // skip visibility flag
	    cnt++;

	    int offset = parseSymDesc(stabstr, cnt);

	    // Find base class type identifier
            int baseID = parseSymDesc(stabstr, cnt);
	    addBaseClassToClass(mod, baseID, newType2, offset);
	}

	cnt++;	// skip ;
    }

    // parse dataMembers
    stabstr = parseFieldList(mod, newType2, &stabstr[cnt], sunStyle);
    cnt = 0;

    if (stabstr[0]) {
	// parse member functions
	cnt++;
	while (stabstr[cnt] && (stabstr[cnt] != ';')) {
	    std::string pd_funcName = getIdentifier(stabstr, cnt, true);
            const char *funcName = pd_funcName.c_str();

	    funcName++;	// skip ppp-code

	    if (*funcName == '-') funcName++; // it's a pure vitual

	    while (isdigit(*funcName)) funcName++; // skip virtual function index
	    funcName++;

	    char *className = strdup(currentRawSymbolName.c_str());
	    className[3] = 'c';
	    className[strlen(className)-1] = '\0';	// remove tailing "_"
	    std::string methodName = std::string(className) + std::string(funcName) + std::string("_");
		char * name = P_cplus_demangle( methodName.c_str(), mod->exec()->isNativeCompiler() );
		if( name != NULL ) {
			funcName = strrchr( name, ':' );
			if( funcName ) { funcName++; }
			else { funcName = name; }
			}

	    // should include position for virtual methods
	    Type *fieldType = mod->getModuleTypes()->findType("void");

	    std::string fName = convertCharToString(funcName);

	    typeFunction *funcType = new typeFunction( ID, fieldType, fName);
            newType2->addField( fName, funcType);
					    
	    free(name);
	    free(className);
	    if (stabstr[cnt] == ' ') cnt++;
	}
    }

    cnt = strlen(stabstr);
    return(&(stabstr[cnt]));
}

//
// This function takes a <typeDef> and parses it 
//
//	<typeDef> = <symDesc> 	|
//		<crossRef>	|	
//		*<typeUse>	|	Pointer to a type
//		<arrayDef>	|
//		f<typeUse>	|	function type
//		R<int>,<int>	|	Real type 
//		b[u|s][c|]<int>;<int>;<int>	|	Builtin
//		<rangeType>	|
//		e<enumType>	|
//		<attrType>	|
//		<refType>	|
//		k<typeDef>	|	SunPro constant
//		B<typeDef>	|	SunPro volatile
//		M<symDesc>;<int>|	Fortran CHARACTER array
//		s<int><fields>	|	Structure <int> is size
//		u<int><fields>	|	Union <int> is size
//		V<typeUse>
//
//	<enumType> = <ident>:<int> | <ident>:<int>,<enumType>
//
// It adds the typeDef to the type definition with the name name, and id ID.
//
static char *parseTypeDef(Module *mod, char *stabstr, 
                         	const char *name, int ID, unsigned int sizeHint)
{

    Type * newType = NULL, *newType2 = NULL;
    fieldListType * newFieldType = NULL, *newFieldType2 = NULL;
    Type * ptrType = NULL;
  
    std::string compsymdesc;
  
    dataClass typdescr;
    int ptrID=0;
    
    int value;
    int cnt,i,j,k;
    int structsize;
    int type;
    cnt = i = j = k = 0;
#ifdef IBM_BPATCH_COMPAT_STAB_DEBUG
    //bperr( "%s[%d]:  inside parseTypeDef, stabstr = %s\n", __FILE__, __LINE__, 
	    (stabstr == NULL) ? "NULL" : stabstr);
#endif

    assert (stabstr[0] != '=');

    // //bperr( "parsing %s\n", stabstr);
    if (isSymId(stabstr[0])) {
	typdescr = dataScalar;
	type = parseSymDesc(stabstr, cnt);
	 	    
	if (ID == type) {
            // Type:tFOO = FOO
            // as far as I can tell, this only happens when defining an empty
            // type (i.e. void)
	    
	    std::string tName = convertCharToString(name);
            newType = new typeScalar(ID, 0, tName);
	    if (newType) { newType2 = mod->getModuleTypes()->addOrUpdateType(newType); }
	    if(!newType2) {
	      //bpfatal(" Can't Allocate newType ");
	      exit(-1);
	    } else
               newType->decrRefCount();
	} else if (stabstr[cnt] == '=') {
	    // XXX - in the new type t(0,1)=(0,2)=s... is possible
	    // 	     skip the second id for now -- jkh 3/21/99
	    stabstr = parseTypeDef(mod, &(stabstr[cnt+i+1]), name, type);
	    cnt = 0;
	    Type *oldType;
	    
	    oldType = mod->getModuleTypes()->findOrCreateType(type);
	    if(!oldType) oldType = mod->exec()->type_Untyped;
	    std::string tName = convertCharToString(name);
            newType = new typeTypedef(ID, oldType, tName, sizeHint);
	    if (newType) {
               mod->getModuleTypes()->addOrUpdateType(newType);
               newType->decrRefCount();
            }
            
	} else {
	    Type *oldType;
	    std::string tName = convertCharToString(name);
	    oldType = mod->getModuleTypes()->findOrCreateType(type);
            newType = new typeTypedef(ID, oldType, tName, sizeHint);
	    if(newType) { newType2 = mod->getModuleTypes()->addOrUpdateType(newType); }
	    if(!newType2) {
	        //bpfatal(" Can't Allocate newType ");
	        exit(-1);
	    } else
               newType->decrRefCount();
	}
    } else {
      switch (stabstr[0]) {
	  case 'x':  //cross reference 
	  {
	    parseCrossRef(mod->getModuleTypes(), name, ID, stabstr, cnt);
	    break;
	  }   
	  case '*':
	  {
	    /* pointer to another type */
	    cnt++;
	    ptrID = parseTypeUse(mod, stabstr, cnt, NULL);

	    // Create a new B_type that points to a structure
	    ptrType = mod->getModuleTypes()->findOrCreateType(ptrID);
	    if (!ptrType) ptrType = mod->exec()->type_Untyped;

            newType = new typePointer(ID, ptrType);
	    // Add to typeCollection
	    if(newType) { newType2 = mod->getModuleTypes()->addOrUpdateType(newType); }
	    if(!newType2) {
		//bpfatal(" Can't Allocate new type ");
		exit(-1);
	    } else
               newType->decrRefCount();

	    return(&(stabstr[cnt]));
	    break;
	  }
	  case 'a':
          case 'A':
	  {
	      (void) parseArrayDef(mod, name, ID, stabstr, cnt, sizeHint);
	      return (&stabstr[cnt]);
	      break;
          }
          case 'g':  
	  {
	  	/* function with return type and prototype */

		// g<typeUse>[<typeUse>]*#
		typdescr = dataFunction;

		cnt++; /* skip the g */
	        type = parseTypeUse(mod, stabstr, cnt, name);
                ptrType = mod->getModuleTypes()->findOrCreateType(type);

                {
		   std::string tName = convertCharToString(name);
                   typeFunction *newFunction = 
                      new typeFunction(ID, ptrType, tName);
                   typeFunction *newFunction2 = NULL;
                   
                   if (newFunction) { 
                      newFunction2 = dynamic_cast<typeFunction*>(mod->getModuleTypes()->addOrUpdateType(newFunction)); 
		      newFunction->decrRefCount();
                   }
                   if (!newFunction2) {
                      //bpfatal(" Can't Allocate new type ");
                      exit(-1);
                   }
                   
                   while ((stabstr[cnt] != '#') &&  (stabstr[cnt])) {
                      int paramType;
                      paramType = parseTypeUse(mod, stabstr, cnt, name);
                      newType = mod->getModuleTypes()->findOrCreateType(paramType);
		      newFunction2->addParam(newType);
                      //newFunction2->addField(buffer, newType->getDataClass(), newType, curOffset, newType->getSize());
                   }
                }

		// skip #
		if (stabstr[cnt] == '#') cnt++;
		break;
	  }
	  case 'f':
	  {
	        /* function type */
		typdescr = dataFunction;

		cnt++; /* skip the f */
	        type = parseTypeUse(mod, stabstr, cnt, name);
                ptrType = mod->getModuleTypes()->findOrCreateType(type);

		
                std::string tName = convertCharToString(name);
		newType = new typeFunction(ID, ptrType, tName);
		if (newType) { newType2 = mod->getModuleTypes()->addOrUpdateType(newType); }
		if (!newType2) {
		  //bpfatal(" Can't Allocate new type ");
		  exit(-1);
		} else
                   newType->decrRefCount();

		// skip to end - SunPro Compilers output extra info here - jkh 6/9/3
		// cnt = strlen(stabstr);
		break;
	 }

	 case 'M': 
	 {
		/* CHARACTER ??? */
		cnt++; // skip  'M'

		int baseType = parseSymDesc(stabstr, cnt);
		if (baseType != -2 || (stabstr[cnt] != ';')) {
		    //bperr("unexpected non character array %s\n", stabstr);
		} else {
		    cnt++; // skip ';'
		    int size;
		    if (stabstr[cnt] == 'T') {
		      /* Fortran stack-based array bounds */
		      size = 0;
		      cnt++; // skip 'T'
		      (void) parseSymDesc(stabstr, cnt);
		    } else if (stabstr[cnt] == 'J') {
                      /* Unbounded range */
                      size = 0;
                      cnt++; // skip 'J';
		      (void) parseSymDesc(stabstr, cnt);
                    } else
		      size = parseSymDesc(stabstr, cnt);

		    ptrType = mod->getModuleTypes()->findOrCreateType(baseType);
		    std::string tName = convertCharToString(name);
                    newType = new typeArray(ID, ptrType, 1, size, tName);
		    mod->getModuleTypes()->addOrUpdateType(newType);
                    newType->decrRefCount();
		}
		break;

	 }
	 case 'R': 
	 {
		// Define a floating point type - R fp-type; bytes;
		cnt++;
		(void) parseSymDesc(stabstr, cnt);
		cnt ++;

		int bytes = parseSymDesc(stabstr, cnt);

                newType = new typeScalar(ID, bytes, name);
		mod->getModuleTypes()->addOrUpdateType(newType);
                newType->decrRefCount();

		if (stabstr[cnt] == ';') cnt++;	// skip the final ';'

		// gcc 3.0 adds an extra field that is always 0 (no indication in the code why)
		if (stabstr[cnt] == '0') cnt += 2;	// skip the final '0;'

		break;
	  }

	  case 'b': 
	  {
		// builtin type b  - signed char-flag width; offset; nbits
		int limit = strlen(&stabstr[cnt]);

		// skip to width
		while (!isdigit(stabstr[cnt+i]) && (i < limit)) i++;	
		if (i >= limit) return(&(stabstr[cnt]));

                cnt += i;
                int size = parseSymDesc(stabstr,cnt);
                cnt -= i;
		i++;	// skip the ';'

		// skip to bits
		while (stabstr[cnt+i] != ';' && (i < limit)) i++;	
		if (i >= limit) return(&(stabstr[cnt]));
		i++;

		cnt += i;
		parseSymDesc(stabstr, cnt);
		
		if (stabstr[cnt]) cnt++;	// skip the final ';'

                newType = new typeScalar(ID, size, name);
		//Add to Collection
		mod->getModuleTypes()->addOrUpdateType(newType);
                newType->decrRefCount();

		return &stabstr[cnt];
		break;
	}
	case 'r': 		// range type
	{    
	    return parseRangeType(mod, name, ID, stabstr, sizeHint);
	    break;
	}
	case 'e':		// Enumerated type
	{
	    cnt++; 	/* skip the 'e' */

	    // Create new Enum type
	    std::string tName = convertCharToString(name);
	    typeEnum *newEnumType = new typeEnum(ID, tName);
	    // Add type to collection
	    newEnumType = dynamic_cast<typeEnum *>(mod->getModuleTypes()->addOrUpdateType(newEnumType));
            newEnumType->decrRefCount();
		
	    while (stabstr[cnt]) {
		/* Get enum component value */
		compsymdesc = getIdentifier(stabstr, cnt);
		cnt++; /* skip colon */

#ifdef IBM_BPATCH_COMPAT_STAB_DEBUG
		//bperr( "%s[%d]:  before parseSymDesc -- enumerated type \n", 
			__FILE__, __LINE__);
#endif		  
		value = parseSymDesc(stabstr, cnt);

		// add enum field to type
		newEnumType->addConstant(compsymdesc, value);
		  
		cnt++; /* skip trailing comma */
		if ((stabstr[cnt]) == ';') cnt++; /* End of enum stab record */
	    }
	    break;
	}    
        case '@':  // type attribute, defines size and type (negative num)
	{
	    return parseAttrType(mod, name, ID, stabstr, cnt);
	    break;
	}
        case '&': //XXX- Need to complete, may be more to parse jdd 4/22/99
	{    
	    return parseRefType(mod, name, ID, stabstr, cnt);
	    break;
	}
        case 'k':	// Sun constant type s<typeDef> - parse as <typeDef>
	{
	    return parseTypeDef(mod, &stabstr[cnt+1], name, ID);
	    break;
	}
	case 'V':	// AIX colatile ? type V<typeDef> - parse as <typeDef>
        case 'B':	// Sun volatile type B<typeDef> - parse as <typeDef>
	    return parseTypeDef(mod, &stabstr[cnt+1], name, ID);
	    break;

	case 's':	// struct
	case 'u':	// union
        case 'T':       // Fortran TYPE
	{    
	    /* Type descriptor */
	    if (stabstr[cnt] == 's' || stabstr[cnt] == 'T') {
	        typdescr = dataStructure;
	    } else {
	        typdescr = dataUnion;
	    }

	    cnt++;		// skip to size
	    structsize = parseSymDesc(stabstr, cnt);
	   
	    std::string tName = convertCharToString(name);
	    //Create new type
            if (typdescr == dataStructure)
               newFieldType = new typeStruct(ID, tName);
            else
               newFieldType = new typeUnion(ID, tName);
	    //add to type collection
	    newFieldType2 = dynamic_cast<fieldListType *>(mod->getModuleTypes()->addOrUpdateType(newFieldType));
            newFieldType->decrRefCount();
	    char *ret = parseFieldList(mod, newFieldType2, &stabstr[cnt], false);
            return ret;

	    break;
	}
	case 'Y':
	{
	    // C++ specific stabs (Sun compiler)
	    return parseCPlusPlusInfo(mod, stabstr, name, ID);
	    break;
	}
	case 'Z':  // What is this ??? - jkh 10/14/99 (xlc compiler uses it)
	{    
	    return (&stabstr[1]);
	    break;
	}
	case '#':
	{
	    //Class method definition
	    cnt++; //Skip '#'
	    if (stabstr[cnt] == '#') {
		//Get return type
	    	cnt++; //Skip '#'
		parseTypeUse(mod, stabstr, cnt, name);
	    }
	    else {
	    	while(1) {
			//Skip class type, return typ and arg types
			parseTypeUse(mod, stabstr, cnt, name);
			if (stabstr[cnt] == ',')
				cnt++;
			else if (stabstr[cnt] == ';')
				break;
		}
	    }

	    cnt++; //Skip ';'
    	    return(&(stabstr[cnt]));
	    break;
	}
	default:
	    //bperr( "ERROR: Unrecognized str = %s\n", &stabstr[cnt]);
	    //	    return NULL;
	    // Null probably isn't the right choice here.
	    cnt = strlen(stabstr);
	    break;
      }
    }

    return(&(stabstr[cnt]));
} /* end of parseTypeDef*/

//
// parseConstantUse - parse a constant (used by Fortran PARAMETERS)
//
// <constantUse> = =i<int> |
//		   =r <float>
//
//
static Type *parseConstantUse(Module *mod, char *stabstr, int &cnt)
{
    // skip =
    cnt++;

    Type *ret;

    if (stabstr[cnt] == 'i') {
	ret = mod->getModuleTypes()->findType("integer*4");
    } else if (stabstr[cnt] == 'r') {
	ret = mod->getModuleTypes()->findType("double");
    } else if (stabstr[cnt] == 's') {
        ret = mod->getModuleTypes()->findType("char *");
    } else {
	//bperr("unknown constant type %s\n", &stabstr[cnt]);
	ret = NULL;
    }

    cnt = strlen(stabstr);

    return ret;
}

