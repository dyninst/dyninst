#if ! defined(__MC_Error_h)
#define __MC_Error_h


enum MC_ErrorCodes{MC_ENONE=0,
                   MC_EBADCONFIG_IO,
                   MC_EBADCONFIG_FMT,
                   MC_ENETWORK_CYCLE,
                   MC_ENETWORK_NOTCONNECTED,
                   MC_ENETWORK_FAILURE,
                   MC_EOUTOFMEMORY,
                   MC_EFMTSTR_MISMATCH,
                   MC_ECREATPROCFAILURE,
                   MC_ECANNOTBINDPORT,
                   MC_ESOCKETCONNECT,
                   MC_EPACKING};

enum MC_ErrorLevels{MC_INFO=0,
                    MC_WARN,
                    MC_ERR,
                    MC_CRIT};

enum MC_ErrorResponse{MC_IGNORE=0,
                      MC_ALERT,
                      MC_RETRY,
                      MC_ABORT};

struct MC_ErrorDefs
{
 enum MC_ErrorCodes code;
 enum MC_ErrorLevels level;
 enum MC_ErrorResponse response;
 const char *msg;
};

class MC_Error{
 protected:
  bool _fail;
  enum MC_ErrorCodes mc_errno;

 public:
  MC_Error();
  bool good();
  bool fail();
  void perror(const char *);
};
#endif /* __MC_Error_h */
