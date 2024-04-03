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

#ifndef _BPatch_binaryEdit_h_
#define _BPatch_binaryEdit_h_


//#include "BPatch_snippet.h"
#include "BPatch_dll.h"
#include "BPatch_Vector.h"
#include "BPatch_image.h"
#include "BPatch_addressSpace.h"

#include "BPatch_callbacks.h"

#include <vector>
#include <map>
#include <string>

#include <stdio.h>
#include <signal.h>

class BinaryEdit;
class AddressSpace;
class miniTrampHandle;
class BPatch;
class BPatch_thread;
class BPatch_process;
class BPatch_point;
class BPatch_funcMap;
class BPatch_instpMap;
class func_instance;
class rpcMgr;
struct batchInsertionRecord;

class BPATCH_DLL_EXPORT BPatch_binaryEdit : public BPatch_addressSpace {
    friend class BPatch;
    friend class BPatch_image;
    friend class BPatch_function;
    friend class BPatch_frame;
    friend class BPatch_module;
    friend class BPatch_basicBlock;
    friend class BPatch_flowGraph;
    friend class BPatch_loopTreeNode;
    friend class BPatch_point;
    friend class BPatch_funcCallExpr;
    friend class BPatch_instruction;
    friend class AstNode;
    friend class AstOperatorNode;
    friend class AstMemoryNode;

    private:
    std::map<std::string, BinaryEdit *> llBinEdits;
    std::map<std::string, BPatch_object*> loadedLibrary; 
    BinaryEdit *origBinEdit;

    std::vector<BinaryEdit *> rtLib;

    BPatch_binaryEdit(const char *path, bool openDependencies);
    bool creation_error;

    bool replaceTrapHandler();
    

    //        protected:
 public:
    void getAS(std::vector<AddressSpace *> &as);

    public:

    BinaryEdit *lowlevel_edit() const { return origBinEdit; }

    bool isMultiThreadCapable() const;
    processType getType();
    bool getTerminated() {return false;}
    bool getMutationsActive() {return true;}

    bool writeFile(const char * outFile);

  
    ~BPatch_binaryEdit();

    BPatch_image * getImage();

    void beginInsertionSet();

    bool finalizeInsertionSet(bool atomic, bool *modified = NULL);
                                       
    virtual BPatch_object * loadLibrary(const char *libname, bool reload = false);
};    

#endif /* BPatch_binaryEdit_h_ */
