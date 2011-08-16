/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* This is an automatically generated copyright prolog.             */
/* After initializing,  DO NOT MODIFY OR MOVE                       */
/*  --------------------------------------------------------------- */
/*                                                                  */
/* (C) Copyright IBM Corp.  2004, 2009                              */
/* IBM CPL License                                                  */
/*                                                                  */
/*  --------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */

#ifndef CIODEBUGGERPROTOCOL_H
#define CIODEBUGGERPROTOCOL_H

#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <bpcore/bgp_types.h>

//Printf wrapping added for ProcControlAPI
#define fprintf wrap_fprintf

namespace DebuggerInterface {

/*
   Protocol Version Change Log:
   1 - Initial implementation
   2 - Thread id passed in message header to reduce message traffic
   3 - Added GET_STACK_TRACE and END_DEBUG messages, Signal for detach from nodes
   4-  Added GET_PROCESS_DATA and GET_THREAD_DATA messages
   5 - Added HOLD_THREAD, RELEASE_THREAD, SIGACTION, MAP_MEM and FAST_TRAP messages
   6 - Added debugger ignore sinal function - DEBUG_IGNORE_SIG message
*/
#define BG_Debugger_PROTOCOL_VERSION 6

#define BG_DEBUGGER_WRITE_PIPE 3
#define BG_DEBUGGER_READ_PIPE  4
#define BG_PIPE_TIMEOUT 10

// Max threads passed on get active threads at one time
#define BG_Debugger_MAX_THREAD_IDS   32

//! Buffer size for GET_AUX_VECTORS request
#define BG_Debugger_AUX_VECS_BUFFER 1024

//! Number of stack frames returned from compute node
#define BG_Debugger_MAX_STACK_FRAMES 400 


// Some typedefs to insure consistency later.

typedef uint32_t BG_NodeNum_t;    // Which compute node under an I/O node
typedef uint32_t BG_ThreadID_t;   // Which thread in the compute node
typedef uint32_t BG_GPR_t;        // GPRs are 32 bit unsigned
typedef uint32_t BG_Addr_t;       // 32 bit virtual address



/* Register numbers

   The value of each enum is magically the same as the offset in
   words into the GPRSet_t structure.  The system is similar to,
   but not necessarily the same as used by ptrace.h.
 
*/

typedef enum {

  BG_GPR0 = 0,
  BG_GPR1,
  BG_GPR2,
  BG_GPR3,
  BG_GPR4,
  BG_GPR5,
  BG_GPR6,
  BG_GPR7,
  BG_GPR8,
  BG_GPR9,
  BG_GPR10,
  BG_GPR11,
  BG_GPR12,
  BG_GPR13,
  BG_GPR14,
  BG_GPR15,
  BG_GPR16,
  BG_GPR17,
  BG_GPR18,
  BG_GPR19,
  BG_GPR20,
  BG_GPR21,
  BG_GPR22,
  BG_GPR23,
  BG_GPR24,
  BG_GPR25,
  BG_GPR26,
  BG_GPR27,
  BG_GPR28,
  BG_GPR29,
  BG_GPR30,
  BG_GPR31 = 31,

  BG_FPSCR      = 32,
  BG_LR         = 33,
  BG_CR         = 34,
  BG_XER        = 35,
  BG_CTR        = 36,
  BG_IAR        = 37,
  BG_MSR        = 38,
  BG_DEAR       = 39,
  BG_ESR        = 40

} BG_GPR_Num_t;





/* Floating point register numbers

   We have 'double hummer'.  We will pretend that we have 32 registers,
   each 128 bits (4 words) wide.  Once again, the enum value is the
   offset into the FPRSet_t struct.

*/

typedef enum {

  BG_FPR0 = 0,
  BG_FPR1,
  BG_FPR2,
  BG_FPR3,
  BG_FPR4,
  BG_FPR5,
  BG_FPR6,
  BG_FPR7,
  BG_FPR8,
  BG_FPR9,
  BG_FPR10,
  BG_FPR11,
  BG_FPR12,
  BG_FPR13,
  BG_FPR14,
  BG_FPR15,
  BG_FPR16,
  BG_FPR17,
  BG_FPR18,
  BG_FPR19,
  BG_FPR20,
  BG_FPR21,
  BG_FPR22,
  BG_FPR23,
  BG_FPR24,
  BG_FPR25,
  BG_FPR26,
  BG_FPR27,
  BG_FPR28,
  BG_FPR29,
  BG_FPR30,
  BG_FPR31 = 31

} BG_FPR_Num_t;


/* MsgType

   This doesn't match the ptrace interface very well.
   Ptrace doesn't define most of these primitives.
   Therefore, the enums are completely arbitrary.

   Notice that except for SIGNAL_ENCOUNTERED, each
   request from the debugger has a corresponding
   acknowledgement message.

*/

typedef enum { GET_REG = 0,
               GET_ALL_REGS,		// request gprs and other non-fp regs 
               SET_REG,			// set a specific register
               GET_MEM,			// get memory values
               SET_MEM,			// set memory
               GET_FLOAT_REG,
               GET_ALL_FLOAT_REGS,
               SET_FLOAT_REG,
               SINGLE_STEP,		// step one instruction
               CONTINUE,		// tell compute node to continue running
               KILL,			// send a signal w/ implicit continue
               ATTACH,			// mark compute node
	       DETACH,			// unmark compute node

               GET_REG_ACK,
               GET_ALL_REGS_ACK,	// sent when compute node responds
               SET_REG_ACK,		// send when compute node responds
               GET_MEM_ACK,		// sent when compute node responds
               SET_MEM_ACK,		// sent when compute node responds
               GET_FLOAT_REG_ACK,
               GET_ALL_FLOAT_REGS_ACK,
               SET_FLOAT_REG_ACK,
               SINGLE_STEP_ACK,		// sent when compute node completes step
               CONTINUE_ACK,		// sent when compute node is told
               KILL_ACK,		// send when signal is sent
               ATTACH_ACK,		// sent after compute node is marked
	       DETACH_ACK,		// sent after compute node is unmarked

               SIGNAL_ENCOUNTERED,	// sent when a signal is encountered

               PROGRAM_EXITED,		// with implicit detach

               VERSION_MSG,
               VERSION_MSG_ACK,

               GET_DEBUG_REGS,
               GET_DEBUG_REGS_ACK,

               SET_DEBUG_REGS,
               SET_DEBUG_REGS_ACK,

               GET_THREAD_INFO,
               GET_THREAD_INFO_ACK,
               THREAD_ALIVE,
               THREAD_ALIVE_ACK,
               GET_THREAD_ID,
               GET_THREAD_ID_ACK,
               SET_THREAD_OPS,
               SET_THREAD_OPS_ACK,

               GET_REGS_AND_FLOATS,  // Get regs and floating point registers
               GET_REGS_AND_FLOATS_ACK,  // sent when compute node responds

               GET_AUX_VECTORS,  // Request to get the aux vectors
               GET_AUX_VECTORS_ACK, // Send when compute node responds 
              
               GET_STACK_TRACE,  // Request to get the stack traceback
               GET_STACK_TRACE_ACK,  // Sent when compute node responds

               END_DEBUG,  // Indicate done debugging
               END_DEBUG_ACK,  // Sent when CIOD acknowledges ending of debug

               GET_PROCESS_DATA,
               GET_PROCESS_DATA_ACK,

               GET_THREAD_DATA,
               GET_THREAD_DATA_ACK,

               HOLD_THREAD,
               HOLD_THREAD_ACK,

               RELEASE_THREAD,
               RELEASE_THREAD_ACK,

               SIGACTION,
               SIGACTION_ACK,

               MAP_MEM,
               MAP_MEM_ACK,

               FAST_TRAP,
               FAST_TRAP_ACK,

               DEBUG_IGNORE_SIG,
               DEBUG_IGNORE_SIG_ACK,

               THIS_SPACE_FOR_RENT

} BG_MsgType_t;


/* GPRSet_t

   This is the set of general purpose registers.

*/

typedef struct {

  uint32_t gpr[32];     // gprs
  uint32_t fpscr;       // fpscr
  uint32_t lr;          // link
  uint32_t cr;          // condition reg
  uint32_t xer;         // xer
  uint32_t ctr;         // count register
  uint32_t iar;         // pc
  uint32_t msr;         // machine status register
  uint32_t dear;        // data exception address register
  uint32_t esr;         // exception syndrome register
  
} BG_GPRSet_t;



/* FPRSet_t

   Our FPRs are a little bit different because of the double hummer.
   We actually have 2 sets of FPRs.

   The convention that we'll use is to treat the FPRs as one set of
   32, with each FPR being four words instead of two.

*/


typedef struct {
  uint32_t w0;    // First word of FPR in first FPR set
  uint32_t w1;    // Second word of FPR in first FPR set
  uint32_t w2;    // First word of FPR in second FPR set
  uint32_t w3;    // Second word of FPR in second FPR set
} BG_FPR_t;

typedef struct {
  BG_FPR_t fprs[32];
} BG_FPRSet_t;



typedef struct {
  uint32_t DBCR0;    // Debug Control Register 0
  uint32_t DBCR1;    // Debug Control Register 1
  uint32_t DBCR2;    // Debug Control Register 2
  uint32_t DBSR;     // Debug Status Register
  uint32_t IAC1;     // Instruction Address Compare Register 1
  uint32_t IAC2;     // Instruction Address Compare Register 2
  uint32_t IAC3;     // Instruction Address Compare Register 3
  uint32_t IAC4;     // Instruction Address Compare Register 4
  uint32_t DAC1;     // Data Address Compare Register 1
  uint32_t DAC2;     // Data Address Compare Register 2
  uint32_t DVC1;     // Data Value Compare Register 1
  uint32_t DVC2;     // Data Value Compare Register 2
} BG_DebugSet_t;



// Structure for stack data
typedef struct
{
  BG_Addr_t frameAddr;
  BG_Addr_t savedLR;
} BG_Stack_Info_t;


// Structure for process data.
typedef struct {
  uint32_t rank;                       // MPI rank
  uint32_t tgid;                       // Thread group id
  uint32_t xCoord;                     // X coordinate
  uint32_t yCoord;                     // Y coordinate
  uint32_t zCoord;                     // Z coordinate
  uint32_t tCoord;                     // T coordinate
  BG_Addr_t sharedMemoryStartAddr;     // Shared memory region start address
  BG_Addr_t sharedMemoryEndAddr;       // Shared memory region end address
  BG_Addr_t persistMemoryStartAddr;    // Persistent memory region start address
  BG_Addr_t persistMemoryEndAddr;      // Persistent memory region end address
  BG_Addr_t heapStartAddr;             // Heap start address
  BG_Addr_t heapEndAddr;               // Heap end address 
  BG_Addr_t heapBreakAddr;             // Heap break address
  BG_Addr_t mmapStartAddr;             // Memory map start address
  BG_Addr_t mmapEndAddr;               // Memory map end address
  struct timeval jobTime;              // Time job has been running
} BG_Process_Data_t;

// State of a thread.
typedef enum {
  Running = 1,                         // Thread is running executable code
  Sleeping,                            // Thread is sleeping
  Waiting,                             // Thread is waiting for event
  Zombie,                              // Thread is ended
  Idle                                 // Thread is available 
} BG_Thread_State_t;

// Structure for thread data.
typedef struct {
  BG_ThreadID_t threadID;              // Thread id
  int core;                            // Core number
  BG_Thread_State_t state;             // Thread state
  BG_Addr_t stackStartAddr;            // Stack start address
  BG_Addr_t stackEndAddr;              // Stack end address
  BG_Addr_t guardStartAddr;            // Guard page start address
  BG_Addr_t guardEndAddr;              // Guard page end address
  BG_GPRSet_t gprs;                    // Set of general purpose registers
  BG_FPRSet_t fprs;                    // Set of floating point registers
  BG_DebugSet_t debugRegisters;        // Set of debug registers
  uint32_t numStackFrames;             // Number of stack frame addr/saved lr entries returned
  BG_Stack_Info_t stackInfo[BG_Debugger_MAX_STACK_FRAMES]; // Stack trace back
} BG_Thread_Data_t;


typedef enum {

  RC_NO_ERROR          = 0,
  RC_NOT_ATTACHED      = 1,
  RC_NOT_RUNNING       = 2,
  RC_BAD_NODE          = 3,
  RC_BAD_THREAD        = 4,
  RC_BAD_COMMAND       = 5,
  RC_BAD_REGISTER      = 6,
  RC_NOT_APP_SPACE     = 7,
  RC_LEN_TOO_LONG      = 8,
  RC_DENIED            = 9,
  RC_BAD_SIGNAL        = 10,
  RC_NOT_STOPPED       = 11,
  RC_NOT_INITIALIZED   = 12,
  RC_TIMEOUT           = 13
  
} BG_ErrorCode_t;




// Externs for managing the pipe.  Use by a client is optional.
//
// DebuggerReadStarted and DebuggerWriteStarted: get set whenever you call
// our public method to read or write on the debugger pipe.  An alarm
// handler can look to see how long you've been waiting on the pipe.
//
// AbortPipeIO: If the alarm handler wants to abort a read or write on
// the pipe it can set this.  When the loop is resumed it will check
// this flag and end the loop if necessary.

extern volatile time_t DebuggerReadStarted;
extern volatile time_t DebuggerWriteStarted;
extern volatile int AbortPipeIO;


/* Debugger_Msg_t

   This is the packet header for the pipe.  All messages use this.

     messageType is self explanatory
     nodeNumber is the target compute node
     thread is the thread on the compute node
     sequence can be used to keep track of packet flow
     returnCode might be needed
     dataLength is the size in chars of the payload

   The 'payload' should follow the message header, and be received
   into a buffer of size dataLength.

   Not all messages have payloads.  If your message doesn't have
   a payload, set dataLength to 0.

   'sequence' is probably best used as a 'packet count' or a sequence
   number.  This way you can match acknowledgement packets with the
   original packet if things aren't being done in lockstep.

   'returnCode' isn't architected yet, but we probably need it.  If your
   GET_MEM request fails how else will you know?

   'dataStartsHere' is a placeholder.  As a result, to get the true size
   of this data structure you have to subtract 1 byte.

*/

#define BG_Debugger_Msg_MAX_SIZE     4096
#define BG_Debugger_Msg_HEADER_SIZE    24
#define BG_Debugger_Msg_MAX_PAYLOAD_SIZE (BG_Debugger_Msg_MAX_SIZE-BG_Debugger_Msg_HEADER_SIZE)
#define BG_Debugger_Msg_MAX_MEM_SIZE 4064



class BG_Debugger_Msg {

