/*
 * Copyright (c) 1996-2011 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */
/**
 * We've been told by IBM that this header is no longer IBM confidential.
 * The contents are being released and we were given premission to 
 * redistribute this--although they never gave us the new header without
 * this copyright.
 **/

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
/*                                                                  */
/*                                                                  */
/* ---------------------------------------------------------------  */
/* 3/22/09 Todd Gamblin    Modified to change BGL_ prefixes so that */
/*                         this matches the BG/P header.  Allows us */
/*                         to generify BG stackwalker code.         */
/* ---------------------------------------------------------------  */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
#ifndef DEBUGGER_INTERFACE_H
#define DEBUGGER_INTERFACE_H
#include <stdint.h>
#include <stdio.h>
namespace DebuggerInterface {
#define BG_DEBUGGER_WRITE_PIPE 3
#define BG_DEBUGGER_READ_PIPE 4
   // Some typedefs to insure consistency later.
   typedef uint32_t BG_NodeNum_t; // Which compute node under an I/O node
   typedef uint32_t BG_ThreadID_t; // Which thread in the compute node
   typedef uint32_t BG_GPR_t; // GPRs are 32 bit unsigned
   typedef uint32_t BG_Addr_t; // 32 bit virtual address
   /* Register numbers
      The value of each enum is magically the same as the offset in
      words into the GPRSet_t structure. The system is similar to,
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
      BG_FPSCR = 32,
      BG_LR = 33,
      BG_CR = 34,
      BG_XER = 35,
      BG_CTR = 36,
      BG_IAR = 37,
      BG_MSR = 38,
      BG_DEAR = 39,
      BG_ESR = 40
   } BG_GPR_Num_t;
   /* Floating point register numbers
      We have 'double hummer'. We will pretend that we have 32 registers,
      each 128 bits (4 words) wide. Once again, the enum value is the
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
   } BG_MsgType_t;
   extern const char *BG_Packet_Names[];
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
   } BG_GPRSet_t;
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
   } BG_FPR_t;
   typedef struct {
      BG_FPR_t fprs[32];
   } BG_FPRSet_t;
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
   } BG_DebugSet_t;
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
   } BG_ErrorCode_t;
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
#define BG_Debugger_Msg_MAX_SIZE 4096
#define BG_Debugger_Msg_HEADER_SIZE 24
#define BG_Debugger_Msg_MAX_PAYLOAD_SIZE (BG_Debugger_Msg_MAX_SIZE-BG_Debugger_Msg_HEADER_SIZE)
#define BG_Debugger_Msg_MAX_MEM_SIZE 4064
   class BG_Debugger_Msg {
   public:
      typedef struct {
         BG_MsgType_t messageType;
         BG_NodeNum_t nodeNumber;
         BG_ThreadID_t thread;
         uint32_t sequence;
         uint32_t returnCode;
         uint32_t dataLength; // excluding this header
      } Header;
      Header header;
      typedef union {
         struct {
            BG_GPR_Num_t registerNumber;
         } GET_REG;
         struct {
            BG_GPR_Num_t registerNumber;
            BG_GPR_t value;
         } GET_REG_ACK;
         struct {
         } GET_ALL_REGS;
         struct {
            BG_GPRSet_t gprs;
         } GET_ALL_REGS_ACK;
         struct {
            BG_GPR_Num_t registerNumber;
            BG_GPR_t value;
         } SET_REG;
         struct {
            BG_GPR_Num_t registerNumber;
         } SET_REG_ACK;
         struct {
            BG_Addr_t addr;
            uint32_t len;
         } GET_MEM;
         struct {
            BG_Addr_t addr;
            uint32_t len;
            unsigned char data[BG_Debugger_Msg_MAX_MEM_SIZE];
         } GET_MEM_ACK;
         struct {
            BG_Addr_t addr;
            uint32_t len;
            unsigned char data[BG_Debugger_Msg_MAX_MEM_SIZE];
         } SET_MEM;
         struct {
            BG_Addr_t addr;
            uint32_t len;
         } SET_MEM_ACK;
         struct {
            BG_FPR_Num_t registerNumber;
         } GET_FLOAT_REG;
         struct {
            BG_FPR_Num_t registerNumber;
            BG_FPR_t value;
         } GET_FLOAT_REG_ACK;
         struct {
         } GET_ALL_FLOAT_REGS;
         struct {
            BG_FPRSet_t fprs;
         } GET_ALL_FLOAT_REGS_ACK;
         struct {
            BG_FPR_Num_t registerNumber;
            BG_FPR_t value;
         } SET_FLOAT_REG;
         struct {
            BG_FPR_Num_t registerNumber;
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
            BG_DebugSet_t debugRegisters;
         } GET_DEBUG_REGS_ACK;
         struct {
            BG_DebugSet_t debugRegisters;
         } SET_DEBUG_REGS;
         struct {
         } SET_DEBUG_REGS_ACK;
         unsigned char dataStartsHere;
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
      static BG_Debugger_Msg generateErrorPacket( BG_Debugger_Msg &original, BG_ErrorCode_t ec );
      static void dump( BG_Debugger_Msg &msg );
      static void dump( BG_Debugger_Msg &msg, FILE *outfile );

      static bool writeOnFd(int fd, BG_Debugger_Msg &msg)
      {
        int result;
        result = write(fd, &msg.header, sizeof(msg.header));
        if (result != -1) {
          result = write(fd, &msg.dataArea, msg.header.dataLength);
        }
        
        return (result != -1);
      }
      
      static bool readFromFd(int fd, BG_Debugger_Msg &msg)
      {
        int result;
        result = read(fd, &msg.header, sizeof(msg.header));
        if (result != -1 && msg.header.dataLength) {
          result = read(fd, &msg.dataArea, msg.header.dataLength);
        }
        return (result != -1);
      }
     
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
            "THIS_SPACE_FOR_RENT"
          };
  
        if ((type >= GET_REG) && (type < THIS_SPACE_FOR_RENT)) {
          return BG_Packet_Names[type];
        }
        else {
          return "UNKNOWN";
        }
      }
   };
}

#endif // DEBUGGER_INTERFACE_H
