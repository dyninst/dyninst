#include <stdio.h>


#include "mrnet/src/Errors.h"

namespace MRN
{

static struct ErrorDefs errors[]= {
   { MRN_ENONE, MRN_INFO, MRN_IGNORE, "No Error"},
   { MRN_EBADCONFIG_IO, MRN_CRIT, MRN_ABORT, "Config File Input Error"},
   { MRN_EBADCONFIG_FMT, MRN_CRIT, MRN_ABORT, "Config File Format Error"},
   { MRN_ENETWORK_CYCLE, MRN_CRIT, MRN_ABORT,
     "Config File Error: Network cycle exists"},
   { MRN_ENETWORK_NOTCONNECTED, MRN_CRIT, MRN_ABORT,
     "Config File Error: Network not fully connected"},
   { MRN_ENETWORK_FAILURE, MRN_CRIT, MRN_ABORT, "Network Failure"}, 
   { MRN_EOUTOFMEMORY, MRN_CRIT, MRN_ABORT, "Out of Memory"}, 
   { MRN_EFMTSTR_MISMATCH, MRN_ERR, MRN_ALERT, "Format string mismatch"}, 
   { MRN_ECREATPROCFAILURE, MRN_ERR, MRN_ALERT, "Cannot create process"}, 
   { MRN_ECANNOTBINDPORT, MRN_ERR, MRN_ABORT, "Cannot bind to port"}, 
   { MRN_ESOCKETCONNECT, MRN_ERR, MRN_ABORT, "Cannot connect to socket"}, 
   { MRN_EPACKING, MRN_CRIT, MRN_ABORT, "PDR encoding/decoding failure"} };

Error::Error()
  :MRN_errno(MRN_ENONE)
{
}

bool Error::good()
{
  return (MRN_errno == MRN_ENONE);
}

bool Error::fail()
{
  return !(MRN_errno == MRN_ENONE);
}

void Error::perror(const char *str)
{
  fprintf(stderr, "%s: %s\n", str, errors[MRN_errno].msg);
  return;
}

} // namespace MRN
