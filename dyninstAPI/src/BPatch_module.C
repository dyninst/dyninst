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

#define BPATCH_FILE

#include "process.h"
#include "symtab.h"
#include "showerror.h"
#include "BPatch.h"
#include "BPatch_module.h"
#include "BPatch_snippet.h" // For BPatch_function; remove if we move it
#include "BPatch_collections.h"
#include "common/h/String.h"
#include "BPatch_type.h"    // For BPatch_type related stuff
#include "BPatch_Vector.h"
#include "LineInformation.h"
#if defined(TIMED_PARSE)
#include <sys/time.h>
#endif
#ifdef TIMED_PARSE
int max_addr_per_line =0;
int max_line_per_addr =0;
#endif

char * current_func_name = NULL;


/*
 * BPatch_module::getSourceObj()
 *
 * Return the contained source objects (e.g. functions).
 *
 */
bool BPatch_module::getSourceObj(BPatch_Vector<BPatch_sourceObj *> &vect)
{
    BPatch_Vector<BPatch_function *> *temp;
    temp = getProcedures();
    if (temp) {
       vect = *(BPatch_Vector<BPatch_sourceObj *> *) temp;
       return true;
    } else {
       return false;
    }
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

    strncpy(buffer, str.c_str(), length);

    return buffer;
}


char *BPatch_module::getFullName(char *buffer, int length)
{
    string str = mod->fullName();

    strncpy(buffer, str.c_str(), length);

    return buffer;
}


BPatch_module::BPatch_module(process *_proc, pdmodule *_mod,BPatch_image *_img):
    proc(_proc), mod(_mod), img(_img), BPfuncs(NULL),lineInformation(NULL) 
{
#if defined(TIMED_PARSE)
  struct timeval starttime;
  gettimeofday(&starttime, NULL);
#endif

    _srcType = BPatch_sourceModule;
    nativeCompiler = _mod->exec()->isNativeCompiler();

    moduleTypes = new BPatch_typeCollection;

    // load all of the type information
#if !defined(mips_sgi_irix6_4)

    if (BPatch::bpatch->parseDebugInfo()){ 
#ifdef PARSE_ALL_AT_ONCE
	lineInformation = new LineInformation(mod->fileName());
#endif
	parseTypes();
#ifdef PARSE_ALL_AT_ONCE
	lineInformation->cleanEmptyFunctions();
#endif
	}
#endif

#if defined(TIMED_PARSE)
  struct timeval endtime;
  gettimeofday(&endtime, NULL);
  unsigned long lstarttime = starttime.tv_sec * 1000 * 1000 + starttime.tv_usec;
  unsigned long lendtime = endtime.tv_sec * 1000 * 1000 + endtime.tv_usec;
  unsigned long difftime = lendtime - lstarttime;
  double dursecs = difftime/(1000 );
  cout << __FILE__ << ":" << __LINE__ <<": BPatch_module("<< mod->fileName()
       <<") took "<<dursecs <<" msecs" << endl;
#endif

}

BPatch_module::~BPatch_module()
{
    delete moduleTypes;

    for (unsigned int f = 0; f < BPfuncs->size(); f++) {
	delete (*BPfuncs)[f];
    }
    delete BPfuncs;
    if(lineInformation) delete lineInformation;
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

    pdvector<function_base *> *funcs = mod->getFunctions();

    for (unsigned int f = 0; f < funcs->size(); f++)
	BPfuncs->push_back(
		proc->findOrCreateBPFunc((pd_Function*)(*funcs)[f], this));

    return BPfuncs;
}

/*
 * BPatch_module::findFunction
 *
 * Returns a vector of BPatch_function* with the same name that is provided or
 * NULL if no function with that name is in the module.  This function
 * searches the BPatch_function vector of the module followed by
 * the function_base of the module.  If a function_base is found, then
 * a BPatch_function is created and added to the BPatch_function vector of
 * the module.
 * name The name of function to look up.
 */

/*
extern bool buildDemangledName(const string &mangled, string &use, bool nativeCompiler);

BPatch_function * BPatch_module::findFunction(const char * name)
{
    // Did not find BPatch_function with name match in BPatch_function vector
    // trying pdmodule
    pd_Function *func = (pd_Function*)mod->findFunctionFromAll(name);

    if (func == NULL) {
	string fullname = string("_") + string(name);
	func = (pd_Function*)mod->findFunctionFromAll(fullname);
    }

    if (func == NULL) {
	//Try with demangled name
	string mangled_name = name;
	string demangled;
	if (buildDemangledName(mangled_name, demangled, nativeCompiler))
		func = (pd_Function*)mod->findFunctionFromAll(demangled);
    }

    if (func == NULL) {
	return NULL;
    }
  
    // Fortran compilers often put the _ at the end of the name
    if (func == NULL) {
	string fullname = string(name) + string("_");
	func = (pd_Function*)mod->findFunctionFromAll(fullname);
    }
 
    bool new_flag = !proc->PDFuncToBPFuncMap.defines(func);

    BPatch_function * bpfunc = proc->findOrCreateBPFunc(func, this);

    if(!new_flag)
	return bpfunc;

#if defined(sparc_sun_solaris2_4)
    // Adding new BPatch_Function to BPatch_function vector
    if (this->BPfuncs) this->BPfuncs->push_back(bpfunc);
#endif

    return bpfunc;
    
}
*/

BPatch_Vector<BPatch_function *> *
BPatch_module::findFunction(const char *name, BPatch_Vector<BPatch_function *> *funcs,
			    bool regex_case_sensitive)
{
  pdvector<function_base *> pdfuncs;

  if (!name) {
    cerr << __FILE__ << __LINE__ << ":  findFunction(NULL), failing "<<endl; 
    return NULL;
  }
  if (NULL == mod->findFunctionFromAll(string(name), &pdfuncs, regex_case_sensitive) 
      || !pdfuncs.size()) {
    string msg = string("Unable to find function: ") + string(name);
    BPatch_reportError(BPatchSerious, 100, msg.c_str());
    return NULL;
  } 

  // found function(s), translate to BPatch_functions  
  for (unsigned int i = 0; i < pdfuncs.size(); ++i) {
    BPatch_function * bpfunc = proc->findOrCreateBPFunc((pd_Function *)pdfuncs[i], this);
    funcs->push_back(bpfunc);
    if (!proc->PDFuncToBPFuncMap.defines(pdfuncs[i])) {
      this->BPfuncs->push_back(bpfunc);
    }
  }

  return funcs;
}

string* processDirectories(string* fn){
	if(!fn)
		return NULL;

	if(!strstr(fn->c_str(),"/./") &&
	   !strstr(fn->c_str(),"/../"))
		return fn;

	string* ret = NULL;
	char suffix[10] = "";
	char prefix[10] = "";
	char* pPath = new char[strlen(fn->c_str())+1];

	strcpy(pPath,fn->c_str());

	if(pPath[0] == '/')
           strcpy(prefix, "/");
	else
           strcpy(prefix, "");

	if(pPath[strlen(pPath)-1] == '/')
           strcpy(suffix, "/");
	else
           strcpy(suffix, "");

	int count = 0;
	char* pPathLocs[1024];
	char* p = strtok(pPath,"/");
	while(p){
		if(!strcmp(p,".")){
			p = strtok(NULL,"/");
			continue;
		}
		else if(!strcmp(p,"..")){
			count--;
			if(((count < 0) && (*prefix != '/')) || 
			   ((count >= 0) && !strcmp(pPathLocs[count],"..")))
			{
				count++;
				pPathLocs[count++] = p;
			}
			if(count < 0) count = 0;
		}
		else
			pPathLocs[count++] = p;

		p = strtok(NULL,"/");
	}

	ret = new string;
	*ret += prefix;
	for(int i=0;i<count;i++){
		*ret += pPathLocs[i];
		if(i != (count-1))
			*ret += "/";
	}
	*ret += suffix;

	delete[] pPath;
	delete fn;
	return ret;
}
extern char *parseStabString(BPatch_module *, int linenum, char *str, 
	int fPtr, BPatch_type *commonBlock = NULL);

