//Example shows how to retrieve a structure type object from a given "Type" object
using namespace Dyninst;
using namespace SymtabAPI;

//obj represents a handle to a parsed object file using symtabAPI
//Find a structure type in the object file
Type* structType = nullptr;
obj->findType(structType, "structType1");

// Get the specific typeStruct object
typeStruct* stType = structType->getStructType();
