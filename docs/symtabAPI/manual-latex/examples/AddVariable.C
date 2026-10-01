using namespace Dyninst;
using namespace SymtabAPI;

// obj represents a handle to a parsed object file.
for (auto* m : obj->findModulesByName("/path/to/foo.c"))  {

  // Create a new variable in that module
  Variable* newVar = obj->createVariable("newIntVar", // Name of new variable
                                         0x12345,     // Offset from data section
                                         sizeof(int), // Size of symbol
                                         m);          // Module to create it in
}
