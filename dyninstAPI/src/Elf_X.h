#ifndef __ELF_X_H__
#define __ELF_X_H__

#include <elf.h>
#include <libelf.h>

// Wrappers to allow word-independant use of libelf routines.

#if defined(arch_ia64)
// ------------------------------------------------------------------------
// Class Elf_X_Dyn simulates the Elf(32|64)_Dyn structure.
class Elf_X_Dyn {
  public:
    Elf_X_Dyn()
	: data(NULL), dyn32(NULL), dyn64(NULL), is64(false) { }

    Elf_X_Dyn(bool is64_, Elf_Data *input)
	: data(input), dyn32(NULL), dyn64(NULL), is64(is64_) {

	if (input) {
	    if (!is64) dyn32 = (Elf32_Dyn *)data->d_buf;
	    else       dyn64 = (Elf64_Dyn *)data->d_buf;
	}
    }

    // Read Interface
    signed long d_tag(int i) const { return (!is64 ? dyn32[i].d_tag
						   : dyn64[i].d_tag); }
    unsigned long d_val(int i) const { return (!is64 ? dyn32[i].d_un.d_val
						     : dyn64[i].d_un.d_val); }
    unsigned long d_ptr(int i) const { return (!is64 ? dyn32[i].d_un.d_ptr
						     : dyn64[i].d_un.d_ptr); }

    // Write Interface
    void d_tag(int i, signed long input) { if (!is64) dyn32[i].d_tag = input;
					   else       dyn64[i].d_tag = input; }
    void d_val(int i, unsigned long input) { if (!is64) dyn32[i].d_un.d_val = input;
					     else       dyn64[i].d_un.d_val = input; }
    void d_ptr(int i, unsigned long input) { if (!is64) dyn32[i].d_un.d_ptr = input;
					     else       dyn64[i].d_un.d_ptr = input; }

    // Meta-Info Interface
    unsigned long count() const { return (data->d_size / (!is64 ? sizeof(Elf32_Dyn)
								: sizeof(Elf64_Dyn) )); }
    bool isValid() const { return (dyn32 || dyn64); }

  protected:
    Elf_Data *data;
    Elf32_Dyn *dyn32;
    Elf64_Dyn *dyn64;
    bool is64;
};
#endif

#if defined(arch_mips)
// ------------------------------------------------------------------------
// Class Elf_X_RegInfo simulates the Elf(32|64)_RegInfo structure.
class Elf_X_RegInfo {
  public:
    unsigned long ri_gprmask() const = 0;
    unsigned long ri_gprmask(unsigned long input) = 0;
    unsigned long ri_cprmask(int i) const = 0;
    unsigned long ri_cprmask(int i, unsigned long input) = 0;
    unsigned long ri_gp_value() const = 0;
    unsigned long ri_gp_value(unsigned long input) = 0;
};

class Elf_32_RegInfo : public Elf_X_RegInfo {
  public:
    Elf_32_RegInfo(Elf_Data *input) : reg((Elf32_RegInfo *)input.d_buf) { }

    // Read Interface
    unsigned long ri_gprmask() const { return reg->ri_gprmask; }
    unsigned long ri_cprmask(int i) const { return reg->ri_cprmask[i]; }
    unsigned long ri_gp_value() const { return reg->ri_gp_value; }

    // Write Interface
    unsigned long ri_gprmask(unsigned long input) { return (reg->ri_gprmask = input); }
    unsigned long ri_cprmask(int i, unsigned long input) { return (reg->ri_cprmask[i] = input); }
    unsigned long ri_gp_value(unsigned long input) { return (reg->ri_gp_value = input); }

  private:
    Elf32_RegInfo *reg;
};

class Elf_64_RegInfo : public Elf_X_RegInfo {
  public:
    Elf_64_RegInfo(Elf_Data *input) : reg((Elf64_RegInfo *)input.d_buf) { }

