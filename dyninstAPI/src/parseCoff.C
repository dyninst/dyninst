#include <stdio.h>
#include <filehdr.h>
#include <syms.h>
#include <cmplrs/demangle_string.h>  // For native C++ (cxx) name demangling.
#include <ctype.h>

#include "BPatch.h"
#include "BPatch_module.h"
#include "BPatch_collections.h"
#include "LineInformation.h"
#include <ldfcn.h>  // *** GCC 3.x bug: (Short explanation)
		    // <ldfcn.h> must be included after "BPatch.h"

		    // (Long explanation): The obj.h header
		    // (which is included by ldfcn.h) has this line:
		    // "#define global".  This becomes a problem when
		    // iostream is eventually included (by BPatch.h).
		    // A variable named "global" is defined, but the
		    // keyword is erased during preprocessing.

#define NOTYPENAME ""

// Stab definitions (from gdb) 
#define CODE_MASK 0x8F300
#define ECOFF_IS_STAB(sym) (((sym)->index & 0xFFF00) == CODE_MASK)
#define ECOFF_MARK_STAB(code) ((code)+CODE_MASK)
#define ECOFF_UNMARK_STAB(code) ((code)-CODE_MASK)
#define STABS_SYMBOL "@stabs"

// Main functions needed to parse stab strings.
extern char *current_func_name;
extern char *parseStabString(BPatch_module *, int linenum, char *str,
			     int fPtr, BPatch_type *commonBlock = NULL);

typedef union {
    pEXTR ext;
    pSYMR sym;
} SYMBOL, *pSYMBOL;

// eCoffParseInfo will hold the necessary state information
// as we parse an eCoff symbol table.
typedef struct {
    bool isExternal;
    pCHDRR symtab;
    pCFDR file;

    // File Descriptor Information
    char *strBase;
    SYMBOL symBase;
    pAUXU auxBase;
    pPDR pdrBase;
    pRFDT rfdBase;
    int symCount;
    int pdrCount;
} eCoffParseInfo;

// --------------------------------------------------------------------------
// Class eCoffSymbol definition. --------------------------------------------
// --------------------------------------------------------------------------
//
// eCoffSymbol will encapulate symbol data,
// abstracting away the idea of internal/external symbols.
class eCoffSymbol {
    SYMBOL sym_internal;
    eCoffParseInfo *info;

public:
    pdstring name;
    pSYMR sym;
    pAUXU aux;
    int ifd;

    eCoffSymbol(eCoffParseInfo *_info = NULL) { init(_info); }
    ~eCoffSymbol() { clear(); }

    void init(eCoffParseInfo *, bool = false);
    int index() { return (info->isExternal ? sym_internal.ext - info->symBase.ext
			  		   : sym_internal.sym - info->symBase.sym); }
    int id() { return 10000 + index() + (info->isExternal ? info->symtab->hdr.isymMax
							  : info->file->pfd->isymBase); }
    bool empty() { return sym_internal.ext == NULL; }
    eCoffParseInfo *getParseInfo() { return info; }

    void operator++();
    void operator=(int);
    void operator+=(int);

    void clear(bool = false);

private:
    void setState();
};

void eCoffSymbol::init(eCoffParseInfo *_info, bool cleanup)
{
    clear(cleanup);

    info = _info;
    if (info)
	sym_internal = info->symBase;
    else
	sym_internal.ext = NULL;

    setState();
}

void eCoffSymbol::operator++()
{
    if (info->isExternal)
	++(sym_internal.ext);
    else
	++(sym_internal.sym);

    // Bounds check.
    if (empty() || index() >= info->symCount) {
	sym_internal.ext = NULL;
	return;
    }
    setState();
}

void eCoffSymbol::operator=(int i)
{
    // Bounds check.
    if (i < 0 || i >= info->symCount) {
	sym_internal.ext = NULL;
	return;
    }

    sym_internal = info->symBase;
    if (info->isExternal)
        sym_internal.ext += i;
    else
        sym_internal.sym += i;
    setState();
}

void eCoffSymbol::operator+=(int i)
{
    if (info->isExternal)
        sym_internal.ext += i;
    else
        sym_internal.sym += i;

    // Bounds check.
    if (empty() || index() >= info->symCount) {
	sym_internal.ext = NULL;
	return;
    }
    setState();
}

