/************************************************************************
 * Object-coff.h: COFF object files.
 * Note - this is DEC OSF/1 coff which probably isn't the real thing
 *
 * Note - this probably isn't perfect
 *      - the symbol dictionary will only map one symbol table entry to each name
 *      - I am not determining if symbols are external
 *      - I may be ignoring some useful symbols from the symbol table
 *      - I am checking the dictionary to make sure no existing entry get clobbered
 *      - I am using mmap
 *      - All of these objects need some type of "destroy" method to munmap and
 *          free memory.  This method will be invoked explictly, not implicitly.
************************************************************************/





#if !defined(_Object_coff_h_)
#define _Object_coff_h_



#define LOG_WORD 2

/************************************************************************
 * header files.
************************************************************************/

#include <util/h/Dictionary.h>
//#include <util/h/Line.h>
#include <util/h/String.h>
#include <util/h/Symbol.h>
#include <util/h/Types.h>
#include <util/h/Vector.h>

extern "C" {
#include <a.out.h>
};

#include <filehdr.h>
#include <syms.h>
#include <ldfcn.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>





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

private:
    inline void    load_object ();
};

inline
Object::Object(const string file, void (*err_func)(const char *))
    : AObject(file, err_func) {
    load_object();
}

inline Object::Object (const string file, u_int BaseAddr, void (*err_func)(const char *) = log_msg)
  :AObject(file,err_func)
{


}

inline
Object::Object(const Object& obj)
    : AObject(obj) {
    load_object();
}

inline
Object::~Object() {
}

inline
Object&
Object::operator=(const Object& obj) {
    (void) AObject::operator=(obj);
    return *this;
}

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


inline
void
Object::load_object() {
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
	    if (!symbols_.defines(name)) {
	      // cout << "ok\n";
	      symbols_[name] = Symbol(name, module, type, linkage,
				      (Address) symbol.value, st_kludge);
	    } else
	      ; // cout << "Not ok\n";
	  }

	}
      }

    /* catch */
cleanup: {
        if (did_open && (ldclose(ldptr) == FAILURE)) {
            log_perror(err_func_, "close");
        }
    }
    free(file);
}





#endif /* !defined(_Object_bsd_h_) */
