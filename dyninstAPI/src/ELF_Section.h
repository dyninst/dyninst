/* $Id: ELF_Section.h,v 1.4 2003/09/05 16:28:36 schendel Exp $ */

/* ccw 21 nov 2001 */

#if defined(sparc_sun_solaris2_4) || defined(i386_unknown_linux2_0) || defined(ia64_unknown_linux2_4)

#ifndef ELF_Section__
#define ELF_Section__
typedef struct {
	unsigned int vaddr;
	void *data;
	unsigned int dataSize;
	Elf32_Shdr *shdr;
	char *name;
	int nameIndx;
	unsigned int align;
	unsigned int flags;
	unsigned int type;
	bool loadable;
} ELF_Section;
//make this a class w/desctuctor


typedef struct {
      Elf32_Sword d_tag;
      union {
          Elf32_Sword d_val;
          Elf32_Addr d_ptr;
      } d_un;
  } __Elf32_Dyn;


#define DT_NULL   0
#define DT_NEEDED 1
#define DT_STRTAB 5
#define DT_PLTREL 20
#define DT_PLTGOT 3
#define DT_HASH 4
#define DT_SYMTAB 6
#define DT_RELA 7
#define DT_INIT 12
#define DT_FINI 13
#define DT_REL 17
#define DT_VERDEF 0x6ffffffc
#define DT_VERDEFNUM 0x6ffffffd
#define DT_VERNEED 0x6ffffffe
#define DT_VERNEEDNUM 0x6fffffff
#define DT_JMPREL 23
#define DT_STRSZ 10
#define DT_CHECKSUM 0x6ffffdf8
#define DT_DEBUG 21

#endif
#endif
