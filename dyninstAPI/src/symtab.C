/*
 *  Copyright 1993 Jeff Hollingsworth.  All rights reserved.
 *
 */

#ifndef lint
static char Copyright[] = "@(#) Copyright (c) 1993 Jeff Hollingsowrth\
    All rights reserved.";

static char rcsid[] = "@(#) $Header: /home/jaw/CVSROOT_20081103/CVSROOT/core/dyninstAPI/src/symtab.C,v 1.13 1994/09/30 19:47:16 rbi Exp $";
#endif

/*
 * symtab.C - generic symbol routines.  Implements an ADT for a symbol
 *   table.  The make calls to machine and a.out specific routines to handle
 *   the implementation dependent parts.
 *
 * $Log: symtab.C,v $
 * Revision 1.13  1994/09/30 19:47:16  rbi
 * Basic instrumentation for CMFortran
 *
 * Revision 1.12  1994/09/22  02:26:40  markc
 * Made structs classes
 *
 * Revision 1.11  1994/08/08  20:13:47  hollings
 * Added suppress instrumentation command.
 *
 * Revision 1.10  1994/08/02  18:25:07  hollings
 * fixed modules to use list template for lists of functions.
 *
 * Revision 1.9  1994/07/28  22:40:48  krisna
 * changed definitions/declarations of xalloc functions to conform to alloc.
 *
 * Revision 1.8  1994/07/22  19:21:10  hollings
 * removed mistaken divid by 1Meg for predicted cost.
 *
 * Revision 1.7  1994/07/20  23:23:41  hollings
 * added insn generated metric.
 *
 * Revision 1.6  1994/07/12  19:26:15  jcargill
 * Added internal prefix for TRACELIB
 *
 * Revision 1.5  1994/06/29  02:52:51  hollings
 * Added metricDefs-common.{C,h}
 * Added module level performance data
 * cleanedup types of inferrior addresses instrumentation defintions
 * added firewalls for large branch displacements due to text+data over 2meg.
 * assorted bug fixes.
 *
 * Revision 1.4  1994/06/27  21:28:23  rbi
 * Abstraction-specific resources and mapping info
 *
 * Revision 1.3  1994/06/27  18:57:15  hollings
 * removed printfs.  Now use logLine so it works in the remote case.
 * added internalMetric class.
 * added extra paramter to metric info for aggregation.
 *
 * Revision 1.2  1994/05/16  22:31:55  hollings
 * added way to request unique resource name.
 *
 * Revision 1.1  1994/01/27  20:31:45  hollings
 * Iinital version of paradynd speaking dynRPC igend protocol.
 *
 * Revision 1.8  1993/12/13  19:57:20  hollings
 * added nearest line match for function to support Fortran convention of making
 * first sline record be the first executable statement in the function.
 *
 * Revision 1.7  1993/10/12  20:10:35  hollings
 * zero memory on a realloc.
 *
 * Revision 1.6  1993/10/04  21:39:08  hollings
 * made missing internal symbols a fatal condition.
 *
 * Revision 1.5  1993/08/23  23:16:05  hollings
 * added third parameter to findInternalSymbol.
 *
 * Revision 1.4  1993/08/20  22:02:39  hollings
 * removed unused local variables.
 *
 * Revision 1.3  1993/07/13  18:32:46  hollings
 * new include file syntax.
 * added name demangler.
 *
 * Revision 1.2  1993/06/08  20:14:34  hollings
 * state prior to bc net ptrace replacement.
 *
 * Revision 1.1  1993/03/19  22:45:45  hollings
 * Initial revision
 *
 *
 */

extern "C" {
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <memory.h>
}

#include "util/h/list.h"
#include "symtab.h"
#include "util.h"
#include "dyninstP.h"

resource *moduleRoot;
extern "C" char *cplus_demangle(char *, int);
static image *allImages;
extern stringPool pool;

extern image *loadSymTable(const char *file, int offset, List<libraryFunc*> libraryFunctions, 
			   const char **iSym);

/* imported from platform specific library list.  This is lists all
   library functions we are interested in instrumenting. */
extern List<libraryFunc*> libraryFunctions;

/* store text address of source code line numbers */
void lineTable::setLineAddr(int line, caddr_t lineAddr)
{
  if (line < 0) {
    sprintf(errorLine, "symbol table entry: N_SLINE, line#=%d, addr=%x\n",
	    line, (unsigned)lineAddr);
    logLine(errorLine);
    return;
  }
  if (line >= maxLine) {
    int increment;
    int moreLines = line - maxLine + 100;
    addr = (caddr_t *) xrealloc(addr, sizeof(caddr_t)*(moreLines + maxLine));
    increment = moreLines;
    memset((char*)&addr[maxLine],'\0',sizeof(caddr_t)*increment);
    maxLine = moreLines;
  }
  addr[line] = lineAddr;
}