#if defined(rs6000_ibm_aix4_1)

#include <linenum.h>
#include <syms.h>

void parseLineInformation(process* proc,LineInformation* lineInformation,
			  BPatch_Vector<IncludeFileInfo>& includeFiles,string* currentSourceFile,
			  char* symbolName,
			  SYMENT *sym,
			  Address linesfdptr,char* lines,int nlines)
{
      unsigned int j;
      union auxent *aux;

      /* if it is beginning of include files then update the data structure 
         that keeps the beginning of the include files. If the include files contain 
         information about the functions and lines we have to keep it */
      if (sym->n_sclass == C_BINCL){
		includeFiles.push_back(IncludeFileInfo((sym->n_value-linesfdptr)/LINESZ, symbolName));
      }
      /* similiarly if the include file contains function codes and line information
         we have to keep the last line information entry for this include file */
      else if (sym->n_sclass == C_EINCL){
		if (includeFiles.size() > 0) {
			includeFiles[includeFiles.size()-1].end = (sym->n_value-linesfdptr)/LINESZ;
		}
      }
      /* if the enrty is for a function than we have to collect all info
         about lines of the function */
      else if (sym->n_sclass == C_FUN){
		assert(currentSourceFile);
		/* creating the string for function name */
		char* ce = strchr(symbolName,':'); if(ce) *ce = '\0';
		string currentFunctionName(symbolName);

		/* getting the real function base address from the symbols*/
    		Address currentFunctionBase=0;
		Symbol info;
		proc->getSymbolInfo(currentFunctionName,info,
				    currentFunctionBase);
		currentFunctionBase += info.addr();

		/* getting the information about the function from C_EXT */
		int initialLine = 0;
		int initialLineIndex = 0;
		Address funcStartAddress = 0;
		for(j=-1;;j--){
			SYMENT *extsym = (SYMENT*)(((unsigned)sym)+j*SYMESZ);
			if(extsym->n_sclass == C_EXT){
				aux = (union auxent*)((char*)extsym+SYMESZ);
#ifndef __64BIT__
				initialLineIndex = (aux->x_sym.x_fcnary.x_fcn.x_lnnoptr-linesfdptr)/LINESZ;
#endif
				funcStartAddress = extsym->n_value;
				break;
			}
		}

		/* access the line information now using the C_FCN entry*/
		SYMENT *bfsym = (SYMENT*)(((unsigned)sym)+SYMESZ);

		if (bfsym->n_sclass != C_FCN) {
		    printf("unable to process line info for %s\n", symbolName);
		    return;
		}

		aux = (union auxent*)((char*)bfsym+SYMESZ);
		initialLine = aux->x_sym.x_misc.x_lnsz.x_lnno;

		string whichFile = *currentSourceFile;
		/* find in which file is it */
		for(j=0;j<includeFiles.size();j++)
			if((includeFiles[j].begin <= (unsigned)initialLineIndex) &&
			   (includeFiles[j].end >= (unsigned)initialLineIndex)){
				whichFile = includeFiles[j].name;
				break;
			}

		FunctionInfo* currentFuncInfo = NULL;
		FileLineInformation* currentFileInfo = NULL;
		lineInformation->insertSourceFileName(currentFunctionName,whichFile,
						&currentFileInfo,&currentFuncInfo);

		for(j=initialLineIndex+1;j<nlines;j++){
			LINENO* lptr = (LINENO*)(lines+j*LINESZ);
			if(!lptr->l_lnno)
				break;
			if(currentFileInfo)
	    			currentFileInfo->insertLineAddress(
					currentFuncInfo,
					lptr->l_lnno+initialLine-1,
					(lptr->l_addr.l_paddr-funcStartAddress)+currentFunctionBase);
		}
      }
}


// Gets the stab and stabstring section and parses it for types
// and variables
void BPatch_module::parseTypes()
{
    int i, j;
    int nlines;
    int nstabs;
    char* lines;
    SYMENT *syms;
    SYMENT *tsym;
    char *stringPool;
    char tempName[9];
    char *stabstr=NULL;
    union auxent *aux;
    image * imgPtr=NULL;
    char* funcName = NULL;
    Address staticBlockBaseAddr;
    unsigned long linesfdptr;
    BPatch_type *commonBlock = NULL;
    BPatch_variableExpr *commonBlockVar;
    string* currentSourceFile = NULL;

#if defined(TIMED_PARSE)
  struct timeval starttime;
  gettimeofday(&starttime, NULL);
#endif
    BPatch_Vector<IncludeFileInfo> includeFiles;

    //Using pdmodule to get the image Object.
    imgPtr = mod->exec();

    //Using the image to get the Object (class)
    Object *objPtr = (Object *) &(imgPtr->getObject());

    //Using the Object to get the pointers to the .stab and .stabstr
    objPtr->get_stab_info(stabstr, nstabs, ((Address&) syms), stringPool); 

    this->BPfuncs = this->getProcedures();


    objPtr->get_line_info(nlines,lines,linesfdptr); 

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

	 if(currentSourceFile) delete currentSourceFile;
	 currentSourceFile = new string(moduleName);
	 currentSourceFile = processDirectories(currentSourceFile);

	 if (strrchr(moduleName, '/')) {
	     moduleName = strrchr(moduleName, '/');
	     moduleName++;
	 }

	 if (!strcmp(moduleName, mod->fileName().c_str())) {
		parseActive = true;
	 } else {
		parseActive = false;
	 }
      }

      if (!parseActive) continue;

      char *nmPtr;
      if (!sym->n_zeroes && ((sym->n_sclass & DBXMASK) ||
			     (sym->n_sclass == C_BINCL) ||
			     (sym->n_sclass == C_EINCL))) {
	  if (sym->n_offset < 3) {
	      if (sym->n_offset == 2 && stabstr[0]) {
		  nmPtr = &stabstr[0];
	      } else {
		  nmPtr = &stabstr[sym->n_offset];
	      }
	  } else if (!stabstr[sym->n_offset-3]) {
	      nmPtr = &stabstr[sym->n_offset];
	  } else {
	      /* has off by two error */
	      nmPtr = &stabstr[sym->n_offset-2];
	  }
#ifdef notdef
	  printf("using nmPtr = %s\n", nmPtr);
	  printf("got n_offset = (%d) %s\n", sym->n_offset, &stabstr[sym->n_offset]);
	  if (sym->n_offset>=2) 
	      printf("got n_offset-2 = %s\n", &stabstr[sym->n_offset-2]);
	  if (sym->n_offset>=3) 
	      printf("got n_offset-3 = %x\n", stabstr[sym->n_offset-3]);
	  if (sym->n_offset>=4) 
	      printf("got n_offset-4 = %x\n", stabstr[sym->n_offset-4]);
#endif
      } else {
	  // names 8 or less chars on inline, not in stabstr
	  memset(tempName, 0, 9);
	  strncpy(tempName, sym->n_name, 8);
	  nmPtr = tempName;
      }

      if ((sym->n_sclass == C_BINCL) ||
	  (sym->n_sclass == C_EINCL) ||
	  (sym->n_sclass == C_FUN)) {
		if (funcName) { 
		    free(funcName);
		    funcName = NULL;
		}
		funcName = strdup(nmPtr);
		parseLineInformation(proc,lineInformation,includeFiles,
				     currentSourceFile,funcName,sym,
				     linesfdptr,lines,nlines);
      }

      if (sym->n_sclass & DBXMASK) {
	  if (sym->n_sclass == C_BCOMM) {
	      char *commonBlockName;

	      commonBlockName = nmPtr;

	      // find the variable for the common block
	      BPatch_image *progam = (BPatch_image *) getObjParent();
	      
	      commonBlockVar = progam->findVariable(commonBlockName);
	      if (!commonBlockVar) {
		  printf("unable to find variable %s\n", commonBlockName);
	      } else {
		  commonBlock = 
		      const_cast<BPatch_type *> (commonBlockVar->getType());
		  if (commonBlock->getDataClass() != BPatch_dataCommon) {
		      // its still the null type, create a new one for it
		      commonBlock = new BPatch_type(commonBlockName, false);
		      commonBlockVar->setType(commonBlock);
		      moduleTypes->addGlobalVariable(commonBlockName, commonBlock);

		      commonBlock->setDataClass(BPatch_dataCommon);
		  }
		  // reset field list
		  commonBlock->beginCommonBlock();
	      }
	  } else if (sym->n_sclass == C_ECOMM) {
	      // copy this set of fields
	    BPatch_Vector<BPatch_function *> bpmv;
   	    if (NULL == findFunction(funcName, &bpmv) || !bpmv.size()) {
	      printf("unable to locate current function %s\n", funcName);
	      } else {
		BPatch_function *func = bpmv[0];
		commonBlock->endCommonBlock(func, commonBlockVar->getBaseAddr());
	      }

	      // update size if needed
	      if (commonBlockVar)
		  commonBlockVar->setSize(commonBlock->getSize());
	      commonBlockVar = NULL;
	      commonBlock = NULL;
	  } else if (sym->n_sclass == C_BSTAT) {
	      char *staticName, tempName[9];
	      // begin static block
	      // find the variable for the common block
	      tsym = (SYMENT *) (((unsigned) syms) + sym->n_value * SYMESZ);

	      if (!tsym->n_zeroes) {
		  staticName = &stringPool[tsym->n_offset];
	      } else {
		  memset(tempName, 0, 9);
		  strncpy(tempName, tsym->n_name, 8);
		  staticName = tempName;
	      }
	      BPatch_image *progam = (BPatch_image *) getObjParent();

	      BPatch_variableExpr *staticBlockVar = progam->findVariable(staticName);
	      if (!staticBlockVar) {
		  printf("unable to find static block %s\n", staticName);
		  staticBlockBaseAddr = 0;
	      } else {
		  staticBlockBaseAddr = (Address) staticBlockVar->getBaseAddr();
	      }
	  } else if (sym->n_sclass == C_ESTAT) {
	      staticBlockBaseAddr = 0;
	  }

	  if (staticBlockBaseAddr && (sym->n_sclass == C_STSYM)) {
	      parseStabString(this, 0, nmPtr, 
		  sym->n_value+staticBlockBaseAddr, commonBlock);
	  } else {
	      parseStabString(this, 0, nmPtr, sym->n_value, commonBlock);
	  }
      }
    }
