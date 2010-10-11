/*
 * Copyright (c) 1996-2010 Barton P. Miller
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

#include "pcProcess.h"
#include "pcThread.h"
#include "function.h"

PCProcess *PCProcess::createProcess(const std::string file, pdvector<std::string> *argv,
                                    BPatch_hybridMode &analysisMode,
                                    pdvector<std::string> *envp,
                                    const std::string dir, int stdin_fd, int stdout_fd,
                                    int stderr_fd)
{
    return NULL;
}

PCProcess *PCProcess::attachProcess(const std::string &progpath, int pid,
                                    void *container_proc_,
                                    BPatch_hybridMode &analysisMode)
{
    return NULL;
}

bool PCProcess::continueProcess() {
    return false;
}

bool PCProcess::stopProcess() {
    return false;
}

bool PCProcess::terminateProcess() {
    return false;
}

bool PCProcess::detachProcess(bool /*cont*/) {
    return false;
}

bool PCProcess::dumpCore(const std::string coreFile) {
    return false;
}

bool PCProcess::isBootstrapped() const {
    return false;
}

bool PCProcess::isAttached() const {
    return false;
}

bool PCProcess::isStopped() const {
    return false;
}

bool PCProcess::isTerminated() const {
    return false;
}

bool PCProcess::hasExited() const {
    return false;
}

bool PCProcess::isExecing() const {
    return false;
} 

bool PCProcess::writeDebugDataSpace(void *inTracedProcess, u_int amount,
                         const void *inSelf)
{
    return false;
}

bool PCProcess::writeDataSpace(void *inTracedProcess,
                    u_int amount, const void *inSelf)
{
    return false;
}

bool PCProcess::writeDataWord(void *inTracedProcess,
                   u_int amount, const void *inSelf) 
{
    return false;
}

bool PCProcess::readDataSpace(const void *inTracedProcess, u_int amount,
                   void *inSelf, bool displayErrMsg)
{
    return false;
}

bool PCProcess::readDataWord(const void *inTracedProcess, u_int amount,
                  void *inSelf, bool displayErrMsg)
{
    return false;
}

bool PCProcess::writeTextSpace(void *inTracedProcess, u_int amount, const void *inSelf)
{
    return false;
}

bool PCProcess::writeTextWord(void *inTracedProcess, u_int amount, const void *inSelf)
{
    return false;
}

bool PCProcess::readTextSpace(const void *inTracedProcess, u_int amount,
                   void *inSelf)
{
    return false;
}

bool PCProcess::readTextWord(const void *inTracedProcess, u_int amount,
                  void *inSelf)
{
    return false;
}

PCThread *PCProcess::getInitialThread() const {
    return NULL;
}

PCThread *PCProcess::getThread(dynthread_t tid) {
    return NULL;
}

void PCProcess::getThreads(pdvector<PCThread* > &threads) {
}

bool PCProcess::wasRunningWhenAttached() const {
    return false;
}

bool PCProcess::wasCreatedViaAttach() const {
    return false;
}

unsigned PCProcess::getMemoryPageSize() const {
    return 0;
}

int PCProcess::getPid() const {
    return 0;
}

unsigned PCProcess::getAddressWidth() const {
    return 0;
}

bool PCProcess::isMultithreadCapable() const {
    return false;
}

bool PCProcess::walkStacks(pdvector<pdvector<Frame> > &stackWalks) {
    return false;
}

void PCProcess::getActiveMultiMap(std::map<Address, multiTramp *> &map) {

}

void PCProcess::updateActiveMultis() {

}

void PCProcess::addActiveMulti(multiTramp *multi) {

}

Address PCProcess::inferiorMalloc(unsigned size, inferiorHeapType type,
                       Address near_, bool *err)
{
    return 0;
}

bool PCProcess::uninstallMutations() {
    return false;
}

bool PCProcess::reinstallMutations() {
    return false;
}

bool PCProcess::postRPC(AstNodePtr action,
             void *userData,
             bool runProcessWhenDone,
             bool lowmem, PCThread *thread,
             bool synchronous)
{
    return false;
}

BPatch_hybridMode PCProcess::getHybridMode() {
    return BPatch_normalMode;
}

bool PCProcess::setMemoryAccessRights(Address start, Address size, int rights) {
    return false;
}

bool PCProcess::getOverwrittenBlocks(std::map<Address, unsigned char *>& overwrittenPages,//input
                          std::map<Address,Address>& overwrittenRegions,//output
                          std::set<bblInstance *> &writtenBBIs) //output
{
    return false;
}

void PCProcess::updateMappedFile(std::map<Dyninst::Address,unsigned char*>& owPages,
                      std::map<Address,Address> owRegions)
{
}

bool PCProcess::getDeadCodeFuncs(std::set<bblInstance *> &deadBlocks, // input
                      std::set<int_function*> &affectedFuncs, //output
                      std::set<int_function*> &deadFuncs) //output
{
    return false;
}

void PCProcess::debugSuicide() {
}

void PCProcess::deleteThread(dynthread_t tid) {
}

bool PCProcess::dumpImage(std::string outFile) {
    return false;
}

void PCProcess::cleanupProcess() {
}

int PCProcess::getStopThreadCB_ID(const Address cb) {
    return 0;
}

// Function relocation requires a version of process::convertPCsToFuncs 
// in which null functions are not passed into ret. - Itai 
pdvector<int_function *> PCProcess::pcsToFuncs(pdvector<Frame> stackWalk) {
    pdvector <int_function *> ret;
    unsigned i;
    int_function *fn;
    for(i=0;i<stackWalk.size();i++) {
        fn = (int_function *)findFuncByAddr(stackWalk[i].getPC());
        // no reason to add a null function to ret
        if (fn != 0) ret.push_back(fn);
    }
    return ret;
}

bool PCProcess::isInSignalHandler(long unsigned int&) {
    return false;
}

void PCProcess::installInstrRequests(const pdvector<instMapping*> &requests) {

}

int_function *PCProcess::findActiveFuncByAddr(Address addr) {
    return NULL;
}

bool PCProcess::mappedObjIsDeleted(mapped_object *obj) {
    return false;
}

// power only -- needs refactoring
Address PCProcess::getTOCoffsetInfo(Address) {
    return 0;
}

Address PCProcess::getTOCoffsetInfo(int_function *) {
    return 0;
}
