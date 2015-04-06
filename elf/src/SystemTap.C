#include "elf/src/SystemTap.h"
#include "elf/h/Elf_X.h"
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi_parse.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/spirit/include/phoenix_bind.hpp>
#include <boost/spirit/include/phoenix_statement.hpp>
#include <boost/fusion/adapted/std_pair.hpp>
#include <boost/tokenizer.hpp>

#include <iostream>
#include <string>
#include <set>
#include <vector>
#include <map>

#include <elf.h>

using namespace std;

namespace qi = boost::spirit::qi;
namespace phoenix = boost::phoenix;
namespace ascii = boost::spirit::ascii;

namespace Dyninst {

struct OperandParser {
   typedef qi::rule<string::const_iterator, ArgTree::ptr(), ascii::space_type> ArgTreeRule;

   std::map<std::string, Dyninst::MachRegister> register_names;
   ArgTree::ptr getReg(std::string name);

   ArgTree::ptr newConstant(const signed long &i);
   ArgTree::ptr newDeref(ArgTree::ptr p);
   ArgTree::ptr newSegment(ArgTree::ptr a, ArgTree::ptr b);
   ArgTree::ptr identity(ArgTree::ptr p);
   ArgTree::ptr newDerefAdd(ArgTree::ptr a, ArgTree::ptr b);
   ArgTree::ptr newBaseIndexScale(ArgTree::ptr base, ArgTree::ptr index, int scale);
   ArgTree::ptr newAdd(ArgTree::ptr a, ArgTree::ptr b);
   ArgTree::ptr newRegister(const std::vector<char> &v);
};

struct x86OperandParser : public qi::grammar<std::string::const_iterator, ArgTree::ptr(), ascii::space_type>,
                          public OperandParser
{
   ArgTreeRule operand, shex, reg, mem_modrm_nobase, mem_modrm, modrm;

   void createRegisterNames(Dyninst::Architecture arch);
   x86OperandParser(Dyninst::Architecture arch);
};

struct ppcOperandParser : public qi::grammar<std::string::const_iterator, ArgTree::ptr(), ascii::space_type>,
                          public OperandParser
{
   ArgTreeRule operand, shex, num, reg;

   void createRegisterNames(Dyninst::Architecture arch);
   ppcOperandParser(Dyninst::Architecture arch);
};

struct aarch64OperandParser : public qi::grammar<std::string::const_iterator, ArgTree::ptr(), ascii::space_type>,
                          public OperandParser
{
   ArgTreeRule operand, shex, num, reg;

   void createRegisterNames(Dyninst::Architecture arch);
   aarch64OperandParser(Dyninst::Architecture arch);
};

}

using namespace Dyninst;

map<Elf_X *, SystemTapEntries *> SystemTapEntries::all_entries;
SystemTapEntries *SystemTapEntries::createSystemTapEntries(Elf_X *file_)
{
   map<Elf_X *, SystemTapEntries *>::iterator i = all_entries.find(file_);
   if (i != all_entries.end())
      return i->second;

   SystemTapEntries *st = new SystemTapEntries(file_);
   bool result = st->parse();
   if (!result) {
      delete st;
      st = NULL;
   }

   all_entries.insert(make_pair(file_, st));
   return st;
}

SystemTapEntries::SystemTapEntries(Elf_X *file_) :
   file(file_)
{
}

SystemTapEntries::~SystemTapEntries()
{
}

bool SystemTapEntries::parse() {
   switch (file->e_machine()) {
      case EM_386:
         arch = Arch_x86;
         break;
      case EM_X86_64:
         arch = Arch_x86_64;
         break;
      case EM_PPC:
         arch = Arch_ppc32;
         break;
      case EM_PPC64:
         arch = Arch_ppc64;
         break;
   }
   word_size = getArchAddressWidth(arch);

   return parseAllNotes();
}

