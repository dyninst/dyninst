using namespace Dyninst;
using namespace SymtabAPI;
std::vector <Symbol*> syms;
std::vector <Function*> funcs;

// search for a function with demangled (pretty) name "bar".
if (obj->findFunctionsByName(funcs, "bar"))  {
  // Add a new (mangled) primary name to the first function
  funcs[0]->addMangledName("newname", true);
}

// search for symbol of any type with demangled (pretty) name "bar".
if (obj->findSymbol(syms, "bar", Symbol::ST_UNKNOWN))  {

  // change the type of the found symbol to type variable(ST_OBJECT)
  syms[0]->setSymbolType(Symbol::ST_OBJECT);

  // These changes are automatically added to symtabAPI; no further
  // actions are required by the user.
}
