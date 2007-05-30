/*
 * Copyright (c) 1996-2007 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

// $Id: Object-coff.C,v 1.5 2007/05/30 19:20:45 legendre Exp $

#include "common/h/Dictionary.h"
#include "Object.h"
#include "Object-coff.h"
#include <cmplrs/demangle_string.h>  // For native C++ (cxx) name demangling.

bool GCC_COMPILED=false; //Used to detect compiler type. True if mutatee is 
			 //compiled with a GNU compiler. parseCoff.C needs this

/* ScName() and StName() used for debugging only.

const char *StName(int st)
{
	switch (st) {
	case stNil:		return "stNil";
	case stGlobal:		return "stGlobal";
	case stStatic:		return "stStatic";
	case stParam:		return "stParam";
	case stLocal:		return "stLocal";
	case stLabel:		return "stLabel";
	case stProc:		return "stProc";
	case stBlock:		return "stBlock";
	case stEnd:		return "stEnd";
	case stMember:		return "stMember";
	case stTypedef:		return "stTypedef";
	case stFile:		return "stFile";
	case stRegReloc:	return "stRegReloc";
	case stForward:		return "stForward";
	case stStaticProc:	return "stStaticProc";
	case stConstant:	return "stConstant";
	case stStaParam:	return "stStaParam";
	case stBase:		return "stBase";
	case stVirtBase:	return "stVirtBase";
	case stTag:		return "stTag";
	case stInter:		return "stInter";
	case stSplit:		return "stSplit";
	case stModule:		return "stModule";
	case stModview:		return "stModview";
	case stAlias:		return "stAlias";
	case stStr:		return "stStr";
	case stNumber:		return "stNumber";
	case stExpr:		return "stExpr";
	case stType:		return "stType";
	}
	return "Bad St";
}

const char *ScName(int sc)
{
	switch (sc) {
	case scNil:		return "scNil";
	case scText:		return "scText";
	case scData:		return "scData";
	case scBss:		return "scBss";
	case scRegister:	return "scRegister";
	case scAbs:		return "scAbs";
	case scUndefined:	return "scUndefined";
	case scUnallocated:	return "scUnallocated";
	case scBits:		return "scBits";
	case scTlsUndefined:	return "scTlsUndefined";
	case scRegImage:	return "scRegImage";
	case scInfo:		return "scInfo";
	case scUserStruct:	return "scUserStruct";
	case scSData:		return "scSData";
	case scSBss:		return "scSBss";
	case scRData:		return "scRData";
	case scVar:		return "scVar";
	case scCommon:		return "scCommon";
	case scSCommon:		return "scSCommon";
	case scVarRegister:	return "scVarRegister";
	case scVariant:		return "scVariant";
	case scSUndefined:	return "scSUndefined";
	case scInit:		return "scInit";
	case scBasedVar:	return "scBasedVar";
	case scXData:		return "scXData";
	case scPData:		return "scPData";
	case scFini:		return "scFini";
	case scRConst:		return "scRConst";
	case scSymRef:		return "scSymRef";
	case scTlsCommon:	return "scTlsCommon";
	case scTlsData:		return "scTlsData";
	case scTlsBss:		return "scTlsBss";
	}
	return "Bad Sc";
}
*/

static inline bool obj_read_section(SCNHDR& secthead, LDFILE *ldptr,
				    Word *buffer) {
  if (!secthead.s_scnptr) return false;
  if (ldfseek(ldptr, secthead.s_scnptr, SEEK_SET) == -1) return false;

  if (ldfread((void*) buffer, 1, secthead.s_size, ldptr) != secthead.s_size)
    return false;
  else
    return true;
}

// The possible data sections
// These should only be used locally
#define K_D_INDEX    0
#define K_XD_INDEX   1
#define K_PD_INDEX   2
#define K_SD_INDEX   3
#define K_RD_INDEX   4
#define K_RC_INDEX   5 
#define K_L4_INDEX   6
#define K_L8_INDEX   7
#define K_LA_INDEX   8

