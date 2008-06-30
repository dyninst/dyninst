/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* This is an automatically generated copyright prolog.             */
/* After initializing, DO NOT MODIFY OR MOVE                        */
/* ---------------------------------------------------------------- */
/* IBM Confidential                                                 */
/*                                                                  */
/* OCO Source Materials                                             */
/*                                                                  */
/* Product(s):                                                      */
/* 5733-BG1                                                         */
/*                                                                  */
/* (C)Copyright IBM Corp. 2004, 2004                                */
/*                                                                  */
/* The Source code for this program is not published or otherwise   */
/* divested of its trade secrets, irrespective of what has been     */
/* deposited with the U.S. Copyright Office.                        */
/* ---------------------------------------------------------------  */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
#ifndef DEBUGGER_INTERFACE_H
#define DEBUGGER_INTERFACE_H
#include <stdint.h>
#include <stdio.h>
namespace DebuggerInterface {
#define BGL_DEBUGGER_WRITE_PIPE 3
#define BGL_DEBUGGER_READ_PIPE 4
   // Some typedefs to insure consistency later.
   typedef uint32_t BGL_NodeNum_t; // Which compute node under an I/O node
   typedef uint32_t BGL_ThreadID_t; // Which thread in the compute node
   typedef uint32_t BGL_GPR_t; // GPRs are 32 bit unsigned
   typedef uint32_t BGL_Addr_t; // 32 bit virtual address
   /* Register numbers
      The value of each enum is magically the same as the offset in
      words into the GPRSet_t structure. The system is similar to,
      but not necessarily the same as used by ptrace.h.
   */
   typedef enum {
      BGL_GPR0 = 0,
      BGL_GPR1,
      BGL_GPR2,
      BGL_GPR3,
      BGL_GPR4,
      BGL_GPR5,
      BGL_GPR6,
      BGL_GPR7,
      BGL_GPR8,
      BGL_GPR9,
      BGL_GPR10,
      BGL_GPR11,
      BGL_GPR12,
      BGL_GPR13,
      BGL_GPR14,
      BGL_GPR15,
      BGL_GPR16,
      BGL_GPR17,
      BGL_GPR18,
      BGL_GPR19,
      BGL_GPR20,
      BGL_GPR21,
      BGL_GPR22,
      BGL_GPR23,
      BGL_GPR24,
      BGL_GPR25,
      BGL_GPR26,
      BGL_GPR27,
      BGL_GPR28,
      BGL_GPR29,
      BGL_GPR30,
      BGL_GPR31 = 31,
      BGL_FPSCR = 32,
      BGL_LR = 33,
      BGL_CR = 34,
      BGL_XER = 35,
      BGL_CTR = 36,
      BGL_IAR = 37,
      BGL_MSR = 38,
      BGL_DEAR = 39,
      BGL_ESR = 40
   } BGL_GPR_Num_t;
   /* Floating point register numbers
      We have 'double hummer'. We will pretend that we have 32 registers,
      each 128 bits (4 words) wide. Once again, the enum value is the
      offset into the FPRSet_t struct.
   */
   typedef enum {
      BGL_FPR0 = 0,
      BGL_FPR1,
      BGL_FPR2,
      BGL_FPR3,
      BGL_FPR4,
      BGL_FPR5,
      BGL_FPR6,
      BGL_FPR7,
      BGL_FPR8,
      BGL_FPR9,
      BGL_FPR10,
      BGL_FPR11,
      BGL_FPR12,
      BGL_FPR13,
      BGL_FPR14,
      BGL_FPR15,
      BGL_FPR16,
      BGL_FPR17,
      BGL_FPR18,
      BGL_FPR19,
      BGL_FPR20,
      BGL_FPR21,
      BGL_FPR22,
      BGL_FPR23,
      BGL_FPR24,
      BGL_FPR25,
      BGL_FPR26,
      BGL_FPR27,
      BGL_FPR28,
      BGL_FPR29,
      BGL_FPR30,
      BGL_FPR31 = 31
   } BGL_FPR_Num_t;
   /* MsgType
      This doesn't match the ptrace interface very well.
      Ptrace doesn't define most of these primitives.
      Therefore, the enums are completely arbitrary.
      Notice that except for SIGNAL_ENCOUNTERED, each
      request from the debugger has a corresponding
      acknowledgement message.
   */
   typedef enum { GET_REG,
                  GET_ALL_REGS, // request gprs and other non-fp regs
                  SET_REG, // set a specific register
                  GET_MEM, // get memory values
                  SET_MEM, // set memory
                  GET_FLOAT_REG, //5
                  GET_ALL_FLOAT_REGS,
                  SET_FLOAT_REG,
                  SINGLE_STEP, // step one instruction
                  CONTINUE, // tell compute node to continue running
                  KILL, // 10 send a signal w/ implicit continue
                  ATTACH, // mark compute node
                  DETACH, // unmark compute node
                  GET_REG_ACK,
                  GET_ALL_REGS_ACK, // sent when compute node responds
                  SET_REG_ACK, // 15 send when compute node responds
                  GET_MEM_ACK, // sent when compute node responds
                  SET_MEM_ACK, // sent when compute node responds
                  GET_FLOAT_REG_ACK,
                  GET_ALL_FLOAT_REGS_ACK,
                  SET_FLOAT_REG_ACK, //20
                  SINGLE_STEP_ACK, // sent when compute node completes step
                  CONTINUE_ACK, // sent when compute node is told
                  KILL_ACK, // send when signal is sent
                  ATTACH_ACK, // sent after compute node is marked
                  DETACH_ACK, // 25 sent after compute node is unmarked
                  SIGNAL_ENCOUNTERED, // sent when a signal is encountered
                  PROGRAM_EXITED, // with implicit detach
                  VERSION_MSG,
                  VERSION_MSG_ACK,
                  GET_DEBUG_REGS, //30
                  GET_DEBUG_REGS_ACK,
                  SET_DEBUG_REGS,
                  SET_DEBUG_REGS_ACK,
                  THIS_SPACE_FOR_RENT
   } BGL_MsgType_t;
   extern const char *BGL_Packet_Names[];
   /* GPRSet_t
      This is the set of general purpose registers.
   */
   typedef struct {
      uint32_t gpr[32]; // gprs
      uint32_t fpscr; // fpscr
      uint32_t lr; // link
      uint32_t cr; // condition reg
      uint32_t xer; // xer
      uint32_t ctr; // count register
      uint32_t iar; // pc
      uint32_t msr; // machine status register
      uint32_t dear; // data exception address register
      uint32_t esr; // exception syndrome register
   } BGL_GPRSet_t;
   /* FPRSet_t
      Our FPRs are a little bit different because of the double hummer.
      We actually have 2 sets of FPRs.
      The convention that we'll use is to treat the FPRs as one set of
      32, with each FPR being four words instead of two.
   */
   typedef struct {
      uint32_t w0; // First word of FPR in first FPR set
      uint32_t w1; // Second word of FPR in first FPR set
      uint32_t w2; // First word of FPR in second FPR set
      uint32_t w3; // Second word of FPR in second FPR set
   } BGL_FPR_t;
   typedef struct {
      BGL_FPR_t fprs[32];
   } BGL_FPRSet_t;
   typedef struct {
      uint32_t DBCR0; // Debug Control Register 0
      uint32_t DBCR1; // Debug Control Register 1
      uint32_t DBCR2; // Debug Control Register 2
      uint32_t DBSR; // Debug Status Register
      uint32_t IAC1; // Instruction Address Compare Register 1
      uint32_t IAC2; // Instruction Address Compare Register 2
      uint32_t IAC3; // Instruction Address Compare Register 3
      uint32_t IAC4; // Instruction Address Compare Register 4
      uint32_t DAC1; // Data Address Compare Register 1
      uint32_t DAC2; // Data Address Compare Register 2
      uint32_t DVC1; // Data Value Compare Register 1
      uint32_t DVC2; // Data Value Compare Register 2
   } BGL_DebugSet_t;
   typedef enum {
      RC_NO_ERROR = 0,
      RC_NOT_ATTACHED = 1,
      RC_NOT_RUNNING = 2,
      RC_BAD_NODE = 3,
      RC_BAD_THREAD = 4,
      RC_BAD_COMMAND = 5,
      RC_BAD_REGISTER = 6,
      RC_NOT_APP_SPACE = 7,
      RC_LEN_TOO_LONG = 8,
      RC_DENIED = 9,
      RC_BAD_SIGNAL = 10,
      RC_NOT_STOPPED = 11
   } BGL_ErrorCode_t;
   /* Debugger_Msg_t
      This is the packet header for the pipe. All messages use this.
      messageType is self explanatory
      nodeNumber is the target compute node
      thread is the thread on the compute node
      sequence can be used to keep track of packet flow
      returnCode might be needed
      dataLength is the size in chars of the payload
      The 'payload' should follow the message header, and be received
      into a buffer of size dataLength.
      Not all messages have payloads. If your message doesn't have
      a payload, set dataLength to 0.
      'sequence' is probably best used as a 'packet count' or a sequence
      number. This way you can match acknowledgement packets with the
      original packet if things aren't being done in lockstep.
      'returnCode' isn't architected yet, but we probably need it. If your
      GET_MEM request fails how else will you know?
      'dataStartsHere' is a placeholder. As a result, to get the true size
      of this data structure you have to subtract 1 byte.
   */
#define BGL_Debugger_Msg_MAX_SIZE 4096
#define BGL_Debugger_Msg_HEADER_SIZE 24
#define BGL_Debugger_Msg_MAX_PAYLOAD_SIZE (BGL_Debugger_Msg_MAX_SIZE-BGL_Debugger_Msg_HEADER_SIZE)
#define BGL_Debugger_Msg_MAX_MEM_SIZE 4064
   class BGL_Debugger_Msg {
   public:
      typedef struct {
         BGL_MsgType_t messageType;
         BGL_NodeNum_t nodeNumber;
         BGL_ThreadID_t thread;
         uint32_t sequence;
         uint32_t returnCode;
         uint32_t dataLength; // excluding this header
      } Header;
      Header header;
      typedef union {
         struct {
            BGL_GPR_Num_t registerNumber;
         } GET_REG;
         struct {
            BGL_GPR_Num_t registerNumber;
            BGL_GPR_t value;
         } GET_REG_ACK;
         struct {
         } GET_ALL_REGS;
         struct {
            BGL_GPRSet_t gprs;
         } GET_ALL_REGS_ACK;
         struct {
            BGL_GPR_Num_t registerNumber;
            BGL_GPR_t value;
         } SET_REG;
         struct {
            BGL_GPR_Num_t registerNumber;
         } SET_REG_ACK;
         struct {
            BGL_Addr_t addr;
            uint32_t len;
         } GET_MEM;
         struct {
            BGL_Addr_t addr;
            uint32_t len;
            unsigned char data[BGL_Debugger_Msg_MAX_MEM_SIZE];
         } GET_MEM_ACK;
         struct {
            BGL_Addr_t addr;
            uint32_t len;
            unsigned char data[BGL_Debugger_Msg_MAX_MEM_SIZE];
         } SET_MEM;
         struct {
            BGL_Addr_t addr;
            uint32_t len;
         } SET_MEM_ACK;
         struct {
            BGL_FPR_Num_t registerNumber;
         } GET_FLOAT_REG;
         struct {
            BGL_FPR_Num_t registerNumber;
            BGL_FPR_t value;
         } GET_FLOAT_REG_ACK;
         struct {
         } GET_ALL_FLOAT_REGS;
         struct {
            BGL_FPRSet_t fprs;
         } GET_ALL_FLOAT_REGS_ACK;
         struct {
            BGL_FPR_Num_t registerNumber;
            BGL_FPR_t value;
         } SET_FLOAT_REG;
         struct {
            BGL_FPR_Num_t registerNumber;
         } SET_FLOAT_REG_ACK;
         struct {
         } SINGLE_STEP;
         struct {
         } SINGLE_STEP_ACK;
         struct {
            uint32_t signal;
         } CONTINUE;
         struct {
         } CONTINUE_ACK;
         struct {
            uint32_t signal;
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
            uint32_t signal;
         } SIGNAL_ENCOUNTERED;
         struct {
            int32_t type; // 0 = exit, 1 = signal
            int32_t rc;
         } PROGRAM_EXITED;
         struct {
         } VERSION_MSG;
         struct {
            uint32_t protocolVersion;
            uint32_t numPhysicalProcessors;
            uint32_t numLogicalProcessors;
         } VERSION_MSG_ACK;
         struct {
         } GET_DEBUG_REGS;
         struct {
            BGL_DebugSet_t debugRegisters;
         } GET_DEBUG_REGS_ACK;
         struct {
            BGL_DebugSet_t debugRegisters;
         } SET_DEBUG_REGS;
         struct {
         } SET_DEBUG_REGS_ACK;
         unsigned char dataStartsHere;
      } DataArea;
      DataArea dataArea;
      // Ctor
      BGL_Debugger_Msg( void ) { header.messageType = THIS_SPACE_FOR_RENT; }
      BGL_Debugger_Msg( BGL_MsgType_t type,
                        BGL_NodeNum_t node,
                        BGL_ThreadID_t thread,
                        uint32_t sequence,
                        uint32_t returnCode )
         {
            header.messageType = type;
            header.nodeNumber = node;
            header.thread = thread;
            header.sequence = sequence;
            header.returnCode = returnCode;
         }
      static BGL_Debugger_Msg generateErrorPacket( BGL_Debugger_Msg &original, BGL_ErrorCode_t ec );
      static bool readFromFd( int fd, BGL_Debugger_Msg &msg );
      static bool writeOnFd( int fd, BGL_Debugger_Msg &msg );
      static void dump( BGL_Debugger_Msg &msg );
      static void dump( BGL_Debugger_Msg &msg, FILE *outfile );
   };
}
#endif // DEBUGGER_INTERFACE_H
