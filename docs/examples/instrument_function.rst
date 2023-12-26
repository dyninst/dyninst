
========================
Instrumenting a function
========================

.. code-block:: cpp

   #include <stdio.h>
   #include "BPatch.h"
   #include "BPatch_addressSpace.h"
   #include "BPatch_process.h"
   #include "BPatch_binaryEdit.h"
   #include "BPatch_point.h"
   #include "BPatch_function.h"
   
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
   
   // Attach, create, or open a file for rewriting
   BPatch_addressSpace* startInstrumenting(
       accessType_t accessType,
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
   
   // Find a point at which to insert instrumentation
   std::vector<BPatch_point*>* findPoint(
       BPatch_addressSpace* app,
       const char* name,
       BPatch_procedureLocation loc) {
   
     std::vector<BPatch_function*> functions;
     std::vector<BPatch_point*>* points;
   
     // Scan for functions named "name"
     BPatch_image* appImage = app->getImage();
     appImage->findFunction(name, functions);
   
     if (functions.size() == 0) {
       fprintf(stderr, "No function %s\n", name);
       return points;
     } else if (functions.size() > 1) {
       fprintf(stderr, "More than one %s; using the first one\n", name);
     }
   
     // Locate the relevant points
     points = functions[0]->findPoint(loc);
     
     return points;
   }
   
   // Create and insert an increment snippet
   bool createAndInsertSnippet(
       BPatch_addressSpace* app,
       std::vector<BPatch_point*>* points) {
   
     BPatch_image* appImage = app->getImage();
   
     // Create an increment snippet
     BPatch_variableExpr* intCounter =
     app->malloc(*(appImage->findType("int")), "myCounter");
   
     BPatch_arithExpr addOne(
       BPatch_assign,
       *intCounter,
       BPatch_arithExpr(
         BPatch_plus,
         *intCounter,
         BPatch_constExpr(1)
       )
     );
   
     // Insert the snippet
     if (!app->insertSnippet(addOne, *points)) {
       fprintf(stderr, "insertSnippet failed\n");
       return false;
     }
     return true;
   }
   
   // Create and insert a printf snippet
   bool createAndInsertSnippet2(
     BPatch_addressSpace* app,
     std::vector<BPatch_point*>* points) {
   
     BPatch_image* appImage = app->getImage();
   
     // Create the printf function call snippet
     std::vector<BPatch_snippet*> printfArgs;
     BPatch_snippet* fmt = new BPatch_constExpr("InterestingProcedure called %d times\n");
     printfArgs.push_back(fmt);
     BPatch_variableExpr* var = appImage->findVariable("myCounter");
   
     if (!var) {
       fprintf(stderr, "Could not find 'myCounter' variable\n");
       return false;
     } else {
       printfArgs.push_back(var);
     }
   
     // Find the printf function
     std::vector<BPatch_function*> printfFuncs;
     appImage->findFunction("printf", printfFuncs);
   
     if (printfFuncs.size() == 0) {
       fprintf(stderr, "Could not find printf\n");
       return false;
     }
   
     // Construct a function call snippet
     BPatch_funcCallExpr printfCall(*(printfFuncs[0]), printfArgs);
   
     // Insert the snippet
     if (!app->insertSnippet(printfCall, *points)) {
       fprintf(stderr, "insertSnippet failed\n");
       return false;
     }
     return true;
   }
   
   void finishInstrumenting(BPatch_addressSpace* app, const char*newName) {
     BPatch_process* appProc = dynamic_cast<BPatch_process*>(app);
     BPatch_binaryEdit* appBin = dynamic_cast<BPatch_binaryEdit*>(app);
     
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
     const char* progName = "InterestingProgram";
     int progPID = 42;
     const char* progArgv[] = {"InterestingProgram", "-h", NULL};
   
     accessType_t mode = create;
   
     // Create/attach/open a binary
     BPatch_addressSpace* app = startInstrumenting(mode, progName, progPID, progArgv);
     if (!app) {
       fprintf(stderr, "startInstrumenting failed\n");
       exit(1);
     }
   
     // Find the entry point for function InterestingProcedure
     const char* interestingFuncName = "InterestingProcedure";
     std::vector<BPatch_point*>* entryPoint = findPoint(app, interestingFuncName, BPatch_entry);
     
     if (!entryPoint || entryPoint->size() == 0) {
       fprintf(stderr, "No entry points for %s\n", interestingFuncName);
       exit(1);
     }
   
     // Create and insert instrumentation snippet
     if (!createAndInsertSnippet(app, entryPoint)) {
       fprintf(stderr, "createAndInsertSnippet failed\n");
       exit(1);
     }
   
     // Find the exit point of main
     std::vector<BPatch_point*>* exitPoint = findPoint(app, "main", BPatch_exit);
     if (!exitPoint }} exitPoint->size() == 0) {
       fprintf(stderr, "No exit points for main\n");
       exit(1);
     }
   
     // Create and insert instrumentation snippet 2
     if (!createAndInsertSnippet2(app, exitPoint)) {
     fprintf(stderr, "createAndInsertSnippet2 failed\n");
     exit(1);
     }
   
     // Finish instrumentation
     const char* progName2 = "InterestingProgram-rewritten";
     finishInstrumenting(app, progName2);
   }