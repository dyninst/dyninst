
/*
 *  A simple code coverage tool using DyninstAPI
 *
 *  This tool uses DyninstAPI to instrument the functions and basic blocks in
 *  an executable and its shared libraries in order to record code coverage
 *  data when the executable is run. This code coverage data is output when the
 *  rewritten executable finishes running.
 *
 *  The intent of this tool is to demonstrate some capabilities of DyninstAPI;
 *  it should serve as a good stepping stone to building a more feature-rich
 *  code coverage tool on top of Dyninst.
 */

#include <cstdlib>
#include <iostream>
#include <vector>
#include <string>
using namespace std;

// Command line parsing
#include <getopt.h>

// DyninstAPI includes
#include "BPatch.h"
#include "BPatch_binaryEdit.h"
#include "BPatch_flowGraph.h"
#include "BPatch_function.h"
#include "BPatch_point.h"

#include "BPatch_object.h"
#include "BPatch_module.h"


using namespace Dyninst;

static const char *USAGE = " [-bpsa] <binary> <output binary>\n \
                            -b: Basic block level code coverage\n \
                            -p: Print all functions (including functions that are never executed)\n \
                            -s: Instrument shared libraries also\n \
                            -a: Sort results alphabetically by function name\n";

static const char *OPT_STR = "bpsa";

// configuration options
char *inBinary = NULL;
char *outBinary = NULL;
bool includeSharedLib = false;
int printAll = 0;
bool bbCoverage = false;
int alphabetical = 0;

set < string > skipLibraries;

/* Every Dyninst mutator needs to declare one instance of BPatch */
BPatch bpatch;

void initSkipLibraries ()
{
    /* List of shared libraries to skip instrumenting */
    /* Do not instrument the instrumentation library */
    skipLibraries.insert ("libInst.so");
    skipLibraries.insert ("libc.so.6");
    skipLibraries.insert ("libc.so.7");
    skipLibraries.insert ("ld-2.5.so");
    skipLibraries.insert ("ld-linux.so.2");
    skipLibraries.insert ("ld-lsb.so.3");
    skipLibraries.insert ("ld-linux-x86-64.so.2");
    skipLibraries.insert ("ld-lsb-x86-64.so");
    skipLibraries.insert ("ld-elf.so.1");
    skipLibraries.insert ("ld-elf32.so.1");
    skipLibraries.insert ("libstdc++.so.6");
    return;
}

bool parseArgs (int argc, char *argv[])
{
    int c;
    while ((c = getopt (argc, argv, OPT_STR)) != -1) {
        switch ((char) c) {
            case 'b':
                bbCoverage = true;
                break;
            case 'p':
                printAll = 1;
                break;
            case 's':
                /* if includeSharedLib is set,
                 * all libraries linked to the binary will also be instrumented */
                includeSharedLib = true;
                break;
            case 'a':
                alphabetical = 1;
                break;
            default:
                cerr << "Usage: " << argv[0] << USAGE;
                return false;
        }
    }

    int endArgs = optind;

    if (endArgs >= argc) {
        cerr << "Input binary not specified." << endl
            << "Usage: " << argv[0] << USAGE;
        return false;
    }
    /* Input Binary */
    inBinary = argv[endArgs];

    endArgs++;
    if (endArgs >= argc) {
        cerr << "Output binary not specified." << endl
            << "Usage: " << argv[0] << USAGE;
        return false;
    }

    /* Rewritten Binary */
    outBinary = argv[endArgs];

    return true;
}

BPatch_function *findFuncByName (BPatch_image * appImage, char *funcName)
{
    /* fundFunctions returns a list of all functions with the name 'funcName' in the binary */
    BPatch_Vector < BPatch_function * >funcs;
    if (NULL == appImage->findFunction (funcName, funcs) || !funcs.size ()
            || NULL == funcs[0]) {
        cerr << "Failed to find " << funcName <<
            " function in the instrumentation library" << endl;
        return NULL;
    }
    return funcs[0];
}