module *newModule(image *curr, const char *currentDirectory, const char *name, caddr_t addr)
{
    module *ret;
    char fileName[255];

    // modules can be defined several times in C++ due to templates and
    //   in-line member functions.
    ret = findModule(curr, name);
    if (ret) {
	return(ret);
    }

    ret = new module;
    sprintf(fileName, "%s%s", currentDirectory, name);
    ret->compileInfo = NULL;
    ret->fullName = pool.findAndAdd(fileName);
    ret->fileName = pool.findAndAdd(name);
    ret->language = unknown;
    ret->addr = addr;
    ret->exec = curr;
    ret->next = curr->modules;

    curr->modules = ret;
    curr->moduleCount++;
    return(ret);
}

module *moduleFindOrAdd(image *exec, caddr_t addr, char *name)
{
    module *curr;

    for (curr=exec->modules; curr; curr = curr->next) {
	if (curr->addr == addr) {
	    return(curr);
	}
    }
    sprintf(errorLine, "warning no symbol table for module %s\n", name);
    logLine(errorLine);
    curr = new module;
    curr->fileName = pool.findAndAdd(name);
    curr->fullName = NULL;
    curr->language = unknown;
    curr->addr = addr;
    curr->next = exec->modules;
    exec->modules = curr;
    return(curr);
}

stringHandle buildDemangledName(pdFunction *func)
{
    char *tempName;
    stringHandle prettyName;

    tempName = cplus_demangle((char*)func->symTabName, 0);
    if (tempName) {
	prettyName = pool.findAndAdd(tempName);
	free(tempName);
    } else {
	prettyName = func->symTabName;
    }
    return(prettyName);
}

pdFunction *funcFindOrAdd(image *exec, module *mod, caddr_t addr, char *name)
{
    pdFunction *func;

    for (func = exec->funcs; func; func=func->next) {
	if (func->addr == addr) {
	    return(func);
	}
    }

    func = new pdFunction;
    func->symTabName = pool.findAndAdd(name);
    func->prettyName = buildDemangledName(func);
    func->line = UNKNOWN_LINE;	
    func->file = mod;
    func->addr = addr;
    func->next = exec->funcs;

    mod->funcs.add(func);

    exec->funcs = func;
    exec->funcCount++;
    return(func);
}

pdFunction *newFunc(image *exec, module *mod, const char *name, int addr)
{
    char *p;
    pdFunction *func;

    if (!mod) {
	logLine("Error function without module\n");
    }
    func = new pdFunction;

    if (exec->funcAddrHash.find((void *) addr)) {
	sprintf(errorLine, "function defined twice\n");
	logLine(errorLine);
	abort();
    }

    exec->funcAddrHash.add(func, (void *) addr);

    p = strchr(name, ':');
    if (p) *p = '\0';
    func->symTabName = pool.findAndAdd(name);
    func->prettyName = buildDemangledName(func);
    func->line = UNKNOWN_LINE;	/* ???? fix this */
    func->file = mod;
    func->addr = (caddr_t) addr;
    func->sibling = findFunction(exec, name);

    mod->funcs.add(func);

    func->next = exec->funcs;
    exec->funcs = func;
    exec->funcCount++;
    return(func);
}

/*
 * List of prefixes for internal symbols we need to find.
 *
 */
const char *internalPrefix[] = {
    "DYNINST",
    "TRACELIB",
    NULL
};

/*
 * load an executable:
 *   1.) parse symbol table and identify rotuines.
 *   2.) scan executable to identify inst points.
 *
 *  offset is normally zero except on CM-5 where we have two images in one
 *    file.  The offset passed to parseImage is the logical offset (0/1), not
 *    the physical point in the file.  This makes it faster to "parse" multiple
 *    copies of the same image since we don't have to stat and read to find the
 *    physical offset. 
 */
