//Example showing how to iterate over the line information for a given module.
using namespace Dyninst;
using namespace SymtabAPI;

//obj represents a handle to a parsed object file using symtabAPI
//Find the module 'foo' within the object.
for (auto* m : obj->findModulesByName("foo"))  {

  // Get the Line Information for module foo.
  LineInformation* info = m->getLineInformation();

  //Iterate over the line information
  LineInformation::const_iterator iter;
  for (iter = info->begin(); iter != info->end(); iter++)
  {
    //Each entry is a Statement, which is itself an address range
    Statement* stmt = *iter;

    //The range of addresses generated for this line
    Offset lowAddr = stmt->startAddr();
    Offset highAddr = stmt->endAddr();

    //and the source position they came from
    const std::string& file = stmt->getFile();
    unsigned int line = stmt->getLine();
  }
}