void eCoffSymbol::setState()
{
    char prettyName[1024];

    if (sym_internal.ext == NULL)
	return;

    sym = info->isExternal ? &sym_internal.ext->asym : sym_internal.sym;
    ifd = info->isExternal ? sym_internal.ext->ifd : -1;

    name = (sym->iss == issNil ? "" : info->strBase + sym->iss);
    if (ECOFF_IS_STAB(sym)) {
	// Stab strings may be continued on the next symbol.
	while (name.length() > 0 && name[name.length() - 1] == '\\') {
	    if (info->isExternal)
		++(sym_internal.ext);
	    else
		++(sym_internal.sym);
	    sym = info->isExternal ? &sym_internal.ext->asym : sym_internal.sym;
	    ifd = info->isExternal ? sym_internal.ext->ifd : -1;

	    name = name.substr(0, name.length() - 1); // Remove '\\'
	    name += (sym->iss == issNil ? "" : info->strBase + sym->iss);
	}
    }

    if (name.length() < 1024) {
	MLD_demangle_string(name.c_str(), prettyName, 1024,
			    MLD_SHOW_DEMANGLED_NAME | MLD_NO_SPACES);
	if (strchr(prettyName, '('))
	    *strchr(prettyName, '(') = 0;
	name = prettyName;
    }

    // The AUX field is a bit more tricky.  I pulled these straight
    // from the Alpha Symbol Table Specification, section 5.3.7.3.
    if (sym->st == stGlobal ||
	sym->st == stStatic ||
	sym->st == stParam ||
	sym->st == stLocal && !info->isExternal ||
	sym->st == stProc && !info->isExternal ||
	sym->st == stMember && sym->sc == scInfo ||
	sym->st == stTypedef && sym->sc == scInfo ||
	sym->st == stStaticProc && !info->isExternal ||
	sym->st == stConstant ||
	sym->st == stBase && sym->sc == scInfo ||
	sym->st == stVirtBase && sym->sc == scInfo ||
	sym->st == stTag && sym->sc == scInfo ||
	sym->st == stInter && sym->sc == scInfo ||
	sym->st == stNamespace && sym->sc == scInfo ||
	sym->st == stUsing && sym->sc == scInfo ||
	sym->st == stAlias && sym->sc == scInfo)

	aux = (sym->index != indexNil ? info->auxBase + sym->index : NULL);
    else
	aux = NULL;
}

void eCoffSymbol::clear(bool cleanup)
{
    if (cleanup && info) delete info;
    sym_internal.ext = NULL;
}
// --------------------------------------------------------------------------
// End class eCoffSymbol definition. ----------------------------------------
// --------------------------------------------------------------------------

// Function Prototypes.
bool eCoffFindModule(pCHDRR, const pdstring &, eCoffParseInfo &);
void eCoffFillInfo(pCHDRR, int, eCoffParseInfo &);
void FindLineInfo(LDFILE *, eCoffParseInfo &, pdstring, LineInformation*&);
BPatch_type *eCoffParseType(BPatch_module *, eCoffSymbol &, bool = false);
BPatch_type *eCoffHandleTIR(BPatch_module *, eCoffSymbol &, bool = false);
BPatch_type *eCoffHandleTQ(eCoffSymbol &, BPatch_type *, unsigned int);
void eCoffParseProc(BPatch_module *, eCoffSymbol &, bool = false);
BPatch_type *eCoffParseStruct(BPatch_module *, eCoffSymbol &, bool = false);
void eCoffHandleRange(eCoffSymbol &, long int &, long int &, bool = false);
void eCoffHandleRange(eCoffSymbol &, pdstring &, pdstring &, bool = false);
long int eCoffHandleWidth(eCoffSymbol &, bool = false);
eCoffSymbol eCoffHandleRNDX(eCoffSymbol &, bool = false);
int eCoffGetOffset(eCoffSymbol &, pPDR);
pPDR eCoffGetFunction(eCoffSymbol &);
int stabsGetOffset(eCoffSymbol &, const char *, int);

// *** FUTURE BUG: The following code is currently designed for
// paradyn/dyninst's internal pdstring class.  While this code will still
// work if ever replaced by the STL pdstring, it could be better optimized.
// RSC (11/2002)

// fcn to construct type information
void parseCoff(BPatch_module *mod, char *exeName, const pdstring &modName,
	       LineInformation* lineInformation)
{
    LDFILE *ldptr;
    pCHDRR symtab;
    eCoffSymbol symbol;
    eCoffParseInfo currInfo;

    //
    // Initialize ldfcn structures.
    //
    ldptr = ldopen(exeName, NULL);
    if (!ldptr) {
	fprintf(stdout, "Error opening %s\n", exeName);
	// Print an error message here.
	return;
    }

    if (LDSWAP(ldptr)) {
	// Print an error message here.
	ldaclose(ldptr);
	return; // Bytes are swapped
    }

    symtab = SYMTAB(ldptr);
    if (!symtab) {
	// Print an error message here.
	// fprintf(stderr, "%s has no symbol table.\n", argv[1]);
	ldaclose(ldptr);
	return;
    }

    if (!eCoffFindModule(symtab, modName, currInfo)) {
	// Print an error message here.
	return;
    }

    if (!currInfo.isExternal)
	FindLineInfo(ldptr, currInfo, exeName, lineInformation);

    // STAB parse preparation.
    current_func_name = NULL;
    bool stabFormat = false;

    //
    // Main parsing loop.
    //
    for (symbol.init(&currInfo); !symbol.empty(); ++symbol) {

	//
	// STABS encoded within eCoff.
	//
	if (stabFormat && ECOFF_IS_STAB(symbol.sym)) {
	    int typeCode = ECOFF_UNMARK_STAB(symbol.sym->index);
	    if (typeCode == 138)
		typeCode = 128; //Why?
	    else if (typeCode == 42)
		typeCode = 32; //Why?
	    else if (typeCode == 46)
		typeCode = 36; //Why?
	    else if (typeCode == 170)
		typeCode = 160; //Why?

	    switch (typeCode) {
	    case 0x64:  // N_SO
	    case 0x84:  // N_SOL
		current_func_name = NULL; // reset for next object file
		continue;

	    case 32:    // Global symbols -- N_GSYM
	    case 36:    // functions and text segments -- N_FUN
	    case 128:   // typedefs and variables -- N_LSYM
	    case 160:   // parameter variable -- N_PSYM
		int value = symbol.sym->value;
		if ( ((typeCode == 128) || (typeCode == 160)) && current_func_name ) {
		    // See note above about STL strings.  This section of code
		    // applies in particular.
		    int varType = stLocal;
		    char *p = strchr(symbol.name.c_str(), ':');

		    if (!p)
			continue; // Not stab format!
		    p++;
		    if (*p == 'p') varType = stParam;

		    value = stabsGetOffset(symbol, current_func_name, varType);
		}

		char *temp = parseStabString(mod, 0, (char *)symbol.name.c_str(), value);
		if (*temp) {
		    // Error parsing the stabstr, return should be \0
		    fprintf(stderr, "Stab string parsing ERROR!! More to parse: %s\n", temp);
		}
		break;
	    }
	    continue;
	}

	else if (stabFormat || ECOFF_IS_STAB(symbol.sym)) {
	    if (symbol.name == STABS_SYMBOL)
		stabFormat = true;
	    continue;
	}

	//
	// Pure eCoff.
	//
	else {
	    switch(symbol.sym->st) {

	    case stProc:  // Function definitions
	    case stStaticProc:
		eCoffParseProc(mod, symbol);
		break;

	    case stGlobal:
	    case stConstant:
	    case stStatic:
		if (_SC_IS_DATA(symbol.sym->sc) || _SC_IS_TLSDATA(symbol.sym->sc)) {
		    BPatch_type *ptrType = eCoffParseType(mod, symbol);
		    if (ptrType)
			mod->moduleTypes->addGlobalVariable(symbol.name.c_str(), ptrType);
		}
		break;

	    case stTypedef: // Simple typedef.
	    case stBlock:   // Possible C structure, union, or enum.
	    case stBase:    // Base class.
	    case stTag:     // C++ class, structure, union, or enum.
		eCoffParseType(mod, symbol, true);
		break;

	    case stFile:
		// Module file name information.  We already have the
		// information contained in this symbol.  Ignore it.

	    case stLocal:
	    case stParam:
		// These two should never happen outside of an stProc
		// or stStaticProc block.  Should we print an error?

	    default:
		break;
	    } //switch
	}
    }

    //Close the file
    ldaclose(ldptr);
}

