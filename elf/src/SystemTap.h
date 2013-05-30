#include "dyntypes.h"
#include "dyn_regs.h"
#include <vector>
#include <boost/shared_ptr.hpp>

#if !defined(SystemTap_h_)
#define SystemTap_h_
namespace Dyninst {

class Elf_X;
class Elf_X_Shdr;

class ArgTree {
private:
   ArgTree() {}
public:
   typedef boost::shared_ptr<ArgTree> ptr;
   enum op_type_t {
      Register,
      Constant,
      Add,
      Multiply,
      Dereference,
      Segment
   };
   union op_data_t{
      signed int reg;
      signed long val;
   };
   op_type_t op_type;
   op_data_t op_data;
   ArgTree::ptr lchild;
   ArgTree::ptr rchild;

   void print(FILE *f);
   static ArgTree::ptr createConstant(const signed long &v);
   static ArgTree::ptr createRegister(Dyninst::MachRegister r);
   static ArgTree::ptr createDeref(ArgTree::ptr sub);
   static ArgTree::ptr createSegment(ArgTree::ptr l, ArgTree::ptr r);
   static ArgTree::ptr createAdd(ArgTree::ptr l, ArgTree::ptr r);
   static ArgTree::ptr createMultiply(ArgTree::ptr l, ArgTree::ptr r);
};

class x86OperandParser;
class ppcOperandParser;

class SystemTapEntries {
  public:
   struct Arg {
      unsigned arg_size;
      bool is_arg_signed;
      ArgTree::ptr tree;
   };

   struct Entry {
      Dyninst::Address addr;
      Dyninst::Address base_addr;
      Dyninst::Address semaphore_addr;
      std::string provider;
      std::vector<Arg> args;
   };

   static SystemTapEntries *createSystemTapEntries(Elf_X *file_);

   typedef std::map<std::string, const Entry *> entry_list_t;

   const entry_list_t &entryList() { return name_to_entry; }
  private:
   SystemTapEntries(Elf_X *file_);
   ~SystemTapEntries();


   static x86OperandParser *x86_parser;
   static x86OperandParser *x86_64_parser;
   static ppcOperandParser *ppc32_parser;
   static ppcOperandParser *ppc64_parser;
   static std::map<Elf_X *, SystemTapEntries *> all_entries;
   Elf_X *file;
   Dyninst::Architecture arch;
   unsigned int word_size;

   bool readAddr(const unsigned char *buffer, size_t size, unsigned &offset,
                 Dyninst::Address &result, unsigned int read_size = 0);
   bool readString(const unsigned char *buffer, size_t bsize, unsigned &offset, 
                   std::string &result);

   bool parse();
   bool parseAllNotes();
   bool parseNotes(Elf_X_Shdr &shdr);
   bool parseOperands(std::string ops, Entry &entry);
   bool parseOperand_x86(std::string op, Arg &result);
   bool parseOperand_ppc(std::string op, Arg &result);

   entry_list_t name_to_entry;
};

}

#endif
