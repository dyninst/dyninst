/*
 * Copyright (c) 1996 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as Paradyn") on an AS IS basis, and do not warrant its
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

#include "BPatch.h"
#include "BPatch_module.h"
#include "BPatch_collections.h"
#include "showerror.h"

extern char *current_func_name;

// Forward references for parsing routines
static int parseSymDesc(char *stabstr, int &cnt);
static char *parseTypeDef(BPatch_module*, char *stabstr, char *name, int ID);
static int parseTypeUse(BPatch_module*, char *&stabstr, int &cnt, char *name);
static inline bool isSymId(char ch);
static char *getIdentifier(char *stabstr, int &cnt);

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

// This function takes the stab stabstring and parses it to create a new 
// type or variable object.  This function only defines the type/variable 
// name and ID.
//
// <stabString> = <ident>:<symDesc> 			|
//		  <ident>:f<symDesc> 			|
//		  <ident>:f<syymDesc>,<ident>,<ident> 	|
//		  <ident>:F<typeUse><paramList> 	|
//		  <ident>:G<typeUse> 			|
//		  <ident>:r<int> 			|
//		  <ident>:S<symDesc>			|
//		  <ident>:[pPr]<typeUse>		|
//		  <ident>::T<typeUse>			|
//		  <ident>:t<typeUse>			|
//		  <ident>:T<typeUse>			|
//		  <ident>:v<typeUse>			|
//		  <ident>:V<symDesc>
//
// <paramList> = | <typeUse>;<paramList> 
//
char *parseStabString(BPatch_module *mod, int linenum, char *stabstr, 
		      int framePtr)
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
    char *name = getIdentifier(stabstr, cnt);

    if (*name) {
       // non-null string
       // AIX puts out some symbols like this eg .bs
       if (stabstr[cnt] != ':') {
	   return name;
       }
    }
    if (stabstr[cnt] == ':') {
       // skip to type part
       cnt++;
    }

    if (isSymId(stabstr[cnt])) {
	/* instance of a predefined type */

	ID = parseSymDesc(stabstr, cnt);
	
	//printf("Variable: %s  Type ID: %d, file ID %d \n",name, ID, file_ID);
	if (stabstr[cnt] == '=') {
	  /* More Stuff to parse, call parseTypeDef */
	  stabstr = parseTypeDef(mod, (&stabstr[cnt+1]), name, ID);
	  cnt = 0;
	  ptrType = mod->moduleTypes->findType( ID);
	  if (current_func_name) {
	    // XXX-may want to use N_LBRAC and N_RBRAC to set function scope 
	    // -- jdd 5/13/99
	    // Still need to add to local variable list if in a function
	    fp = mod->findFunction( current_func_name );
	    if (!fp) {
		char modName[100];
		mod->getName(modName, 99);
		printf(" Can't find function %s in module %s\n", current_func_name, modName);
		printf("Unable to add %s to local variable list in %s\n",
		       name,current_func_name);
	    } else {
	        locVar = new BPatch_localVar(name, ptrType, linenum, framePtr);
	        fp->localVariables->addLocalVar( locVar);
	    }
	  }
	} else if (current_func_name) {
	  // Try to find the BPatch_Function
	  ptrType = mod->moduleTypes->findType( ID);
	  fp = mod->findFunction(current_func_name);
	  if (!fp) {
	      char modName[100];
	      mod->getName(modName, 99);
	      printf(" Can't find function in BPatch_function vector: %s in module %s\n",
		   current_func_name, modName);
	  } else {
	      locVar = new BPatch_localVar(name, ptrType, linenum, framePtr);
	      fp->localVariables->addLocalVar( locVar);
	  }
	}
    } else if (stabstr[cnt]) {
	switch (stabstr[cnt]) {
	    case 'f': /*Local Function*/ {
	      char *scopeName=NULL;
	      char *lfuncName=NULL;
	      cnt++;
	      current_func_name = name;
	      // funcReturnID = parseSymDesc(stabstr, cnt);
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
		      fprintf(stderr, "Extra: %s\n", &stabstr[cnt]);
		  }
	      }

	      if (!scopeName) { // Not an embeded function
		  ptrType = mod->moduleTypes->findType(funcReturnID);
		  if( !ptrType) ptrType = BPatch::bpatch->type_Untyped;

		  fp = mod->findFunction( name );
		  if (!fp) {
		      showInfoCallback(string("missing local function ") +
			  string(name));
		  } else {
		      // set return type.
		      fp->setReturnType(ptrType);
		  }
	      } else {
		  printf("%s is an embedded function in %s\n",name, scopeName);
	      }
	      break;
	  }  

	  case 'F':/* Global Function */
	      cnt++; /*skipping 'F' */

	      funcReturnID = parseTypeUse(mod, stabstr, cnt, name);
	      current_func_name = name;

	      //
	      // For SunPro compilers there may be a parameter list after 
	      //   the return
	      //
	      while (stabstr[cnt] == ';') {
		  cnt++;	// skip ';'
		  (void) parseTypeUse(mod, stabstr, cnt, "");
	      }

	      ptrType = mod->moduleTypes->findType(funcReturnID);
	      if (!ptrType) ptrType = BPatch::bpatch->type_Untyped;

	      fp = mod->findFunction( name );
	      if (!fp) {
		  // for FORTRAN look with trailing _
		  char tempName[strlen(name)+2];
		  sprintf(tempName, "%s_", name);

		  // this leaks tempName XXXX 
		  current_func_name = strdup(tempName);

		  fp = mod->findFunction(tempName);
		  if (!fp) {
		      showInfoCallback(string("missing local function ") + 
			  string(name));
		  }
	      }

	      if (fp) {
		  // set return type.
		  fp->setReturnType(ptrType);
	      }
	      break;

	  case 'G':/* Global Varaible */
	      cnt++; /* skip the 'G' */

	      /* Get variable type number */
	      symdescID = parseTypeUse(mod, stabstr, cnt, name);
	      if (stabstr[cnt]) 
		  fprintf(stderr, "\tMore to parse %s\n", &stabstr[cnt]);

	      // lookup symbol and set type
	      BPatch_type *BPtype;
      
	      BPtype = mod->moduleTypes->findType(symdescID);
	      if (!BPtype) {
		  fprintf(stderr, 
		      "ERROR: unable to find type #%d for variable %s\n", 
		       symdescID, name);
	      } else {
		  mod->moduleTypes->addGlobalVariable(name, BPtype);
	      }
	      break;

	  case 'R':	// function parameter passed in a register (AIX style)
	  case 'P':	// function parameter passed in a register (GNU/Solaris)
	  case 'v':	// Fortran Local Variable
          case 'p': {	// Function Parameter
	      cnt++; /* skip the 'p' */

	      /* Get variable type number */
	      symdescID = parseTypeUse(mod, stabstr, cnt, name);

	      if (stabstr[cnt] == ';') {
		  // parameter type information, not used for now
		  cnt = strlen(stabstr);
	      } else if (stabstr[cnt]) {
		  fprintf(stderr, "\tMore to parse %s\n", &stabstr[cnt]);
	      }

	      ptrType = mod->moduleTypes->findType(symdescID);
	      if (!ptrType) ptrType = BPatch::bpatch->type_Untyped;

	      BPatch_localVar *param;
	      param = new BPatch_localVar(name, ptrType, linenum, framePtr);
      
	      if (current_func_name) {
		  fp = mod->findFunction( current_func_name );
		  if (!fp) {
		      showInfoCallback(string("missing local function ") + 
			 string(current_func_name));
		  } else { // found function, add parameter
		      fp->funcParameters->addLocalVar(param);
		      fp->addParam(name, ptrType, linenum, framePtr);
		  }
	      } else {
		  showInfoCallback(string("parameter without local function ") 
		       + string(stabstr));
	      }
	      break;
	  }

	  case 'r':/* Register Variable */
	      cnt++; /*move past the 'r'*/
	      /* get type reference */

	      symdescID = parseSymDesc(stabstr, cnt);

	      if (stabstr[cnt])
		printf("Parsing Error More register info to Parse!!: %s\n",
		    &(stabstr[cnt]));
	      break;

	  case 'S':/* Global Static Variable */
	      cnt++; /*move past the 'S'*/

	      /* get type reference */
	      symdescID = parseSymDesc(stabstr, cnt);

	      if (stabstr[cnt])
		printf("Parsing Error More Global Static info to Parse!!: %s\n",
		       &(stabstr[cnt]));
	      break;

	  case 't':	// Type Name 
	      cnt++; /*move past the 't'*/

	      /* get type reference */
	      symdescID = parseSymDesc(stabstr, cnt);

	      //Create BPatch_type.
	      if (stabstr[cnt] == '=') {
		/* More Stuff to parse, call parseTypeDef */
		stabstr = parseTypeDef(mod, (&stabstr[cnt+1]), name, symdescID);
		cnt = 0;
		// AIX seems to append an semi at the end of these
		if (stabstr[0] && strcmp(stabstr, ";")) {
		    fprintf(stderr, "\tMore to parse %s\n", stabstr);
		}
	      } else {
		//Create BPatch_type defined as a pre-exisitng type.
		ptrType = mod->moduleTypes->findType(symdescID);
		if (!ptrType)
		  ptrType = BPatch::bpatch->type_Untyped;
		newType = new BPatch_type(name, symdescID, ptrType);
		if (newType) {
		    mod->moduleTypes->addType(newType);
		}
	      }
	      break;
    
	  case ':':	// :T... - skip ":" and parse 'T'
	      if ((stabstr[cnt+1] == 't') || (stabstr[cnt+1] == 'T')) {
		  // parse as a normal typedef
		  parseStabString(mod, linenum, &stabstr[cnt+1], framePtr);
	      } else {
	          fprintf(stderr, "Unknown type seen %s\n", stabstr);
	      }
	      break;

          case 'T':/* Aggregate type tag -struct, union, enum */
	      cnt++; /*move past the 'T'*/

	      if (stabstr[cnt] == 't') {
		  //C++ struct  tag "T" and type def "t"
		  //printf("SKipping C++ Identifier t of Tt\n");
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
		      fprintf(stderr, "\tMore to parse %s\n", (&stabstr[cnt]));
		  }
	      } else {
		  //Create BPatch_type defined as a pre-exisitng type.
		  newType = new BPatch_type(name, symdescID);
		  if (newType) mod->moduleTypes->addType(newType);
	      }

	      break;

	  case 'V':/* Local Static Variable */
	      cnt++; /*move past the 'V'*/

	      /* get type reference */
	      symdescID = parseSymDesc(stabstr, cnt);
     
	      if (stabstr[cnt])
		printf("Parsing Error More Local Static info to Parse!!: %s\n",
		       &(stabstr[cnt]));
	      break;

	  default:
	      fprintf(stderr, "Unknown symbol descriptor: %c\n", stabstr[cnt]);
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

    // printf("\n    parseSymDesc call on %s\n", &stabstr[cnt+i]);

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
char *getIdentifier(char *stabstr, int &cnt)
{
    int i = 0;
    char *ret;
    int brCnt = 0;
    bool idChar = true;

    while(idChar) {
	switch(stabstr[cnt+i]) {
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

	case '\0':
	case ':':
	case ',':
	case ';':
		if (brCnt)
			i++;
		else
			idChar = false;
		break;
	default:
		i++;
		break;
	}
    }

    ret = (char *) malloc(i+1);
    assert(ret);

    strncpy(ret, &stabstr[cnt], i);
    ret[i] = '\0';
    cnt += i;

    return ret;
}