bool eCoffFindModule(pCHDRR symtab, const pdstring &modName, eCoffParseInfo &info)
{
    if (modName == "DEFAULT_MODULE") {
	// As far as I can tell, the external symbols hold the
	// information requested by DEFAULT_MODULE.
	eCoffFillInfo(symtab, -1, info);
	return true;

    } else {
	pCFDR file = symtab->pcfd;
	for (int i = 0; i < symtab->cfd; ++i) {
	    char *name = file[i].pss + file[i].pfd->rss;

	    // Ignore anonymous modules.
	    if (file[i].pfd->rss == issNil)
		continue;

	    // Ignore full pathname.  (Should we be doing this?)
	    if (strrchr(name, '/') != NULL)
		name = strrchr(name, '/') + 1;

	    // Check to see if we've found the desired module.
	    if (modName == name) {
		eCoffFillInfo(symtab, i, info);
		return true;
	    }
	}
    }
    return false;
}

void eCoffFillInfo(pCHDRR symtab, int fileIndex, eCoffParseInfo &info)
{
    // Sanity Check
    assert(fileIndex != -1 || symtab != NULL);

    info.symtab = symtab;
    if (fileIndex == -1) {
	// Retrieve external symbol information

	info.isExternal = true;
	info.file = NULL;
	info.strBase = symtab->pssext;
	info.symBase.ext = symtab->pext;
	info.auxBase = symtab->paux;
	info.pdrBase = symtab->ppd;
	info.rfdBase = NULL; // This will be filled out later.

	info.symCount = symtab->cext;
	info.pdrCount = symtab->hdr.ipdMax;

    } else {
	pCFDR file = symtab->pcfd + fileIndex;

	info.isExternal = false;
	info.file = file;
	info.strBase = file->pss;
	info.symBase.sym = file->psym;
	info.auxBase = file->paux;
	info.pdrBase = file->ppd;
	info.rfdBase = file->prfd;

	info.symCount = file->pfd->csym;
	info.pdrCount = file->pfd->cpd;
    }
}

