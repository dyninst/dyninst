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

#include "bluegeneq.h"
using namespace bgq;

#define CMD_RET_SIZE(X) case X: return sizeof(X ## Cmd)
#define CMD_RET_VAR_SIZE(X, F) case X: return sizeof(X ## Cmd) + static_cast<const X ## Cmd &>(cmd).F
uint32_t bgq_process::getCommandLength(uint16_t cmd_type, const ToolCommand &cmd)
{
   switch (cmd_type) {
      CMD_RET_SIZE(GetSpecialRegs);
      CMD_RET_SIZE(GetSpecialRegsAck);
      CMD_RET_SIZE(GetGeneralRegs);
      CMD_RET_SIZE(GetGeneralRegsAck);
      CMD_RET_SIZE(GetFloatRegs);
      CMD_RET_SIZE(GetFloatRegsAck);
      CMD_RET_SIZE(GetDebugRegs);
      CMD_RET_SIZE(GetDebugRegsAck);
      CMD_RET_SIZE(GetMemory);
      CMD_RET_VAR_SIZE(GetMemoryAck, length);
      CMD_RET_SIZE(GetThreadList);
      CMD_RET_SIZE(GetThreadListAck);
      CMD_RET_SIZE(GetAuxVectors);
      CMD_RET_SIZE(GetAuxVectorsAck);
      CMD_RET_SIZE(GetProcessData);
      CMD_RET_SIZE(GetProcessDataAck);
      CMD_RET_SIZE(GetThreadData);
      CMD_RET_SIZE(GetThreadDataAck);
      CMD_RET_SIZE(GetPreferences);
      CMD_RET_SIZE(GetPreferencesAck);
      CMD_RET_SIZE(GetFilenames);
      CMD_RET_SIZE(GetFilenamesAck);
      CMD_RET_SIZE(GetFileStatData);
      CMD_RET_SIZE(GetFileStatDataAck);
      CMD_RET_SIZE(GetFileContents);
      CMD_RET_VAR_SIZE(GetFileContentsAck, numbytes);
      CMD_RET_SIZE(SetGeneralReg);
      CMD_RET_SIZE(SetGeneralRegAck);
      CMD_RET_SIZE(SetFloatReg);
      CMD_RET_SIZE(SetFloatRegAck);
      CMD_RET_SIZE(SetDebugReg);
      CMD_RET_SIZE(SetDebugRegAck);
      CMD_RET_VAR_SIZE(SetMemory, length);
      CMD_RET_SIZE(SetMemoryAck);
      CMD_RET_SIZE(HoldThread);
      CMD_RET_SIZE(HoldThreadAck);
      CMD_RET_SIZE(ReleaseThread);
      CMD_RET_SIZE(ReleaseThreadAck);
      CMD_RET_SIZE(InstallTrapHandler);
      CMD_RET_SIZE(InstallTrapHandlerAck);
      CMD_RET_SIZE(AllocateMemory);
      CMD_RET_SIZE(AllocateMemoryAck);
      CMD_RET_SIZE(SendSignal);
      CMD_RET_SIZE(SendSignalAck);
      CMD_RET_SIZE(ContinueProcess);
      CMD_RET_SIZE(ContinueProcessAck);
      CMD_RET_SIZE(StepThread);
      CMD_RET_SIZE(StepThreadAck);
      CMD_RET_SIZE(SetBreakpoint);
      CMD_RET_SIZE(SetBreakpointAck);
      CMD_RET_SIZE(ResetBreakpoint);
      CMD_RET_SIZE(ResetBreakpointAck);
      CMD_RET_SIZE(SetWatchpoint);
      CMD_RET_SIZE(SetWatchpointAck);
      CMD_RET_SIZE(ResetWatchpoint);
      CMD_RET_SIZE(ResetWatchpointAck);
      CMD_RET_SIZE(RemoveTrapHandler);
      CMD_RET_SIZE(RemoveTrapHandlerAck);
      CMD_RET_SIZE(SetPreferences);
      CMD_RET_SIZE(SetPreferencesAck);
      CMD_RET_SIZE(FreeMemory);
      CMD_RET_SIZE(FreeMemoryAck);
      CMD_RET_SIZE(SetSpecialReg);
      CMD_RET_SIZE(SetSpecialRegAck);
      CMD_RET_SIZE(SetGeneralRegs);
      CMD_RET_SIZE(SetGeneralRegsAck);
      CMD_RET_SIZE(SetFloatRegs);
      CMD_RET_SIZE(SetFloatRegsAck);
      CMD_RET_SIZE(SetDebugRegs);
      CMD_RET_SIZE(SetDebugRegsAck);
      CMD_RET_SIZE(SetSpecialRegs);
      CMD_RET_SIZE(SetSpecialRegsAck);
      CMD_RET_SIZE(ReleaseControl);
      CMD_RET_SIZE(ReleaseControlAck);
      CMD_RET_SIZE(SetContinuationSignal);
      CMD_RET_SIZE(SetContinuationSignalAck);
      default:
         perr_printf("Unknown command type\n");
         assert(0);
         return 0;
   }
}