bool SystemTapEntries::parseAllNotes()
{
   for (unsigned short i = 0; i < file->e_shnum(); i++) {
      Elf_X_Shdr &shdr = file->get_shdr(i);
      if (!shdr.isValid())
         continue;
      if (shdr.sh_type() != SHT_NOTE)
         continue;

      bool result = parseNotes(shdr);
      if (!result)
         return false;
   }

   return true;
}

#if !defined(_SDT_NOTE_TYPE)
#define SDT_NOTE_TYPE 3
#endif
#if !defined(_SDT_NOTE_NAME)
#define SDT_NOTE_NAME "stapsdt"
#endif

bool SystemTapEntries::parseNotes(Elf_X_Shdr &shdr)
{
   bool parseError = false;

   for (Elf_X_Nhdr note = shdr.get_note(); note.isValid(); note = note.next()) {
      if (note.n_type() != SDT_NOTE_TYPE ||
              strcmp(note.get_name(), SDT_NOTE_NAME) != 0)
          continue;

      Entry e;
      unsigned i = 0;
      size_t size = note.n_descsz();
      const unsigned char *buffer = (const unsigned char *)note.get_desc();

      //System tap structure format looks like:
      // struct {
      //   Address addr;
      //   Address base_addr;
      //   Address semaphore_addr;
      //   char provider[]
      //   char probe_name[]
      //   char argument_string[]
      // }
      bool result = readAddr(buffer, size, i, e.addr);
      if (!result) {
         parseError = true;
         break;
      }
      result = readAddr(buffer, size, i, e.base_addr);
      if (!result) {
         parseError = true;
         break;
      }
      result = readAddr(buffer, size, i, e.semaphore_addr);
      if (!result) {
         parseError = true;
         break;
      }
      result = readString(buffer, size, i, e.provider);
      if (!result) {
         parseError = true;
         break;
      }
      std::string name;
      result = readString(buffer, size, i, name);
      if (!result) {
         parseError = true;
         break;
      }
      std::string args;
      result = readString(buffer, size, i, args);
      if (!result) {
         parseError = true;
         break;
      }
      while (i % 4 != 0) i++;

      result = parseOperands(args, e);
      if (!result) {
         parseError = true;
         break;
      }

      Entry *entry = new Entry(e);
      name_to_entry.insert(make_pair(name, entry));
   }

   return !parseError;
}

bool SystemTapEntries::readAddr(const unsigned char *buffer, size_t bsize, unsigned &offset,
                                Dyninst::Address &result, unsigned int read_size)
{
   if (!read_size) {
      read_size = word_size;
   }
   if (offset + read_size > bsize)
      return false;

   if (read_size == 4)
      result = *((const uint32_t *) (buffer + offset));
   else if (read_size == 8)
      result = *((const uint64_t *) (buffer + offset));
   offset += read_size;

   return true;
}

bool SystemTapEntries::readString(const unsigned char *buffer, size_t bsize, unsigned &offset,
                                  std::string &result)
{
   unsigned int start = offset;
   unsigned int end = start;

   if (start >= bsize)
      return false;

   while (buffer[end] != '\0' && end < bsize) end++;
   result = std::string(((const char *) buffer)+start, end-start);
   offset = end+1;
   return true;
}

bool SystemTapEntries::parseOperands(std::string ops, Entry &entry)
{
   if (ops.empty() || ops == string(":")) {
      //Empty operand list
      return true;
   }

   typedef boost::tokenizer<boost::char_separator<char> > tok_t;
   boost::char_separator<char> sep(" ");
   tok_t tokens(ops, sep);

   for (tok_t::iterator i = tokens.begin(); i != tokens.end(); i++) {
      Arg result;
      string arg = *i;

      string operand = arg;
      result.arg_size = 0;
      result.is_arg_signed = false;

      //Extract the type info encoded in the <int> of the form <int>@<arg>
      size_t at_pos = arg.find('@');
      if (at_pos != string::npos) {
         string type_info_str(operand, 0, at_pos);
         if (!type_info_str.empty()) {
            char *endptr = NULL;
            signed long type_info = strtol(type_info_str.c_str(), &endptr, 10);
            if (*endptr == '\0') {
               //We have the type info in integer form;
               result.arg_size = abs(type_info);
               result.is_arg_signed = (type_info < 0);
               operand = string(arg, at_pos+1);
            }
         }
      }
      bool bres = true;
      if (arch == Arch_x86 || arch == Arch_x86_64)
         bres = parseOperand_x86(operand, result);
      else if (arch == Arch_ppc32 || arch == Arch_ppc64)
         bres = parseOperand_ppc(operand, result);

      if (!bres) {
         return false;
      }
      entry.args.push_back(result);
   }

   return true;
}