#if defined(TIMED_PARSE)
  struct timeval endtime;
  gettimeofday(&endtime, NULL);
  unsigned long lstarttime = starttime.tv_sec * 1000 * 1000 + starttime.tv_usec;
  unsigned long lendtime = endtime.tv_sec * 1000 * 1000 + endtime.tv_usec;
  unsigned long difftime = lendtime - lstarttime;
  double dursecs = difftime/(1000 );
  cout << __FILE__ << ":" << __LINE__ <<": parseTypes("<< mod->fileName()
       <<") took "<<dursecs <<" msecs" << endl;
#endif
}

#endif

#if defined(sparc_sun_solaris2_4) || \
    defined(i386_unknown_solaris2_5) || \
    defined(i386_unknown_linux2_0) || \
    defined(ia64_unknown_linux2_4) /* Temporary duplication -- TLM. */

#ifndef PARSE_ALL_AT_ONCE
// parseTypes:  parses type and variable info, does some init
//              does NOT parse file-line info anymore, this is done later, upon request.
void BPatch_module::parseTypes() 
{
  int stab_nsyms;
  char *stabstr_nextoffset;
  const char *stabstrs = 0;
  struct stab_entry *stabptr = NULL;
  int i;
  char *modName;
  char * temp=NULL;
  image * imgPtr=NULL;
  char *ptr, *ptr2, *ptr3;
  bool parseActive = false;

  string* currentFunctionName = NULL;
  Address currentFunctionBase = 0;
  BPatch_variableExpr *commonBlockVar = NULL;
 char *commonBlockName;
  BPatch_type *commonBlock = NULL;
 int mostRecentLinenum = 0;

#if defined(TIMED_PARSE)
  struct timeval starttime;
  gettimeofday(&starttime, NULL);
  unsigned int pss_count = 0;
  double pss_dur = 0;
  unsigned int src_count = 0;
  double src_dur = 0;
  unsigned int fun_count = 0;
  double fun_dur = 0;
  struct timeval t1, t2;
#endif

  //Using pdmodule to get the image Object.
  imgPtr = mod->exec();

  // Building the BPatch_Vector<BPatch_function *> for use later when playing
  // with BPatch_functions
  this->BPfuncs = this->getProcedures();


  //Using the image to get the Object (class)
  Object *objPtr = (Object *) &(imgPtr->getObject());

  //Using the Object to get the pointers to the .stab and .stabstr
  // XXX - Elf32 specific needs to be in seperate file -- jkh 3/18/99
  objPtr->get_stab_info((void **) &stabptr, stab_nsyms, 
	(void **) &stabstr_nextoffset);

  // cerr << "checkpoint A" << endl;

  for(i=0;i<stab_nsyms;i++){
    // if (stabstrs) printf("parsing #%d, %s\n", stabptr[i].type, &stabstrs[stabptr[i].name]);
    switch(stabptr[i].type){
    case N_UNDF: /* start of object file */
      /* value contains offset of the next string table for next module */
      // assert(stabptr[i].name == 1);
      stabstrs = stabstr_nextoffset;
      stabstr_nextoffset = (char*)stabstrs + stabptr[i].val;
      
      //N_UNDF is the start of object file. It is time to 
      //clean source file name at this moment.
      /*
      if(currentSourceFile){
	delete currentSourceFile;
	currentSourceFile = NULL;
	delete absoluteDirectory;
	absoluteDirectory = NULL;
	delete currentFunctionName;
	currentFunctionName = NULL;
	currentFileInfo = NULL;
	currentFuncInfo = NULL;
      }
      */
      break;
      
    case N_ENDM: /* end of object file */
      break;

    case N_SO: /* compilation source or file name */
      /* printf("Resetting CURRENT FUNCTION NAME FOR NEXT OBJECT FILE\n");*/
#ifdef TIMED_PARSE
      src_count++;
      gettimeofday(&t1, NULL);
#endif
      current_func_name = NULL; // reset for next object file
      modName = (char*)(&stabstrs[stabptr[i].name]);
      // cerr << "checkpoint B" << endl;
      ptr = strrchr(modName, '/');
      //  cerr << "checkpoint C" << endl;
      if (ptr) {
	ptr++;
	modName = ptr;
      }

      if (!strcmp(modName, mod->fileName().c_str())) {
	parseActive = true;
	switch (stabptr[i].desc) {
	case N_SO_FORTRAN:
	  setLanguage(BPatch_fortran);
	  break;
	  
	case N_SO_F90:
	  setLanguage(BPatch_fortran90);
	  break;
	  
	case N_SO_AS:
	  setLanguage(BPatch_assembly);
	  break;
	  
	case N_SO_ANSI_C:
	case N_SO_C:
	  setLanguage(BPatch_c);
	  break;
	  
	case N_SO_CC:
	  setLanguage(BPatch_cPlusPlus);
	  break;
	  
	default:
	  setLanguage(BPatch_unknownLanguage);
	  break;
	}
      } else {
	parseActive = false;
      }

#ifdef TIMED_PARSE
	    gettimeofday(&t2, NULL);
	    src_dur += (t2.tv_sec - t1.tv_sec)*1000.0 + (t2.tv_usec - t1.tv_usec)/1000.0;
	    //src_dur += (t2.tv_sec/1000 + t2.tv_usec*1000) - (t1.tv_sec/1000 + t1.tv_usec*1000) ;
#endif
	    break;
    case N_SLINE:
      mostRecentLinenum = stabptr[i].desc;
      break;
    default:
      break;
    }


    if(parseActive || mod->isShared()) {
      BPatch_Vector<BPatch_function *> bpfv;
      switch(stabptr[i].type){
      case N_FUN:
#ifdef TIMED_PARSE
	fun_count++;
	gettimeofday(&t1, NULL);
#endif
	//all we have to do with function stabs at this point is to assure that we have
	//properly set the var currentFunctionName for the later case of (parseActive)
      int currentEntry = i;
      ptr = new char[1024];
      strcpy(ptr,(const char *)&stabstrs[stabptr[currentEntry].name]);
      while(strlen(ptr) != 0 && ptr[strlen(ptr)-1] == '\\'){
	ptr[strlen(ptr)-1] = '\0';
	currentEntry++;
	strcat(ptr,(const char *)&stabstrs[stabptr[currentEntry].name]);
      }
      
      char* colonPtr = NULL;
      if(currentFunctionName) delete currentFunctionName;
      if(!ptr || !(colonPtr = strchr(ptr,':')))
	currentFunctionName = NULL;
      else {
	char* tmp = new char[colonPtr-ptr+1];
	strncpy(tmp,ptr,colonPtr-ptr);
	tmp[colonPtr-ptr] = '\0';
	currentFunctionName = new string(tmp);
      }
      //  used to be a symbol lookup here to find currentFunctionBase, do we need it?
      delete[] ptr;
#ifdef TIMED_PARSE
      gettimeofday(&t2, NULL);
      fun_dur += (t2.tv_sec - t1.tv_sec)*1000.0 + (t2.tv_usec - t1.tv_usec)/1000.0;
      //fun_dur += (t2.tv_sec/1000 + t2.tv_usec*1000) - (t1.tv_sec/1000 + t1.tv_usec*1000);
#endif
      break;
      }
    if (!parseActive) continue;

    switch(stabptr[i].type){
      case N_BCOMM:	{
	// begin Fortran named common block 
	commonBlockName = (char *) &stabstrs[stabptr[i].name];

	// find the variable for the common block
	BPatch_image *progam = (BPatch_image *) getObjParent();
#ifdef TIMED_PARSE
	cout << __FILE__ << __LINE__ << ": parseTypes N_BCOMM "<< commonBlockName << endl;
#endif
	commonBlockVar = progam->findVariable(commonBlockName);
	if (!commonBlockVar) {
	  printf("unable to find variable %s\n", commonBlockName);
	} else {
	  commonBlock = const_cast<BPatch_type *> (commonBlockVar->getType());
	  if (commonBlock->getDataClass() != BPatch_dataCommon) {
	    // its still the null type, create a new one for it
	    commonBlock = new BPatch_type(commonBlockName, false);
	    commonBlockVar->setType(commonBlock);
	    moduleTypes->addGlobalVariable(commonBlockName, commonBlock);
	    
	    commonBlock->setDataClass(BPatch_dataCommon);
	  }
	  // reset field list
	  commonBlock->beginCommonBlock();
	}
	break;
      }
      
      case N_ECOMM: {
	// copy this set of fields
	
	assert(currentFunctionName);
#ifdef TIMED_PARSE
	cout << __FILE__ << __LINE__ << ": parseTypes N_ECOMM "<< currentFunctionName << endl;
#endif
	if (NULL == findFunction(currentFunctionName->c_str(), &bpfv) || !bpfv.size()) {
	  printf("unable to locate current function %s\n", currentFunctionName->c_str());
	} else {
	  if (bpfv.size() > 1) {
	    // warn if we find more than one function with this name
	    printf("%s[%d]:  WARNING: found %d funcs matching name %s, using the first\n",
		   __FILE__, __LINE__, bpfv.size(), currentFunctionName->c_str());
	  }
	  
	  BPatch_function *func = bpfv[0];
	  commonBlock->endCommonBlock(func, commonBlockVar->getBaseAddr());
	}
	
	// update size if needed
	if (commonBlockVar)
	  commonBlockVar->setSize(commonBlock->getSize());
	commonBlockVar = NULL;
	commonBlock = NULL;
	break;
      }
      
      // case C_BINCL: -- what is the elf version of this jkh 8/21/01
      // case C_EINCL: -- what is the elf version of this jkh 8/21/01
      case 32:    // Global symbols -- N_GYSM 
      case N_FUN:
      case 128:   // typedefs and variables -- N_LSYM
      case 160:   // parameter variable -- N_PSYM 
#ifdef TIMED_PARSE
	pss_count++;
	gettimeofday(&t1, NULL);
#endif
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
	if (nativeCompiler)
	  temp = parseStabString(this, mostRecentLinenum, (char *)ptr, stabptr[i].val, commonBlock);
	else
	  temp = parseStabString(this, stabptr[i].desc, (char *)ptr, stabptr[i].val, commonBlock);
	if (*temp) {
	  //Error parsing the stabstr, return should be \0
	  fprintf(stderr, "Stab string parsing ERROR!! More to parse: %s\n",
		  temp);
	}
	
#ifdef TIMED_PARSE
	gettimeofday(&t2, NULL);
	pss_dur += (t2.tv_sec - t1.tv_sec)*1000.0 + (t2.tv_usec - t1.tv_usec)/1000.0;
	//      pss_dur += (t2.tv_sec/1000 + t2.tv_usec*1000) - (t1.tv_sec/1000 + t1.tv_usec*1000);
#endif
	break;
      default:
	break;
      }
    }       		    
  }

#if defined(TIMED_PARSE)
  struct timeval endtime;
  gettimeofday(&endtime, NULL);
  unsigned long lstarttime = starttime.tv_sec * 1000 * 1000 + starttime.tv_usec;
  unsigned long lendtime = endtime.tv_sec * 1000 * 1000 + endtime.tv_usec;
  unsigned long difftime = lendtime - lstarttime;
  double dursecs = difftime/(1000 );
  cout << __FILE__ << ":" << __LINE__ <<": parseTypes("<< mod->fileName()
       <<") took "<<dursecs <<" msecs" << endl;
  cout << "Breakdown:" << endl;
  cout << "     Functions: " << fun_count << " took " << fun_dur << "msec" << endl;
  cout << "     Sources: " << src_count << " took " << src_dur << "msec" << endl;
  cout << "     parseStabString: " << pss_count << " took " << pss_dur << "msec" << endl;
  cout << "     Total: " << pss_dur + fun_dur + src_dur 
       << " msec" << endl;
#endif
}

