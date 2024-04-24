.. _`sec:Elf_X.h`:

Elf_X.h
#######

.. cpp:namespace:: Dyninst

Wrappers to allow word-independant use of libelf routines.


.. cpp:class:: Elf_X

  Elf_X simulates the Elf(32|64)_Ehdr structure. Also works for ELF archives.

  .. cpp:function:: static Elf_X *newElf_X(int input, Elf_Cmd cmd, Elf_X *ref = NULL, std::string name = std::string())
  .. cpp:function:: static Elf_X *newElf_X(char *mem_image, size_t mem_size, std::string name = std::string())
  .. cpp:function:: void end()

  ......

  .. rubric::
    Read Interface

  .. cpp:function:: Elf *e_elfp() const
  .. cpp:function:: unsigned char *e_ident() const
  .. cpp:function:: unsigned short e_type() const
  .. cpp:function:: unsigned short e_machine() const
  .. cpp:function:: unsigned long e_version() const
  .. cpp:function:: unsigned long e_entry() const
  .. cpp:function:: unsigned long e_phoff() const
  .. cpp:function:: unsigned long e_shoff() const
  .. cpp:function:: unsigned long e_flags() const
  .. cpp:function:: unsigned short e_ehsize() const
  .. cpp:function:: unsigned short e_phentsize() const
  .. cpp:function:: unsigned long e_phnum()
  .. cpp:function:: unsigned short e_shentsize() const
  .. cpp:function:: unsigned long e_shnum()
  .. cpp:function:: unsigned long e_shstrndx()
  .. cpp:function:: const char *e_rawfile(size_t &nbytes) const
  .. cpp:function:: unsigned short e_endian() const
  .. cpp:function:: Elf_X *e_next(Elf_X *ref)
  .. cpp:function:: Elf_X *e_rand(unsigned offset)

  ......

  .. rubric::
    Write Interface    

  .. cpp:function:: void e_ident(unsigned char *input)
  .. cpp:function:: void e_type(unsigned short input)
  .. cpp:function:: void e_machine(unsigned short input)
  .. cpp:function:: void e_version(unsigned long input)
  .. cpp:function:: void e_entry(unsigned long input)
  .. cpp:function:: void e_phoff(unsigned long input)
  .. cpp:function:: void e_shoff(unsigned long input)
  .. cpp:function:: void e_flags(unsigned long input)
  .. cpp:function:: void e_ehsize(unsigned short input)
  .. cpp:function:: void e_phentsize(unsigned short input)
  .. cpp:function:: void e_phnum(unsigned short input)
  .. cpp:function:: void e_shentsize(unsigned short input)
  .. cpp:function:: void e_shnum(unsigned short input)
  .. cpp:function:: void e_shstrndx(unsigned short input)
  .. cpp:function:: void e_endian(unsigned short input)

  ......

  .. rubric::
    Data Interface    

  .. cpp:function:: bool isValid() const
  .. cpp:function:: int wordSize() const
  .. cpp:function:: Elf_X_Phdr &get_phdr(unsigned int i = 0)
  .. cpp:function:: Elf_X_Shdr &get_shdr(unsigned int i)
  .. cpp:function:: bool findDebugFile(std::string origfilename, std::string &output_name, char *&output_buffer, unsigned long &output_buffer_size)
  .. cpp:function:: Dyninst::Architecture getArch() const
  .. cpp:member:: protected Elf *elf{}
  .. cpp:member:: protected Elf32_Ehdr *ehdr32{}
  .. cpp:member:: protected Elf64_Ehdr *ehdr64{}
  .. cpp:member:: protected Elf32_Phdr *phdr32{}
  .. cpp:member:: protected Elf64_Phdr *phdr64{}
  .. cpp:member:: protected int filedes{}
  .. cpp:member:: protected bool is64{}
  .. cpp:member:: protected bool isArchive{}
  .. cpp:member:: protected bool isBigEndian{}
  .. cpp:member:: protected std::vector<Elf_X_Shdr> shdrs
  .. cpp:member:: protected std::vector<Elf_X_Phdr> phdrs
  .. cpp:member:: protected unsigned int ref_count{}
  .. cpp:member:: protected std::string filename
  .. cpp:member:: protected char *cached_debug_buffer{}
  .. cpp:member:: protected unsigned long cached_debug_size{}
  .. cpp:member:: protected std::string cached_debug_name
  .. cpp:member:: protected bool cached_debug{}
  .. cpp:function:: protected Elf_X()
  .. cpp:function:: protected Elf_X(int input, Elf_Cmd cmd, Elf_X *ref = NULL)
  .. cpp:function:: protected Elf_X(char *mem_image, size_t mem_size)
  .. cpp:function:: protected ~Elf_X()

  .. cpp:member:: protected static std::map<std::pair<std::string, int>, Elf_X *> elf_x_by_fd

    One nameFD for Elf_Xs created that way

  .. cpp:member:: protected static std::map<std::pair<std::string, char *>, Elf_X *> elf_x_by_ptr

    One name/baseaddr

