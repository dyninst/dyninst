#include <stdio.h>
#include <stdlib.h>

#include "proccontrol/h/PCErrors.h"

#include "common/h/dthread.h"

using namespace Dyninst;
using namespace ProcControlAPI;

static err_t last_error;
static const char *last_error_msg;
static signed long gen_thrd_id;
static signed long handler_thrd_id;

FILE *pctrl_err_out;
bool dyninst_debug_proccontrol = false;

const char *thrdName()
{
   signed long self = DThread::self();
   if (self == gen_thrd_id)
      return "G";
   else if (self == handler_thrd_id) 
      return "H";
   else
      return "U";
}

void setGeneratorThread(long t)
{
   gen_thrd_id = t;
}

void setHandlerThread(long t)
{
   handler_thrd_id = t;
}

bool isGeneratorThread() {
   return DThread::self() == gen_thrd_id;
}

bool isHandlerThread() {
   return DThread::self() == handler_thrd_id;
}

err_t Dyninst::ProcControlAPI::getLastError()
{
   return last_error;
}

void Dyninst::ProcControlAPI::clearLastError()
{
   last_error = 0;
}

const char *Dyninst::ProcControlAPI::getLastErrorMsg()
{
   return last_error_msg;
}

void Dyninst::ProcControlAPI::setLastError(err_t err, const char *msg)
{
   last_error = err;
   last_error_msg = msg;
}

void Dyninst::ProcControlAPI::setDebugChannel(FILE *f)
{
   pctrl_err_out = f;
}

void Dyninst::ProcControlAPI::setDebug(bool enable)
{
   dyninst_debug_proccontrol = enable; 
}

class init_debug_channel
{
public:
   init_debug_channel() 
   {
      pctrl_err_out = stderr;
      if (strcmp(getenv("DYNINST_DEBUG_PROCCONTROL"), "1") == 0) {
         setDebug(true);
      }
   }
};
static init_debug_channel idc;