// Attempt to find the largest contiguous (in virtual address space) region.
// This region must include ".data", and may include the other data like regions
static inline bool find_data_region(vector<Address>& all_addr,
				    vector<long>& all_size,
				    vector<long>& all_disk,
				    unsigned long& data_len, Address& data_off) {
  // Start at data and work back
  assert(all_addr[K_D_INDEX]); assert(all_size[K_D_INDEX]);
  assert(all_addr.size() == all_size.size());

  Address current = all_addr[K_D_INDEX];
  Address min_adr = current;
  Address max_adr = current + all_size[K_D_INDEX];

  unsigned index, max=all_addr.size();

  bool updated=true;
  while (updated) {
    updated = false;
    for (index=0; index<max; index++) {
      if (all_addr[index] && all_size[index] && all_disk[index] &&
	  ((all_addr[index] + all_size[index]) == current)) {
	current = all_addr[index];
	updated = true;
      }
    }
  }
  min_adr = current;

  // Start at data and work forward
  current = max_adr;
  updated=true;
  while (updated) {
    updated = false;
    for (index=0; index<max; index++) {
      if (all_addr[index] && all_size[index] && all_disk[index] && 
	  (all_addr[index] == current)) {
	current = all_addr[index] + all_size[index];
	updated = true;
      }
    }
  }

  max_adr = current;
  
  data_len = (max_adr - min_adr);
  data_off = min_adr;
  assert(min_adr <= all_addr[K_D_INDEX]);
  assert(max_adr >= all_addr[K_D_INDEX] + all_size[K_D_INDEX]);
  return true;
}

// Read in from the contiguous data regions, put the data in 'buffer'
static inline bool read_data_region(vector<Address>& all_addr,
				    vector<long>& all_size,
				    vector<long>& all_disk,
				    unsigned long& data_len, Address& data_off,
				    Word *buffer, LDFILE *ldptr) {
  unsigned index, max = all_disk.size();
  Address max_adr = data_off + data_len;
  assert(all_size.size() == all_addr.size());
  assert(all_disk.size() == all_addr.size());
  for (index=0; index<max; index++) {
    if ((all_addr[index] >= data_off) &&
	((all_addr[index] + all_size[index]) <= max_adr)) {
      if (ldfseek(ldptr, all_disk[index], SEEK_SET) == -1) return false;
      Word *buf_temp = buffer + (all_addr[index] - data_off);
      if (ldfread((void*) buf_temp, 1, all_size[index], ldptr) != all_size[index])
	return false;
    }
  }
  return true;
}

