/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */

#include <ctype.h>
// from libiberty's demangle.h
#define DMGL_PARAMS   (1 << 0) 
#define DMGL_ANSI     (1 << 1) 
#define DMGL_VERBOSE  (1 << 3) 
#include "BPatch.h"
#include "BPatch_module.h"
#include "BPatch_collections.h"
#include "showerror.h"
#include "util.h"
#include "Object.h" // For looking up compiler type

extern char *current_func_name;
extern char *current_mangled_func_name;
extern BPatch_function *current_func;

// Forward references for parsing routines
static int parseSymDesc(char *stabstr, int &cnt);
static BPatch_type *parseConstantUse(BPatch_module*, char *stabstr, int &cnt);
static char *parseTypeDef(BPatch_module*, char *stabstr, 
                          const char *name, int ID);
static int parseTypeUse(BPatch_module*, char *&stabstr, int &cnt,
                        const char *name);
static inline bool isSymId(char ch);
static char *getIdentifier(char *stabstr, int &cnt, bool stopOnSpace=false);

static char *currentRawSymbolName;
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

void vectorNameMatchKLUDGE(BPatch_module *mod, char *demangled_sym, BPatch_Vector<BPatch_function *> &bpfv, pdvector<int> &matches)
{
  const int bufsize = 1024;
  // iterate through all matches and demangle names with extra parameters, compare
  for (unsigned int i = 0; i < bpfv.size(); ++i) {
    char l_mangled[bufsize];
    bpfv[i]->getMangledName(l_mangled, bufsize);
    
    char * l_demangled_raw = P_cplus_demangle( l_mangled, mod->isNativeCompiler() );
    if( l_demangled_raw == NULL ) {
    	l_demangled_raw = strdup( l_mangled );
    	assert( l_demangled_raw != NULL );
    	}
    
    if (!strcmp(l_demangled_raw, demangled_sym)) {
      matches.push_back(i);
    }

	free( l_demangled_raw );
  } /* end iteration over function vector */
}

