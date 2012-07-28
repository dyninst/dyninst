/*
 * See the dyninst/COPYRIGHT file for copyright information.
 * 
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
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
#ifndef __IPC_H__
#define __IPC_H__

#include <cstdarg>
using namespace std;

#include "config.h"
#include "log.h"

enum statusID {
    ID_INFO,
    ID_TEST,
    ID_WARN,
    ID_FAIL,
    ID_PASS
};

enum messageID {
    ID_INVALID,

    ID_INIT_CREATE_BPATCH,
    ID_INIT_REGISTER_EXIT,
    ID_INIT_REGISTER_FORK,
    ID_INIT_CREATE_PROCESS,
    ID_INIT_ATTACH_PROCESS,
    ID_INIT_MERGE_TRAMP,
    ID_INIT_GET_IMAGE,
    ID_POST_FORK,

    ID_PARSE_MODULE,
    ID_PARSE_FUNC,
    ID_PARSE_MODULE_CFG,
    ID_PARSE_FUNC_CFG,

    ID_INST_START_TRANS,
    ID_INST_END_TRANS,

    ID_INST_MODULE,
    ID_INST_FUNC,
    ID_INST_FUNC_ENTRY,
    ID_INST_FUNC_EXIT,
    ID_INST_BASIC_BLOCK,
    ID_INST_MEM_READ,
    ID_INST_MEM_WRITE,

    ID_INST_GET_BB,
    ID_NO_BB,
    ID_INST_GET_BB_POINTS,
    ID_GET_CFG,
    ID_INST_BB_LIST,

    ID_INST_FIND_INT,
    ID_INST_MALLOC_INT,
    ID_FAILED_INST_INIT_INT,
    ID_INST_GET_FUNCS,
    ID_INST_FIND_POINTS,
    ID_NO_POINTS,
    ID_INST_INSERT_CODE,

    ID_TRACE_INIT,
    ID_TRACE_INIT_MUTATEE,
    ID_TRACE_FIND_OPEN,
    ID_TRACE_FIND_WRITE,
    ID_TRACE_OPEN_READER,
    ID_TRACE_OPEN_WRITER,
    ID_TRACE_INSERT,
    ID_TRACE_INSERT_ONE,
    ID_TRACE_READ,
    ID_TRACE_POINT,

    ID_ALLOC_COUNTER,

    ID_RUN_CHILD,
    ID_WAIT_TERMINATION,
    ID_CHECK_TERMINATION,
    ID_WAIT_STATUS_CHANGE,
    ID_POLL_STATUS_CHANGE,
    ID_EXIT_CODE,
    ID_EXIT_SIGNAL,
    ID_DETACH_CHILD,
    ID_STOP_CHILD,

    ID_SUMMARY_INSERT,
    ID_SUMMARY_START,
    ID_SUMMARY_END,

    ID_DATA_STRING,

    ID_CRASH_HUNT_NUM_ACTIONS,

    ID_MAX
};

#define encodeID(msgID, pri, statID)	( ((msgID) << 8) | ((pri) << 4) | (statID) )
#define getMsgID(x)			((messageID)((x) >> 8))
#define getStatID(x)			((statusID)((x) & 0xF))
#define getPriID(x)			((logLevel)(((x) >> 4) & 0xF))

struct message {
    unsigned id_data;
    char *str_data;
    int int_data;
};

void setSigHandlers();
void resetSigHandlers();
const char *msgStr(messageID);
message *readMsg(FILE *, message *);
void printMsg(FILE *, messageID, int = 0);
void sendMsg(FILE *, messageID, logLevel, statusID = ID_TEST, const char * = NULL);
void sendMsg(FILE *, messageID, logLevel, statusID, int);
void killProcess(pid_t);
void cleanupProcess();
void cleanupFinal();

int readStr(FILE *, char *, int);

#endif
