/*
 *  Copyright 1993 Jeff Hollingsworth.  All rights reserved.
 *
 */

#ifndef lint
static char Copyright[] = "@(#) Copyright (c) 1993 Jeff Hollingsowrth\
    All rights reserved.";

static char rcsid[] = "@(#) $Header: /home/jaw/CVSROOT_20081103/CVSROOT/core/paradynd/src/Attic/sym-bsd.C,v 1.10 1994/09/22 02:25:51 markc Exp $";
#endif

/*
 * sym-bsd.C - parse BSD style a.out files.
 *
 * $Log: sym-bsd.C,v $
 * Revision 1.10  1994/09/22 02:25:51  markc
 * changed frees to deletes
 * changed *allocs to news
 *
 * Revision 1.9  1994/09/15  19:18:49  rbi
 * Removed unneeded switch branch.
 *
 * Revision 1.8  1994/09/14  19:57:08  rbi
 * Fixed system calls in loadSymTable()
 *
 * Revision 1.7  1994/07/28  22:40:46  krisna
 * changed definitions/declarations of xalloc functions to conform to alloc.
 *
 * Revision 1.6  1994/07/22  19:21:09  hollings
 * removed mistaken divid by 1Meg for predicted cost.
 *
 * Revision 1.5  1994/07/12  19:38:34  jcargill
 * Fixed iSymCount problem, improved speed of search for lib functions,
 * removed old/dead CM5 code, and fixed pagemask error
 *
 * Revision 1.4  1994/07/05  03:26:19  hollings
 * observed cost model
 *
 * Revision 1.3  1994/06/29  02:52:49  hollings
 * Added metricDefs-common.{C,h}
 * Added module level performance data
 * cleanedup types of inferrior addresses instrumentation defintions
 * added firewalls for large branch displacements due to text+data over 2meg.
 * assorted bug fixes.
 *
 * Revision 1.2  1994/06/27  18:57:11  hollings
 * removed printfs.  Now use logLine so it works in the remote case.
 * added internalMetric class.
 * added extra paramter to metric info for aggregation.
 *
 * Revision 1.1  1994/01/27  20:31:43  hollings
 * Iinital version of paradynd speaking dynRPC igend protocol.
 *
 * Revision 1.9  1993/12/13  19:56:24  hollings
 * include initalized data segment with text segment in things that we
 * read from the binary image.  This was required by binist utility which
 * puts rtinst code into the data segment.
 *
 * Revision 1.8  1993/09/10  20:32:37  hollings
 * New, faster function to find library functions.
 *
 * Revision 1.7  1993/08/26  18:20:59  hollings
 * further refinements of which symbols get processed.
 *
 * Revision 1.6  1993/08/23  22:57:07  hollings
 * added check that internal symbols are only N_TEXT, N_DATA, or N_BSS.
 *
 * Revision 1.5  1993/08/11  01:42:30  hollings
 * added checks/fixes for ZMAGIC vs. OMAGIC (jkh for jcargill).
 *
 * Revision 1.4  1993/08/11  01:37:41  hollings
 * removed processing of dummy gnu symbols.
 *
 * Revision 1.3  1993/07/13  18:31:15  hollings
 *  new include file syntax.
 *  added support for N_SOL stab (included file).
 *  changed defauly endUserAddr to exec.a_entry to prevent extra symbols from
 *  getting into the dyninst.
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
#include <stdio.h>
#include <a.out.h>
#include <stab.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <errno.h>
#include <string.h>
}

extern "C" {
int getpagesize();
}

#include "util/h/list.h"
#include "symtab.h"
#include "util.h"

/* these three really are global to the symbol table module */
static int nsyms;
static char *strings;
static struct nlist *stabs;

void findInternalSymbols(image *ret, const char **iSym)
{
    int i;
    int len;
    char *str;
    int iCount;
    int index;

    ret->iSymCount=0;

    index=0;
    while (iSym[index]) {
      len = strlen(iSym[index]);
      for (i=0; i < nsyms; i++) {
	str = &strings[stabs[i].n_un.n_strx];
	if (!strncmp(str+1, iSym[index], len))  {
	  ret->iSymCount++;
	  // sprintf(errorLine, "Counting internal symbol:  '%s'\n", str);
	  // logLine(errorLine);
	}
      }
      index++;
    }

    ret->iSyms = new internalSym[ret->iSymCount];
    iCount=0;
    index=0;
    while (iSym[index]) {
      len = strlen(iSym[index]);
      for (i=0; i < nsyms; i++) {
	switch (stabs[i].n_type & 0xfe) {
	case N_TEXT:
	case N_DATA:
	case N_BSS:
	  str = &strings[stabs[i].n_un.n_strx];
	  if (!strncmp(str+1, iSym[index], len)) {
	    ret->iSyms[iCount].addr = stabs[i].n_value;
	    ret->iSyms[iCount].name = pool.findAndAdd(str+1);
	    // printf ("Found internal symbol:  '%s'\n", str);
	    iCount++;
	    break;
	  }
	default:
	  break;
	}
      }
      index++;
    }
    /* actual useful items found */
    ret->iSymCount = iCount;
}