  public:

    typedef struct {

      BG_MsgType_t      messageType;
      BG_NodeNum_t      nodeNumber;
      BG_ThreadID_t     thread;
      uint32_t           sequence;
      uint32_t           returnCode;
      uint32_t           dataLength;   // excluding this header

    } Header;

    Header header;

    typedef union {

      struct {
        BG_GPR_Num_t      registerNumber;
      } GET_REG;

      struct {
        BG_GPR_Num_t      registerNumber;
        BG_GPR_t          value;
      } GET_REG_ACK;

      struct {
      } GET_ALL_REGS;

      struct {
        BG_GPRSet_t       gprs;
      } GET_ALL_REGS_ACK;

      struct {
        BG_GPR_Num_t     registerNumber;
        BG_GPR_t         value;
      } SET_REG;

      struct {
        BG_GPR_Num_t     registerNumber;
      } SET_REG_ACK;  

      struct {
        BG_Addr_t        addr;
        uint32_t          len;
      } GET_MEM;

      struct {
        BG_Addr_t        addr;
        uint32_t          len;
        unsigned char     data[BG_Debugger_Msg_MAX_MEM_SIZE];
      } GET_MEM_ACK;

      struct {
        BG_Addr_t        addr;
        uint32_t          len;
        unsigned char     data[BG_Debugger_Msg_MAX_MEM_SIZE];
      } SET_MEM;

      struct {
        BG_Addr_t        addr;
        uint32_t          len;
      } SET_MEM_ACK;

      struct {
        BG_FPR_Num_t     registerNumber;
      } GET_FLOAT_REG;

      struct {
        BG_FPR_Num_t     registerNumber;
        BG_FPR_t         value;
      } GET_FLOAT_REG_ACK;  

      struct {
      } GET_ALL_FLOAT_REGS;

      struct {
        BG_FPRSet_t      fprs;
      } GET_ALL_FLOAT_REGS_ACK;

      struct {
        BG_FPR_Num_t     registerNumber;
        BG_FPR_t         value;
      } SET_FLOAT_REG;

      struct {
        BG_FPR_Num_t     registerNumber;
      } SET_FLOAT_REG_ACK;  

      struct {
      } SINGLE_STEP;

      struct {
      } SINGLE_STEP_ACK;

      struct {
         uint32_t          signal;
      } CONTINUE;

      struct {
      } CONTINUE_ACK;

      struct {
         uint32_t          signal;
      } KILL;

      struct {
      } KILL_ACK;

      struct {
      } ATTACH;

      struct {
      } ATTACH_ACK;

      struct {
      } DETACH;

      struct {
      } DETACH_ACK;

      struct {
        uint32_t          signal;
      } SIGNAL_ENCOUNTERED;

      struct {
        int32_t           type;  // 0 = exit, 1 = signal
        int32_t           rc;
      } PROGRAM_EXITED;

      struct {
      } VERSION_MSG;

      struct {
        uint32_t          protocolVersion;
        uint32_t          numPhysicalProcessors;
        uint32_t          numLogicalProcessors;
      } VERSION_MSG_ACK;

      struct {
      } GET_DEBUG_REGS;

      struct {
        BG_DebugSet_t   debugRegisters;
      } GET_DEBUG_REGS_ACK;

      struct {
        BG_DebugSet_t   debugRegisters;
      } SET_DEBUG_REGS;

      struct {
      } SET_DEBUG_REGS_ACK;

      struct {
      } GET_THREAD_INFO;

      struct {
        uint32_t numThreads;
        uint32_t threadIDS[BG_Debugger_MAX_THREAD_IDS];
      } GET_THREAD_INFO_ACK;

      struct {
        uint32_t tid;
      } THREAD_ALIVE;

      struct {
      } THREAD_ALIVE_ACK;

      struct {
      } GET_THREAD_ID;

      struct {
        uint32_t tid;
      } GET_THREAD_ID_ACK;

      struct {
        uint32_t tid;
        int32_t operation;  // 0='g' operations 1='c' operations
      } SET_THREAD_OPS;

      struct {
      } SET_THREAD_OPS_ACK;

      struct {
      } GET_REGS_AND_FLOATS;

      struct {
        BG_GPRSet_t      gprs;
        BG_FPRSet_t      fprs;
      } GET_REGS_AND_FLOATS_ACK;

      struct {
        uint32_t auxVecBufferOffset; // Requested offset within the available AuxVec data to begin retrieval
        uint32_t auxVecBufferLength; // Requested length of data to be returned
      } GET_AUX_VECTORS;

      struct {
        uint32_t auxVecData[BG_Debugger_AUX_VECS_BUFFER/sizeof(uint32_t)]; 
        uint32_t auxVecBufferOffset; // Offset within the available AuxVec data of retrieved data
        uint32_t auxVecBufferLength; // Number of bytes of aux vec data stored within the auxVecData buffer
        bool endOfVecData;           // Indicator set to true if the end of the AuxVec data was reached
      } GET_AUX_VECTORS_ACK;

      struct {
      } GET_STACK_INFO;

      struct {
        uint32_t lr;  // Current link register
        uint32_t iar;  // Current instruction address
        uint32_t stackFrame;  // Current stack frame pointer(R1)
        uint32_t numStackFrames;  // Number of stack frame addr/saved lr entries returned
        BG_Stack_Info_t stackInfo[ BG_Debugger_MAX_STACK_FRAMES ];
      } GET_STACK_INFO_ACK;

      struct {
      } END_DEBUG;

      struct {
      } END_DEBUG_ACK;

      struct {
      } GET_PROCESS_DATA;

      struct {
        BG_Process_Data_t processData;
        uint32_t numThreads;
        BG_ThreadID_t threadIDS[BG_Debugger_MAX_THREAD_IDS];
      } GET_PROCESS_DATA_ACK;

      struct {
      } GET_THREAD_DATA;

      struct {
         BG_Thread_Data_t threadData;
      } GET_THREAD_DATA_ACK;

      struct {
        uint32_t timeout;
      } HOLD_THREAD;

      struct {
      } HOLD_THREAD_ACK;

      struct {
      } RELEASE_THREAD;

      struct {
      } RELEASE_THREAD_ACK;

      struct {
        uint32_t signum;        // signal number. Only supported value is SIGTRAP
        __sighandler_t handler; // SIGTRAP signal handler function
        uint32_t       mask;    // signal mask  (see syscall sigaction)
        uint32_t       flags;   // signal flags (see syscall sigaction)
      } SIGACTION;

      struct {
      } SIGACTION_ACK;

      struct {
        uint32_t          len;
      } MAP_MEM;

      struct {
        BG_Addr_t        addr;
      } MAP_MEM_ACK;

      struct {
        bool         enable;
      } FAST_TRAP;

      struct {
      } FAST_TRAP_ACK;

      struct {
        sigset_t     ignoreSet;
      } DEBUG_IGNORE_SIG;

      struct {
      } DEBUG_IGNORE_SIG_ACK;

      unsigned char      dataStartsHere;

    } DataArea;

