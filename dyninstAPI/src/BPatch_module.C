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

#include <stdio.h>
#include <ctype.h>

#include "process.h"
#include "symtab.h"
#include "showerror.h"
#include "BPatch.h"
#include "BPatch_module.h"
#include "BPatch_snippet.h" // For BPatch_function; remove if we move it
#include "BPatch_collections.h"
#include "util/h/String.h"
#include "BPatch_type.h"    // For BPatch_type related stuff
#include "BPatch_Vector.h"

char * current_func_name = NULL;


/* XXX temporary */
char *BPatch_module::getName(char *buffer, int length)
{
    string str = mod->fileName();

    strncpy(buffer, str.string_of(), length);

    return buffer;
}


BPatch_module::BPatch_module(process *_proc, pdmodule *_mod):
    proc(_proc), mod(_mod), BPfuncs(NULL) 
{
    moduleTypes = new BPatch_typeCollection;

    // load all of the type information
#if defined(sparc_sun_solaris2_4)    
    parseTypes();
#endif
}

extern dictionary_hash <function_base*, BPatch_function*> PDFuncToBPFunc;

/*
 * BPatch_module::getProcedures
 *
 * Returns a list of all procedures in the module upon success, and NULL
 * upon failure.
 */
BPatch_Vector<BPatch_function *> *BPatch_module::getProcedures()
{
    if (BPfuncs) return BPfuncs;
    
    BPfuncs = new BPatch_Vector<BPatch_function *>;

    if (BPfuncs == NULL) return NULL;

    // XXX Also, what should we do about getting rid of this?  Should
    //     the BPatch_functions already be made and kept around as long
    //     as the process is, so the user doesn't have to delete them?
    vector<function_base *> *funcs = mod->getFunctions();

    for (unsigned int f = 0; f < funcs->size(); f++) {
	BPatch_function *bpfunc;
	bpfunc = PDFuncToBPFunc[(*funcs)[f]];
	if (!bpfunc) bpfunc = new BPatch_function(proc, (*funcs)[f], this);
	BPfuncs->push_back(bpfunc);
    }

    return BPfuncs;
}

/*
 * BPatch_module::findFunction
 *
 * Returns a BPatch_function* with the same name that is provided or
 * NULL if no function with that name is in the module.  This function
 * searches the BPatch_function vector of the module followed by
 * the function_base of the module.  If a function_base is found, then
 * a BPatch_function is created and added to the BPatch_function vector of
 * the module.
 * name The name of function to look up.
 */

BPatch_function * BPatch_module::findFunction(const char * name)
{

    // Did not find BPatch_function with name match in BPatch_function vector
    // trying pdmodule
    function_base *func = mod->findFunction(name);
    if (func == NULL) {
	string fullname = string("_") + string(name);
	func = mod->findFunction(fullname);
    }

    if (func == NULL) {
	return NULL;
    }
  
    BPatch_function * bpfunc; 
    if (bpfunc = PDFuncToBPFunc[func]) {
	// we have a BPatch_function for this already
	return bpfunc;
    }

    // Found function in module and creating BPatch_function
    bpfunc = new BPatch_function(proc, func, this);

#if defined(sparc_sun_solaris2_4) 
    // Adding new BPatch_Function to BPatch_function vector
    this->BPfuncs->push_back(bpfunc);
#endif
    return bpfunc;
    
}

#if defined(sparc_sun_solaris2_4)    
// XXX - move this out too
#include <libelf.h>

// Gets the stab and stabstring section and parses it for types
// and variables
void BPatch_module::parseTypes()
{
  
  char *module;
  image * imgPtr=NULL;
  bool parseActive = false;
  struct stab_entry *stabptr = NULL;
  int stab_nsyms;
  int i;
  char *stabstr_nextoffset;
  const char *stabstrs = 0;
  char * temp=NULL;
  
  //Using pdmodule to get the image Object.
  imgPtr = mod->exec();
  
  //Using the image to get the Object (class)
  Object *objPtr = (Object *) &(imgPtr->getObject());

  //Using the Object to get the pointers to the .stab and .stabstr
  // XXX - Elf32 specific needs to be in seperate file -- jkh 3/18/99
  objPtr->get_stab_info((void **) &stabptr, stab_nsyms, 
	(void **) &stabstr_nextoffset);

  // Building the BPatch_Vector<BPatch_function *> for use later when playing
  // with BPatch_functions
  //printf("GETTING PROCEDURES for BPatch_Function VECTOR %x!!!\n", &(this->BPfuncs));
  this->BPfuncs = this->getProcedures();
  
  for(i=0;i<stab_nsyms;i++){
    switch(stabptr[i].type){

      //   case 32:    //global symbols -- N_GSYM
      //   case 36:    //function name or text segment -- N_FUN
      //   case 40:    //local static variable -- LCSYM
      //   case 64:    //register variable -- N_RSYM
      //   case 160:   //parameter variable -- N_PSYM
    case N_UNDF: /* start of object file */
	    /* value contains offset of the next string table for next module */
#if !defined(i386_unknown_linux2_0)
	    assert(stabptr[i].name == 1);
#endif

	    stabstrs = stabstr_nextoffset;
	    stabstr_nextoffset = stabstrs + stabptr[i].val;

            module = &stabstrs[stabptr[i].name];
	    // printf("    module name %s\n", module);
	    if (!strcmp(module, mod->fileName().string_of())) {
		parseActive = true;
	    } else {
		parseActive = false;
	    }
	    break;

    case N_ENDM: /* end of object file */
            break;

    case N_SO: /* compilation source or file name */
      /* printf("Resetting CURRENT FUNCTION NAME FOR NEXT OBJECT FILE\n");*/
            current_func_name = NULL; // reset for next object file
            module = &stabstrs[stabptr[i].name];
            break;

    case 32:    // Global symbols -- N_GYSM 
    case 36:    // functions and text segments -- N_FUN
    case 128:   // typedefs and variables -- N_LSYM
    case 160:   // parameter variable -- N_PSYM 
      char *ptr, *ptr2, *ptr3;

      if (!parseActive) break;

      ptr = (char *) &stabstrs[stabptr[i].name];
      while (ptr[strlen(ptr)-1] == '\\') {
	//ptr[strlen(ptr)-1] = '\0';
	  ptr2 =  (char *) &stabstrs[stabptr[i+1].name];
	  ptr3 = (char *) malloc(strlen(ptr) + strlen(ptr2));
	  strcpy(ptr3, ptr);
	  ptr3[strlen(ptr)-1] = '\0';
	  strcat(ptr3, ptr2);
	  
	  ptr = ptr3;
	  i++;
	  // XXX - memory leak on multiple cont. lines
      }

      // printf("%d parsing %s\n", i, ptr);
     // may be nothing to parse - XXX  jdd 5/13/99
      temp = parseStabStringSymbol(i, (char *)ptr, stabptr);
      if (*temp) {
	//Error parsing the stabstr, return should be \0
	cerr << "Stab string parsing error!! More to parse: "<< temp <<endl;
      }
     

      break;
    default:
      break;
    }       		    
  }
}


