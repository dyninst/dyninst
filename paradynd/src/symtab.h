/*
 *  Copyright 1993 Jeff Hollingsworth.  All rights reserved.
 *
 */

/*
 * symtab.h - interface to generic symbol table.
 *
 * $Log: symtab.h,v $
 * Revision 1.8  1994/10/25 22:20:34  hollings
 * Added code to suppress "functions" that have aninvalid instruction
 * as their first instruction.  These are really read-only data that has
 * been placed in the text segment to protect it from writing.
 *
 * Revision 1.7  1994/09/30  19:47:17  rbi
 * Basic instrumentation for CMFortran
 *
 * Revision 1.6  1994/09/22  02:26:56  markc
 * Made structs classes
 *
 * Revision 1.5  1994/08/02  18:25:08  hollings
 * fixed modules to use list template for lists of functions.
 *
 * Revision 1.4  1994/07/22  19:21:11  hollings
 * removed mistaken divid by 1Meg for predicted cost.
 *
 * Revision 1.3  1994/07/20  23:23:43  hollings
 * added insn generated metric.
 *
 * Revision 1.2  1994/06/29  02:52:52  hollings
 * Added metricDefs-common.{C,h}
 * Added module level performance data
 * cleanedup types of inferrior addresses instrumentation defintions
 * added firewalls for large branch displacements due to text+data over 2meg.
 * assorted bug fixes.
 *
 * Revision 1.1  1994/01/27  20:31:46  hollings
 * Iinital version of paradynd speaking dynRPC igend protocol.
 *
 * Revision 1.4  1993/12/13  19:58:12  hollings
 * added sibling filed for functions that occur multiple times in the same
 * binary image (statics, and c++ template classes for example).
 *
 * Revision 1.3  1993/07/13  18:33:11  hollings
 * new include file syntax.
 *
 * Revision 1.2  1993/06/08  20:14:34  hollings
 * state prior to bc net ptrace replacement.
 *
 * Revision 1.1  1993/03/19  22:51:05  hollings
 * Initial revision
 *
 *
 */

extern "C" {
#include <sys/types.h>
#include <stdlib.h>
}

#include "util/h/stringPool.h"
#include "util/h/list.h"
#include "dyninst.h"
#include "arch-sparc.h"
#include "util.h"

/*
 * List of supported languages.
 *
 */
typedef enum { unknown, assembly, C, cPlusPlus, gnuCPlusPlus,
    fortran, CMFortran } supportedLanguages;

#define LIBRARY_MODULE	"LIBRARY_MODULE"
#define NO_SYMS_MODULE	"NO_SYMS_MODULE"

class pdFunction;
class instPoint;
class module;
class image;
class internalSym;

class pdFunction {
 public:
    pdFunction();
    stringHandle symTabName;		/* name as it appears in the symbol table */
    stringHandle prettyName;		/* user's view of name (i.e. de-mangled) */
    int line;			/* first line of function */
    module *file;		/* pointer to file that defines func. */
    caddr_t addr;		/* address of the start of the func */
    instPoint *funcEntry;	/* place to instrument entry (often not addr) */
    instPoint *funcReturn;	/* exit point for function */
    int callLimit;		/* max val of calls array */
    int callCount;		/* number of sub-routine cal points */
    instPoint **calls;		/* pointer to the calls */
    int ljmpCount;		/* number of long jumps out of func */
    instPoint *jmps;		/* long jumps out */
    int tag;			/* tags to ident special (library) funcs. */
    pdFunction *next;		/* next function in global function list */
    pdFunction *sibling;		/* next function with the same name - WARNING
					we assume name equality so this
					could either be c++ template functions
					beging replicated or other non global
					functions that appear twice.
				*/
};


class instPoint {
 public:
    instPoint() {
      addr = 0; originalInstruction.raw = 0; delaySlotInsn.raw = 0;
      aggregateInsn.raw =0;
      inDelaySlot=0; isDelayed = 0; callIndirect=0; callAggregate=0;
      callee=NULL; func=NULL;
    }
    int addr;                   /* address of inst point */
    instruction originalInstruction;    /* original instruction */
    instruction delaySlotInsn;  /* original instruction */
    instruction aggregateInsn;  /* aggregate insn */
    int inDelaySlot;            /* Is the instruction in a dealy slot */
    int isDelayed;		/* is the instruction a delayed instruction */
    int callIndirect;		/* is it a call whose target is rt computed ? */
    int callAggregate;		/* calling a func that returns an aggregate
				   we need to reolcate three insns in this case
				 */
    pdFunction *callee;		/* what function is called */
    pdFunction *func;		/* what function we are inst */
};