BPatch_function *mangledNameMatchKLUDGE(char *pretty, char *mangled, 
					BPatch_module *mod)
{

  BPatch_Vector<BPatch_function *> bpfv;
  if ((NULL == mod->findFunction(pretty, bpfv)) || !bpfv.size()) {
    // cerr << __FILE__ << __LINE__ << ":  KLUDGE Cannot find " << pretty << endl;
    return NULL;  // no pretty name hits, expecting multiple
  }

  //cerr << __FILE__ << __LINE__ << ":  mangledNameMatchKLUDGE: language = " 
  //     << mod->getLanguageStr() << endl;
  if (BPatch_f90_demangled_stabstr == mod->getLanguage()) {
      // debug function symbols are presented in "demangled" style.
      if (bpfv.size() == 1)
	return bpfv[0];
      else {
	cerr << __FILE__ << __LINE__ << ":  FIXME!" << endl;
	return NULL;
      }
    }

  // demangle name with extra parameters
  char * demangled_sym = P_cplus_demangle( mangled, mod->isNativeCompiler(), true );
  if( demangled_sym == NULL ) {
  	demangled_sym = strdup( mangled );
  	assert( demangled_sym != NULL );
  	}

  pdvector<int> matches;

  vectorNameMatchKLUDGE(mod, demangled_sym, bpfv, matches);

  BPatch_function *ret = NULL;

  if (matches.size() == 1) {ret = bpfv[matches[0]]; goto clean_up;}
  if (matches.size() > 1) goto clean_up;
  
  // check in the uninstrumentable pile
  bpfv.clear();
  matches.clear();

  if (NULL == mod->findUninstrumentableFunction(pretty, bpfv) || !bpfv.size())
    goto clean_up;
  else {
    //cerr << __FILE__ << __LINE__ << ":  KLUDE found uninstrumentable " << mangled << endl;
  }
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
char *parseStabString(BPatch_module *mod, int linenum, char *stabstr, 
		      int framePtr, BPatch_type *commonBlock = NULL)
{

    int cnt;
    int ID = 0;
    int symdescID = 0;
    int funcReturnID = 0;
    BPatch_function  *fp;
    BPatch_type * ptrType = NULL;
    BPatch_type * newType = NULL; // For new types to add to the collection
    BPatch_localVar *locVar = NULL;
    cnt= 0;

    /* get type or variable name */
    char * mangledname = getIdentifier( stabstr, cnt );
	if( mangledname == NULL ) { mangledname = ""; }

    currentRawSymbolName = mangledname;
    char * name = P_cplus_demangle( mangledname, mod->isNativeCompiler() );
    if( name == NULL ) {
    	name = strdup( mangledname );
    	assert( name != NULL );
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
	
	//bperr("Variable: %s  Type ID: %d, file ID %d \n",name, ID, file_ID);
	if (stabstr[cnt] == '=') {
	  /* More Stuff to parse, call parseTypeDef */
	  stabstr = parseTypeDef(mod, (&stabstr[cnt+1]), name, ID);
	  cnt = 0;
	  ptrType = mod->moduleTypes->findOrCreateType( ID);
	  if (!current_func) {
	      // XXX-may want to use N_LBRAC and N_RBRAC to set function scope 
	      // -- jdd 5/13/99
	      // Still need to add to local variable list if in a function

	      char modName[100];
	      mod->getName(modName, 99);
	      bperr("%s[%d] Can't find function %s in module %s\n", __FILE__, __LINE__,
		     current_mangled_func_name, modName);
	      bperr("Unable to add %s to local variable list in %s\n",
		     name,current_func_name);
	  } else {
	      locVar = new BPatch_localVar(name, ptrType, linenum, framePtr);
	      if (!ptrType) {
		//bperr("adding local var with missing type %s, type = %d\n",
		//      name, ID);
	      }
	      current_func->localVariables->addLocalVar( locVar);
	  }
	} else if (current_func) {
	  // Try to find the BPatch_Function
	  ptrType = mod->moduleTypes->findOrCreateType( ID);
	  
	  locVar = new BPatch_localVar(name, ptrType, linenum, framePtr);
	  if (!ptrType) {
	    //bperr("adding local var with missing type %s, type = %d\n",
	    //	     name, ID);
	  }
	  current_func->localVariables->addLocalVar( locVar);
	}
    } else if (stabstr[cnt]) {
      BPatch_Vector<BPatch_function *> bpfv;
      switch (stabstr[cnt]) {
	    case 'f': /*Local Function*/ {
	      char *scopeName=NULL;
	      char *lfuncName=NULL;
	      cnt++;

	      current_func_name = name;
	      current_mangled_func_name = mangledname;

	      funcReturnID = parseTypeUse(mod, stabstr, cnt, name);
      
	      if (stabstr[cnt]==',') {
		  cnt++; 	/*skip the comma*/

		  /* Local Function Name */
		  lfuncName = getIdentifier(stabstr, cnt);

		  assert(stabstr[cnt] == ',');
		  cnt++; 	/*skip the comma*/

		  /* Scope Name of Local Function */
		  scopeName = getIdentifier(stabstr, cnt);

		  if (stabstr[cnt]) {
		      bperr("Extra: %s\n", &stabstr[cnt]);
		  }
	      }

	      if (!scopeName) { // Not an embeded function
		  ptrType = mod->moduleTypes->findOrCreateType(funcReturnID);
		  if( !ptrType) ptrType = BPatch::bpatch->type_Untyped;

		  if (NULL == mod->findFunction( name, bpfv ) || !bpfv.size()) {
		    showInfoCallback(pdstring("missing local function ") +
				     pdstring(name) + "\n");
		  } else {
		    if (bpfv.size() > 1) {
		      // warn if we find more than one function with current_func_name
		      bperr("%s[%d]:  WARNING: found %d functions with name %s, using the first",
			     __FILE__, __LINE__, bpfv.size(), name);
		      fp = bpfv[0];
                    }else { // bpfv.size() == 0
                      bperr("%s[%d]:  SERIOUS: found 0 functions with name %s",
                             __FILE__, __LINE__, name);
                      break;
                    }
		    // set return type.
		    fp->setReturnType(ptrType);
		  }
	      } else {
		  bperr("%s is an embedded function in %s\n",name, scopeName);
	      }

	      // skip to end - SunPro Compilers output extra info here - jkh 6/9/3
	      cnt = strlen(stabstr);

	      break;
	  }  

	  case 'F':/* Global Function */
	      cnt++; /*skipping 'F' */

	      funcReturnID = parseTypeUse(mod, stabstr, cnt, name);

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

	      ptrType = mod->moduleTypes->findOrCreateType(funcReturnID);
	      if (!ptrType) ptrType = BPatch::bpatch->type_Untyped;

	      if (NULL == (fp = mod->findFunctionByMangled(current_mangled_func_name))){
		char modName[100];
		mod->getName(modName, 99);
		
		if (NULL == (fp = mangledNameMatchKLUDGE(current_func_name, 
							 current_mangled_func_name, mod))){
		cerr << __FILE__ << __LINE__ << ":  Cannot find function " 
		     << current_mangled_func_name << "/"<< current_func_name 
		     << " with return type " << ptrType->getName() 
		     << " in module " << modName << endl;
		char prefix[5];
		strncpy(prefix, current_mangled_func_name, 4);
		prefix[4] = '\0';
		// mod->dumpMangled(prefix);
		return(&stabstr[cnt]);
		}
	      }

	      fp->setReturnType(ptrType);
	      current_func = fp;
	      break;
	
	  case 'U':/* Class Declaration - for Sun Compilers - jkh 6/6/03 */
	  case 'E':/* Extern'd Global ??? - undocumented type for Sun Compilers - jkh 6/6/03 */
	  case 'G':/* Global Varaible */
	      cnt++; /* skip the 'G' */

	      /* Get variable type number */
	      symdescID = parseTypeUse(mod, stabstr, cnt, name);
	      if (stabstr[cnt]) {
		  bperr( "\tMore to parse - global var %s\n", &stabstr[cnt]);
		  bperr( "\tFull String: %s\n", stabstr);
	      }
	      // lookup symbol and set type
	      BPatch_type *BPtype;
      
	      BPtype = mod->moduleTypes->findOrCreateType(symdescID);
	      if (!BPtype) {
		      bperr("ERROR: unable to find type #%d for variable %s\n", 
		       symdescID, name);
	      } else {
		  /** XXXX - should add local too here **/
		  mod->moduleTypes->addGlobalVariable(name, BPtype);
	      }
	      break;

	  case 'P':	// function parameter passed in a register (GNU/Solaris)
	  case 'R':	// function parameter passed in a register (AIX style)
	  case 'v':	// Fortran Local Variable
	  case 'X':	// Fortran function return Variable (e.g. function name)
          case 'p': {	// Function Parameter
	      cnt++; /* skip the 'p' */

	      /* Get variable type number */
	      symdescID = parseTypeUse(mod, stabstr, cnt, name);

	      if (stabstr[cnt] == ';') {
		  // parameter type information, not used for now
		  cnt = strlen(stabstr);
	      } else if (stabstr[cnt]) {
		  bperr( "\tMore to parse func param %s\n", &stabstr[cnt]);
		  bperr( "\tFull String: %s\n", stabstr);
	      }

	      ptrType = mod->moduleTypes->findOrCreateType(symdescID);
	      if (!ptrType) ptrType = BPatch::bpatch->type_Untyped;

	      BPatch_localVar *param;
	      param = new BPatch_localVar(name, ptrType, linenum, framePtr);
      
	      if (current_func) {
		  current_func->funcParameters->addLocalVar(param);
		  current_func->addParam(name, ptrType, linenum, framePtr);
	      } else {
		  showInfoCallback(pdstring("parameter without local function ") 
				 + pdstring(stabstr) + "\n");
	      }
	      break;
	  }

	  case 'c': /* constant */
	      cnt++; /*move past the 'c' */

	      ptrType = parseConstantUse(mod, stabstr, cnt);
	      if (stabstr[cnt])
		bperr("Parsing Error More constant info to Parse!!: %s\n",
		    &(stabstr[cnt]));

	      if (!ptrType) ptrType = BPatch::bpatch->type_Untyped;

	      BPatch_localVar *var;
	      var = new BPatch_localVar(name, ptrType, linenum, 0);
      
	      if (current_mangled_func_name) {
		if (NULL == (fp = mod->findFunctionByMangled(current_mangled_func_name))){
		  showInfoCallback(pdstring("missing local function ") + 
				   pdstring(current_func_name) + "\n");
		} else { // found function, add parameter
		  current_func = fp;
		  fp->funcParameters->addLocalVar(var);
		  fp->addParam(name, ptrType, linenum, 0);
		}
	      } else {
		showInfoCallback(pdstring("parameter without local function ") 
				 + pdstring(stabstr));
	      }
	      break;

	  case 'r':/* Register Variable */
	      cnt++; /*move past the 'r'*/
	      /* get type reference */

	      symdescID = parseSymDesc(stabstr, cnt);

#ifdef notdef
	      /* often have rNN=*yy - what is this ? jkh 11/30/00 */
	      if (stabstr[cnt])
		bperr("Parsing Error More register info to Parse!!: %s\n",
		    &(stabstr[cnt]));
#endif
	      break;

	  case 'S':/* Global Static Variable */ {
	      cnt++; /*move past the 'S'*/

	      /* get type reference */
	      symdescID = parseTypeUse(mod, stabstr, cnt, name);

	      // lookup symbol and set type
	      BPatch_type *BPtype;
      
	      if (strchr(name, '.')) {
		  char *defaultNameSpace;

		  defaultNameSpace = strdup(name);
		  name = strchr(defaultNameSpace, '.');
		  *name = '\0';
		  mod->setDefaultNamespacePrefix(defaultNameSpace);
		  name++;
	      }

	      BPtype = mod->moduleTypes->findOrCreateType(symdescID);
	      if (!BPtype) {
		      bperr("ERROR: unable to find type #%d for variable %s\n", 
		       symdescID, name);
	      } else {
		  BPatch_image *img = (BPatch_image *) mod->getObjParent();
		  if (img->findVariable(name,false)) {
		      mod->moduleTypes->addGlobalVariable(name, BPtype);
		  }
	      }

	      if (stabstr[cnt])
		bperr("Parsing Error More Global Static info to Parse!!: %s\n",
		       &(stabstr[cnt]));
	      break;
	  }

	  case 't':	// Type Name 
	      cnt++; /*move past the 't'*/

	      /* get type reference */
	      symdescID = parseSymDesc(stabstr, cnt);

	      //Create BPatch_type.
	      if (stabstr[cnt] == '=') {
		/* More Stuff to parse, call parseTypeDef */
		char *oldStr = stabstr;
		stabstr = parseTypeDef(mod, (&stabstr[cnt+1]), name, symdescID);
		cnt = 0;
		// AIX seems to append an semi at the end of these
		if (stabstr[0] && strcmp(stabstr, ";")) {
		    bperr("\tMore to parse creating type %s\n", stabstr);
		  bperr( "\tFull String: %s\n", oldStr);
		}
	      } else {
		//Create BPatch_type defined as a pre-exisitng type.
		ptrType = mod->moduleTypes->findOrCreateType(symdescID);
		if (!ptrType)
		  ptrType = BPatch::bpatch->type_Untyped;
		newType = new BPatch_type(name, symdescID, ptrType);
		if (newType) {
		    newType = mod->moduleTypes->addOrUpdateType(newType);
		}
	      }
	      break;
    
	  case ':':	// :T... - skip ":" and parse 'T'
	      if ((stabstr[cnt+1] == 't') || (stabstr[cnt+1] == 'T')) {
		  // parse as a normal typedef
		  parseStabString(mod, linenum, &stabstr[cnt+1], framePtr);
	      } else {
	          bperr("Unknown type seen %s\n", stabstr);
	      }
	      break;

          case 'T':/* Aggregate type tag -struct, union, enum */
	      cnt++; /*move past the 'T'*/

	      if (stabstr[cnt] == 't') {
		  //C++ struct  tag "T" and type def "t"
		  //bperr("SKipping C++ Identifier t of Tt\n");
		  cnt++;  //skip it
	      }

	      /* get type reference */
	      symdescID = parseSymDesc(stabstr, cnt);
    
	      //Create BPatch_type.
	      if (stabstr[cnt] == '=') {
		  /* More Stuff to parse, call parseTypeDef */
		  stabstr = parseTypeDef(mod,(&stabstr[cnt+1]),name,symdescID);
		  cnt = 0;
		  if (stabstr[0]) {
		      bperr( "\tMore to parse aggregate type %s\n", (&stabstr[cnt]));
		  bperr("\tFull String: %s\n", stabstr);
		  }
	      } else {
		  //Create BPatch_type defined as a pre-exisitng type.
		  newType = new BPatch_type(name, symdescID);
		  if (newType) { newType = mod->moduleTypes->addOrUpdateType(newType); }
	      }

	      break;

	  case 'V':/* Local Static Variable (common block vars too) */
	      cnt++; /*move past the 'V'*/

	      // bperr("parsing 'v' type of %s\n", stabstr);
	      /* Get variable type number */
	      symdescID = parseTypeUse(mod, stabstr, cnt, name);
	      if (stabstr[cnt]) {
		  bperr( "\tMore to parse local static %s\n", &stabstr[cnt]);
		  bperr( "\tFull String: %s\n", stabstr);
	      }
	      // lookup symbol and set type
	      BPtype = mod->moduleTypes->findOrCreateType(symdescID);
	      if (!BPtype) {
		      bperr("ERROR: unable to find type #%d for variable %s\n", 
		       symdescID, name);
		  break;
	      }
	      if (commonBlock) {
		  /* This variable is in a common block */
		  /* add it only if not already there, common block
		     are re-defined for each subroutine but subroutines
		     define only the member they care about
		  */
		  bool found = false;
		  BPatch_Vector<BPatch_field *> *fields;

		  fields = commonBlock->getComponents();
		  if (fields) {
		      for (unsigned int i=0; i < fields->size(); i++) {
			  if (!strcmp((*fields)[i]->getName(), name)) {
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
			      bpwarn("WARN: EQUIVALENCE used in %s: %s and %s\n",
				  current_func_name, name, (*fields)[i]->getName());
			      found = true;
			      break;
			  }
		      }
		  }
		  if (!found) {
		      commonBlock->addField(name, BPatch_dataScalar, BPtype,
			  framePtr, BPtype->getSize());
		  }
	      } else {
		  // put it into the local variable scope
		if (!current_func) {
		  bperr("Unable to add %s to local variable list in %s\n",
			 name,current_func_name);
		} else {
		  locVar = new BPatch_localVar(name, BPtype, linenum, 
					       framePtr, 5, false);
		  current_func->localVariables->addLocalVar( locVar);
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
	    cnt = strlen(stabstr);
	    break;

	  default:
	      bperr( "Unknown symbol descriptor: %c\n", stabstr[cnt]);
	      bperr( " : %s\n", stabstr);
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
char * getIdentifier( char *stabstr, int &cnt, bool stopOnSpace ) {
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
				/* If we're inside a bracket and we haven't reached
				   the end of the string, continue. */
				if( brCnt != 0 && stabstr[ cnt + i ] != '\0' ) {
					i++;
					}
				else if( brCnt ) {
					bperr( "Failed to find identifier in stabstring '%s;\n", stabstr );
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
	
	return identifier;
	} /* end getIdentifier() */

//
// Parse a use of a type.  
//
//	<typeUse> = <symDesc> | <symDesc>=<typeDef>
//
static int parseTypeUse(BPatch_module *mod,char *&stabstr, int &cnt, 
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
static char *parseCrossRef(BPatch_typeCollection *moduleTypes,const char *name,
                           int ID, char *stabstr, int &cnt)
{
    char *temp;
    BPatch_type *newType;
    BPatch_dataClass typdescr = BPatch_dataPointer;
    cnt++; /* skip 'x'*/

    if ((stabstr[cnt] == 's') || 	// struct 
	(stabstr[cnt] == 'u') ||	// union
        (stabstr[cnt] == 'e')) {	// enum
      cnt++;

      temp = getIdentifier(stabstr, cnt);
      cnt++; /*skip ':' */

      // Find type that this one points to.
      BPatch_type *ptrType = moduleTypes->findType(temp);
      if (ptrType) {
	//Create a new B_type that points to a structure
	newType = new BPatch_type(name, ID, typdescr, ptrType);
	// Add to typeCollection
	if(newType) { newType = moduleTypes->addOrUpdateType(newType); }
	if(!newType) {
	  bperr(" Can't Allocate new type ");
	  exit(-1);
	}
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
// 		   ar<symDesc>;<symDesc>;<symDesc>;<arrayDef>
//
static BPatch_type *parseArrayDef(BPatch_module *mod, const char *name,
		     int ID, char *&stabstr, int &cnt)
{
    char *symdesc;
    int symdescID;
    int elementType;
    BPatch_type *newType = NULL;
    BPatch_type *ptrType = NULL;
    int lowbound, hibound;

    // format is ar<indexType>;<lowBound>;<highBound>;<elementType>

    assert(stabstr[cnt] == 'a');

    cnt ++;
    if (stabstr[cnt] != 'r') {
	bperr("unknown array definition seen %s\n", &stabstr[cnt]);
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
	/* Fortran runtime bound array - T0 is the form */
	hibound = 0;
	cnt += 2;
    } else {
	hibound = parseSymDesc(stabstr, cnt);
    }

    cnt++; /* skip semicolon */
    elementType = parseSymDesc(stabstr, cnt);

    if (stabstr[cnt] == 'a') {
	/* multi dimensional array - Fortran style */
	/* it has no valid id, so we give it a known duplicate */
	ptrType = parseArrayDef(mod, name, 0, stabstr, cnt);
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
		//bperr("Skipping C++ rest of array def:  %s\n",name );
		while (stabstr[cnt] != ';') cnt++;
	    }
	}
	ptrType = mod->moduleTypes->findOrCreateType(elementType);
    }

    //  bperr("Symbol Desriptor: %s Descriptor ID: %d Type: %d, Low Bound: %d, Hi Bound: %d,\n", symdesc, symdescID, elementType, lowbound, hibound);


    if (ptrType) {
	// Create new type - field in a struct or union
	newType = new BPatch_type(name, ID, BPatch_dataArray, ptrType,
				 lowbound, hibound);
	if (newType) {
	    // Add to Collection
	    newType = mod->moduleTypes->addOrUpdateType(newType);
	} else {
	    bperr( " Could not create newType Array\n");
	    exit(-1);
	}
    }
	    
    // bperr( "parsed array def to %d, remaining %s\n", cnt, &stabstr[cnt]);
    return (newType);
}

#if defined(i386_unknown_linux2_0) || defined(ia64_unknown_linux2_4) /* temporary duplication - TLM */
//
// parse range type of the form:	
//
//	<rangeType> = r<typeNum>;<low>;<high>;
//
static char *parseRangeType(BPatch_module *mod, const char *name, int ID, 
                            char *stabstr)
{
    int cnt, i;
    //int sign = 1;

    cnt = i = 0;

    assert(stabstr[0] == 'r');
    cnt++;

    BPatch_dataClass typdescr = BPatchSymTypeRange;

    // range index type - not used
    (void) parseSymDesc(stabstr, cnt);

    // bperr("\tSymbol Descriptor: %c and Value: %d\n",tmpchar, symdescID);

    cnt++; /* Discarding the ';' */
    if (stabstr[cnt] == '-' ) cnt++;

    /* Getting type range or size */
    i=0;
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
	BPatch_type *newType = new BPatch_type( name, ID, typdescr, size);
	//Add to Collection
	newType = mod->moduleTypes->addOrUpdateType(newType);
    }
    else {
	//Range
        //Create new type
        BPatch_type *newType = new BPatch_type( name, ID, typdescr, low, hi);
        //Add to Collection
        newType = mod->moduleTypes->addOrUpdateType(newType);
    }

    free(low);
    free(hi);
    hi=low=NULL;

    cnt = cnt + i;
    if( stabstr[cnt] == ';')
      cnt++;
    if( stabstr[cnt] ) {
      bperr( "ERROR: More to parse in type-r- %s\n", &(stabstr[cnt]));
		  bperr( "\tFull String: %s\n", stabstr);
    }
    
    return(&(stabstr[cnt]));
}

#else
//
// parse range type of the form:	
//
//	<rangeType> = r<typeNum>;<low>;<high>;
//
static char *parseRangeType(BPatch_module *mod, const char *name, int ID,
                            char *stabstr)
{
    int cnt, i;

    cnt = i = 0;

    assert(stabstr[0] == 'r');
    cnt++;

    BPatch_dataClass typdescr = BPatchSymTypeRange;

    // range index type - not used
    (void) parseSymDesc(stabstr, cnt);

    // bperr("\tSymbol Descriptor: %c and Value: %d\n",tmpchar, symdescID);

    cnt++; /* Discarding the ';' */
    if (stabstr[cnt] == '-' ) cnt++;

    /* Getting type range or size */
    i=0;
    while (isdigit(stabstr[cnt+i])) i++;

    char *temp = (char *)malloc(sizeof(char)*(i+1));
    if(!strncpy(temp, &(stabstr[cnt]), i))
      /* Error copying size/range*/
      exit(1);
    temp[i] = '\0';
    int j = atol(temp);
    
    if( j <= 0 ){
      /* range */
      char *low = temp;
      temp = NULL;
      
      cnt = cnt + i + 1; /* Discard other Semicolon */
      i = 0;
      if((stabstr[cnt]) == '-')
	i++; /* discard '-' for (long) unsigned int */
      
      while(isdigit(stabstr[cnt+i])){
	i++;
      }
      char *hi = (char *)malloc(sizeof(char)*(i+1));
      if(!strncpy(hi, &(stabstr[cnt]), i))
	/* Error copying upper range */
	exit(1);
      hi[i] = '\0';
      // bperr("\tLower limit: %s and Upper limit: %s\n", low, hi);
      //Create new type
      BPatch_type *newType = new BPatch_type( name, ID, typdescr, low, hi);
      //Add to Collection
      newType = mod->moduleTypes->addOrUpdateType(newType);

      free(low);
      free(hi);
      hi=low=NULL;
      
    } else if( j > 0){
      /*size */
      int size = (int)j;
      
      cnt = cnt + i + 1; /* Discard other Semicolon */
      i = 0;
      while(isdigit(stabstr[cnt+i])){
	i++;
      }
      temp = (char *)malloc(sizeof(char)*(i+1));
      if(!strncpy(temp, &(stabstr[cnt]), i))
	/* Error copying Zero  */
	exit(1);
      temp[i] = '\0';
      
      j = atol(temp);
      free(temp);
      if(j == 0){
	// bperr("\tSize of Type : %d bytes\n",size);
	//Create new type
	BPatch_type *newType = new BPatch_type( name, ID, typdescr, size);
	//Add to Collection
	newType = mod->moduleTypes->addOrUpdateType(newType);
      } else {
	// bperr("Type RANGE: ERROR!!\n");
      }	
    }
    cnt = cnt + i;
    if( stabstr[cnt] == ';')
      cnt++;
#ifdef notdef
    // ranges can now be used as part of an inline typdef
    if( stabstr[cnt] ) {
      bperr( "ERROR: More to parse in type-r- %s\n", &(stabstr[cnt]));
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
static void parseAttrType(BPatch_module *mod, const char *name,
			 int ID, char *stabstr, int &cnt)
{
    bool includesRange = false;
    char *low = NULL, *high = NULL;

    // format @s(size in bits); negative type number;
    BPatch_dataClass typdescr = BPatch_dataTypeAttrib;

    assert(stabstr[cnt] == '@');
    cnt++; /* skip the @ */

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
      BPatch_type *ptrType =BPatch::bpatch->builtInTypes->findBuiltInType(type);
      
      if (!ptrType) ptrType = BPatch::bpatch->type_Untyped;
      
      BPatch_type *newType = new BPatch_type(name, ID, typdescr, size/8, ptrType);
      if(!newType) {
	    bperr(" Can't Allocate new type ");
	    exit(-1);
      }

      if (includesRange) {
	  newType->setLow(low);
	  newType->setHigh(high);
	  free(low);
	  free(high);
      }

      // Add type to collection
      newType = mod->moduleTypes->addOrUpdateType(newType);

      if (stabstr[cnt]) {
	  bperr("More Type Attribute to Parse: %s ID %d : %s\n", name,
	       ID, &(stabstr[cnt]));
	  bperr("got type = %d\n", type);
	  bperr("full string = %s\n", stabstr);
      }
    } else {
	//bperr(" Unable to parse Type Attribute: %s ID %d : %s\n", 
	// name,ID, &(stabstr[cnt]));
    }
}

//
//  <refType> = &<typeUse>
//
static char *parseRefType(BPatch_module *mod, const char *name,
		   int ID, char *stabstr, int &cnt)
{
    /* reference to another type */
    assert(stabstr[cnt] == '&');
    cnt++;
    
    int refID = parseTypeUse(mod, stabstr, cnt, name);
    
    // Create a new B_type that points to a structure
    BPatch_type *ptrType = mod->moduleTypes->findOrCreateType(refID);
    if (!ptrType) ptrType = BPatch::bpatch->type_Untyped;
    
    BPatch_type *newType = new BPatch_type(name, ID, BPatch_dataPointer, ptrType);
    // Add to typeCollection
    if(newType) {
	newType = mod->moduleTypes->addOrUpdateType(newType);
    } else {
        bperr(" Can't Allocate new type ");
        exit(-1);
    }
    
    return(&(stabstr[cnt]));
}

//
// Given a base class and a new type, add all visible fields to the new class
//
void addBaseClassToClass(BPatch_module *mod, int baseID, BPatch_type *newType, int offset)
{

    //Find base class
    BPatch_type *baseCl = mod->moduleTypes->findType(baseID);
    if( ! baseCl ) {
	bperr( "can't find class %d\n", baseID);
	baseCl = mod->moduleTypes->findOrCreateType( baseID );
	baseCl->setDataClass( BPatch_dataStructure );
	newType->addField( "{superclass}", BPatch_dataStructure, baseCl, -1, BPatch_visUnknown );
	return;
    }

    newType->addField( "{superclass}", BPatch_dataStructure, baseCl, -1, BPatch_visUnknown );

    //Get field descriptions of the base type
    BPatch_Vector<BPatch_field *> *baseClFields = baseCl->getComponents();
    for (unsigned int fieldNum=0; fieldNum < baseClFields->size(); fieldNum++) {
	BPatch_field *field = (*baseClFields)[fieldNum];

	if (field->getVisibility() == BPatch_private)
	    continue; //Can not add this member

	newType->addField(field->getName(), field->getTypeDesc(), field->getType(), field->getOffset()+offset, field->getVisibility());
    }
}

//
// parse a list of fields.
//    Format is [A|B|C-M|N|O][c][G]<fieldName>:<type-desc>;offset;size;
//
static char *parseFieldList(BPatch_module *mod, BPatch_type *newType, 
			    char *stabstr, bool sunCPlusPlus)
{
    int cnt = 0;
    int size = 0;
    char *compname;
    int comptype= 0;
    int beg_offset=0;
    BPatch_visibility _vis = BPatch_visUnknown;
    BPatch_dataClass typedescr;


#ifdef IBM_BPATCH_COMPAT_STAB_DEBUG
      bperr( "%s[%d]:  inside parseFieldList\n", __FILE__, __LINE__);
#endif

    if (stabstr[cnt] == '!') {
	//Inheritance definition, Add base class field list to the current one
	//according to visibility rules.

	cnt++; //Skip '!'

	//Get # of base classes
	int baseClNum = atoi(getIdentifier(stabstr, cnt));
	cnt++; //Skip ','

	//Skip information for each base class
	for(int i=0; i<baseClNum; ++i) {
		//Skip virtual inheritance flag, base visibility flag and base offset
		getIdentifier(stabstr, cnt);
		cnt++; //Skip ','

		//Find base class type identifier
		int baseID = parseSymDesc(stabstr, cnt);

		cnt++; //Skip ';'

		addBaseClassToClass(mod, baseID, newType, 0);
	}
     }

     while (stabstr[cnt] && (stabstr[cnt] != ';')) {
	typedescr = BPatch_dataScalar;

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

	compname = getIdentifier(stabstr, cnt);
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
	  typedescr = BPatch_dataMethod;
	  cnt++;
	}

	if ((stabstr[cnt]) == '/') { // visibility C++
	  cnt++; /* get '/' */
	  switch (stabstr[cnt]) {
	  case '0':
	    _vis = BPatch_private;
	    break;
	  case '1':
	    _vis = BPatch_protected;
	    break;
	  case '2':
	    _vis = BPatch_public;
	    break;
	  case '9':
	    _vis = BPatch_optimized;
	    break;
	  default:
	    _vis = BPatch_visUnknown;
	  }
	  cnt++; // get visibility value
	}

	// should be a typeDescriptor
	comptype = parseTypeUse(mod, stabstr, cnt, "");

	if (stabstr[cnt] == ':') {
		cnt++; //Discard ':'

		beg_offset = 0;
		size = 0;
		if (typedescr == BPatch_dataMethod) {
			while(1) {
				//Mangling of arguments
				while(stabstr[cnt] != ';') cnt++;

				cnt++; //Skip ';'
				cnt++; //Skip visibility
				cnt++; //Skip method modifier
				if (stabstr[cnt] == '*') {
					//Virtual fcn definition
					cnt++;
					while(stabstr[cnt] != ';') cnt++; //Skip vtable index
					cnt++; //Skip ';'
					while(stabstr[cnt] != ';') cnt++; //Skip type number to 
									  //the base class
					while(stabstr[cnt] == ';') cnt++; //Skip all ';'
				}
				else if ( (stabstr[cnt] == '.') ||
					  (stabstr[cnt] == '?') )
					cnt++; //Skip '.' or '?'

				if (isSymId(stabstr[cnt])) {
					//Still more to process, but what is this?
					//It seems, it is another fcn definition
					parseTypeUse(mod, stabstr, cnt, "");
					if (stabstr[cnt] == ':') 
						cnt++; //Discard ':'
				}
				else {
					if (stabstr[cnt] == '~')
						cnt--; //Get back to ';'
					else if (stabstr[cnt] == ';') {
						//Skip all ';' except last one or two
						while(stabstr[cnt] == ';') cnt++;
						if (!stabstr[cnt] || (stabstr[cnt] == ',') )
							cnt--; //There must be two ';'
						cnt--;
					}
					else {
						//Something wrong! Skip entire stab record and exit
						while(stabstr[cnt]) cnt++;
						return (&stabstr[cnt]);
					}

					break;
				}
			} //While 1
			if (!stabstr[cnt]) {
			    bperr("error got to end of %s\n", stabstr);
			}
		}
		else {
			//Static member var
			char *varName = getIdentifier(stabstr, cnt);
			free(varName);
			//Don't know what to do!
		}
	}
	else if (stabstr[cnt] == ',') {
		assert(stabstr[cnt] == ',');
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
	bperr( "%s[%d]:  asserting last char of stabstr == ';':  stabstr = %s\n", 
		__FILE__, __LINE__, stabstr);
#endif
	
	if (stabstr[cnt] == ';') // jaw 03/15/02-- major kludge here for DPCL compat
	  cnt++;  // needs further examination
	// bperr("\tType: %d, Starting Offset: %d (bits), Size: %d (bits)\n", comptype, beg_offset, size);
	// Add struct field to type
	BPatch_type *fieldType = mod->moduleTypes->findOrCreateType( comptype );
	if (fieldType == NULL) {
		//C++ compilers may add extra fields whose types might not available.
		//Assign void type to these kind of fields. --Mehmet
		fieldType = mod->moduleTypes->findType("void");
	}
	if (_vis == BPatch_visUnknown) {
	    newType->addField(compname, typedescr, fieldType,
			    beg_offset, size);
	} else {
	    // bperr( "Adding field '%s' to type '%s' @ 0x%x\n", compname, newType->getName(), newType );
	    newType->addField(compname, typedescr, fieldType,
			    beg_offset, size, _vis);
	    //bperr("Adding Component with VISIBILITY STRUCT\n");
	}
    }

    // should end with a ';'
    if (stabstr[cnt] == ';') {
	return &stabstr[cnt+1];
    } else if (stabstr[cnt] == '\0') {
	return &stabstr[cnt];
    } else {
	bperr("invalid stab record: %s\n", &stabstr[cnt]);
	abort();
    }
}


//
//	Y<type><size><className>;<Bases>;<DataMembers>;<MemberFunctions>;<StaticDataMembers>;
//		<Friends>;<VirtualFunctionInfo>;<NestedClassList>;<AccessAdjustments>;
//		<VirtualBaseClassOffsets>;<TemplatmentMembers>;<PassMethod>;
//
static char *parseCPlusPlusInfo(BPatch_module *mod,
	     char *stabstr, const char *mangledName, int ID)
{
    int cnt;
    char *name;
    int structsize;
    bool sunStyle = true;
    bool nestedType = false;
    BPatch_dataClass typdescr;
    BPatch_type * newType = NULL;

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
	    typdescr = BPatch_dataTypeClass;
	    break;

	case 'S':
	    nestedType = true;
	case 's':
	    typdescr = BPatch_dataStructure;
	    break;

	case 'U':
	    nestedType = true;
	case 'u':
	    typdescr = BPatch_dataUnion;
	    break;

        case 'n':	// namespace - ignored
	    cnt = strlen(stabstr);
	    return(&(stabstr[cnt]));
	    break;

	default:
	    bperr( "ERROR: Unrecognized C++ str = %s\n", stabstr);
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

    //Create new type
    newType = new BPatch_type(name, ID, typdescr, structsize);
    //add to type collection
    newType = mod->moduleTypes->addOrUpdateType(newType);

    if (sunStyle) {
	cnt++;
	// base class(es) 
	while (stabstr[cnt] != ';') {
	    // skip visibility flag
	    cnt++;

	    int offset = parseSymDesc(stabstr, cnt);

	    // Find base class type identifier
	    int baseID = parseSymDesc(stabstr, cnt);
	    addBaseClassToClass(mod, baseID, newType, offset);
	}

	cnt++;	// skip ;
    }

    // parse dataMembers
    stabstr = parseFieldList(mod, newType, &stabstr[cnt], sunStyle);
    cnt = 0;

    if (stabstr[0]) {
	// parse member functions
	cnt++;
	while (stabstr[cnt] && (stabstr[cnt] != ';')) {
	    char *funcName = getIdentifier(stabstr, cnt, true);

	    funcName++;	// skip ppp-code

	    if (*funcName == '-') funcName++; // it's a pure vitual

	    while (isdigit(*funcName)) funcName++; // skip virtual function index
	    funcName++;

	    char *className = strdup(currentRawSymbolName);
	    className[3] = 'c';
	    className[strlen(className)-1] = '\0';	// remove tailing "_"
	    pdstring methodName = pdstring(className) + pdstring(funcName) + pdstring("_");
		char * name = P_cplus_demangle( methodName.c_str(), mod->isNativeCompiler() );
		if( name != NULL ) {
			funcName = strrchr( name, ':' );
			if( funcName ) { funcName++; }
			else { funcName = name; }
			}

	    // should include position for virtual methods
	    BPatch_type *fieldType = mod->moduleTypes->findType("void");
	    newType->addField(funcName, BPatch_dataMethod, fieldType, 0, 0);

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
static char *parseTypeDef(BPatch_module *mod, char *stabstr, 
                          const char *name, int ID)
{

    BPatch_type * newType = NULL;
    BPatch_type * ptrType = NULL;
  
    char * temp=NULL;
    char * compsymdesc=NULL;
  
    BPatch_dataClass typdescr;
    int ptrID=0;
    
    int value;
    int cnt,i,j,k;
    int structsize;
    int type;
    cnt = i = j = k = 0;
  
#ifdef IBM_BPATCH_COMPAT_STAB_DEBUG
    bperr( "%s[%d]:  inside parseTypeDef, stabstr = %s\n", __FILE__, __LINE__, 
	    (stabstr == NULL) ? "NULL" : stabstr);
#endif

    assert (stabstr[0] != '=');

    // bperr( "parsing %s\n", stabstr);
    if (isSymId(stabstr[0])) {
	typdescr = BPatch_dataScalar;
	type = parseSymDesc(stabstr, cnt);
	 	    
	if (ID == type) {
	    newType = new BPatch_type( name, ID, typdescr, type);
	    if (newType) { newType = mod->moduleTypes->addOrUpdateType(newType); }
	    if(!newType) {
	      bpfatal(" Can't Allocate newType ");
	      exit(-1);
	    }
	} else if (stabstr[cnt] == '=') {
	    // XXX - in the new type t(0,1)=(0,2)=s... is possible
	    // 	     skip the second id for now -- jkh 3/21/99
	    stabstr = parseTypeDef(mod, &(stabstr[cnt+i+1]), name, type);
	    cnt = 0;
	    BPatch_type *oldType;
	    
	    oldType = mod->moduleTypes->findOrCreateType(type);
	    if(!oldType) oldType = BPatch::bpatch->type_Untyped;
	    newType = new BPatch_type( name, ID, typdescr, oldType);
	    if (newType) { newType = mod->moduleTypes->addOrUpdateType(newType); }
	} else {
	    BPatch_type *oldType;
	    
	    oldType = mod->moduleTypes->findOrCreateType(type);
	    newType = new BPatch_type( name, ID, typdescr, oldType);
	    if(newType) { newType = mod->moduleTypes->addOrUpdateType(newType); }
	    if(!newType) {
	        bpfatal(" Can't Allocate newType ");
	        exit(-1);
	    }
	}
    } else {
      switch (stabstr[0]) {
	  case 'x':  //cross reference 
	    parseCrossRef(mod->moduleTypes, name, ID, stabstr, cnt);
	    break;
	     
	  case '*':
	    typdescr = BPatch_dataPointer;
	    /* pointer to another type */
	    cnt++;
	    ptrID = parseTypeUse(mod, stabstr, cnt, NULL);

	    // Create a new B_type that points to a structure
	    ptrType = mod->moduleTypes->findOrCreateType(ptrID);
	    if (!ptrType) ptrType = BPatch::bpatch->type_Untyped;

	    newType = new BPatch_type(NULL, ID, BPatch_dataPointer, ptrType);
	    // Add to typeCollection
	    if(newType) { newType = mod->moduleTypes->addOrUpdateType(newType); }
	    if(!newType) {
		bpfatal(" Can't Allocate new type ");
		exit(-1);
	    }

	    return(&(stabstr[cnt]));
	    break;

	  case 'a':
	      (void) parseArrayDef(mod, name, ID, stabstr, cnt);
	      return (&stabstr[cnt]);
	      break;

          case 'g':  // We really should user parameter types (JMO && JKH)
	        /* function with return type and prototype */

		// g<typeUse>[<typeUse>]*#
		typdescr = BPatch_dataFunction;

		cnt++; /* skip the f */
	        type = parseTypeUse(mod, stabstr, cnt, name);

		while ((stabstr[cnt] != '#') &&  (stabstr[cnt])) {
		    int paramType;
		    paramType = parseTypeUse(mod, stabstr, cnt, name);
		}

		// skip #
		if (stabstr[cnt] == '#') cnt++;
		break;

	  case 'f':
	        /* function type */
		typdescr = BPatch_dataFunction;

		cnt++; /* skip the f */
	        type = parseTypeUse(mod, stabstr, cnt, name);

		// skip to end - SunPro Compilers output extra info here - jkh 6/9/3
		cnt = strlen(stabstr);

		break;

	 case 'M': {
		/* CHARACTER ??? */
		cnt++; // skip  'M'

		int baseType = parseSymDesc(stabstr, cnt);
		if (baseType != -2 || (stabstr[cnt] != ';')) {
		    bperr("unexpected non character array %s\n", stabstr);
		} else {
		    cnt++; // skip ';'
		    int size;
		    if (stabstr[cnt] == 'T') {
		      /* Fortran stack-based array bounds */
		      size = 0;
		      cnt++; // skip ';'
		      (void) parseSymDesc(stabstr, cnt);
		    } else
		      size = parseSymDesc(stabstr, cnt);

		    ptrType = mod->moduleTypes->findOrCreateType(baseType);
		    newType = new BPatch_type(name, ID, BPatch_dataArray, ptrType,
			1, size);
		    newType = mod->moduleTypes->addOrUpdateType(newType);
		}
		break;

	 }
	 case 'R': {
		// Define a floating point type - R fp-type; bytes;
		cnt++;
		(void) parseSymDesc(stabstr, cnt);
		cnt ++;

		int bytes = parseSymDesc(stabstr, cnt);

		newType = new BPatch_type(name, ID, BPatch_dataBuilt_inType, bytes);
		newType = mod->moduleTypes->addOrUpdateType(newType);

		if (stabstr[cnt] == ';') cnt++;	// skip the final ';'

		// gcc 3.0 adds an extra field that is always 0 (no indication in the code why)
		if (stabstr[cnt] == '0') cnt += 2;	// skip the final '0;'

		break;
	  }

	  case 'b': {
		// builtin type b  - signed char-flag width; offset; nbits
		typdescr = BPatch_dataBuilt_inType;
		int limit = strlen(&stabstr[cnt]);

		// skip to width
		while (stabstr[cnt+i] != ';' && (i < limit)) i++;	
		if (i >= limit) return(&(stabstr[cnt]));

		i++;	// skip the ';'

		// skip to offset
		while (stabstr[cnt+i] != ';' && (i < limit)) i++;	
		if (i >= limit) return(&(stabstr[cnt]));
		i++;

		cnt += i;
		int nbits = parseSymDesc(stabstr, cnt);
		
		if (stabstr[cnt]) cnt++;	// skip the final ';'

		newType = new BPatch_type(name, ID, BPatch_dataBuilt_inType, 
		    nbits/8);
		//Add to Collection
		newType = mod->moduleTypes->addOrUpdateType(newType);

		return &stabstr[cnt];
		break;
	    }

	case 'r': 		// range type
	    return parseRangeType(mod, name, ID, stabstr);
	    break;

	case 'e':		// Enumerated type
	    typdescr = BPatch_dataEnumerated;
	    cnt++; 	/* skip the 'e' */

	    // Create new Enum type
	    newType = new BPatch_type(name, ID, typdescr);
	    // Add type to collection
	    newType = mod->moduleTypes->addOrUpdateType(newType);
		
	    while (stabstr[cnt]) {
		/* Get enum component value */
		compsymdesc = getIdentifier(stabstr, cnt);
		cnt++; /* skip colon */

#ifdef IBM_BPATCH_COMPAT_STAB_DEBUG
		bperr( "%s[%d]:  before parseSymDesc -- enumerated type \n", 
			__FILE__, __LINE__);
#endif		  
		value = parseSymDesc(stabstr, cnt);

		// add enum field to type
		newType->addField(compsymdesc, BPatch_dataScalar, value);
		  
		free(temp);
		free(compsymdesc);
		temp = compsymdesc = NULL;

		cnt++; /* skip trailing comma */
		if ((stabstr[cnt]) == ';') cnt++; /* End of enum stab record */
	    }
	    break;
	    
        case '@':  // type attribute, defines size and type (negative num)
	    parseAttrType(mod, name, ID, stabstr, cnt);
	    break;

        case '&': //XXX- Need to complete, may be more to parse jdd 4/22/99
	    return parseRefType(mod, name, ID, stabstr, cnt);
	    break;

        case 'k':	// Sun constant type s<typeDef> - parse as <typeDef>
	    return parseTypeDef(mod, &stabstr[cnt+1], name, ID);
	    break;

	case 'V':	// AIX colatile ? type V<typeDef> - parse as <typeDef>
        case 'B':	// Sun volatile type B<typeDef> - parse as <typeDef>
	    return parseTypeDef(mod, &stabstr[cnt+1], name, ID);
	    break;

	case 's':	// struct
	case 'u':	// union
	    /* Type descriptor */
	    if (stabstr[cnt] == 's') {
	        typdescr = BPatch_dataStructure;
	    } else {
	        typdescr = BPatch_dataUnion;
	    }

	    cnt++;		// skip to size
	    structsize = parseSymDesc(stabstr, cnt);
	    
	    //Create new type
	    newType = new BPatch_type(name, ID, typdescr, structsize);
	    //add to type collection
	    newType = mod->moduleTypes->addOrUpdateType(newType);
#ifdef IBM_BPATCH_COMPAT_STAB_DEBUG
	    bperr( "%s[%d]:  before parseFieldList -- strlen(&stabstr[%d]) =%d \n", 
		    __FILE__, __LINE__, cnt, strlen(&stabstr[cnt]));
#endif
	    return parseFieldList(mod, newType, &stabstr[cnt], false);

	    break;

	case 'Y':
	    // C++ specific stabs (Sun compiler)
	    return parseCPlusPlusInfo(mod, stabstr, name, ID);
	    break;


	case 'Z':  // What is this ??? - jkh 10/14/99 (xlc compiler uses it)
	    return (&stabstr[1]);
	    break;

	case '#':
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

	default:
	    bperr( "ERROR: Unrecognized str = %s\n", &stabstr[cnt]);
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
static BPatch_type *parseConstantUse(BPatch_module *mod, char *stabstr, int &cnt)
{
    // skip =
    cnt++;

    BPatch_type *ret;

    if (stabstr[cnt] == 'i') {
	ret = mod->moduleTypes->findType("integer*4");
    } else if (stabstr[cnt] == 'r') {
	ret = mod->moduleTypes->findType("double");
    } else {
	bperr("unknown constant type %s\n", &stabstr[cnt]);
	ret = NULL;
    }

    cnt = strlen(stabstr);

    return ret;
}