    // Read Interface
    unsigned long ri_gprmask() const { return reg->ri_gprmask; }
    unsigned long ri_cprmask(int i) const { return reg->ri_cprmask[i]; }
    unsigned long ri_gp_value() const { return reg->ri_gp_value; }

    // Write Interface
    unsigned long ri_gprmask(unsigned long input) { return (reg->ri_gprmask = input); }
    unsigned long ri_cprmask(int i, unsigned long input) { return (reg->ri_cprmask[i] = input); }
    unsigned long ri_gp_value(unsigned long input) { return (reg->ri_gp_value = input); }

  private:
    Elf64_RegInfo *reg;
};
#endif

// ------------------------------------------------------------------------
// Class Elf_X_Rel simulates the Elf(32|64)_Rel structure.
class Elf_X_Rel {
  public:
    Elf_X_Rel()
	: data(NULL), rel32(NULL), rel64(NULL), is64(false) { }

    Elf_X_Rel(bool is64_, Elf_Data *input)
	: data(input), rel32(NULL), rel64(NULL), is64(is64_) {

	if (input) {
	    if (!is64) rel32 = (Elf32_Rel *)data->d_buf;
	    else       rel64 = (Elf64_Rel *)data->d_buf;
	}
    }

    // Read Interface
    unsigned long r_offset(int i) const { return (!is64 ? rel32[i].r_offset
							: rel64[i].r_offset); }
    unsigned long r_info(int i) const { return (!is64 ? rel32[i].r_info
						      : rel64[i].r_info); }
    unsigned long R_SYM(int i) const { return (!is64 ? ELF32_R_SYM( rel32[i].r_info )
						     : ELF64_R_SYM( rel64[i].r_info ) ); }
    unsigned long R_TYPE(int i) const { return (!is64 ? ELF32_R_TYPE( rel32[i].r_info )
						      : ELF64_R_TYPE( rel64[i].r_info ) ); };

    // Write Interface
    void r_offset(int i, unsigned long input) { if (!is64) rel32[i].r_offset = input;
						else       rel64[i].r_offset = input; }
    void r_info(int i, unsigned long input) { if (!is64) rel32[i].r_info = input;
					      else       rel64[i].r_info = input; }

    // Meta-Info Interface
    unsigned long count() const { return (data->d_size / (!is64 ? sizeof(Elf32_Rel)
								: sizeof(Elf64_Rel)) ); }
    bool isValid() const { return (rel32 || rel64); }

  protected:
    Elf_Data *data;
    Elf32_Rel *rel32;
    Elf64_Rel *rel64;
    bool is64;
};

// ------------------------------------------------------------------------
// Class Elf_X_Rela simulates the Elf(32|64)_Rela structure.
class Elf_X_Rela {
  public:
    Elf_X_Rela()
	: data(NULL), rela32(NULL), rela64(NULL), is64(false) { }

    Elf_X_Rela(bool is64_, Elf_Data *input)
	: data(input), rela32(NULL), rela64(NULL), is64(is64_) {

	if (input) {
	    if (!is64) rela32 = (Elf32_Rela *)data->d_buf;
	    else       rela64 = (Elf64_Rela *)data->d_buf;
	}
    }

    // Read Interface
    unsigned long r_offset(int i) const { return (!is64 ? rela32[i].r_offset
							: rela64[i].r_offset); }
    unsigned long r_info(int i) const { return (!is64 ? rela32[i].r_info
						      : rela64[i].r_info); }
    signed long r_addend(int i) const { return (!is64 ? rela32[i].r_addend
						      : rela64[i].r_addend); }
    unsigned long R_SYM(int i) const { return (!is64 ? ELF32_R_SYM( rela32[i].r_info )
						     : ELF64_R_SYM( rela64[i].r_info ) ); }
    unsigned long R_TYPE(int i) const { return (!is64 ? ELF32_R_TYPE( rela32[i].r_info )
						      : ELF64_R_TYPE( rela64[i].r_info ) ); };

