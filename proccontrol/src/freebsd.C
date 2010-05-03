/*
 * Copyright (c) 1996-2009 Barton P. Miller
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

#include "dynutil/h/dyn_regs.h"
#include "dynutil/h/dyntypes.h"
#include "proccontrol/h/PCErrors.h"
#include "proccontrol/h/Generator.h"
#include "proccontrol/h/Event.h"
#include "proccontrol/h/Handler.h"
#include "proccontrol/src/procpool.h"
#include "proccontrol/src/irpc.h"
#include "proccontrol/src/freebsd.h"
#include "proccontrol/src/int_handler.h"
#include "common/h/parseauxv.h"

#include <cassert>

using namespace Dyninst;
using namespace ProcControlAPI;

#define NA_FREEBSD "This function is not implemented on FreeBSD"

Generator *Generator::getDefaultGenerator() {
    static GeneratorFreeBSD *gen = NULL;
    if( NULL == gen ) {
        gen = new GeneratorFreeBSD();
        assert(gen);
        gen->launch();
    }
    return static_cast<Generator *>(gen);
}

GeneratorFreeBSD::GeneratorFreeBSD() :
    GeneratorMT(std::string("FreeBSD Generator"))
{
    decoders.insert(new DecoderFreeBSD());
}

GeneratorFreeBSD::~GeneratorFreeBSD()
{
}

bool GeneratorFreeBSD::initialize() {
    return true;
}

bool GeneratorFreeBSD::canFastHandle() {
    return false;
}

ArchEvent *GeneratorFreeBSD::getEvent(bool block) {

}

int_process *int_process::createProcess(int, std::string) {
    assert(!NA_FREEBSD);
    return NULL;
}

int_process *int_process::createProcess(std::string, std::vector<std::string>) {
    assert(!NA_FREEBSD);
    return NULL;
}

int_process *int_process::createProcess(int, int_process *) {
    assert(!NA_FREEBSD);
    return NULL;
}

int_thread *int_thread::createThreadPlat(int_process *, Dyninst::THR_ID,
        Dyninst::LWP, bool)
{
    assert(!NA_FREEBSD);
    return NULL;
}

bool installed_breakpoint::plat_install(int_process *, bool) {
    assert(!NA_FREEBSD);
    return NULL;
}

HandlerPool *plat_createDefaultHandlerPool(HandlerPool *) {
    assert(!NA_FREEBSD);
    return NULL;
}

bool ProcessPool::LWPIDsAreUnique() {
    assert(!NA_FREEBSD);
    return NULL;
}

bool iRPCMgr::collectAllocationResult(int_thread *, Dyninst::Address &, bool &) {
    assert(!NA_FREEBSD);
    return false;
}

bool iRPCMgr::createDeallocationSnippet(int_process *, Dyninst::Address, unsigned long,
        void * &, unsigned long &, unsigned long &)
{
    assert(!NA_FREEBSD);
    return false;
}

bool iRPCMgr::createAllocationSnippet(int_process *, Dyninst::Address,
        bool, unsigned long, void * &, unsigned long &, unsigned long &)
{
    assert(!NA_FREEBSD);
    return false;
}

bool int_notify::createPipe() {
    assert(!NA_FREEBSD);
    return false;
}

void int_notify::readFromPipe() {
    assert(!NA_FREEBSD);
}

void int_notify::writeToPipe() {
    assert(!NA_FREEBSD);
}

freebsd_process::freebsd_process(Dyninst::PID p, std::string e, std::vector<std::string> a)
    : sysv_process(p, e, a)
{
    assert(!NA_FREEBSD);
}

freebsd_process::freebsd_process(Dyninst::PID pid_, int_process *p) 
    : sysv_process(pid_, p) 
{
    assert(!NA_FREEBSD);
}

freebsd_process::~freebsd_process() {
    assert(!NA_FREEBSD);
}

bool freebsd_process::plat_create() {
    assert(!NA_FREEBSD);
    return false;
}

bool freebsd_process::plat_attach() {
    assert(!NA_FREEBSD);
    return false;
}

bool freebsd_process::plat_forked() {
    assert(!NA_FREEBSD);
    return false;
}

bool freebsd_process::post_forked() {
    assert(!NA_FREEBSD);
    return false;
}

bool freebsd_process::plat_execed() { 
    assert(!NA_FREEBSD);
    return false;
}

bool freebsd_process::plat_detach() {
    assert(!NA_FREEBSD);
    return false;
}

bool freebsd_process::plat_terminate(bool &) {
    assert(!NA_FREEBSD);
    return false;
}

bool freebsd_process::plat_readMem(int_thread *, void *, 
                         Dyninst::Address, size_t) {
    assert(!NA_FREEBSD);
    return false;
}

bool freebsd_process::plat_writeMem(int_thread *, void *, 
                          Dyninst::Address, size_t) {
    assert(!NA_FREEBSD);
    return false;
}

bool freebsd_process::needIndividualThreadAttach() {
    assert(!NA_FREEBSD);
    return false;
}

bool freebsd_process::getThreadLWPs(std::vector<Dyninst::LWP> &) {
    assert(!NA_FREEBSD);
    return false;
}

Dyninst::Architecture freebsd_process::getTargetArch() {
    assert(!NA_FREEBSD);
    return Dyninst::Arch_none;
}

unsigned freebsd_process::getTargetPageSize() {
    assert(!NA_FREEBSD);
    return 0;
}

Dyninst::Address freebsd_process::plat_mallocExecMemory(Dyninst::Address, unsigned) {
    assert(!NA_FREEBSD);
    return 0;
}

bool freebsd_process::independentLWPControl() {
    assert(!NA_FREEBSD);
    return false;
}

bool freebsd_process::plat_individualRegAccess() {
    assert(!NA_FREEBSD);
    return false;
}

freebsd_thread::freebsd_thread(int_process *p, Dyninst::THR_ID t, Dyninst::LWP l)
    : int_thread(p, t, l)
{
    assert(!NA_FREEBSD);
}

freebsd_thread::~freebsd_thread() {
    assert(!NA_FREEBSD);
}

bool freebsd_thread::plat_cont() {
    assert(!NA_FREEBSD);
    return false;
}

bool freebsd_thread::plat_stop() {
    assert(!NA_FREEBSD);
    return false;
}

bool freebsd_thread::plat_getAllRegisters(int_registerPool &) {
    assert(!NA_FREEBSD);
    return false;
}

bool freebsd_thread::plat_getRegister(Dyninst::MachRegister, Dyninst::MachRegisterVal &) {
    assert(!NA_FREEBSD);
    return false;
}

bool freebsd_thread::plat_setAllRegisters(int_registerPool &) {
    assert(!NA_FREEBSD);
    return false;
}

bool freebsd_thread::plat_setRegister(Dyninst::MachRegister, Dyninst::MachRegisterVal) {
    assert(!NA_FREEBSD);
    return false;
}

bool freebsd_thread::attach() {
    assert(!NA_FREEBSD);
    return false;
}
