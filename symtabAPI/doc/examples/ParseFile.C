using namespace Dyninst;
using namespace SymtabAPI;

//Name the object file to be parsed:
std::string file = "libfoo.so";

//Declare a pointer to an object of type Symtab; this represents the file.
Symtab* obj = NULL;

// Parse the object file
bool err = Symtab::openFile(obj, file);