    DataArea dataArea;

    // Ctor

    BG_Debugger_Msg( void ) { header.messageType = THIS_SPACE_FOR_RENT; }


    BG_Debugger_Msg( BG_MsgType_t type,
		        BG_NodeNum_t node,
		       	BG_ThreadID_t thread,
		       	uint32_t sequence,
		       	uint32_t returnCode )
    {
      header.messageType = type;
      header.nodeNumber = node;
      header.thread = thread;
      header.sequence = sequence;
      header.returnCode = returnCode;
    } 

    static BG_Debugger_Msg generateErrorPacket( BG_Debugger_Msg &original, BG_ErrorCode_t ec )
    {

       BG_Debugger_Msg errMsg;

       errMsg.header.nodeNumber = original.header.nodeNumber;
       errMsg.header.thread     = original.header.thread;
       errMsg.header.sequence   = original.header.sequence;
       errMsg.header.returnCode = ec;


       switch ( original.header.messageType ) {

         case GET_REG: {
           errMsg.header.messageType = GET_REG_ACK;
           errMsg.header.dataLength  = sizeof( errMsg.dataArea.GET_REG_ACK );
           errMsg.dataArea.GET_REG_ACK.registerNumber = original.dataArea.GET_REG.registerNumber;
           errMsg.dataArea.GET_REG_ACK.value = 0xDEADBEEF;
           break;
         }

         case GET_ALL_REGS: {
           errMsg.header.messageType = GET_ALL_REGS_ACK;
           errMsg.header.dataLength = 0;
           break;
         }

         case SET_REG: {
           errMsg.header.messageType = SET_REG_ACK;
           errMsg.header.dataLength  = sizeof( errMsg.dataArea.SET_REG_ACK );
           errMsg.dataArea.SET_REG_ACK.registerNumber = original.dataArea.SET_REG.registerNumber;
           break;
         }

         case GET_MEM: {
           errMsg.header.messageType = GET_MEM_ACK;
           errMsg.header.dataLength  = sizeof( errMsg.dataArea.GET_MEM_ACK ) - BG_Debugger_Msg_MAX_MEM_SIZE;
           errMsg.dataArea.GET_MEM_ACK.addr = original.dataArea.GET_MEM.addr;
           errMsg.dataArea.GET_MEM_ACK.len  = original.dataArea.GET_MEM.len;
           break;
         }

         case SET_MEM: {
           errMsg.header.messageType = SET_MEM_ACK;
           errMsg.header.dataLength  = sizeof( errMsg.dataArea.SET_MEM_ACK );
           errMsg.dataArea.SET_MEM_ACK.addr = original.dataArea.SET_MEM.addr;
           errMsg.dataArea.SET_MEM_ACK.len  = original.dataArea.SET_MEM.len;
           break;
         }

         case GET_FLOAT_REG: {
           errMsg.header.messageType = GET_FLOAT_REG_ACK;
           errMsg.header.dataLength  = sizeof( errMsg.dataArea.GET_FLOAT_REG_ACK );
           errMsg.dataArea.GET_FLOAT_REG_ACK.registerNumber = original.dataArea.GET_FLOAT_REG.registerNumber;
           errMsg.dataArea.GET_FLOAT_REG_ACK.value.w0 = 0xDEADBEEF;
           errMsg.dataArea.GET_FLOAT_REG_ACK.value.w1 = 0xDEADBEEF;
           errMsg.dataArea.GET_FLOAT_REG_ACK.value.w2 = 0xDEADBEEF;
           errMsg.dataArea.GET_FLOAT_REG_ACK.value.w3 = 0xDEADBEEF;
           break;
         }

         case GET_ALL_FLOAT_REGS: {
           errMsg.header.messageType = GET_ALL_FLOAT_REGS_ACK;
           errMsg.header.dataLength  = 0;
           break;
         }

         case SET_FLOAT_REG: {
           errMsg.header.messageType = SET_FLOAT_REG_ACK;
           errMsg.header.dataLength  = sizeof( errMsg.dataArea.SET_FLOAT_REG_ACK );
           errMsg.dataArea.SET_FLOAT_REG_ACK.registerNumber = original.dataArea.SET_FLOAT_REG.registerNumber;
           break;
         }

         case SINGLE_STEP: {
           errMsg.header.messageType = SINGLE_STEP_ACK;
           errMsg.header.dataLength  = sizeof( errMsg.dataArea.SINGLE_STEP_ACK );
           break;
         }

         case CONTINUE: {
           errMsg.header.messageType = CONTINUE_ACK;
           errMsg.header.dataLength  = sizeof( errMsg.dataArea.CONTINUE_ACK );
           break;
         }

         case KILL: {
           errMsg.header.messageType = KILL_ACK;
           errMsg.header.dataLength  = sizeof( errMsg.dataArea.KILL_ACK );
           break;
         }

         case ATTACH: {
           errMsg.header.messageType = ATTACH_ACK;
           errMsg.header.dataLength  = sizeof( errMsg.dataArea.ATTACH_ACK );
           break;
         }

         case DETACH: {
           errMsg.header.messageType = DETACH_ACK;
           errMsg.header.dataLength  = sizeof( errMsg.dataArea.DETACH_ACK );
           break;
         }

         case GET_DEBUG_REGS: {
           errMsg.header.messageType = GET_DEBUG_REGS_ACK;
           errMsg.header.dataLength  = 0;
           break;
         }

         case SET_DEBUG_REGS: {
           errMsg.header.messageType = SET_DEBUG_REGS_ACK;
           errMsg.header.dataLength  = 0;
           break;
         }

         case GET_THREAD_INFO: {
           errMsg.header.messageType = GET_THREAD_INFO_ACK;
           errMsg.header.dataLength  = 0;
           break;
         }

         case THREAD_ALIVE: {
           errMsg.header.messageType = THREAD_ALIVE_ACK;
           errMsg.header.dataLength  = sizeof( errMsg.dataArea.THREAD_ALIVE_ACK );
           break;
         }

         case GET_THREAD_ID: {
           errMsg.header.messageType = GET_THREAD_ID_ACK;
           errMsg.header.dataLength  = 0;
           break;
         }

         case SET_THREAD_OPS: {
           errMsg.header.messageType = SET_THREAD_OPS_ACK;
           errMsg.header.dataLength  = 0;
           break;
         }

         case GET_REGS_AND_FLOATS: {
           errMsg.header.messageType = GET_ALL_REGS_ACK;
           errMsg.header.dataLength = 0;
           break;
         }

         case GET_AUX_VECTORS: {
           errMsg.header.messageType = GET_AUX_VECTORS_ACK;
           errMsg.header.dataLength = 0;
           break;
         }

         case GET_STACK_TRACE: {
           errMsg.header.messageType = GET_STACK_TRACE_ACK;
           errMsg.header.dataLength = 0;
           break;
         }

         case END_DEBUG: {
           errMsg.header.messageType = END_DEBUG_ACK;
           errMsg.header.dataLength  = 0;
           break;
         }

         case GET_PROCESS_DATA: {
           errMsg.header.messageType = GET_PROCESS_DATA_ACK;
           errMsg.header.dataLength = 0;
           break;
         }

         case GET_THREAD_DATA: {
           errMsg.header.messageType = GET_THREAD_DATA_ACK;
           errMsg.header.dataLength = 0;
           break;
         }

         default: {
           errMsg.header.messageType = original.header.messageType;
           errMsg.header.dataLength = 0;
           break;
         }

       }

       return errMsg;

    }

