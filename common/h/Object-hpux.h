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
 * Object-hpux.h: HPUX object files.
************************************************************************/





#if !defined(_Object_hpux_h_)
#define _Object_hpux_h_





/************************************************************************
 * header files.
************************************************************************/

#include <util/h/Dictionary.h>
#include <util/h/String.h>
#include <util/h/Symbol.h>
#include <util/h/Types.h>
#include <util/h/Vector.h>

extern "C" {
#include <a.out.h>
};

#include <fcntl.h>
#include <stab.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
extern "C" {
#include <sys/mman.h>
};
#include <sys/stat.h>

// Information of symbol debug table for the gcc compiled program

struct internal_nlist {
  unsigned long  n_strx;		/* index into string table of name */
  unsigned char  n_type;		/* type of symbol */
  unsigned char  n_other;		/* misc info (not used here) */
  unsigned short n_desc;		/* description field      */
  unsigned long  n_value;		/* value of symbol        */
};

// The n_type field is the symbol type, containing: 
// the following are the ones that I am using   /

#define N_UNDF	0	 /* Undefined symbol */
#define N_FUN   0x24     /* function   */
#define N_ENDM  0x62     /* end module */
#define N_ENTRY 0xa4     /* other entry point */
#define N_NBTEXT 0xf0
#define N_LBRAC 0xc0
#define N_RBRAC 0xe0

// Information of symbol debug table for the NATIVE cc compiled program


#define K_SRCFILE        0
#define K_FUNCTION       2

typedef unsigned int BITS;
typedef int KINDTYPE;
typedef unsigned int LANGTYPE;
typedef long  VTPOINTER;
typedef long  SLTPOINTER;
typedef long  ADDRESS;
typedef long  DNTTPOINTER;

struct  DNTT_SRCFILE {
/*0*/   BITS          extension: 1;   /* Always zero                  */
        KINDTYPE      kind:     10;   /* always K_SRCFILE             */
        LANGTYPE      language:  4;   /* Language type                */
        BITS          unused:   17;   /* 17 bits filler to the end... */ 
/*1*/   VTPOINTER     name;           /* Source/listing file name     */
/*2*/   SLTPOINTER    address;        /* code and text locations      */
};


struct  DNTT_FUNC {
/*0*/   BITS           extension: 1;   /* always zero                  */
        KINDTYPE       kind:     10;   /* K_FUNCTION, K_ENTRY,         */
                                       /* K_BLOCKDATA, or K_MEMFUNC    */
        BITS           public1:   1;   /* 1 => globally visible  (name */
	                               /* was public, but who cares? L.*/ 
        LANGTYPE       language:  4;   /* type of language             */
        BITS           level:     5;   /* nesting level (top level = 0)*/
        BITS           optimize:  2;   /* level of optimization        */
        BITS           varargs:   1;   /* ellipses.  Pascal/800 later  */
        BITS           info:      4;   /* lang-specific stuff; F_xxxx  */
#ifdef CPLUSPLUS
        BITS           inlined:   1;
        BITS           localloc:  1;   /* 0 at top, 1 at end of block  */
#ifdef TEMPLATES
        BITS           expansion: 1;   /* 1 = function expansion       */
        BITS           unused:    1;
#else /* TEMPLATES */
        BITS           unused:    2;
#endif /* TEMPLATES */
#else
        BITS           unused:    4;
#endif
/*1*/   VTPOINTER      name;           /* name of function             */
/*2*/   VTPOINTER      alias;          /* alternate name, if any       */
/*3*/   DNTTPOINTER    firstparam;     /* first FPARAM, if any         */
/*4*/   SLTPOINTER     address;        /* code and text locations      */
/*5*/   ADDRESS        entryaddr;      /* address of entry point       */
/*6*/   DNTTPOINTER    retval;         /* return type, if any          */
/*7*/   ADDRESS        lowaddr;        /* lowest address of function   */
/*8*/   ADDRESS        hiaddr;         /* highest address of function  */
};                                     /* nine words                   */



