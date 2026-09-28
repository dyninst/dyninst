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

static void firstParty()
{
#include "FirstParty.C"
}

static void thirdParty()
{
#include "ThirdParty.C"
}

static void attachWalk()
{
#include "AttachWalk.C"
}

static void notificationLoop()
{
#include "NotificationLoop.C"
}
