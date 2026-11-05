// Rewrites an executable whose DT_RELA and DT_JMPREL name the same section and
// checks the relocation ranges of the result (#2585).
//
// A riscv64 executable linked -no-pie whose only dynamic relocations are its
// PLT ones has this layout.  The rewrite follows BinaryEdit::writeFile: a new
// .dyninstInst section holding one reference to a function in another
// library, then Symtab::emit.  The new relocation goes in a new dynamic
// relocation section, which must not take the PLT section's name, and whose
// DT_RELASZ must be its own size.  DT_JMPREL and DT_PLTRELSZ must still cover
// the PLT relocations.
//
//   rela_jmprel_shared <executable> <library> <function> <reloc type> <output>
//
// The files are read and written, never run, so any host can check them.  The
// result is read with libelf rather than SymtabAPI.

#include "Region.h"
#include "Symbol.h"
#include "Symtab.h"

#include <cstdlib>
#include <fcntl.h>
#include <gelf.h>
#include <iostream>
#include <map>
#include <string>
#include <unistd.h>
#include <vector>

namespace st = Dyninst::SymtabAPI;

namespace {

struct RelaSection {
  std::string name;
  GElf_Xword size;
  std::vector<GElf_Addr> offsets;  // r_offset of each relocation
};

// The dynamic tags and the SHT_RELA sections, keyed by address, of an ELF file.
bool read_elf(char const *path, std::map<GElf_Sxword, GElf_Xword> &tags,
              std::map<GElf_Addr, RelaSection> &relas)
{
  int fd{open(path, O_RDONLY)};
  if (fd < 0)  {
    std::cerr << "Unable to open '" << path << "'\n";
    return false;
  }
  elf_version(EV_CURRENT);
  Elf *elf{elf_begin(fd, ELF_C_READ, nullptr)};
  size_t shstrndx{};
  if (!elf || elf_getshdrstrndx(elf, &shstrndx) != 0)  {
    std::cerr << "'" << path << "' is not a readable ELF file\n";
    close(fd);
    return false;
  }
  for (Elf_Scn *scn = elf_nextscn(elf, nullptr); scn; scn = elf_nextscn(elf, scn))  {
    GElf_Shdr shdr{};
    gelf_getshdr(scn, &shdr);
    Elf_Data *data{elf_getdata(scn, nullptr)};
    if (shdr.sh_type == SHT_DYNAMIC && data)  {
      size_t n{shdr.sh_size / shdr.sh_entsize};
      for (size_t i = 0; i < n; i++)  {
        GElf_Dyn dyn{};
        gelf_getdyn(data, i, &dyn);
        if (!tags.count(dyn.d_tag))  {
          tags[dyn.d_tag] = dyn.d_un.d_val;
        }
      }
    }  else if (shdr.sh_type == SHT_RELA && (shdr.sh_flags & SHF_ALLOC) && data)  {
      RelaSection &r = relas[shdr.sh_addr];
      r.name = elf_strptr(elf, shstrndx, shdr.sh_name);
      r.size = shdr.sh_size;
      size_t n{shdr.sh_size / shdr.sh_entsize};
      for (size_t i = 0; i < n; i++)  {
        GElf_Rela rela{};
        gelf_getrela(data, i, &rela);
        r.offsets.push_back(rela.r_offset);
      }
    }
  }
  elf_end(elf);
  close(fd);
  return true;
}

// The SHT_RELA section a tag names, or null.  Says what is wrong on stderr.
RelaSection const *tagged_section(std::map<GElf_Sxword, GElf_Xword> const &tags,
                                  std::map<GElf_Addr, RelaSection> const &relas,
                                  GElf_Sxword tag, char const *tag_name)
{
  auto t = tags.find(tag);
  if (t == tags.end())  {
    std::cerr << "no " << tag_name << '\n';
    return nullptr;
  }
  auto r = relas.find(t->second);
  if (r == relas.end())  {
    std::cerr << tag_name << " 0x" << std::hex << t->second << std::dec
              << " is not the address of a relocation section\n";
    return nullptr;
  }
  return &r->second;
}

}