// parseLineInformation():  parses symtab for file and line info.  Should not be called before parseTypes
// the ptr to lineInformation should be NULL before this is called.
#if !defined(rs6000_ibm_aix4_1)
void BPatch_module::parseFileLineInfo() 
{
 int i;
  char *modName;
  image * imgPtr=NULL;
  char *ptr;
  int stab_nsyms;
  char *stabstr_nextoffset;
  const char *stabstrs = 0;
  struct stab_entry *stabptr = NULL;
  int parseActive = false; 

  if (lineInformation) {
    cerr << __FILE__ << ":" << __LINE__ << ": Internal error, not fatal, probabl:, duplicated call to"
	 << "parseFileLineInfo()...  ignoring" << endl;
  }

  lineInformation = new LineInformation(mod->fileName());
 

#if defined(TIMED_PARSE)
  struct timeval starttime;
  gettimeofday(&starttime, NULL);
  unsigned int fun_count = 0;
  double fun_dur = 0;
  unsigned int src_count = 0;
  double src_dur = 0;
  unsigned int sol_count = 0;
  double sol_dur = 0;
  unsigned int sline_count = 0;
  double sline_dur = 0;
  struct timeval t1, t2;
#endif
 //Using pdmodule to get the image Object.
  imgPtr = mod->exec();

  //Using the image to get the Object (class)
  Object *objPtr = (Object *) &(imgPtr->getObject());
  assert (objPtr);

  //Using the Object to get the pointers to the .stab and .stabstr
  // XXX - Elf32 specific needs to be in seperate file -- jkh 3/18/99
  objPtr->get_stab_info((void **) &stabptr, stab_nsyms, 
	(void **) &stabstr_nextoffset);


  //these variables are used to keep track of the source files
  //and function names being processes at a moment

  string* currentFunctionName = NULL;
  Address currentFunctionBase = 0;
  string* currentSourceFile = NULL;
  string* absoluteDirectory = NULL;
  FunctionInfo* currentFuncInfo = NULL;
  FileLineInformation* currentFileInfo = NULL;

  for(i=0;i<stab_nsyms;i++){
    // if (stabstrs) printf("parsing #%d, %s\n", stabptr[i].type, &stabstrs[stabptr[i].name]);
    switch(stabptr[i].type){

    case N_UNDF: /* start of object file */
	    /* value contains offset of the next string table for next module */
	    // assert(stabptr[i].name == 1);
	    stabstrs = stabstr_nextoffset;
	    stabstr_nextoffset = (char*)stabstrs + stabptr[i].val;

	    //N_UNDF is the start of object file. It is time to 
	    //clean source file name at this moment.
	    if(currentSourceFile){
	  	delete currentSourceFile;
		currentSourceFile = NULL;
		delete absoluteDirectory;
		absoluteDirectory = NULL;
		currentFileInfo = NULL;
		currentFuncInfo = NULL;
	    }
	    break;

    case N_SO: /* compilation source or file name */
      /* printf("Resetting CURRENT FUNCTION NAME FOR NEXT OBJECT FILE\n");*/
#ifdef TIMED_PARSE
      src_count++;
      gettimeofday(&t1, NULL);
#endif
            current_func_name = NULL; // reset for next object file

	    //  JAW -- not sure we need this block here
            modName = (char*)(&stabstrs[stabptr[i].name]);
            ptr = strrchr(modName, '/');
            if (ptr) {
                ptr++;
		modName = ptr;
	    }

	    if (!strcmp(modName, mod->fileName().c_str())) 
	      parseActive = true;
	    else
	      parseActive = false;
	    //time to create the source file name to be used
	    //for latter processing of line information
	    if(!currentSourceFile){
		currentSourceFile = new string(&stabstrs[stabptr[i].name]);
	    	absoluteDirectory = new string(*currentSourceFile);
	    }
	    else if(!strlen(&stabstrs[stabptr[i].name])){
		delete currentSourceFile;
		currentSourceFile = NULL;
		delete absoluteDirectory;
		absoluteDirectory = NULL;
	    }
	    else
		*currentSourceFile += &stabstrs[stabptr[i].name];

	    currentSourceFile = processDirectories(currentSourceFile);
 

#ifdef TIMED_PARSE
	    gettimeofday(&t2, NULL);
	    src_dur += (t2.tv_sec - t1.tv_sec)*1000.0 + (t2.tv_usec - t1.tv_usec)/1000.0;
	    //src_dur += (t2.tv_sec/1000 + t2.tv_usec*1000) - (t1.tv_sec/1000 + t1.tv_usec*1000) ;
#endif
           break;
    default:
      break;
    }

    if( parseActive || mod->isShared())
      switch(stabptr[i].type){
      case N_SOL:
#ifdef TIMED_PARSE
	sol_count++;
	gettimeofday(&t1, NULL);
#endif
	if(absoluteDirectory){
	  const char* newSuffix = &stabstrs[stabptr[i].name];
	  if(newSuffix[0] == '/'){
	    delete currentSourceFile;
	    currentSourceFile = new string;
	  }
	  else{
	    char* tmp = new char[absoluteDirectory->length()+1];
	    strcpy(tmp,absoluteDirectory->c_str());
	    char* p=strrchr(tmp,'/');
	    if(p) 
	      *(++p)='\0';
	    delete currentSourceFile;
	    currentSourceFile = new string(tmp);
	    delete[] tmp;
	  }
	  (*currentSourceFile) += newSuffix;
	  currentSourceFile = processDirectories(currentSourceFile);
	  if(currentFunctionName)
	    lineInformation->insertSourceFileName(
						  *currentFunctionName,
						  *currentSourceFile,
						  &currentFileInfo,&currentFuncInfo);
	}
	else{
	  currentSourceFile = new string(&stabstrs[stabptr[i].name]);
	  currentSourceFile = processDirectories(currentSourceFile);
	  if(currentFunctionName)
	    lineInformation->insertSourceFileName(
						  *currentFunctionName,
						  *currentSourceFile,
						  &currentFileInfo,&currentFuncInfo);
	}
#ifdef TIMED_PARSE
	gettimeofday(&t2, NULL);
	sol_dur += (t2.tv_sec - t1.tv_sec)*1000.0 + (t2.tv_usec - t1.tv_usec)/1000.0;
	//sol_dur += (t2.tv_sec/1000 + t2.tv_usec*1000) - (t1.tv_sec/1000 + t1.tv_usec*1000); 
#endif
	break;
      case N_SLINE:
#ifdef TIMED_PARSE
	sline_count++;
	gettimeofday(&t1, NULL);
#endif
	//if the stab information is a line information
	//then insert an entry to the line info object
	if(!currentFunctionName) break;
	if(currentFileInfo)
	  currentFileInfo->insertLineAddress(currentFuncInfo,
					     stabptr[i].desc,
					     stabptr[i].val+currentFunctionBase);
#ifdef TIMED_PARSE
	gettimeofday(&t2, NULL);
	sline_dur += (t2.tv_sec - t1.tv_sec)*1000.0 + (t2.tv_usec - t1.tv_usec)/1000.0;
	//sline_dur += (t2.tv_sec/1000 + t2.tv_usec*1000) - (t1.tv_sec/1000 + t1.tv_usec*1000) ;
#endif
	break;
    case N_FUN:
#ifdef TIMED_PARSE
      fun_count++;
      gettimeofday(&t1, NULL);
#endif
      //if it is a function stab then we have to insert an entry 
      //to initialize the entries in the line information object
      int currentEntry = i;
      ptr = new char[1024];
      strcpy(ptr,(const char *)&stabstrs[stabptr[currentEntry].name]);
      while(strlen(ptr) != 0 && ptr[strlen(ptr)-1] == '\\'){
	ptr[strlen(ptr)-1] = '\0';
	currentEntry++;
	strcat(ptr,(const char *)&stabstrs[stabptr[currentEntry].name]);
      }
      
      char* colonPtr = NULL;
      if(currentFunctionName) delete currentFunctionName;
      if(!ptr || !(colonPtr = strchr(ptr,':')))
	currentFunctionName = NULL;
      else {
	char* tmp = new char[colonPtr-ptr+1];
	strncpy(tmp,ptr,colonPtr-ptr);
	tmp[colonPtr-ptr] = '\0';
	currentFunctionName = new string(tmp);
	
	currentFunctionBase = 0;
	Symbol info;
	if (!proc->getSymbolInfo(*currentFunctionName,
				 info,currentFunctionBase))
	  {
	    string fortranName = *currentFunctionName + string("_");
	    if (proc->getSymbolInfo(fortranName,info,
				    currentFunctionBase))
	      {
		delete currentFunctionName;
		currentFunctionName = new string(fortranName);
	      }
	  }
	
	currentFunctionBase += info.addr();
	
	delete[] tmp;		
	if(currentSourceFile)
	  lineInformation->insertSourceFileName(
						*currentFunctionName,
						*currentSourceFile,
						&currentFileInfo,&currentFuncInfo);
      }
      delete[] ptr;
#ifdef TIMED_PARSE
      gettimeofday(&t2, NULL);
      fun_dur += (t2.tv_sec - t1.tv_sec)*1000.0 + (t2.tv_usec - t1.tv_usec)/1000.0;
      //fun_dur += (t2.tv_sec/1000 + t2.tv_usec*1000) - (t1.tv_sec/1000 + t1.tv_usec*1000);
#endif
      break;
      }
  }

  lineInformation->cleanEmptyFunctions();

#if defined(TIMED_PARSE)
  struct timeval endtime;
  gettimeofday(&endtime, NULL);
  unsigned long lstarttime = starttime.tv_sec * 1000 * 1000 + starttime.tv_usec;
  unsigned long lendtime = endtime.tv_sec * 1000 * 1000 + endtime.tv_usec;
  unsigned long difftime = lendtime - lstarttime;
  double dursecs = difftime/(1000 );
  cout << __FILE__ << ":" << __LINE__ <<": parseTypes("<< mod->fileName()
       <<") took "<<dursecs <<" msecs" << endl;
  cout << "Breakdown:" << endl;
  cout << "     Functions: " << fun_count << " took " << fun_dur << "msec" << endl;
  cout << "     Sources: " << src_count << " took " << src_dur << "msec" << endl;
  cout << "     SOL?s: " << sol_count << " took " << sol_dur << "msec" << endl;
  cout << "     Sliness: " << sline_count << " took " << sline_dur << "msec" << endl;
  cout << "     Total: " << sline_dur + sol_dur + fun_dur + src_dur 
       << " msec" << endl;
  lineInformation->print();
  cout << "Max addr per line = " << max_addr_per_line << ".  Max line per addr = "
       << max_line_per_addr << endl;
#endif
}
#endif //!rs6k
#endif // notdef PARSE_ALL_AT_ONCE

