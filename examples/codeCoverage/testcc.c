/*
 * A toy test program to demonstrate the codeCoverage tool
 */

#include <stdlib.h>

#include "libtestcc.h"

int twoCalled = 0;
int threeCalled = 0;

void three() {
    threeCalled = 1;
}

void two(int callThree) {
    if( callThree ) three();
    twoCalled = 1;
}

void one(int callThree, int callLib) {
    two(callThree);

    if( callLib ) libFooFunction(callThree);
}

int main(int argc, char *argv[]) {
    int callThree = 0, callLib = 0;

    if( argc >= 2 ) {
        callThree = 1;
    }

    if( argc >= 3 ) {
        callLib = 1;
    }

    one(callThree, callLib);

    return EXIT_SUCCESS;
}