void Object::load_object(bool sharedLibrary) {
    bool        success=true, text_read=false;
    HDRR        sym_hdr;
    pCHDRR      sym_tab_ptr = NULL;
    long        index=0;
    SYMR        symbol;
    unsigned short sectindex=1;
    SCNHDR      secthead;
    unsigned 	nsymbols;
    vector<Symbol> allSymbols;
    vector<Address> all_addr(9, 0);
    vector<long> all_size(9, 0);
    vector<bool> all_dex(9, false);
    vector<long> all_disk(9, 0);

	// Read the text and data sections
	unsigned short fmax = fhdr.f_nscns;

	dynamicallyLinked = false;

	// Life would be so much easier if there were a guaranteed order for
	// these sections.  But the man page makes no mention of it.
	while (sectindex < fmax) {
	  if (ldshread(ldptr, sectindex, &secthead) == SUCCESS) {
	    // cout << "Section: " << secthead.s_name << "\tStart: " << secthead.s_vaddr 
	    // << "\tEnd: " << secthead.s_vaddr + secthead.s_size << endl;
	    sections_.push_back(new Dyn_Section(sectindex, secthead.s_name, secthead.s_vaddr, secthead.s_size));
	    if (!P_strcmp(secthead.s_name, ".text")) {
	      code_len_ = (Word) secthead.s_size;
	      Word *buffer = new Word[code_len_+1];
	      code_ptr_ = buffer;
	      code_off_ = (Address) secthead.s_vaddr;
	      if (!obj_read_section(secthead, ldptr, buffer)) {
		success = false;
		//bperr("failed text region\n");
		ldclose(ldptr);
		free(file);
		return;
	      }
	      text_read = true;
	    } else if (!P_strcmp(secthead.s_name, ".data")) {
	      if (secthead.s_vaddr && secthead.s_scnptr) {
		all_addr[K_D_INDEX] = secthead.s_vaddr;
		all_size[K_D_INDEX] = secthead.s_size;
		all_dex[K_D_INDEX] = true;
		all_disk[K_D_INDEX] = secthead.s_scnptr;
	      } else {
		//bperr("failed data region\n");
		success = false;
		ldclose(ldptr);
		free(file);
		return;
	      }
	    } else if (!P_strcmp(secthead.s_name, ".xdata")) {
	      if (secthead.s_vaddr && secthead.s_scnptr) {
		all_addr[K_XD_INDEX] = secthead.s_vaddr;
		all_size[K_XD_INDEX] = secthead.s_size;
		all_dex[K_XD_INDEX] = true;
		all_disk[K_XD_INDEX] = secthead.s_scnptr;
	      }
	    } else if (!P_strcmp(secthead.s_name, ".sdata")) {
	      if (secthead.s_vaddr && secthead.s_scnptr) {
		all_addr[K_SD_INDEX] = secthead.s_vaddr;
		all_size[K_SD_INDEX] = secthead.s_size;
		all_dex[K_SD_INDEX] = true;
		all_disk[K_SD_INDEX] = secthead.s_scnptr;
	      }
	    } else if (!P_strcmp(secthead.s_name, ".rdata")) {
	      if (secthead.s_vaddr && secthead.s_scnptr) {
		all_addr[K_RD_INDEX] = secthead.s_vaddr;
		all_size[K_RD_INDEX] = secthead.s_size;
		all_dex[K_RD_INDEX] = true;
		all_disk[K_RD_INDEX] = secthead.s_scnptr;
	      }
	    } else if (!P_strcmp(secthead.s_name, ".lit4")) {
	      if (secthead.s_vaddr && secthead.s_scnptr) {
		all_addr[K_L4_INDEX] = secthead.s_vaddr;
		all_size[K_L4_INDEX] = secthead.s_size;
		all_dex[K_L4_INDEX] = true;
		all_disk[K_L4_INDEX] = secthead.s_scnptr;
	      }
	    } else if (!P_strcmp(secthead.s_name, ".lita")) {
	      if (secthead.s_vaddr && secthead.s_scnptr) {
		all_addr[K_LA_INDEX] = secthead.s_vaddr;
		all_size[K_LA_INDEX] = secthead.s_size;
		all_dex[K_LA_INDEX] = true;
		all_disk[K_LA_INDEX] = secthead.s_scnptr;
	      }
	    } else if (!P_strcmp(secthead.s_name, ".rconst")) {
	      if (secthead.s_vaddr && secthead.s_scnptr) {
		all_addr[K_RC_INDEX] = secthead.s_vaddr;
		all_size[K_RC_INDEX] = secthead.s_size;
		all_dex[K_RC_INDEX] = true;
		all_disk[K_RC_INDEX] = secthead.s_scnptr;
	      }
	    } else if (!P_strcmp(secthead.s_name, ".lit8")) {
	      if (secthead.s_vaddr && secthead.s_scnptr) {
		all_addr[K_L8_INDEX] = secthead.s_vaddr;
		all_size[K_L8_INDEX] = secthead.s_size;
		all_dex[K_L8_INDEX] = true;
		all_disk[K_L8_INDEX] = secthead.s_scnptr;
	      }
	    } else if (!P_strncmp(secthead.s_name, ".dynamic", 8)) {
	      // a section .dynamic implies the program is dynamically linked
	      dynamicallyLinked = true; 
	    }
	  } else {
	    success = false;
	    //bperr("failed header region\n");
	    ldclose(ldptr);
	    free(file);
	    return;
	  }
	  sectindex++;
	}
	no_of_sections = sectindex;

	if (!text_read) { 
	  success = false;
	  //bperr("failed text region\n");
	  ldclose(ldptr);
	  free(file);
	  return;
	}

	// I am assuming that .data comes before all other data sections
	// I will include all other contiguous data sections
	// Determine the size of the data section(s)
	if (all_disk[K_D_INDEX]) {
	    if (!find_data_region(all_addr, all_size, all_disk,
                                  data_len_, data_off_)) {
	      success = false;
	      //bperr("failed find data region\n");
	      ldclose(ldptr);
	      free(file);
	      return;
	    }
	}

	// Now read in the data from the assorted data regions
	Word *buffer = new Word[data_len_+1];
	data_ptr_ = buffer;
	if (!read_data_region(all_addr, all_size, all_disk,
			      data_len_, data_off_, buffer, ldptr)) {
	  success = false;
          //bperr("failed read data region\n");
	  ldclose(ldptr);
	  free(file);
	  return;
	}

	// COFF doesn't have segments, so the entire code/data sections are valid
	code_vldS_ = code_off_;
	code_vldE_ = code_off_ + code_len_;
	data_vldS_ = data_off_;
	data_vldE_ = data_off_ + code_len_;

        // Check for the symbol table
	if (!(sym_tab_ptr = PSYMTAB(ldptr))) {
	    success = false;
            //bperr("failed check for symbol table - object may be strip'd!\n");
	    ldclose(ldptr);
	    free(file);
	    return;
	}

	// Read the symbol table
	sym_hdr = SYMHEADER(ldptr);
	if (sym_hdr.magic != magicSym) {
	    success = false;
            //bperr("failed check for magic symbol\n");
	    ldclose(ldptr);
	    free(file);
	    return;
        }
	if (!(sym_tab_ptr = SYMTAB(ldptr))) {
	    success = false;
            //bperr("failed read symbol table\n");
	    ldclose(ldptr);
	    free(file);
	    return;
	}
	if (LDSWAP(ldptr)) {
	  // These bytes are swapped
	  // supposedly libsex.a will unswap them
	  assert(0);
	}

	string module = "DEFAULT_MODULE";
   if (sharedLibrary) {
      module = file_;
      allSymbols.push_back(Symbol(module, module, Symbol::ST_MODULE, 
                                  Symbol::SL_GLOBAL, (Address) 0, false,));
	} else {
      module = "DEFAULT_MODULE";
	}

   string name = "DEFAULT_SYMBOL";
	int moduleEndIdx = -1;
	map<string, int> fcnNames;

	while (ldtbread(ldptr, index, &symbol) == SUCCESS) {
	  // TODO -- when global?
	  Symbol::SymbolLinkage linkage = Symbol::SL_GLOBAL;
	  Symbol::SymbolType type = Symbol::ST_UNKNOWN;
	  bool st_kludge = false;
	  bool sym_use = true;
	  unsigned secNumber;
	  char *name = ldgetname(ldptr, &symbol);
	  char prettyName[1024];

	switch(symbol.st) {
	case stProc:
	case stStaticProc:
		// Native C++ name demangling.
		MLD_demangle_string(name, prettyName, 1024,
				    MLD_SHOW_DEMANGLED_NAME | MLD_NO_SPACES);
		if (strchr(prettyName, '('))
		    *strchr(prettyName, '(') = 0;
		name = prettyName;

		if (symbol.sc == scText && (fcnNames.find(name)==fcnNames.end()))
		{
			type = Symbol::ST_FUNCTION;
			fcnNames[name] = 1;
			secNumber = findSecNumber(".text");
		}
		else 
			sym_use = false;
		break;

	case stGlobal:
	case stConstant:
	case stStatic:
		switch(symbol.sc) {
		case scData:
			secNumber = findSecNumber(".data");
			type = Symbol::ST_OBJECT;
			break;
		case scBss:
			secNumber = findSecNumber(".bss");
			type = Symbol::ST_OBJECT;
			break;
		case scSData:
		case scSBss:
		case scRData:
		case scRConst:
		case scTlsData:
		case scTlsBss:
			secNumber = findSecNumber(".sdata");
			type = Symbol::ST_OBJECT;
			break;
		default:
			sym_use = false;
		}
		break;

	case stLocal:
	case stParam:
		linkage = Symbol::SL_LOCAL;

		switch(symbol.sc) {
		case scAbs:
		case scRegister:
		case scVar:
		case scVarRegister:
		case scUnallocated:
			secNumber = findSecNumber(".rdata");
			type = Symbol::ST_OBJECT;
			break;
		case scData:
			secNumber = findSecNumber(".data");
			 //Parameter is static var. Don't know what to do
			if (symbol.st == stParam)
				type = Symbol::ST_OBJECT;
			else
				sym_use = false;
			break;
		case scSData:
		case scBss:
		case scSBss:
		case scRConst:
		case scRData:
			secNumber = findSecNumber(".rdata");
			 //Parameter is static var. Don't know what to do
			if (symbol.st == stParam)
				type = Symbol::ST_OBJECT;
			else
				sym_use = false;
			break;

		default:
			sym_use = false;
		}
		break;

	case stTypedef:
	case stBase: //Base class
	case stTag: //C++ class, structure or union
		secNumber = findSecNumber(".rdata");
		if (symbol.sc == scInfo)
			type = Symbol::ST_OBJECT;
		else
			sym_use = false;
		break;

	case stFile:	
		secNumber = findSecNumber(".rdata");
		if (!sharedLibrary) {
			module = ldgetname(ldptr, &symbol); assert(module.length());
			type   = Symbol::ST_MODULE;
			moduleEndIdx = symbol.index - 1;
			//Detect the compiler type by searching libgcc.
			if (strstr(module.c_str(), "libgcc"))
				GCC_COMPILED = true;
		}
		break;

	case stLabel:
		// For stLabel/scText combinations, if the symbol address falls
		// within the text section and has a valid name, process it as
		// a function.
		if (symbol.sc == scText &&
		    code_vldS_ <= (unsigned) symbol.value && (unsigned) symbol.value < code_vldE_ &&
		    name && *name && (fcnNames.find(name)=fcnNames.end())) {
			// Native C++ name demangling.
		        // Keep this in sync with stProc case above.
			
			secNumber = findSecNumber(".text");
			MLD_demangle_string(name, prettyName, 1024,
					    MLD_SHOW_DEMANGLED_NAME | MLD_NO_SPACES);
			if (strchr(prettyName, '('))
			    *strchr(prettyName, '(') = 0;
			name = prettyName;

			type = Symbol::ST_FUNCTION;
			fcnNames[name] = 1;

		} else {
			sym_use = false;
		}
		break;

	case stEnd:
		if (index == moduleEndIdx)
			module = "DEFAULT_MODULE";
		sym_use = false;
		break;

	default:
		sym_use = false;
	}

	// Skip eCoff encoded stab string.  Functions and global variable
	// addresses will still be found via the external symbols.
	if (P_strchr(name, ':'))
	    sym_use = false;
	
	// cout << index << "\t" << name << "\t" << StName(symbol.st) << "\t" << ScName(symbol.sc) << "\t" << symbol.value << "\n";

	  index++;

	  if (sym_use) {
	    // cout << index << "\t" << module << "\t" << name << "\t" << type << "\t" << symbol.value << "\n";
	    allSymbols.push_back(Symbol(name, module, type, linkage,
				      (Address) symbol.value, st_kludge,secNumber));
	  }

    } //while

    sort(allSymbols.begin(),allSymbols.end(),symbol_compare());
    // find the function boundaries
    nsymbols = allSymbols.size();
    no_of_symbols = nsymbols;

    //Insert global symbols
    for (unsigned u = 0; u < nsymbols; u++) {
	unsigned size = 0;
	if (allSymbols[u].type() == Symbol::ST_FUNCTION) {
	    unsigned v = u+1;
	    while (v < nsymbols) {
		// The .ef below is a special symbol that gcc puts in to
                // mark the end of a function.
                if (allSymbols[v].addr() != allSymbols[u].addr() &&
                      (allSymbols[v].type() == Symbol::ST_FUNCTION ||
                       allSymbols[v].name() == ".ef"))
                break;
                v++;
            }
	    if (v < nsymbols) {
                  size = (unsigned)allSymbols[v].addr()
                         - (unsigned)allSymbols[u].addr();
	    } else {
                size = (unsigned)(code_off_+code_len_)
                         - (unsigned)allSymbols[u].addr();
	    }
	}
	
	if (allSymbols[u].linkage() != Symbol::SL_LOCAL) {
		symbols_[allSymbols[u].name()].push_back( 
	   		Symbol(allSymbols[u].name(), allSymbols[u].module(), 
	      		allSymbols[u].type(), allSymbols[u].linkage(), 
	       		allSymbols[u].addr(), allSymbols[u].kludge(), allSymbols[u].snumber(), size) ); 
	}
    }

    //Insert local symbols (Do we need this?)
    for (unsigned u = 0; u < nsymbols; u++) {
	if ( (allSymbols[u].linkage() == Symbol::SL_LOCAL) &&
		(!symbols_.defines(allSymbols[u].name())) ) {
		symbols_[allSymbols[u].name()].push_back( 
	   		Symbol(allSymbols[u].name(), allSymbols[u].module(), 
	      		allSymbols[u].type(), allSymbols[u].linkage(), 
	       		allSymbols[u].addr(), allSymbols[u].kludge(), allSymbols[u].snumber(), 0) );
	}
    }
		
    if (did_open && (ldclose(ldptr) == FAILURE)) {
        log_perror(err_func_, "close");
    }
    free(file);

}