#define CMD_RET_ACK_SIZE(X) case X: return sizeof(X ## AckCmd)
#define CMD_RET_VAR_ACK_SIZE(X, F) case X: return sizeof(X ## AckCmd) + static_cast<const X ## Cmd &>(cmd).F
uint32_t bgq_process::getCommandAckLength(uint16_t cmd_type, const ToolCommand &cmd)
{
   switch (cmd_type) {
      CMD_RET_ACK_SIZE(GetSpecialRegs);
      CMD_RET_ACK_SIZE(GetGeneralRegs);
      CMD_RET_ACK_SIZE(GetFloatRegs);
      CMD_RET_ACK_SIZE(GetDebugRegs);
      CMD_RET_VAR_ACK_SIZE(GetMemory, length);
      CMD_RET_ACK_SIZE(GetThreadList);
      CMD_RET_ACK_SIZE(GetAuxVectors);
      CMD_RET_ACK_SIZE(GetProcessData);
      CMD_RET_ACK_SIZE(GetThreadData);
      CMD_RET_ACK_SIZE(GetPreferences);
      CMD_RET_ACK_SIZE(GetFilenames);
      CMD_RET_ACK_SIZE(GetFileStatData);
      CMD_RET_VAR_ACK_SIZE(GetFileContents, numbytes);
      CMD_RET_ACK_SIZE(SetGeneralReg);
      CMD_RET_ACK_SIZE(SetFloatReg);
      CMD_RET_ACK_SIZE(SetDebugReg);
      CMD_RET_ACK_SIZE(SetMemory);
      CMD_RET_ACK_SIZE(HoldThread);
      CMD_RET_ACK_SIZE(ReleaseThread);
      CMD_RET_ACK_SIZE(InstallTrapHandler);
      CMD_RET_ACK_SIZE(AllocateMemory);
      CMD_RET_ACK_SIZE(SendSignal);
      CMD_RET_ACK_SIZE(ContinueProcess);
      CMD_RET_ACK_SIZE(StepThread);
      CMD_RET_ACK_SIZE(SetBreakpoint);
      CMD_RET_ACK_SIZE(ResetBreakpoint);
      CMD_RET_ACK_SIZE(SetWatchpoint);
      CMD_RET_ACK_SIZE(ResetWatchpoint);
      CMD_RET_ACK_SIZE(RemoveTrapHandler);
      CMD_RET_ACK_SIZE(SetPreferences);
      CMD_RET_ACK_SIZE(FreeMemory);
      CMD_RET_ACK_SIZE(SetSpecialReg);
      CMD_RET_ACK_SIZE(SetGeneralRegs);
      CMD_RET_ACK_SIZE(SetFloatRegs);
      CMD_RET_ACK_SIZE(SetDebugRegs);
      CMD_RET_ACK_SIZE(SetSpecialRegs);
      CMD_RET_ACK_SIZE(ReleaseControl);
      CMD_RET_ACK_SIZE(SetContinuationSignal);
      default:
         perr_printf("Unknown command type\n");
         assert(0);
         return 0;
   }
}


