/****************************************************************************
 * Copyright © 2003-2005 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.          *
 ****************************************************************************/

#if ! defined(__Error_h)
#define __Error_h

#include <list>
#include <errno.h>

#include "mrnet/MRNet.h"

namespace MRN
{

typedef enum {
    MRN_ENONE=0,
    MRN_EBADCONFIG_IO,
    MRN_EBADCONFIG_FMT,
    MRN_EBADCONFIG_CYCLE,
    MRN_EBADCONFIG_NOTCONNECTED,
    MRN_ENETWORK_FAILURE,
    MRN_EOUTOFMEMORY,
    MRN_EFMTSTR,
    MRN_ECREATPROCFAILURE,
    MRN_ECANNOTBINDPORT,
    MRN_ESOCKETCONNECT,
    MRN_EPACKING,
    MRN_EINTERNAL,
    MRN_ESYSTEM
} ErrorCode;

typedef enum{
    MRN_INFO=0,
    MRN_WARN,
    MRN_ERR,
    MRN_CRIT
} ErrorLevel;

typedef enum {
    MRN_IGNORE=0,
    MRN_ALERT,
    MRN_RETRY,
    MRN_ABORT
} ErrorResponse;

typedef struct 
{
    ErrorCode code;
    ErrorLevel level;
    ErrorResponse response;
    const char *msg;
}ErrorDef;

extern ErrorDef errors[];

class Error{
 protected:
    mutable ErrorCode MRN_errno;

 public:
    Error(): MRN_errno(MRN_ENONE) { }
    virtual ~Error() { }

    inline bool good() const {
        return (MRN_errno == MRN_ENONE);
    }

    inline bool fail() const {
        return (MRN_errno != MRN_ENONE);
    }

    inline void perror(const char *str) const {
        fprintf(stderr, "%s: %s\n", str, errors[MRN_errno].msg);
        return;
    }

    virtual void error( ErrorCode, const char *, ... ) const;
};

} // namespace MRN
#endif /* __Error_h */