x86OperandParser *SystemTapEntries::x86_parser = NULL;
x86OperandParser *SystemTapEntries::x86_64_parser = NULL;
ppcOperandParser *SystemTapEntries::ppc32_parser = NULL;
ppcOperandParser *SystemTapEntries::ppc64_parser = NULL;

bool SystemTapEntries::parseOperand_x86(std::string op, Arg &arg)
{
   x86OperandParser* &parser = (arch == Arch_x86) ? x86_parser : x86_64_parser;
   if (!parser)
      parser = new x86OperandParser(arch);

   using boost::spirit::ascii::space;
   std::string::const_iterator iter = op.begin();
   std::string::const_iterator end = op.end();
   bool result = phrase_parse(iter, end, *parser, space, arg.tree);
   if (!result || iter != end) {
      //Failed parse.
      return false;
   }
   return true;
}

bool SystemTapEntries::parseOperand_ppc(std::string op, Arg &arg)
{
   ppcOperandParser* &parser = (arch == Arch_ppc32) ? ppc32_parser : ppc64_parser;
   if (!parser)
      parser = new ppcOperandParser(arch);

   using boost::spirit::ascii::space;
   std::string::const_iterator iter = op.begin();
   std::string::const_iterator end = op.end();
   bool result = phrase_parse(iter, end, *parser, space, arg.tree);
   if (!result || iter != end) {
      //Failed parse.
      return false;
   }
   return true;
}

ArgTree::ptr OperandParser::getReg(std::string name) {
   std::map<std::string, Dyninst::MachRegister>::iterator i = register_names.find(name);
   if (i == register_names.end())
      return ArgTree::createRegister(Dyninst::InvalidReg);
   return ArgTree::createRegister(i->second);
}

//Wrappers to fix odd compiler errors from phoenix::bind
ArgTree::ptr OperandParser::newConstant(const signed long &i) {
   return ArgTree::createConstant(i);
}

ArgTree::ptr OperandParser::newDeref(ArgTree::ptr p) {
   return ArgTree::createDeref(p);
}

ArgTree::ptr OperandParser::newSegment(ArgTree::ptr a, ArgTree::ptr b) {
   return ArgTree::createSegment(a, b);
}

ArgTree::ptr OperandParser::identity(ArgTree::ptr p) {
   return p;
}

ArgTree::ptr OperandParser::newDerefAdd(ArgTree::ptr a, ArgTree::ptr b) {
   return ArgTree::createDeref(ArgTree::createAdd(a, b));
}

ArgTree::ptr OperandParser::newBaseIndexScale(ArgTree::ptr base, ArgTree::ptr index, int scale) {
   return ArgTree::createAdd(base, ArgTree::createMultiply(index, ArgTree::createConstant(scale)));
}

ArgTree::ptr OperandParser::newAdd(ArgTree::ptr a, ArgTree::ptr b) {
   return ArgTree::createAdd(a, b);
}

ArgTree::ptr OperandParser::newRegister(const std::vector<char> &v) {
   std::string s;
   for (std::vector<char>::const_iterator i = v.begin(); i != v.end(); i++) s += *i;
   return getReg(s);
}