#define STR_CASE(X) case X: return #X
const char *getCommandName(uint16_t cmd_type)
{
   switch (cmd_type) {
      STR_CASE(GetSpecialRegs);
      STR_CASE(GetSpecialRegsAck);
      STR_CASE(GetGeneralRegs);
      STR_CASE(GetGeneralRegsAck);
      STR_CASE(GetFloatRegs);
      STR_CASE(GetFloatRegsAck);
      STR_CASE(GetDebugRegs);
      STR_CASE(GetDebugRegsAck);
      STR_CASE(GetMemory);
      STR_CASE(GetMemoryAck);
      STR_CASE(GetThreadList);
      STR_CASE(GetThreadListAck);
      STR_CASE(GetAuxVectors);
      STR_CASE(GetAuxVectorsAck);
      STR_CASE(GetProcessData);
      STR_CASE(GetProcessDataAck);
      STR_CASE(GetThreadData);
      STR_CASE(GetThreadDataAck);
      STR_CASE(GetPreferences);
      STR_CASE(GetPreferencesAck);
      STR_CASE(GetFilenames);
      STR_CASE(GetFilenamesAck);
      STR_CASE(GetFileStatData);
      STR_CASE(GetFileStatDataAck);
      STR_CASE(GetFileContents);
      STR_CASE(GetFileContentsAck);
      STR_CASE(SetGeneralReg);
      STR_CASE(SetGeneralRegAck);
      STR_CASE(SetFloatReg);
      STR_CASE(SetFloatRegAck);
      STR_CASE(SetDebugReg);
      STR_CASE(SetDebugRegAck);
      STR_CASE(SetMemory);
      STR_CASE(SetMemoryAck);
      STR_CASE(HoldThread);
      STR_CASE(HoldThreadAck);
      STR_CASE(ReleaseThread);
      STR_CASE(ReleaseThreadAck);
      STR_CASE(InstallTrapHandler);
      STR_CASE(InstallTrapHandlerAck);
      STR_CASE(AllocateMemory);
      STR_CASE(AllocateMemoryAck);
      STR_CASE(SendSignal);
      STR_CASE(SendSignalAck);
      STR_CASE(ContinueProcess);
      STR_CASE(ContinueProcessAck);
      STR_CASE(StepThread);
      STR_CASE(StepThreadAck);
      STR_CASE(SetBreakpoint);
      STR_CASE(SetBreakpointAck);
      STR_CASE(ResetBreakpoint);
      STR_CASE(ResetBreakpointAck);
      STR_CASE(SetWatchpoint);
      STR_CASE(SetWatchpointAck);
      STR_CASE(ResetWatchpoint);
      STR_CASE(ResetWatchpointAck);
      STR_CASE(RemoveTrapHandler);
      STR_CASE(RemoveTrapHandlerAck);
      STR_CASE(SetPreferences);
      STR_CASE(SetPreferencesAck);
      STR_CASE(FreeMemory);
      STR_CASE(FreeMemoryAck);
      STR_CASE(SetSpecialReg);
      STR_CASE(SetSpecialRegAck);
      STR_CASE(SetGeneralRegs);
      STR_CASE(SetGeneralRegsAck);
      STR_CASE(SetFloatRegs);
      STR_CASE(SetFloatRegsAck);
      STR_CASE(SetDebugRegs);
      STR_CASE(SetDebugRegsAck);
      STR_CASE(SetSpecialRegs);
      STR_CASE(SetSpecialRegsAck);
      STR_CASE(ReleaseControl);
      STR_CASE(ReleaseControlAck);
      STR_CASE(SetContinuationSignal);
      STR_CASE(SetContinuationSignalAck);
      default:
         perr_printf("Unknown command type\n");
         assert(0);
         return 0;
   }
}