void FindLineInfo(LDFILE *ldptr, eCoffParseInfo &info,
                  pdstring fileName, LineInformation*& lineInformation)
{
    // For some reason, the count fields of pCFDR structures are not
    // filled out correctly when the symbol table is read in via the
    // SYMTAB() macro (or any other way).  We must use the underlying
    // FDR structure instead.
    char funcName[1024];
    pCFDR fileDesc = info.file;
    pSYMR tempSym;

    //
    // Manually read in line information due to buggy LDFCN library.
    //
    unsigned char *lineInfo = (unsigned char *)malloc(info.symtab->hdr.cbLine);
    if (!lineInfo) {
	fprintf(stderr, "*** WARNING: Cannot read line information ");
	fprintf(stderr, "for file %s: Malloc error.\n", fileName.c_str());
	return;
    }
    int numleft = info.symtab->hdr.cbLine;
    FSEEK(ldptr, info.symtab->hdr.cbLineOffset, 0);
    while (numleft > 0) numleft -= FREADM(lineInfo, 1, numleft, ldptr);

    // for each procedure entry look at the information
    for (int i = 0; i < fileDesc->pfd->cpd; i++) {
        pPDR procedureDesc = fileDesc->ppd + i;

        // No name is available for proc.
        if (!fileDesc->psym || !fileDesc->pfd->csym ||
	    procedureDesc->isym == -1) continue;

        // Get the (demangled) name of the procedure, if available.
        tempSym = fileDesc->psym + procedureDesc->isym;	
        MLD_demangle_string(fileDesc->pss + tempSym->iss, funcName, 1024, 
                            MLD_SHOW_DEMANGLED_NAME | MLD_NO_SPACES);
	if (P_strchr(funcName, '(')) *P_strchr(funcName, '(') = 0;

	if (P_strlen(funcName) == 0)
	    continue;

        FunctionInfo* currentFuncInfo = NULL;
        FileLineInformation* currentFileInfo = NULL;
        lineInformation->insertSourceFileName(
            pdstring(funcName), fileName.c_str(),
            &currentFileInfo,&currentFuncInfo);

	// No space to store file information
	if (!currentFileInfo || !currentFuncInfo)
	    continue;

        // no line information for the filedesc is available
        if (!fileDesc->pfd->cline || !fileDesc->pline || 
	    procedureDesc->iline == -1) continue;

	// (Possibly buggy) GCC line information check.
	if (procedureDesc->lnLow == 6 && procedureDesc->lnHigh == 6) {
	    //
	    // GCC eCoff Line Information
	    //
	    tempSym = (fileDesc->psym + procedureDesc->isym + 1);

	    while (tempSym->st == stLabel && tempSym->sc == scText) {
		currentFileInfo->insertLineAddress(currentFuncInfo,
						   tempSym->index,
						   tempSym->value);
		++tempSym;
	    }

	} else {
	    //
	    // Pure eCoff Line Information
	    //
	    int idx = fileDesc->pfd->cbLineOffset +
		      procedureDesc->cbLineOffset;
	    int endidx;
	    int currLine = procedureDesc->lnLow;
	    unsigned long currAddr = procedureDesc->adr;

	    // Find end index
	    int nextpd = -1;
	    for (int pcount = 0; pcount < fileDesc->pfd->cpd; ++pcount) {
		pPDR pd = fileDesc->ppd + pcount;
		if (pd != procedureDesc && pd->iline != ilineNil &&
		    pd->cbLineOffset >= procedureDesc->cbLineOffset)

		    if (nextpd == -1 ||
			pd->cbLineOffset < fileDesc->ppd[nextpd].cbLineOffset)
			nextpd = pcount;
	    }
	    endidx = (nextpd == -1 ? fileDesc->pfd->cbLine
				   : fileDesc->ppd[nextpd].cbLineOffset);
	    endidx -= procedureDesc->cbLineOffset;
	    endidx += idx;

	    while (idx < endidx) {
		long delta;

		currentFileInfo->insertLineAddress(currentFuncInfo,
						   currLine, currAddr);
		currAddr += ((lineInfo[idx] & 0xFU) + 1) * 4;
		if ((lineInfo[idx] & 0xF0U) == 0x80U) {
		    // Extended Delta
		    ++idx;
		    delta = ((signed char)lineInfo[idx]) << 8;
		    ++idx;
		    delta |= lineInfo[idx];
		} else
		    delta = ((signed char)lineInfo[idx]) >> 4;

		currLine += delta;
		++idx;
	    }
        }
    }
    free(lineInfo);
}

BPatch_type *eCoffParseType(BPatch_module *mod, eCoffSymbol &symbol, bool typeDef) {
    int id = symbol.id();
    pdstring name;
    eCoffSymbol remoteSymbol;
    BPatch_type *newType = NULL;

    // Symbol has no type information.
    if (!symbol.aux && symbol.sym->st != stBlock)
	return NULL;

    newType = mod->moduleTypes->findType(id);
    if (newType) {
	// This type is already inserted.

	if (symbol.sym->st == stTag ||
	    symbol.sym->st == stBase ||
	    symbol.sym->st == stBlock) {

	    // Skip to end of block definition.
	    eCoffParseStruct(mod, symbol, true);
	}
	return newType;
    }

    switch (symbol.sym->st) {

    case stTag:    // C++ class, structure, union, or enum.
    case stBase:   // C++ base class.
    case stBlock:  // Possible C structure, union, or enum.
	newType = eCoffParseStruct(mod, symbol);
	break;

    case stInter:
	eCoffHandleRNDX(symbol);
	// Fall through (don't break).

    case stUsing:
	eCoffHandleRNDX(symbol);
	return NULL;

    case stAlias:
	remoteSymbol = eCoffHandleRNDX(symbol);
	newType = eCoffParseType(mod, remoteSymbol);
	remoteSymbol.clear(true);
	break;

    case stTypedef:
	name = symbol.name;
	newType = eCoffHandleTIR(mod, symbol, true);

	if (newType)
	    newType = new BPatch_type(name.c_str(), id, newType);
	break;

    default:
	newType = eCoffHandleTIR(mod, symbol);
	break;
    }

    if (typeDef && newType)
	mod->moduleTypes->addType(newType);

    if (!newType) {
	fprintf(stdout, "*** Error processing symbol %s\n", symbol.name.c_str());
	fflush(stdout);
    }

    return newType;
}

