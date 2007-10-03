#if !defined(_emit_Elf_h_)
#define _emit_Elf_h_

#include "Object.h"

namespace Dyninst{
namespace SymtabAPI{

class emitElf{
  public:
    emitElf(Elf_X &oldElfHandle_, bool isStripped_ = false, int BSSexpandflag = false, void (*)(const char *) = log_msg);
    ~emitElf();

    bool checkIfStripped(Symtab *obj, vector<Symbol *>&functions, vector<Symbol *>&variables, vector<Symbol *>&mods, vector<Symbol *>&notypes);
    bool driver(Symtab *obj, std::string fName);
 
  private:
    Elf_X oldElfHandle;
    Elf *newElf;
    Elf *oldElf;
    
    //New Section & Program Headers
    Elf32_Ehdr *newEhdr;
    Elf32_Ehdr *oldEhdr;
    
    Elf32_Phdr *newPhdr;
    Elf32_Phdr *oldPhdr;

    //important data sections in the
    //new Elf that need updated
    Elf_Data *textData;
    Elf_Data *symStrData;
    Elf_Data *dynStrData;
    Elf_Data *symTabData;
    Elf_Data *hashData;
    Elf_Data *dynsymData;
    Elf_Data *rodata;
    Elf_Data *dataData;
    
    Elf32_Shdr *textSh;
    Elf32_Shdr *rodataSh;
    
    //Symbol table(.symtab) symbols
    std::vector<Elf32_Sym *> symbols;
    std::vector<std::string> symbolStrs; //names of symbols
    unsigned symbolNamesLength; //Total size of all the names

    std::vector<Section *>nonLoadableSecs;

    // Needed when adding a new segment
    Elf32_Off newSegmentStart;
    Elf32_Shdr *firstNewLoadSec;// initialize to NULL
 
    //text & data segment ends
    Elf32_Off dataSegEnd, textSegEnd;

    //flags
    // Expand NOBITS sections within the object file to their size
    bool BSSExpandFlag;
    bool addNewSegmentFlag;
    
    bool isStripped;

    void (*err_func_)(const char*);

    bool getBackSymbol(Symbol *symbol);
    void findSegmentEnds();
    void addSectionNames(Elf_Data *&, Elf_Data *, unsigned , unsigned , vector<std::string> &);
    bool createNonLoadableSections(Elf32_Shdr *shdr, unsigned shstrtabDataSize, unsigned);
    void addSectionNames(Elf_Data *&, Elf_Data *, unsigned , unsigned , std::vector<std::string> &, std::vector<Section*>&);
    bool createLoadableSections( Elf32_Shdr *shdr, std::vector<Section *>&newSecs, std::vector<std::string> &loadSecNames, unsigned &shstrtabDataSize, unsigned &nonLoadableNamesSize, unsigned &loadSecTotalSize);

    void updateSymbols(Elf_Data* symtabData,Elf_Data* strData, unsigned long loadSecsSize);
    
    void log_elferror(void (*err_func)(const char *), const char* msg);
};

} // namespace SymtabAPI
} // namespace Dyninst

#endif
