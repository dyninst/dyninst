BPatch_sequence entrySequence(entrySnippetVect);
BPatch_sequence exitSequence(exitSnippetVect);
appProc->insertSnippet(entrySequence, *functions[i]->findPoint(BPatch_entry));
appProc->insertSnippet(exitSequence, *functions[i]->findPoint(BPatch_exit));
