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


/*
 * BPatch_module::getSourceObj()
 *
 * Return the contained source objects (e.g. functions).
 *
 */
BPatch_Vector<BPatch_sourceObj *> *BPatch_module::getSourceObj()
{
    return  (BPatch_Vector<BPatch_sourceObj *> *) getProcedures();
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

/* XXX temporary */
char *BPatch_module::getName(char *buffer, int length)
{
    string str = mod->fileName();

    strncpy(buffer, str.string_of(), length);

    return buffer;
}


BPatch_module::BPatch_module(process *_proc, pdmodule *_mod,BPatch_image *_img):
    proc(_proc), mod(_mod), img(_img), BPfuncs(NULL) 
{
    _srcType = BPatch_sourceModule;

    moduleTypes = new BPatch_typeCollection;

    // load all of the type information
#if defined(sparc_sun_solaris2_4) || \
    defined(rs6000_ibm_aix4_1) || \
    defined(alpha_dec_osf4_0) || \
    defined(i386_unknown_linux2_0) || \
    defined(i386_unknown_solaris2_5)

    if (BPatch::bpatch->parseDebugInfo()) 
	parseTypes();
#endif
}

BPatch_module::~BPatch_module()
{
    delete moduleTypes;

    for (unsigned int f = 0; f < BPfuncs->size(); f++) {
	delete (*BPfuncs)[f];
    }
    delete BPfuncs;
}

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
	bpfunc = proc->PDFuncToBPFuncMap[(*funcs)[f]];
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

extern bool buildDemangledName(const string &mangled, string &use);

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
	//Try with demangled name
	string mangled_name = name;
	string demangled;
	if (buildDemangledName(mangled_name, demangled))
		func = mod->findFunction(demangled);
    }

    if (func == NULL) {
	return NULL;
    }
  
    BPatch_function * bpfunc; 
    if ((bpfunc = proc->PDFuncToBPFuncMap[func])) {
	// we have a BPatch_function for this already
	return bpfunc;
    }

    // Found function in module and creating BPatch_function
    bpfunc = new BPatch_function(proc, func, this);

#if defined(sparc_sun_solaris2_4)
    // Adding new BPatch_Function to BPatch_function vector
    if (this->BPfuncs) this->BPfuncs->push_back(bpfunc);
#endif
    return bpfunc;
    
}


extern char *parseStabString(BPatch_module *, int linenum, char *str, int fPtr);

#if defined(rs6000_ibm_aix4_1)

#include <syms.h>

