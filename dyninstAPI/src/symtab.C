/*
 *  Copyright 1993 Jeff Hollingsworth.  All rights reserved.
 *
 */

#ifndef lint
static char Copyright[] = "@(#) Copyright (c) 1993 Jeff Hollingsowrth\
    All rights reserved.";

static char rcsid[] = "@(#) $Header: /home/jaw/CVSROOT_20081103/CVSROOT/core/dyninstAPI/src/symtab.C,v 1.3 1994/06/27 18:57:15 hollings Exp $";
#endif

/*
 * symtab.C - generic symbol routines.  Implements an ADT for a symbol
 *   table.  The make calls to machine and a.out specific routines to handle
 *   the implementation dependent parts.
 *
 * $Log: symtab.C,v $
 * Revision 1.3  1994/06/27 18:57:15  hollings
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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "dyninst.h"
#include "symtab.h"
#include "util.h"

extern "C" char *cplus_demangle(char *, int);
static image *allImages;
stringPool pool;

/* imported from platform specific library list.  This is lists all
   library functions we are interested in instrumenting. */
extern libraryList libraryFunctions;

void processLine(module *mod, int line, int addr)
{
    int increment;

    if (line >= mod->lines.maxLine) {
	mod->lines.addr = (int *) 
	    xrealloc(mod->lines.addr, sizeof(int)*(line+100));
	increment = line+100 - mod->lines.maxLine;
	memset(&mod->lines.addr[mod->lines.maxLine],'\0',sizeof(int)*increment);
	mod->lines.maxLine = line+100;
    }
    mod->lines.addr[line] = addr;
}

module *newModule(image *curr, char *currentDirectory, char *name, int addr)
{
    module *ret;
    char fileName[255];

    ret = (module *) xcalloc(sizeof(image),1);
    sprintf(fileName, "%s%s", currentDirectory, name);
    ret->compileInfo = NULL;
    ret->fullName = pool.findAndAdd(fileName);
    ret->fileName = pool.findAndAdd(name);
    ret->language = unknown;
    ret->addr = addr;
    ret->funcCount = 0;
    ret->funcs = NULL;
    ret->exec = curr;
    ret->next = curr->modules;

    ret->lines.maxLine = 100;
    ret->lines.addr = (int *) xcalloc(sizeof(int), 100);

    curr->modules = ret;
    curr->moduleCount++;
    return(ret);
}

module *moduleFindOrAdd(image *exec, int addr, char *name)
{
    module *curr;

    for (curr=exec->modules; curr; curr = curr->next) {
	if (curr->addr == addr) {
	    return(curr);
	}
    }
    sprintf(errorLine, "warning no symbol table for module %s\n", name);
    logLine(errorLine);
    curr = (module *) xcalloc(sizeof(module), 1);
    curr->fileName = pool.findAndAdd(name);
    curr->fullName = NULL;
    curr->language = unknown;
    curr->addr = addr;
    curr->next = exec->modules;
    exec->modules = curr;
    return(curr);
}

char *buildDemangledName(function *func)
{
    char *tempName;
    char *prettyName;

    tempName = cplus_demangle(func->symTabName, 0);
    if (tempName) {
	prettyName = pool.findAndAdd(tempName);
	free(tempName);
    } else {
	prettyName = func->symTabName;
    }
    return(prettyName);
}

function *funcFindOrAdd(image *exec, module *mod, int addr, char *name)
{
    function *func;

    for (func = exec->funcs; func; func=func->next) {
	if (func->addr == addr) {
	    return(func);
	}
    }
    func = (function *) xcalloc(sizeof(function), 1);
    func->symTabName = pool.findAndAdd(name);
    func->prettyName = buildDemangledName(func);
    func->line = UNKNOWN_LINE;	
    func->file = mod;
    func->addr = addr;
    func->next = exec->funcs;

    mod->funcs = func;
    mod->funcCount++;

    exec->funcs = func;
    exec->funcCount++;
    return(func);
}