    //! \brief  Read a debugger message from a descriptor.
    //! \param  fd Descriptor to read from.
    //! \param  msg Reference to debugger message.
    //! \return True when message was read successfully, false if there was an error.

    static bool readFromFd( int fd, BG_Debugger_Msg &msg )
    {
       return readFromFd_p( fd, msg, NULL );
    }

    //! \brief  Read a debugger message from a descriptor.
    //! \param  fd Descriptor to read from.
    //! \param  msg Reference to debugger message.
    //! \param  abortPipeIO Pointer to integer that indicates whether to abort read on error.
    //! \return True when message was read successfully, false if there was an error.

    static bool readFromFd( int fd, BG_Debugger_Msg &msg, volatile int *abortPipeIO )
    {
       return readFromFd_p( fd, msg, abortPipeIO );
    }

    //! \brief  Write a debugger message to a descriptor.
    //! \param  fd Descriptor to write to.
    //! \param  msg Reference to debugger message.
    //! \return True when message was written successfully, false if there was an error.

    static bool writeOnFd( int fd, BG_Debugger_Msg &msg )
    {
       return writeOnFd_p( fd, msg, NULL );
    }

    //! \brief  Write a debugger message to a descriptor.
    //! \param  fd Descriptor to write to.
    //! \param  msg Reference to debugger message.
    //! \param  abortPipeIO Pointer to integer that indicates whether to abort read on error.
    //! \return True when message was written successfully, false if there was an error.

