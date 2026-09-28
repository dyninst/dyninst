// Not typeset.  The examples beside this file are printed without the
// headers and declarations they assume, so this supplies them and pulls
// each one in for the docs build to compile.

#include "walker.h"
#include "frame.h"
#include "framestepper.h"
#include "procstate.h"
#include "symlookup.h"
#include "steppergroup.h"

#include <sys/select.h>

#include <iostream>
#include <string>
#include <vector>

using namespace std;
using namespace Dyninst;
using namespace Dyninst::Stackwalker;

// The process the third-party examples attach to.
static Dyninst::PID pid;

// An example is an excerpt: it names a thing to show it exists and stops
// there.  These two warnings fire on that by construction, so they are off
// for the included text and on everywhere else.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-parameter"

void firstParty()
{
#include "FirstParty.C"
}

void thirdParty()
{
#include "ThirdParty.C"
}

void attachWalk()
{
#include "AttachWalk.C"
}

void notificationLoop()
{
#include "NotificationLoop.C"
}

#pragma GCC diagnostic pop
