std::set<Function*> funcs;
block->getFuncs(std::inserter(funcs, funcs.begin()));