/*
 * Check for user functions that were compiled without -g.
 *
 *   We only load routines that are between DYNINSTstartUserCode and
 *     DYNINSTendUserCode.  This prevents loading non-user code from
 *     program that were not compiled for dyninst.
 *
 */
void locateRogueFuncations(image *ret, struct exec *exec)
{
    int endUserAddr;
    int startUserAddr;
    internalSym *endUserFunc;
    internalSym *startUserFunc;
#ifdef notdef
    int i;
    char *str;
    module *currentModule;
    pdFunction *currentFunc;
#endif

    startUserFunc = findInternalSymbol(ret, "DYNINSTstartUserCode", False);
    startUserAddr = startUserFunc ? startUserFunc->addr: exec->a_entry;

    endUserFunc = findInternalSymbol(ret, "DYNINSTendUserCode", False);
    //endUserAddr = endUserFunc?endUserFunc->addr: exec->a_entry + exec->a_text;
    endUserAddr = endUserFunc ? endUserFunc->addr: exec->a_entry;

#ifdef notdef
    currentModule = newModule(ret, "", NO_SYMS_MODULE, 0);
    for (i=0; i < nsyms; i++) {
	str = &strings[stabs[i].n_un.n_strx];
	if (((stabs[i].n_type & 0xfe) == N_TEXT) &&
	     (stabs[i].n_value >= startUserAddr) && 
	     (stabs[i].n_value <= endUserAddr) && 
	     (*str == '_') && (strcmp(str, "___gnu_compiled_c"))) {
		/* a user function */
		currentFunc = findFunction(ret, str+1);
		if (!currentFunc || (currentFunc->addr != stabs[i].n_value)) {
		    /* that is not in our symbol table */
		    (void) newFunc(ret, currentModule, str+1, stabs[i].n_value);
		}
	}
    }
#endif
}

void locateLibFunctions(image *ret, List<libraryFunc*> libFuncs)
{
    int i, j;
    char *str;
    int scount;
    libraryFunc *lSym;
    module *currentModule;
    pdFunction *currentFunc;
    libraryFunc **tempSyms;


    tempSyms = new libraryFunc*[libFuncs.count()];
    for (scount=0; lSym = *libFuncs; libFuncs++) {
	currentFunc = findFunction(ret, (char*)lSym->name);
	if (currentFunc) {
//	    printf ("locateLibFunctions: found %s\n", lSym->name);
	    currentFunc->tag = lSym->tags;
	    locateInstPoints(currentFunc, ret->code, ret->textOffset, 0);
	} else {
//	    printf ("locateLibFunctions: did NOT find %s\n", lSym->name);
	    tempSyms[scount++] = lSym;
	}
    }

    /*
     * Create a special module for library functions, and do the inst on
     *   the desired functions.
     *
     */
    currentModule = newModule(ret, LIBRARY_MODULE, LIBRARY_MODULE, 0);
    for (i=0; i < nsyms; i++) {
	if ((stabs[i].n_type & 0xfe) == N_TEXT)  {
	    str = &strings[stabs[i].n_un.n_strx];
//	    printf ("Checking for lib function, found %s\n", str+1);
	    for (j=0; j < scount; j++) {
		if (!strcmp(str+1, (char*)tempSyms[j]->name))  {
		    currentFunc = newFunc(ret, currentModule, (char*)tempSyms[j]->name,
			stabs[i].n_value);
		    currentFunc->tag = tempSyms[j]->tags;
		    locateInstPoints(currentFunc, ret->code, ret->textOffset,0);
		    tempSyms[j]=tempSyms[--scount];  /* delete if found */
		    break;	/* why look further? */
		}
		
	    }
	}
    }

    /* Check for library functions not found... */
    for (j=0; j < scount; j++) {
	printf ("Warning:  Couldn't find library function %s\n", 
		(char*) tempSyms[j]->name);
    }

    delete(tempSyms);
}