image *parseImage(char *file, int offset)
{
    image *ret;
    module *mod;
    Boolean status;
    pdFunction *func;
    caddr_t endUserAddr;
    List<pdFunction*> curr;
    resource *modResource;
    caddr_t startUserAddr;
    internalSym *endUserFunc;
    internalSym *startUserFunc;
    extern findNodeOffset(char *, int);

    /*
     * Check to see if we have parsed this image at this offeset before.
     *
     */
    for (ret=allImages; ret; ret = ret->next) {
	if (!strcmp((char*)ret->file, file) && (ret->offset == offset)) {
	    return(ret);
	}
    }

    /*
     * load the symbol table. (This is the a.out format specific routine).
     *
     */
    ret = loadSymTable(file, findNodeOffset(file, offset), libraryFunctions, 
	internalPrefix);

    /* check that we loaded o.k. */
    if (!ret) {
	return(NULL);
    }

    /*
     * Add to master image list.
     *
     */
    ret->next = allImages;
    ret->offset = offset;
    allImages = ret;

    /*
     * mark as lib, library pdFunctions that were compiled with symbols.
     *
     */
    startUserFunc = findInternalSymbol(ret, "DYNINSTstartUserCode", False);
    startUserAddr = (caddr_t) ((startUserFunc) ? startUserFunc->addr : 0x0);

    endUserFunc = findInternalSymbol(ret, "DYNINSTendUserCode", False);
    if (endUserFunc && endUserFunc->addr) {
	endUserAddr = (caddr_t) endUserFunc->addr;
    } else {
	endUserAddr = (caddr_t) startUserAddr;
    }

    // if (endUserFunc) {
    if (1) {
	for (func = ret->funcs; func; func=func->next) {
	    if ((func->addr >= endUserAddr) ||
	        (func->addr <= startUserAddr)) {
		func->tag |= TAG_LIB_FUNC;
//		printf ("function %s tagged as a library function\n",
//			func->prettyName);
	    }
//	    else
//		printf ("function %s NOT tagged as a library function\n",
//			func->prettyName);
	}
    }
    

    /*
     * Now identify the points in the functions to instrument.
     *   This is the machine specific routine.
     *
     */
    status = locateAllInstPoints(ret);
    if (status == FALSE) {
	return(NULL);
    }

    moduleRoot = newResource(rootResource, NULL, NULL, "Procedure", 0.0, FALSE);

    // define all modules.
    for (mod = ret->modules; mod; mod=mod->next) {
	modResource = NULL;
	for (curr = mod->funcs;  func = *curr; curr++) {
	    if ((!func->tag & TAG_LIB_FUNC) && (func->line)) {

		// see if we have created module yet.
		if (!modResource) {
		    modResource = newResource(moduleRoot, mod, NULL, 
			(const char*) mod->fileName, 0.0, FALSE);
		}
		(void) newResource(modResource, func, NULL, 
		    (const char*) func->prettyName,0.0,FALSE);
	    } else {
		func->tag |= TAG_LIB_FUNC;
	    }
	}
    }

    /*
     * Optional post-processing step.  
     */
    if (ret->symbolExists("CMRT_init")) {
      ret->postProcess(NULL);
    }

    /* this may not be heap allocated memory, comment out for now */
    /* will look at this later */
    /* zero out ret->code so it can't be followed - mdc */
    /* free(ret->code); */
    ret->code = 0;

    return(ret);
}


internalSym *findInternalSymbol(image *i, const char *name, Boolean warn)
{
    int count;
    stringHandle iName;

    iName = pool.findAndAdd(name);
    for (count = 0; count < i->iSymCount; count++) {
	if (i->iSyms[count].name == iName) {
	    return(&i->iSyms[count]);
	}
    }
    if (warn) {
	sprintf(errorLine, "unable to find internal symbol %s\n", name);
	logLine(errorLine);
    }
    
    return(NULL);
}

caddr_t findInternalAddress(image *i, const char *name, Boolean warn)
{
    int count;
    stringHandle iName;
    internalSym *curr;

    iName = pool.findAndAdd(name);
    for (count = 0, curr=i->iSyms; count < i->iSymCount; count++, curr++) {
	if (curr->name == iName) {
	    return((caddr_t) curr->addr);
	}
    }
    if (warn) {
	printf("unable to find internal symbol %s\n", name);
	abort();
    }
    return(NULL);
}

module *findModule(image *i, const char *name)
{
    stringHandle iName;
    module *mod;

    iName = pool.findAndAdd(name);
    for (mod = i->modules; mod; mod=mod->next) {
	if (iName == mod->fileName) {
	     return(mod);
	} else if (iName == mod->fullName) {
	     return(mod);
	}
    }
    return(NULL);
}

//
// Warning the sibling code depends on findFunction working down the
//    function list in order!!  
//
pdFunction *findFunction(image *i, const char *name)
{
    stringHandle iName;
    pdFunction *func;

    iName = pool.findAndAdd(name);
    for (func = i->funcs; func; func=func->next) {
	if (iName == func->prettyName) {
	     return(func);
	} else if (iName == func->symTabName) {
	     return(func);
	}
    }
    return(NULL);
}

