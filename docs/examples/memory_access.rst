
=============================
Instrumenting Memory Accesses
=============================

.. code-block:: cpp

   #include "BPatch.h"
   #include "BPatch_addressSpace.h"
   #include "BPatch_binaryEdit.h"
   #include "BPatch_function.h"
   #include "BPatch_point.h"
   #include "BPatch_process.h"
   #include <stdio.h>
   
   using namespace std;
   using namespace Dyninst;
   
   // Create an instance of class BPatch
   BPatch bpatch;
   
   // Different ways to perform instrumentation
   typedef enum { create, attach, open } accessType_t;
   
   // Attach, create, or open a file for rewriting
   BPatch_addressSpace *startInstrumenting(accessType_t accessType,
                                           const char *name, int pid,
                                           const char *argv[]) {
     BPatch_addressSpace *handle = NULL;
     switch (accessType) {
     case create:
       handle = bpatch.processCreate(name, argv);
       if (!handle) {
         fprintf(stderr, "processCreate failed\n");
       }
       break;
     case attach:
       handle = bpatch.processAttach(name, pid);
       if (!handle) {
         fprintf(stderr, "processAttach failed\n");
       }
       break;
     case open:
       // Open the binary file; do not open dependencies
       handle = bpatch.openBinary(name, false);
       if (!handle) {
         fprintf(stderr, "openBinary failed\n");
       }
       break;
     }
     return handle;
   }
   
   bool instrumentMemoryAccesses(BPatch_addressSpace *app) {
     BPatch_image *appImage = app->getImage();
     
     // We're interested in loads and stores
     BPatch_Set<BPatch_opCode> axs;
     axs.insert(BPatch_opLoad);
     axs.insert(BPatch_opStore);
     
     // Scan the function InterestingProcedure
     // and create instrumentation points
     std::vector<BPatch_function *> functions;
     appImage->findFunction("InterestingProcedure", functions);
     std::vector<BPatch_point *> *points = functions[0]->findPoint(axs);
     if (!points) {
       fprintf(stderr, "No load/store points found\n");
       return false;
     }
     
     // Create the printf function call snippet
     std::vector<BPatch_snippet *> printfArgs;
     BPatch_snippet *fmt = new BPatch_constExpr("Access at: 0x%lx\n");
     printfArgs.push_back(fmt);
     BPatch_snippet *eae = new BPatch_effectiveAddressExpr();
     printfArgs.push_back(eae);
     
     // Find the printf function
     std::vector<BPatch_function *> printfFuncs;
     appImage->findFunction("printf", printfFuncs);
     if (printfFuncs.size() == 0) {
       fprintf(stderr, "Could not find printf\n");
       return false;
     }
     
     // Construct a function call snippet
     BPatch_funcCallExpr printfCall(*(printfFuncs[0]), printfArgs);
     // Insert the snippet at the instrumentation points
     if (!app->insertSnippet(printfCall, *points)) {
       fprintf(stderr, "insertSnippet failed\n");
       return false;
     }
     return true;
   }

   void finishInstrumenting(BPatch_addressSpace *app, const char *newName) {
     BPatch_process *appProc = dynamic_cast<BPatch_process *>(app);
     BPatch_binaryEdit *appBin = dynamic_cast<BPatch_binaryEdit *>(app);
     if (appProc) {
       if (!appProc->continueExecution()) {
         fprintf(stderr, "continueExecution failed\n");
       }
       while (!appProc->isTerminated()) {
         bpatch.waitForStatusChange();
       }
     } else if (appBin) {
       if (!appBin->writeFile(newName)) {
         fprintf(stderr, "writeFile failed\n");
       }
     }
   }
   int main() {
     // Set up information about the program to be instrumented
     const char *progName = "InterestingProgram";
     int progPID = 42;
     const char *progArgv[] = {"InterestingProgram", "-h", NULL};
     accessType_t mode = create;
     // Create/attach/open a binary
     BPatch_addressSpace *app =
         startInstrumenting(mode, progName, progPID, progArgv);
     if (!app) {
       fprintf(stderr, "startInstrumenting failed\n");
       exit(1);
     }
     // Instrument memory accesses
     if (!instrumentMemoryAccesses(app)) {
       fprintf(stderr, "instrumentMemoryAccesses failed\n");
       exit(1);
     }
     // Finish instrumentation
     const char *progName2 = "InterestingProgram-rewritten";
     finishInstrumenting(app, progName2);
   }