// Information of unwind table which is used to assist the stack walking
 
struct unwind_table_entry {
  unsigned int region_start;               
  unsigned int region_end;
  unsigned int Cannot_unwind               :  1;
  unsigned int Millicode                   :  1;
  unsigned int Millicode_save_sr0          :  1;
  unsigned int Region_description          :  2;
  unsigned int reserved1                   :  1;
  unsigned int Entry_SR                    :  1;
  unsigned int Entry_FR                    :  4; 
  unsigned int Entry_GR                    :  5; 
  unsigned int Args_stored                 :  1;
  unsigned int Variable_Frame              :  1;
  unsigned int Separate_Package_Body       :  1;
  unsigned int Frame_Extension_Millicode   :  1;
  unsigned int Stack_Overflow_Check        :  1;
  unsigned int Two_Instruction_SP_Increment:  1;
  unsigned int Ada_Region                  :  1;
  unsigned int reserved2                   :  4;
  unsigned int Save_SP                     :  1;
  unsigned int Save_RP                     :  1;
  unsigned int Save_MRP_in_frame           :  1;
  unsigned int extn_ptr_defined            :  1;
  unsigned int Cleanup_defined             :  1;
  unsigned int MPE_XL_interrupt_marker     :  1;
  unsigned int HP_UX_interrupt_marker      :  1;
  unsigned int Large_frame                 :  1;
  unsigned int reserved4                   :  2;
  unsigned int Total_frame_size            : 27;
};






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

    vector<unwind_table_entry> unwind;     

private:
    void    load_object ();

    static const struct som_exec_auxhdr*
        get_som_exec_auxhdr (const struct header *);
};

inline
Object::Object(const string file, void (*err_func)(const char *))
    : AObject(file, err_func) {
    load_object();
}

inline
Object::Object(const Object& obj)
    : AObject(obj) {
    load_object();
}

