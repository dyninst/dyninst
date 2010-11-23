    hash_map<Address, bool> seen;
    vector<Function *> funcs;
    SymtabCodeSource *sts;
    CodeObject *co;

    // Create a new binary code object from the filename argument
    sts = new SymtabCodeSource( argv[1] );
    co = new CodeObject( sts );

    // Parse the binary
    co->parse();

    printf("digraph G {\n");

    // Print the control flow graph
    CodeObject::funclist & all = co->funcs();
    CodeObject::funclist::iterator fit = all.begin();
    for( ; fit != all.end(); ++fit) {
        Function * f = *fit;

        if(f->retstatus() == NORETURN) 
            printf("\t\"%lx\" [shape=box,color=red]\n",f->addr()); 
        else 
            printf("\t\"%lx\" [shape=box]\n",f->addr()); 
    
        Function::blocklist::iterator bit = f->blocks().begin();
        for( ; bit != f->blocks().end(); ++bit) {
            Block * b = *bit;

            // Don't revisit blocks in shared code
            if(seen.find(b->start()) != seen.end())
                continue;
            seen[b->start()] = true;

            Block::edgelist::iterator it = b->targets().begin();
            for( ; it != b->targets().end(); ++it) {
               char * s = "";
               if((*it)->type() == CALL)
                s = " [color=blue]";
               else if((*it)->type() == RET)
                s = " [color=green]";
               printf("\t\t\"%lx\" -> \"%lx\"%s\n",(*it)->src()->start(),
                (*it)->trg()->start(),s); 
            }
        }
    }

    printf("}\n");

    delete co;
    delete sts;