image *loadSymTable(const char *file, int offset, List<libraryFunc*> libFuncs,
		    const char **iSym)
{
    int i;
    int fd;
    char *str;
    image *ret;
    char *name;
    int dynamic;
    int pagemask;
    int fileOffset;
    caddr_t mapAddr;
    int stringLength;
    struct exec exec;
    pdFunction *currentFunc;
    module *currentModule;
    char *currentDirectory;

    if (!(ret= new image)) {
      perror("new image");
      exit(-1);
    }
    ret->file = pool.findAndAdd(file);

    name = strrchr(file, '/');
    ret->name = pool.findAndAdd(name ? name+1 : file);

    fd = open(file, O_RDONLY);
    if (fd < 0) {
	perror(file);
	delete ret;
	return(NULL);
    }

    lseek(fd, offset, SEEK_SET);
    if (read(fd, (char *) &exec, sizeof(exec)) != sizeof(exec)) {
	perror("read");
	delete ret;
	return(NULL);
    }

    if (N_BADMAG(exec)) {
	perror("Bad exec");
	delete ret;
	return(NULL);
    }

    dynamic = 0;
    if (exec.a_dynamic) {
      logLine("Warning: Program dynamicly linked, can not inst system calls\n");
      dynamic = 1;
    }

    ret->textOffset = (unsigned) N_TXTADDR(exec);
#ifdef notdef
    ret->code = (void *) xmalloc(exec.a_text+exec.a_data);
    lseek(fd, N_TXTOFF(exec)+offset, SEEK_SET);
    if (read(fd, (char *) ret->code, exec.a_text+exec.a_data) != exec.a_text+exec.a_data) {
#endif

    // pagemask is the mask to remove page offset bits.
    pagemask = getpagesize() - 1;
    fileOffset = (N_TXTOFF(exec)+offset) & ~pagemask;

    // use mmap to get program into memory.
    mapAddr = mmap(0, exec.a_text+exec.a_data, PROT_READ, MAP_SHARED, fd, fileOffset);

    // get correct offset into the first mapped page.
    ret->code = mapAddr + (N_TXTOFF(exec)+offset - fileOffset);
    if (((int) mapAddr) == -1) {
	extern char *sys_errlist[];

	free(stabs);
	free(ret->code);
	free(strings);
	free(ret);
	sprintf(errorLine,"Unable to map text segment: %s\n", 
	    sys_errlist[errno]);
	logLine(errorLine);
	return(NULL);
    }

    stabs = (struct nlist *) xmalloc(exec.a_syms);
    lseek(fd, N_SYMOFF(exec)+offset, SEEK_SET);
    if (read(fd, (char *) stabs, exec.a_syms) != exec.a_syms) {
	free(stabs);
	free(ret->code);
	free(strings);
	free(ret);
	logLine("Unable to read symbols segment\n");
	return(NULL);
    }

    /* now read the string pool */
    (void) lseek(fd,  N_STROFF(exec)+offset, SEEK_SET);
    if (read(fd, (char *) &stringLength, sizeof(int)) != sizeof(int)) {
	free(ret->code);
	free(stabs);
	free(strings);
	free(ret);
	return(NULL);
    }

    strings = new char[stringLength];
    (void) lseek(fd,  N_STROFF(exec)+offset, SEEK_SET);

    if (read(fd, strings, stringLength) != stringLength) {
	free(ret->code);
	free(stabs);
	free(strings);
	free(ret);
	return(NULL);
    }

    (void) close(fd);
    nsyms = exec.a_syms/sizeof(struct nlist);
    currentDirectory = strdup("./");

    currentModule = NULL;
    for (i=0; i < nsyms; i++) {
	/* switch on symbol type to call correct routine */
	switch (stabs[i].n_type & 0xfe) {
	    case N_SLINE:
		currentModule->setLineAddr(stabs[i].n_desc,
					   (caddr_t) stabs[i].n_value);
		break;

	    case N_SO:
	    case N_SOL:
		str = &strings[stabs[i].n_un.n_strx];
		if (str[strlen(str)-1] == '/') {
		    /* directory definition */
		    currentDirectory = str;
		} else {
			mapLines(currentModule);
			currentModule = newModule(ret, currentDirectory, 
						  str, (caddr_t) stabs[i].n_value);
		}
		break;

	    case N_FUN:
		str = &strings[stabs[i].n_un.n_strx];
		currentFunc = newFunc(ret, currentModule, str,stabs[i].n_value);
		break;
	    case N_ENTRY:
		logLine("warning code contains alternate entry point\n");
		break;
	    default:
		break;
	}
    }


    if (!dynamic) {
	locateLibFunctions(ret, libFuncs);
    }

    /*
     * Now find all of the symbols used by dyninst.
     *
     */
    findInternalSymbols(ret, iSym);

    locateRogueFuncations(ret, &exec);

    free(strings);
    free(stabs);
    return(ret);
}