inline bool isSymId(char ch)
{
    return ((ch == '(') || isdigit(ch));
}

/*
 * parse a Symbol Descriptor ID
 *
 */
int getSymDescID(char *stabstr, int cnt, int &i)
{
    int id;
    int lid;
    int hid;
    int sign = 1;
    bool newForm = false;

    // printf("\n    getSymDescID call on %s\n", &stabstr[cnt+i]);

    hid = 0; //file-number
    // parse both an int and (int,int) format (file-number, type ID)
    if (stabstr[cnt+i] == '(') {
	i++;
	while (isdigit(stabstr[cnt+i])) {
	    hid = hid * 10 + stabstr[cnt+i] - '0';
	    i++;
	}

	// skip ","
	if (stabstr[cnt+i] == ',') i++;
	newForm = true;
    }
       
    if (stabstr[cnt+i] == '-') {
	sign = -1;
	i++;
    }

    lid = 0; //type ID
    while (isdigit(stabstr[cnt+i])) {
        lid = lid * 10 + stabstr[cnt+i] - '0';
        i++;
    }
    if( hid != 0 )
      assert(lid < 65536);
    
    // skip closing ')'
    if (newForm) i++;

    id = hid * 65536 + lid;
    id = id * sign;
    // printf("    getSymDescID returns %d\n", id);
    return id;
}