bool insertFuncEntry (BPatch_binaryEdit * appBin, BPatch_function * curFunc,
        char *funcName, BPatch_function * instIncFunc,
        int funcId)
{
    /* Find the instrumentation points */
    vector < BPatch_point * >*funcEntry = curFunc->findPoint (BPatch_entry);
    if (NULL == funcEntry) {
        cerr << "Failed to find entry for function " << funcName << endl;
        return false;
    }

    cout << "Inserting instrumention at function entry of " << funcName << endl;
    /* Create a vector of arguments to the function
     * incCoverage function takes the function name as argument */
    BPatch_Vector < BPatch_snippet * >instArgs;
    BPatch_constExpr id (funcId);
    instArgs.push_back (&id);
    BPatch_funcCallExpr instIncExpr (*instIncFunc, instArgs);

    /* Insert the snippet at function entry */
    BPatchSnippetHandle *handle =
        appBin->insertSnippet (instIncExpr, *funcEntry, BPatch_callBefore,
                BPatch_lastSnippet);
    if (!handle) {
        cerr << "Failed to insert instrumention at function entry of " << funcName
            << endl;
        return false;
    }
    return true;

}

bool insertBBEntry (BPatch_binaryEdit * appBin, BPatch_function * curFunc,
        char *funcName, const char *moduleName,
        BPatch_function * instBBIncFunc,
        BPatch_function * registerBB, int *bbIndex,
        BPatch_Vector < BPatch_snippet * >*registerCalls)
{
    BPatch_flowGraph *appCFG = curFunc->getCFG ();
    BPatch_Set < BPatch_basicBlock * >allBlocks;
    BPatch_Set < BPatch_basicBlock * >::iterator iter;
    if (!appCFG) {
        cerr << "Failed to find CFG for function " << funcName << endl;
        return EXIT_FAILURE;
    }
    if (!appCFG->getAllBasicBlocks (allBlocks)) {
        cerr << "Failed to find basic blocks for function " << funcName << endl;
        return EXIT_FAILURE;
    } else if (allBlocks.size () == 0) {
        cerr << "No basic blocks for function " << funcName << endl;
        return EXIT_FAILURE;
    }

    /* Instrument the entry of every basic block */

    for (iter = allBlocks.begin (); iter != allBlocks.end (); iter++) {
        unsigned long address = (*iter)->getStartAddress ();
        cout << "Instrumenting Basic Block 0x" << hex << address << " of " <<
            funcName << endl;
        BPatch_Vector < BPatch_snippet * >instArgs;
        BPatch_constExpr bbId (*bbIndex);
        instArgs.push_back (&bbId);
        BPatch_point *bbEntry = (*iter)->findEntryPoint ();
        if (NULL == bbEntry) {
            cerr << "Failed to find entry for basic block at 0x" << hex << address
                << endl;
            return false;
        }
        BPatch_funcCallExpr instIncExpr (*instBBIncFunc, instArgs);
        BPatchSnippetHandle *handle =
            appBin->insertSnippet (instIncExpr, *bbEntry, BPatch_callBefore,
                    BPatch_lastSnippet);
        if (!handle) {
            cerr << "Failed to insert instrumention in basic block at 0x" << hex <<
                address << endl;
            return false;
        }

        /* Create a call to the registration function for this basic block */
        BPatch_Vector < BPatch_snippet * >regArgs;
        BPatch_constExpr bbIdReg (*bbIndex);
        regArgs.push_back (&bbIdReg);
        BPatch_constExpr coverageFunc (funcName);
        regArgs.push_back (&coverageFunc);
        BPatch_constExpr coverageModule (moduleName);
        regArgs.push_back (&coverageModule);
        BPatch_constExpr addrArg (address);
        regArgs.push_back (&addrArg);

        BPatch_funcCallExpr *regCall =
            new BPatch_funcCallExpr (*registerBB, regArgs);
        registerCalls->push_back (regCall);

        (*bbIndex)++;
    }

    return true;
}