#define MESSAGE_RET_SIZE(X) case X: return sizeof(X ## Message)
#define MESSAGE_RET_CMDLIST_SIZE(X)                                     \
   case X: do {                                                         \
      const X ## Message &s = static_cast<const X ## Message &>(msg);   \
      assert(s.numCommands);                                            \
      const CommandDescriptor &last = s.cmdList[s.numCommands-1];       \
      return sizeof(X ## Message) + last.offset + last.length;          \
   } while (0)

uint32_t bgq_process::getMessageLength(const ToolMessage &msg)
{
   switch (msg.header.type) {
      MESSAGE_RET_SIZE(ErrorAck);
      MESSAGE_RET_SIZE(Attach);
      MESSAGE_RET_SIZE(AttachAck);
      MESSAGE_RET_SIZE(Detach);
      MESSAGE_RET_SIZE(DetachAck);
      MESSAGE_RET_CMDLIST_SIZE(Query);
      MESSAGE_RET_CMDLIST_SIZE(QueryAck);
      MESSAGE_RET_CMDLIST_SIZE(Update);
      MESSAGE_RET_CMDLIST_SIZE(UpdateAck);
      MESSAGE_RET_SIZE(SetupJob);
      MESSAGE_RET_SIZE(SetupJobAck);
      MESSAGE_RET_SIZE(Notify);
      MESSAGE_RET_SIZE(NotifyAck);
      MESSAGE_RET_SIZE(Control);
      MESSAGE_RET_SIZE(ControlAck);
      default:
         perr_printf("Unknown command type\n");
         assert(0);
   }
}

#define RET_ACK(CMD) case CMD: return CMD ## Ack; 
uint16_t bgq_process::getCommandExpectedAck(uint16_t cmd_id)
{
   switch (cmd_id) {
      RET_ACK(GetSpecialRegs);
      RET_ACK(GetGeneralRegs);
      RET_ACK(GetFloatRegs);
      RET_ACK(GetDebugRegs);
      RET_ACK(GetMemory);
      RET_ACK(GetThreadList);
      RET_ACK(GetAuxVectors);
      RET_ACK(GetProcessData);
      RET_ACK(GetThreadData);
      RET_ACK(GetPreferences);
      RET_ACK(GetFilenames);
      RET_ACK(GetFileStatData);
      RET_ACK(GetFileContents);
      RET_ACK(SetGeneralReg);
      RET_ACK(SetFloatReg);
      RET_ACK(SetDebugReg);
      RET_ACK(SetMemory);
      RET_ACK(HoldThread);
      RET_ACK(ReleaseThread);
      RET_ACK(InstallTrapHandler);
      RET_ACK(AllocateMemory);
      RET_ACK(SendSignal);
      RET_ACK(ContinueProcess);
      RET_ACK(StepThread);
      RET_ACK(SetBreakpoint);
      RET_ACK(ResetBreakpoint);
      RET_ACK(SetWatchpoint);
      RET_ACK(ResetWatchpoint);
      RET_ACK(RemoveTrapHandler);
      RET_ACK(SetPreferences);
      RET_ACK(FreeMemory);
      RET_ACK(SetSpecialReg);
      RET_ACK(SetGeneralRegs);
      RET_ACK(SetFloatRegs);
      RET_ACK(SetDebugRegs);
      RET_ACK(SetSpecialRegs);
      RET_ACK(ReleaseControl);
      RET_ACK(SetContinuationSignal);
   }
   assert(0);
   return 0;
}

uint16_t bgq_process::getCommandMsgType(uint16_t cmd_id)
{
   switch (cmd_id) {
      case GetSpecialRegs:
      case GetGeneralRegs:
      case GetFloatRegs:
      case GetDebugRegs:
      case GetMemory:
      case GetThreadList:
      case GetAuxVectors:
      case GetProcessData:
      case GetThreadData:
      case GetPreferences:
      case GetFilenames:
      case GetFileStatData:
      case GetFileContents:
         return Query;
      case GetSpecialRegsAck:
      case GetGeneralRegsAck:
      case GetFloatRegsAck:
      case GetDebugRegsAck:
      case GetMemoryAck:
      case GetThreadListAck:
      case GetAuxVectorsAck:
      case GetProcessDataAck:
      case GetThreadDataAck:
      case GetPreferencesAck:
      case GetFilenamesAck:
      case GetFileStatDataAck:
      case GetFileContentsAck:
         return QueryAck;
      case SetGeneralReg:
      case SetFloatReg:
      case SetDebugReg:
      case SetMemory:
      case HoldThread:
      case ReleaseThread:
      case InstallTrapHandler:
      case AllocateMemory:
      case SendSignal:
      case ContinueProcess:
      case StepThread:
      case SetBreakpoint:
      case ResetBreakpoint:
      case SetWatchpoint:
      case ResetWatchpoint:
      case RemoveTrapHandler:
      case SetPreferences:
      case FreeMemory:
      case SetSpecialReg:
      case SetGeneralRegs:
      case SetFloatRegs:
      case SetDebugRegs:
      case SetSpecialRegs:
      case ReleaseControl:
      case SetContinuationSignal:
         return Update;
      case SetGeneralRegAck:
      case SetFloatRegAck:
      case SetDebugRegAck:
      case SetMemoryAck:
      case HoldThreadAck:
      case ReleaseThreadAck:
      case InstallTrapHandlerAck:
      case AllocateMemoryAck:
      case SendSignalAck:
      case ContinueProcessAck:
      case StepThreadAck:
      case SetBreakpointAck:
      case ResetBreakpointAck:
      case SetWatchpointAck:
      case ResetWatchpointAck:
      case RemoveTrapHandlerAck:
      case SetPreferencesAck:
      case FreeMemoryAck:
      case SetSpecialRegAck:
      case SetGeneralRegsAck:
      case SetFloatRegsAck:
      case SetDebugRegsAck:
      case SetSpecialRegsAck:
      case ReleaseControlAck:
      case SetContinuationSignalAck:
         return UpdateAck;
      default:
         perr_printf("Unknown command %u\n", (unsigned) cmd_id);
         assert(0);
         return 0;
   }
}

