// create a new struct Type
// typedef struct{
//int field1,
//int field2[10]
// } struct1;

using namespace Dyninst;
using namespace SymtabAPI;

// Find a handle to the integer type; obj is a handle to a parsed object file
Type* lookupType = nullptr;
obj->findType(lookupType, "int");

// Convert the generic type object to the specific scalar type object
typeScalar* intType = lookupType->getScalarType();

//create a new array type(int field2[10]); create takes its name by
//non-const reference, so it cannot be passed a literal
std::string arrayName = "intArray";
typeArray* intArray = typeArray::create(arrayName, intType, 0, 9, obj);

//types of the structure fields, held by pointer
std::pair<std::string, Type*> field1("field1", intType);
std::pair<std::string, Type*> field2("field2", intArray);

// container to hold names and types of the new structure type
dyn_c_vector<std::pair<std::string, Type*>*> fields;
fields.push_back(&field1);
fields.push_back(&field2);

//create the structure type
std::string structName = "struct1";
typeStruct* struct1 = typeStruct::create(structName, fields, obj);