int main (int argc, char *argv[])
{
    if (!parseArgs (argc, argv))
        return EXIT_FAILURE;

    /* Initialize list of libraries that should not be instrumented - relevant only if includeSharedLib is true */
    initSkipLibraries ();


    /* Open the specified binary for binary rewriting. 
     * When the second parameter is set to true, all the library dependencies 
     * as well as the binary are opened */

    BPatch_binaryEdit *appBin = bpatch.openBinary (inBinary, true);
    if (appBin == NULL) {
        cerr << "Failed to open binary" << endl;
        return EXIT_FAILURE;
    }

    /* Open the instrumentation library.
     * loadLibrary loads the instrumentation library into the binary image and
     * adds it as a new dynamic dependency in the rewritten library */
    const char *instLibrary = "libInst.so";
    if (!appBin->loadLibrary (instLibrary)) {
        cerr << "Failed to open instrumentation library" << endl;
        return EXIT_FAILURE;
    }

    BPatch_image *appImage = appBin->getImage ();
    /* Find code coverage functions in the instrumentation library */
    BPatch_function *instInitFunc =
        findFuncByName (appImage, (char *) "initCoverage");
    BPatch_function *registerFunc =
        findFuncByName (appImage, (char *) "registerFunc");
    BPatch_function *instIncFunc =
        findFuncByName (appImage, (char *) "incFuncCoverage");
    BPatch_function *registerBB =
        findFuncByName (appImage, (char *) "registerBB");
    BPatch_function *instBBIncFunc =
        findFuncByName (appImage, (char *) "incBBCoverage");
    BPatch_function *instExitFunc =
        findFuncByName (appImage, (char *) "exitCoverage");

    if (!instInitFunc || !instIncFunc || !instExitFunc || !instBBIncFunc
            || !registerFunc || !registerBB) {
        return EXIT_FAILURE;
    }

    /* To instrument every function in the binary
     * --> iterate over all the modules in the binary 
     * --> iterate over all functions in each modules */

    vector < BPatch_module * >*modules = appImage->getModules ();
    vector < BPatch_module * >::iterator moduleIter;
    BPatch_module *defaultModule;

    BPatch_Vector < BPatch_snippet * >registerCalls;

    int bbIndex = 0;
    int funcIndex = 0;
    for (moduleIter = modules->begin (); moduleIter != modules->end ();
            ++moduleIter) {
        char moduleName[1024];
        (*moduleIter)->getName (moduleName, 1024);

        /* if includeSharedLib is not set, skip instrumenting dependent libraries */
        if ((*moduleIter)->isSharedLib ()) {
            if (!includeSharedLib
                    || skipLibraries.find (moduleName) != skipLibraries.end ()) {
//                cout << "Skipping library: " << moduleName << endl;
                continue;
            }
        }

        /* Every binary has one default module.
         * code coverage initialize and finalize functions should be called only once.
         * Hence call them from the default module */
        if (string (moduleName).find ("DEFAULT_MODULE") != string::npos) {
            defaultModule = (*moduleIter);
        }

        cout << "Instrumenting module: " << moduleName << endl;
        vector < BPatch_function * >*allFunctions =
            (*moduleIter)->getProcedures ();
        vector < BPatch_function * >::iterator funcIter;

        /* Insert snippets at the entry of every function */
        for (funcIter = allFunctions->begin (); funcIter != allFunctions->end ();
                ++funcIter) {
            BPatch_function *curFunc = *funcIter;

            char funcName[1024];
            curFunc->getName (funcName, 1024);

            /*
             * Replace DEFAULT_MODULE with the name of the input binary in the output 
             */
            string passedModName (moduleName);
            if (passedModName.find ("DEFAULT_MODULE") != string::npos) {
                // Strip the directory
                passedModName = inBinary;
                passedModName =
                    passedModName.substr (passedModName.find_last_of ("\\/") + 1);
            }

            insertFuncEntry (appBin, curFunc, funcName, instIncFunc, funcIndex);

            /* Create a call to the registration function */
            BPatch_Vector < BPatch_snippet * >regArgs;
            BPatch_constExpr funcIdReg (funcIndex);
            regArgs.push_back (&funcIdReg);

            /* BPatch_constExpr will internally make a deep copy of the string */
            BPatch_constExpr coverageFunc (funcName);
            regArgs.push_back (&coverageFunc);
            BPatch_constExpr coverageModule (passedModName.c_str ());
            regArgs.push_back (&coverageModule);

            /* BPatch_funcCallExpr will make a copy of regArgs.
             * So, it is fine to define regArgs and BPatch_constExpr as local variables.
             */
            BPatch_funcCallExpr *regCall =
                new BPatch_funcCallExpr (*registerFunc, regArgs);
            registerCalls.push_back (regCall);

            funcIndex++;

            if (bbCoverage) {
                insertBBEntry (appBin, curFunc, funcName, passedModName.c_str (),
                        instBBIncFunc, registerBB, &bbIndex, &registerCalls);
            }
        }
    }

    /* Create argument list for initCoverage function 
     * with the number of functions the number of basic blocks */
    BPatch_Vector < BPatch_snippet * >instInitArgs;
    BPatch_constExpr numFuncs (funcIndex);
    instInitArgs.push_back (&numFuncs);
    BPatch_constExpr numBBs (bbIndex);
    instInitArgs.push_back (&numBBs);
    BPatch_funcCallExpr instInitExpr (*instInitFunc, instInitArgs);

    // Execute the function registration and basic block registration calls after
    // the call to the initialization function, i.e.,
    // initCoverage()
    // registerFunc()
    // ...
    // registerBB()
    // ...
    BPatch_sequence registerSequence (registerCalls);

    BPatch_Vector < BPatch_snippet * >initSequenceVec;
    initSequenceVec.push_back (&instInitExpr);
    initSequenceVec.push_back (&registerSequence);

    BPatch_sequence initSequence (initSequenceVec);

    /* Locate _init */
    BPatch_Vector<BPatch_function*> funcs;
    appImage->findFunction("_init", funcs);
    if (funcs.size()) {
        BPatch_Vector<BPatch_function*>::iterator fIter;
        for (fIter = funcs.begin(); fIter != funcs.end(); ++fIter) {
            BPatch_module * mod = (*fIter)->getModule();
            char modName[1024];
            mod->getName(modName, 1024);
             if (!mod->isSharedLib ()) {
                mod->getObject()->insertInitCallback(initSequence);
                cerr << "insertInitCallback on " << modName << endl;
            }
        }
    }

    /* Insert initCoverage function in the init section of the default module 
     * to be executed exactly once when the module is loaded */
//    if (!defaultModule->insertInitCallback (initSequence)) {
//        cerr << "Failed to insert init function in the module" << endl;
//        return EXIT_FAILURE;
//    }

    /* Insert exitCoverage function in the fini section of the default module 
     * to be executed exactly once when the module is unloaded */
    BPatch_Vector < BPatch_snippet * >instExitArgs;
    BPatch_constExpr varPrint (printAll);
    instExitArgs.push_back (&varPrint);
    BPatch_constExpr varBBPrint (bbCoverage ? 1 : 0);
    instExitArgs.push_back (&varBBPrint);
    BPatch_constExpr varAlpha (alphabetical);
    instExitArgs.push_back (&varAlpha);
    BPatch_funcCallExpr instExitExpr (*instExitFunc, instExitArgs);
    
    /* Locate _fini */
    funcs.clear();
    appImage->findFunction("_fini", funcs);
    if (funcs.size()) {
        BPatch_Vector<BPatch_function*>::iterator fIter;
        for (fIter = funcs.begin(); fIter != funcs.end(); ++fIter) {
            BPatch_module * mod = (*fIter)->getModule();
            char modName[1024];
            mod->getName(modName, 1024);
             if (!mod->isSharedLib ()) {
                mod->getObject()->insertFiniCallback(instExitExpr);
                cerr << "insertFiniCallback on " << modName << endl;
            }
        }
    }
    
//    if (!defaultModule->insertFiniCallback (instExitExpr)) {
//        cerr << "Failed to insert exit function in the module" << endl;
//        return EXIT_FAILURE;
//    }

    // Output the instrumented binary
    if (!appBin->writeFile (outBinary)) {
        cerr << "Failed to write output file: " << outBinary << endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