    // Write Interface
    void r_offset(int i, unsigned long input) { if (!is64) rela32[i].r_offset = input;
						else       rela64[i].r_offset = input; }
    void r_info(int i, unsigned long input) { if (!is64) rela32[i].r_info = input;
					      else       rela64[i].r_info = input; }
    void r_addend(int i, signed long input) { if (!is64) rela32[i].r_addend = input;
					      else       rela64[i].r_addend = input; }

    // Meta-Info Interface
    unsigned long count() const { return (data->d_size / (!is64 ? sizeof(Elf32_Rela)
								: sizeof(Elf64_Rela))); }
    bool isValid() const { return (rela32 || rela64); }

  protected:
    Elf_Data *data;
    Elf32_Rela *rela32;
    Elf64_Rela *rela64;
    bool is64;
};

// ------------------------------------------------------------------------
// Class Elf_X_Sym simulates the Elf(32|64)_Sym structure.
class Elf_X_Sym {
  public:
    Elf_X_Sym()
	: data(NULL), sym32(NULL), sym64(NULL), is64(false) { }

    Elf_X_Sym(bool is64_, Elf_Data *input)
	: data(input), sym32(NULL), sym64(NULL), is64(is64_) {

	if (input) {
	    if (!is64) sym32 = (Elf32_Sym *)data->d_buf;
	    else       sym64 = (Elf64_Sym *)data->d_buf;
	}
    }

    // Read Interface
    unsigned long st_name(int i) const { return (!is64 ? sym32[i].st_name
						       : sym64[i].st_name); }
    unsigned long st_value(int i) const { return (!is64 ? sym32[i].st_value
							: sym64[i].st_value); }
    unsigned long st_size(int i) const { return (!is64 ? sym32[i].st_size
						       : sym64[i].st_size); }
    unsigned char st_info(int i) const { return (!is64 ? sym32[i].st_info
						       : sym64[i].st_info); }
    unsigned char st_other(int i) const { return (!is64 ? sym32[i].st_other
							: sym64[i].st_other); }
    unsigned short st_shndx(int i) const { return (!is64 ? sym32[i].st_shndx
							 : sym64[i].st_shndx); }
    unsigned char ST_BIND(int i) const { return (!is64 ? ELF32_ST_BIND( sym32[i].st_info )
						       : ELF64_ST_BIND( sym64[i].st_info )); }
    unsigned char ST_TYPE(int i) const { return (!is64 ? ELF32_ST_TYPE( sym32[i].st_info )
						       : ELF64_ST_TYPE( sym64[i].st_info )); }

    // Write Interface
    void st_name(int i, unsigned long input) { if (!is64) sym32[i].st_name = input;
					       else       sym64[i].st_name = input; }
    void st_value(int i, unsigned long input) { if (!is64) sym32[i].st_value = input;
						else       sym64[i].st_value = input; }
    void st_size(int i, unsigned long input) { if (!is64) sym32[i].st_size = input;
					       else       sym64[i].st_size = input; }
    void st_info(int i, unsigned char input) { if (!is64) sym32[i].st_info = input;
					       else       sym64[i].st_info = input; }
    void st_other(int i, unsigned char input) { if (!is64) sym32[i].st_other = input;
						else       sym64[i].st_other = input; }
    void st_shndx(int i, unsigned short input) { if (!is64) sym32[i].st_shndx = input;
						 else       sym64[i].st_shndx = input; }

    // Meta-Info Interface
    unsigned long count() const { return (data->d_size / (!is64 ? sizeof(Elf32_Sym)
								: sizeof(Elf64_Sym))); }
    bool isValid() const { return data; }

  protected:
    Elf_Data *data;
    Elf32_Sym *sym32;
    Elf64_Sym *sym64;
    bool is64;
};