.. cpp:class:: Elf_X_Phdr


    Class Elf_X_Phdr simulates the Elf(32|64)_Phdr structure.

  .. cpp:function:: Elf_X_Phdr()
  .. cpp:function:: Elf_X_Phdr(bool is64_, void *input)

  ......

  .. rubric::
    Read Interface

  .. cpp:function:: unsigned long p_type() const
  .. cpp:function:: unsigned long p_offset() const
  .. cpp:function:: unsigned long p_vaddr() const
  .. cpp:function:: unsigned long p_paddr() const
  .. cpp:function:: unsigned long p_filesz() const
  .. cpp:function:: unsigned long p_memsz() const
  .. cpp:function:: unsigned long p_flags() const
  .. cpp:function:: unsigned long p_align() const

  ......

  .. rubric::
    Write Interface

  .. cpp:function:: void p_type(unsigned long input)
  .. cpp:function:: void p_offset(unsigned long input)
  .. cpp:function:: void p_vaddr(unsigned long input)
  .. cpp:function:: void p_paddr(unsigned long input)
  .. cpp:function:: void p_filesz(unsigned long input)
  .. cpp:function:: void p_memsz(unsigned long input)
  .. cpp:function:: void p_flags(unsigned long input)
  .. cpp:function:: void p_align(unsigned long input)
  .. cpp:function:: bool isValid() const
  .. cpp:member:: private Elf32_Phdr *phdr32
  .. cpp:member:: private Elf64_Phdr *phdr64
  .. cpp:member:: private bool is64

.. cpp:class:: Elf_X_Shdr

  Elf_X_Shdr simulates the Elf(32|64)_Shdr structure.

  .. cpp:function:: Elf_X_Shdr()
  .. cpp:function:: Elf_X_Shdr(bool is64_, Elf_Scn *input)

  ......

  .. rubric::
    Read Interface

  .. cpp:function:: unsigned long sh_name() const
  .. cpp:function:: unsigned long sh_type() const
  .. cpp:function:: unsigned long sh_flags() const
  .. cpp:function:: unsigned long sh_addr() const
  .. cpp:function:: unsigned long sh_offset() const
  .. cpp:function:: unsigned long sh_size() const
  .. cpp:function:: unsigned long sh_link() const
  .. cpp:function:: unsigned long sh_info() const
  .. cpp:function:: unsigned long sh_addralign() const
  .. cpp:function:: unsigned long sh_entsize() const
  .. cpp:function:: bool isFromDebugFile() const

  ......

  .. rubric::
    Write Interface

  .. cpp:function:: void sh_name(unsigned long input)
  .. cpp:function:: void sh_type(unsigned long input)
  .. cpp:function:: void sh_flags(unsigned long input)
  .. cpp:function:: void sh_addr(unsigned long input)
  .. cpp:function:: void sh_offset(unsigned long input)
  .. cpp:function:: void sh_size(unsigned long input)
  .. cpp:function:: void sh_link(unsigned long input)
  .. cpp:function:: void sh_info(unsigned long input)
  .. cpp:function:: void sh_addralign(unsigned long input)
  .. cpp:function:: void sh_entsize(unsigned long input)
  .. cpp:function:: void setDebugFile(bool b)

  ......

  .. rubric::
    Section Data Interface

  .. cpp:function:: Elf_X_Data get_data() const

  ......

  .. rubric::
    For sections with multiple data sections

  .. cpp:function:: void first_data()
  .. cpp:function:: bool next_data()

  ......

  .. cpp:function:: bool isValid() const
  .. cpp:function:: unsigned wordSize() const
  .. cpp:function:: Elf_Scn *getScn() const
  .. cpp:function:: Elf_X_Nhdr get_note() const
  .. cpp:member:: protected Elf_Scn *scn
  .. cpp:member:: protected Elf_Data *data
  .. cpp:member:: protected Elf32_Shdr *shdr32
  .. cpp:member:: protected Elf64_Shdr *shdr64
  .. cpp:member:: protected bool is64
  .. cpp:member:: protected bool fromDebugFile
  .. cpp:member:: protected const Elf_X *_elf