int main(int argc, char **argv)
{
  if (argc != 6)  {
    std::cerr << "Usage: " << argv[0]
              << " <executable> <library> <function> <reloc type> <output>\n";
    return EXIT_FAILURE;
  }
  char const *const out_path{argv[5]};

  // The layout this is about.
  std::map<GElf_Sxword, GElf_Xword> in_tags;
  std::map<GElf_Addr, RelaSection> in_relas;
  if (!read_elf(argv[1], in_tags, in_relas))  {
    return EXIT_FAILURE;
  }
  if (!in_tags.count(DT_RELA) || in_tags[DT_RELA] != in_tags[DT_JMPREL])  {
    std::cerr << argv[1] << ": DT_RELA and DT_JMPREL differ; not the layout to test\n";
    return EXIT_FAILURE;
  }
  GElf_Xword const plt_relocs{in_tags[DT_PLTRELSZ]};

  st::Symtab *exe{};
  st::Symtab *lib{};
  if (!st::Symtab::openFile(exe, argv[1]) || !st::Symtab::openFile(lib, argv[2]))  {
    std::cerr << "Unable to open the inputs\n";
    return EXIT_FAILURE;
  }
  std::vector<st::Symbol *> syms;
  if (!lib->findSymbol(syms, argv[3]) || syms.empty())  {
    std::cerr << argv[2] << ": no symbol " << argv[3] << '\n';
    return EXIT_FAILURE;
  }

  static char code[16]{};
  exe->addRegion(exe->getFreeOffset(sizeof code), code, sizeof code, ".dyninstInst",
                 st::Region::RT_TEXTDATA, true);
  st::Region *inst{};
  if (!exe->findRegion(inst, ".dyninstInst"))  {
    std::cerr << "The new section was not added\n";
    return EXIT_FAILURE;
  }
  Dyninst::Offset const ref_addr{inst->getMemOffset()};
  unsigned long const rel_type{std::strtoul(argv[4], nullptr, 0)};
  st::relocationEntry rel(ref_addr, syms[0]->getMangledName(), syms[0], rel_type);
  if (!exe->addExternalSymbolReference(syms[0], inst, rel))  {
    std::cerr << "addExternalSymbolReference failed\n";
    return EXIT_FAILURE;
  }
  if (!exe->emit(out_path))  {
    std::cerr << "Unable to write '" << out_path << "'\n";
    return EXIT_FAILURE;
  }

  std::map<GElf_Sxword, GElf_Xword> tags;
  std::map<GElf_Addr, RelaSection> relas;
  if (!read_elf(out_path, tags, relas))  {
    return EXIT_FAILURE;
  }

  int failures{0};
  RelaSection const *dyn{tagged_section(tags, relas, DT_RELA, "DT_RELA")};
  if (!dyn)  {
    failures++;
  }  else  {
    if (tags[DT_RELASZ] != dyn->size)  {
      std::cerr << "DT_RELASZ is " << tags[DT_RELASZ] << ", but the section DT_RELA "
                << "names (" << dyn->name << ") is " << dyn->size << " bytes\n";
      failures++;
    }
    bool has_ref{false};
    for (auto off : dyn->offsets)  {
      if (off == ref_addr)  {
        has_ref = true;
      }
    }
    if (!has_ref)  {
      std::cerr << "the section DT_RELA names (" << dyn->name
                << ") lacks the added relocation\n";
      failures++;
    }
    if (dyn->name == ".rela.plt")  {
      std::cerr << "the section DT_RELA names is called .rela.plt\n";
      failures++;
    }
  }

  RelaSection const *plt{tagged_section(tags, relas, DT_JMPREL, "DT_JMPREL")};
  if (!plt)  {
    failures++;
  }  else  {
    if (tags[DT_PLTRELSZ] != plt->size)  {
      std::cerr << "DT_PLTRELSZ is " << tags[DT_PLTRELSZ]
                << ", but the section DT_JMPREL names (" << plt->name << ") is "
                << plt->size << " bytes\n";
      failures++;
    }
    if (plt->size != plt_relocs)  {
      std::cerr << "the PLT relocations are " << plt->size
                << " bytes, the original's " << plt_relocs << '\n';
      failures++;
    }
  }

  return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