function *newFunc(image *exec, module *mod, char *name, int addr)
{
    char *p;
    function *func;

    if (!mod) {
	logLine("Error function without module\n");
    }
    func = (function *) xcalloc(sizeof(function),1);
    p = strchr(name, ':');
    if (p) *p = '\0';
    func->symTabName = pool.findAndAdd(name);
    func->prettyName = buildDemangledName(func);
    func->line = UNKNOWN_LINE;	/* ???? fix this */
    func->file = mod;
    func->addr = addr;
    func->sibling = findFunction(exec, name);

    mod->funcs = func;
    mod->funcCount++;

    func->next = exec->funcs;
    exec->funcs = func;
    exec->funcCount++;
    return(func);
}

/*
 * List of prefixes for internal symbols we need to find.
 *
 */
char *internalPrefix[] = {
    "DYNINST",
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
    function *func;
    int endUserAddr;
    int startUserAddr;
    resource procedureRoot;
    internalSym *endUserFunc;
    internalSym *startUserFunc;
    extern findNodeOffset(char *, int);

    /*
     * Check to see if we have parsed this image at this offeset before.
     *
     */
    for (ret=allImages; ret; ret = ret->next) {
	if (!strcmp(ret->file, file) && (ret->offset == offset)) {
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
     * mark as lib, library functions that were compiled with symbols.
     *
     */
    startUserFunc = findInternalSymbol(ret, "DYNINSTstartUserCode", False);
    startUserAddr = (startUserFunc) ? startUserFunc->addr : 0x0;

    endUserFunc = findInternalSymbol(ret, "DYNINSTendUserCode", False);
    endUserAddr = (endUserFunc) ? endUserFunc->addr : 0xffffffff;

    if (endUserFunc) {
	for (func = ret->funcs; func; func=func->next) {
	    if ((func->addr >= endUserAddr) ||
	        (func->addr <= startUserAddr)) {
		func->tag |= TAG_LIB_FUNC;
	    }
	}
    }
    

    /*
     * Now find identify the points in the functions to instrument.
     *   This is the machine specific routine.
     *
     */
    locateAllInstPoints(ret);

    /*
     * Define all of the functions in the code resource hierarchy.
     *
     */
    procedureRoot = newResource(rootResource, NULL, "Procedure", 0.0, FALSE);
    for (func = ret->funcs; func; func=func->next) {
	if ((!func->tag & TAG_LIB_FUNC) && (func->line)) {
	    (void) newResource(procedureRoot, func, func->prettyName,0.0,FALSE);
	} else {
	    func->tag |= TAG_LIB_FUNC;
	}
    }
    
    free(ret->code);
    return(ret);
}


internalSym *findInternalSymbol(image *i, char *name, Boolean warn)
{
    int count;
    char *iName;

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

int findInternalAddress(image *i, char *name, Boolean warn)
{
    int count;
    char *iName;
    internalSym *curr;

    iName = pool.findAndAdd(name);
    for (count = 0, curr=i->iSyms; count < i->iSymCount; count++, curr++) {
	if (curr->name == iName) {
	    return(curr->addr);
	}
    }
    if (warn) {
	printf("unable to find internal symbol %s\n", name);
	abort();
    }
    return(NULL);
}

function *findFunction(image *i, char *name)
{
    char *iName;
    function *func;

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

function *findFunctionByAddr(image *i, int addr)
{
    function *func;

    for (func = i->funcs; func; func=func->next) {
	if (addr == func->addr) {
	     return(func);
	}
    }
    return(NULL);
}

int intComp(int *i, int *j)
{
    return(*i - *i);
}

void mapLines(module *mod)
{
    int i, j;
    function *func;

    if (!mod) return;

    qsort(mod->lines.addr, mod->lines.maxLine, sizeof(int), intComp);
    for (i=0, func = mod->funcs; i < mod->funcCount; i++, func=func->next) {
	for (j=0; j < mod->lines.maxLine; j++) {
	    if (func->addr <= mod->lines.addr[j]) {
		func->line = j;
		break;
	    }
	}
    }
}
