#include <stdio.h>


#include "mrnet/src/MC_Errors.h"

static struct MC_ErrorDefs errors[]= {
   { MC_ENONE, MC_INFO, MC_IGNORE, "No Error"},
   { MC_EBADCONFIG_IO, MC_CRIT, MC_ABORT, "Config File Input Error"},
   { MC_EBADCONFIG_FMT, MC_CRIT, MC_ABORT, "Config File Format Error"},
   { MC_ENETWORK_CYCLE, MC_CRIT, MC_ABORT,
     "Config File Error: Network cycle exists"},
   { MC_ENETWORK_NOTCONNECTED, MC_CRIT, MC_ABORT,
     "Config File Error: Network not fully connected"},
   { MC_ENETWORK_FAILURE, MC_CRIT, MC_ABORT, "Network Failure"}, 
   { MC_EOUTOFMEMORY, MC_CRIT, MC_ABORT, "Out of Memory"}, 
   { MC_EFMTSTR_MISMATCH, MC_ERR, MC_ALERT, "Format string mismatch"}, 
   { MC_ECREATPROCFAILURE, MC_ERR, MC_ALERT, "Cannot create process"}, 
   { MC_ECANNOTBINDPORT, MC_ERR, MC_ABORT, "Cannot bind to port"}, 
   { MC_ESOCKETCONNECT, MC_ERR, MC_ABORT, "Cannot connect to socket"}, 
   { MC_EPACKING, MC_CRIT, MC_ABORT, "PDR encoding/decoding failure"} };

MC_Error::MC_Error()
  :mc_errno(MC_ENONE)
{
}

bool MC_Error::good()
{
  return (mc_errno == MC_ENONE);
}

bool MC_Error::fail()
{
  return !(mc_errno == MC_ENONE);
}

void MC_Error::perror(const char *str)
{
  fprintf(stderr, "%s: %s\n", str, errors[mc_errno].msg);
  return;
}