//
// Parse a use of a type.  
//
//	<typeUse> = <symDesc> | <symDesc>=<typeDef>
//
static int parseTypeUse(BPatch_module *mod,char *&stabstr, int &cnt, char *name)
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
static char *parseCrossRef(BPatch_typeCollection *moduleTypes, char *name,
		    int ID, char *stabstr, int &cnt)
{
    char *temp;
    BPatch_type *newType;
    BPatch_dataClass typdescr = BPatch_pointer;
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
	if(newType)
	  moduleTypes->addType(newType);
	if(!newType) {
	  printf(" Can't Allocate new type ");
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
static char *parseArrayDef(BPatch_module *mod, char *name,
		     int ID, char *stabstr, int &cnt)
{
    char *symdesc;
    int symdescID;
    int elementType;
    int lowbound, hibound;

    assert(stabstr[cnt] == 'a');

    cnt ++;
    if (stabstr[cnt] == 'r') {
        // format is ar<indexType>;<lowBound>;<highBound>;<elementType>
        /* array with range */
        symdesc = &(stabstr[cnt]);

        cnt++;	/* skip 'r' */
        symdescID = parseSymDesc(stabstr, cnt);
     
        cnt++; /* skip semicolon */
        lowbound = parseSymDesc(stabstr, cnt);

        cnt++; /* skip semicolon */
        hibound = parseSymDesc(stabstr, cnt);

        cnt++; /* skip semicolon */
        elementType = parseSymDesc(stabstr, cnt);

        /* multi dimensional array */
        if (stabstr[cnt] == '=') {
	    stabstr = parseTypeDef(mod, &(stabstr[cnt+1]), NULL, elementType);
	    cnt = 0;
	    if (stabstr[cnt] == ':') {
		//C++ stuff
	        //printf("Skipping C++ rest of array def:  %s\n",name );
	        while (stabstr[cnt] != ';') cnt++;
	    }
        }
    }

    // fprintf(stderr, "Symbol Desriptor: %s Descriptor ID: %d Type: %d, Low Bound: %d, Hi Bound: %d,\n", symdesc, symdescID, elementType, lowbound, hibound);


    BPatch_type *ptrType = mod->moduleTypes->findType(elementType);
    if (ptrType) {
	// Create new type - field in a struct or union
	BPatch_type *newType = new BPatch_type(name, ID, BPatch_array, ptrType,
				 lowbound, hibound);
	if (newType) {
	    // Add to Collection
	    mod->moduleTypes->addType(newType);
	} else {
	    fprintf(stderr, " Could not create newType Array\n");
	    exit(-1);
	}
    }
	    
    // fprintf(stderr, "parsed array def to %d, remaining %s\n", cnt, &stabstr[cnt]);
    return (&stabstr[cnt]);
}

#if defined(i386_unknown_linux2_0)
//
// parse range type of the form:	
//
//	<rangeType> = r<typeNum>;<low>;<high>;
//
static char *parseRangeType(BPatch_module *mod,char *name,int ID, char *stabstr)
{
    int cnt, i;
    int sign = 1;

    cnt = i = 0;

    assert(stabstr[0] == 'r');
    cnt++;

    BPatch_dataClass typdescr = BPatchSymTypeRange;

    // range index type - not used
    (void) parseSymDesc(stabstr, cnt);

    // printf("\tSymbol Descriptor: %c and Value: %d\n",tmpchar, symdescID);

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
	mod->moduleTypes->addType(newType);
    }
    else {
	//Range
        //Create new type
        BPatch_type *newType = new BPatch_type( name, ID, typdescr, low, hi);
        //Add to Collection
        mod->moduleTypes->addType(newType);
    }

    free(low);
    free(hi);
    hi=low=NULL;

    cnt = cnt + i;
    if( stabstr[cnt] == ';')
      cnt++;
    if( stabstr[cnt] ) {
      fprintf(stderr, "ERROR: More to parse in type-r- %s\n", &(stabstr[cnt]));
    }
    
    return(&(stabstr[cnt]));
}

#else
//
// parse range type of the form:	
//
//	<rangeType> = r<typeNum>;<low>;<high>;
//
static char *parseRangeType(BPatch_module *mod,char *name,int ID, char *stabstr)
{
    int cnt, i;

    cnt = i = 0;

    assert(stabstr[0] == 'r');
    cnt++;

    BPatch_dataClass typdescr = BPatchSymTypeRange;

    // range index type - not used
    (void) parseSymDesc(stabstr, cnt);

    // printf("\tSymbol Descriptor: %c and Value: %d\n",tmpchar, symdescID);

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
      // printf("\tLower limit: %s and Upper limit: %s\n", low, hi);
      //Create new type
      BPatch_type *newType = new BPatch_type( name, ID, typdescr, low, hi);
      //Add to Collection
      mod->moduleTypes->addType(newType);

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
	// printf("\tSize of Type : %d bytes\n",size);
	//Create new type
	BPatch_type *newType = new BPatch_type( name, ID, typdescr, size);
	//Add to Collection
	mod->moduleTypes->addType(newType);
      } else {
	// printf("Type RANGE: ERROR!!\n");
      }	
    }
    cnt = cnt + i;
    if( stabstr[cnt] == ';')
      cnt++;
    if( stabstr[cnt] ) {
      fprintf(stderr, "ERROR: More to parse in type-r- %s\n", &(stabstr[cnt]));
    }
    
    return(&(stabstr[cnt]));
}

