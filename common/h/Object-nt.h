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

/************************************************************************
 * Object-nt.h: Windows NT object files.
************************************************************************/





#if !defined(_Object_nt_h_)
#define _Object_nt_h_




/************************************************************************
 * header files.
************************************************************************/

#include "util/h/Dictionary.h"
#include "util/h/String.h"
#include "util/h/Symbol.h"
#include "util/h/Types.h"
#include "util/h/Vector.h"

#include <stdlib.h>
#include <winnt.h>





/************************************************************************
 * class Object
************************************************************************/

class Object : public AObject {
public:
             Object (const string, void (*)(const char *) = log_msg);
             Object (const Object &);
	     Object (const string, u_int, void (*)(const char *) = log_msg);

    virtual ~Object ();

    Object&   operator= (const Object &);

    //void *getFPOtable() const; // returns a pointer to the FPO table
    //void *getFPOtableEntry(unsigned offset) const;

private:
    void    load_object ();
    

    HANDLE fileHandle; // handle for file
    HANDLE mapHandle;  // handle for mapping
    char *baseAddr;    // base address of mapping

    //DWORD ptr_to_rdata;
     
};

static int symbol_compare(const void *x, const void *y) {
    Symbol *s1 = (Symbol *)x;
    Symbol *s2 = (Symbol *)y;
    return (s1->addr() - s2->addr());
}



/* break fullName into name and path parts */
static void getFileNameAndPath(const string fullName, string &name, string &path) {
  unsigned long size = GetFullPathName(fullName.string_of(), 0, 0, 0);
  char *buffer = new char[size];
  char *fptr;
  GetFullPathName(fullName.string_of(), size, buffer, &fptr);
  name = fptr;
  *fptr = 0;
  path = buffer;
  delete buffer;
}

/* get the value of environment variable varName */
static void getEnvVar(const string varName, string &value) {
  unsigned long pathLen = GetEnvironmentVariable(varName.string_of(), 0, 0);
  char *buffer = new char[pathLen];
  GetEnvironmentVariable(varName.string_of(), buffer, pathLen);
  value = buffer;
  delete buffer;
}