x86OperandParser::x86OperandParser(Dyninst::Architecture arch) :
   x86OperandParser::base_type(operand)
{
   using namespace qi::labels;
   using qi::uint_;
   using qi::int_;
   using qi::lit;
   using qi::hex;
   using qi::alnum;

   using phoenix::construct;
   using phoenix::val;
   using boost::phoenix::ref;

   createRegisterNames(arch);

   shex =
      lit("0x") >> hex        [qi::_val = phoenix::bind(&OperandParser::newConstant, this, qi::_1)]
      | lit("-0x") >> hex     [qi::_val = phoenix::bind(&OperandParser::newConstant, this, -1*qi::_1)]
      | int_                  [qi::_val = phoenix::bind(&OperandParser::newConstant, this, qi::_1)]
      ;

   reg = '%' >> (+alnum)      [qi::_val = phoenix::bind(&OperandParser::newRegister, this, qi::_1)];

   mem_modrm_nobase =
      ('(' >> reg >> ')')     [qi::_val = phoenix::bind(&OperandParser::identity, this, qi::_1)]
      | ( '(' >> reg >> ',' >> reg >> ',' >> uint_ >> ')' )
      [qi::_val = phoenix::bind(&OperandParser::newBaseIndexScale, this, qi::_1, qi::_2, qi::_3)]
      ;

   mem_modrm =
      (shex >> mem_modrm_nobase)  [qi::_val = phoenix::bind(&OperandParser::newDerefAdd, this, qi::_1, qi::_2)]
      | mem_modrm_nobase          [qi::_val = phoenix::bind(&OperandParser::newDeref, this, qi::_1)]
      | shex                      [qi::_val = phoenix::bind(&OperandParser::newDeref, this, qi::_1)]

      ;

   modrm =
      reg             [qi::_val = phoenix::bind(&OperandParser::identity, this, qi::_1)]
      | mem_modrm     [qi::_val = phoenix::bind(&OperandParser::identity, this, qi::_1)]
      | '$' >> shex   [qi::_val = phoenix::bind(&OperandParser::identity, this, qi::_1)]
      ;

   operand =
      modrm                    [qi::_val = phoenix::bind(&OperandParser::identity, this, qi::_1)]
      | (reg >> ':' >> modrm)  [qi::_val = phoenix::bind(&OperandParser::newSegment, this, qi::_1, qi::_2)]
      ;
}

void x86OperandParser::createRegisterNames(Dyninst::Architecture arch) {
   Dyninst::MachRegister::NameMap::iterator i = Dyninst::MachRegister::names()->begin();
   for (; i != Dyninst::MachRegister::names()->end(); i++) {
      Dyninst::MachRegister reg(i->first);

      if (reg.getArchitecture() != arch) {
         continue;
      }
      unsigned int gpr_code = (arch == Dyninst::Arch_x86 ? Dyninst::x86::GPR : Dyninst::x86_64::GPR);
      unsigned int seg_code = (arch == Dyninst::Arch_x86 ? Dyninst::x86::SEG : Dyninst::x86_64::SEG);
      if (reg.regClass() != gpr_code &&
          reg.regClass() != seg_code &&
          !reg.isPC()) {
         continue;
      }
      std::string full_reg_name = reg.name();
      size_t pos = full_reg_name.find("::");
      std::string reg_name = std::string(full_reg_name, pos+2);
      register_names[reg_name] = reg;
   }
}


ppcOperandParser::ppcOperandParser(Dyninst::Architecture arch) :
   ppcOperandParser::base_type(operand)
{
   using namespace qi::labels;
   using qi::uint_;
   using qi::int_;
   using qi::lit;
   using qi::hex;
   using qi::alnum;

   using phoenix::construct;
   using phoenix::val;
   using boost::phoenix::ref;

   createRegisterNames(arch);

   shex =
      lit("0x") >> hex        [qi::_val = phoenix::bind(&OperandParser::newConstant, this, qi::_1)]
      | lit("-0x") >> hex     [qi::_val = phoenix::bind(&OperandParser::newConstant, this, -1*qi::_1)]
      ;

   num = int_                 [qi::_val = phoenix::bind(&OperandParser::newConstant, this, qi::_1)]
      ;

   reg = 'r' >> (+alnum)      [qi::_val = phoenix::bind(&OperandParser::newRegister, this, qi::_1)];
      ;

   operand =
      reg                     [qi::_val = phoenix::bind(&OperandParser::identity, this, qi::_1)]
      | (num >> lit("(") >> reg >> lit(")"))
                              [qi::_val = phoenix::bind(&OperandParser::newDerefAdd, this, qi::_1, qi::_2)]
      | shex                  [qi::_val = phoenix::bind(&OperandParser::identity, this, qi::_1)]
      ;
}