/* Stores source code to address in text association for modules */
class lineTable {
 public:
    lineTable() {
      maxLine = 100;
      addr = (caddr_t *) xcalloc(100, sizeof(caddr_t));
    }
    ~lineTable() {
      if (addr)
	free(addr);
    }
    void qsortLines() {
      qsort(addr, maxLine, sizeof(int), intComp);
    }
    int getMaxLine() { return maxLine;}
    void setLineAddr (int line, caddr_t lineAddr);
    caddr_t getLineAddr (int line) {
      if ((line >= 0) && (line < maxLine))
	return (addr[line]);
      else
	return (NULL);
    }

  private:
    int maxLine;		/* max possible line */
    caddr_t *addr;		/* addr[line] is the addr of line */
};

class module {
 public:
    module();
    void setLineAddr(int line, caddr_t addr) {
      lines.setLineAddr(line, addr);
    }
    caddr_t getLineAddr(int line) {
      return (lines.getLineAddr(line));
    }
    char *compileInfo;
    stringHandle fileName;		/* short file */
    stringHandle fullName;		/* full path to file */
    supportedLanguages language;
    caddr_t addr;		/* starting address of module */
    List<pdFunction*> funcs;	/* functions defined in this module */
    image *exec;		/* what executable it came from */
    lineTable lines;		/* line mapping info */
    module *next;		/* pointer to next module */
};

/* contents of line number field if line is unknown */
#define UNKNOWN_LINE	0

#define TAG_LIB_FUNC	0x01
#define TAG_IO_FUNC	0x02
#define TAG_MSG_FUNC	0x04
#define TAG_SYNC_FUNC	0x08
#define TAG_CPU_STATE	0x10	/* does the func block waiting for ext. event */
#define TAG_NON_FUNC	0x20	/* has an invalid instruction at entry point */

/*
 * symbols we need to find from our RTinst library.  This is how we know
 *   were our inst primatives got loaded as well as the data space variables
 *   we use to put counters/timers and inst trampolines.  An array of these
 *   is placed in the image structure.
 *
 */
class internalSym {
 public:
    internalSym() { 
      name = NULL; addr=0;
    }
    stringHandle name;		/* name as it appears in the symbol table. */
    unsigned int addr;		/* absolute address of the symbol */
};

class image {
 public:
    image();
    stringHandle file;		/* image file name */
    stringHandle name;		/* filename part of file */
    int moduleCount;		/* number of modules */
    module *modules;		/* pointer to modules */
    int funcCount; 		/* number of functions */
    pdFunction *funcs;		/* pointer to linked list of functions */
    int iSymCount;		/* # of internal RTinst library symbols */
    internalSym *iSyms;		/* internal RTinst library symbols */
    void *code;			/* pointer to code */
    unsigned int textOffset;	/* base of where code is loaded */
    int offset;			/* offset of a.out in file */
    HTable<pdFunction*> funcAddrHash; /* hash table to find functions by address */
    image *next;		/* next in our list of images */
    int symbolExists(const char *); /* Does the symbol exist in the image? */
    void postProcess(const char *);              /* Load .pif file */
};


/*
 * a definition of a library function that we may wish to identify.  This is
 *   how we describe it to the symbol table parser, not how it appears in
 *   the symbol table.  Library functions are placed in a pseudo module 
 *   named LIBRARY_MODULE. 
 *
 */
image *parseImage(char *file, int offset);
module *newModule(image*, const char *currDir, const char *name, caddr_t addr);
pdFunction *newFunc(image*, module *, const char *name, int addr);
extern stringPool pool;

class libraryFunc {
    public:
	libraryFunc(const char *n, int t) {
	    name = pool.findAndAdd(n);
	    tags = t;
	}
	stringHandle name;
	int tags;
};

/*
 * Functions provided by machine/os/vendor specific files.
 *
 * iSym is the prefix to match on to find the callable inst functions.
 *
 */
/*
image *loadSymTable(const char *file, int offset, List<libraryFunc*> libraryFunctions, 
		    const char **iSym);
*/
Boolean locateAllInstPoints(image *i);

/*
 * main interface to the symbol table based code.
 *
 */
image *parseImage(char *file, int offset);

/*
 * symbol table access functions.
 *
 */

/* find the named internal symbol */
internalSym *findInternalSymbol(image *, const char *name, Boolean warn = True);

/* find the address of the named internal symbol */
caddr_t findInternalAddress(image *, const char *name, Boolean warn = True);

/* find the named module */
module *findModule(image *, const char *name);

/* find the named funcation */
pdFunction *findFunction(image *, const char *name);

/* find the function add the passed addr */
pdFunction *findFunctionByAddr(image *, caddr_t addr);

/* look through func for inst points */
void locateInstPoints(pdFunction*, void *, int, int calls);

/* record line # for each func entry */
void mapLines(module *);
