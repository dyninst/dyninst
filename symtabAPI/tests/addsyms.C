/* Given a binary with missing symbols as input, produce a new
   binary with new symbols filled in for functions discovered
   by Dyninst's techniques.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <BPatch.h>
#include <Dyn_Symtab.h>
#include <BPatch_function.h>

using namespace std;

BPatch bpatch;

void usage(char *name)
{
    printf("Usage: %s <binary> [output binary]\n",name);
}

int main(int argc, char**argv)
{
    char * inName;
    char * outName;
    string inStr;
    string outStr;

    BPatch_process * proc;
    BPatch_image * img;
    const char * bpatch_argv[1];
    BPatch_Vector<BPatch_function *> * dyn_funcs;
    BPatch_Vector<BPatch_module *> * dyn_mods;

    Dyn_Symtab *symtab;

    char fname[128];

    vector<Dyn_Symbol *> allSyms; 
    vector<Dyn_Module *> sym_mods;

    if(argc < 2) {
        usage(argv[0]);
        exit(1);
    }

    inName = argv[1];
    inStr = inName;
    if(argc == 3) {
        outName = argv[2];
    } else {
        outName = new char[strlen(argv[1]) + 5];
        snprintf(outName, strlen(argv[1]) + 5, "%s-out",argv[1]);
    }
    outStr = outName;

    /* 1. Load the binary and parse with Dyninst */
    printf("*** Loading image...\n");
    
    bpatch_argv[0] = NULL;
    proc = bpatch.processCreate(inName,bpatch_argv);
    if(!proc) {
        fprintf(stderr,"Unable to create process %s, exiting\n",inName);
        exit(1);
    }
    img = proc->getImage();

    printf("*** Parsing binary image...\n");

        // cause Dyninst to do a full parse of the binary
        dyn_funcs = img->getProcedures();
        if(!dyn_funcs)  {
            fprintf(stderr,"Unable to parse binary %s, exiting\n",inName);
            exit(1);
        }

    BPatch_module *mod = img->findModule("DEFAULT_MODULE");
    if(!mod) {
        fprintf(stderr,"unable to find default module, exiting\n");
        exit(1);
    }

    Dyn_Symtab * st = mod->getDynSymtab();
    if(st) {
      
        printf("*** Emitting new binary\n"); 
       /* st->getAllModules(sym_mods);

        for(unsigned m=0;m<sym_mods.size(); m++)
        {
            // for each module, find the analogue in Dyninst

            printf("module: %s (%s)\n",sym_mods[m]->fileName().c_str(),
                sym_mods[m]->fullName().c_str());
            
        } */
        st->emitSymbols(outStr);
    } else {   
        printf("*** Failed to locate Dyn_Symtab object\n"); 
    }

    return 0;
}