#endif

//
//  <attrType> = @s<int>;<int>
//
static void parseAttrType(BPatch_module *mod, char *name,
			 int ID, char *stabstr, int &cnt)
{
    // format @s(size in bits); negative type number;
    BPatch_dataClass typdescr = BPatch_typeAttrib;

    assert(stabstr[cnt] == '@');
    cnt++; /* skip the @ */

    if (stabstr[cnt] == 's') {
      cnt++;
      
      int size = parseSymDesc(stabstr, cnt);
      cnt++;  // skip ';'

      int type = parseSymDesc(stabstr, cnt);
      // skip ';' end of stab record ??? (at least for bool)
      cnt++;

      // Create a new B_type that points to a builtInTypes
      BPatch_type *ptrType =BPatch::bpatch->builtInTypes->findBuiltInType(type);
      
      if (!ptrType) ptrType = BPatch::bpatch->type_Untyped;
      
      BPatch_type *newType = new BPatch_type(name, ID, typdescr, size/8, ptrType);
      
      // Add type to collection
      if(newType) mod->moduleTypes->addType(newType);
      if(!newType) {
	    printf(" Can't Allocate new type ");
	    exit(-1);
      }
      
      if (stabstr[cnt]) {
	  printf("More Type Attribute to Parse: %s ID %d : %s\n", name,
	       ID, &(stabstr[cnt]));
      }
    } else {
	//printf(" Unable to parse Type Attribute: %s ID %d : %s\n", 
	// name,ID, &(stabstr[cnt]));
    }
}