BPatch_type *eCoffHandleTIR(BPatch_module *mod, eCoffSymbol &symbol, bool typeDef)
{
    pdstring low, high, name;
    long int width = -1;
    eCoffSymbol remoteSymbol;
    pAUXU currTIR = (symbol.aux)++;
    BPatch_type *result, *subType;

    // Handle explicit width field (bitfields)
    if (currTIR->ti.fBitfield) {
	width = eCoffHandleWidth(symbol, false);
    }

    switch(currTIR->ti.bt) {

    case btEnum:
    case btClass:
    case btStruct:
    case btUnion:
	remoteSymbol = eCoffHandleRNDX(symbol);
	result = eCoffParseType(mod, remoteSymbol, true);
	remoteSymbol.clear(true);
	break;

    case btRange:
    case btRange_64:
	remoteSymbol = eCoffHandleRNDX(symbol);
	subType = eCoffParseType(mod, remoteSymbol);
	remoteSymbol.clear(true);

	eCoffHandleRange(symbol, low, high, currTIR->ti.bt == btRange_64);
	result = new BPatch_type(symbol.name.c_str(), symbol.id(), BPatchSymTypeRange,
				 low.c_str(), high.c_str());
	break;

    case btTypedef:
	name = symbol.name;
	remoteSymbol = eCoffHandleRNDX(symbol);
	subType = eCoffParseType(mod, remoteSymbol);
	remoteSymbol.clear(true);

	if (subType)
	    result = new BPatch_type(name.c_str(), symbol.id(), subType);
	break;

    case btIndirect:
	// Rndx points to an entry in the aux symbol table that contains a TIR
	remoteSymbol = eCoffHandleRNDX(symbol, true);
	result = eCoffHandleTIR(mod, remoteSymbol);
	remoteSymbol.clear(true);

	// All done.  No need to parse TIR records.
	return result;

    case btProc:
	remoteSymbol = eCoffHandleRNDX(symbol);
	remoteSymbol.clear(true);

	// Not clear that this is the correct behavior.
	result = new BPatch_type(symbol.name.c_str(), symbol.id(), BPatch_dataFunction);
	break;

    case btSet:
    case btAdr32:
    case btComplex:
    case btDComplex:
    case btFixedDec:
    case btFloatDec:
    case btString:
    case btBit:
    case btPicture:
    case btPtrMem:
    case btVptr:
    case btAdr64:
    case btChecksum:
 // case btAdr: Same as btAdr64
 // case btArrayDesc: FORTRAN 90: Array descriptor
 // case btScaledBin: COBOL: Scaled Binary
 // case btDecimal: COBOL: packed/unpacked decimal
 // case btFixedBin: COBOL: Fixed binary
	// Nothing to do!
	result = NULL;
	break;

    case btNil: // Regarded as void
    case btVoid:
	if (typeDef)
	    result = new BPatch_type(symbol.name.c_str(), -11, BPatch_built_inType, 0);
	else 
	    result = new BPatch_type("void", -11, BPatch_built_inType, 0);
	break;

    case btChar:
	if (typeDef)
	    result = new BPatch_type(symbol.name.c_str(), -2, BPatch_built_inType, 1);
	else
	    result = new BPatch_type("char", -2, BPatch_built_inType, 1);
	break;

    case btUChar:
	if (typeDef)
	    result = new BPatch_type(symbol.name.c_str(), -5, BPatch_built_inType, 1);
	else
	    result = new BPatch_type("unsigned char", -5, BPatch_built_inType, 1);
	break;

    case btShort:
	if (typeDef)
	    result = new BPatch_type(symbol.name.c_str(), -3, BPatch_built_inType, 2);
	else
	    result = new BPatch_type("short", -3, BPatch_built_inType, 2);
	break;

    case btUShort:
	if (typeDef)
	    result = new BPatch_type(symbol.name.c_str(), -7, BPatch_built_inType, 2);
	else
	    result = new BPatch_type("unsigned short", -7, BPatch_built_inType, 2);
	break;

    case btInt32:
 // case btInt: Same as btInt32
	if (typeDef)
	    result = new BPatch_type(symbol.name.c_str(), -1, BPatch_built_inType, 4);
	else
	    result = new BPatch_type("int", -1, BPatch_built_inType, 4);
	break;

    case btUInt32:
 // case btUInt: Same as btUInt32
	if (typeDef)
	    result = new BPatch_type(symbol.name.c_str(), -8, BPatch_built_inType, 4);
	else
	    result = new BPatch_type("unsigned int", -8, BPatch_built_inType, 4);
	break;

    case btLong32:
	if (typeDef)
	    result = new BPatch_type(symbol.name.c_str(), -4, BPatch_built_inType, 4);
	else
	    result = new BPatch_type("long", -4, BPatch_built_inType, 4);
	break;

    case btULong32:
	if (typeDef)
	    result = new BPatch_type(symbol.name.c_str(), -10, BPatch_built_inType,
				     sizeof(unsigned long));
	else
	    result = new BPatch_type("unsigned long", -10, BPatch_built_inType,
				     sizeof(unsigned long));
	break;

    case btFloat:
	if (typeDef)
	    result = new BPatch_type(symbol.name.c_str(), -12, BPatch_built_inType, sizeof(float));
	else
	    result = new BPatch_type("float", -12, BPatch_built_inType,
				     sizeof(float));
	break;

    case btDouble:
	if (typeDef)
	    result = new BPatch_type(symbol.name.c_str(), -13, BPatch_built_inType,
				     sizeof(double));
	else
	    result = new BPatch_type("double", -13, BPatch_built_inType,
				     sizeof(double));
	break;

    case btLong64:
 // case btLong: Same as btLong64
    case btLongLong64:
 // case btLongLong: Same as btLongLong64
    case btInt64:
	if (typeDef)
	    result = new BPatch_type(symbol.name.c_str(), -32, BPatch_built_inType, 8);
	else
	    result = new BPatch_type("long long", -32, BPatch_built_inType, 8);
	break;

    case btULong64:
 // case btULong: Same as btULong64
    case btULongLong64:
 // case btULongLong: Same as btULongLong64
    case btUInt64:
	if (typeDef)
	    result = new BPatch_type(symbol.name.c_str(), -33, BPatch_built_inType, 8);
	else
	    result = new BPatch_type("unsigned long long", -33,
				     BPatch_built_inType, 8);
	break;

    case btLDouble:
	if (typeDef)
	    result = new BPatch_type(symbol.name.c_str(), -14, BPatch_built_inType,
				     sizeof(long double));
	else
	    result = new BPatch_type("long double", -14, BPatch_built_inType,
				     sizeof(long double));
	break;

    case btInt8:
    case btUInt8:
	if (typeDef)
	    result = new BPatch_type(symbol.name.c_str(), -27, BPatch_built_inType, 1);
	else
	    result = new BPatch_type("integer*1", -27, BPatch_built_inType, 1);
	break;

    default:
	// Never to reach here!
	result = NULL;
	break;
    } // switch

    if (!result)
	return NULL; // Something wrong!

    // Handle type qualifiers
    while (currTIR) {
	unsigned int tq;
  	for (int tcount = 0; tcount < itqMax; ++tcount) {
	    switch (tcount) {
		case 0: tq = currTIR->ti.tq0; break;
		case 1: tq = currTIR->ti.tq1; break;
		case 2: tq = currTIR->ti.tq2; break;
		case 3: tq = currTIR->ti.tq3; break;
		case 4: tq = currTIR->ti.tq4; break;
		case 5: tq = currTIR->ti.tq5; break;
	    }
	    if (tq != tqNil)
		result = eCoffHandleTQ(symbol, result, tq);
	    else
		break;
	}
	currTIR = (currTIR->ti.continued ? ++(symbol.aux) : NULL);
    }
    return result;
}