.. cpp:class:: Elf_X_Data

  Elf_X_Data simulates the Elf_Data structure.

  .. cpp:function:: Elf_X_Data()
  .. cpp:function:: Elf_X_Data(bool is64_, Elf_Data *input)

  ......

  .. rubric::
    Read Interface
    
  .. cpp:function:: void *d_buf() const
  .. cpp:function:: Elf_Data *elf_data() const
  .. cpp:function:: Elf_Type d_type() const
  .. cpp:function:: unsigned int d_version() const
  .. cpp:function:: size_t d_size() const
  .. cpp:function:: off_t d_off() const
  .. cpp:function:: size_t d_align() const
  .. cpp:function:: void xlatetom(unsigned int encode)
  .. cpp:function:: void xlatetof(unsigned int encode)

  ......

  .. rubric::
    Write Interface

  .. cpp:function:: void d_buf(void *input)
  .. cpp:function:: void d_type(Elf_Type input)
  .. cpp:function:: void d_version(unsigned int input)
  .. cpp:function:: void d_size(unsigned int input)
  .. cpp:function:: void d_off(signed int input)
  .. cpp:function:: void d_align(unsigned int input)

  ......

  .. rubric::
    Data Interface

  .. cpp:function:: const char *get_string() const
  .. cpp:function:: Elf_X_Dyn get_dyn()
  .. cpp:function:: Elf_X_Versym get_versyms()
  .. cpp:function:: Elf_X_Verneed *get_verNeedSym()
  .. cpp:function:: Elf_X_Verdef *get_verDefSym()

  ......

  .. cpp:function:: Elf_X_Rel get_rel()
  .. cpp:function:: Elf_X_Rela get_rela()
  .. cpp:function:: Elf_X_Sym get_sym()
  .. cpp:function:: bool isValid() const
  .. cpp:member:: protected Elf_Data *data
  .. cpp:member:: protected bool is64

.. cpp:class:: Elf_X_Versym


    Class Elf_X_Versym simulates the SHT_GNU_versym structure.

  .. cpp:function:: Elf_X_Versym()
  .. cpp:function:: Elf_X_Versym(bool is64_, Elf_Data *input)

  ......

  .. rubric::
    Read Interface

  .. cpp:function:: unsigned long get(int i) const

  ......

  .. rubric::
    Meta-Info Interface

  .. cpp:function:: unsigned long count() const
  .. cpp:function:: bool isValid() const

  ......

  .. cpp:member:: protected Elf_Data *data
  .. cpp:member:: protected Elf32_Half *versym32
  .. cpp:member:: protected Elf64_Half *versym64
  .. cpp:member:: protected bool is64

.. cpp:class:: Elf_X_Verdaux
  
  Elf_X_Verdaux simulates the Elf(32|64)_Verdaux structure.

  .. cpp:function:: Elf_X_Verdaux()
  .. cpp:function:: Elf_X_Verdaux(bool is64_, void *input)
  .. cpp:function:: unsigned long vda_name() const

    Read Interface

  .. cpp:function:: unsigned long vda_next() const
  .. cpp:function:: Elf_X_Verdaux *get_next() const
  .. cpp:function:: bool isValid() const

    Meta-Info Interface

  .. cpp:member:: protected void *data
  .. cpp:member:: protected Elf32_Verdaux *verdaux32
  .. cpp:member:: protected Elf64_Verdaux *verdaux64
  .. cpp:member:: protected bool is64

.. cpp:class:: Elf_X_Verdef

  Elf_X_Verdef simulates the Elf(32|64)_Verdef structure.

  .. cpp:function:: Elf_X_Verdef()
  .. cpp:function:: Elf_X_Verdef(bool is64_, void *input)

  ......

  .. rubric::
    Read Interface

  .. cpp:function:: unsigned long vd_version() const
  .. cpp:function:: unsigned long vd_flags() const
  .. cpp:function:: unsigned long vd_ndx() const
  .. cpp:function:: unsigned long vd_cnt() const
  .. cpp:function:: unsigned long vd_hash() const
  .. cpp:function:: unsigned long vd_aux() const
  .. cpp:function:: unsigned long vd_next() const
  .. cpp:function:: Elf_X_Verdaux *get_aux() const
  .. cpp:function:: Elf_X_Verdef *get_next() const

  ......

  .. rubric::
    Meta-info Interface

  .. cpp:function:: bool isValid() const

  ......

  .. cpp:member:: protected void *data
  .. cpp:member:: protected Elf32_Verdef *verdef32
  .. cpp:member:: protected Elf64_Verdef *verdef64
  .. cpp:member:: protected bool is64

