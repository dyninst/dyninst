#include "makenan.h"
#include "headers.h"
#include <iostream>
#include <math.h>

int
main(int /*argc*/, char */*argv*/[])
{
    float test_nan = PARADYN_NaN;
    if (!isnan(test_nan))
    {
        cerr << "NaN generation failed" << endl;
        return -1;
    }

    cout << "NaN generation works" << endl;
    return 0;
}