// This function takes the stab section and associated stabstring and
// parses it to create a new type or variable object.
// This function only defines the type/variable name and ID.
char * BPatch_module::parseStabStringSymbol(int line, char * stabstr, void *ptr)
{

  char * temp=NULL;
  char * name=NULL;
  char * scopeName=NULL;
  char * lfuncName=NULL;
  struct stab_entry *stabptr = (struct stab_entry *) ptr;
  
  symDescr_t symdescr;
  int symdescID = 0;
  int funcReturnID = 0;
  int ID = 0;
  int cnt,i,j;
  int framePtr;
  int linenum;
  int type;
  int internalType = 0;
 
  BPatch_type * ptrType = NULL;
  BPatch_type * newType = NULL; //For creating new types to add to the collection
  BPatch_localVar * param = NULL;
  BPatch_localVar * locVar = NULL;
  BPatch_function  * fp;
  function_base * fb;

  cnt=i=j=0;
  
  /* Recursive calls pass line=0, and stabptr=NULL */
  if( stabptr != NULL ){
    type = stabptr[line].type;
    linenum = stabptr[line].desc;
    framePtr = stabptr[line].val;
  } else {
    /* internal defined type */
    internalType = 1;
  }
   /* get type or variable name */
  for (i=0;stabstr[i]!= '\0';i++){
    if( stabstr[i] == ':')
      break;
  }
  if( i ){  // if not a blank line
    name = (char *) malloc(sizeof(char)*(i+1));

    if(!strncpy(name, stabstr, i))
      /* Error copying variable name or type */
      exit(1);
    
    /* Null terminate name or type */
    name[i]='\0';
    
    cnt = i+1;/* End of Variable name skipping ':' */
    i=0;
  }
  /* Variable or type declaration */
  if( internalType ){
    if( stabstr[cnt] == '/'){
      // C++ visibility value
      cnt = cnt + 2; //skip '/' and vis value
    }
  }
  // printf("Type or variable name: %s\n",name);
  
  if (isSymId(stabstr[cnt])) {
    /* instance of a predefined type */
    symdescr = BPatchSymLocalVar;

    /* Get variable type number */
    ID = getSymDescID(stabstr, cnt, i);
    
    //printf("Variable: %s  Type ID: %d, file ID %d \n",name, ID, file_ID);
    cnt = cnt +i;
    i=0;
    if (stabstr[cnt]) {
      /* More Stuff to parse, call parseStabStringType */
      temp = parseStabStringType( (&stabstr[cnt]), name, ID);
      stabstr = temp;
      cnt = 0;
      ptrType = moduleTypes->findType( ID);
      if( current_func_name ) {
	// XXX-may want to use N_LBRAC and N_RBRAC to set function scope 
	// -- jdd 5/13/99
	// Still need to add to local variable list if in a function
	fp = findFunction( current_func_name );
	if (!fp) {
	    printf(" Can't find function %s in module\n", current_func_name);
	    printf("Unable to add %s to local variable list in %s\n",
		   name,current_func_name);
	} else {
	  locVar = new BPatch_localVar(name, ptrType, linenum, framePtr);
	  fp->localVariables->addLocalVar( locVar);
	  // printf("Adding %s to function %s local variable collection\n",
		 // name, current_func_name);
	}
      }
    } else if (current_func_name) {
      // Try to find the BPatch_Function
      ptrType = moduleTypes->findType( ID);
      fp = findFunction(current_func_name);
      if (!fp) {
	  printf(" Can't find function in BPatch_function vector: %s\n",
	       current_func_name);
      } else {
	  locVar = new BPatch_localVar(name, ptrType, linenum, framePtr);
	  fp->localVariables->addLocalVar( locVar);
	  // printf("Adding %s to function %s local variable collection\n",
		 // name, current_func_name);
      }
    }
  } else if (stabstr[cnt]) {
    switch (stabstr[cnt]) {
    case 'f': /*Local Function*/
      symdescr = BPatchSymLocalFunc;
      cnt++;
      i=0;
      current_func_name = name;
      funcReturnID = getSymDescID(stabstr, cnt, i);
      
      /*printf("local function : %s  return type ID: %d \n",name,
	        funcReturnID);*/
      
      cnt = cnt+i;  
      i =0;
      j=0;
      if (stabstr[cnt+i]==',') {/*skip the comma*/
	cnt++;
      }
      while(stabstr[cnt]){
	if(stabstr[cnt+i]==','){
	  j++;
	  cnt++;
	}
	while((stabstr[cnt+i]!=',') && (stabstr[cnt+i]) ){
	  i++;
	}
	if( j == 0){
	/* Local Function Name */
	lfuncName = (char *)malloc(sizeof(char)*(i+1));
	if(!strncpy(lfuncName, &(stabstr[cnt]), i))
	  /* Error copying Symbol Descriptor TYPE ID */
	  exit(1);
	lfuncName[i] = '\0';
	
	}
	else if( j == 1 ){
	  /* Scope Name of Local Function */
	scopeName = (char *)malloc(sizeof(char)*(i+1));
	if(!strncpy(scopeName, &(stabstr[cnt]), i))
	  /* Error copying Symbol Descriptor TYPE ID */
	  exit(1);
	scopeName[i] = '\0';
	}
	if(j >= 2 ){
	  printf("Error Parsing local function definition!! :%s\n",
		 &(stabstr[cnt+i]));
	}
	cnt = cnt+i;
	i=0;
      }
      /*printf("\tLocal Function:%s %d with return type %d in %s\n",
	     lfuncName ? lfuncName: name, symdescr,funcReturnID,
	     scopeName ? scopeName : "");*/
      if (!scopeName) { // Not an embeded function
	ptrType = moduleTypes->findType(funcReturnID);
	if( !ptrType) ptrType = BPatch::bpatch->type_Untyped;

	fp = this->findFunction( name );
	if ( !fp ) {
#ifdef notdef
	  abort();
	  /*printf(" Can't find local function in BPatch_function vector: %s\n",
		 current_func_name);*/
	  // Try to find function!!
	  string f_name = strdup(current_func_name);
	  fb = mod->findFunction(f_name);
	  if (!fb) {
	    /*printf(" Can't find local function %s in module\n",
		   current_func_name);*/
	    vector<module *> *mods = proc->getAllModules();
	    for (unsigned int m = 0; m < mods->size(); m++) {
	      pdmodule *curr = (pdmodule *) (*mods)[m];
	      //cout <<"MODULE NAME: "<<&(curr->fullName())<<"\t"<<&(curr->fileName)<<endl;
	      fb = curr->findFunction(name);
	      if (fb){
		//printf("FOUND FUNCTION BY SEARCHING ALL THE MODULES!!!!!\n");
		// found function_base, create new BPatch_function
		fp = new BPatch_function(proc, fb, ptrType, NULL);
		// add BPatch_function to BPfuncs
		//cout<<"BPfuncs address: "<< &(this->BPfuncs)<<endl;
		//cout<<"Module address: "<< this <<endl;

		this->BPfuncs->push_back(fp);

		break;
	      } else {
		//printf("FUNCTION IS NO WHERE TO BE FOUND!!!\n");
	      }
	    }
	  } else {
	    // found function_base, create new BPatch_function
	    fp = new BPatch_function(proc, fb, ptrType, this);
	    // add BPatch_function to BPfuncs
	    this->BPfuncs->push_back(fp);
	  }
#endif
	} else {
	  // set return type.
	  fp->setReturnType(ptrType);
	}
      } else {
	printf("%s is an embedded function in %s\n",name, scopeName);
      }
      break;
    case 'F':/* Global Function */
      symdescr = BPatchSymGlobalFunc;
      cnt++; /*skipping 'F' */
      i =0;

      funcReturnID = getSymDescID(stabstr, cnt, i);
      current_func_name = name;
      cnt = cnt+i;
      i=0;

      // printf("\tGlobal Function:%d -  %s with return type %d\n", symdescr,
	     // name, funcReturnID);

      if(stabstr[cnt]){
	/* More Stuff to parse, call parseStabStringType */
	temp = parseStabStringType((&stabstr[cnt]), name, funcReturnID);
	cnt = 0;
	stabstr = temp;
       
	if( temp[0] )
	  printf("\tERROR More to parse %s\n", temp);	

      }
      ptrType = moduleTypes->findType(funcReturnID);
      if( !ptrType) ptrType = BPatch::bpatch->type_Untyped;

      fp = this->findFunction( name );
      if( !fp ){
	abort();
	/*printf(" Can't find function in BPatch_function vector: %s\n",
	       current_func_name);*/
	// Try to find function!!
	string f_name = strdup(current_func_name);
	fb = mod->findFunction(f_name);
	if(!fb){
	  //printf(" Can't find function %s in module\n", current_func_name);
	  vector<module *> *mods = proc->getAllModules();
	 
	  for (unsigned int m = 0; m < mods->size(); m++) {
	    pdmodule *curr = (pdmodule *) (*mods)[m];
	    //cout <<"MODULE NAME: "<<&(curr->fullName())<<"\t"<<&(curr->fileName)<<endl;
	    fb = curr->findFunction(name);
	    if (fb) {
	      printf("FOUND FUNCTION BY SEARCHING ALL THE MODULES!!!!!\n");
	      // found function_base, create new BPatch_function
	      fp = new BPatch_function(proc, fb, ptrType, NULL);
	      // add BPatch_function to BPfuncs
	      //cout<<"BPfuncs address: "<< &(this->BPfuncs)<<endl;
	      //cout<<"Module address: "<< this <<endl;
	      
	      this->BPfuncs->push_back(fp);
	      break;
	    }
	    else{
	      //printf("FUNCTION IS NO WHERE TO BE FOUND!!!\n");
	    }
	  }
	} else {
	  // found function_base, create new BPatch_function
	  fp = new BPatch_function(proc, fb, ptrType, this);
	  // add BPatch_function to BPfuncs
	  this->BPfuncs->push_back(fp);
	}
      } else {
	// set return type.
	fp->setReturnType(ptrType);
      }
      /*printf("\tGlobal Function:%d -  %s with return type %d\n", symdescr,
	     name, funcReturnID);*/
      break;
    case 'G':/* Global Varaible */
      symdescr = BPatchSymGlobalVar;
      cnt++; /* skip the 'G' */
      i = 0;
      /* Get variable type number */
      symdescID = getSymDescID(stabstr, cnt, i);

      cnt = cnt + i;
      i = 0;

      if(stabstr[cnt]){
	/* More Stuff to parse, call parseStabStringType */
	temp = parseStabStringType((&stabstr[cnt]), name, symdescID);
	cnt = 0;
	stabstr = temp;
	if( temp[0] )
	  printf("\tMore to parse %s\n", temp);
       
      }

      // printf("\tGlobal Variable:%d -  %s of type %d\n", symdescr,
      //	     name, symdescID);

      // lookup symbol and set type
      BPatch_type *BPtype;
      
      BPtype = moduleTypes->findType(symdescID);
      if (!BPtype) {
	  printf("ERROR: unable to find type #%d for variable %s\n", 
	       symdescID, name);
      }
      else {
	//printf("The moduleType address is : %x\n", &(moduleTypes));
	//printf("Adding new Global: %s\n",name); 
	moduleTypes->addGlobalVariable(name, BPtype);
      }

      break;
    case 'p':/* Function Parameter */
      symdescr = BPatchSymFuncParam;
      cnt++; /* skip the 'p' */
      i = 0;
      /* Get variable type number */
      symdescID = getSymDescID(stabstr, cnt, i);
      
      cnt = cnt+i;
      i=0;

      /*printf("\tFunction Parameter:%d -  %s for %s of type %d\n", symdescr,
	     name, current_func_name, symdescID);*/
      if(stabstr[cnt]){
	/* More Stuff to parse, call parseStabStringType */
	temp = parseStabStringType((&stabstr[cnt]), name, symdescID);
	cnt = 0;
	stabstr = temp;
	if( temp[0] )
	  printf("\tMore to parse %s\n", temp);
	
      }
      ptrType = moduleTypes->findType(symdescID);
      if( !ptrType) ptrType = BPatch::bpatch->type_Untyped;

      param = new BPatch_localVar(name, ptrType, linenum, framePtr);
      
      if( current_func_name)
	fp = this->findFunction( current_func_name );
      if( !fp ) {
	//printf(" Can't find function %s\n", current_func_name);
	// Try to find function!!
	abort();
	string f_name = strdup(current_func_name);
	fb = mod->findFunction(f_name);
	if(!fb){
	  //printf(" Can't find function %s in module\n", current_func_name);
	  /*function_base * ffb = findFunction(current_func_name);
	  if(!ffb){
	    printf(" Can't find function %s with image\n", current_func_name);
	  }*/
	} else {
	  // found function_base, create new BPatch_function
	  fp = new BPatch_function(proc, fb, this);
	  // add BPatch_function to BPfuncs
	  this->BPfuncs->push_back(fp);
	  fp->funcParameters->addLocalVar(param);
	  // adds to the parameter vector
	  //printf(" function pointer %x\n", fp);
	  fp->addParam(name, ptrType, linenum, framePtr);
	  
	}
      } else{ // found function, add parameter
	// printf("Adding parameter %s to function %s\n",name,current_func_name);
	fp->funcParameters->addLocalVar(param);
	// adds to the parameter vector
	//printf(" function pointer %x\n", fp);
	fp->addParam(name, ptrType, linenum, framePtr);
      }
      break;
    case 'r':/* Register Variable */
      symdescr = BPatchSymRegisterVar;
      cnt++; /*move past the 'r'*/
      /* get type reference */
      i=0;

      symdescID = getSymDescID(stabstr, cnt, i);

      printf("\tRegister variable:%d -  %s of type %d", symdescr,
	     name, symdescID);
      if (stabstr[cnt+i])
	printf("Parsing Error More register info to Parse!!: %s\n",
	       &(stabstr[cnt+i]));
      cnt = cnt+i;
      i =0;
      break;
    case 'S':/* Global Static Variable */
      symdescr = BPatchSymStaticGlobal;
      cnt++; /*move past the 'S'*/
      /* get type reference */
      i=0;

      symdescID = getSymDescID(stabstr, cnt, i);

      /*printf("\tGlobal Static Variable:%d -  %s of type %d\n", symdescr,
	     name, symdescID);
	     */
      if (stabstr[cnt+i])
	printf("Parsing Error More Global Static info to Parse!!: %s\n",
	       &(stabstr[cnt+i]));
      cnt = cnt+i;
      i =0;
      break;
    case 't':/* Type Name */
      symdescr = BPatchSymTypeName;
      cnt++; /*move past the 't'*/
      /* get type reference */
      i=0;

      symdescID = getSymDescID(stabstr, cnt, i);

      cnt =cnt + i;
      i = 0;

      // printf("\tType name:%d -  %s of type %d\n", symdescr, name, symdescID);
      //Create BPatch_type.
     
      if(stabstr[cnt]){
	/* More Stuff to parse, call parseStabStringType */
	temp = parseStabStringType((&stabstr[cnt]), name, symdescID);
	cnt = 0;
	stabstr = temp;
	if( temp[0] ){
	  printf("\tMore to parse %s\n", temp);
	  temp = NULL;
	}
	
      }
      else{
	//Create BPatch_type defined as a pre-exisitng type.
	ptrType = moduleTypes->findType(symdescID);
	if (!ptrType)
	  ptrType = BPatch::bpatch->type_Untyped;
	newType = new BPatch_type(name, symdescID, ptrType);
	if(newType){
	  moduleTypes->addType(newType);
	}
      }
      break;
    case 'T':/* Aggregate type tag -struct, union, enum */
      symdescr = BPatchSymAggType;
      cnt++; /*move past the 'T'*/

      i=0;
      if( stabstr[cnt] == 't' ){
	//C++ struct  tag "T" and type def "t"
	//printf("SKipping C++ Identifier t of Tt\n");
	symdescr = BPatchSymTypeTag;
	cnt++;  //skip it
      }

      /* get type reference */
      symdescID = getSymDescID(stabstr, cnt, i);
    
      cnt =cnt + i;
      i = 0;

      // printf("\tAggregate Type:%d -  %s of type %d\n", symdescr,
	     // name, symdescID);
      //Create BPatch_type.
      if(stabstr[cnt]){
	/* More Stuff to parse, call parseStabStringType */
	temp = parseStabStringType((&stabstr[cnt]), name, symdescID);
	cnt = 0;
	stabstr = temp;
	if( temp[0] ){
	  printf("\tMore to parse %s\n", (&stabstr[cnt]));
	  temp = NULL;
	}
      }
      else{
	//Create BPatch_type defined as a pre-exisitng type.
	newType = new BPatch_type(name, symdescID);
	if(newType)
	  moduleTypes->addType(newType);
      }
      break;
    case 'V':/* Local Static Variable */
      symdescr = BPatchSymStaticLocalVar;
      cnt++; /*move past the 'V'*/
      /* get type reference */
      i=0;

      symdescID = getSymDescID(stabstr, cnt, i);
     
      printf("\tLocal Static Variable:%d - %s of type %d\n",symdescr,
	      name, symdescID);
      
      if (stabstr[cnt+i])
	printf("Parsing Error More Local Static info to Parse!!: %s\n",
	       &(stabstr[cnt+i]));
      cnt = cnt+i;
      i =0;
      break;
    default:
      printf("Unknown symbol descriptor: %c\n", stabstr[cnt]);
    }   
  }

  if( newType != NULL){
    // add to Type collection
  }
  return(&stabstr[cnt]);
}/* end of ParseStabStringSymbol*/



	    
// This function takes the newType and gets further information:
// size, range, type, pointer to another type, and component
// definition to add to the type definition.

