/*
 * A toy library to demonstrate the codeCoverage tool
 */

#include "libtestcc.h"

static int otherFunctionCalled = 0;

static void otherFunction() {
    otherFunctionCalled = 1;
}

void libFooFunction(int callOtherFunction) {
    if( callOtherFunction ) otherFunction();
}
