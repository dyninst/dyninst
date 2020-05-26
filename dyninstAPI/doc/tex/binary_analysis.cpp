#include <stdio.h>

#include "BPatch.h"
#include "BPatch_addressSpace.h"
#include "BPatch_process.h"
#include "BPatch_binaryEdit.h"
#include "BPatch_function.h"
#include "BPatch_flowGraph.h"

using namespace std;
using namespace Dyninst;

// Create an instance of class BPatch
BPatch bpatch;

// Different ways to perform instrumentation
typedef enum {
    create,
    attach,
    open
} accessType_t; 

BPatch_addressSpace* startInstrumenting(accessType_t accessType,
        const char* name,
        int pid,
        const char* argv[]) {
    BPatch_addressSpace* handle = NULL;

    switch(accessType) {
        case create:
            handle = bpatch.processCreate(name, argv);
            if (!handle) { fprintf(stderr, "processCreate failed\n"); }
            break;
        case attach:
            handle = bpatch.processAttach(name, pid);
            if (!handle) { fprintf(stderr, "processAttach failed\n"); }
            break;
        case open:
            // Open the binary file and all dependencies
            handle = bpatch.openBinary(name, true);
            if (!handle) { fprintf(stderr, "openBinary failed\n"); }
            break;
    }

    return handle;
}

int binaryAnalysis(BPatch_addressSpace* app) {
    BPatch_image* appImage = app->getImage();

    int insns_access_memory = 0;

    std::vector<BPatch_function*> functions;
    appImage->findFunction("InterestingProcedure", functions);

    if (functions.size() == 0) {
        fprintf(stderr, "No function InterestingProcedure\n");
        return insns_access_memory;
    } else if (functions.size() > 1) {
        fprintf(stderr, "More than one InterestingProcedure; using the first one\n");
    }

    BPatch_flowGraph* fg = functions[0]->getCFG();

    std::set<BPatch_basicBlock*> blocks;
    fg->getAllBasicBlocks(blocks);

    for (auto block_iter = blocks.begin(); 
            block_iter != blocks.end(); 
            ++block_iter) {
        BPatch_basicBlock* block = *block_iter;
        std::vector<InstructionAPI::Instruction> insns;
        block->getInstructions(insns);

        for (auto insn_iter = insns.begin(); 
                insn_iter != insns.end(); 
                ++insn_iter) {
            if (insn_iter->readsMemory() || insn_iter->writesMemory()) {
                insns_access_memory++;
            }
        }
    }

    return insns_access_memory;
}






int main() {
    // Set up information about the program to be instrumented
    const char* progName = "InterestingProgram";
    int progPID = 42;
    const char* progArgv[] = {"InterestingProgram", "-h", NULL};
    accessType_t mode = create;

    // Create/attach/open a binary
    BPatch_addressSpace* app = 
        startInstrumenting(mode, progName, progPID, progArgv);
    if (!app) {
        fprintf(stderr, "startInstrumenting failed\n");
        exit(1);
    }
    
    int memAccesses = binaryAnalysis(app);
    
    fprintf(stderr, "Found %d memory accesses\n", memAccesses);
}

