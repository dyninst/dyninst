using namespace Dyninst;
using namespace SymtabAPI;

// obj represents a handle to a parsed object file using symtabAPI
// Get the Function object representing function bar
std::vector<Function*> funcs;
obj->findFunctionsByName(funcs, "bar");

// Find the local var foo within function bar
std::vector<localVar*> vars;
funcs[0]->findLocalVariable(vars, "foo");