#if defined(arch_mips)
// ------------------------------------------------------------------------
// Class Elf_X_Options simulates the Elf_Options structure.
class Elf_X_Options {
  public:
    Elf_X_Options(Elf_Data *input) : data(input), opt((Elf_Options *)data->d_buf) { }

    // Read Interface
    unsigned char kind(int i) const { return opt[i].kind; }
    unsigned char size(int i) const { return opt[i].size; }
    unsigned short section(int i) const { return opt[i].section; }
    unsigned long info(int i) const { return opt[i].info; }

    // Write Interface
    unsigned char kind(int i, unsigned char input) { return (opt[i].kind = input); }
    unsigned char size(int i, unsigned char input) { return (opt[i].size = input); }
    unsigned short section(int i, unsigned short input) { return (opt[i].section = input); }
    unsigned long info(int i, unsigned long input) { return (opt[i].info = input); }

    // Meta-Info Interface
    unsigned long count() const { return (data->d_size / sizeof(Elf_Options)); }

  private:
    Elf_Data *data;
    Elf_Options *opt;
};
#endif

// ------------------------------------------------------------------------
// Class Elf_X_Data simulates the Elf_Data structure.
class Elf_X_Data {
  public:
    Elf_X_Data()
	: data(NULL), is64(false) { }

    Elf_X_Data(bool is64_, Elf_Data *input)
	: data(input), is64(is64_) { }

    // Read Interface
    void *d_buf() const { return data->d_buf; }
    Elf_Type d_type() const { return data->d_type; }
    unsigned int d_version() const { return data->d_version; }
    size_t d_size() const { return data->d_size; }
    off_t d_off() const { return data->d_off; }
    size_t d_align() const { return data->d_align; }

    // Write Interface
    void d_buf(void *input) { data->d_buf = input; }
    void d_type(Elf_Type input) { data->d_type = input; }
    void d_version(unsigned int input) { data->d_version = input; }
    void d_size(unsigned int input) { data->d_size = input; }
    void d_off(signed int input) { data->d_off = input; }
    void d_align(unsigned int input) { data->d_align = input; }

    // Data Interface
    const char *get_string() { return (const char *)data->d_buf; }
#if defined(arch_ia64)
    Elf_X_Dyn get_dyn() { return Elf_X_Dyn(is64, data); }
#endif
#if defined(arch_mips)
    Elf_X_RegInfo get_regInfo() { return Elf_X_RegInfo(is64, data); }
#endif
    Elf_X_Rel get_rel() { return Elf_X_Rel(is64, data); }
    Elf_X_Rela get_rela() { return Elf_X_Rela(is64, data); }
    Elf_X_Sym get_sym() { return Elf_X_Sym(is64, data); }
#if defined(arch_mips)
    Elf_X_Options get_options() { return Elf_X_Options(is64, data); }
#endif

    bool isValid() { return data; }

  protected:
    Elf_Data *data;
    bool is64;
};

// ------------------------------------------------------------------------
// Class Elf_X_Shdr simulates the Elf(32|64)_Shdr structure.
class Elf_X_Shdr {
  public:
    Elf_X_Shdr()
	: scn(NULL), shdr32(NULL), shdr64(NULL), is64(false) { }

    Elf_X_Shdr(bool is64_, Elf_Scn *input)
	: scn(input), shdr32(NULL), shdr64(NULL), is64(is64_) {

	if (input) {
	    first_data();
	    if (!is64) shdr32 = elf32_getshdr(scn);
	    else       shdr64 = elf64_getshdr(scn);
	}
    }

