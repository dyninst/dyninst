#include "Symtab.h"
#include "Symbol.h"
#include <cstdio>
#include <string>
#include <vector>

int main(int argc, char **argv) {
    if (argc != 3) return 2;
    const std::string kind(argv[2]);
    if (kind != "pcrel" && kind != "omit" && kind != "invalid" && kind != "oversized") return 2;
    using namespace Dyninst::SymtabAPI;
    Symtab *symtab = nullptr;
    if (!Symtab::openFile(symtab, argv[1])) return 1;
    std::vector<ExceptionBlock *> exceptions;
    symtab->getAllExceptions(exceptions);
    bool ok = false;
    if (kind == "invalid" || kind == "oversized") {
        ok = exceptions.empty();
    } else {
        std::vector<Symbol *> start, landing;
        symtab->findSymbol(start, "lsda_try");
        symtab->findSymbol(landing, "lsda_catch");
        ok = start.size() == 1 && landing.size() == 1 && exceptions.size() == 1;
        if (ok) {
            ok = exceptions[0]->tryStart() == start[0]->getOffset() &&
                 exceptions[0]->trySize() == 4 &&
                 exceptions[0]->catchStart() == landing[0]->getOffset();
            if (!ok) std::fprintf(stderr, "try=%lx expected=%lx catch=%lx expected=%lx\n",
                         exceptions[0]->tryStart(), start[0]->getOffset(),
                         exceptions[0]->catchStart(), landing[0]->getOffset());
        }
    }
    if (!ok) std::fprintf(stderr, "%s: exceptions=%zu, expected=%u\n", kind.c_str(),
                          exceptions.size(), (kind == "invalid" || kind == "oversized") ? 0u : 1u);
    Symtab::closeSymtab(symtab);
    return ok ? 0 : 1;
}