/*Object::Object(const string file, void (*err_func)(const char *))
    : AObject(file, err_func) {
    load_object(false);
}*/

/* 
 * Called to init a shared library.
 */
Object::Object (const string fileName, const Address /*BaseAddr*/,
        void (*err_func)(const char *))
  :AObject(fileName,err_func)
{

  load_object(true);
}

Object::Object(const fDescriptor &desc, void (*err_func)(const char *))
  : AObject(desc.file(), err_func) 
 {
     switch(desc.type)
     {
         case 1:			//Has a filename
     	 {
	     char* file = strdup(file_.c_str());
    	     did_open = false;
    	     ldptr = NULL;
	
       	     if (!(ldptr = ldopen(file, ldptr))) 
	     {
                 log_perror(err_func_, file);
	   	 success = false;
	    	 //bperr("failed open\n");
	    	 free(file);
	    	 return;
             }
             did_open = true;

	     if (TYPE(ldptr) != ALPHAMAGIC) 
	     {
	    	 //bperr( "%s is not an alpha executable\n", file);
	    	 success = false;
	    	 //bperr("failed magic region\n");
	    	 ldclose(ldptr);
	    	 free(file);
	    	 return;
             }

	     // Read the text and data sections
	     fhdr = HEADER(ldptr);
	     unsigned flags = fhdr.f_flags;
	     if(flags & F_EXEC )
	    	 load_object(false);
	     else
	    	 load_object(true);
       }
       case 2:			//mem image todo
       {
       }
       default:
       {	
       	     log_perror(err_func_, "ELF header");
	     return;
	     break;
    	}
   }    
 
   load_object(desc.isSharedObject());
}

Object::Object(const Object& obj)
    : AObject(obj) {
    load_object(false);
}

void Dyn_Symtab::getModuleLanguageInfo(hash_map<string, supportedLanguages> *)
{
}
