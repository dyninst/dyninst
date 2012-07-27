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
#include <iostream>
#include <list>
#include <vector>
#include <string>
#include <algorithm>
#include <unistd.h>

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_snippet.h"
#include "BPatch_function.h"

#include "dyninstCompat.h"
#include "config.h"
#include "ipc.h"
#include "log.h"

using namespace std;

void printSummary(BPatch_thread *, BPatch_exitType);
void reportNewProcess(BPatch_thread *, BPatch_thread *);

dynHandle *mutatorInit(void)
{
    dynHandle *dh = new dynHandle;
    if (!dh) {
	dlog(ERR, "Could not allocate %d bytes of memory in mutatorInit().\n", sizeof(dynHandle));
	return NULL;
    }

    sendMsg(config.outfd, ID_INIT_CREATE_BPATCH, INFO);
    dh->bpatch = new BPatch;
    if (!dh->bpatch) {
	sendMsg(config.outfd, ID_INIT_CREATE_BPATCH, INFO, ID_FAIL,
		"Failure creating new BPatch object");
	return NULL;
    } else
	sendMsg(config.outfd, ID_INIT_CREATE_BPATCH, INFO, ID_PASS);

    /*
     * BPatch class level flags.
     */
    dh->bpatch->setDelayedParsing(true);

    /*
     * We should probably have a notion of a default callback
     * handler to return here instead of NULL to differentiate
     * between the error case.
     */
    sendMsg(config.outfd, ID_INIT_REGISTER_EXIT, INFO);
    dh->bpatch->registerExitCallback(printSummary);
    sendMsg(config.outfd, ID_INIT_REGISTER_EXIT, INFO, ID_PASS);

    sendMsg(config.outfd, ID_INIT_REGISTER_FORK, INFO);
    dh->bpatch->registerPostForkCallback(reportNewProcess);
    sendMsg(config.outfd, ID_INIT_REGISTER_FORK, INFO, ID_PASS);

    if (config.use_attach) {
	sendMsg(config.outfd, ID_INIT_ATTACH_PROCESS, INFO);
	dh->proc = dh->bpatch->attachProcess(config.target, config.attach_pid);
	if (!dh->proc) {
	    sendMsg(config.outfd, ID_INIT_ATTACH_PROCESS, INFO, ID_FAIL,
		    "Failure in BPatch::attachProcess()");
	    return NULL;
	} else {
	    config.dynlib = dh;
	    sendMsg(config.outfd, ID_INIT_ATTACH_PROCESS, INFO, ID_PASS,
		    dh->proc->getPid());
	}

    } else {
	sendMsg(config.outfd, ID_INIT_CREATE_PROCESS, INFO);
	dh->proc = dh->bpatch->createProcess(config.target, (const char **)config.argv);
	if (!dh->proc) {
	    sendMsg(config.outfd, ID_INIT_CREATE_PROCESS, INFO, ID_FAIL,
		    "Failure in BPatch::createProcess()");
	    return NULL;
	} else {
	    config.dynlib = dh;
	    sendMsg(config.outfd, ID_INIT_CREATE_PROCESS, INFO, ID_PASS,
		    dh->proc->getPid());
	}
    }

    sendMsg(config.outfd, ID_INIT_GET_IMAGE, INFO);
    dh->image = dh->proc->getImage();
    if (!dh->image) {
	sendMsg(config.outfd, ID_INIT_GET_IMAGE, INFO, ID_FAIL,
		"Failure in BPatch_process::getImage()");
	return NULL;
    } else
	sendMsg(config.outfd, ID_INIT_GET_IMAGE, INFO, ID_PASS);

    return dh;
}

bool dynStartTransaction(dynHandle *dh)
{
    sendMsg(config.outfd, ID_DATA_STRING, INFO, ID_INFO,
	    "Instrumentation transactions not available in DyninstAPI v4.x");
    return true;
}

bool dynEndTransaction(dynHandle *dh)
{
    return true;
}
