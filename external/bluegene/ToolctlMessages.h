/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* This is an automatically generated copyright prolog.             */
/* After initializing,  DO NOT MODIFY OR MOVE                       */
/* ================================================================ */
/* IBM Confidential                                                 */
/*                                                                  */
/* Licensed Machine Code Source Materials                           */
/*                                                                  */
/* Product(s):                                                      */
/*     Blue Gene/Q Licensed Machine Code                            */
/*                                                                  */
/* (C) Copyright IBM Corp.  2011, 2011                              */
/*                                                                  */
/* The Source code for this program is not published  or otherwise  */
/* divested of its trade secrets,  irrespective of what has been    */
/* deposited with the U.S. Copyright Office.                        */
/* ================================================================ */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */

//! \file  ToolctlMessages.h
//! \brief Declarations for bgcios::toolctl message classes.

#ifndef TOOLCTLMESSAGES_H
#define TOOLCTLMESSAGES_H

// Includes
//#include <ramdisk/include/services/MessageHeader.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>

namespace bgcios
{
    namespace toolctl
    {

//! Type for thread ids.
typedef uint32_t BG_ThreadID_t;

//! Type for virtual addresses.
typedef uint64_t BG_Addr_t;

//! Type for special, general purpose, and debug registers.
typedef uint64_t BG_Reg_t;

//! Type for signal set;
typedef uint64_t BG_Sigset_t;


//-------------------------------
// Constants
//-------------------------------

//! Base port number for RDMA connections.
const uint16_t BaseRdmaPort = 7614;

//! Current version of protocol.
const uint8_t ProtocolVersion = 4;

//! Maximum number of query commands in one message.
const uint16_t MaxQueryCommands   = 16;

//! Maximum number of update commands in one message.
const uint16_t MaxUpdateCommands  =  16;

//! Maximum number of bytes of memory in GetMemoryAck and SetMemory commands.
const uint32_t MaxMemorySize = 65024;

//! Maximum number of thread ids in GetThreadListAck command.
const uint32_t MaxThreadIds = 1024;

//! Maximum number of aux vector words in GetAuxVectorAck command.
const uint32_t MaxAuxVecDWords = 32;

//! Maximum number of stack frames in GetThreadDataAck command.
const uint32_t MaxStackFrames = 400;

//! Number of general purpose registers.
const uint32_t NumGPRegs = 32;

//! Number of floating point registers.
const uint32_t NumFPRegs = 32;

//! Priority level for query access in Attach message.
const uint8_t QueryPriorityLevel = 0;

//! Minimum priority level for update access in Attach message.
const uint8_t MinUpdatePriorityLevel = 1;

//! Maximum priority level for update access in Attach message.
const uint8_t MaxUpdatePriorityLevel = 99;

//! Maximum number of active tools.
const size_t MaxActiveTools = 4;

//! Maximum number of ranks per compute node.
const uint16_t MaxRanksPerNode = 64;

//! Maximum pathname size for persistent files
const uint16_t MaxPersistPathnameSize = 128;

//! Maximum number of persistent files to be returned
const uint16_t MaxPersistPathnames = 256;

//!Tool tag size
const uint16_t ToolTagSize = 8;

//! Message types

const uint16_t ErrorAck           = 5000;
const uint16_t Attach             = 5001;
const uint16_t AttachAck          = 5002;
const uint16_t Detach             = 5003;
const uint16_t DetachAck          = 5004;
const uint16_t Query              = 5005;
const uint16_t QueryAck           = 5006;
const uint16_t Update             = 5007;
const uint16_t UpdateAck          = 5008;
const uint16_t SetupJob           = 5009;
const uint16_t SetupJobAck        = 5010;
const uint16_t Notify             = 5011;
const uint16_t NotifyAck          = 5012;
const uint16_t Control            = 5013;
const uint16_t ControlAck         = 5014;

//! Command types for Query and QueryAck messages.

const uint16_t GetSpecialRegs     = 101;
const uint16_t GetSpecialRegsAck  = 102;
const uint16_t GetGeneralRegs     = 103;
const uint16_t GetGeneralRegsAck  = 104;
const uint16_t GetFloatRegs       = 105;
const uint16_t GetFloatRegsAck    = 106;
const uint16_t GetDebugRegs       = 107;
const uint16_t GetDebugRegsAck    = 108;
const uint16_t GetMemory          = 109;
const uint16_t GetMemoryAck       = 110;
const uint16_t GetThreadList      = 111;
const uint16_t GetThreadListAck   = 112;
const uint16_t GetAuxVectors      = 113;
const uint16_t GetAuxVectorsAck   = 114;
const uint16_t GetProcessData     = 115;
const uint16_t GetProcessDataAck  = 116;
const uint16_t GetThreadData      = 117;
const uint16_t GetThreadDataAck   = 118;
const uint16_t GetPreferences     = 119;
const uint16_t GetPreferencesAck  = 120;
const uint16_t GetFilenames       = 121;
const uint16_t GetFilenamesAck    = 122;
const uint16_t GetFileStatData    = 123;
const uint16_t GetFileStatDataAck = 124;
const uint16_t GetFileContents    = 125;
const uint16_t GetFileContentsAck = 126;

//! Command types for Update and UpdateAck messages.

const uint16_t SetGeneralReg            = 201;
const uint16_t SetGeneralRegAck         = 202;
const uint16_t SetFloatReg              = 203;
const uint16_t SetFloatRegAck           = 204;
const uint16_t SetDebugReg              = 205;
const uint16_t SetDebugRegAck           = 206;
const uint16_t SetMemory                = 207;
const uint16_t SetMemoryAck             = 208;
const uint16_t HoldThread               = 209;
const uint16_t HoldThreadAck            = 210;
const uint16_t ReleaseThread            = 211;
const uint16_t ReleaseThreadAck         = 212;
const uint16_t InstallTrapHandler       = 213;
const uint16_t InstallTrapHandlerAck    = 214;
const uint16_t AllocateMemory           = 215;
const uint16_t AllocateMemoryAck        = 216;
const uint16_t SendSignal               = 217;
const uint16_t SendSignalAck            = 218;
const uint16_t ContinueProcess          = 219;
const uint16_t ContinueProcessAck       = 220;
const uint16_t StepThread               = 221;
const uint16_t StepThreadAck            = 222;
const uint16_t SetBreakpoint            = 223;
const uint16_t SetBreakpointAck         = 224;
const uint16_t ResetBreakpoint          = 225;
const uint16_t ResetBreakpointAck       = 226;
const uint16_t SetWatchpoint            = 227;
const uint16_t SetWatchpointAck         = 228;
const uint16_t ResetWatchpoint          = 229;
const uint16_t ResetWatchpointAck       = 230;
const uint16_t RemoveTrapHandler        = 233;
const uint16_t RemoveTrapHandlerAck     = 234;
const uint16_t SetPreferences           = 235;
const uint16_t SetPreferencesAck        = 236;
const uint16_t FreeMemory               = 237;
const uint16_t FreeMemoryAck            = 238;
const uint16_t SetSpecialReg            = 239;
const uint16_t SetSpecialRegAck         = 240;
const uint16_t SetGeneralRegs           = 241;
const uint16_t SetGeneralRegsAck        = 242;
const uint16_t SetFloatRegs             = 243;
const uint16_t SetFloatRegsAck          = 244;
const uint16_t SetDebugRegs             = 245; 
const uint16_t SetDebugRegsAck          = 246;
const uint16_t SetSpecialRegs           = 247; 
const uint16_t SetSpecialRegsAck        = 248;
const uint16_t ReleaseControl           = 249;
const uint16_t ReleaseControlAck        = 250;
const uint16_t SetContinuationSignal    = 251;
const uint16_t SetContinuationSignalAck = 252;

 

// ------------------------------
// Enumerations
// ------------------------------

//! Process select enumerations.

enum ProcessSelect
{
    RankInHeader = 0,         //!< Target is the rank number in the message header.
    RanksInNode               //!< Target is all ranks within the node.
};

//! Dynamic notify mode enumerations.

enum DynamicNotifyMode
{
    DynamicNotifyDLoader = 0, //!< On job start, interrupt executable at start of the dynamic linker.
    DynamicNotifyStart        //!< On job start, interrupt executable at start of the application.
};

//! Data Address Compare interrupt behavior enumerations.

enum DACTrapMode
{
    DACTrap_NoChange = 0, //!< Do not modify the current DAC Trap Mode preference.    
    TrapOnDAC,            //!< Data address compare interrupts will be presented on the instruction that attempted to access the data. 
    TrapAfterDAC          //!< Data address compare interrupts will be presented on the instruction following the data access. 
};

//! Notify message types

enum NotifyMessageType
{
    NotifyMessageType_Signal,      //!< Notify message is due to the occurrence of a signal.
    NotifyMessageType_Termination, //!< Notify message is due to process termination.
    NotifyMessageType_Control      //!< Notify message is due to a change or conflict in control authority.
};

//! Notification reason enumerations. 

enum NotifySignalReason
{
    NotifySignal_Generic,         //!< Notification due to a signal.
    NotifySignal_Breakpoint,      //!< Notification due to a breakpoint trap condition.
    NotifySignal_WatchpointRead,  //!< Notification due to a read watchpoint match.
    NotifySignal_WatchpointWrite, //!< Notification due to a write watchpoint match.
    NotifySignal_StepComplete,    //!< Notification due to a step request completion.
};

//! Notification control reason enumerations.

enum NotifyControlReason
{
    NotifyControl_Conflict,       //!< Notification due to another tool requesting control authority.
    NotifyControl_Available       //!< Notification due to a tool relinquishing control authority.
};

// Notification control action for the ReleaseControl message.

enum ReleaseControlNotify
{
    ReleaseControlNotify_Inactive, //!< Do not send an Available notify message when the next tool releases control.
    ReleaseControlNotify_Active    //!< Send Available notify message when next tool releases control.
};

//! Speculative access control.

enum SpecAccess
{
    SpecAccess_UseThreadState = 0,  //!< Read and write operations will use the current state of the thread.
    SpecAccess_ForceNonSpeculative  //!< Read and write operations will be forced to be non-speculative.
};

//! Shared memory access control enumerations.

enum SharedMemoryAccess
{
    SharedMemoryAccess_NotAllow = 0, //!< Do not allow shared process memory to be operated on.
    SharedMemoryAccess_Allow         //!< Allow shared process memory to be operatated on.
};

//! Fast Breakpoint enable/disable enumerations.

enum FastBreakMode
{
    FastBreak_NoChange = 0, //!< Do not modify the current FastTrapMode preference.
    FastBreak_Disable,      //!< Disable Fast Breakpoint mode.
    FastBreak_Enable        //!< Enable Fast Breakpoint mode. 
};

//! Fast Watchpoint enabled/disable enumerations.

enum FastWatchMode
{
    FastWatch_NoChange = 0, //!< Do not modify the current FastTrapMode preference.
    FastWatch_Disable,      //!< Disable Fast Watchpoint mode.
    FastWatch_Enable        //!< Enable Fast Watchpoint mode. 
};

//! Command Return Code enumerations. 

enum CmdReturnCode
{
    CmdSuccess    = 0,       //!< The command executed successfully.
    CmdTIDinval   = 1,       //!< The Thread ID specified no longer corresponds to an active thread in this process or the thread is not in the expected state.
    CmdInval      = 2,       //!< The command is not recognized.
    CmdTimeout    = 3,       //!< Timeout waiting for the target thread to respond.
    CmdAllocErr   = 4,       //!< Could not allocate necessary memory in the outbound message area.
    CmdParmErr    = 5,       //!< An invalid paramter was supplied with the command.
    CmdBrkptFail  = 6,       //!< A failure occurred while attempting to set or clear a breakpoint.
    CmdAddrErr    = 7,       //!< Invalid value provided for the address parameter. 
    CmdLngthErr   = 8,       //!< Invalid value provided for the length parameter.
    CmdHwdUnavail = 9,       //!< Hardware resource conflict.
    CmdMemUnavail = 10,      //!< The requested memory allocation could not be satified.
    CmdFileNotFound= 11,     //!< The requested filename was not found.
    CmdConflict   = 12,      //!< The command conflicts with other commands in the command list.
    CmdPendingNotify = 13    //!< The release control command could not complete because of a pending signal notify that has not been satisfied.
};

//! State of a thread enumerations.

enum BG_Thread_State
{
    Run = 1,     //!< Thread is running executable code.
    Sleep,       //!< Thread is in a timed sleep.
    FutexWait,   //!< Thread is waiting on a futex.
    Idle         //!< Thread is available.
};

//! Tool state of thread enumerations.

enum BG_Thread_ToolState
{
    Active = 1,  //!< Thread state has not been modified by a tool.
    Hold,        //!< The thread is in the Hold state. A ReleaseThread will remove the hold.
    Suspend,     //!< Signal delivery on a thread in the process has suspended this thread. 
    HoldSuspend  //!< Thread is Suspended and Held.
};

//! Speculation state of thread enumerations.

enum BG_Thread_SpecState
{
    NonSpeculative,                 //!< Thread is running in non-speculative mode.
    TransactionalMemory,            //!< Thread is running in transactional memory mode.
    TransactionalMemory_Invalid,    //!< Thread is running in transactional memory mode, but the specid is invalid.  
                                    //!< Read operations return non-speculative storage, write operations are discarded.
    SpeculativeExecution,           //!< Thread is running in speculative execution mode.
    SpeculativeExecution_Invalid    //!< Thread is running in speculative execution mode, but the specid is invalid.  
                                    //!< Read operations return non-speculative storage, write operations are discarded.
};

//! Watchpoint type enumerations.

enum WatchType
{
    WatchRead = 1,  //!< Watch read. 
    WatchWrite,     //!< Watch write.
    WatchReadWrite  //!< Watch read and write.
};

//! Set Special Register enumerations.

enum SpecialRegSelect 
{
    iar = 1,   //!< Instruction address register.
    lr,        //!< Link register.
    msr,       //!< Machine status register.
    cr,        //!< Condition register.
    ctr,       //!< Count register.
    xer,       //!< XER register.
    fpscr,     //!< Floating point status and control register.
    dear,      //!< Data effective address register.
    esr        //!< Exception status register.
};

//! Set General Register enumerations.

enum GeneralRegSelect
{
    gpr0=0,gpr1,gpr2,gpr3,gpr4,gpr5,gpr6,gpr7, 
    gpr8,gpr9,gpr10,gpr11,gpr12,gpr13,gpr14,gpr15,      
    gpr16,gpr17,gpr18,gpr19,gpr20,gpr21,gpr22,gpr23,     
    gpr24,gpr25,gpr26,gpr27,gpr28,gpr29,gpr30,gpr31      
};

//! Set General Register enumerations.

enum FloatRegSelect
{
    fpr0=0,fpr1,fpr2,fpr3,fpr4,fpr5,fpr6,fpr7, 
    fpr8,fpr9,fpr10,fpr11,fpr12,fpr13,fpr14,fpr15,      
    fpr16,fpr17,fpr18,fpr19,fpr20,fpr21,fpr22,fpr23,     
    fpr24,fpr25,fpr26,fpr27,fpr28,fpr29,fpr30,fpr31      
};

//! Set Debug Register enumerations.

enum DebugRegSelect
{
    dbcr0 = 1,  //!< Debug control register 0.
    dbcr1,      //!< Debug control register 1.
    dbcr2,      //!< Debug control register 2.
    dbcr3,      //!< Debug control register 3.
    dac1,       //!< Data address compare register 1.
    dac2,       //!< Data address compare register 2.
    dac3,       //!< Data address compare register 3.
    dac4,       //!< Data address compare register 4.
    iac1,       //!< Instruction address compare register 1.
    iac2,       //!< Instruction address compare register 2.
    iac3,       //!< Instruction address compare register 3.
    iac4,       //!< Instruction address compare register 4.
    dbsr        //!< Debug status register.
};


//-------------------------------
// Message and Command Structures
//-------------------------------

//! QPX Floating point register

union BG_Float
{
    double    d[ 4];
    float     f[ 8];
    uint64_t ll[ 4];
    uint32_t  l[ 8];
    uint8_t   b[32];
};

//! Descriptor for a command in Query, QueryAck, Update and UpdateAck messages.

struct CommandDescriptor
{
    uint16_t type;        //!< Command type.
    uint16_t reserved;    //!< Reserved for future use.
    uint32_t offset;      //!< Offset to command in message.
    uint32_t length;      //!< Length of command data.
    uint32_t returnCode;  //!< Command return code.
};

//! Tool message base class

class ToolMessage
{
public:
    struct MessageHeader header;        //!< Message header.
    uint32_t toolId;                    //!< Tool identifier.
};

//! Message to acknowledge a message that is in error.

struct ErrorAckMessage : public ToolMessage
{
};

//! Message to attach to a compute node process.

struct AttachMessage : public ToolMessage 
{
public:
    char           toolTag[ToolTagSize];  //!< Character string identifier provided by the calling tool.
    uint8_t        priority;              //!< Priority level for delivery of Notify message.
    ProcessSelect  procSelect;            //!< Attach to either all processes in the node or one process in the node
};

//! Message to acknowledge attaching to a compute node process.

struct AttachAckMessage : public ToolMessage
{
    uint16_t numProcess;             //!< Number of processes attached to.
    uint32_t rank[MaxRanksPerNode]; //!< Rank numbers of all attached processes.
};

//! Message to request control authority.

class ControlMessage : public ToolMessage 
{
public:
    BG_Sigset_t       notifySet;        //!< Set of signals that tool is notified of when encountered. Use notifySignalSet(sigset_t*) to initialize
    uint32_t          sndSignal;        //!< Signal, if any, to be sent to the process leader after control authority has been granted (e.g. SIGTRAP, SIGSTOP)
    DynamicNotifyMode dynamicNotifyMode;//!< Dynamic application attachment notify mode.
    DACTrapMode       dacTrapMode;      //!< Data Address Compare trap presentation mode.