BPatch_type *eCoffHandleTQ(eCoffSymbol &symbol, BPatch_type *prevType, unsigned int tq)
{
    long int low, high;
    BPatch_type *newType = NULL;
    eCoffSymbol indexType;

    assert(prevType);

    switch (tq) {
    case tqProc:
	newType = new BPatch_type(symbol.name.c_str(), symbol.id(), BPatch_dataPointer, prevType);
	break;

    case tqRef:
	newType = new BPatch_type(symbol.name.c_str(), symbol.id(), BPatch_reference, prevType);
	break;

    case tqPtr:
    case tqFar: // Ptr type qualifier
	newType = new BPatch_type(symbol.name.c_str(), symbol.id(), BPatch_pointer, prevType);
	break;

    case tqArray:
    case tqArray_64: // Array handling
	// Parse (but ignore) rndx to array subscript.
	indexType = eCoffHandleRNDX(symbol);
	indexType.clear(true); // Memory clean up

	// Parse range.
	eCoffHandleRange(symbol, low, high, tq == tqArray_64);

	// Parse (but ignore) width.
	eCoffHandleWidth(symbol, tq == tqArray_64);

	newType = new BPatch_type(symbol.name.c_str(), -1, BPatch_array, prevType, low, high);
	break;

    case tqVol:		// Volitile
    case tqConst:	// Constant
    case tqShar:	// Shareable UPC
    case tqSharArr_64:	// Array_64 distributed across processors UPC
	// Don't know what to do with these, but they are
	// valid.  Return the original BPatch_type unmodified.
	newType = prevType;
	break;

    case tqNil: // End of record.  Return NULL.
	break;
    }
    return newType;
}

