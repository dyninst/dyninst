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

// $Id: Object-coff.C,v 1.1 1999/01/20 22:21:53 hollings Exp $

#include "util/h/Object.h"
#include "util/h/Object-coff.h"

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
  
  data_len = (max_adr - min_adr) / sizeof(Word);
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
  Address max_adr = data_off + data_len * sizeof(Word);
  assert(all_size.size() == all_addr.size());
  assert(all_disk.size() == all_addr.size());
  for (index=0; index<max; index++) {
    if ((all_addr[index] >= data_off) &&
	((all_addr[index] + all_size[index]) <= max_adr)) {
      if (ldfseek(ldptr, all_disk[index], SEEK_SET) == -1) return false;
      Word *buf_temp = buffer + ((all_addr[index] - data_off) / sizeof(Word));
      if (ldfread((void*) buf_temp, 1, all_size[index], ldptr) != all_size[index])
	return false;
    }
  }
  return true;
}

void Object::load_object() {
    char* file = strdup(file_.string_of());
    bool        did_open = false, success=true, text_read=false;
    LDFILE      *ldptr = NULL;
    HDRR        sym_hdr;
    pCHDRR      sym_tab_ptr = NULL;
    long        index=0;
    SYMR        symbol;
    unsigned short sectindex=1;
    SCNHDR      secthead;
    filehdr     fhdr;
    unsigned 	nsymbols;
    vector<Symbol> allSymbols;
    vector<Address> all_addr(9, 0);
    vector<long> all_size(9, 0);
    vector<bool> all_dex(9, false);
    vector<long> all_disk(9, 0);

    /* try */ {
        if (!(ldptr = ldopen(file, ldptr))) {
            log_perror(err_func_, file);
	    success = false;
            /* throw exception */ goto cleanup;
        }
        did_open = true;

	if (TYPE(ldptr) != ALPHAMAGIC) {
	    log_printf(err_func_, "%s is not an alpha executable\n", file);
	    success = false;
	    /* throw exception */ goto cleanup;
        }

	// Read the text and data sections
	fhdr = HEADER(ldptr);
	unsigned short fmax = fhdr.f_nscns;

	// Life would be so much easier if there were a guaranteed order for
	// these sections.  But the man page makes no mention of it.
	while (sectindex < fmax) {
	  if (ldshread(ldptr, sectindex, &secthead) == SUCCESS) {
	    // cout << "Section: " << secthead.s_name << "\tStart: " << secthead.s_vaddr 
	    // << "\tEnd: " << secthead.s_vaddr + secthead.s_size << endl;
	    if (!P_strcmp(secthead.s_name, ".text")) {
	      code_len_ = Word(secthead.s_size >> LOG_WORD);
	      Word *buffer = new Word[code_len_+1];
	      code_ptr_ = buffer;
	      code_off_ = (Address) secthead.s_vaddr;
	      if (!obj_read_section(secthead, ldptr, buffer)) {
		success = false;
		/* throw exception */ goto cleanup;
	      }
	      text_read = true;
	    } else if (!P_strcmp(secthead.s_name, ".data")) {
	      if (secthead.s_vaddr && secthead.s_scnptr) {
		all_addr[K_D_INDEX] = secthead.s_vaddr;
		all_size[K_D_INDEX] = secthead.s_size;
		all_dex[K_D_INDEX] = true;
		all_disk[K_D_INDEX] = secthead.s_scnptr;
	      } else {
		success = false;
		/* throw exception */ goto cleanup;
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
	    }
	  } else {
	    success = false;
	    /* throw exception */ goto cleanup;
	  }
	  sectindex++;
	}

	if (!text_read || !all_disk[K_D_INDEX]) {
	  success = false;
	  /* throw exception */ goto cleanup;
	}

	// I am assuming that .data comes before all other data sections
	// I will include all other contiguous data sections
	// Determine the size of the data section(s)
	if (!find_data_region(all_addr, all_size, all_disk,data_len_,data_off_)) {
	  success = false;
	  /* throw exception */ goto cleanup;
	}

	// Now read in the data from the assorted data regions
	Word *buffer = new Word[data_len_+1];
	data_ptr_ = buffer;
	if (!read_data_region(all_addr, all_size, all_disk,
			      data_len_, data_off_, buffer, ldptr)) {
	  success = false;
	  /* throw exception */ goto cleanup;
	}

	// Read the symbol table
	sym_hdr = SYMHEADER(ldptr);
	if (sym_hdr.magic != magicSym) {
	    success = false;
	  /* throw exception */ goto cleanup;
        }
	if (!(sym_tab_ptr = SYMTAB(ldptr))) {
	    success = false;
	  /* throw exception */ goto cleanup;
	}
	if (LDSWAP(ldptr)) {
	  // These bytes are swapped
	  // supposedly libsex.a will unswap them
	  assert(0);
	}

        string        module = "DEFAULT_MODULE";
        string        name   = "DEFAULT_SYMBOL";

	while (ldtbread(ldptr, index, &symbol) == SUCCESS) {
	  // TODO -- when global?
	  Symbol::SymbolLinkage linkage = Symbol::SL_GLOBAL;
	  Symbol::SymbolType type = Symbol::PDST_UNKNOWN;
	  bool st_kludge = false;
	  bool sym_use = true;

	  switch (symbol.sc) {
	  case scText:

	    switch (symbol.st) {
	    case stProc:
	      // cout << "Procedure: " << sym_name << "\tValue: "
	      // << symbol.value << endl;
	      type = Symbol::PDST_FUNCTION;
	      break;
	    case stGlobal:
	    case stStatic:
	    case stLocal:
	    case stConstant:
	      type = Symbol::PDST_OBJECT;
	      break;
	    default:
	      sym_use = false;
	    }
	    break;
	  case scData:
	  case scSData:
	  case scRData:
	  case scRConst:
	  case scXData:
	  case scBss:
	  case scSBss:
	    // cout << "Data: " << sym_name << "\tType:" << symbol.st 
	    // << "\tValue: " << symbol.value << endl;
	    type = Symbol::PDST_OBJECT;
	    switch (symbol.st) {
	    case stGlobal:
	    case stStatic:
	    case stLocal:
	    case stConstant:
	      break;
	    default:
	      sym_use = false;
	    }
	    break;
	  default:
	    switch (symbol.st) {
	    case stFile:
	      // cout << "File: " << sym_name << "\tValue:" << symbol.value << endl;
	      module = ldgetname(ldptr, &symbol); assert(module.length());
	      type   = Symbol::PDST_MODULE;
	      break;
	    default:
	      // cout << "Other: " << sym_name <<  "\tType:" << symbol.st 
	      // << "\tValue: " << symbol.value << endl;
	      sym_use = false;
	    }
	    break;
	  }
	  index++;

	  if (sym_use) {
	    char *name = ldgetname(ldptr, &symbol);
	    // cout << "Define: " << name << "\tat: " << symbol.value;
	    // if (!allSymbols.defines(name)) {
	      // cout << "ok\n";
	      allSymbols += Symbol(name, module, type, linkage,
				      (Address) symbol.value, st_kludge);
	    // } else
	      ; // cout << "Not ok\n";
	  }

	}
      }

    allSymbols.sort(symbol_compare);
    // find the function boundaries
    nsymbols = allSymbols.size();

    for (unsigned u = 0; u < nsymbols; u++) {
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
	    if (v < nsymbols) {
                  size = (unsigned)allSymbols[v].addr()
                         - (unsigned)allSymbols[u].addr();
	    } else {
                  size = (unsigned)(code_off_+code_len_*sizeof(Word))
                         - (unsigned)allSymbols[u].addr();
	    }
	}
	symbols_[allSymbols[u].name()] =
	       Symbol(allSymbols[u].name(), allSymbols[u].module(), 
	       allSymbols[u].type(), allSymbols[u].linkage(), 
	       allSymbols[u].addr(), allSymbols[u].kludge(), size);
    }

    /* catch */
cleanup: {
        if (did_open && (ldclose(ldptr) == FAILURE)) {
            log_perror(err_func_, "close");
        }
    }
    free(file);
}
