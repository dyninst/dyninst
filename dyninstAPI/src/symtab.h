/*
 *  Copyright 1993 Jeff Hollingsworth.  All rights reserved.
 *
 */

/*
 * symtab.h - interface to generic symbol table.
 *
 * $Log: symtab.h,v $
 * Revision 1.5  1994/08/02 18:25:08  hollings
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
#include <sys/types.h>

#include "util/h/stringPool.h"
#include "util/h/list.h"
#include "dyninst.h"

/*
 * List of supported languages.
 *
 */
typedef enum { unknown, assembly, C, cPlusPlus, gnuCPlusPlus,
    fortran, CMFortran } supportedLanguages;

typedef struct functionRec function;
typedef struct moduleRec module;
typedef struct instPointRec instPoint;
typedef struct imageRec image;

#define LIBRARY_MODULE	"LIBRARY_MODULE"
#define NO_SYMS_MODULE	"NO_SYMS_MODULE"

typedef struct {
    int maxLine;		/* max possible line */
    caddr_t *addr;		/* addr[line] is the addr of line */
} lineTable;

struct moduleRec {
    char *compileInfo;
    char *fileName;		/* short file */
    char *fullName;		/* full path to file */
    supportedLanguages language;
    caddr_t addr;		/* starting address of module */
    List<function*> funcs;	/* functions defined in this module */
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

struct functionRec {
    char *symTabName;		/* name as it appears in the symbol table */
    char *prettyName;		/* user's view of name (i.e. de-mangled) */
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
    function *next;		/* next function in global function list */
    function *sibling;		/* next function with the same name - WARNING
					we assume name equality so this
					could either be c++ template functions
					beging replicated or other non global
					functions that appear twice.
				*/
};

/*
 * symbols we need to find from our RTinst library.  This is how we know
 *   were our inst primatives got loaded as well as the data space variables
 *   we use to put counters/timers and inst trampolines.  An array of these
 *   is placed in the image structure.
 *
 */
struct _internalSym {
    char *name;			/* name as it appears in the symbol table. */
    unsigned int addr;		/* absolute address of the symbol */
};

typedef _internalSym internalSym;

struct imageRec {
    char *file;			/* image file name */
    char *name;			/* filename part of file */
    int moduleCount;		/* number of modules */
    module *modules;		/* pointer to modules */
    int funcCount; 		/* number of functions */
    function *funcs;		/* pointer to linked list of functions */
    int iSymCount;		/* # of internal RTinst library symbols */
    internalSym *iSyms;		/* internal RTinst library symbols */
    void *code;			/* pointer to code */
    unsigned int textOffset;	/* base of where code is loaded */
    int offset;			/* offset of a.out in file */
    HTable<function*> funcAddrHash; /* hash table to find functions by address */
    image *next;		/* next in our list of images */
};

/*
 * a definition of a library function that we may wish to identify.  This is
 *   how we describe it to the symbol table parser, not how it appears in
 *   the symbol table.  Library functions are placed in a pseudo module 
 *   named LIBRARY_MODULE. 
 *
 */
image *parseImage(char *file, int offset);
module *newModule(image*, char *currDir, char *name, caddr_t addr);
function *newFunc(image*, module *, char *name, int addr);
extern stringPool pool;

class libraryFunc {
    public:
	libraryFunc(char *n, int t) {
	    name = pool.findAndAdd(n);
	    tags = t;
	}
	char *name;
	int tags;
};

typedef List<libraryFunc*> libraryList;

/*
 * Functions provided by machine/os/vendor specific files.
 *
 * iSym is the prefix to match on to find the callable inst functions.
 *
 */
image *loadSymTable(char *file, int offset, libraryList libraryFunctions, 
    char **iSym);

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
internalSym *findInternalSymbol(image *, char *name, Boolean warn = True);

/* find the address of the named internal symbol */
caddr_t findInternalAddress(image *, char *name, Boolean warn = True);

/* find the named module */
module *findModule(image *, char *name);

/* find the named funcation */
function *findFunction(image *, char *name);

/* find the function add the passed addr */
function *findFunctionByAddr(image *, caddr_t addr);

/* look through func for inst points */
void locateInstPoints(function*, void *, int, int calls);

/*
 * upcalls from machine specific code to main symbol table package 
 *
 */
void processLine(module*, int line, caddr_t addr);

/* record line # for each func entry */
void mapLines(module *);