.. cpp:class:: Elf_X_Vernaux

  Elf_X_Vernaux simulates the Elf(32|64)_Vernaux structure.

  .. cpp:function:: Elf_X_Vernaux()
  .. cpp:function:: Elf_X_Vernaux(bool is64_, void *input)

  ......

  .. rubric::
    Read Interface

  .. cpp:function:: unsigned long vna_hash() const
  .. cpp:function:: unsigned long vna_flags() const
  .. cpp:function:: unsigned long vna_other() const
  .. cpp:function:: unsigned long vna_name() const
  .. cpp:function:: unsigned long vna_next() const
  .. cpp:function:: Elf_X_Vernaux *get_next() const

  ......

  .. rubric::
    Meta-Info Interface

  .. cpp:function:: bool isValid() const

  ......

  .. cpp:member:: protected void *data
  .. cpp:member:: protected Elf32_Vernaux *vernaux32
  .. cpp:member:: protected Elf64_Vernaux *vernaux64
  .. cpp:member:: protected bool is64

.. cpp:class:: Elf_X_Verneed

  Elf_X_Verneed simulates the Elf(32|64)_Verneed structure.

  .. cpp:function:: Elf_X_Verneed()
  .. cpp:function:: Elf_X_Verneed(bool is64_, void *input)

  ......

  .. rubric::
    Read Interface

  .. cpp:function:: unsigned long vn_version() const
  .. cpp:function:: unsigned long vn_cnt() const
  .. cpp:function:: unsigned long vn_file() const
  .. cpp:function:: unsigned long vn_aux() const
  .. cpp:function:: unsigned long vn_next() const
  .. cpp:function:: Elf_X_Vernaux *get_aux() const
  .. cpp:function:: Elf_X_Verneed *get_next() const

  ......

  .. rubric::
    Meta-Info Interface

  .. cpp:function:: bool isValid() const

  ......

  .. cpp:member:: protected void *data
  .. cpp:member:: protected Elf32_Verneed *verneed32
  .. cpp:member:: protected Elf64_Verneed *verneed64
  .. cpp:member:: protected bool is64

.. cpp:class:: Elf_X_Sym

  Elf_X_Sym simulates the Elf(32|64)_Sym structure.

  .. cpp:function:: Elf_X_Sym()
  .. cpp:function:: Elf_X_Sym(bool is64_, Elf_Data *input)

  ......

  .. rubric::
    Read Interface

  .. cpp:function:: unsigned long st_name(int i) const
  .. cpp:function:: unsigned long st_value(int i) const
  .. cpp:function:: unsigned long st_size(int i) const
  .. cpp:function:: unsigned char st_info(int i) const
  .. cpp:function:: unsigned char st_other(int i) const
  .. cpp:function:: unsigned short st_shndx(int i) const
  .. cpp:function:: unsigned char ST_BIND(int i) const
  .. cpp:function:: unsigned char ST_TYPE(int i) const
  .. cpp:function:: unsigned char ST_VISIBILITY(int i) const
  .. cpp:function:: void *st_symptr(int i) const
  .. cpp:function:: unsigned st_entsize() const

  ......

  .. rubric::
    Write Interface

  .. cpp:function:: void st_name(int i, unsigned long input)
  .. cpp:function:: void st_value(int i, unsigned long input)
  .. cpp:function:: void st_size(int i, unsigned long input)
  .. cpp:function:: void st_info(int i, unsigned char input)
  .. cpp:function:: void st_other(int i, unsigned char input)
  .. cpp:function:: void st_shndx(int i, unsigned short input)

  ......

  .. rubric::
    Meta-Info Interface

  .. cpp:function:: unsigned long count() const
  .. cpp:function:: bool isValid() const

  ......

  .. cpp:member:: protected Elf_Data *data
  .. cpp:member:: protected Elf32_Sym *sym32
  .. cpp:member:: protected Elf64_Sym *sym64
  .. cpp:member:: protected bool is64

