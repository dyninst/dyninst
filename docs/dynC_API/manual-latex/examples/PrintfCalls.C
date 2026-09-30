BPatch_funcCallExpr entryCall(*BPF_printf, entryArgs);
BPatch_funcCallExpr exitCall(*BPF_printf, exitArgs);
entrySnippetVect.push_back(&entryCall);
exitSnippetVect.push_back(&exitCall);