    // Read Interface
    unsigned long sh_name() const { return (!is64 ? shdr32->sh_name
						  : shdr64->sh_name); }
    unsigned long sh_type() const { return (!is64 ? shdr32->sh_type
						  : shdr64->sh_type); }
    unsigned long sh_flags() const { return (!is64 ? shdr32->sh_flags
						   : shdr64->sh_flags); }
    unsigned long sh_addr() const { return (!is64 ? shdr32->sh_addr
						  : shdr64->sh_addr); }
    unsigned long sh_offset() const { return (!is64 ? shdr32->sh_offset
						    : shdr64->sh_offset); }
    unsigned long sh_size() const { return (!is64 ? shdr32->sh_size
						  : shdr64->sh_size); }
    unsigned long sh_link() const { return (!is64 ? shdr32->sh_link
						  : shdr64->sh_link); }
    unsigned long sh_info() const { return (!is64 ? shdr32->sh_info
						  : shdr64->sh_info); }
    unsigned long sh_addralign() const { return (!is64 ? shdr32->sh_addralign
						       : shdr64->sh_addralign); }
    unsigned long sh_entsize() const { return (!is64 ? shdr32->sh_entsize
						     : shdr64->sh_entsize); }

    // Write Interface
    void sh_name(unsigned long input) { if (!is64) shdr32->sh_name = input;
					else       shdr64->sh_name = input; }
    void sh_type(unsigned long input) { if (!is64) shdr32->sh_type = input;
					else       shdr64->sh_type = input; }
    void sh_flags(unsigned long input) { if (!is64) shdr32->sh_flags = input;
					 else       shdr64->sh_flags = input; }
    void sh_addr(unsigned long input) { if (!is64) shdr32->sh_flags = input;
					else       shdr64->sh_flags = input; }
    void sh_offset(unsigned long input) { if (!is64) shdr32->sh_offset = input;
					  else       shdr64->sh_offset = input; }
    void sh_size(unsigned long input) { if (!is64) shdr32->sh_size = input;
					else       shdr64->sh_size = input; }
    void sh_link(unsigned long input) { if (!is64) shdr32->sh_link = input;
					else       shdr64->sh_link = input; }
    void sh_info(unsigned long input) { if (!is64) shdr32->sh_info = input;
					else       shdr64->sh_info = input; }
    void sh_addralign(unsigned long input) { if (!is64) shdr32->sh_addralign = input;
					     else       shdr64->sh_addralign = input; }
    void sh_entsize(unsigned long input) { if (!is64) shdr32->sh_entsize = input;
					   else       shdr64->sh_entsize = input; }

    // Section Data Interface
    Elf_X_Data get_data() const { return Elf_X_Data(is64, data); }

    // For Sections with Multiple Data Sections
    void first_data() { data = elf_getdata(scn, NULL); }
    bool next_data() {
	Elf_Data *nextData = elf_getdata(scn, data);
	if (nextData) data = nextData;
	return nextData;
    }

    bool isValid() const { return (shdr32 || shdr64); }

  protected:
    Elf_Scn *scn;
    Elf_Data *data;
    Elf32_Shdr *shdr32;
    Elf64_Shdr *shdr64;
    bool is64;
};

// ------------------------------------------------------------------------
// Class Elf_X_Phdr simulates the Elf(32|64)_Phdr structure.
class Elf_X_Phdr {
  public:
    Elf_X_Phdr()
	: phdr32(NULL), phdr64(NULL), is64(false) { }

    Elf_X_Phdr(bool is64_, void *input)
	: phdr32(NULL), phdr64(NULL), is64(is64_) {

	if (input) {
	    if (!is64) phdr32 = (Elf32_Phdr *)input;
	    else       phdr64 = (Elf64_Phdr *)input;
	}
    }

    // Read Interface
    unsigned long p_type() const { return (!is64 ? phdr32->p_type
						 : phdr64->p_type); }
    unsigned long p_offset() const { return (!is64 ? phdr32->p_offset
						   : phdr64->p_offset); }
    unsigned long p_vaddr() const { return (!is64 ? phdr32->p_vaddr
						  : phdr64->p_vaddr); }
    unsigned long p_paddr() const { return (!is64 ? phdr32->p_paddr
						  : phdr64->p_paddr); }
    unsigned long p_filesz() const { return (!is64 ? phdr32->p_filesz
						   : phdr64->p_filesz); }
    unsigned long p_memsz() const { return (!is64 ? phdr32->p_memsz
						  : phdr64->p_memsz); }
    unsigned long p_flags() const { return (!is64 ? phdr32->p_flags
						  : phdr64->p_flags); }
    unsigned long p_align() const { return (!is64 ? phdr32->p_align
						  : phdr64->p_align); }