void eCoffParseProc(BPatch_module *mod, eCoffSymbol &symbol, bool skip)
{
    int endIndex;
    BPatch_type *typePtr;
    BPatch_localVar *local;

    BPatch_Vector<BPatch_function *> bpfv;
    BPatch_function *fp = NULL;

       // get the base address of the procedure
    pdstring sname = symbol.name;
    const char *fname = sname.c_str();
    if (!fname) return;  // Some findFunction in this file is passing in NULL

    mod->findFunction(fname, bpfv, false /* no print */);
    if (!bpfv.size()) {
      //cerr << __FILE__ << __LINE__ << ":  Unable to find function: " << symbol.name << endl;
      return;
    }
    fp = bpfv[0];
    if (!fp) return;

    // Sanity check.
    if (!fp || symbol.sym->st != stProc && symbol.sym->st != stStaticProc)
	return;

    // Handle external pointers into local tables.
    if (symbol.getParseInfo()->isExternal) {
	eCoffParseInfo localInfo, *info = symbol.getParseInfo();
	eCoffSymbol localSymbol;

	eCoffFillInfo(info->symtab, symbol.ifd, localInfo);
	localSymbol.init(&localInfo);
	localSymbol += symbol.sym->index;
	eCoffParseProc(mod, localSymbol);

	return;
    }

    // Skip to end if requested, or if we've already seen this function.
    endIndex = symbol.aux->isym - 1;
    ++(symbol.aux);
    if (skip || fp->getReturnType()) {
	symbol = endIndex;
	return;
    }

    // Set return type.
    typePtr = eCoffParseType(mod, symbol);
    if (typePtr) fp->setReturnType(typePtr);

    if (symbol.sym->sc == scInfo) {
	switch (symbol.sym->value) {
	case -1: // Procedure with no code.
	    // Skip to end for now.
	    // I don't know what to do with it.
	    symbol = endIndex;
	    break;

	case -2: // Function prototype or function pointer definition.
	    
	    break;

	default: // C++ virtual member function.
	    break;
	}

    } else if (symbol.sym->sc == scText) {
	//
	// Function definition.
	//
	pPDR currFunc = eCoffGetFunction(symbol);

	++symbol;
	while (symbol.index() < endIndex) {
	    bool isOffset = (symbol.sym->sc == scAbs || symbol.sym->sc == scVar);
	    int value = (isOffset ? eCoffGetOffset(symbol, currFunc) : symbol.sym->value);

	    switch (symbol.sym->st) {
	    case stParam:
		if (symbol.sym->sc == scAbs ||
		    symbol.sym->sc == scRegister ||
		    symbol.sym->sc == scVar ||
		    symbol.sym->sc == scVarRegister ||
		    symbol.sym->sc == scUnallocated ||
		    _SC_IS_DATA(symbol.sym->sc) ) {

		    typePtr = eCoffParseType(mod, symbol);
		    local = new BPatch_localVar(symbol.name.c_str(), typePtr, -1,
						value, symbol.sym->sc, isOffset);
		    fp->funcParameters->addLocalVar(local);
		    fp->addParam(symbol.name.c_str(), typePtr,
				 -1, value, symbol.sym->sc);
		}
		break;

	    case stLocal:
		if (symbol.sym->sc == scAbs ||
		    symbol.sym->sc == scRegister ||
		    symbol.sym->sc == scVar ||
		    symbol.sym->sc == scVarRegister ||
		    symbol.sym->sc == scUnallocated) {

		    // Unfortunatly, eCoff has no notion of line numbers for
		    // local variable definitions.
		    typePtr = eCoffParseType(mod, symbol);
		    local = new BPatch_localVar(symbol.name.c_str(), typePtr, -1,
						value, symbol.sym->sc, isOffset);
		    fp->localVariables->addLocalVar(local);
		}
		break;

	    case stStatic:
		if (symbol.sym->sc == scCommon) {
		    BPatch_image *img = (BPatch_image *)mod->getObjParent();
		    BPatch_Vector<BPatch_field *> *fields;
		    long baseAddr;

		    typePtr = eCoffParseType(mod, symbol);
		    baseAddr = (long)img->findVariable(symbol.name.c_str())->getBaseAddr();
		    if (typePtr) {
			fields = typePtr->getComponents();
			for (unsigned int i = 0; i < fields->size(); ++i) {
			    int offset = (*fields)[i]->getOffset() / 8;
			    local = new BPatch_localVar((*fields)[i]->getName(),
							(*fields)[i]->getType(),
							-1,
							baseAddr + offset,
							scCommon,
							false);
			    fp->localVariables->addLocalVar(local);
			}
		    }
		}

	    case stProc:
	    case stStaticProc:
		if (symbol.aux->isym == indexNil) {
		    // Fortran alternate entry point.
		    ++symbol;
		    while (symbol.sym->st == stParam) {
			++symbol;
		    }
		} else {
		    // Nested function.
		    eCoffParseProc(mod, symbol);
		}
		break;
	    }
	    ++symbol;
	}
    }
}

// Parses the symbols associated with user-defined types.
// Eg: Classes, structures, unions, and enumerations.
BPatch_type *eCoffParseStruct(BPatch_module *mod, eCoffSymbol &symbol, bool skip)
{
   int endIndex;
   BPatch_type *result, *field;
   BPatch_dataClass dataType = BPatch_unknownType;
   bool isKnown = false, isEnum = false, isStruct = false;

   // Sanity checks.
   if (! ((symbol.sym->st == stTag) ||
          (symbol.sym->st == stBlock && (symbol.sym->sc == scInfo ||
                                         symbol.sym->sc == scCommon ||
                                         symbol.sym->sc == scSCommon))))
      return NULL;

   // Block already parsed.  Skip to end.
   if (skip) {
      if (symbol.sym->st == stTag) ++symbol;
      symbol = symbol.sym->index - 1;
      return result;
   }

   // Determine data class.
   if (symbol.sym->st == stTag) {

      // C++ structure, union, or enumeration.
      switch (symbol.aux->ti.bt) {
        case btStruct:	dataType = BPatch_structure; break;
        case btUnion:	dataType = BPatch_union; break;
        case btEnum:	dataType = BPatch_enumerated; break;
        case btClass:	dataType = BPatch_typeClass; break;
      }
      if (dataType != BPatch_unknownType)
         isKnown = true;

      ++symbol;

   } else if (symbol.sym->sc == scCommon ||
              symbol.sym->sc == scSCommon) {

      // Fortran common block.
      dataType = BPatch_dataCommon;
      isKnown = true;

   } else {

      // C style structure, union, or enumeration.
      // Assume enumeration for now.
      dataType = BPatch_enumerated;
   }

   result = new BPatch_type(symbol.name.c_str(), symbol.id(),
                            dataType, symbol.sym->value);

   if (!isKnown) {
      isEnum = true;
      isStruct = false;
   }

   endIndex = symbol.sym->index - 1;
   while (symbol.index() < endIndex) {
      pdstring fieldName = symbol.name;
      long fieldOffset = symbol.sym->value;

      // Terrible capitalization hack (Fortran only).
      if (dataType == BPatch_dataCommon) {
         fieldName = "";
         for (int i = 0; i < symbol.name.length(); ++i)
            fieldName += pdstring((char)tolower(symbol.name[i]));
         symbol.name = fieldName;
      }

      if (symbol.sym->st == stMember) {

         field = eCoffHandleTIR(mod, symbol);
         if (field) {
            // Adding a struct/union member
            result->addField(fieldName.c_str(), field->getDataClass(), field,
                             fieldOffset, field->getSize(), BPatch_visUnknown);
            if (fieldOffset > 0) isStruct = true;
            isEnum = false;

         } else
            // Adding an enum member
            result->addField(fieldName.c_str(), BPatch_scalar, symbol.sym->value,
                             BPatch_visUnknown);
      }
      if (symbol.sym->st == stProc && symbol.sym->sc == scInfo)
         eCoffParseProc(mod, symbol, true);

      ++symbol;
   }
   if (!isKnown && !isEnum)
      result->setDataClass(isStruct ? BPatch_structure : BPatch_union);

   return result;
}

