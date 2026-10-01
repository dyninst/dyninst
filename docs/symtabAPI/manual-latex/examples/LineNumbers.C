using namespace Dyninst;
using namespace SymtabAPI;

// obj represents a handle to a parsed object file using symtabAPI
// Container to hold the address range
std::vector<AddressRange> ranges;

// Get the address range for the line 30 in source file foo.c
obj->getAddressRanges(ranges, "foo.c", 30);