void ppcOperandParser::createRegisterNames(Dyninst::Architecture arch)
{
   Dyninst::MachRegister::NameMap::iterator i = Dyninst::MachRegister::names()->begin();
   for (; i != Dyninst::MachRegister::names()->end(); i++) {
      Dyninst::MachRegister reg(i->first);

      if (reg.getArchitecture() != arch) {
         continue;
      }
      unsigned int gpr_code = (arch == Dyninst::Arch_ppc32 ? Dyninst::ppc32::GPR : Dyninst::ppc64::GPR);
      if (reg.regClass() != gpr_code) {
         continue;
      }
      std::string full_reg_name = reg.name();
      size_t pos = full_reg_name.find("::r");
      std::string reg_name = std::string(full_reg_name, pos+3);
      register_names[reg_name] = reg;
   }
}

void ArgTree::print(FILE *f)
{
   switch (op_type) {
      case Register:
         fprintf(f, "%%%s", Dyninst::MachRegister(op_data.reg).name().c_str());
         break;
      case Constant:
         fprintf(f, "%ld", op_data.val);
         break;
      case Add:
         fprintf(f, "(");
         lchild->print(f);
         fprintf(f, " + ");
         rchild->print(f);
         fprintf(f, ")");
         break;
      case Multiply:
         fprintf(f, "(");
         lchild->print(f);
         fprintf(f, " * ");
         rchild->print(f);
         fprintf(f, ")");
         break;
      case Dereference:
         fprintf(f, "*(");
         lchild->print(f);
         fprintf(f, ")");
         break;
      case Segment:
         lchild->print(f);
         fprintf(f, ":");
         rchild->print(f);
         break;
   }
}

ArgTree::ptr ArgTree::createConstant(const signed long &v) {
   ArgTree::ptr ret = ArgTree::ptr(new ArgTree);
   ret->op_type = Constant;
   ret->op_data.val = v;
   return ret;
}

ArgTree::ptr ArgTree::createRegister(Dyninst::MachRegister r) {
   ArgTree::ptr ret = ArgTree::ptr(new ArgTree);
   ret->op_type = Register;
   ret->op_data.reg = r;
   return ret;
}

ArgTree::ptr ArgTree::createDeref(ArgTree::ptr sub) {
   ArgTree::ptr ret = ArgTree::ptr(new ArgTree);
   ret->op_type = Dereference;
   ret->op_data.val = 0;
   ret->lchild = sub;
   return ret;
}

ArgTree::ptr ArgTree::createSegment(ArgTree::ptr l, ArgTree::ptr r) {
   ArgTree::ptr ret = ArgTree::ptr(new ArgTree);
   ret->op_type = Segment;
   ret->op_data.val = 0;
   ret->lchild = l;
   ret->rchild = r;
   return ret;
}

ArgTree::ptr ArgTree::createAdd(ArgTree::ptr l, ArgTree::ptr r) {
   ArgTree::ptr ret = ArgTree::ptr(new ArgTree);
   ret->op_type = Add;
   ret->op_data.val = 0;
   ret->lchild = l;
   ret->rchild = r;
   return ret;
}

ArgTree::ptr ArgTree::createMultiply(ArgTree::ptr l, ArgTree::ptr r) {
   ArgTree::ptr ret = ArgTree::ptr(new ArgTree);
   ret->op_type = Multiply;
   ret->op_data.val = 0;
   ret->lchild = l;
   ret->rchild = r;
   return ret;
}