    // Write Interface
    void p_type(unsigned long input) { if (!is64) phdr32->p_type = input;
				       else       phdr64->p_type = input; }
    void p_offset(unsigned long input) { if (!is64) phdr32->p_offset = input;
					 else       phdr64->p_offset = input; }
    void p_vaddr(unsigned long input) { if (!is64) phdr32->p_vaddr = input;
					else       phdr64->p_vaddr = input; }
    void p_paddr(unsigned long input) { if (!is64) phdr32->p_paddr = input;
					else       phdr64->p_paddr = input; }
    void p_filesz(unsigned long input) { if (!is64) phdr32->p_filesz = input;
					 else       phdr64->p_filesz = input; }
    void p_memsz(unsigned long input) { if (!is64) phdr32->p_memsz = input;
					else       phdr64->p_memsz = input; }
    void p_flags(unsigned long input) { if (!is64) phdr32->p_flags = input;
					else       phdr64->p_flags = input; }
    void p_align(unsigned long input) { if (!is64) phdr32->p_align = input;
					else       phdr64->p_align = input; }

    bool isValid() const { return (phdr32 || phdr64); }

  private:
    Elf32_Phdr *phdr32;
    Elf64_Phdr *phdr64;
    bool is64;
};

// ------------------------------------------------------------------------
// Class Elf_X simulates the Elf(32|64)_Ehdr structure.
class Elf_X {
  public:
    Elf_X()
	: elf(NULL), ehdr32(NULL), ehdr64(NULL), phdr32(NULL), phdr64(NULL) { }

    Elf_X(int input, Elf_Cmd cmd)
	: ehdr32(NULL), ehdr64(NULL), phdr32(NULL), phdr64(NULL) {

	if (elf_version(EV_CURRENT) != EV_NONE) {
	    elf_errno(); // Reset elf_errno to zero.
	    elf = elf_begin(input, cmd, NULL);
	    if (elf) {
		if (elf_kind(elf) == ELF_K_ELF) {
		    char *identp = elf_getident(elf, NULL);
		    is64 = (identp && identp[EI_CLASS] == ELFCLASS64);
		}

		if (!is64) ehdr32 = elf32_getehdr(elf);
		else	   ehdr64 = elf64_getehdr(elf);

		if (!is64) phdr32 = elf32_getphdr(elf);
		else	   phdr64 = elf64_getphdr(elf);
	    }
	}
    }

    void end() {
	if (elf) {
	    elf_end(elf);
	    elf = NULL;
	    ehdr32 = NULL;
	    ehdr64 = NULL;
	    phdr32 = NULL;
	    phdr64 = NULL;
	}
    }