// Gets the stab and stabstring section and parses it for types
// and variables
void BPatch_module::parseTypes()
{
    int i, j;
    int nstabs;
    SYMENT *syms;
    char *modName;
    char name[256];
    char *stringPool;
    char *stabstr=NULL;
    union auxent *aux;
    image * imgPtr=NULL;

    //Using pdmodule to get the image Object.
    imgPtr = mod->exec();
  
    //Using the image to get the Object (class)
    Object *objPtr = (Object *) &(imgPtr->getObject());

    //Using the Object to get the pointers to the .stab and .stabstr
    objPtr->get_stab_info(stabstr, nstabs, ((Address&) syms), stringPool); 

    this->BPfuncs = this->getProcedures();

    bool parseActive = true;
    for (i=0; i < nstabs; i++) {
      /* do the pointer addition by hand since sizeof(struct syment)
       *   seems to be 20 not 18 as it should be */
      SYMENT *sym = (SYMENT *) (((unsigned) syms) + i * SYMESZ);
      // SYMENT *sym = (SYMENT *) (((unsigned) syms) + i * sizeof(struct syment));

      if (sym->n_sclass == C_FILE) {
	 char *moduleName;
	 if (!sym->n_zeroes) {
	    moduleName = &stringPool[sym->n_offset];
	 } else {
	    char tempName[9];
	    memset(tempName, 0, 9);
	    strncpy(tempName, sym->n_name, 8);
	    moduleName = tempName;
	 }
	 /* look in aux records */
	 for (j=1; j <= sym->n_numaux; j++) {
	    aux = (union auxent *) ((char *) sym + j * SYMESZ);
	    if (aux->x_file._x.x_ftype == XFT_FN) {
		if (!aux->x_file._x.x_zeroes) {
                     moduleName = &stringPool[aux->x_file._x.x_offset];
                } else {
                     // x_fname is 14 bytes
                     memset(moduleName, 0, 15);
                     strncpy(moduleName, aux->x_file.x_fname, 14);
                }
	    }
	 }

	 if (strrchr(moduleName, '/')) {
	     moduleName = strrchr(moduleName, '/');
	     moduleName++;
	 }

	 if (!strcmp(moduleName, mod->fileName().string_of())) {
		parseActive = true;
	 } else {
		parseActive = false;
	 }
      }

      if (!parseActive) continue;

      if (sym->n_sclass & DBXMASK) {
	  if (!sym->n_zeroes) {
	      // see if this is the version of records with the off by
	      // two bug in stabs
	      // this assumes there are no strings of length 3 or less, but
	      //   those are represented inline on AIX
	      char *nmPtr;
	      if (!stabstr[sym->n_offset-3]) {
		  nmPtr = &stabstr[sym->n_offset];
	      } else {
		  nmPtr = &stabstr[sym->n_offset-2];
	      }
	      // printf("passing by %s\n", nmPtr);
	      parseStabString(this, 0, nmPtr, sym->n_value);
	  } else {
	      // names 8 or less chars on inline, not in stabstr
	      char tempName[9];
	      memset(tempName, 0, 9);
	      strncpy(tempName, sym->n_name, 8);
	      parseStabString(this, 0, tempName, sym->n_value);
	  }
      }
    }
}

#endif

#if defined(sparc_sun_solaris2_4) || \
    defined(i386_unknown_solaris2_5) || \
    defined(i386_unknown_linux2_0)

// Gets the stab and stabstring section and parses it for types
// and variables
void BPatch_module::parseTypes()
{
  char *modName, *ptr;
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

    case N_UNDF: /* start of object file */
	    /* value contains offset of the next string table for next module */
	    // assert(stabptr[i].name == 1);
	    stabstrs = stabstr_nextoffset;
	    stabstr_nextoffset = (char*)stabstrs + stabptr[i].val;
	    break;

    case N_ENDM: /* end of object file */
            break;

    case N_SO: /* compilation source or file name */
      /* printf("Resetting CURRENT FUNCTION NAME FOR NEXT OBJECT FILE\n");*/
            current_func_name = NULL; // reset for next object file
            modName = (char*)(&stabstrs[stabptr[i].name]);
            ptr = strrchr(modName, '/');
            if (ptr) {
                ptr++;
		modName = ptr;
	    }

	    if (!strcmp(modName, mod->fileName().string_of())) {
		parseActive = true;
	    } else {
		parseActive = false;
	    }
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

      // printf("stab #%d = %s\n", i, ptr);
      // may be nothing to parse - XXX  jdd 5/13/99
      temp = parseStabString(this, stabptr[i].desc, (char *)ptr, 
	  stabptr[i].val);
      if (*temp) {
	  //Error parsing the stabstr, return should be \0
	  fprintf(stderr, "Stab string parsing ERROR!! More to parse: %s\n",
	      temp);
      }
      break;

    default:
      break;
    }       		    
  }
}

#endif //end of #if defined(i386_unknown_linux2_0)

// Parsing symbol table for Alpha platform
// Mehmet

#if defined(alpha_dec_osf4_0)
extern void parseCoff(BPatch_module *mod, char *exeName, const string& modName);

void BPatch_module::parseTypes()
{
  image * imgPtr=NULL;

  //Using pdmodule to get the image Object.
  imgPtr = mod->exec();

  //Get the path name of the process
  char *file = (char *)(imgPtr->file()).string_of();

  // with BPatch_functions
  this->BPfuncs = this->getProcedures();

  parseCoff(this, file, mod->fileName());
}

#endif