.. cpp:class:: Elf_X_Rel

  Elf_X_Rel simulates the Elf(32|64)_Rel structure.

  .. cpp:function:: Elf_X_Rel()
  .. cpp:function:: Elf_X_Rel(bool is64_, Elf_Data *input)

  ......

  .. rubric::
    Read Interface
    
  .. cpp:function:: unsigned long r_offset(int i) const
  .. cpp:function:: unsigned long r_info(int i) const
  .. cpp:function:: unsigned long R_SYM(int i) const
  .. cpp:function:: unsigned long R_TYPE(int i) const

  ......

  .. rubric::
    Write Interface

  .. cpp:function:: void r_offset(int i, unsigned long input)
  .. cpp:function:: void r_info(int i, unsigned long input)

  ......

  .. rubric::
    Meta-info Interface

  .. cpp:function:: unsigned long count() const
  .. cpp:function:: bool isValid() const

  ......

  .. cpp:member:: protected Elf_Data *data
  .. cpp:member:: protected Elf32_Rel *rel32
  .. cpp:member:: protected Elf64_Rel *rel64
  .. cpp:member:: protected bool is64

.. cpp:class:: Elf_X_Rela

  Elf_X_Rela simulates the Elf(32|64)_Rela structure.

  .. cpp:function:: Elf_X_Rela()
  .. cpp:function:: Elf_X_Rela(bool is64_, Elf_Data *input)

  ......

  .. rubric::
    Read Interface

  .. cpp:function:: unsigned long r_offset(int i) const
  .. cpp:function:: unsigned long r_info(int i) const
  .. cpp:function:: signed long r_addend(int i) const
  .. cpp:function:: unsigned long R_SYM(int i) const
  .. cpp:function:: unsigned long R_TYPE(int i) const

  ......

  .. rubric::
    Write Interface

  .. cpp:function:: void r_offset(int i, unsigned long input)
  .. cpp:function:: void r_info(int i, unsigned long input)
  .. cpp:function:: void r_addend(int i, signed long input)

  ......

  .. rubric::
    Meta-Info Interface

  .. cpp:function:: unsigned long count() const
  .. cpp:function:: bool isValid() const

  ......

  .. cpp:member:: protected Elf_Data *data
  .. cpp:member:: protected Elf32_Rela *rela32
  .. cpp:member:: protected Elf64_Rela *rela64
  .. cpp:member:: protected bool is64

.. cpp:class:: Elf_X_Dyn

  Elf_X_Dyn simulates the Elf(32|64)_Dyn structure.

  .. cpp:function:: Elf_X_Dyn()
  .. cpp:function:: Elf_X_Dyn(bool is64_, Elf_Data *input)

  ......

  .. rubric::
    Read Interface

  .. cpp:function:: signed long d_tag(int i) const
  .. cpp:function:: unsigned long d_val(int i) const
  .. cpp:function:: unsigned long d_ptr(int i) const

  ......

  .. rubric::
    Write Interface

  .. cpp:function:: void d_tag(int i, signed long input)
  .. cpp:function:: void d_val(int i, unsigned long input)
  .. cpp:function:: void d_ptr(int i, unsigned long input)

  ......

  .. rubric::
    Meta-Info Interface

  .. cpp:function:: unsigned long count() const
  .. cpp:function:: bool isValid() const

  .....

  .. cpp:member:: protected Elf_Data *data
  .. cpp:member:: protected Elf32_Dyn *dyn32
  .. cpp:member:: protected Elf64_Dyn *dyn64
  .. cpp:member:: protected bool is64

.. cpp:class:: Elf_X_Nhdr

  Class Elf_X_Nhdr simulates the Elf(32|64)_Shdr structure.

  .. cpp:function:: Elf_X_Nhdr()
  .. cpp:function:: Elf_X_Nhdr(Elf_Data *data_, size_t offset)

  ......

  .. rubric::
    Read Interface

  .. cpp:function:: unsigned long n_namesz() const
  .. cpp:function:: unsigned long n_descsz() const
  .. cpp:function:: unsigned long n_type() const

  ......

  .. rubric::
    Meta-Info Interface

  .. cpp:function:: bool isValid() const
  .. cpp:function:: const char *get_name() const
  .. cpp:function:: const void *get_desc() const
  .. cpp:function:: Elf_X_Nhdr next() const

  ......

  .. cpp:member:: protected Elf_Data *data
  .. cpp:member:: protected Elf32_Nhdr *nhdr


.. code:: cpp

  #ifndef EM_CUDA
  # define EM_CUDA   190 /* NVIDIA CUDA */
  #endif

  #ifndef EM_INTEL_GEN9
  # define EM_INTEL_GEN9   182 /* INTEL GEN9 */
  #endif

  #ifndef EM_INTELGT
  # define EM_INTELGT    205 /* INTEL Graphics Technology */
  #endif