    static bool writeOnFd( int fd, BG_Debugger_Msg &msg, volatile int *abortPipeIO )
    {
       return writeOnFd_p( fd, msg, abortPipeIO );
    }

    //! \brief  Get a string describing a debugger message type.
    //! \param  type Debugger message type.
    //! \return Pointer to string.

    static const char *getMessageName(BG_MsgType_t type)
    {
       static const char *BG_Packet_Names[] = 
       {
          "GET_REG",
          "GET_ALL_REGS",
          "SET_REG",
          "GET_MEM",
          "SET_MEM",
          "GET_FLOAT_REG",
          "GET_ALL_FLOAT_REGS",
          "SET_FLOAT_REG",
          "SINGLE_STEP",
          "CONTINUE",
          "KILL",
          "ATTACH",
          "DETACH",
          "GET_REG_ACK",
          "GET_ALL_REGS_ACK",
          "SET_REG_ACK",
          "GET_MEM_ACK",
          "SET_MEM_ACK",
          "GET_FLOAT_REG_ACK",
          "GET_ALL_FLOAT_REGS_ACK",
          "SET_FLOAT_REG_ACK",
          "SINGLE_STEP_ACK",
          "CONTINUE_ACK",
          "KILL_ACK",
          "ATTACH_ACK",
          "DETACH_ACK",
          "SIGNAL_ENCOUNTERED",
          "PROGRAM_EXITED",
          "VERSION_MSG",
          "VERSION_MSG_ACK",
          "GET_DEBUG_REGS",
          "GET_DEBUG_REGS_ACK",
          "SET_DEBUG_REGS",
          "SET_DEBUG_REGS_ACK",
          "GET_THREAD_INFO",
          "GET_THREAD_INFO_ACK",
          "THREAD_ALIVE",
          "THREAD_ALIVE_ACK",
          "GET_THREAD_ID",
          "GET_THREAD_ID_ACK",
          "SET_THREAD_OPS",
          "SET_THREAD_OPS_ACK",
          "GET_REGS_AND_FLOATS",
          "GET_REGS_AND_FLOATS_ACK",
          "GET_AUX_VECTORS",
          "GET_AUX_VECTORS_ACK",
          "GET_STACK_TRACE",
          "GET_STACK_TRACE_ACK",
          "END_DEBUG",
          "END_DEBUG_ACK",
          "GET_PROCESS_DATA",
          "GET_PROCESS_DATA_ACK",
          "GET_THREAD_DATA",
          "GET_THREAD_DATA_ACK",
          "HOLD_THREAD",
          "HOLD_THREAD_ACK",
          "RELEASE_THREAD",
          "RELEASE_THREAD_ACK",
          "SIGACTION",
          "SIGACTION_ACK",
          "MAP_MEM",
          "MAP_MEM_ACK",
          "FAST_TRAP",
          "FAST_TRAP_ACK",
          "DEBUG_IGNORE_SIG",
          "DEBUG_IGNORE_SIG_ACK"
       };

       if ((type >= GET_REG) && (type < THIS_SPACE_FOR_RENT)) {
          return BG_Packet_Names[type];
       }
       else {
          return "UNKNOWN";
       }

    }

    //! \brief  Print details about a debugger message to standard output.
    //! \param  msg Reference to debugger message.
    //! \return Nothing.

    static void dump( BG_Debugger_Msg &msg )
    {
       dump( msg, stdout );
    }

    //! \brief  Print details about a debugger message to the specified file.
    //! \param  msg Reference to debugger message.
    //! \param  outfile Pointer to file for printing message.
    //! \return Nothing.