/* find a .dbg file for the .exe or .dll file */
static bool findDbgFile(const string exeFile, DWORD exeTimeStamp,
			HANDLE &dbgFile, 
			HANDLE &dbgMapping, const char * &dbgAddr,
			IMAGE_SYMBOL * &syms, unsigned long &nsyms,
			char * &strs) {
  string fileName;
  string filePath;
  getFileNameAndPath(exeFile, fileName, filePath);

  /*
     kludge: we only read a .dbg file if it is for kernel32.dll.
     There seem to be some problems with some files - the symbols
     don't match the code, even though the timestamps in the
     files match, so we will not read them for now.
  */
  if (fileName != "kernel32.dll" && fileName != "KERNEL32.dll") {
    return false;
  }

  string dbgName = string(fileName.string_of(), fileName.length()-3) + "dbg";

  // first, look in the same directory of the .exe file
  string dbgPath = filePath + dbgName;
  dbgFile = CreateFile(dbgPath.string_of(), GENERIC_READ, FILE_SHARE_READ, 
		       NULL, OPEN_EXISTING, NULL, NULL);
  if (dbgFile != INVALID_HANDLE_VALUE) {
    dbgMapping = CreateFileMapping(dbgFile, NULL, PAGE_READONLY,0,0,NULL);
    if (dbgMapping == INVALID_HANDLE_VALUE) {
      CloseHandle(dbgFile);
      return false;
    }
    dbgAddr = (char *)MapViewOfFile(dbgMapping, FILE_MAP_READ, 0, 0, 0);
    if (dbgAddr == NULL) {
      CloseHandle(dbgMapping);
      CloseHandle(dbgFile);
      return false;
    }
  } else {
    // now look in %SystemRoot%\SYMBOLS\DLL
    string sysRootDir;
    getEnvVar("SystemRoot", sysRootDir);
    dbgPath = sysRootDir + "\\SYMBOLS\\DLL\\" + dbgName;
    dbgFile = CreateFile(dbgPath.string_of(), GENERIC_READ, FILE_SHARE_READ, 
			 NULL, OPEN_EXISTING, NULL, NULL);
    if (dbgFile == INVALID_HANDLE_VALUE)
      return false;
    dbgMapping = CreateFileMapping(dbgFile, NULL, PAGE_READONLY,0,0,NULL);
    if (dbgMapping == INVALID_HANDLE_VALUE) {
      CloseHandle(dbgFile);
      return false;
    }
    dbgAddr = (char *)MapViewOfFile(dbgMapping, FILE_MAP_READ, 0, 0, 0);
    if (dbgAddr == NULL) {
      CloseHandle(dbgMapping);
      CloseHandle(dbgFile);
      return false;
    }
  }

  // read the header
  IMAGE_SEPARATE_DEBUG_HEADER *dbgHeader =
       (IMAGE_SEPARATE_DEBUG_HEADER *)dbgAddr;
  IMAGE_DEBUG_DIRECTORY *dbgDir;

  if (dbgHeader->Signature == IMAGE_SEPARATE_DEBUG_SIGNATURE
      && dbgHeader->TimeDateStamp == exeTimeStamp) {
    //printf("dgbHeader->TimeDateStamp %x %x %x %x %d\n", dbgHeader->TimeDateStamp,
    //	     exeTimeStamp, dbgHeader->CheckSum, dbgHeader->ImageBase,
    //	     dbgHeader->SizeOfImage);
    dbgDir = (IMAGE_DEBUG_DIRECTORY *)
               &dbgAddr[sizeof(IMAGE_SEPARATE_DEBUG_HEADER) +
		        dbgHeader->NumberOfSections*
		       sizeof(IMAGE_SECTION_HEADER) + 
		       dbgHeader->ExportedNamesSize];

    if (dbgDir->Type == IMAGE_DEBUG_TYPE_COFF) {
      IMAGE_COFF_SYMBOLS_HEADER *coffHdr = 
	(IMAGE_COFF_SYMBOLS_HEADER *)&dbgAddr[dbgDir->PointerToRawData];
      nsyms = coffHdr->NumberOfSymbols;
      syms = (IMAGE_SYMBOL *) &dbgAddr[dbgDir->PointerToRawData+
				       coffHdr->LvaToFirstSymbol];
      strs = (char *) &dbgAddr[dbgDir->PointerToRawData+
			       coffHdr->LvaToFirstSymbol+
			       nsyms*sizeof(IMAGE_SYMBOL)];
      return true;
    }
  }

  // generate error message -- TODO
  if (dbgHeader->Signature != IMAGE_SEPARATE_DEBUG_SIGNATURE) {
    fprintf(stderr,"Unable to read debug file %s: invalid signature\n", dbgPath.string_of());
  } else if (dbgHeader->TimeDateStamp != exeTimeStamp) {
    fprintf(stderr,"Unable to read debug file %s: wrong date/timestamp\n",
    	   dbgPath.string_of());
  } else {
    fprintf(stderr,"Unable to process debug file %s: file is not in COFF format\n",
	   dbgPath.string_of());
  }
  
  UnmapViewOfFile(dbgAddr);
  CloseHandle(dbgMapping);
  CloseHandle(dbgFile);
  return false;

}