char * BPatch_module::parseStabStringType(char * stabstr, char * name, int ID ){

  BPatch_type * newType = NULL;
  BPatch_type * ptrType = NULL;
  
  char * temp=NULL;
  char * symdesc=NULL;
  char * compsymdesc=NULL;
  char * compname;
  char tmpchar;
  
  char scope = 'L';

  BPatch_dataClass typdescr;
  BPatch_visibility _vis = BPatch_visUnknown;
  int symdescID =0;
  int ptrID=0;
  int lowbound= 0;
  int hibound=0;
  
  /* type range */
  char * low;
  char * hi;

  int value;
  int size=0;
  int cnt,i,j,k;
  int structsize;
  int type;
  int comptype= 0;
  int  beg_offset=0;
  int err = 0;
  int compcnt = 0;
  int refID = 0;
  cnt = i = j = k = 0;
  
  while(stabstr[cnt+i]){
      if (stabstr[cnt+i] == '=') {
	i++;
	if (isSymId(stabstr[cnt+i])) {
	  cnt = cnt+i;
	  i = 0;

	  typdescr = BPatch_scalar;
	  type = getSymDescID(stabstr, cnt, i);
	 	    
	  cnt = cnt +i;
	  i = 0;

	  // printf("\tType : %d\n", type);
	  //Create new type -- ID==type for void.
	  if (ID == type) {
	    newType = new BPatch_type( name, ID, typdescr, type);
	    if(newType)
	      moduleTypes->addType(newType);
	    if(!newType) {
	      printf(" Can't Allocate newType ");
	      exit(-1);
	    }
	  } else if (stabstr[cnt+i] == '=') {
	    // XXX - in the new type t(0,1)=(0,2)=s... is possible
	    // 	     skip the second id for now -- jkh 3/21/99
	    temp = parseStabStringType(&(stabstr[cnt+i]), name, type);
	    cnt = i = 0;
	    stabstr = temp;
	    BPatch_type *oldType;
	    
	    oldType = moduleTypes->findType(type);
	    if(!oldType)
	      oldType = BPatch::bpatch->type_Untyped;
	    newType = new BPatch_type( name, ID, typdescr, oldType);
	    if(newType)
	      moduleTypes->addType(newType);
	    //continue;
	  } else {
	    BPatch_type *oldType;
	    
	    oldType = moduleTypes->findType(type);
	    newType = new BPatch_type( name, ID, typdescr, oldType);
	    if(newType)
	      moduleTypes->addType(newType);
	    if(!newType) {
	      printf(" Can't Allocate newType ");
	      exit(-1);
	    }
	    // printf(" adding typedef Type \n");
	    // printf("    (name = %s, type = %d, id = %d\n", name, type, ID);
	  }
	} else{
	 
	  switch( stabstr[cnt+i]){
	  case 'x':  //cross reference 
	    /* internal struct/union pointer */
	    typdescr = BPatch_pointer;
	    cnt= cnt+i+1; /* skip 'x'*/

	    if(stabstr[cnt] == 's'){
	      /* struct */
	      cnt++;
	      i=0;
	      while( (stabstr[cnt+i]!= ':')){
		i++;
	      }
	      /* struct/union name (internal ??)*/
	      temp = (char *)malloc(sizeof(char)*(i+1));
	      if(!strncpy(temp, &(stabstr[cnt]), i))
		/* Error copying union size */
		exit(1);
	      temp[i]= '\0';
	      // printf("Pointer to internal Struct : %s\n", temp);
	      cnt = cnt+i+1; /*skip ':' */
	      // Find type that this one points to.
	      ptrType = moduleTypes->findType(temp);
	      if( ptrType){
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
		
	      return( &(stabstr[cnt]));
	    }
	    else if( stabstr[cnt] == 'u'){
	      /* union */
	      cnt++;
	      i=0;
	      while( (stabstr[cnt+i]!= ':')){
		i++;
	      }
	      /* struct/union name (internal ??)*/
	      temp = (char *)malloc(sizeof(char)*(i+1));
	      if(!strncpy(temp, &(stabstr[cnt]), i))
		/* Error copying union size */
		exit(1);
	      temp[i]= '\0';
	      //printf("Pointer to internal Union : %s\n", temp);
	      cnt = cnt+i+1; /*skip ':' */
	      
	      // Find type that this one points to.
	      ptrType = moduleTypes->findType(temp);
	      if( ptrType){
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


	      return( &(stabstr[cnt]));
	    }
	    else if( stabstr[cnt] == 'e'){
	      /* Enumeration */
	      cnt++;
	      i=0;
	      while( (stabstr[cnt+i]!= ':')){
		i++;
	      }
	      /* struct/union name (internal ??)*/
	      temp = (char *)malloc(sizeof(char)*(i+1));
	      if(!strncpy(temp, &(stabstr[cnt]), i))
		/* Error copying union size */
		exit(1);
	      temp[i]= '\0';
	      printf("Pointer to internal Enumeration : %s\n", temp);
	      cnt = cnt+i+1; /*skip ':' */

	      // Find type that this one points to.
	      ptrType = moduleTypes->findType(temp);
	      if( ptrType){
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
	    }
	    else {
	      /* don't know what it is?? */
	      
	      i=0;
	      while( (stabstr[cnt+i]!= ':')){
		i++;
	      }
	      /* struct/union name (internal ??)*/
	      temp = (char *)malloc(sizeof(char)*(i+1));
	      if(!strncpy(temp, &(stabstr[cnt]), i))
		/* Error copying union size */
		exit(1);
	      temp[i]= '\0';
	      printf("Pointer to internal something : %s\n", temp);
	      cnt = cnt+i+1; /*skip ':' */
	      return( &(stabstr[cnt]));
	    }
	    break;
	     
	  case '*':
	    typdescr = BPatch_pointer;
	    /* pointer to another type */
	    cnt = cnt+i+1;

	    i=0;
	    ptrID = getSymDescID(stabstr, cnt, i);
	   
	    cnt=cnt+i;
	    i=0;

	    // printf("Pointer to Type ID: %d\n",ptrID);
	    if (stabstr[cnt] == '=') {
	      temp = parseStabStringType( &(stabstr[cnt]), NULL , ptrID);
	      cnt = 0;
	      stabstr = temp;
	    }

	    // Create a new B_type that points to a structure
	    ptrType = moduleTypes->findType(ptrID);
	    if (!ptrType) ptrType = BPatch::bpatch->type_Untyped;

	    newType = new BPatch_type(NULL, ID, BPatch_pointer, ptrType);
	    // Add to typeCollection
	    if(newType)
		moduleTypes->addType(newType);
	    if(!newType) {
		printf(" Can't Allocate new type ");
		exit(-1);
	    }

	    return(&(stabstr[cnt]));
	    break;

	  case 'a':
	    /* array type */
	    typdescr = BPatch_array;
	    cnt = cnt+i;
	    i=1;
	    if( stabstr[cnt+i] == 'r'){
	      /* array with range */
	      i++;
	      symdesc = (char *)malloc(sizeof(char)*(i+1));
	      if(!strncpy( symdesc, &(stabstr[cnt]), i))
		/* Error copying Global symbol descriptor */
		exit(1);
	      symdesc[i]= '\0';
	      cnt=cnt+i;
	      i=0;
	      symdescID = getSymDescID(stabstr, cnt, i);
	     
	      cnt=cnt+i+1; /* skip semicolon */
	      i=0;

	      j = 0;
	      while(stabstr[cnt+i]){
		if( j == 0)
		  lowbound = getSymDescID(stabstr, cnt, i);
		if(j == 1)
		  hibound = getSymDescID(stabstr, cnt, i);
		if(j==2) {
		  type = getSymDescID(stabstr, cnt, i);
		  /* multi dimensional array */
		  if (stabstr[cnt+i] == '=') {
		    
		    temp = parseStabStringType(&(stabstr[cnt+i]), NULL, type);
		    cnt = i = 0;
		    stabstr = temp;
		    if(stabstr[cnt] == ':'){
		      //C++ stuff
		      //printf("Skipping C++ rest of structure:  %s\n",name );
		       while(stabstr[cnt+i]){
			 i++;
		       }
		    }
		  }
		}
		
		j++;
		if(stabstr[cnt+i] == ';'){
		  cnt=cnt+i+1; /* skip semicolon */
		  i = 0;
		} else{
		  cnt = cnt+i;
		  i = 0;
		}
		if( (j == 3) &&  (stabstr[cnt+i] == ',')){
		  // printf("Symbol Desriptor: %s, Descriptor ID: %d Type: %d, Low Bound: %d, Hi Bound: %d,\n", symdesc, symdescID, type, lowbound, hibound);

		  ptrType = moduleTypes->findType(type);
		  if( ptrType ){

		    // Create new type - field in a struct or union
		    newType = new BPatch_type(name, ID, typdescr, ptrType,
					     lowbound, hibound);
		    if(newType){
		    // Add to Collection
		    moduleTypes->addType(newType);
		    }
		    else{
		      printf(" Could not create newType Array\n");
		      exit(-1);
		    }
		  }
			
		  free(symdesc);
		  symdesc = NULL;
		  return(&(stabstr[cnt+i]));
		}
	      }
	      // printf("\tSymbol Desriptor: %s, Descriptor ID: %d Type: %d, Low Bound: %d, Hi Bound: %d,\n", symdesc, symdescID, type, lowbound, hibound);
	      ptrType = moduleTypes->findType(type);
	      if( ptrType ){

		// Create new type 
		newType = new BPatch_type( name, ID, typdescr, ptrType,
					   lowbound, hibound);
		if( newType ){
		  // Add to Collection
		  moduleTypes->addType(newType);
		}
		else{
		  printf(" Could not create newType Array %s %d\n", name, ID);
		  //exit(-1);
		}
	      }
	      free(symdesc);
	      symdesc = NULL;
	    } 
	  break;
	  case 'f':
	    /* function type */
	    typdescr = BPatch_func;
	    cnt = cnt + i + 1; /* skip the f */
	    i = 0;
	    type = getSymDescID(stabstr, cnt, i);
	   
	    cnt=cnt+i;
	    i = 0;
	    //printf("\tFunction Pointer of Type: %d \n",type);
	    /*THis can still point to something else;may not be done parsing */
	    if(stabstr[cnt] == '='){
	      temp = parseStabStringType( &(stabstr[cnt]), name, ID);
	      stabstr = temp;
	      return( stabstr);
	    }
	    else
	      return(&(stabstr[cnt]));
	    break;
	  case 'r':
	    /* range type*/
	    tmpchar = stabstr[cnt+i];
	    typdescr = BPatchSymTypeRange;

	    cnt = cnt + i;
	    symdescID = getSymDescID(stabstr, cnt, i);
	  
	    cnt = cnt + i;
	    i =0;

	    // printf("\tSymbol Descriptor: %c and Value: %d\n",tmpchar, symdescID);
	    
	    cnt++; /* Discarding the ';' */
	    if( stabstr[cnt] == '-' )
	      i++;
	    /* Getting type range or size */
	    while(isdigit(stabstr[cnt+i])){
	      i++;
	    }
	    temp = (char *)malloc(sizeof(char)*(i+1));
	    if(!strncpy(temp, &(stabstr[cnt]), i))
	      /* Error copying size/range*/
	      exit(1);
	    temp[i] = '\0';
	    j = atol(temp);
	    
	    if( j <= 0 ){
	      /* range */
	      low = temp;
	      temp = NULL;
	      
	      cnt = cnt + i + 1; /* Discard other Semicolon */
	      i = 0;
	      if((stabstr[cnt]) == '-')
		i++; /* discard '-' for (long) unsigned int */
	      
	      while(isdigit(stabstr[cnt+i])){
		i++;
	      }
	      hi = (char *)malloc(sizeof(char)*(i+1));
	      if(!strncpy(hi, &(stabstr[cnt]), i))
		/* Error copying upper range */
		exit(1);
	      hi[i] = '\0';
	      // printf("\tLower limit: %s and Upper limit: %s\n", low, hi);
	      //Create new type
	      newType = new BPatch_type( name, ID, typdescr, low, hi);
	      //Add to Collection
	      moduleTypes->addType(newType);

	      free(low);
	      free(hi);
	      hi=low=NULL;
	      
	    }
	    else if( j > 0){
	      /*size */
	      size = (int)j;
	      
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
		newType = new BPatch_type( name, ID, typdescr, size);
		//Add to Collection
		moduleTypes->addType(newType);
	      } else {
		// printf("Type RANGE: ERROR!!\n");
	      }	
	    }
	    cnt = cnt + i;
	    if( stabstr[cnt] == ';')
	      cnt++;
	    if( stabstr[cnt] ) {
	      printf("ERROR: More to parse in type-r- %s\n", &(stabstr[cnt]));
	    }
	    
	    return(&(stabstr[cnt]));
	    break;
	  case 'e':
	    typdescr = BPatch_enumerated;
	    // printf("\tSymbol Descriptor: %c\n",stabstr[cnt+i]);
	    cnt = cnt + i + 1; /* skip the 'e' */
	    i = 0;
	    // Create new Enum type
	    newType = new BPatch_type(name, ID, typdescr);
	    // Add type to collection
	    moduleTypes->addType(newType);
	    
	    while(stabstr[cnt]){
	      j=0;
	      /* Get enum component value */
	      while( stabstr[cnt+i]!= ':' ){
		  i++;
		}
	      i++; /* get colon */
	      
	      compsymdesc = (char*) malloc(sizeof(char)*(i+1));
	      if(!strncpy(compsymdesc, &(stabstr[cnt]), i))
		/* Error copying enum component name */
		exit(1);
	      compsymdesc[i-1]= '\0';
	      
	      cnt = cnt+i; /* enumerated type value next */
	      i =0;
	      
	      value = getSymDescID(stabstr, cnt, i);
	      // printf("\tEnum Component Descriptor, name: %s\t value: %d\n",
	      // compsymdesc, value);
	      // add enum field to type
	      newType->addField(compsymdesc, typdescr, value);
	      
	      free(temp);
	      free(compsymdesc);
	      temp = compsymdesc = NULL;
	      cnt = cnt + i + 1 ; /* skip trailing comma */
	      i=0;
	      if( (stabstr[cnt]) == ';'){
		cnt++; /* End of enum stab record */
	      }
	    }
	    if(!(stabstr[cnt]))
	      return(&(stabstr[cnt]));
	    break;
	    
	  case '@':  // type attribute, defines size and type (negative num)
	    // format @s(size in bits); negative type number;
	    typdescr = BPatch_typeAttrib;
	    cnt = cnt + i + 1; /* skip the @ */
	    i = 0;
	    if ( stabstr[cnt] == 's' ){
	      cnt++;
	      
	      size = getSymDescID(stabstr, cnt, i);
	      cnt++;  // skip ';'
	      type = getSymDescID(stabstr, cnt, i);
	      // skip ';' end of stab record ??? (at least for bool)
	      cnt = cnt + i + 1;  
	      i=0;
	      // Create a new B_type that points to a builtInTypes
	      ptrType = BPatch::bpatch->builtInTypes->findBuiltInType(type);
	      
	      if (!ptrType) ptrType = BPatch::bpatch->type_Untyped;
	      
	      newType = new BPatch_type(name, ID, typdescr, size, ptrType);
	      
	      // Add type to collection
	      if(newType)
		moduleTypes->addType(newType);
	      if(!newType) {
		printf(" Can't Allocate new type ");
		exit(-1);
	      }
	      
	      
	      if( stabstr[cnt] ){
		printf("More Type Attribute to Parse: %s ID %d : %s\n", name,
		       ID, &(stabstr[cnt]));
	      }
	    }
	    else{
	      //printf(" Unable to parse Type Attribute: %s ID %d : %s\n", name,ID, &(stabstr[cnt]));
	    }
    
	    break;
	  case '&': //XXX- Need to complete, may be more to parse jdd 4/22/99
	    typdescr = BPatch_reference;
	    /* reference to another type */
	    cnt = cnt+i+1;
	    
	    i=0;
	    refID = getSymDescID(stabstr, cnt, i);
	    cnt=cnt+i;
	    i=0;
	    
	    ///printf("Reference to Type ID: %d\n",refID);
	    if (stabstr[cnt] == '=') {
	      temp = parseStabStringType( &(stabstr[cnt]), NULL , refID);
	      cnt = 0;
	      stabstr = temp;
	    }
	    
	    // Create a new B_type that points to a structure
	    ptrType = moduleTypes->findType(refID);
	    if (!ptrType)
	      ptrType = BPatch::bpatch->type_Untyped;
	    
	    newType = new BPatch_type(name, ID, BPatch_pointer, ptrType);
	    // Add to typeCollection
	    if(newType)
	      moduleTypes->addType(newType);
	    if(!newType) {
	      printf(" Can't Allocate new type ");
	      exit(-1);
	    }
	    
	    return(&(stabstr[cnt]));
	    break;
	  default:
	    /* Must be struct or union */
	    err++;
	    break;
	  }
  	  /* Must be struct or union */

	  if( stabstr[cnt+i]){
	    /* Type descriptor */
	    tmpchar=stabstr[cnt+i];
	    cnt = cnt + i +1; /* skip to size */
	    i = 0;
	    while(isdigit(stabstr[cnt+i])){
	      i++;
	    }
	    /* get aggregate type size */
	    temp = (char *)malloc(sizeof(char)*(i+1));
	    if(!strncpy(temp, &(stabstr[cnt]), i))
	      /* Error copying union size */
	      exit(1);
	    temp[i]= '\0';
	    structsize = atoi(temp);
	    free(temp);
	    temp = NULL;
	    cnt = cnt + i; /* point to first union/struct component name */
	    
	    switch(tmpchar){
	    case 's':
	      typdescr = BPatch_structure;
	      err--; /* parsing struct- Cancel default err++ */
	      // printf("Symbol Descriptor: %c size: %d\n", tmpchar, structsize);
	      
	      //Create new struct type
	      newType = new BPatch_type(name, ID, typdescr, structsize);
	      //add to type collection
	      moduleTypes->addType(newType);
	      
	      while (stabstr[cnt]) {
		i=0;
		/* Get struct component name */
		while( stabstr[cnt+i]!= ':' ){
		  i++;
		}
		i++; /* get colon */
		compcnt++;
		compname = (char *)malloc(sizeof(char)*(i+1));
		if(!strncpy(compname, &(stabstr[cnt]), i))
		  /* Error copying struct component name */
		  exit(1);
		compname[i-1]= '\0';
		//printf("\tStruct %s Component Name: %s Component number: %d\n",name, compname, compcnt);
		cnt = cnt +i;
		i = 0;
		j = 0;  // set/reset j
		 
		if((stabstr[cnt+i]) == '/'){ // visibility C++
		  i++; /* get '/' */
		  switch (stabstr[cnt+i]){
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
		  i++; // get visibility value
		  cnt = cnt +i;
		  i = 0;
		}
		if((stabstr[cnt+i]) == ':'){
		  // C++ data structure
		  //printf("Skipping C++ rest of structure:  %s\n", name );
		  while(stabstr[cnt+i]){
		    i++;
		  }
		  cnt = cnt + i; 
		  return(&(stabstr[cnt]));
		}
		
		while ((stabstr[cnt] != ';') && stabstr[cnt] ) {
		  k=0;
		  i=0;
		  if( stabstr[cnt] == ':'){
		    /*printf("Unable to parse the rest of Struct component %s\n",
			   &(stabstr[cnt]));*/
		    while(stabstr[cnt+i]){
		      i++;
		    }
		  }

		  if(stabstr[cnt] == '='){
		    if( compcnt > 1 ){
		      while( stabstr[cnt-k] != ';'){
			k++;
		      }
		      if(stabstr[cnt-k+1] == '/'){
			k = k - 2; //gets rid of visibility
		      }
		      temp = parseStabStringSymbol(0,&(stabstr[cnt-k+1]),NULL);
		      cnt = 0;
		      i=0;
		      stabstr = temp;
		    }
		    else if(compcnt ==1){/* first component in struct */
		      k++; /* already == '=' */
		      while( stabstr[cnt-k] != '='){
			k++;
		      }
		      k--; /*get rid of '=' */
		      k--; /*get rid of 's' */
		      
		      /* get rid of structure size */
		      while(isdigit(stabstr[cnt-k] )){
			k--;
		      }
		      
		      temp = parseStabStringSymbol(0,&(stabstr[cnt-k]),NULL);
		      cnt = 0;
		      i=0;
		      stabstr = temp;
		    }
		  }
		  
		  if ((stabstr[cnt+i] == ',')) {
		    cnt++;
		    j++;
		  }
		  
		  if (j == 0) {
		    comptype = getSymDescID(stabstr, cnt, i);
		    // printf("Type ID: %d \n", comptype);
		  }
		  if (j == 1)
		    beg_offset = getSymDescID(stabstr, cnt, i);
		  if(j == 2)
		    size = getSymDescID(stabstr, cnt, i);
		  cnt=cnt+i;
		  i = 0;
		}
		// printf("\tType: %d, Starting Offset: %d (bits), Size: %d (bits)\n", comptype, beg_offset, size);
		// Add struct field to type
		BPatch_type *fieldType = moduleTypes->findType(comptype);
		if( _vis == BPatch_visUnknown ){
		  newType->addField(compname, BPatch_scalar, fieldType,
				    beg_offset, size);
		}
		else{
		  newType->addField(compname, BPatch_scalar, fieldType,
				    beg_offset, size, _vis);
		  //printf("Adding Component with VISIBILITY STRUCT\n");
		}
				
		if(stabstr[cnt] == ';'){
		  cnt++;
		  if(stabstr[cnt] == ';'){
		    cnt++; /* at the end of the structure, done parsing */
		    return(&(stabstr[cnt]));
		  }
		}
	      }
	      break;
	    case 'u':
	      typdescr = BPatch_union;
	      err--; /* parsing struct- Cancel default err++ */
	      // printf("Symbol Descriptor: %c size: %d\n", tmpchar, structsize);
	      //Create new struct type
	      newType = new BPatch_type(name, ID, typdescr, structsize);
	      //add to type collection
	      moduleTypes->addType(newType);

	      while(stabstr[cnt]){
		i=0;
		/* Get union component name */
		while( stabstr[cnt+i]!= ':' ){
		  i++;
		}
		i++; /* get semicolon */
		compcnt++;
		compname = (char *)malloc(sizeof(char)*(i+1));
		if(!strncpy(compname, &(stabstr[cnt]), i))
		  /* Error copying struct component name */
		  exit(1);
		compname[i-1]= '\0';
		// printf("\tUnion Component Name: %s ", compname);
		cnt = cnt +i;
		i = 0;
		j = 0;  // set/reset j
		
		if((stabstr[cnt+i]) == '/'){ // visibility C++
		  i++; /* get '/' */
		  switch (stabstr[cnt+i]){
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
		  i++;  // get rid of visibility value
		  cnt = cnt +i;
		  i = 0;
		}
		if((stabstr[cnt+i]) == ':'){
                  // C++ data structure
                  //printf("Skipping C++ rest of structure:  %s\n", name );

                  while(stabstr[cnt+i]){
                  i++;
                  }
                  cnt = cnt + i;
                  return(&(stabstr[cnt]));
                }
		
		while ((stabstr[cnt] != ';') && stabstr[cnt]){
		  k=0;
		  i=0;

		  if(stabstr[cnt] == '='){
		    if( compcnt > 1 ){
		      while( stabstr[cnt-k] != ';'){
			k++;
		      }

		      temp = parseStabStringSymbol(0,&(stabstr[cnt-k+1]),NULL);
		      cnt = 0;
		      i=0;
		      stabstr = temp;
		    }
		    else if(compcnt == 1){/* first component in struct */
		      k++; /* already == '=' */
		      while( stabstr[cnt-k] != '='){
			k++;
		      }
		      
		      k--; /*get rid of '=' */
		      k--; /*get rid of 's' */
		      
		      /* get rid of structure size */
		      while( isdigit(stabstr[cnt-k]) ){
			k--;
		      }
		      if(stabstr[cnt-k] == '/'){
			k = k - 2; //gets rid of visibility
		      }
		      temp = parseStabStringSymbol(0,&(stabstr[cnt-k]),NULL);
		      cnt = 0;
		      i=0;
		      stabstr = temp;
		    }
		  }
		  
		  if( (stabstr[cnt+i] == ',')){
		    cnt++;
		    j++;
		  }
		  
		  /* Get component info-  ID, offset, size */
		  if( j == 0){
		    comptype = getSymDescID(stabstr, cnt, i);
		    // printf("Type ID: %d \n", comptype);
		  }
		  if (j == 1)
		    beg_offset = getSymDescID(stabstr, cnt, i);
		  if(j == 2)
		    size = getSymDescID(stabstr, cnt, i);
		  cnt=cnt+i;
		  i = 0;
		}
		// printf("\tType: %d, Starting Offset: %d (bits), Size: %d (bits)\n", comptype, beg_offset, size);
		// Add struct field to type
		BPatch_type *fieldType = moduleTypes->findType(comptype);
		if( _vis == BPatch_visUnknown )
		  newType->addField(compname, BPatch_scalar, fieldType,
				    beg_offset,size);
		else{ // XXX- I don't think there exists such a thing
		  // but I have included the code just in case -jdd 4/29/99
		  newType->addField(compname, BPatch_scalar, fieldType,
				    beg_offset, size, _vis);
		  //printf("Adding Component with VISIBILITY UNION\n");
		}
		if(stabstr[cnt] == ';'){
		  cnt++;
		  if(stabstr[cnt] == ';'){
		    cnt++; /* at the end of the union, done parsing */
		    return(&(stabstr[cnt]));
		  }
		}
	      }
	      break;
	    default:
	      printf("ERROR: Unrecognized Type Descriptor : %c\n", tmpchar);
	      cerr<<"For type: "<< name <<" and ID: "<< ID << stabstr<<endl;
	      cerr << "Advancing Pointer to the End of the string"<<endl;
	  
	      
	      while( stabstr[cnt]){
		cnt++;
	      }
	      cerr<<&(stabstr[cnt])<<endl;
	      cerr << "Pointer Advanced to end of Stab String!!"<<endl;
	      
	      return(&(stabstr[cnt]));
	      break;
	    }/* end of switch */
	  }
	}
      }
      /*   return(&(stabstr[cnt]));*/
  }
  if(err!=0 )
    printf("\nType parsing mismatch!!\n");
  return(&(stabstr[cnt]));
} /* end of ParseStabStringType*/

#endif //end of #if defined(sparc_sun_solaris2_4)
