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

//! \file  MessageHeader.h
//! \brief Declaration for bgcios::MessageHeader class.

#ifndef MESSAGEHEADER_H
#define MESSAGEHEADER_H

// Includes
#include <inttypes.h>
#include <string.h>

namespace bgcios
{

//! Header describing a I/O service message.

struct MessageHeader
{
   uint8_t  service;              //!< Service to process message.
   uint8_t  version;              //!< Protocol version number.
   uint16_t type;                 //!< Content of message.
   uint32_t rank;                 //!< Rank message is associated with.
   uint32_t sequenceId;           //!< Correlate requests and acknowledgements.
   uint32_t returnCode;           //!< Result of previous request.
   uint32_t errorCode;            //!< Error detail (typically errno value).
   uint32_t length;               //!< Amount of data in message (including this header).
   uint64_t jobId;                //!< Job message is associated with.
};

//! Values for service field of MessageHeader.

const uint8_t IosctlService  = 1; //!< I/O control service.
const uint8_t JobctlService  = 2; //!< Job control service.
const uint8_t StdioService   = 3; //!< Standard I/O service.
const uint8_t SysioService   = 4; //!< System I/O service.
const uint8_t ToolctlService = 5; //!< Tool control service.

//! Values for return code field of MessageHeader.

enum ReturnCode {
   Success = 0,                   //!< Success (no error).
   WrongService,                  //!< Service value in message header is not valid.
   UnsupportedType,               //!< Type value in message header is not supported.
   JobIdError,                    //!< Job id value is not valid.
   ProcessIdError,                //!< Rank value is not valid.
   RequestFailed,                 //!< Requested operation failed.   
   SubBlockJobError,              //!< Sub-block job specifications are not valid.  
   SendError,                     //!< Sending a message failed.
   RecvError,                     //!< Receiving a message failed.
   VersionMismatch,               //!< Protocol versions do not match.
   NodeNotReady,                  //!< Compute node is not ready for requested operation.
   SecondaryGroupIdError,         //!< Setting secondary group id failed.
   PrimaryGroupIdError,           //!< Setting primary group id failed.
   UserIdError,                   //!< Setting user id failed.
   WorkingDirError,               //!< Changing to working directory failed.
   AppOpenError,                  //!< Opening application executable failed.
   AppAuthorityError,             //!< No authority to application executable.
   AppReadError,                  //!< Reading data from application executable failed.
   AppElfHeaderSize,              //!< Application executable ELF header is wrong size.
   AppElfHeaderError,             //!< Application executable ELF header contains invalid value.
   AppNoCodeSection,              //!< Application executable contains no code sections.
   AppCodeSectionSize,            //!< Application executable code section is too big.
   AppSegmentAlignment,           //!< Application executable segment has wrong alignment.
   AppTooManySegments,            //!< Application executable has too many segments.
   AppStaticTLBError,             //!< Generating static TLB map for application failed.
   AppMemoryError,                //!< Initializing memory for process failed.
   ArgumentListSize,              //!< Argument list has too many items.
   ToolStartError,                //!< Starting tool process failed.
   ToolAuthorityError,            //!< No authority to tool executable.
   ToolIdError,                   //!< Tool id is not valid.
   ToolTimeoutExpired,            //!< Timeout expired ending a tool.
   ToolPriorityConflict,          //!< Tool priority conflict.
   ToolMaxAttachedExceeded,       //!< Tool maximum number of tools exceeded.
   ToolIdConflict,                //!< Tool id conflict.
   JobsDirError,                  //!< Creating /jobs directory failed.
   JobsObjectError,               //!< Creating object in /jobs directory failed.
   ToolPriorityError,             //!< Tool priority level is not valid.
   ToolRankNotFound,              //!< Tool request could not find requested target process.
   CornerCoreError,               //!< Corner core number is not valid.
   NumCoresInProcessError,        //!< Number of cores allocated to a process is not valid.
   ProcessActive,                 //!< Process is currently active on a hardware thread.
   NumProcessesError,             //!< Number of processes on node is not valid.
   RanksInJobError,               //!< Number of active ranks in job is not valid.
   ClassRouteDataError,           //!< Class route data is not valid.
   ToolNumberOfCmdsExceeded,      //!< Tool number of commands is not valid.
   RequestIncomplete,             //!< Requested operation was partially successful.
   PrologPgmStartError,           //!< Starting job prolog program process failed.
   PrologPgmError,                //!< Job prolog program failed. 
   EpilogPgmStartError,           //!< Starting job epilog program process failed.
   EpilogPgmError,                //!< Job epilog program failed. 
   RequestInProgress,             //!< Requested operation is currently in progress.
   ToolControlConflict,           //!< Control authority conflict with another tool.
   NodesInJobError,               //!< No compute nodes matched job specifications.
   ToolConflictingCmds,           //!< An invalid combination of commands was specifed in the command list.
   ReturnCodeListEnd              //!< End of the return code enumerations. 
};

//! Default size for small message memory regions (match Linux page size).
const uint32_t SmallMessageRegionSize = 65536;

//! Maximum size of data in a small message.
const uint32_t SmallMessageDataSize = 65472;

//! Size of an immediate message (fits in one packet).
const uint32_t ImmediateMessageSize = 512;

//! \brief  Initialize message header.
//! \param  header Pointer to message header.
//! \return Nothing.

inline void initHeader(struct MessageHeader *header) { memset(header, 0x00, sizeof(struct MessageHeader)); }

//! \brief  Get the length of the data in a message (can be zero).
//! \param  header Pointer to message header.
//! \return Length of data in message.

inline uint32_t dataLength(struct MessageHeader *header) { return header->length - sizeof(struct MessageHeader); }

} // namespace bgcios

#endif // MESSAGEHEADER_H