// for shared object files: not currently implemented
// this should call a load_sharedobject routine to parse the shared obj. file
inline
Object::Object(const string file,u_int,void (*err_func)(const char *))
    : AObject(file, err_func) {
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

inline
const struct som_exec_auxhdr*
Object::get_som_exec_auxhdr(const struct header* hdrp) {
    const struct aux_id* idp = (const struct aux_id *)
        ((const void *) ((const char *) hdrp + (hdrp->aux_header_location)));
    unsigned aux_seen = 0;
    while (aux_seen < hdrp->aux_header_size) {
        if (idp->type == HPUX_AUX_ID) {
            return (const struct som_exec_auxhdr *) ((const void *) idp);
        }
        unsigned skip = sizeof(struct aux_id) + idp->length;

        idp = (const struct aux_id *)
            ((const void *) ((const char *) idp + skip));
        aux_seen += skip;
    }
    return 0;
}


static int symbol_compare(const void *x, const void *y) {
    const Symbol *s1 = (const Symbol *)x;
    const Symbol *s2 = (const Symbol *)y;
    return (s1->addr() - s2->addr());
}



inline
void
Object::load_object() {
    const char* file = file_.string_of();
    struct stat st;
    int         fd  = -1;
    char*       ptr = 0;

    bool        did_open = false;

    /* try */ {
        if (((fd = open(file, O_RDONLY)) == -1) || (fstat(fd, &st) == -1)) {
            log_perror(err_func_, file);
            /* throw exception */ goto cleanup;
        }
        did_open = true;

        if ((ptr = (char *) P_mmap(0, st.st_size,
            PROT_READ, MAP_PRIVATE, fd, 0)) == (char *) -1) {
            log_perror(err_func_, "mmap");
            /* throw exception */ goto cleanup;
        }

        const struct header* hdrp = (struct header *) ((void*) ptr);
        const struct som_exec_auxhdr* som_hdrp = 0;
        if ((som_hdrp = get_som_exec_auxhdr(hdrp)) == 0) {
            log_perror(err_func_, "cannot locate HPUX_AUX_ID");
            /* throw exception */ goto cleanup;
        }

        unsigned file_type = hdrp->a_magic;
        if ((file_type != EXEC_MAGIC) && (file_type != SHARE_MAGIC)) {
            log_perror(err_func_, "application is not EXEC_MAGIC/SHARE_MAGIC");
            /* throw exception */ goto cleanup;
        }

        code_ptr_ = (Word *) ((void*) &ptr[som_hdrp->exec_tfile]);
        code_off_ = (unsigned) som_hdrp->exec_tmem;
        code_len_ = unsigned(som_hdrp->exec_tsize / sizeof(Word));

        data_ptr_ = (Word *) ((void*) &ptr[som_hdrp->exec_dfile]);
        data_off_ = (unsigned) som_hdrp->exec_dmem;
        data_len_ = unsigned(som_hdrp->exec_dsize / sizeof(Word));

	
	string        tempName;

        const struct symbol_dictionary_record* syms =
            (const struct symbol_dictionary_record *)
            ((const void*) &ptr[hdrp->symbol_location]);
        unsigned      nsyms  = hdrp->symbol_total;
        const char*   strs   = &ptr[hdrp->symbol_strings_location];
        string        module = "DEFAULT_MODULE";
        string        name   = "DEFAULT_SYMBOL";
	bool          underScore = false;
	bool          priProg = false;

	string        lastName = name;

	vector<Symbol> allsymbols;

        for (unsigned i = 0; i < nsyms; i++) {


            Symbol::SymbolType type = Symbol::PDST_UNKNOWN;
            Symbol::SymbolLinkage linkage =
                ((syms[i].symbol_scope == SS_UNIVERSAL)
                ? Symbol::SL_GLOBAL
                : Symbol::SL_LOCAL);
            unsigned value = syms[i].symbol_value;

            switch (syms[i].symbol_type) {
            case ST_DATA:
                type = Symbol::PDST_OBJECT;
                break;

            case ST_CODE:
		if ((syms[i].symbol_scope != SS_UNIVERSAL)) {
		    continue;
	        }
            case ST_ENTRY:
	    case ST_SEC_PROG:	
                type   = Symbol::PDST_FUNCTION;
		underScore = true;
                value  = (value >> 2) << 2; /* get rid of the lower-order 2 bits */
                break;

            case ST_MODULE: /* this may be a _pascal_ module !?! */
                module = string(&strs[syms[i].name.n_strx]);
                type   = Symbol::PDST_MODULE;
                break;

	    case ST_PRI_PROG:
	    	type  = Symbol::PDST_FUNCTION;
	    	if (syms[i].arg_reloc) {
	    	    priProg = true;
		    //underScore = true;    
	    	} /* else           XXX: is this necessay? 
 		       continue;         doesn't matter?        */
		value  = (value >> 2) << 2;
	    	break;
		
            default:
                continue;
            }

	    string name = string(&strs[syms[i].name.n_strx]);;  

	    if (priProg==true) { 
		priProg = false;
		name = "main";
	    }
	    // XXXX - Hack to make names match assumptions of symtab.C
	    if (underScore) {
		char temp[512];
                sprintf(temp, "_%s", name.string_of());
                name = string(temp);
		underScore = false;
	    }

            if (!symbols_.defines(name)
                || (symbols_[name].linkage() == Symbol::SL_LOCAL)) {
                allsymbols  += Symbol(name, module, type, linkage,
                                 value, false);
            }
          }


	  // Sort the symbols on address to find the function boundaries
          allsymbols.sort(symbol_compare);

          unsigned nsymbols = allsymbols.size();
          for (unsigned u = 0; u < nsymbols; u++) {
	    unsigned v = u+1;
	    while (v < nsymbols && allsymbols[v].addr() == allsymbols[u].addr())
              v++;
            unsigned size = 0;
            if (v < nsymbols)
              size = (unsigned)allsymbols[v].addr() - (unsigned)allsymbols[u].addr();

            symbols_[allsymbols[u].name()] =
               Symbol(allsymbols[u].name(), allsymbols[u].module(),
                      allsymbols[u].type(), allsymbols[u].linkage(),
                      allsymbols[u].addr(), allsymbols[u].kludge(),
                      size);
	    //cout << allsymbols[u] << " size: " <<size<< endl;
	   }      



	const struct space_dictionary_record *sdr;
	const struct subspace_dictionary_record *subdr;
 
	const char *ssdr;                             /* space        */
	char *gdb_string;                       /* GCC: subspace $GDB_STRING */
	struct internal_nlist *gdb_symbol;      /* GCC: subspace $GDB_SYMBOL */  
	char *vt;                               /* CC:  subspace $GNTT$, $VT$*/
	struct DNTT_SRCFILE *lntt;              /* CC:  subspace $LNTT$      */
	struct unwind_table_entry *unwind_start;/* unwind table  */  
	struct unwind_table_entry *unwind_end;  /* unwind table  */

	char sub[100];
	int index = 1;
	int j;
	int symbol_len, string_len;              
	int lntt_len, vt_len;
	int unwind_start_len, unwind_end_len;
	int cc_compiled = 0;

        sdr = (const struct space_dictionary_record *)
        ((const void *) ((const char *) hdrp + (hdrp->space_location)));

        ssdr = (const char *) hdrp + (hdrp->space_strings_location);


	/* Get the subspace infomation of the $GDB_STRINGS$, $GDB_SYMBOLS$,
	   $LNTT$, $VT$ for module names;
	   Get the subspace infomation of the $UNWIND_START$, $UNWIND_END$,
	   for unwind table infomation used for stack walking. */   

	for (j=0; j<hdrp->subspace_total; j++) {
	    subdr = (const struct subspace_dictionary_record *) 
	        ((const void *) ((const char *) hdrp + 
				 (hdrp->subspace_location)));
		
	    sprintf((char *)&sub, "%s", &((void *)ssdr)[subdr[j].name.n_strx]);

	    if (strncmp((char *)&sub, "$GDB_STRINGS$", 13) == 0) {
		gdb_string = &ptr[subdr[j].file_loc_init_value];
		string_len = subdr[j].subspace_length;
	    } else if (strncmp((char *)&sub, "$GDB_SYMBOLS$", 13) == 0)  { 
		gdb_symbol = (struct internal_nlist *)&ptr[subdr[j].file_loc_init_value];
		symbol_len = subdr[j].subspace_length;
	    } else if (strncmp((char *)&sub, "$LNTT$", 6) == 0) {
		lntt = (struct DNTT_SRCFILE *)&ptr[subdr[j].file_loc_init_value];
		lntt_len = subdr[j].subspace_length;
		cc_compiled = 1;
	    } else if (strncmp((char *)&sub, "$VT$", 4) == 0)  { 
		vt = &ptr[subdr[j].file_loc_init_value];
		vt_len = subdr[j].subspace_length;
	    } else if (strncmp((char *)&sub, "$UNWIND_START$", 14) == 0) {
		unwind_start = (struct unwind_table_entry *)
		    &ptr[subdr[j].file_loc_init_value];
		unwind_start_len = subdr[j].subspace_length;
	    }
	    else if (strncmp((char *)&sub, "$UNWIND_END$", 11) == 0)  { 
		unwind_end = (struct unwind_table_entry *)
		    &ptr[subdr[j].file_loc_init_value];
		unwind_end_len = subdr[j].subspace_length;
	    } 
	}	


	// For gcc compiled modules, get the module infomation
	// from the stab sections:    

	    module = "";
	    index = 1;
	    for (j=0; j < (symbol_len/(sizeof(struct internal_nlist))); j++)
	    {
	      Symbol::SymbolLinkage linkage = 0;
	      Symbol::SymbolType type = Symbol::PDST_MODULE;

	      if (gdb_symbol[j].n_type == 0)
	      {
		  module = gdb_string+index;
		  index += gdb_symbol[j].n_value;
		  
		  while ((gdb_symbol[j].n_type != N_FUN)&&
			 ++j < (symbol_len/(sizeof(struct internal_nlist))))
		  {
		      if (gdb_symbol[j].n_type == 0) {
			  module = gdb_string+index;
			  index += gdb_symbol[j].n_value;
		      }
		      else if (j >= (symbol_len/(sizeof(struct internal_nlist))))
			  break;
		  }
		  
		  unsigned valu = gdb_symbol[j].n_value;
		  symbols_[module] = Symbol(module, module, type, linkage,
					  valu, false);
		  //cout << "Module:" << module.string_of() << valu <<endl; 
	      }
	    }

	// For cc compiled modules, get the module infomation in the 
	// following way:
 
	    if (cc_compiled) {

	    char *srcFile;
	    char *ptr;
	    bool nextModule = false;

	    module = "";
	    for (j=0; j< (lntt_len/(3*sizeof(int))) ; j++) 
	    {

	      Symbol::SymbolLinkage linkage = 0;
	      Symbol::SymbolType type = Symbol::PDST_MODULE;

		if ((lntt[j].extension==0)&&((lntt[j].kind==K_SRCFILE)||
					     (lntt[j].kind==K_FUNCTION))) {
		    if ((lntt[j].name <= vt_len)&& (lntt[j].name >= 0))
		    {
			/* A source file of some kind       */
			if (lntt[j].kind==K_SRCFILE) 
			{
			    srcFile = lntt[j].name + vt;
			
			    ptr = strrchr (srcFile, '.');

			    /* but it could be simply an included file  */
			    if ((!ptr)||(strcmp (ptr, ".h")==0)||(strcmp (ptr, ".incl")==0))
				continue;
			    
			    /* or could be the same one repeated       */
			    if (module == srcFile)
				continue;

			    module = srcFile;
			    nextModule = true;
			    //printf("Module: %s", srcFile);
			}
			else 
			{
			    /* find the address of this module by looking at 
			       the address of the first function in this
			       module; if no function defined in this module,
			       ignore this module                            */

			    if (nextModule) {
				unsigned valu =
				    ((struct DNTT_FUNC *)(&lntt[j]))->lowaddr;
				symbols_[module] = Symbol(module, module, type, 
							  linkage, valu, false);
				//printf(" %d\n", valu);
				nextModule = false;
				}
			    }
			}
		    }
		}
	    }
    


	/* retrieve the entries in the unwind table  */ 
    
	for (j=0; j< (unwind_start_len/(sizeof(struct unwind_table_entry))) ; j++) 
	{
	    unwind += unwind_start[j]; 
	    //printf("{Start: 0x%x(%d); End: 0x%x(%d); Size: %d, Interrupt: %d}\n", 
	    //	   unwind_start[j].region_start, unwind_start[j].region_start,
	    //	   unwind_start[j].region_end, unwind_start[j].region_end,
	    //	   unwind_start[j].Total_frame_size,
	    //	   unwind_start[j].HP_UX_interrupt_marker);
	}

	//printf("\n-----------------------------\n");
        //for (j=0; j< (unwind_end_len/(sizeof(struct unwind_table_entry))) ; j++)
        //{
          //  unwind += unwind_end[j];
          //  printf("{Start: 0x%x(%d); End: 0x%x(%d); Size: %d, Interrupt: %d}\n",
          //         unwind_end[j].region_start, unwind_end[j].region_start,
          //         unwind_end[j].region_end, unwind_end[j].region_end,
          //         unwind_end[j].Total_frame_size,
          //         unwind_end[j].HP_UX_interrupt_marker);
        //}


    }
    /* catch */
cleanup: {
        if (did_open && (close(fd) == -1)) {
            log_perror(err_func_, "close");
        }
    }
}





#endif /* !defined(_Object_hpux_h_) */