#ifdef PARSE_ALL_AT_ONCE
// Gets the stab and stabstring section and parses it for types
// and variables
void BPatch_module::parseTypes()
{
 
  int i;
  char *modName;
  char * temp=NULL;
  image * imgPtr=NULL;

  char *ptr, *ptr2, *ptr3;
  bool parseActive = false;
  int stab_nsyms;
  char *stabstr_nextoffset;
  const char *stabstrs = 0;
  struct stab_entry *stabptr = NULL;
  char *commonBlockName;
  BPatch_type *commonBlock = NULL;
  BPatch_variableExpr *commonBlockVar = NULL;
  assert(lineInformation); // should not be NULL under PARSE_ALL_AT_ONCE
#if defined(TIMED_PARSE)
  struct timeval starttime;
  gettimeofday(&starttime, NULL);
#endif
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

  //these variables are used to keep track of the source files
  //and function names being processes at a moment

  string* currentFunctionName = NULL;
  Address currentFunctionBase = 0;
  string* currentSourceFile = NULL;
  string* absoluteDirectory = NULL;
  FunctionInfo* currentFuncInfo = NULL;
  FileLineInformation* currentFileInfo = NULL;
  
  int mostRecentLinenum = 0;

#ifdef TIMED_PARSE
  unsigned int fun_count = 0;
  double fun_dur = 0;
  unsigned int src_count = 0;
  double src_dur = 0;
  unsigned int sol_count = 0;
  double sol_dur = 0;
  unsigned int sline_count = 0;
  double sline_dur = 0;
  unsigned int pss_count = 0;
  double pss_dur = 0;
  struct timeval t1, t2;
  double dsec1, dsec2, dusec1, dusec2;
#endif

  for(i=0;i<stab_nsyms;i++){
    // if (stabstrs) printf("parsing #%d, %s\n", stabptr[i].type, &stabstrs[stabptr[i].name]);
    switch(stabptr[i].type){

    case N_UNDF: /* start of object file */
	    /* value contains offset of the next string table for next module */
	    // assert(stabptr[i].name == 1);
	    stabstrs = stabstr_nextoffset;
	    stabstr_nextoffset = (char*)stabstrs + stabptr[i].val;

	    //N_UNDF is the start of object file. It is time to 
	    //clean source file name at this moment.
	    if(currentSourceFile){
	  	delete currentSourceFile;
		currentSourceFile = NULL;
		delete absoluteDirectory;
		absoluteDirectory = NULL;
		currentFileInfo = NULL;
		currentFuncInfo = NULL;
	    }
	    break;

    case N_ENDM: /* end of object file */
            break;

    case N_SO: /* compilation source or file name */
      /* printf("Resetting CURRENT FUNCTION NAME FOR NEXT OBJECT FILE\n");*/
#ifdef TIMED_PARSE
      src_count++;
      gettimeofday(&t1, NULL);
#endif
            current_func_name = NULL; // reset for next object file
            modName = (char*)(&stabstrs[stabptr[i].name]);
            ptr = strrchr(modName, '/');
            if (ptr) {
                ptr++;
		modName = ptr;
	    }

	    if (!strcmp(modName, mod->fileName().c_str())) {
		parseActive = true;
		switch (stabptr[i].desc) {
		    case N_SO_FORTRAN:
			setLanguage(BPatch_fortran);
			break;

		    case N_SO_F90:
			setLanguage(BPatch_fortran90);
			break;

		    case N_SO_AS:
			setLanguage(BPatch_assembly);
			break;

	            case N_SO_ANSI_C:
		    case N_SO_C:
			setLanguage(BPatch_c);
			break;

		    case N_SO_CC:
			setLanguage(BPatch_cPlusPlus);
			break;

		    default:
			setLanguage(BPatch_unknownLanguage);
			break;
		}
	    } else {
		parseActive = false;
	    }

	    //time to create the source file name to be used
	    //for latter processing of line information
	    if(!currentSourceFile){
		currentSourceFile = new string(&stabstrs[stabptr[i].name]);
	    	absoluteDirectory = new string(*currentSourceFile);
	    }
	    else if(!strlen(&stabstrs[stabptr[i].name])){
		delete currentSourceFile;
		currentSourceFile = NULL;
		delete absoluteDirectory;
		absoluteDirectory = NULL;
	    }
	    else
		*currentSourceFile += &stabstrs[stabptr[i].name];

	    currentSourceFile = processDirectories(currentSourceFile);
 
#ifdef TIMED_PARSE
	    gettimeofday(&t2, NULL);
	    src_dur += (t2.tv_sec - t1.tv_sec)*1000.0 + (t2.tv_usec - t1.tv_usec)/1000.0;
	    //src_dur += (t2.tv_sec/1000 + t2.tv_usec*1000) - (t1.tv_sec/1000 + t1.tv_usec*1000) ;
#endif
           break;
    }

    if(parseActive || mod->isShared())
      switch(stabptr[i].type){
      case N_SOL:
#ifdef TIMED_PARSE
	sol_count++;
	gettimeofday(&t1, NULL);
#endif
	if(absoluteDirectory){
	  const char* newSuffix = &stabstrs[stabptr[i].name];
	  if(newSuffix[0] == '/'){
	    delete currentSourceFile;
	    currentSourceFile = new string;
	  }
	  else{
	    char* tmp = new char[absoluteDirectory->length()+1];
	    strcpy(tmp,absoluteDirectory->c_str());
	    char* p=strrchr(tmp,'/');
	    if(p) 
	      *(++p)='\0';
	    delete currentSourceFile;
	    currentSourceFile = new string(tmp);
	    delete[] tmp;
	  }
	  (*currentSourceFile) += newSuffix;
	  currentSourceFile = processDirectories(currentSourceFile);
	  if(currentFunctionName)
	    lineInformation->insertSourceFileName(
						  *currentFunctionName,
						  *currentSourceFile,
						  &currentFileInfo,&currentFuncInfo);
	}
	else{
	  currentSourceFile = new string(&stabstrs[stabptr[i].name]);
	  currentSourceFile = processDirectories(currentSourceFile);
	  if(currentFunctionName)
	    lineInformation->insertSourceFileName(
						  *currentFunctionName,
						  *currentSourceFile,
						  &currentFileInfo,&currentFuncInfo);
            }
#ifdef TIMED_PARSE
	gettimeofday(&t2, NULL);
	sol_dur += (t2.tv_sec - t1.tv_sec)*1000.0 + (t2.tv_usec - t1.tv_usec)/1000.0;
	//sol_dur += (t2.tv_sec/1000 + t2.tv_usec*1000) - (t1.tv_sec/1000 + t1.tv_usec*1000); 
#endif
	break;
      case N_SLINE:
#ifdef TIMED_PARSE
	sline_count++;
	gettimeofday(&t1, NULL);
#endif
	mostRecentLinenum = stabptr[i].desc;
	//if the stab information is a line information
	//then insert an entry to the line info object
	if(!currentFunctionName) break;
	if(currentFileInfo)
	  currentFileInfo->insertLineAddress(currentFuncInfo,
					     stabptr[i].desc,
					     stabptr[i].val+currentFunctionBase);
#ifdef TIMED_PARSE
	gettimeofday(&t2, NULL);
	sline_dur += (t2.tv_sec - t1.tv_sec)*1000.0 + (t2.tv_usec - t1.tv_usec)/1000.0;
	//sline_dur += (t2.tv_sec/1000 + t2.tv_usec*1000) - (t1.tv_sec/1000 + t1.tv_usec*1000) ;
#endif
	break;
      case N_FUN:
#ifdef TIMED_PARSE
	fun_count++;
	gettimeofday(&t1, NULL);
#endif
	//if it is a function stab then we have to insert an entry 
	//to initialize the entries in the line information object
	int currentEntry = i;
	ptr = new char[1024];
	strcpy(ptr,(const char *)&stabstrs[stabptr[currentEntry].name]);
	while(ptr[strlen(ptr)-1] == '\\'){
	  ptr[strlen(ptr)-1] = '\0';
	  currentEntry++;
	  strcat(ptr,(const char *)&stabstrs[stabptr[currentEntry].name]);
	}
	
	char* colonPtr = NULL;
	if(currentFunctionName) delete currentFunctionName;
	if(!ptr || !(colonPtr = strchr(ptr,':')))
	  currentFunctionName = NULL;
	else {
	  char* tmp = new char[colonPtr-ptr+1];
	  strncpy(tmp,ptr,colonPtr-ptr);
	  tmp[colonPtr-ptr] = '\0';
	  currentFunctionName = new string(tmp);
	  
	  currentFunctionBase = 0;
	  Symbol info;
	  if (!proc->getSymbolInfo(*currentFunctionName,
				   info,currentFunctionBase))
	    {
	      string fortranName = *currentFunctionName + string("_");
	      if (proc->getSymbolInfo(fortranName,info,
				      currentFunctionBase))
		{
		  delete currentFunctionName;
		  currentFunctionName = new string(fortranName);
		}
	    }

	  currentFunctionBase += info.addr();
	  
	  delete[] tmp;		
	  if(currentSourceFile)
	    lineInformation->insertSourceFileName(
						  *currentFunctionName,
						  *currentSourceFile,
						  &currentFileInfo,&currentFuncInfo);
	}
	delete[] ptr;
#ifdef TIMED_PARSE
	gettimeofday(&t2, NULL);
	fun_dur += (t2.tv_sec - t1.tv_sec)*1000.0 + (t2.tv_usec - t1.tv_usec)/1000.0;
	//fun_dur += (t2.tv_sec/1000 + t2.tv_usec*1000) - (t1.tv_sec/1000 + t1.tv_usec*1000);
#endif
	break;
      }
    
    if (!parseActive) continue;
    
    BPatch_Vector<BPatch_function *> bpfv;
    switch(stabptr[i].type){
    case N_BCOMM:	{
	    // begin Fortran named common block 
      commonBlockName = (char *) &stabstrs[stabptr[i].name];
      
      // find the variable for the common block
      BPatch_image *progam = (BPatch_image *) getObjParent();
#ifdef TIMED_PARSE
      cout << __FILE__ << __LINE__ << ": parseTypes N_BCOMM "<< commonBlockName << endl;
#endif
      commonBlockVar = progam->findVariable(commonBlockName);
      if (!commonBlockVar) {
	printf("unable to find variable %s\n", commonBlockName);
      } else {
	commonBlock = const_cast<BPatch_type *> (commonBlockVar->getType());
	if (commonBlock->getDataClass() != BPatch_dataCommon) {
	  // its still the null type, create a new one for it
	  commonBlock = new BPatch_type(commonBlockName, false);
	  commonBlockVar->setType(commonBlock);
	  moduleTypes->addGlobalVariable(commonBlockName, commonBlock);
	  
	  commonBlock->setDataClass(BPatch_dataCommon);
	}
	// reset field list
	commonBlock->beginCommonBlock();
      }
      break;
    }
    
    case N_ECOMM: {
      // copy this set of fields
      
      assert(currentFunctionName);
#ifdef TIMED_PARSE
      cout << __FILE__ << __LINE__ << ": parseTypes N_ECOMM "<< currentFunctionName << endl;
#endif
      if (NULL == findFunction(currentFunctionName->c_str(), &bpfv) || !bpfv.size()) {
	printf("unable to locate current function %s\n", currentFunctionName->c_str());
      } else {
	if (bpfv.size() > 1) {
	  // warn if we find more than one function with this name
	  printf("%s[%d]:  WARNING: found %d funcs matching name %s, using the first\n",
		 __FILE__, __LINE__, bpfv.size(), currentFunctionName->c_str());
	}


	
	BPatch_function *func = bpfv[0];
	commonBlock->endCommonBlock(func, commonBlockVar->getBaseAddr());
      }
      
      // update size if needed
      if (commonBlockVar)
	commonBlockVar->setSize(commonBlock->getSize());
      commonBlockVar = NULL;
      commonBlock = NULL;
      break;
    }
    
    // case C_BINCL: -- what is the elf version of this jkh 8/21/01
    // case C_EINCL: -- what is the elf version of this jkh 8/21/01
    case 32:    // Global symbols -- N_GYSM 
    case N_FUN:
    case 128:   // typedefs and variables -- N_LSYM
    case 160:   // parameter variable -- N_PSYM 
#ifdef TIMED_PARSE
      pss_count++;
      gettimeofday(&t1, NULL);
#endif

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
      if (nativeCompiler)
	temp = parseStabString(this, mostRecentLinenum, (char *)ptr, stabptr[i].val, commonBlock);
      else
	temp = parseStabString(this, stabptr[i].desc, (char *)ptr, stabptr[i].val, commonBlock);
      if (*temp) {
	//Error parsing the stabstr, return should be \0
	fprintf(stderr, "Stab string parsing ERROR!! More to parse: %s\n",
		temp);
      }

      
#ifdef TIMED_PARSE
      gettimeofday(&t2, NULL);
      pss_dur += (t2.tv_sec - t1.tv_sec)*1000.0 + (t2.tv_usec - t1.tv_usec)/1000.0;
      //      pss_dur += (t2.tv_sec/1000 + t2.tv_usec*1000) - (t1.tv_sec/1000 + t1.tv_usec*1000);
#endif
      break;
    default:
      break;
    }       		    
  }
#if defined(TIMED_PARSE)
  struct timeval endtime;
  gettimeofday(&endtime, NULL);
  unsigned long lstarttime = starttime.tv_sec * 1000 * 1000 + starttime.tv_usec;
  unsigned long lendtime = endtime.tv_sec * 1000 * 1000 + endtime.tv_usec;
  unsigned long difftime = lendtime - lstarttime;
  double dursecs = difftime/(1000 );
  cout << __FILE__ << ":" << __LINE__ <<": parseTypes("<< mod->fileName()
       <<") took "<<dursecs <<" msecs" << endl;
  cout << "Breakdown:" << endl;
  cout << "     Functions: " << fun_count << " took " << fun_dur << "msec" << endl;
  cout << "     Sources: " << src_count << " took " << src_dur << "msec" << endl;
  cout << "     SOL?s: " << sol_count << " took " << sol_dur << "msec" << endl;
  cout << "     Sliness: " << sline_count << " took " << sline_dur << "msec" << endl;
  cout << "     parseStabString: " << pss_count << " took " << pss_dur << "msec" << endl;
  cout << "     Total: " << pss_dur +sline_dur + sol_dur + fun_dur + src_dur 
       << " msec" << endl;
  lineInformation->print();
  cout << "Max addr per line = " << max_addr_per_line << ".  Max line per addr = "
       << max_line_per_addr << endl;
#endif

  
  }

