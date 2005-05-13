#ifndef _DYNINST_RT_EXPORT_H_
#define _DYNINST_RT_EXPORT_H_
#ifndef ASSEMBLER
  /*  This file contains function prototypes that may be useful for 
      dyninst users to directly have access to from their own runtime
      libraries.
  */

  /*
    DYNINSTuserMessage(void *msg, unsigned int msg_size) may be used 
    in conjunction with the dyninstAPI method 
    BPatch_process::registerUserMessageCallback(), to implement a generic
    user-defined, asynchronous communications protocol from the mutatee
    (via this runtime library) to the mutator.

    Calls to DYNINSTuserMessage() will result in <msg> (of <msg_size> bytes)
    being sent to the mutator, and then passed to the callback function
    provided by the API user via registerUserMessageCallback().

    Returns zero on success, nonzero on failure.
  */

extern int DYNINSTuserMessage(void *msg, unsigned int msg_size);


#endif
#endif