//
//  <refType> = &<typeUse>
//
static char *parseRefType(BPatch_module *mod, char *name,
		   int ID, char *stabstr, int &cnt)
{
    /* reference to another type */
    assert(stabstr[cnt] == '&');
    cnt++;
    
    int refID = parseTypeUse(mod, stabstr, cnt, name);
    
    // Create a new B_type that points to a structure
    BPatch_type *ptrType = mod->moduleTypes->findType(refID);
    if (!ptrType) ptrType = BPatch::bpatch->type_Untyped;
    
    BPatch_type *newType = new BPatch_type(name, ID, BPatch_pointer, ptrType);
    // Add to typeCollection
    if(newType) {
	mod->moduleTypes->addType(newType);
    } else {
        printf(" Can't Allocate new type ");
        exit(-1);
    }
    
    return(&(stabstr[cnt]));
}

//
// parse a list of fields.
//    Format is <fieldName>:<type-desc>;offset;size;
//
static char *parseFieldList(BPatch_module *mod, BPatch_type *newType, 
			    char *stabstr)
{
    int size;
    int cnt = 0;
    int beg_offset;
    char *compname;
    int comptype= 0;
    BPatch_visibility _vis = BPatch_visUnknown;
    BPatch_dataClass typedescr;

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

		//Find base class
		BPatch_type *baseCl = mod->moduleTypes->findType(baseID);
		if (!baseCl || (baseCl->getDataClass() != BPatch_structure) ) 
			continue;

		//Get field descriptions of the base type
		BPatch_Vector<BPatch_field *> *baseClFields = baseCl->getComponents();
		for(int fieldNum=0; fieldNum < baseClFields->size(); fieldNum++) {
			BPatch_field *field = (*baseClFields)[fieldNum];

			if (field->getVisibility() == BPatch_private)
				continue; //Can not add this member

			newType->addField(field->getName(), 
					  field->getTypeDesc(),
					  field->getType(),
					  field->getOffset(),
					  field->getVisibility());
		}
	}
     }

     while (stabstr[cnt] && (stabstr[cnt] != ';')) {
	typedescr = BPatch_scalar;

	if (stabstr[cnt] == '~') {
		//End of virtual class
		while(stabstr[cnt] != ';') cnt++;
		break; //End of class is reached
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
	  typedescr = BPatch_method;
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
		if (typedescr == BPatch_method) {
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

	assert(stabstr[cnt] == ';');
	cnt++;	// skip ';' at end of field

	// printf("\tType: %d, Starting Offset: %d (bits), Size: %d (bits)\n", comptype, beg_offset, size);
	// Add struct field to type
	BPatch_type *fieldType = mod->moduleTypes->findType(comptype);
	if (fieldType == NULL) {
		//C++ compilers may add extra fields whose types might not available.
		//Assign void type to these kind of fields. --Mehmet
		fieldType = mod->moduleTypes->findType("void");
	}
	if (_vis == BPatch_visUnknown) {
	    newType->addField(compname, typedescr, fieldType,
			    beg_offset, size);
	} else {
	    newType->addField(compname, typedescr, fieldType,
			    beg_offset, size, _vis);
	    //printf("Adding Component with VISIBILITY STRUCT\n");
	}
    }

    // should end with a ';'
    assert(stabstr[cnt] == ';');

    return &stabstr[cnt+1];
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
//		s<int><fields>	|	Structure <int> is size
//		u<int><fields>	|	Union <int> is size
//		V<typeUse>
//
//	<enumType> = <ident>:<int> | <ident>:<int>,<enumType>
//
// It adds the typeDef to the type definition with the name name, and id ID.
//
static char *parseTypeDef(BPatch_module *mod, char *stabstr, char *name, int ID)
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
  
    assert (stabstr[0] != '=');

    // fprintf(stderr, "parsing %s\n", stabstr);
    if (isSymId(stabstr[0])) {
	typdescr = BPatch_scalar;
	type = parseSymDesc(stabstr, cnt);
	 	    
	if (ID == type) {
	    newType = new BPatch_type( name, ID, typdescr, type);
	    if (newType) mod->moduleTypes->addType(newType);
	    if(!newType) {
	      printf(" Can't Allocate newType ");
	      exit(-1);
	    }
	} else if (stabstr[cnt] == '=') {
	    // XXX - in the new type t(0,1)=(0,2)=s... is possible
	    // 	     skip the second id for now -- jkh 3/21/99
	    stabstr = parseTypeDef(mod, &(stabstr[cnt+i+1]), name, type);
	    cnt = 0;
	    BPatch_type *oldType;
	    
	    oldType = mod->moduleTypes->findType(type);
	    if(!oldType) oldType = BPatch::bpatch->type_Untyped;
	    newType = new BPatch_type( name, ID, typdescr, oldType);
	    if (newType) mod->moduleTypes->addType(newType);
	} else {
	    BPatch_type *oldType;
	    
	    oldType = mod->moduleTypes->findType(type);
	    newType = new BPatch_type( name, ID, typdescr, oldType);
	    if(newType) mod->moduleTypes->addType(newType);
	    if(!newType) {
	        printf(" Can't Allocate newType ");
	        exit(-1);
	    }
	}
    } else {
      switch (stabstr[0]) {
	  case 'x':  //cross reference 
	    parseCrossRef(mod->moduleTypes, name, ID, stabstr, cnt);
	    break;
	     
	  case '*':
	    typdescr = BPatch_pointer;
	    /* pointer to another type */
	    cnt++;
	    ptrID = parseTypeUse(mod, stabstr, cnt, NULL);

	    // Create a new B_type that points to a structure
	    ptrType = mod->moduleTypes->findType(ptrID);
	    if (!ptrType) ptrType = BPatch::bpatch->type_Untyped;

	    newType = new BPatch_type(NULL, ID, BPatch_pointer, ptrType);
	    // Add to typeCollection
	    if(newType) mod->moduleTypes->addType(newType);
	    if(!newType) {
		printf(" Can't Allocate new type ");
		exit(-1);
	    }

	    return(&(stabstr[cnt]));
	    break;

	  case 'a':
	      return parseArrayDef(mod, name, ID, stabstr, cnt);
	      break;

	  case 'f':
	        /* function type */
		typdescr = BPatch_func;

		cnt++; /* skip the f */
	        type = parseTypeUse(mod, stabstr, cnt, name);

		break;

	 case 'R': {
		// Define a floating point type - R fp-type; bytes;
		cnt++;
		(void) parseSymDesc(stabstr, cnt);
		cnt ++;

		int bytes = parseSymDesc(stabstr, cnt);

		newType = new BPatch_type(name, ID, BPatch_built_inType, bytes);
		mod->moduleTypes->addType(newType);

		if (stabstr[cnt] == ';') cnt++;	// skip the final ';'

		break;
	  }

	  case 'b': {
		// builtin type b  - signed char-flag width; offset; nbits
		typdescr = BPatch_built_inType;
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

		newType = new BPatch_type(name, ID, BPatch_built_inType, 
		    nbits/8);
		//Add to Collection
		mod->moduleTypes->addType(newType);

		return &stabstr[cnt];
		break;
	    }

	case 'r': 		// range type
	    return parseRangeType(mod, name, ID, stabstr);
	    break;

	case 'e':		// Enumerated type
	    typdescr = BPatch_enumerated;
	    cnt++; 	/* skip the 'e' */

	    // Create new Enum type
	    newType = new BPatch_type(name, ID, typdescr);
	    // Add type to collection
	    mod->moduleTypes->addType(newType);
		
	    while (stabstr[cnt]) {
		/* Get enum component value */
		compsymdesc = getIdentifier(stabstr, cnt);
		cnt++; /* skip colon */
		  
		value = parseSymDesc(stabstr, cnt);

		// add enum field to type
		newType->addField(compsymdesc, BPatch_scalar, value);
		  
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
	        typdescr = BPatch_structure;
	    } else {
	        typdescr = BPatch_union;
	    }

	    cnt++;		// skip to size
	    structsize = parseSymDesc(stabstr, cnt);
	    
	    //Create new type
	    newType = new BPatch_type(name, ID, typdescr, structsize);
	    //add to type collection
	    mod->moduleTypes->addType(newType);
	      
	    return parseFieldList(mod, newType, &stabstr[cnt]);

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
	    fprintf(stderr, "ERROR: Unrecognized str = %s\n", &stabstr[cnt]);
	    return "";
	    break;
      }
    }

    return(&(stabstr[cnt]));
} /* end of parseTypeDef*/