void eCoffHandleRange(eCoffSymbol &symbol, long int &low, long int &high, bool range_64)
{
    low = symbol.aux->dnLow;
    ++(symbol.aux);
    if (range_64) {
       // Low range consists of 2 records.
       low |= ((long int)symbol.aux->dnLow) << 32;
       ++(symbol.aux);
    }

    high = symbol.aux->dnHigh;
    ++(symbol.aux);
    if (range_64) {
       // High range consists of 2 records.
       high |= ((long int)symbol.aux->dnHigh) << 32;
       ++(symbol.aux);
    }
}

void eCoffHandleRange(eCoffSymbol &symbol, pdstring &s_low, pdstring &s_high,
                      bool range_64)
{
    long int i_low, i_high;
    char tmp[100]; // The largest 64 bit integer requires 21 bytes to store
		   // as a string.  Buffer overflow should not be a problem.

    eCoffHandleRange(symbol, i_low, i_high, range_64);

    snprintf(tmp, sizeof(tmp), "%ld", i_low);
    s_low = tmp;

    snprintf(tmp, sizeof(tmp), "%ld", i_high);
    s_high = tmp;
}

long int eCoffHandleWidth(eCoffSymbol &symbol, bool width_64)
{
    long int result;

    result = symbol.aux->width;
    ++(symbol.aux);
    if (width_64) {
       result |= ((long int)symbol.aux->width) << 32;
       ++(symbol.aux);
    }
    return result;
}

eCoffSymbol eCoffHandleRNDX(eCoffSymbol &symbol, bool isAux)
{
   pCFDR remoteFile;
   int fileIdx = symbol.aux->rndx.rfd;
   int symIdx = symbol.aux->rndx.index;
   eCoffParseInfo *info = symbol.getParseInfo(), *remoteInfo = new eCoffParseInfo;
   
   ++(symbol.aux);
   if (fileIdx == ST_RFDESCAPE) {
      fileIdx = symbol.aux->isym;
      ++(symbol.aux);
   }
   
   if (symbol.ifd == -1)
      // Local symbol, use current file.
      remoteFile = info->file;
   else
      // External symbol pointing into local table.
      remoteFile = info->symtab->pcfd + symbol.ifd;
   
   if (remoteFile->prfd)
      fileIdx = remoteFile->prfd[fileIdx];
   eCoffFillInfo(info->symtab, fileIdx, *remoteInfo);
   
   eCoffSymbol result(remoteInfo);
   if (isAux) {
      result.name = symbol.name;
      result.sym = symbol.sym;
      result.aux += symIdx;
   } else
      result = symIdx;
   
   return result;
}

// eCoffGetFunction() returns the pPDR structure associated
// with a stProc or stStaticProc.
pPDR eCoffGetFunction(eCoffSymbol &symbol) {
    eCoffParseInfo *info = symbol.getParseInfo();

    for (int i = 0; i < info->pdrCount; ++i)
       if (info->pdrBase[i].isym == symbol.index())
          return info->pdrBase + i;

    return NULL;
}

// Straight from the documentation:
// Alpha Symbol Table Specification, Section 5.3.4.3
int eCoffGetOffset(eCoffSymbol &symbol, pPDR func) {
    if (!func) return -1;
    if (symbol.sym->st == stLocal)
       return (func->frameoffset - func->localoff + symbol.sym->value);
    else
       return (func->frameoffset - 48 + symbol.sym->value);
}

// This version returns the pPDR structure associated
// with a text string.  Very inefficient.
pPDR stabsGetFunction(eCoffSymbol &origSymbol, const char *func) {
    eCoffParseInfo *info = origSymbol.getParseInfo();
    eCoffSymbol symbol(info);

    for (int i = 0; i < info->pdrCount; ++i) {
       symbol = info->pdrBase[i].isym;
       if (symbol.name == func)
          return info->pdrBase + i;
    }
    return NULL;
}

int stabsGetOffset(eCoffSymbol &symbol, const char *func, int st) {
    if (!func) return -1;
    pPDR func_pdr = stabsGetFunction(symbol, func);

    if (st == stLocal)
       return (func_pdr->frameoffset - func_pdr->localoff + symbol.sym->value);
    else
       return (func_pdr->frameoffset - 48 + symbol.sym->value);
}
