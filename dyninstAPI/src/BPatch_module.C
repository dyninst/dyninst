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
#include "common/h/String.h"
#include "BPatch_type.h"    // For BPatch_type related stuff
#include "BPatch_Vector.h"
#include "LineInformation.h"

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
    proc(_proc), mod(_mod), img(_img), BPfuncs(NULL),lineInformation(NULL) 
{
    _srcType = BPatch_sourceModule;

    moduleTypes = new BPatch_typeCollection;

    // load all of the type information
#if !defined(mips_sgi_irix6_4)

    if (BPatch::bpatch->parseDebugInfo()){ 
	lineInformation = new LineInformation(mod->fileName());
	parseTypes();
    }
#endif
}

BPatch_module::~BPatch_module()
{
    delete moduleTypes;

    for (int f = 0; f < BPfuncs->size(); f++) {
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

#include <linenum.h>
#include <syms.h>

#define INC_FILE_SIZE 255

typedef struct {
	int count;
	unsigned begin[INC_FILE_SIZE];	
	unsigned end[INC_FILE_SIZE];
	string* name[INC_FILE_SIZE];
} IncludeFiles;

void parseLineInformation(process* proc,LineInformation* lineInformation,
			  IncludeFiles& includeFiles,string* currentSourceFile,
			  char* symbolName,
			  SYMENT *sym,
			  Address linesfdptr,char* lines,int nlines)
{
      int j;
      union auxent *aux;

      /* if it is beginning of include files then update the data structure 
         that keeps the beginning of the incoude files. If the include files contain 
         information about the functions and lines we have to keep it */
      if (sym->n_sclass == C_BINCL){
		includeFiles.begin[includeFiles.count] = (sym->n_value-linesfdptr)/LINESZ;
		includeFiles.name[includeFiles.count] = new string(symbolName);
      }
      /* similiarly if the include file contains function codes and line information
         we have to keep the last line information entry for this include file */
      else if (sym->n_sclass == C_EINCL){
		includeFiles.end[includeFiles.count] = (sym->n_value-linesfdptr)/LINESZ;
		includeFiles.count++;
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
				initialLineIndex = (aux->x_sym.x_fcnary.x_fcn.x_lnnoptr-linesfdptr)/LINESZ;
				funcStartAddress = extsym->n_value;
				break;
			}
		}

		/* access the line information now using the C_FCN entry*/
		SYMENT *bfsym = (SYMENT*)(((unsigned)sym)+SYMESZ);
		assert(bfsym->n_sclass == C_FCN);
		aux = (union auxent*)((char*)bfsym+SYMESZ);
		initialLine = aux->x_sym.x_misc.x_lnsz.x_lnno;

		string* whichFile = currentSourceFile;
		/* find in which file is it */
		for(j=0;j<includeFiles.count;j++)
			if((includeFiles.begin[j] <= (unsigned)initialLineIndex) &&
			   (includeFiles.end[j] >= (unsigned)initialLineIndex)){
				whichFile = includeFiles.name[j];
				break;
			}
		lineInformation->insertSourceFileName(currentFunctionName,
						     *whichFile);
		for(j=initialLineIndex+1;j<nlines;j++){
			LINENO* lptr = (LINENO*)(lines+j*LINESZ);
			if(!lptr->l_lnno)
				break;
	    		lineInformation->insertLineAddress(
				currentFunctionName,*whichFile,
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
    int nstabs;
    SYMENT *syms;
    char *modName;
    char name[256];
    char *stringPool;
    char *stabstr=NULL;
    union auxent *aux;
    image * imgPtr=NULL;
    int nlines;
    char* lines;
    unsigned long linesfdptr;

    string* currentSourceFile = NULL;
    IncludeFiles includeFiles;

    //Using pdmodule to get the image Object.
    imgPtr = mod->exec();
  
    //Using the image to get the Object (class)
    Object *objPtr = (Object *) &(imgPtr->getObject());

    //Using the Object to get the pointers to the .stab and .stabstr
    objPtr->get_stab_info(stabstr, nstabs, ((Address&) syms), stringPool); 

    this->BPfuncs = this->getProcedures();


    objPtr->get_line_info(nlines,lines,linesfdptr); 

    includeFiles.count = 0;

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

	 if(currentSourceFile) delete currentSourceFile;
	 currentSourceFile = new string(moduleName);

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

      if ((sym->n_sclass == C_BINCL) ||
	  (sym->n_sclass == C_EINCL) ||
	  (sym->n_sclass == C_FUN)) {
		char* fn = NULL;
		if(!sym->n_zeroes){
			char* cb = NULL;
			if (sym->n_sclass == C_FUN) 
				cb = !stabstr[sym->n_offset-3] ?
					&stabstr[sym->n_offset] :
					&stabstr[sym->n_offset-2];
			else
				cb = &stringPool[sym->n_offset];
			fn = new char[strlen(cb)+1];
			strcpy(fn,cb);
		}
		else{
			fn = new char[9];
			strncpy(fn,sym->n_name,8);
			fn[8] = '\0';
		}	
		parseLineInformation(proc,lineInformation,includeFiles,
				     currentSourceFile,fn,sym,
				     linesfdptr,lines,nlines);
		delete[] fn;
      }

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
    for(i=0;i<includeFiles.count;i++)
	delete includeFiles.name[i];
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

  //these variables are used to keep track of the source files
  //and function names being processes at a moment

  string* currentFunctionName = NULL;
  Address currentFunctionBase = 0;
  string* currentSourceFile = NULL;
  

  for(i=0;i<stab_nsyms;i++){
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
	    }
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

	    //time to create the source file name to be used
	    //for latter processing of line information
	    if(!currentSourceFile)
		currentSourceFile = new string(&stabstrs[stabptr[i].name]);
	    else
		*currentSourceFile += &stabstrs[stabptr[i].name];

            break;

    case N_SOL:
	    if(currentSourceFile){
   		char* tmp = new char[currentSourceFile->length()+1];
                strncpy(tmp,currentSourceFile->string_of(),
			currentSourceFile->length());
                tmp[currentSourceFile->length()] = '\0';
		if(strstr(tmp,&stabstrs[stabptr[i].name])){
			delete[] tmp;
			break;
		}
                char* p=strrchr(tmp,'/');
		if(p) 
                	*(++p)='\0';
                delete currentSourceFile;
                currentSourceFile = new string(tmp);
                (*currentSourceFile) += &stabstrs[stabptr[i].name];
                if(currentFunctionName)
			lineInformation->insertSourceFileName(
				*currentFunctionName,
				*currentSourceFile);
                delete[] tmp;
            }
            else{
                currentSourceFile = new string(&stabstrs[stabptr[i].name]);
                if(currentFunctionName)
			lineInformation->insertSourceFileName(
					*currentFunctionName,
  					*currentSourceFile);
            }
            break;
		
    case N_SLINE:
	    //if the stab information is a line information
	    //then insert an entry to the line info object
	    if(!currentFunctionName) break;
	    lineInformation->insertLineAddress(
			*currentFunctionName,*currentSourceFile,
			stabptr[i].desc,
			stabptr[i].val+currentFunctionBase);
	    break;

    case 32:    // Global symbols -- N_GYSM 
    case 36:    // functions and text segments -- N_FUN
    case 128:   // typedefs and variables -- N_LSYM
    case 160:   // parameter variable -- N_PSYM 
      char *ptr, *ptr2, *ptr3;
      int currentEntry;
      currentEntry = i;

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

      //if it is a function stab then we have to insert an entry 
      //to initialize the entries in the line information object
      if(stabptr[currentEntry].type == N_FUN){
	   char* colonPtr = NULL;
	   if(currentFunctionName) delete currentFunctionName;
	   if(!ptr || !(colonPtr = strchr(ptr,':')))
		currentFunctionName = NULL;
	   else{
		char* tmp = new char[colonPtr-ptr+1];
		strncpy(tmp,ptr,colonPtr-ptr);
		tmp[colonPtr-ptr] = '\0';
		currentFunctionName = new string(tmp);

		currentFunctionBase = 0;
		Symbol info;
		if (!proc->getSymbolInfo(*currentFunctionName,info,
				    currentFunctionBase)) {
		   string fortranName = *currentFunctionName + string("_");
		   if (proc->getSymbolInfo(fortranName,info,
						       currentFunctionBase)) {
		       delete currentFunctionName;
		       currentFunctionName = new string(fortranName);
		   }
		}

		currentFunctionBase += info.addr();

		delete[] tmp;		
		if(currentSourceFile)
			lineInformation->insertSourceFileName(
				*currentFunctionName,
				*currentSourceFile);
	   }
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
extern void parseCoff(BPatch_module *mod, char *exeName, 
			const string& modName, LineInformation* linfo);

void BPatch_module::parseTypes()
{
  image * imgPtr=NULL;

  //Using pdmodule to get the image Object.
  imgPtr = mod->exec();

  //Get the path name of the process
  char *file = (char *)(imgPtr->file()).string_of();

  // with BPatch_functions
  this->BPfuncs = this->getProcedures();

  parseCoff(this, file, mod->fileName(),lineInformation);
}

#endif



#if defined(i386_unknown_nt4_0)

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

  //The code below is adapted from Object-nt.C
  IMAGE_DEBUG_INFORMATION* pDebugInfo = NULL;

  // access the module's debug information
  pDebugInfo = MapDebugInformation(NULL, (LPTSTR)(imgPtr->file()).string_of(), NULL, 0);
  if( pDebugInfo == NULL )
  {
	printf("Unable to get debug information!\n");
	return;
  }

  // determine the location of the relevant sections
  unsigned int i, textSectionId;

  // currently we care only about the .text and .data segments
  for( i = 0; i < pDebugInfo->NumberOfSections; i++ )
  {
	IMAGE_SECTION_HEADER& section = pDebugInfo->Sections[i];

	if( strncmp( (const char*)section.Name, ".text", 5 ) == 0 ) {
		textSectionId = i + 1; // note that section numbers are one-based
		break;
	}
  }

  //
  // parse the symbols, if available
  // (note that we prefer CodeView over COFF)
  //
  if( pDebugInfo->CodeViewSymbols != NULL )
  {
	// we have CodeView debug information
	CodeView *cv = new CodeView( (const char*)pDebugInfo->CodeViewSymbols, 
					textSectionId );
	cv->CreateTypeInfo(this);
  }
  else if( pDebugInfo->CoffSymbols != NULL )
  {
	// we have COFF debug information
	// ParseCOFFSymbols( pDebugInfo );
  }
  else
  {
	// TODO - what to do when there's no debug information?
  }
}
#endif


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
#ifdef DEBUG_LINE
		cerr << "BPatch_module::getLineToAddr : ";
		cerr << "Line information is not available.\n";
#endif
		return false;
	}
	
	//query the line info object to get set of addresses if it exists.
	BPatch_Set<Address> addresses;
	if(!lineInformation->getAddrFromLine(addresses,lineNo,exactMatch))
		return false;

	//then insert the elements to the vector given
	Address* elements = new Address[addresses.size()];
	addresses.elements(elements);
	for(int i=0;i<addresses.size();i++)
		buffer.push_back(elements[i]);
	delete[] elements;
	return true;
}