    static void dump( BG_Debugger_Msg &msg, FILE *outfile )
    {
       fprintf( outfile, "\n" );

       fprintf( outfile, "Type: %s from node: %d, return code: %d\n",
                getMessageName(msg.header.messageType),
                msg.header.nodeNumber,
                msg.header.returnCode );

       switch ( msg.header.messageType ) {

         case GET_REG: {
           fprintf( outfile, "  Register number: %d\n", msg.dataArea.GET_REG.registerNumber );
           break;
         }

         case GET_REG_ACK: {
           fprintf( outfile, "  Register number: %d    Value: %08x\n", msg.dataArea.GET_REG.registerNumber, msg.dataArea.GET_REG_ACK.value );
           break;
         }

         case GET_ALL_REGS_ACK: {
           dumpGPRSet( &msg.dataArea.GET_ALL_REGS_ACK.gprs, outfile );
           break;
         }


         case SET_REG: {
           fprintf( outfile, "  Register number: %d    Value: %08x\n", msg.dataArea.SET_REG.registerNumber, msg.dataArea.SET_REG.value );
           break;
         }

         case SET_REG_ACK: {
           fprintf( outfile, "  Register number: %d\n", msg.dataArea.SET_REG_ACK.registerNumber );
           break;
         }

         case GET_MEM: {
           fprintf( outfile, "  Memory address: %08x    Length: %d\n  Vals: ", msg.dataArea.GET_MEM.addr, msg.dataArea.GET_MEM.len );
           break;
         }

         case GET_MEM_ACK: {
           fprintf( outfile, "  Memory address: %08x    Length: %d\n  Vals: ", msg.dataArea.GET_MEM_ACK.addr, msg.dataArea.GET_MEM_ACK.len );
           for ( unsigned int i = 0; i < msg.dataArea.GET_MEM_ACK.len; i++ ) fprintf( outfile, "%02x ", msg.dataArea.GET_MEM_ACK.data[i] );
           fprintf( outfile, "\n" );
           break;
         }

         case SET_MEM: {
           fprintf( outfile, "  Memory address: %08x    Length: %d\n  Vals: ", msg.dataArea.SET_MEM.addr, msg.dataArea.SET_MEM.len );
           for ( unsigned int i = 0; i < msg.dataArea.SET_MEM.len; i++ ) fprintf( outfile, "%02x ", msg.dataArea.SET_MEM.data[i] );
           fprintf( outfile, "\n" );
           break;
         }

         case SET_MEM_ACK: {
           fprintf( outfile, "  Memory address: %08x    Length: %d\n", msg.dataArea.SET_MEM_ACK.addr, msg.dataArea.SET_MEM_ACK.len );
           break;
         }

         case CONTINUE: {
           fprintf( outfile, "  Continue with signal number: %d\n", msg.dataArea.CONTINUE.signal );
           break;
         }

         case KILL: {
           fprintf( outfile, "  Signal number: %d\n", msg.dataArea.KILL.signal );
           break;
         }

         case SIGNAL_ENCOUNTERED: {
           fprintf( outfile, "  Signal number: %d\n", msg.dataArea.SIGNAL_ENCOUNTERED.signal );
           break;
         }

         case PROGRAM_EXITED: {
           fprintf( outfile, "  Exit=0,Signal=1: %d   Return code: %d\n", msg.dataArea.PROGRAM_EXITED.type, msg.dataArea.PROGRAM_EXITED.rc );
           break;
         }

         case GET_ALL_FLOAT_REGS_ACK: {
           dumpFPRSet( &msg.dataArea.GET_ALL_FLOAT_REGS_ACK.fprs, outfile );
           break;
         }

         case VERSION_MSG_ACK: {
           fprintf( outfile, "  Protocol version: %u   Physical Processors: %u  Logical Processors: %u\n",
                    msg.dataArea.VERSION_MSG_ACK.protocolVersion,
                    msg.dataArea.VERSION_MSG_ACK.numPhysicalProcessors,
                    msg.dataArea.VERSION_MSG_ACK.numLogicalProcessors );
           break;
         }

         case GET_DEBUG_REGS_ACK: {
           dumpDebugSet( &msg.dataArea.GET_DEBUG_REGS_ACK.debugRegisters, outfile );
           break;
         }

         case SET_DEBUG_REGS: {
           dumpDebugSet( &msg.dataArea.SET_DEBUG_REGS.debugRegisters, outfile );
           break;
         }

         case GET_THREAD_INFO_ACK: {
           fprintf( outfile, "  Number of threads: %u  TIDs:", msg.dataArea.GET_THREAD_INFO_ACK.numThreads );
           for ( unsigned int i = 0; i < msg.dataArea.GET_THREAD_INFO_ACK.numThreads; i++ ) {
             fprintf( outfile, " %u", msg.dataArea.GET_THREAD_INFO_ACK.threadIDS[i] );
           }
           fprintf( outfile, "\n");
           break;
         }

         case THREAD_ALIVE: {
           fprintf( outfile, "  TID: %u\n", msg.dataArea.THREAD_ALIVE.tid );
           break;
         }

         case GET_THREAD_ID_ACK: {
           fprintf( outfile, "  TID: %u\n", msg.dataArea.GET_THREAD_ID_ACK.tid );
           break;
         }

         case SET_THREAD_OPS: {
           fprintf( outfile, "  TID: %u  Operation: %d\n", msg.dataArea.SET_THREAD_OPS.tid, msg.dataArea.SET_THREAD_OPS.operation );
           break;
         }

         case GET_REGS_AND_FLOATS_ACK: {
           dumpGPRSet( &msg.dataArea.GET_REGS_AND_FLOATS_ACK.gprs, outfile );
           dumpFPRSet( &msg.dataArea.GET_REGS_AND_FLOATS_ACK.fprs, outfile );
           break;
         }

         case GET_AUX_VECTORS: {
           fprintf( outfile, "  Offset: %08x    Requested Length: %d\n  Vals: ", msg.dataArea.GET_AUX_VECTORS.auxVecBufferOffset, msg.dataArea.GET_AUX_VECTORS.auxVecBufferLength );
           break;
         }

         case GET_AUX_VECTORS_ACK: {
           fprintf( outfile, "  Offset: %08x    Length: %d\n  Vals: ", msg.dataArea.GET_AUX_VECTORS_ACK.auxVecBufferOffset, msg.dataArea.GET_AUX_VECTORS_ACK.auxVecBufferLength );
           for ( unsigned int i = 0; i < msg.dataArea.GET_AUX_VECTORS_ACK.auxVecBufferLength; i++ ) 
             fprintf( outfile, "%02x ", msg.dataArea.GET_AUX_VECTORS_ACK.auxVecData[i] );
           fprintf( outfile, "\n" );
           break;
         }

         case GET_STACK_TRACE_ACK: {
           fprintf( outfile, "  LR: %08x  IAR: %08x  R1: %08x  Number of stack frames: %u\n",
                    msg.dataArea.GET_STACK_INFO_ACK.lr, msg.dataArea.GET_STACK_INFO_ACK.iar,
                    msg.dataArea.GET_STACK_INFO_ACK.stackFrame, msg.dataArea.GET_STACK_INFO_ACK.numStackFrames );
           for ( unsigned int i = 0; i < msg.dataArea.GET_STACK_INFO_ACK.numStackFrames; i++ ) {
             fprintf( outfile, "  Frame address: %08x  Saved LR: %08x\n", msg.dataArea.GET_STACK_INFO_ACK.stackInfo[i].frameAddr, msg.dataArea.GET_STACK_INFO_ACK.stackInfo[i].savedLR );
           }
           break;
         }

         case GET_PROCESS_DATA_ACK: {
           fprintf( outfile, "  Rank: %u  TGID: %u  xCoord: %u  yCoord: %u  zCoord: %u  tCoord: %u\n",
                    msg.dataArea.GET_PROCESS_DATA_ACK.processData.rank, msg.dataArea.GET_PROCESS_DATA_ACK.processData.tgid,
                    msg.dataArea.GET_PROCESS_DATA_ACK.processData.xCoord, msg.dataArea.GET_PROCESS_DATA_ACK.processData.yCoord,
                    msg.dataArea.GET_PROCESS_DATA_ACK.processData.zCoord, msg.dataArea.GET_PROCESS_DATA_ACK.processData.tCoord );
           fprintf( outfile, "  Shared memory start: %08x  Shared memory end: %08x  Persistent memory start: %08x  Persistent memory end: %08x\n",
                    msg.dataArea.GET_PROCESS_DATA_ACK.processData.sharedMemoryStartAddr, msg.dataArea.GET_PROCESS_DATA_ACK.processData.sharedMemoryEndAddr,
                    msg.dataArea.GET_PROCESS_DATA_ACK.processData.persistMemoryStartAddr, msg.dataArea.GET_PROCESS_DATA_ACK.processData.persistMemoryEndAddr );
           fprintf( outfile, "  Heap start: %08x  Heap end: %08x  Heap break: %08x  Mmap start: %08x  Mmap end: %08x\n",
                    msg.dataArea.GET_PROCESS_DATA_ACK.processData.heapStartAddr, msg.dataArea.GET_PROCESS_DATA_ACK.processData.heapEndAddr,
                    msg.dataArea.GET_PROCESS_DATA_ACK.processData.heapBreakAddr, msg.dataArea.GET_PROCESS_DATA_ACK.processData.mmapStartAddr,
                    msg.dataArea.GET_PROCESS_DATA_ACK.processData.mmapEndAddr );
           fprintf( outfile, "  Number of threads: %u  TIDs:", msg.dataArea.GET_PROCESS_DATA_ACK.numThreads );
           for ( unsigned int i = 0; i < msg.dataArea.GET_PROCESS_DATA_ACK.numThreads; i++ ) {
             fprintf( outfile, " %u", msg.dataArea.GET_PROCESS_DATA_ACK.threadIDS[i] );
           }
           fprintf( outfile, "\n");
           break;
         }

         case GET_THREAD_DATA_ACK: {
           fprintf( outfile, "  TID: %u  Core: %d  Stack start: %08x  Stack end: %08x  Guard start: %08x  Guard end: %08x\n",
                    msg.dataArea.GET_THREAD_DATA_ACK.threadData.threadID, msg.dataArea.GET_THREAD_DATA_ACK.threadData.core,
                    msg.dataArea.GET_THREAD_DATA_ACK.threadData.stackStartAddr, msg.dataArea.GET_THREAD_DATA_ACK.threadData.stackEndAddr,
                    msg.dataArea.GET_THREAD_DATA_ACK.threadData.guardStartAddr, msg.dataArea.GET_THREAD_DATA_ACK.threadData.guardEndAddr );
           dumpGPRSet( &msg.dataArea.GET_THREAD_DATA_ACK.threadData.gprs, outfile );
           dumpFPRSet( &msg.dataArea.GET_THREAD_DATA_ACK.threadData.fprs, outfile );
           dumpDebugSet( &msg.dataArea.GET_THREAD_DATA_ACK.threadData.debugRegisters, outfile );
           for ( unsigned int i = 0; i < msg.dataArea.GET_THREAD_DATA_ACK.threadData.numStackFrames; i++ ) {
             fprintf( outfile, "  Frame address: %08x  Saved LR: %08x\n", msg.dataArea.GET_THREAD_DATA_ACK.threadData.stackInfo[i].frameAddr, msg.dataArea.GET_THREAD_DATA_ACK.threadData.stackInfo[i].savedLR );
           }
           break;
         }

         case HOLD_THREAD: {
             fprintf( outfile, "Timeout(usec): %08x\n", msg.dataArea.HOLD_THREAD.timeout);
             break;
         }

         case SIGACTION: {
             fprintf( outfile, "Signum: %08x  flags: %08x mask: %08x\n", msg.dataArea.SIGACTION.signum, msg.dataArea.SIGACTION.flags, msg.dataArea.SIGACTION.mask);  
             break;
         }

         case MAP_MEM: {
             fprintf( outfile, "MemMap size: %08x\n", msg.dataArea.MAP_MEM.len);
             break;
         }
         case FAST_TRAP: {
             fprintf( outfile, "FastTrap option: %d\n", msg.dataArea.FAST_TRAP.enable);
             break;
         }

         case DEBUG_IGNORE_SIG: {
             fprintf( outfile, "Debug ignore signal option: \n");
             break;
         }

         case GET_FLOAT_REG:
         case SET_FLOAT_REG:
         case GET_FLOAT_REG_ACK:
         case SET_FLOAT_REG_ACK:
         case GET_ALL_REGS:
         case GET_ALL_FLOAT_REGS:
         case SINGLE_STEP:
         case SINGLE_STEP_ACK:
         case CONTINUE_ACK:
         case KILL_ACK:
         case ATTACH:
         case ATTACH_ACK:
         case DETACH:
         case DETACH_ACK:
         case VERSION_MSG:
         case GET_DEBUG_REGS:
         case SET_DEBUG_REGS_ACK:
         case GET_THREAD_ID:
         case GET_THREAD_INFO:
         case THREAD_ALIVE_ACK:
         case SET_THREAD_OPS_ACK:
         case GET_REGS_AND_FLOATS:
         case GET_STACK_TRACE:
         case END_DEBUG:
         case END_DEBUG_ACK:
         case GET_PROCESS_DATA:
         case GET_THREAD_DATA:
         case HOLD_THREAD_ACK:
         case RELEASE_THREAD:
         case RELEASE_THREAD_ACK:
         case SIGACTION_ACK:
         case MAP_MEM_ACK:
         case FAST_TRAP_ACK:
         case DEBUG_IGNORE_SIG_ACK:
         case THIS_SPACE_FOR_RENT: {
           // Nothing to do for these packet types unless data is added to them
           break;
         }

       }

       return;
    }

private:

   //! \brief  Read a debugger message from a descriptor.
   //! \param  fd Descriptor to read from.
   //! \param  msg Reference to debugger message.
   //! \param  abortPipeIO Pointer to integer that indicates whether to abort read on error.
   //! \return True when message was read succesfully, false if there was an error.

   static bool readFromFd_p( int fd, BG_Debugger_Msg &msg, volatile int *abortPipeIO )
   {
      // Read header first
      int headerBytesToRead = sizeof( msg.header );
      int headerBytesRead = 0;

      while ( headerBytesRead < headerBytesToRead ) {

         if ( (abortPipeIO != NULL) && (*abortPipeIO != 0) ) {
            *abortPipeIO = 0;
            return false;
         }

         int headerRc = read( fd, ((unsigned char *)&msg.header)+headerBytesRead, headerBytesToRead - headerBytesRead );

         if ( headerRc == 0 ) {
            // End of file
            return false;
         }
         else if ( headerRc == -1 ) {

            // EINTR could be tolerable ... the others are not though
            if ( errno != EINTR ) {
               perror( "BG_Debugger_Msg::readFromFd" );
               return false;
            }

         }
         else {
            headerBytesRead += headerRc;
         }
      }


      // Read the rest of the packet now
      uint32_t payloadBytesRead = 0;

      while ( payloadBytesRead < msg.header.dataLength ) {

         if ( (abortPipeIO != NULL) && (*abortPipeIO != 0) ) {
            *abortPipeIO = 0;
            return false;
         }

         int payloadRc = read( fd, ((unsigned char *)&msg.dataArea)+payloadBytesRead, msg.header.dataLength - payloadBytesRead );

         if ( payloadRc == 0 ) {
            // End of file
            return false;
         }
         else if ( payloadRc == -1 ) {

           // EINTR could be tolerable ... the others are not though
           if ( errno != EINTR ) {
              int err = errno;
              perror( "BG_Debugger_Msg::readFromFd" );
              errno = err;
              return false;
           }

         }
         else {
            payloadBytesRead += payloadRc;
         }

      }
      return true;
   }