#endif // PARSE_ALL_AT_ONCE
#endif //end of #if defined(i386_unknown_linux2_0)

// Parsing symbol table for Alpha platform
// Mehmet

#if defined(alpha_dec_osf4_0)
extern void parseCoff(BPatch_module *mod, char *exeName, 
			const string& modName, LineInformation* linfo);

void BPatch_module::parseTypes()
{
  image * imgPtr=NULL;

  //Using pdmodule to get the image Object.
  imgPtr = mod->exec();

  //Get the path name of the process
  char *file = (char *)(imgPtr->file()).c_str();

  // with BPatch_functions
  this->BPfuncs = this->getProcedures();

  assert(lineInformation);

  parseCoff(this, file, mod->fileName(),lineInformation);
}

#endif



#if defined(i386_unknown_nt4_0) || defined(mips_unknown_ce2_11) //ccw 6 apr 2001

// Parsing symbol table for NT platform
// Mehmet 7/24/00

#include "CodeView.h"

void BPatch_module::parseTypes()
{
  
  image * imgPtr=NULL;

  //Using pdmodule to get the image Object.
  imgPtr = mod->exec();

  // with BPatch_functions
  this->BPfuncs = this->getProcedures();

  // trying to get at codeview symbols
  // need text section id and addr of codeview symbols
  unsigned int textSectionId = imgPtr->getObject().GetTextSectionId();
  PIMAGE_NT_HEADERS peHdr = imgPtr->getObject().GetImageHeader();

  DWORD modBaseAddr = imgPtr->getObject().code_off();
  PVOID mapAddr = imgPtr->getObject().GetMapAddr();

  PVOID pCodeViewSymbols = NULL;
  ULONG dirEntSize = 0;
  PIMAGE_SECTION_HEADER pDebugSectHeader = NULL;
  PIMAGE_DEBUG_DIRECTORY baseDirEnt = 
	  (PIMAGE_DEBUG_DIRECTORY)ImageDirectoryEntryToDataEx( mapAddr,
											FALSE,   // we mapped using MapViewOfFile
											IMAGE_DIRECTORY_ENTRY_DEBUG,
											&dirEntSize,
											&pDebugSectHeader );
  if( baseDirEnt != NULL )
  {
	PIMAGE_DEBUG_DIRECTORY pDirEnt = baseDirEnt;
	unsigned int nDirEnts = dirEntSize / sizeof(IMAGE_DEBUG_DIRECTORY);
	for( unsigned int i = 0; i < nDirEnts; i++ )
	{
		if( pDirEnt->Type == IMAGE_DEBUG_TYPE_CODEVIEW )
		{
			pCodeViewSymbols = (PVOID)(((char*)mapAddr) + pDirEnt->PointerToRawData);
			break;
		}
		// advance to next entry
		pDirEnt++;
	}
  }

  if( pCodeViewSymbols != NULL )
  {
	CodeView* cv = new CodeView( (const char*)pCodeViewSymbols, 
					textSectionId );

	cv->CreateTypeAndLineInfo(this, modBaseAddr, lineInformation);
  }
}
#endif