pdFunction *findFunctionByAddr(image *i, caddr_t addr)
{
    pdFunction *func;

    func = i->funcAddrHash.find((void *) addr);
#ifdef notdef
    for (func = i->funcs; func; func=func->next) {
	if (addr == func->addr) {
	     return(func);
	}
    }
    return(NULL);
#endif
    return(func);
}


void mapLines(module *mod)
{
    int j;
    pdFunction *func;
    List<pdFunction*> curr;

    if (!mod) return;

    mod->lines.qsortLines();
    for (curr = mod->funcs; func = *curr; curr++) {
	for (j=0; j < mod->lines.getMaxLine(); j++) {
	    if (func->addr <= (caddr_t) mod->lines.getLineAddr(j)) {
		func->line = j;
		break;
	    }
	}
    }
}

void changeLibFlag(resource *res, Boolean setSuppress)
{
    image *ret;
    module *mod;
    pdFunction *func;
    List<pdFunction*> curr;

    for (ret=allImages; ret; ret = ret->next) {
	mod = findModule(ret, res->getName());
	if (mod) {
	    // suppress all procedures.
	    for (curr = mod->funcs; func = *curr; curr++) {
		if (setSuppress) {
		    func->tag |= TAG_LIB_FUNC;
		} else {
		    func->tag &= ~TAG_LIB_FUNC;
		}
	    }
	} else {
	    func = findFunction(ret, res->getName());
	    if (func) {
		if (setSuppress) {
		    func->tag |= TAG_LIB_FUNC;
		} else {
		    func->tag &= ~TAG_LIB_FUNC;
		}
	    }
	}
    }
}

image::image()
{
  file = NULL;
  name = NULL;
  moduleCount = 0;
  modules = NULL;
  funcCount = 0;
  funcs = NULL;
  iSymCount=0;
  iSyms = NULL;
  code = NULL;
  textOffset = 0;
  offset  = 0;
  next = 0;
}

/* 
 * return 0 if symbol <symname> exists in image, non-zero if it does not
 */
int image::symbolExists(const char *symname)
{
  pdFunction *dummy;
  
  dummy = findFunction(this, symname);
  return (dummy != NULL);
}

void image::postProcess(const char *pifname)
{
  FILE *Fil;
  stringHandle fname;
  char temp[5000], errorstr[5000], key[5000];
  char tmp1[5000], abstraction[500];
  resource *parent;

  /* What file to open? */
  if (pifname) {
    fname = pool.findAndAdd(pifname);
  } else {
    sprintf(temp, "%s.pif", (char *) file);
    fname = pool.findAndAdd(temp);
  }

  /* Open the file */
  Fil = fopen((char *) fname, "r");
  if (Fil == NULL) {
    sprintf(errorstr, 
	    "Tried to open PIF file %s, but could not (continuing)\n", 
	    (char *) fname);
    logLine(errorstr);
    return;
  }

  /* Process the file */
  while (!feof(Fil)) {
    fscanf(Fil, "%s", key);
    switch (key[0]) {
    case 'M':
      /* Ignore mapping information for right now */
      fgets(tmp1, 5000, Fil);
      break;
    case 'R':
      /* Create a new resource */
      fscanf(Fil, "%s {", abstraction);
      parent = rootResource;
      do {
	fscanf(Fil, "%s", tmp1);
        if (tmp1[0] != '}') {
          parent = newResource(parent, NULL, abstraction, tmp1, 0.0, FALSE);
        } else {
	  parent = NULL;
	}
      } while (parent != NULL);
      break;
    default:
      sprintf(errorstr, 
	      "Ignoring bad line key (%s) in file %s\n", key, (char *) fname);
      logLine(errorstr);
      fgets(tmp1, 5000, Fil);
      break;
    }
  }
  return;
}

module::module()
{
  compileInfo = NULL;
  fileName = NULL;
  fullName = NULL;
  language = unknown;
  addr = 0;
  exec = 0;
  next = 0;
}

pdFunction::pdFunction()
{
  symTabName = NULL;
  prettyName = NULL;
  line = 0;
  file = NULL;
  addr = 0;
  funcEntry = NULL;
  funcReturn = NULL;
  callLimit = 0;
  callCount = 0;
  calls = NULL;
  ljmpCount = 0;
  jmps = NULL;
  tag = 0;
  next = NULL;
  sibling = NULL;
}
