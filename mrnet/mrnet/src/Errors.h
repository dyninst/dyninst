#if ! defined(__Error_h)
#define __Error_h


enum ErrorCodes{MRN_ENONE=0,
                   MRN_EBADCONFIG_IO,
                   MRN_EBADCONFIG_FMT,
                   MRN_ENETWORK_CYCLE,
                   MRN_ENETWORK_NOTCONNECTED,
                   MRN_ENETWORK_FAILURE,
                   MRN_EOUTOFMEMORY,
                   MRN_EFMTSTR_MISMATCH,
                   MRN_ECREATPROCFAILURE,
                   MRN_ECANNOTBINDPORT,
                   MRN_ESOCKETCONNECT,
                   MRN_EPACKING};

enum ErrorLevels{MRN_INFO=0,
                    MRN_WARN,
                    MRN_ERR,
                    MRN_CRIT};

enum ErrorResponse{MRN_IGNORE=0,
                      MRN_ALERT,
                      MRN_RETRY,
                      MRN_ABORT};

struct ErrorDefs
{
 enum ErrorCodes code;
 enum ErrorLevels level;
 enum ErrorResponse response;
 const char *msg;
};

class Error{
 protected:
  bool _fail;
  enum ErrorCodes MRN_errno;

 public:
  Error();
  bool good();
  bool fail();
  void perror(const char *);
};
#endif /* __Error_h */