bool BPatch_module::getVariables(BPatch_Vector<BPatch_variableExpr *> &vars)
{
 
  BPatch_variableExpr *var;
  pdvector<string> keys = moduleTypes->globalVarsByName.keys();
  int limit = keys.size();
  for (int j = 0; j < limit; j++) {
    string name = keys[j];
    var = img->createVarExprByName(this, name.c_str());
    vars.push_back(var);
  }
  if (limit) 
    return true;
  
  return false;
}

/** method that finds the corresponding addresses for a source line
  * this methid returns true in sucess, otherwise false.
  * it can be called to find the exact match or, in case, exact match
  * does not occur, it will return the address set of the next
  * greater line. It uses the name of the module as a source file name
  * 
  */
bool BPatch_module::getLineToAddr(unsigned short lineNo,
				  BPatch_Vector<unsigned long>& buffer,
		                  bool exactMatch)

{

	//if the line information is not created yet return false

	if(!lineInformation){
#ifndef PARSE_ALL_AT_ONCE
#if !defined(rs6000_ibm_aix4_1)
	  parseFileLineInfo();
#else
	  return false;
#endif
#else
	  return false;
#endif
	}
	
	//query the line info object to get set of addresses if it exists.
	BPatch_Set<Address> addresses;
	if(!lineInformation->getAddrFromLine(addresses,lineNo,exactMatch)) {
	  cerr << __FILE__ << __LINE__ << ":  getAddrFromLine failed!" << endl;
		return false;
	}
	//then insert the elements to the vector given
	Address* elements = new Address[addresses.size()];
	addresses.elements(elements);
	for(int i=0;i<addresses.size();i++)
		buffer.push_back(elements[i]);
	delete[] elements;
	return true;
}



LineInformation* BPatch_module::getLineInformation(){
  
#ifndef PARSE_ALL_AT_ONCE
#if !defined(rs6000_ibm_aix4_1)
  if (!lineInformation) parseFileLineInfo();
#endif
  return lineInformation;	
#else
  return lineInformation;
#endif

}

bool BPatch_module::isSharedLib() const {
  return mod->isShared();
}

#ifdef IBM_BPATCH_COMPAT

bool BPatch_module::getLineNumbers(unsigned int &start, unsigned int &end)
{
    start = 0;
    end = 0;
    return true;
}

char *BPatch_module::getUniqueString(char *buffer, int length)
{
    getName(buffer, length);
    return buffer;
}

int BPatch_module::getSharedLibType()	
{
    return 0;
}

int BPatch_module::getBindingType()
{
    return 0;
}

#endif