inline
void
Object::load_object() {
    // Set these to invalid values in case we fail along the way
    baseAddr = NULL;
    mapHandle = NULL;
    fileHandle = INVALID_HANDLE_VALUE;

    //
    const char* file = file_.string_of();
    bool        did_open = false;

    IMAGE_DOS_HEADER * dosHdr;
    IMAGE_NT_HEADERS * ntHdrs;
    IMAGE_FILE_HEADER * header;
    IMAGE_SECTION_HEADER * sections;
    IMAGE_SYMBOL * syms;
    unsigned long nsyms;
    char *strs;

    string        module = "DEFAULT_MODULE";
    string        name   = "DEFAULT_SYMBOL";
    vector<Symbol> allSymbols;
    Address       addr;
    Address       imageBase;
    bool isDll = false;

    bool useDbgFile = false;
    HANDLE dbgFile;     // handle to .dbg file
    HANDLE dbgMapping;  // handle to mapping of .dbg file
    char * dbgAddr;     // base address of mapping

    /* try */ {

        fileHandle = CreateFile(file, GENERIC_READ, FILE_SHARE_READ, 
			NULL, OPEN_EXISTING, NULL, NULL);
	if (fileHandle == INVALID_HANDLE_VALUE) {
            log_perror(err_func_, "CreateFile");
            /* throw exception */ goto cleanup;
        }
        did_open = true;

	mapHandle = CreateFileMapping(fileHandle, NULL, PAGE_READONLY,0,0,NULL);
	if (mapHandle == INVALID_HANDLE_VALUE) {
            log_perror(err_func_, "CreateFileMapping");
            /* throw exception */ goto cleanup;
	}
	baseAddr = (char *)MapViewOfFile(mapHandle, FILE_MAP_READ, 0, 0, 0);
	if (baseAddr == NULL) {
            log_perror(err_func_, "MapViewOfFile");
            /* throw exception */ goto cleanup;
        }

	/* read the DOS header */
        dosHdr = (IMAGE_DOS_HEADER *)baseAddr;
	if (dosHdr->e_magic != IMAGE_DOS_SIGNATURE) {
	    log_perror(err_func_, "Bad magic number");
            /* throw exception */ goto cleanup;
	}

	/* read NT headers */
	ntHdrs = (IMAGE_NT_HEADERS *)(&baseAddr[dosHdr->e_lfanew]);
	if (ntHdrs->Signature != IMAGE_NT_SIGNATURE) {
	    log_perror(err_func_, "Bad magic number");
            /* throw exception */ goto cleanup;
	}

        header = &ntHdrs->FileHeader;

#ifdef DEBUG_LOAD_OBJECT
	printf("Machine = %x\n", header->Machine);
	printf("Num sections = %d\n", header->NumberOfSections);
	printf("Time/date = %x\n", header->TimeDateStamp);
	printf("symbol table = %x\n", header->PointerToSymbolTable);
	printf("Num syms = %d\n", header->NumberOfSymbols);
	printf("Optional = %d\n", header->SizeOfOptionalHeader);
	printf("Chararc = %x\n", header->Characteristics);
	printf("BC: %x\n", ntHdrs->OptionalHeader.BaseOfCode);
	printf("BD: %x\n", ntHdrs->OptionalHeader.BaseOfData);
#endif

        isDll = header->Characteristics & IMAGE_FILE_DLL;

#ifdef DEBUG_LOAD_OBJECT
	if (isDll) printf("Object is a dll.\n");
	else printf("Object is not a dll.\n");
#endif

	imageBase = ntHdrs->OptionalHeader.ImageBase;
#ifdef DEBUG_LOAD_OBJECT
	printf("Image base: %lx\n", (unsigned long)imageBase);
#endif

	/* read the sections */
	sections = (IMAGE_SECTION_HEADER *) (&baseAddr[dosHdr->e_lfanew +
						 +sizeof(DWORD)
	  + sizeof(IMAGE_FILE_HEADER) + header->SizeOfOptionalHeader]);
	
	for (unsigned u1 = 0; u1 < header->NumberOfSections; u1++) {
	  char sname[9];
	  strncpy(sname, (const char *) &sections[u1].Name[0], 8);
	  sname[8] = 0;
#ifdef DEBUG_LOAD_OBJECT
	  printf("Section name: %s\n", sname);
	  printf("  VirtualAddress = %x\n",
		  (unsigned)sections[u1].VirtualAddress);
	  printf("  SizeOfRawData = %d\n",
		  (unsigned)sections[u1].SizeOfRawData);
	  printf("  PointerToRawData = %x\n",
		  (unsigned)sections[u1].PointerToRawData);
	  printf(  "PointerToRelocations = %x\n",
		  (unsigned)sections[u1].PointerToRelocations);
	  printf("  PointerToLinenumbers = %x\n",
		  (unsigned)sections[u1].PointerToLinenumbers);
	  printf("  NumberOfRelocations = %d\n",
		  (unsigned)sections[u1].NumberOfRelocations);
	  printf("  NumberOfLinenumbers = %d\n",
		  sections[u1].NumberOfLinenumbers);
	  printf("  Characteristics = %x\n", sections[u1].Characteristics);
#endif
	  if (strcmp(sname, ".text") == 0) {
	    code_ptr_ = (Word*)&baseAddr[sections[u1].PointerToRawData];
	    code_off_ = (Word)(imageBase +
	                ntHdrs->OptionalHeader.BaseOfCode);
	    code_len_ = (unsigned)(sections[u1].Misc.VirtualSize/sizeof(Word));
#ifdef DEBUG_LOAD_OBJECT
	    printf("codeoff = imageBase(%x) + ntHdrs->Optionalheader.BaseOfCode(%x)\n", (unsigned)imageBase, (unsigned)ntHdrs->OptionalHeader.BaseOfCode);
            printf("codeoff: %x, codelen: %d (to %x)\n",
		    code_off_, code_len_, code_off_ + (code_len_ << 2));
#endif
	  }
	  if (strcmp(sname, ".data") == 0) {
	    data_ptr_ = (Word*)&baseAddr[sections[u1].PointerToRawData];
	    data_off_ = (Word)(imageBase +
			       sections[u1].VirtualAddress
			       /*ntHdrs->OptionalHeader.BaseOfData*/);
	    data_len_ = (unsigned)(sections[u1].Misc.VirtualSize/sizeof(Word));
#ifdef DEBUG_LOAD_OBJECT
            printf("dataoff: %x, datalen: %d\n", data_off_, data_len_);
#endif
	  }

	  //if (strcmp(sname, ".rdata") == 0) {
	  //  ptr_to_rdata = sections[u1].PointerToRawData;
	  //}

        }

	if (ntHdrs->FileHeader.PointerToSymbolTable) {
	  syms = (IMAGE_SYMBOL *)
                      (&baseAddr[ntHdrs->FileHeader.PointerToSymbolTable]);
	  nsyms = ntHdrs->FileHeader.NumberOfSymbols;
	  strs = (char *)(&baseAddr[ntHdrs->FileHeader.PointerToSymbolTable
			    + nsyms * sizeof(IMAGE_SYMBOL)]);
#ifdef DEBUG_LOAD_OBJECT
	  printf("Found symbol table in file.\n");
#endif
	} else {
	  // no symbol table
	  if (findDbgFile(file, header->TimeDateStamp, dbgFile, dbgMapping,
	                  dbgAddr, syms, nsyms, strs)) {
	    useDbgFile = true;
#ifdef DEBUG_LOAD_OBJECT
	    printf("Found symbol table in separate dbg file.\n");
#endif
	  }
	  else {
#ifdef DEBUG_LOAD_OBJECT
	    printf("File %s has no symbol table\n", file);
#endif
	    nsyms = 0;
	  }
	}

#ifdef notdef
	// We could use the imagehlp library to read symbols that are in
	// CodeView format. However, the image help library symbol handler
	// does not give some information we need. In particular,
	// it does not give the type of symbols, so we don't known
        // which symbols are functions and which are not.
      
	if (nsyms == 0) {
	  // object file has no COFF debug info. We use the imagehelp
	  // library to read symbol table in other formats
	  // This is a kludge, we should improve this code later
	  extern HANDLE kludgeProcHandle;
	  if (kludgeProcHandle) {

	    IMAGEHLP_MODULE mod;
	    unsigned numSyms = 0;

	    mod.SizeOfStruct = sizeof(IMAGEHLP_MODULE);
	    if (SymGetModuleInfo(kludgeProcHandle, imageBase, &mod)) {
	      numSyms = mod.NumSyms;
	      //fprintf(stderr,"#### numSyms=%d, type=%d\n", numSyms, mod.SymType);
	    }

	    if (numSyms > 0 && (mod.SymType==SymCoff || mod.SymType==SymCv
				|| mod.SymType==SymPdb)) {
	      
	      char buffer[1024+sizeof(IMAGEHLP_SYMBOL)];
	      IMAGEHLP_SYMBOL *sym = (IMAGEHLP_SYMBOL *)&buffer;

	      sym->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL);
	      sym->Address = code_off_;
	      sym->MaxNameLength = 1024;

	      // SymGetSymNext doesn't always find a symbol if we
              // set the initial address as the code offset,
              // so we keep trying successive addresses 
	      // to make sure we get the first symbol
	      while (!SymGetSymNext(kludgeProcHandle, sym)
		     && sym->Address <= code_off_ + code_len_*sizeof(Word))
		sym->Address++;
	      
	      do {
		//printf(">> %s %x %d\n", sym->Name, sym->Address, sym->Size);
		if (sym->Address > code_off_ + code_len_*sizeof(Word))
		  continue;
		allSymbols += Symbol(sym->Name, "", Symbol::PDST_FUNCTION,
				     Symbol::SL_GLOBAL,
				     sym->Address, false, sym->Size);
	      } while (SymGetSymNext(kludgeProcHandle, sym));
	    }
	  }
	}
#endif

	bool cygwin_dll = false;
	if (isDll) {
	  // for dynamic linked libraries, we have a single module, with
	  // the same name as the library
	  string fileName;
	  string filePath;
	  getFileNameAndPath(file, fileName, filePath);
	  allSymbols += Symbol(fileName, "", Symbol::PDST_MODULE,
				Symbol::SL_GLOBAL, imageBase, false);
	}

	/* 
	 * We need this flag (which is set if the object was compiled with
	 * gcc) because we need it to know how to interpret the addresses in
	 * the symbols table.  Gcc stores absolute addresses for symbols,
	 * whereas Microsoft compilers store addresses relative to imageBase.
	 */
	bool gcc_compiled = false;
	for (unsigned v = 0; v < nsyms; v++) {
	 
	  if (syms[v].N.Name.Short != 0) {
	      char sname[9];
	      strncpy(sname, (char *)(&syms[v].N.ShortName), 8);
	      sname[8] = 0;
	      name = sname;
	  } else {
	      name = &strs[syms[v].N.Name.Long];
	  }

#ifdef DEBUG_LOAD_OBJECT
	  printf("syms[%d] name = <%s>\n", v, name.string_of());
	  printf(" syms[%d].Value = %u\n", v, (unsigned int)syms[v].Value);
	  printf(" syms[%d].SectionNumber = %d\n", v,
		  (int)syms[v].SectionNumber);
	  printf(" syms[%d].Type = %d\n", v, (int)syms[v].Type);
	  printf(" syms[%d].StorageClass = %u\n", v,
		  (unsigned int)syms[v].StorageClass);
	  printf(" syms[%d].NumberOfAuxSymbols = %d\n", v,
		  (int)syms[v].NumberOfAuxSymbols);
#endif


	  if (name.prefixed_by("_$$$") || name.prefixed_by("$$$")) {
	    v += syms[v].NumberOfAuxSymbols;
	  } else if (syms[v].StorageClass == IMAGE_SYM_CLASS_LABEL &&
	      (name == "gcc2_compiled." || name == "___gnu_compiled_c")) {
	    gcc_compiled = true;
#ifdef DEBUG_LOAD_OBJECT
	    printf("This module is gcc_compiled.\n");
#endif
	  } else if (syms[v].StorageClass == IMAGE_SYM_CLASS_FILE) {
	    // a file name. The name of the file is a zero terminated
	    // string in one or more auxiliar entries following this one
	    name = (char *) (&syms[v+1]);
	    // skip the auxiliary entries
      	    v += (strlen(name.string_of()) / sizeof(IMAGE_SYMBOL)) + 1;

	    // there may be a .text entry following the file name
	    // this entry has the starting address for this file
	    if (syms[v+1].N.Name.Short != 0 &&
		strncmp((char *)(&syms[v+1].N.ShortName), ".text", 5)==0) {
	      addr = syms[v+1].Value;
	      v++;
	    } else {
	      addr = 0;
	    }
	    if (!isDll)
	      allSymbols += Symbol(name, "", Symbol::PDST_MODULE,
				Symbol::SL_GLOBAL, imageBase+addr, false);

	  /* XXX Checking for "exit" below is hack to make things work for gcc
	     for the time being.  We need to be able to instrument exit, but
	     we don't parse the dll cygwin32.dll (which contains exit)
	     correctly and we can't tell what's a function and what's not.
	     So, we make exit a funtion no matter what it seems to be. */
	  } else if ((syms[v].StorageClass != IMAGE_SYM_CLASS_TYPE_DEFINITION
		      && ISFCN(syms[v].Type))
	             || (gcc_compiled &&
			 (name == "__exit" || name == "_exit"
			 || name == "exit"))) {
	    if (syms[v].N.Name.Short != 0) {
	      char sname[9];
	      strncpy(sname, (char *)(&syms[v].N.ShortName), 8);
	      sname[8] = 0;
	      name = sname;
	    } else {
	      name = &strs[syms[v].N.Name.Long];
	    }

	    Address fcn_addr;
	    if (gcc_compiled)
		fcn_addr = syms[v].Value;
	    else
		fcn_addr = imageBase+syms[v].Value;
#ifdef DEBUG_LOAD_OBJECT
	    if (gcc_compiled)
		printf("Got function, address is %x\n", fcn_addr);
	    else
    		printf("Got function, address is %x + %x = %x\n",
       		    imageBase, syms[v].Value, fcn_addr);
#endif
	    if (syms[v].StorageClass == IMAGE_SYM_CLASS_EXTERNAL)
	      allSymbols += Symbol(name, "", Symbol::PDST_FUNCTION,
				   Symbol::SL_GLOBAL, fcn_addr, false);
	    else
	      allSymbols += Symbol(name, "", Symbol::PDST_FUNCTION,
				   Symbol::SL_LOCAL, fcn_addr, false);

	    v += syms[v].NumberOfAuxSymbols;
	  } else if (syms[v].SectionNumber > 0) {
	    if (syms[v].N.Name.Short != 0) {
	      char sname[9];
	      strncpy(sname, (char *)(&syms[v].N.ShortName), 8);
	      sname[8] = 0;
	      name = sname;
	    } else {
	      name = &strs[syms[v].N.Name.Long];
	    }
	    Address sym_addr;
	    if (gcc_compiled)
	      sym_addr = syms[v].Value;
	    else
    	      sym_addr = imageBase+syms[v].Value;
#ifdef DEBUG_LOAD_OBJECT
	      printf( "%s: gcc_compiled = %d, imageBase = %lx, syms[v].Value = %lx, sym_addr = %lx\n",
		name.string_of(),
		(int)gcc_compiled,
		(unsigned long)imageBase,
		(unsigned long)syms[v].Value, (unsigned long)sym_addr);
#endif
	    if (name == ".text") {

	    }
	    else if (syms[v].StorageClass == IMAGE_SYM_CLASS_EXTERNAL)
	      allSymbols += Symbol(name, "", Symbol::PDST_OBJECT,
				   Symbol::SL_GLOBAL,
				   sym_addr, false);
	    else
	      allSymbols += Symbol(name, "", Symbol::PDST_OBJECT,
				   Symbol::SL_LOCAL,
				   sym_addr, false);
	    v += syms[v].NumberOfAuxSymbols;
	  } else {
	    v += syms[v].NumberOfAuxSymbols;
	  }

	}

#ifdef DEBUG_LOAD_OBJECT
	printf("All symbols found.\n");
#endif

	const string emptyStr = "";
	// add an extra symbol to mark the end of the text segment
	allSymbols += Symbol("", "", Symbol::PDST_OBJECT, Symbol::SL_GLOBAL, 
			     code_off_+code_len_*sizeof(Word), false);

        // Sort the symbols on address to find the function boundaries
        allSymbols.sort(symbol_compare);


#ifdef XXX_GENERATE_MODULES
static unsigned int modnumber = 0;
static unsigned numfuncs = 0;
static const unsigned funcspermod = 500;
#endif

        // find the function boundaries
        unsigned nsymbols = allSymbols.size();
        string modName = "";
        for (unsigned u = 0; u < nsymbols; u++) {

#ifdef XXX_GENERATE_MODULES
	  if (!isDll && ++numfuncs % 500 == 0) {
            modName = string("Mod") + string(++modnumber);
            symbols_[modName] = Symbol(modName, "", Symbol::PDST_MODULE,
			 Symbol::SL_GLOBAL, allSymbols[u].addr(), false);
	  }
#endif
            unsigned size = 0;
	    if (allSymbols[u].type() == Symbol::PDST_FUNCTION) {
		unsigned v = u+1;
		while (v < nsymbols) {
		  // The .ef below is a special symbol that gcc puts in to
		  // mark the end of a function.
		  if (allSymbols[v].addr() != allSymbols[u].addr() &&
		      (allSymbols[v].type() == Symbol::PDST_FUNCTION ||
		       allSymbols[v].name() == ".ef"))
		    break;
		  v++;
		}
		if (v < nsymbols)
		  size = (unsigned)allSymbols[v].addr() 
			 - (unsigned)allSymbols[u].addr();
		else
		  size = (unsigned)(code_off_+code_len_*sizeof(Word))
		         - (unsigned)allSymbols[u].addr();
	    }

            if (allSymbols[u].name() != "")
	      symbols_[allSymbols[u].name()] =
		Symbol(allSymbols[u].name(), 
		       isDll ? allSymbols[u].module(): modName, 
		       allSymbols[u].type(), allSymbols[u].linkage(),
		       allSymbols[u].addr(), allSymbols[u].kludge(),
		       size);
	}      

      if (useDbgFile) {
	UnmapViewOfFile(dbgAddr);
	CloseHandle(dbgMapping);
	CloseHandle(dbgFile);
      }
    }
    /* catch */
cleanup: {
      /*
        if (did_open && (close(fd) == -1)) {
            log_perror(err_func_, "close");
        }
	*/
    }
}


inline
Object::Object(const string file, void (*err_func)(const char *))
    : AObject(file, err_func) {
    load_object();
}

// for shared object files
inline
Object::Object(const string file,u_int,void (*err_func)(const char *))
    : AObject(file, err_func) {
   load_object();
}

inline
Object::Object(const Object& obj)
    : AObject(obj) {
    load_object();
}


inline
Object::~Object() {
    if (baseAddr != NULL)
	UnmapViewOfFile(baseAddr);
    if (mapHandle != NULL)
    	CloseHandle(mapHandle);
    if (fileHandle != INVALID_HANDLE_VALUE)
    	CloseHandle(fileHandle);
}

inline
Object&
Object::operator=(const Object& obj) {
    (void) AObject::operator=(obj);
    return *this;
}


#endif /* !defined(_Object_nt_h_) */