    //! Set the signals of interest for this tool
    inline void notifySignalSet(sigset_t *sigset)
    {
        // convert the user sigset data into format required by the kernel. 
        notifySet = *((uint64_t*)sigset);
    }
};

//! Message to acknowledge request for control authority.

struct ControlAckMessage : public ToolMessage
{
    uint16_t controllingToolId; //!< ToolID that has control authority.
    char     toolTag[ToolTagSize];        //!< ToolTag of the tool that has control authority
    uint8_t  priority;          //!< Priority of the tool that has control authority
};

//! Message to detach from a compute node process.

struct DetachMessage : public ToolMessage
{
    ProcessSelect  procSelect;  //!< Detach to either all processes in the node or one process in the node
};

//! Message to acknowledge detaching from a compute node process.

struct DetachAckMessage : public ToolMessage
{
    uint16_t numProcess;   //!< Number of processes detached.
    uint32_t rank[MaxRanksPerNode];     //!< Rank numbers of all detached processes.
};

//! Message to query a compute node process.

struct QueryMessage : public ToolMessage
{
    uint16_t numCommands;                               //!< Number of commands in message.
    struct CommandDescriptor cmdList[MaxQueryCommands]; //!< List of command descriptors.
};

//! Message to acknowledge querying a compute node process.

struct QueryAckMessage : public ToolMessage
{
    uint16_t numCommands;                               //!< Number of commands in message.
    struct CommandDescriptor cmdList[MaxQueryCommands]; //!< List of command descriptors.
};

//! Message to update state of a compute node process.

struct UpdateMessage : public ToolMessage
{
    uint16_t numCommands;                                //!< Number of commands in message.
    struct CommandDescriptor cmdList[MaxUpdateCommands]; //!< List of command descriptors.
};

//! Message to acknowledge updating state of a compute node process.

struct UpdateAckMessage : public ToolMessage
{
    uint16_t numCommands;                                //!< Number of commands in message.
    struct CommandDescriptor cmdList[MaxUpdateCommands]; //!< List of command descriptors.
};

//! Message to setup for running a new job.

struct SetupJobMessage : public ToolMessage
{
    uint16_t numRanks;                  //!< Number of ranks running on compute node for job. 
    uint32_t ranks[MaxRanksPerNode];    //!< Rank of each process running on compute node.
    uid_t    userId;                    //!< User id.
    gid_t    groupId;                   //!< Primary group id.
    uint32_t nodeId;                    //!< Node id used to name symbolic link in toolctl_node directory.
                                        //!< The value matches the pid field in the MPIR_PROCDESC structure.
};

//! Message to acknowledge setting up for a new job.

struct SetupJobAckMessage : public ToolMessage
{
};

//! Message to notify of an event within the compute node change of a compute node process.

struct NotifyMessage : public ToolMessage
{
    NotifyMessageType notifyMessageType;    //!< Which type of notify message is this.
    union
    {
        struct {                            //!< Definition for NotifyMessageType_Signal.
            uint32_t           signum;      //!< Signal number.
            BG_ThreadID_t      threadID;    //!< Thread id.
            BG_Addr_t          instAddress; //!< Address of interrupted instruction (signal notification).
            BG_Addr_t          dataAddress; //!< Address of the data access causing a watchpoint event.
            NotifySignalReason reason;      //!< Signal Notification reason.
        } signal;
        struct {                            //!< Definition for NotifyMessageType_Termination.
            int                exitStatus;  //!< Process exit status. Byte2: exit return code Byte3: terminating signal number
        } termination;
        struct {                            //!< Definition for NotifyMessageType_Control.
            uint32_t            toolid;     //!< ToolId.
            char                toolTag[ToolTagSize]; //!< Tool character string.
            uint8_t             priority;   //!< Tool priority.
            NotifyControlReason reason;     //!< Control Authority notification reason: Conflict or Available.
        } control;
    } type;
};

//! Message to acknowledge notifying status change of a compute node process.

struct NotifyAckMessage : public ToolMessage
{
};

//! Structure for stack frame entry.

struct BG_Stack_Info
{
    BG_Addr_t frameAddr;  //!< Address of next stack frame entry.
    BG_Addr_t savedLR;    //!< Saved link register.
};

//! Structure for debug registers

struct BG_Debug_Regs
{
    BG_Reg_t dbcr0;    //!< Debug control register 0.
    BG_Reg_t dbcr1;    //!< Debug control register 1.
    BG_Reg_t dbcr2;    //!< Debug control register 2.
    BG_Reg_t dbcr3;    //!< Debug control register 3.
    BG_Reg_t dac1;     //!< Data address compare register 1.
    BG_Reg_t dac2;     //!< Data address compare register 2.
    BG_Reg_t dac3;     //!< Data address compare register 3.
    BG_Reg_t dac4;     //!< Data address compare register 4.
    BG_Reg_t iac1;     //!< Instruction address compare register 1.
    BG_Reg_t iac2;     //!< Instruction address compare register 2.
    BG_Reg_t iac3;     //!< Instruction address compare register 3.
    BG_Reg_t iac4;     //!< Instruction address compare register 4.
    BG_Reg_t dbsr;     //!< Debug status register.
};

//! Structure for special registers
 
struct BG_Special_Regs
{
    BG_Reg_t iar;      //!< Instruction address register.
    BG_Reg_t lr;       //!< Link register.
    BG_Reg_t msr;      //!< Machine state register.
    BG_Reg_t cr;       //!< Condition register.
    BG_Reg_t ctr;      //!< Count register.
    BG_Reg_t xer;      //!< Integer exception register.
    BG_Reg_t fpscr;    //!< Floating point status and control register.
    BG_Reg_t dear;     //!< Data exception address register.
    BG_Reg_t esr;      //!< Exception syndrome register.
};

//! Tool command base class

class ToolCommand
{
public:
    BG_ThreadID_t threadID;             //!< Thread id.
};

//! Command to get special registers.

struct GetSpecialRegsCmd : public ToolCommand
{
};

//! Command to acknowledge getting special registers.

struct GetSpecialRegsAckCmd : public ToolCommand
{
    BG_Special_Regs sregs;
};

//! Command to get general purpose registers.

struct GetGeneralRegsCmd : public ToolCommand
{
};

//! Command to acknowledge getting general purpose registers.

struct GetGeneralRegsAckCmd : public ToolCommand
{
    BG_Reg_t gpr[NumGPRegs];           //!< Get of general purpose registers.
};

//! Command to get floating point registers.

struct GetFloatRegsCmd : public ToolCommand
{
};

//! Command to acknowledge getting floating point registers.

struct GetFloatRegsAckCmd : public ToolCommand
{
    BG_Float fpr[NumFPRegs];           //!< Get of floating point registers
};

//! Command to get debug registers.

struct GetDebugRegsCmd : public ToolCommand
{
};

//! Command to acknowledge getting debug registers.

struct GetDebugRegsAckCmd : public ToolCommand
{
    BG_Debug_Regs dbregs;               //!< Debug registers
};

//! Command to get memory contents.

struct GetMemoryCmd : public ToolCommand
{
    BG_Addr_t addr;                     //!< Address of memory to return.
    uint32_t length;                    //!< Length of memory in bytes.
    SpecAccess specAccess;              //!< Should speculative access be permitted. 
};

//! Command to acknowledge getting memory contents.

struct GetMemoryAckCmd : public ToolCommand
{
    BG_Addr_t addr;                     //!< Address of returned memory.
    uint32_t length;                    //!< Length of memory in bytes.
    unsigned char data[0];              //!< Memory contents. Variable size data area up to MaxMemorySize.
};

//! Command to get aux vector data.

struct GetAuxVectorsCmd : public ToolCommand
{
};

//! Command to acknowledge getting aux vector data.

struct GetAuxVectorsAckCmd : public ToolCommand
{
    uint32_t length;                    //!< Length of data in bytes.
    uint64_t data[MaxAuxVecDWords];     //!< Aux vector data.
};

//! Command to get process data.

struct GetProcessDataCmd : public ToolCommand
{
};

//! Command to acknowledge getting process data.

struct GetProcessDataAckCmd : public ToolCommand
{
    uint32_t  rank;                      //!< MPI rank.
    uint32_t  tgid;                      //!< Thread group id.
    uint32_t  aCoord;                    //!< A coordinate.
    uint32_t  bCoord;                    //!< B coordinate.
    uint32_t  cCoord;                    //!< C coordinate.
    uint32_t  dCoord;                    //!< D coordinate.
    uint32_t  eCoord;                    //!< E coordinate.
    uint32_t  tCoord;                    //!< T coordinate.
    BG_Addr_t sharedMemoryStartAddr;     //!< Shared memory region start address.
    BG_Addr_t sharedMemoryEndAddr;       //!< Shared memory region end address.
    BG_Addr_t persistMemoryStartAddr;    //!< Persistent memory region start address.
    BG_Addr_t persistMemoryEndAddr;      //!< Persistent memory region end address.
    BG_Addr_t heapStartAddr;             //!< Heap start address.
    BG_Addr_t heapEndAddr;               //!< Heap end address. 
    BG_Addr_t heapBreakAddr;             //!< Heap break address.
    BG_Addr_t mmapStartAddr;             //!< Memory map start address.
    BG_Addr_t mmapEndAddr;               //!< Memory map end address.
    struct timeval jobTime;              //!< Time job has been running.
};

//! Command to get thread data.

struct GetThreadDataCmd : public ToolCommand
{
};


//! Command to acknowledge getting thread data.

struct GetThreadDataAckCmd : public ToolCommand
{
    int                 core;                //!< Processor Core ID (0..16).
    int                 thread;              //!< Processor Thread ID (0..3).
    BG_Thread_State     state;               //!< State of thread.
    BG_Thread_ToolState toolState;           //!< Tool State of thread.
    BG_Thread_SpecState specState;           //!< State of thread's speculative accesses.
    BG_Addr_t           guardStartAddr;      //!< Guard page start address.
    BG_Addr_t           guardEndAddr;        //!< Guard page end address.
    BG_Addr_t           stackStartAddr;      //!< Stack start address.
    BG_Addr_t           stackCurrentAddr;    //!< Current stack address.
    uint32_t            numStackFrames;      //!< Number of stack frame entries in list.
    BG_Stack_Info stackInfo[MaxStackFrames]; //!< Stack trace back.
};

//! Command to get the list of threads for a process.

struct GetThreadListCmd : public ToolCommand
{
};

//! Command to acknowledge getting the list of threads.

struct GetThreadListAckCmd : public ToolCommand
{
    uint32_t numthreads;                 //!< Number of threads returned.
    struct
    {
        uint32_t tid;                    //!< Thread identifier.
        struct
        {
            uint32_t commthread : 1;     //!< Commthread indicator.
            uint32_t unused : 31;
        } info;
    } threadlist[MaxThreadIds];          //!< Thread list entries.
};

//! Command to send a signal.

struct SendSignalCmd : public ToolCommand
{
    uint32_t signum;                     //!< Signal number to send.
};

//! Command to acknowledge getting a signal.

struct SendSignalAckCmd : public ToolCommand
{
};

//! Command to set special register.

struct SetSpecialRegCmd : public ToolCommand
{
    SpecialRegSelect reg_select;        //!< Special purpose register selection.
    BG_Reg_t         value;             //!< Register value.
};

//! Command to acknowledge setting special register.

struct SetSpecialRegAckCmd : public ToolCommand
{
};

//! Command to set set general register.

struct SetGeneralRegCmd : public ToolCommand
{
    GeneralRegSelect reg_select;        //!< General purpose register selection.
    BG_Reg_t         value;             //!< Register value.
};

//! Command to acknowledge setting general register.

struct SetGeneralRegAckCmd : public ToolCommand
{
};

//! Command to set set general registers.

struct SetGeneralRegsCmd : public ToolCommand
{
    BG_Reg_t gpr[NumGPRegs];           //!< Set of general purpose registers.
};

//! Command to acknowledge setting general registers.

struct SetGeneralRegsAckCmd : public ToolCommand
{
};

//! Command to set floating point register.

struct SetFloatRegCmd : public ToolCommand
{
    FloatRegSelect reg_select;         //!< Floating point register selection.
    BG_Float       value;              //!< Register value.

};

//! Command to acknowledge setting of floating point register.

struct SetFloatRegAckCmd : public ToolCommand
{
};

//! Command to set floating point registers.

struct SetFloatRegsCmd : public ToolCommand
{
    BG_Float fpr[NumFPRegs];           //!< Set of floating point registers
};

//! Command to acknowledge setting of floating point registers.

struct SetFloatRegsAckCmd : public ToolCommand
{
};

//! Command to set a debug register.

struct SetDebugRegCmd : public ToolCommand
{
    DebugRegSelect reg_select;          //!< Debug register selection.
    BG_Reg_t       value;               //!< Register value.
};

//! Command to acknowledge setting of a debug register.

struct SetDebugRegAckCmd : public ToolCommand
{
};

//! Command to set a debug registers.

struct SetDebugRegsCmd : public ToolCommand
{
    BG_Debug_Regs dbregs;               //!< Debug registers
};

//! Command to acknowledge setting of a debug registers.

struct SetDebugRegsAckCmd : public ToolCommand
{
};

//! Command to set a special registers.

struct SetSpecialRegsCmd : public ToolCommand
{
    BG_Special_Regs sregs;               //!< Special registers
};

//! Command to acknowledge setting of a special registers.

struct SetSpecialRegsAckCmd : public ToolCommand
{
};

//! Command to set memory. To write a memory breakpoint, a separate command is provided. 

struct SetMemoryCmd : public ToolCommand
{
    BG_Addr_t          addr;                //!< Address of memory to set.
    uint32_t           length;              //!< Length of memory in bytes.
    SpecAccess         specAccess;          //!< Should speculative access be permitted.
    SharedMemoryAccess sharedMemoryAccess;  //!< Should Shared process data writes be allowed. 
    unsigned char      data[0];             //!< Memory contents. Variable size data area up to MaxMemorySize
};

//! Command to acknowledge setting of memory.

struct SetMemoryAckCmd : public ToolCommand
{
    BG_Addr_t addr;     //!< Address of memory.
    uint32_t  length;   //!< Length of memory in bytes.
};

//! Command to hold a thread.

struct HoldThreadCmd : public ToolCommand
{
};

//! Command to acknowledge setting a hold on a thread.

struct HoldThreadAckCmd : public ToolCommand
{
};

//! Command to release a thread.

struct ReleaseThreadCmd : public ToolCommand
{
};

//! Command to acknowledge releasing of a thread.

struct ReleaseThreadAckCmd : public ToolCommand
{
};

//! Command to release all threads in a process.

struct ReleaseAllThreadsCmd : public ToolCommand
{
};

//! Command to acknowledge releasing of all threads.

struct ReleaseAllThreadsAckCmd : public ToolCommand
{
};

//! Command to install a trap handler.

struct InstallTrapHandlerCmd : public ToolCommand
{
    void  (*trap_handler)(int, siginfo_t *, void *);   //!< Signal handler function for SIGTRAP
};

//! Command to acknowledge installing a trap handler.

struct InstallTrapHandlerAckCmd : public ToolCommand
{
};

//! Command to remove a trap handler.

struct RemoveTrapHandlerCmd : public ToolCommand
{
};

//! Command to acknowledge removing a trap handler.

struct RemoveTrapHandlerAckCmd : public ToolCommand
{
};

//! Command to mmap.

struct AllocateMemoryCmd : public ToolCommand
{
    size_t alloc_size;  //!< Amount of memory to allocate.
};

//! Command to acknowledge mmaping.

struct AllocateMemoryAckCmd : public ToolCommand
{
    void *alloc_addr;   //!< Address of allocated memory.
};

//! Command to free an mmap allocation.

struct FreeMemoryCmd : public ToolCommand
{
    void  *alloc_addr;  //!< Address of memory to be freed.
    size_t alloc_size;  //!< Size of the memory to be freed. 
};

//! Command to acknowledge freeing an mmap allocation.

struct FreeMemoryAckCmd : public ToolCommand
{
};

//! Command to set the mask indicating the signals this tool is interested in.

struct SetSignalMaskCmd : public ToolCommand
{
};

//! Command to acknowledge setting of the signals mask.

struct SetSignalMaskAckCmd : public ToolCommand
{
};

//! Command to perform a continue process.

struct ContinueProcessCmd : public ToolCommand
{
};

//! Command to acknowledge the execution of a continue process.

struct ContinueProcessAckCmd : public ToolCommand
{
};

//! Command to perform a step.

struct StepThreadCmd : public ToolCommand
{
};

//! Command to acknowledge the execution of a step request.

struct StepThreadAckCmd : public ToolCommand
{
};

//! Command to set a memory breakpoint.

struct SetBreakpointCmd : public ToolCommand
{
    BG_Addr_t addr;        //!< Address of memory.
    uint32_t  instruction; //!< Trap instruction.
};

//! Command to acknowledge setting of a memory breakpoint.

struct SetBreakpointAckCmd : public ToolCommand
{
};

//! Command to reset a memory breakpoint.

struct ResetBreakpointCmd : public ToolCommand
{
    BG_Addr_t addr;        //!< Address of memory.
    uint32_t  instruction; //!< Original instruction
};

//! Command to acknowledge resetting of a memory breakpoint.

struct ResetBreakpointAckCmd : public ToolCommand
{
};

//! Command to set a memory watchpoint. 
//! The address must be aligned on the same boundary as the length. For example
//! a watch of an 8 byte area must have the address starting on an 8 byte boundary.
//!   
struct SetWatchpointCmd : public ToolCommand
{
    BG_Addr_t  addr;      //!< Address of memory.
    uint32_t   length;    //!< Length of watched memory area
    WatchType  type;      //!< Type of watch: read, write, any.
};

//! Command to acknowledge setting of a memory watchpoint.

struct SetWatchpointAckCmd : public ToolCommand
{
};

//! Command to reset a memory watchpoint.

struct ResetWatchpointCmd : public ToolCommand
{
    BG_Addr_t addr;       //!< Address of memory.
};

//! Command to acknowledge resetting of a memory watchpoint.

struct ResetWatchpointAckCmd : public ToolCommand
{
};

//! Command to set preferences.

struct SetPreferencesCmd : public ToolCommand
{
    FastBreakMode breakpointMode;  //!< Enable or disable fast breakpoints.
    FastWatchMode watchpointMode;  //!< Enable or disable fast watchpoints.
    DACTrapMode   dacMode;         //!< Data address compare interrupt delivery on/after the matching instruction.
};

//! Command to acknowledge set preferences.

struct SetPreferencesAckCmd : public ToolCommand
{
};
//! Command to get preferences.

struct GetPreferencesCmd : public ToolCommand
{
};

//! Command to acknowledge set preferences.

struct GetPreferencesAckCmd : public ToolCommand
{
    FastBreakMode breakpointMode;  //!< Are fast breakpoints enabled or disabled.
    FastWatchMode watchpointMode;  //!< Are fast watchpoints enabled or disabled.
    DACTrapMode   dacMode;         //!< Is trap-on or trap-after mode enabled.
};

//! Command to list the persistent files
struct GetFilenamesCmd : public ToolCommand
{
};

//! Ack Command to return a list of files

struct GetFilenamesAckCmd : public ToolCommand
{
    uint32_t numFiles;
    char     pathname[MaxPersistPathnames][MaxPersistPathnameSize];
};

//! Command to stat a file

struct GetFileStatDataCmd : public ToolCommand
{
    char pathname[MaxPersistPathnameSize];
};

//! Command to ack the stat of a file

struct GetFileStatDataAckCmd : public ToolCommand
{
    struct stat64 stat;
};

//! Command to read a file

struct GetFileContentsCmd : public ToolCommand
{
    char   pathname[MaxPersistPathnameSize];
    size_t offset;
    size_t numbytes;
};

//! Command to ack the read of a file

struct GetFileContentsAckCmd : public ToolCommand
{
    size_t numbytes;
    char   data[0];        //!< Variable size data area up to MaxMemorySize
};

//! Command to release control authority

struct ReleaseControlCmd : public ToolCommand
{
    ReleaseControlNotify notify;        //!< Controls the sending of the next Notify Available message to this tool.
};

//! Command to ack the release of control authority

struct ReleaseControlAckCmd : public ToolCommand
{
};

//! Command to set the continuation signal

struct SetContinuationSignalCmd : public ToolCommand
{
    uint32_t             signum;        //!< Signal to be set.     
};

//! Command to ack the set continuation signal

struct SetContinuationSignalAckCmd : public ToolCommand
{
};


    } // namespace toolctl

} // namespace bgcios

#endif // TOOLCTLMESSAGES_H