   //! \brief  Write a debugger message to a descriptor.
   //! \param  fd Descriptor to write to.
   //! \param  msg Reference to debugger message.
   //! \param  abortPipeIO Pointer to integer that indicates whether to abort write on error.
   //! \return True when message was written successfully, false if there was an error.

   static bool writeOnFd_p( int fd, BG_Debugger_Msg &msg, volatile int *abortPipeIO )
   {
      int bytesToWrite = sizeof( msg.header ) + msg.header.dataLength;
      int bytesWritten = 0;
      while ( bytesWritten < bytesToWrite ) {

         if ( (abortPipeIO != NULL) && (*abortPipeIO != 0) ) {
            *abortPipeIO = 0;
            return false;
         }

         int writeRc = write( fd, ((unsigned char *)&msg)+bytesWritten, bytesToWrite - bytesWritten );

         if ( writeRc == -1 ) {

            if ( errno != EINTR ) {
               int err = errno;
               perror( "BG_Debugger_msg::writeOnFd" );
               errno = err;
               return false;
            }

         }
         else {
            bytesWritten += writeRc;
         }

       }

      return true;
   }

   //! \brief  Print the set of GPRs to the specified file.
   //! \param  gprs Pointer to set of GPRs.
   //! \param  outfile Pointer to file for printing message.
   //! \return Nothing.

   static void dumpGPRSet( BG_GPRSet_t *gprs, FILE *outfile )
   {
     for ( int i=0; i < 8; i++ ) {
       for ( int j=0; j < 4; j++ ) {
         fprintf( outfile, "  r%02d: %08x    ", i*4+j, gprs->gpr[i*4+j] );
       }
       fprintf( outfile, "\n" );
     }
     fprintf( outfile, "FPSCR: %08x       LR: %08x       CR: %08x      XER: %08x\n",
              gprs->fpscr, gprs->lr, gprs->cr, gprs->xer );
     fprintf( outfile, "  CTR: %08x      IAR: %08x      MSR: %08x     DEAR: %08x\n",
              gprs->ctr, gprs->iar, gprs->msr, gprs->dear );
     fprintf( outfile, "  ESR: %08x\n", gprs->esr );
     return;
   }

   //! \brief  Print the set of FPRs to the specified file.
   //! \param  fprs Pointer to set of FPRs.
   //! \param  outfile Pointer to file for printing message.
   //! \return Nothing.

   static void dumpFPRSet( BG_FPRSet_t *fprs, FILE *outfile )
   {
     fprintf( outfile, "  Double hummer set 0:\n" );
     for ( int i=0; i < 8; i++ ) {
       for ( int j=0; j < 4; j++ ) {
         double val;
         memcpy( &val, &fprs->fprs[i*4+j].w0, sizeof( _QuadWord_t ) );
         fprintf( outfile, "  f%02d: %f    ", i*4+j, val );
       }
       fprintf( outfile, "\n" );
     }
     fprintf( outfile, "\n" );
     fprintf( outfile, "  Double hummer set 1:\n" );
     for ( int i=0; i < 8; i++ ) {
       for ( int j=0; j < 4; j++ ) {
         double val;
         memcpy( &val, &fprs->fprs[i*4+j].w2, sizeof( _QuadWord_t ) ); 
         fprintf( outfile, "  f%02d: %f    ", i*4+j, val );
       }
       fprintf( outfile, "\n" );
     }
     fprintf( outfile, "\n" );
     fprintf( outfile, "  Double hummer set 0 in hex:\n" );
     for ( int i=0; i < 8; i++ ) {
       for ( int j=0; j < 4; j++ ) {
         uint32_t val1 = fprs->fprs[i*4+j].w0;
         uint32_t val2 = fprs->fprs[i*4+j].w1;
         fprintf( outfile, "  f%02d: %08x%08x  ", i*4+j, val1, val2 );
       }
       fprintf( outfile, "\n" );
     }
     fprintf( outfile, "\n" );
     fprintf( outfile, "  Double hummer set 1 in hex:\n" );
     for ( int i=0; i < 8; i++ ) {
       for ( int j=0; j < 4; j++ ) {
         uint32_t val1 = fprs->fprs[i*4+j].w2;
         uint32_t val2 = fprs->fprs[i*4+j].w3;
         fprintf( outfile, "  f%02d: %08x%08x  ", i*4+j, val1, val2 );
       }
       fprintf( outfile, "\n" );
     }
     return;
   }

   //! \brief  Print the set of debug registers to the specified file.
   //! \param  gprs Pointer to set of debug registers.
   //! \param  outfile Pointer to file for printing message.
   //! \return Nothing.

   static void dumpDebugSet( BG_DebugSet_t *debugRegisters, FILE *outfile )
   {
     fprintf( outfile, "  DBCR0: %08x   DBCR1: %08x   DBCR2: %08x    DBSR: %08x\n",
              debugRegisters->DBCR0, debugRegisters->DBCR1, debugRegisters->DBCR2, debugRegisters->DBSR );
     fprintf( outfile, "   IAC1: %08x    IAC2: %08x    IAC3: %08x    IAC4: %08x\n",
              debugRegisters->IAC1, debugRegisters->IAC2, debugRegisters->IAC3, debugRegisters->IAC4 );
     fprintf( outfile, "   DAC1: %08x    DAC2: %08x    DVC1: %08x    DVC2: %08x\n",
              debugRegisters->DAC1, debugRegisters->DAC2, debugRegisters->DVC1, debugRegisters->DVC2 );
     return;
   }

};


}
#undef fprintf

#endif // CIODEBUGGERPROTOCOL_H