    // Read Interface
    Elf *e_elfp() const { return elf; }
    unsigned char *e_ident() const { return (!is64 ? ehdr32->e_ident
						   : ehdr64->e_ident); }
    unsigned short e_type() const { return (!is64 ? ehdr32->e_type
						  : ehdr64->e_type); }
    unsigned short e_machine() const { return (!is64 ? ehdr32->e_machine
						     : ehdr64->e_machine); }
    unsigned long e_version() const { return (!is64 ? ehdr32->e_version
						    : ehdr64->e_version); }
    unsigned long e_entry() const { return (!is64 ? ehdr32->e_entry
						  : ehdr64->e_entry); }
    unsigned long e_phoff() const { return (!is64 ? ehdr32->e_phoff
						  : ehdr64->e_phoff); }
    unsigned long e_shoff() const { return (!is64 ? ehdr32->e_shoff
						  : ehdr64->e_shoff); }
    unsigned long e_flags() const { return (!is64 ? ehdr32->e_flags
						  : ehdr64->e_flags); }
    unsigned short e_ehsize() const { return (!is64 ? ehdr32->e_ehsize
						    : ehdr64->e_ehsize); }
    unsigned short e_phentsize() const { return (!is64 ? ehdr32->e_phentsize
						       : ehdr64->e_phentsize); }
    unsigned short e_phnum() const { return (!is64 ? ehdr32->e_phnum
						   : ehdr64->e_phnum); }
    unsigned short e_shentsize() const { return (!is64 ? ehdr32->e_shentsize
						       : ehdr64->e_shentsize); }
    unsigned short e_shnum() const { return (!is64 ? ehdr32->e_shnum
						   : ehdr64->e_shnum); }
    unsigned short e_shstrndx() const { return (!is64 ? ehdr32->e_shstrndx
						      : ehdr64->e_shstrndx); }

    // Write Interface
    void e_ident(unsigned char *input) { if (!is64) P_memcpy(ehdr32->e_ident, input, EI_NIDENT);
					 else       P_memcpy(ehdr64->e_ident, input, EI_NIDENT); }
    void e_type(unsigned short input) { if (!is64) ehdr32->e_type = input;
					else       ehdr64->e_type = input; }
    void e_machine(unsigned short input) { if (!is64) ehdr32->e_machine = input;
    					   else       ehdr64->e_machine = input; }
    void e_version(unsigned long input) { if (!is64) ehdr32->e_version = input;
					  else       ehdr64->e_version = input; }
    void e_entry(unsigned long input) { if (!is64) ehdr32->e_entry = input;
					else       ehdr64->e_entry = input; }
    void e_phoff(unsigned long input) { if (!is64) ehdr32->e_phoff = input;
					else       ehdr64->e_phoff = input; }
    void e_shoff(unsigned long input) { if (!is64) ehdr32->e_shoff = input;
					else       ehdr64->e_shoff = input; }
    void e_flags(unsigned long input) { if (!is64) ehdr32->e_flags = input;
					else       ehdr64->e_flags = input; }
    void e_ehsize(unsigned short input) { if (!is64) ehdr32->e_ehsize = input;
					  else       ehdr64->e_ehsize = input; }
    void e_phentsize(unsigned short input) { if (!is64) ehdr32->e_phentsize = input;
					     else       ehdr64->e_phentsize = input; }
    void e_phnum(unsigned short input) { if (!is64) ehdr32->e_phnum = input;
					 else       ehdr64->e_phnum = input; }
    void e_shentsize(unsigned short input) { if (!is64) ehdr32->e_shentsize = input;
					     else       ehdr64->e_shentsize = input; }
    void e_shnum(unsigned short input) { if (!is64) ehdr32->e_shnum = input;
					 else       ehdr64->e_shnum = input; }
    void e_shstrndx(unsigned short input) { if (!is64) ehdr32->e_shstrndx = input;
					    else       ehdr64->e_shstrndx = input; }

    // Data Interface
    bool isValid() const { return (ehdr32 || ehdr64); }
    int wordSize() const { return (!is64 ? 4 : 8); }
    Elf_X_Phdr get_phdr(unsigned int i = 0) const {
	if (!is64) return Elf_X_Phdr(is64, phdr32 + i);
	else       return Elf_X_Phdr(is64, phdr64 + i);
    }
    Elf_X_Shdr get_shdr(unsigned int i) const {
	Elf_Scn *scn = elf_getscn(elf, i);
	return Elf_X_Shdr(is64, scn);
    }

  protected:
    Elf *elf;
    Elf32_Ehdr *ehdr32;
    Elf64_Ehdr *ehdr64;
    Elf32_Phdr *phdr32;
    Elf64_Phdr *phdr64;
    bool is64;
};

#endif
