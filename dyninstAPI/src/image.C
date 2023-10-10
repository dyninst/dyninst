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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <string>
#include <fstream>

#include "image.h"
#include "common/src/arch.h"
#include "parRegion.h"
#include "util.h"
#include "inst.h"
#include "debug.h"
#include "function.h"
#include "Parsing.h"

#include "common/src/Timer.h"
#include "common/src/pathName.h"
#include "common/src/MappedFile.h"

#include "dyninstAPI/h/BPatch_flowGraph.h"
#include "common/h/util.h"

#include "symtabAPI/h/Function.h"

#include "parseAPI/h/InstructionSource.h"
#include "parseAPI/h/CodeObject.h"
#include "parseAPI/h/CFG.h"

#include "dataflowAPI/h/AbslocInterface.h"
#include "dataflowAPI/h/SymEval.h"

#if defined(TIMED_PARSE)
#include <sys/time.h>
#endif

#if defined( cap_dwarf )
#include "dwarf.h"
#include "elfutils/libdw.h"
#endif

#if defined(_MSC_VER)
#include <dbghelp.h>
#endif

// For callbacks
#include "dyninstAPI/src/mapped_object.h" 

AnnotationClass<image_variable> ImageVariableUpPtrAnno("ImageVariableUpPtrAnno", NULL);
AnnotationClass<parse_func> ImageFuncUpPtrAnno("ImageFuncUpPtrAnno", NULL);
std::vector<image*> allImages;

using namespace std;
using namespace Dyninst;
using namespace Dyninst::ParseAPI;

using Dyninst::SymtabAPI::Symtab;
using Dyninst::SymtabAPI::Symbol;
using Dyninst::SymtabAPI::Region;
using Dyninst::SymtabAPI::Variable;
using Dyninst::SymtabAPI::Module;

char main_function_names[NUMBER_OF_MAIN_POSSIBILITIES][20] = {
    "main",
    "DYNINST_pltMain",
    "_main",
    "WinMain",
    "_WinMain",
    "wWinMain",
    "_wWinMain",
    "tls_cb_0"};

fileDescriptor::fileDescriptor():
        code_(0), data_(0),
        pid_(0), length_(0)
{
    // This shouldn't be called... must be public for std::vector, though
}

bool fileDescriptor::IsEqual(const fileDescriptor &fd) const {
    // Don't test isShared, only file name and addresses
    bool file_match_ = false;

    // Annoying... we often get "foo vs ./foo" or such. So consider it a match
    // if either file name is prefixed by the other; we don't get trailing crud.
    string::size_type len1 = file_.length();
    string::size_type len2 = fd.file_.length();
  
    if(((len1>=len2) && (file_.substr(len1-len2,len2) == fd.file_))
       || ((len2>len1) && (fd.file_.substr(len2-len1,len1) == file_)))
        file_match_ = true;   
#if defined(os_linux)
    struct stat buf1;
    struct stat buf2;
    if (!stat(file_.c_str(),&buf1)
        && !stat(fd.file_.c_str(),&buf2)
        && buf1.st_ino == buf2.st_ino) {
        file_match_ = true;
    }
#endif  

#if defined(os_windows)
    if(extract_pathname_tail(file_) == extract_pathname_tail(fd.file_)) file_match_ = true;
#endif
#if 0
    cerr << hex << "Addr comparison: " << code_ << " ? " << fd.code_
         << ", " << data_ << " ? " << fd.data_ << dec << endl;
#endif
    bool addr_match = (code_ == fd.code_ && data_ == fd.data_);
#if 0
    cerr << "file " << file_match_ 
         << ", addr " << addr_match
         << ", member_ " << (member_ == fd.member_)
         << ", pid_ " << (pid_ == fd.pid_) << endl;
#endif
    if (file_match_ &&
        (addr_match) &&
        (member_ == fd.member_) &&
        (pid_ == fd.pid_))
        return true;
    else
        return false;
}

// only for non-files
void* fileDescriptor::rawPtr()
{
#if defined(os_windows)

    return rawPtr_;

#else

    return NULL;

#endif
}


// All debug_ostream vrbles are defined in process.C (for no particular reason)
extern unsigned enable_pd_sharedobj_debug;

int codeBytesSeen = 0;

#if defined(ppc64_linux)

#include <dataflowAPI/h/slicing.h>
#include <dataflowAPI/h/SymEval.h>
#include <dataflowAPI/h/AbslocInterface.h>
#include <dataflowAPI/h/Absloc.h>
#include <common/h/DynAST.h>

namespace {
    /* On PPC GLIBC (32 & 64 bit) the address of main is in a structure
       located in either .data or .rodata, depending on whether the 
       binary is PIC. The structure has the following format:

        struct 
        {
            void * // "small data area base"
            main   // pointer to main
            init   // pointer to init
            fini   // pointer to fini
        }

        This structure is passed in GR8 as an argument to libc_start_main.

        Annoyingly, the value in GR8 is computed in several different ways,
        depending on how GLIBC was compiled.

        This code follows the i386 linux version closely otherwise.
    */

    class Default_Predicates : public Slicer::Predicates {};

    /* This visitor is capable of simplifying constant value computations
       that involve additions and concatenations (lis instruction). This
       is sufficient to handle the startup struct address calculation in
       GLIBC that we have seen; if additional variants are introduced
       (refer to start.S in glibc or equivalently to the compiled library)
       this visitor should be expanded to handle any new operations */
        
    class SimpleArithVisitor : public ASTVisitor {

        using ASTVisitor::visit;

        virtual ASTPtr visit(AST * a) {return a->ptr();}
        virtual ASTPtr visit(DataflowAPI::BottomAST *a) {return a->ptr(); }
        virtual ASTPtr visit(DataflowAPI::ConstantAST *c) {return c->ptr();}
        virtual ASTPtr visit(DataflowAPI::VariableAST *v) {return v->ptr();}

        virtual ASTPtr visit(DataflowAPI::RoseAST * r) {
            using namespace DataflowAPI;

            AST::Children newKids;
            for(unsigned i=0;i<r->numChildren();++i) {
                newKids.push_back(r->child(i)->accept(this));
            }

            switch(r->val().op) {
                case ROSEOperation::addOp:
                    assert(newKids.size() == 2);
                    if(newKids[0]->getID() == AST::V_ConstantAST &&
                       newKids[1]->getID() == AST::V_ConstantAST)
                    {
                        ConstantAST::Ptr c1 = ConstantAST::convert(newKids[0]);
                        ConstantAST::Ptr c2 = ConstantAST::convert(newKids[1]);
                        return ConstantAST::create(
                            Constant(c1->val().val+c2->val().val));
                    }
                    break;
                case ROSEOperation::concatOp:
                    assert(newKids.size() == 2);
                    if(newKids[0]->getID() == AST::V_ConstantAST &&
                       newKids[1]->getID() == AST::V_ConstantAST)
                    {
                        ConstantAST::Ptr c1 = ConstantAST::convert(newKids[0]);
                        ConstantAST::Ptr c2 = ConstantAST::convert(newKids[1]);
                        unsigned long result = c1->val().val;
                        result |= (c2->val().val << c2->val().size);
                        return ConstantAST::create(result);
                    }
                    break;
                default:
                    startup_printf("%s[%d] unhandled operation in simplification\n",FILE__,__LINE__);
            }
        
            return RoseAST::create(r->val(), newKids);
        }
    };

    struct libc_startup_info {
        void * sda;
        void * main_addr;
        void * init_addr;
        void * fini_addr;
    };

    void *get_raw_symtab_ptr(Symtab *linkedFile, Address addr)
    {
        Region *reg = linkedFile->findEnclosingRegion(addr);
        if (reg != NULL) {
            char *data = (char*)reg->getPtrToRawData();
            data += addr - reg->getMemOffset();
            return data;
        }
        return NULL;
    }

    Address deref_opd(Symtab *linkedFile, Address addr)
    {
        Region *reg = linkedFile->findEnclosingRegion(addr);
        if (reg && reg->getRegionName() == ".opd") {
            // opd symbol needing dereference
            void *data = get_raw_symtab_ptr(linkedFile, addr);
            if (data)
                return *(Address*)data;
        }
        return addr;
    }

    /*
     * b ends with a call to libc_start_main. We are looking for the
     * value in GR8, which is the address of a structure that contains
     * the address to main
     */
    Address evaluate_main_address(Symtab * linkedFile, Function * f, Block *b)
    {
        using namespace DataflowAPI;
        using namespace InstructionAPI;
        // looking for the *last* instruction in the block
        // that defines GR8
    
        Instruction r8_def;
        Address r8_def_addr;
        bool find = false;
    
        InstructionDecoder dec(
            b->region()->getPtrToInstruction(b->start()),
            b->end()-b->start(),
            b->region()->getArch());

        RegisterAST::Ptr r2( new RegisterAST(ppc32::r2) );
        RegisterAST::Ptr r8( new RegisterAST(ppc32::r8) );

        Address cur_addr = b->start();
        while(cur_addr < b->end()) {
            Instruction cur = dec.decode();
            if(cur.isWritten(r8)) {
                find = true;
                r8_def = cur;
                r8_def_addr = cur_addr;  
            }
            cur_addr += cur.size();
        }
        if(!find)
            return 0;

        Address ss_addr = 0;

        // Try a TOC-based lookup first
        if (r8_def.isRead(r2)) {
            set<Expression::Ptr> memReads;
            r8_def.getMemoryReadOperands(memReads);
            Address TOC = f->obj()->cs()->getTOC(r8_def_addr);
            if (TOC != 0 && memReads.size() == 1) {
                Expression::Ptr expr = *memReads.begin();
                expr->bind(r2.get(), Result(u64, TOC));
                const Result &res = expr->eval();
                if (res.defined) {
                    void *res_addr =
                        get_raw_symtab_ptr(linkedFile, res.convert<Address>());
                    if (res_addr)
                        ss_addr = *(Address*)res_addr;
                }
            }
        }

        if (ss_addr == 0) {
            // Get all of the assignments that happen in this instruction
            AssignmentConverter conv(true, true);
            vector<Assignment::Ptr> assigns;
            conv.convert(r8_def,r8_def_addr,f,b,assigns);

            // find the one we care about (r8)
            vector<Assignment::Ptr>::iterator ait = assigns.begin();
            for( ; ait != assigns.end(); ++ait) {
                AbsRegion & outReg = (*ait)->out();
                Absloc const& loc = outReg.absloc();
                if(loc.reg() == r8->getID())
                    break;
            }
            if(ait == assigns.end()) {
                return 0;
            }

            // Slice back to the definition of R8, and, if possible, simplify
            // to a constant
            Slicer slc(*ait,b,f);
            Default_Predicates preds;
            Graph::Ptr slg = slc.backwardSlice(preds);
            DataflowAPI::Result_t sl_res;
            DataflowAPI::SymEval::expand(slg,sl_res);
            AST::Ptr calculation = sl_res[*ait];
            SimpleArithVisitor visit; 
            AST::Ptr simplified = calculation->accept(&visit);
            //printf("after simplification:\n%s\n",simplified->format().c_str());
            if(simplified->getID() == AST::V_ConstantAST) { 
                ConstantAST::Ptr cp = ConstantAST::convert(simplified);
                ss_addr = cp->val().val;
            }
        }

        // need a pointer to the image data
        auto si = (struct libc_startup_info *)
            get_raw_symtab_ptr(linkedFile, ss_addr);
        if (si)
            return (Address)si->main_addr;

        return 0;
    }
}
#endif

#include <Graph.h>
#include <Node.h>
#include <DynAST.h>
#include <dyntypes.h>
#include <SymEval.h>
#include <slicing.h>

class FindMainVisitor : public ASTVisitor
{
    using ASTVisitor::visit;

    public:
    bool resolved;
    bool hardFault;
    Address target;
    FindMainVisitor() : resolved(false), hardFault(false), target(0) {}

    virtual AST::Ptr visit(DataflowAPI::RoseAST * r) 
    {
        using namespace DataflowAPI;

        AST::Children newKids;
        for(unsigned i = 0; i < r->numChildren(); i++) 
            newKids.push_back(r->child(i)->accept(this));

        switch(r->val().op) 
        {
            case ROSEOperation::addOp:

                assert(newKids.size() == 2);
                if(newKids[0]->getID() == AST::V_ConstantAST &&
                        newKids[1]->getID() == AST::V_ConstantAST)
                {
                    ConstantAST::Ptr c1 = ConstantAST::convert(newKids[0]);
                    ConstantAST::Ptr c2 = ConstantAST::convert(newKids[1]);
                    if(!hardFault)
                    {
                        target = c1->val().val + c2->val().val;
                        resolved = true;
                    }
                    return ConstantAST::create(
                            Constant(c1->val().val + c2->val().val));
                }
                break;
            default:
                startup_printf("%s[%d] unhandled FindMainVisitor operation\n",
                        FILE__,__LINE__);
        }

        return RoseAST::create(r->val(), newKids);
    }

    virtual ASTPtr visit(DataflowAPI::ConstantAST * c)
    {
        /* We can only handle constant values */
        if(!target && !hardFault) 
        {
            resolved = true;
            target = c->val().val;
        }

        return c->ptr();
    }

    virtual ASTPtr visit(DataflowAPI::VariableAST* v)
    {
        
        /* If we visit a variable node, we can't do any analysis */
        hardFault = true;
        resolved = false;
        target = 0;

        return v->ptr();
    }
};

/**
 * Search for the Main Symbols in the list of symbols, Only in case
 * if the file is a shared object. If not present add them to the
 * list. Returns zero on success, nonzero otherwise.
 */
int image::findMain()
{
#if defined(ppc64_linux)
    using namespace Dyninst::InstructionAPI;

    // Only look for main in executables, but do allow position-independent
    // executables (PIE) which look like shared objects with an INTERP.
    // (Some strange DSOs also have INTERP, but this is rare.)
    if (linkedFile->isExec())
    {
        bool foundMain = false;
        // check if 'main' is in allsymbols
        vector <SymtabAPI::Function *> funcs;
        if (linkedFile->findFunctionsByName(funcs, "main") ||
                linkedFile->findFunctionsByName(funcs, "_main"))  {
            foundMain = true;
        }

        Region *eReg = NULL;
        bool foundText = linkedFile->findRegion(eReg, ".text");

        if (!foundText)
            return -1;

        if(!foundMain)
        {
            logLine("No main symbol found: attempting to create symbol for main\n");

            Address eAddr = linkedFile->getEntryOffset();
            eAddr = deref_opd(linkedFile, eAddr);

            bool parseInAllLoadableRegions = (BPatch_normalMode != mode_);
            SymtabCodeSource scs(linkedFile, filt, parseInAllLoadableRegions);
            CodeObject tco(&scs,NULL,NULL,false);

            tco.parse(eAddr,false);
            set<CodeRegion *> regions;
            scs.findRegions(eAddr,regions);
            if(regions.empty()) {
                // express puzzlement
                return -1;
            }
            SymtabCodeRegion * reg = 
                static_cast<SymtabCodeRegion*>(*regions.begin());
            Function * func = 
                tco.findFuncByEntry(reg,eAddr);
            if(!func) {
                // again, puzzlement
                return -1;
            }

            Block * b = NULL;
            const Function::edgelist & calls = func->callEdges();
            if (calls.empty()) {
                // when there are no calls, let's hope the entry block is it
                b = tco.findBlockByEntry(reg,eAddr);
            } else if(calls.size() == 1) {
                Function::edgelist::iterator cit = calls.begin();
                b = (*cit)->src();
            } else {
                startup_printf("%s[%d] _start has unexpected number (%lu) of"
                        " call edges, bailing on findMain()\n",
                        FILE__,__LINE__,calls.size());
                return -1;
            }
            if (!b) return -1;

            Address mainAddress = evaluate_main_address(linkedFile,func,b);
            mainAddress = deref_opd(linkedFile, mainAddress);

            if(0 == mainAddress || !scs.isValidAddress(mainAddress)) {
                startup_printf("%s[%d] failed to find main\n",FILE__,__LINE__);
                return -1;
            } else {
                startup_printf("%s[%d] found main at %lx\n",
                        FILE__,__LINE__,mainAddress);
            }
            Symbol *newSym= new Symbol( "main", 
                    Symbol::ST_FUNCTION,
                    Symbol::SL_LOCAL,
                    Symbol::SV_INTERNAL,
                    mainAddress,
                    linkedFile->getDefaultModule(),
                    eReg, 
                    0 );
            linkedFile->addSymbol(newSym);		
        }
    }

#elif defined(i386_unknown_linux2_0) \
    || defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */ \
    || (defined(os_freebsd) \
            && (defined(arch_x86) || defined(arch_x86_64)))
    // Only look for main in executables, but do allow position-independent
    // executables (PIE) which look like shared objects with an INTERP.
    // (Some strange DSOs also have INTERP, but this is rare.)
    if(linkedFile->isExec())
    {
        bool foundMain = false;
        bool foundStart = false;
        bool foundFini = false;

        //check if 'main' is in allsymbols
        vector <SymtabAPI::Function *> funcs;
        if (linkedFile->findFunctionsByName(funcs, "main") ||
                linkedFile->findFunctionsByName(funcs, "_main")) {
            foundMain = true;
        }

        if (linkedFile->findFunctionsByName(funcs, "_start")) {
            foundStart = true;
        }

        if (linkedFile->findFunctionsByName(funcs, "_fini")) {
            foundFini = true;
        }

        Address eAddr = linkedFile->getEntryOffset();
        Region *eReg = linkedFile->findEnclosingRegion(eAddr);

        if (!eReg)
            return -1;
         
        // Address eStart = eReg->getMemOffset();

        if(!foundMain)
        {
            logLine( "No main symbol found: creating symbol for main\n" );

            //find and add main to allsymbols
            // const unsigned char* p;

            // p = (( const unsigned char * ) eReg->getPtrToRawData());

            // if (eAddr > eStart) {
                // p += (eAddr - eStart);
            // }

//            switch(linkedFile->getAddressWidth()) {
//                case 4:
//                    // 32-bit...
//                    startup_printf("%s[%u]:  setting 32-bit mode\n",
//                            FILE__,__LINE__);
//                    ia32_set_mode_64(false);
//                    break;
//                case 8:
//                    startup_printf("%s[%u]:  setting 64-bit mode\n",
//                            FILE__,__LINE__);
//                    ia32_set_mode_64(true);
//                    break;
//                default:
//                    assert(0 && "Illegal address width");
//                    break;
//            }

            Address mainAddress = 0;

            // Create a temporary SymtabCodeSource that we can use for parsing. 
            // We're going to throw it away when we're done so that we can re-sync
            // with the new symbols we're going to add shortly. 
            bool parseInAllLoadableRegions = (BPatch_normalMode != mode_);
            SymtabCodeSource scs(linkedFile, filt, parseInAllLoadableRegions);
            CodeObject co(&scs);

#if !defined(os_freebsd)
            /* Find the entry point, where we start our analysis */
            Address entry_point = (Address)linkedFile->getEntryOffset();

            /* Get the code regions we are looking at */
            std::set<CodeRegion*> regions;
            scs.findRegions(entry_point, regions);

            /* We should only get one region */
            if(regions.size() != 1)
            {
                startup_printf("%s[%d]: Overlapping or non existant regions!\n",
                        FILE__, __LINE__);
                return -1;
            }

            CodeRegion* region = *regions.begin();
            assert(region);

            /* Parse the function we're looking at */
            co.parse(region, entry_point, true);

            /* Get the parsed Function */
            Function* func = co.findFuncByEntry(region, entry_point);

            if(!func)
            {
                startup_printf("%s[%d]: No functions found in our region.\n",
                        FILE__, __LINE__);
                return -1;
            }

            /* Get the call edges for this function */
            Function::edgelist list = func->callEdges();
            
            /* There should be at least one edge */
            ParseAPI::Edge* e = *list.begin();

            if(!e)
            {
                startup_printf("%s[%d]: Error: no call edges found for this function.\n",
                        FILE__, __LINE__);
                return -1;
            }

            /* get the block for this call edge (source) */
            Block* b = e->src();
            assert(b);

	    Block::Insns insns;
	    b->getInsns(insns);
	    if (insns.size() < 2) {
	        startup_printf("%s[%d]: should have at least two instructions\n", FILE__, __LINE__);   
		return -1;
	    }

	    // To get the secont to last instruction, which loads the address of main
	    auto iit = insns.end();
	    --iit;
	    --iit;	    

            /* Let's get the assignment for this instruction. */
            std::vector<Assignment::Ptr> assignments;
            Dyninst::AssignmentConverter assign_convert(true, false);
            assign_convert.convert(iit->second, iit->first, func, b, assignments);
            if(assignments.size() >= 1)
            {
	        
                Assignment::Ptr assignment = assignments[0];
		std::pair<AST::Ptr, bool> res = DataflowAPI::SymEval::expand(assignment, false);
		AST::Ptr ast = res.first;
                if(!ast)
                {
                    /* expand failed */
                    mainAddress = 0x0;
		    startup_printf("%s[%d]:  cannot expand %s from instruction %s\n", FILE__, __LINE__, assignment->format().c_str(),
                           assignment->insn().format().c_str());
                } else { 
		    startup_printf("%s[%d]:  try to visit  %s\n", FILE__, __LINE__, ast->format().c_str());   
                    FindMainVisitor fmv;
                    ast->accept(&fmv);
                    if(fmv.resolved)
                    {
                        mainAddress = fmv.target;
                    } else {
                        mainAddress = 0x0;
			startup_printf("%s[%d]:  FindMainVisitor cannot find main address in %s\n", FILE__, __LINE__, ast->format().c_str());   

                    }
                }
            }
#else
            // Heuristic: main is the target of the 4th call in the text section
            using namespace Dyninst::InstructionAPI;

            unsigned bytesSeen = 0, numCalls = 0;
            InstructionDecoder decoder(p, eReg->getMemSize(), scs.getArch());

            Instruction::Ptr curInsn = decoder.decode();
            while( numCalls < 4 && curInsn && curInsn->isValid() &&
                    bytesSeen < eReg->getMemSize())
            {
                InsnCategory category = curInsn->getCategory();
                if( category == c_CallInsn ) {
                    numCalls++;
                }

                if( numCalls < 4 ) {
                    bytesSeen += curInsn->size();
                    curInsn = decoder.decode();
                }
            }

            if( numCalls != 4 ) {
                logLine("heuristic for finding global constructor function failed\n");
            }else{
                Address callAddress = eReg->getMemOffset() + bytesSeen;
                RegisterAST thePC = RegisterAST(Dyninst::MachRegister::getPC(scs.getArch()));

                Expression::Ptr callTarget = curInsn->getControlFlowTarget();

                if( callTarget.get() ) {
                    callTarget->bind(&thePC, Result(s64, callAddress));
                    Result actualTarget = callTarget->eval();
                    if( actualTarget.defined ) {
                        mainAddress = actualTarget.convert<Address>();
                    }
                }
            }
#endif

            if(!mainAddress || !scs.isValidAddress(mainAddress)) {
                startup_printf("%s[%d]:  invalid main address 0x%lx\n",
                        FILE__, __LINE__, mainAddress);   
            } else {
                startup_printf("%s[%d]:  set main address to 0x%lx\n",
                        FILE__,__LINE__,mainAddress);
            }

            /* Note: creating a symbol for main at the invalid address 
               anyway, because there is guard code for this later in the
               process and otherwise we end up in a weird "this is not an
               a.out" path.

               findMain, like all important utility functions, should have
               a way of gracefully indicating that it has failed. It should
               not return void. NR
               */

            Region *pltsec;
            if((linkedFile->findRegion(pltsec, ".plt")) && pltsec->isOffsetInRegion(mainAddress))
            {
                //logLine( "No static symbol for function main\n" );
                Symbol *newSym = new Symbol("DYNINST_pltMain", 
                        Symbol::ST_FUNCTION, 
                        Symbol::SL_LOCAL,
                        Symbol::SV_INTERNAL,
                        mainAddress,
                        linkedFile->getDefaultModule(),
                        eReg, 
                        0 );
                linkedFile->addSymbol( newSym );
            }
            else
            {
                Symbol *newSym= new Symbol( "main", 
                        Symbol::ST_FUNCTION,
                        Symbol::SL_LOCAL,
                        Symbol::SV_INTERNAL,
                        mainAddress,
                        linkedFile->getDefaultModule(),
                        eReg, 
                        0 );
                linkedFile->addSymbol(newSym);		
            }
        }
        if( !foundStart )
        {
            Symbol *startSym = new Symbol( "_start",
                    Symbol::ST_FUNCTION,
                    Symbol::SL_LOCAL,
                    Symbol::SV_INTERNAL,
                    eReg->getMemOffset(),
                    linkedFile->getDefaultModule(),
                    eReg,
                    0 );
            //cout << "sim for start!" << endl;

            linkedFile->addSymbol(startSym);		
        }
        if( !foundFini )
        {
            Region *finisec = NULL;
            if (linkedFile->findRegion(finisec,".fini")) {
                Symbol *finiSym = new Symbol( "_fini",
                        Symbol::ST_FUNCTION,
                        Symbol::SL_LOCAL,
                        Symbol::SV_INTERNAL,
                        finisec->getMemOffset(),
                        linkedFile->getDefaultModule(),
                        finisec, 
                        0 );
                linkedFile->addSymbol(finiSym);	
            }	
        }
    }

    Region *dynamicsec;
    vector < Symbol *>syms;
    if(linkedFile->findRegion(dynamicsec, ".dynamic")==true)
    {
        if(linkedFile->findSymbol(syms,
                    "_DYNAMIC",
                    Symbol::ST_UNKNOWN,
                    SymtabAPI::mangledName)==false)
        {
            Symbol *newSym = new Symbol( "_DYNAMIC", 
                    Symbol::ST_OBJECT, 
                    Symbol::SL_LOCAL,
                    Symbol::SV_INTERNAL,
                    dynamicsec->getMemOffset(), 
                    linkedFile->getDefaultModule(),
                    dynamicsec, 
                    0 );
            linkedFile->addSymbol(newSym);
        }
    }

#elif defined(i386_unknown_nt4_0)

    if(linkedFile->isExec()) {
        vector <Symbol *>syms;
        vector<SymtabAPI::Function *> funcs;
        Address eAddr = linkedFile->getEntryOffset();
        Region *eReg = linkedFile->findEnclosingRegion(eAddr);

        bool found_main = false;
        for (unsigned i=0; i<NUMBER_OF_MAIN_POSSIBILITIES; i++) {
            if(linkedFile->findFunctionsByName(funcs, main_function_names[i])) {
                found_main = true;
                break;
            }
        }
        if (found_main) {
            if(!linkedFile->findSymbol(syms,"start",Symbol::ST_UNKNOWN, SymtabAPI::mangledName)) {
                //use 'start' for mainCRTStartup.
                Symbol *startSym = new Symbol( "start", 
                        Symbol::ST_FUNCTION,
                        Symbol::SL_GLOBAL, 
                        Symbol::SV_DEFAULT, 
                        eAddr ,
                        linkedFile->getDefaultModule(),
                        eReg,
                        UINT_MAX );
                linkedFile->addSymbol(startSym);
            }
            syms.clear();
        } 
        else {
            // add entry point as main given that nothing else was found
            startup_printf("[%s:%u] - findmain could not find symbol "
                    "for main, using binary entry point %x\n",
                    __FILE__, __LINE__, eAddr);
            linkedFile->addSymbol(new Symbol("main",
                        Symbol::ST_FUNCTION, 
                        Symbol::SL_GLOBAL, 
                        Symbol::SV_DEFAULT,
                        eAddr,
                        linkedFile->getDefaultModule(),
                        eReg));
        }
    }
#endif

    return 0; /* Success */
}

/*
 * Check if image is libdyninstRT
 */
bool image::determineImageType()
{
#if defined(TIMED_PARSE)
  struct timeval starttime;
  gettimeofday(&starttime, NULL);
#endif

  vector <SymtabAPI::Function *>funcs;

  // Checking for libdyninstRT (DYNINSTinit())
  if (linkedFile->findFunctionsByName(funcs, "DYNINSTinit") ||
      linkedFile->findFunctionsByName(funcs, "_DYNINSTinit"))
      is_libdyninstRT = true;
  else
      is_libdyninstRT = false;
   
#if defined(TIMED_PARSE)
  struct timeval endtime;
  gettimeofday(&endtime, NULL);
  unsigned long lstarttime = starttime.tv_sec * 1000 * 1000 + starttime.tv_usec;
  unsigned long lendtime = endtime.tv_sec * 1000 * 1000 + endtime.tv_usec;
  unsigned long difftime = lendtime - lstarttime;
  double dursecs = difftime/(1000 );
  cout << __FILE__ << ":" << __LINE__ <<": determineImageType took "<<dursecs <<" msecs" << endl;
#endif
  return true;
}

bool image::getInferiorHeaps(vector<pair<string,Address> > &codeHeaps,
                             vector<pair<string,Address> > &dataHeaps) {
    if ((codeHeaps_.size() == 0) &&
        (dataHeaps_.size() == 0)) return false;

    for (unsigned i = 0; i < codeHeaps_.size(); i++) {
        codeHeaps.push_back(codeHeaps_[i]);
    }

    for (unsigned i = 0; i < dataHeaps_.size(); i++) {
        dataHeaps.push_back(dataHeaps_[i]);
    }
    return true;
}

bool image::addSymtabVariables()
{
   /* Eventually we'll have to do this on all platforms (because we'll retrieve
    * the type information here).
    */
   
#if defined(TIMED_PARSE)
   struct timeval starttime;
   gettimeofday(&starttime, NULL);
#endif

   std::string mangledName; 

   vector<Variable *> allVars;

   linkedFile->getAllVariables(allVars); 

   for (vector<Variable *>::iterator varIter = allVars.begin();
        varIter != allVars.end(); 
        varIter++) {
       Variable *symVar = *varIter;

       parsing_printf("New variable, mangled %s, module %s...\n",
		      symVar->getFirstSymbol()->getMangledName().c_str(),
                      symVar->getFirstSymbol()->getModule()->fileName().c_str());
       pdmodule *use = getOrCreateModule(symVar->getModule());

       assert(use);
       image_variable *var = new image_variable(symVar, use);
       if (!var->svar()->addAnnotation(var, ImageVariableUpPtrAnno)) {
           fprintf(stderr, "%s[%d]: failed to add annotation here\n", FILE__, __LINE__);
           return false;
       }

       // If this is a Dyninst dynamic heap placeholder, add it to the
       // list of inferior heaps...
       string compString = "DYNINSTstaticHeap";
       if (!var->symTabName().compare(0, compString.size(), compString)) {
           dataHeaps_.push_back(pair<string,Address>(var->symTabName(), var->getOffset()));
       }

       exportedVariables.push_back(var);
       everyUniqueVariable.push_back(var);
       varsByAddr[var->getOffset()] = var;
   }

#if defined(TIMED_PARSE)
   struct timeval endtime;
   gettimeofday(&endtime, NULL);
   unsigned long lstarttime = starttime.tv_sec * 1000 * 1000 + starttime.tv_usec;
   unsigned long lendtime = endtime.tv_sec * 1000 * 1000 + endtime.tv_usec;
   unsigned long difftime = lendtime - lstarttime;
   double dursecs = difftime/(1000 );
   cout << __FILE__ << ":" << __LINE__ <<": addSymtabVariables took "<<dursecs <<" msecs" << endl;
#endif

   return true;
}

pdmodule *image::findModule(const string &name, bool wildcard)
{
   pdmodule *found = NULL;
   //cerr << "image::findModule " << name << " , " << find_if_excluded
   //     << " called" << endl;

   if (!wildcard) {
      if (modsByFileName.find(name) != modsByFileName.end()) {
         //cerr << " (image::findModule) found module in modsByFileName" << endl;
         found = modsByFileName[name];
      }
   }
   else {
      //  if we want a substring, have to iterate over all module names
      //  this is ok b/c there are not usually more than a handful or so
      //
      dyn_hash_map <string, pdmodule *>::iterator mi;
      string str; pdmodule *mod;
      std::string pds = name.c_str();

      for(mi = modsByFileName.begin(); mi != modsByFileName.end() ; mi++)
      {
         str = mi->first;
         mod = mi->second;
         if (wildcardEquiv(pds, mod->fileName())) {
            found = mod; 
            break;
         }
      }
   }

   //cerr << " (image::findModule) did not find module, returning NULL" << endl;
   if (found) {
      // Just-in-time...
      //if (parseState_ == symtab)
      //analyzeImage();
      return found;
   }

   return NULL;
}

const CodeObject::funclist &
image::getAllFunctions()
{
    analyzeIfNeeded();
    return codeObject()->funcs();
}

const std::vector<image_variable*> &image::getAllVariables()
{
    analyzeIfNeeded();
    return everyUniqueVariable;
}

const std::vector<image_variable*> &image::getExportedVariables() const { return exportedVariables; }

const std::vector<image_variable*> &image::getCreatedVariables()
{
  analyzeIfNeeded();
  return createdVariables;
}

bool image::getModules(vector<pdmodule *> &mods) 
{
    bool ret = false;
   std::vector<pdmodule *> toReturn;
    for (map<Module *, pdmodule *>::const_iterator iter = mods_.begin();
         iter != mods_.end(); iter++) {
        ret = true;
        mods.push_back(iter->second);
    }
    return ret;
}

// identify module name from symbol address (binary search)
// based on module tags found in file format (ELF/COFF)
void image::findModByAddr (const Symbol *lookUp, vector<Symbol *> &mods,
			   string &modName, Address &modAddr, 
			   const string &defName)
{
  if (mods.size() == 0) {
    modAddr = 0;
    modName = defName;
    return;
  }

  Address symAddr = lookUp->getOffset();
  int index;
  int start = 0;
  int end = mods.size() - 1;
  int last = end;
  bool found = false;
  while ((start <= end) && !found) {
    index = (start+end)/2;
    if ((index == last) ||
	((mods[index]->getOffset() <= symAddr) && 
	 (mods[index+1]->getOffset() > symAddr))) {
      modName = mods[index]->getMangledName().c_str();
      modAddr = mods[index]->getOffset();      
      found = true;
    } else if (symAddr < mods[index]->getOffset()) {
      end = index - 1;
    } else {
      start = index + 1;
    }
  }
  if (!found) { 
    // must be (start > end)
    modAddr = 0;
    modName = defName;
  }
}

unsigned int int_addrHash(const Address& addr) {
  return (unsigned int)addr;
}

/*
 * load an executable:
 *   1.) parse symbol table and identify routines.
 *   2.) scan executable to identify inst points.
 *
 *  offset is normally zero except on CM-5 where we have two images in one
 *    file.  The offset passed to parseImage is the logical offset (0/1), not
 *    the physical point in the file.  This makes it faster to "parse" multiple
 *    copies of the same image since we don't have to stat and read to find the
 *    physical offset. 
 */

image *image::parseImage(fileDescriptor &desc, 
                         BPatch_hybridMode mode, 
                         bool parseGaps)
{
  /*
   * Check to see if we have parsed this image before. We will
   * consider it a match if the filename matches (Our code is now able
   * to cache the parsing results even if a library is loaded at a
   * different address for the second time).
   */
  unsigned numImages = allImages.size();
  
  for (unsigned u=0; u<numImages; u++) {
      if (desc.isSameFile(allImages[u]->desc())) {
         if (allImages[u]->getObject()->canBeShared()) {
            // We reference count...
            startup_printf("%s[%d]: returning pre-parsed image\n", FILE__, __LINE__);
            return allImages[u]->clone();
         }
      }
  }

  stats_parse.startTimer(PARSE_SYMTAB_TIMER);

  /*
   * load the symbol table. (This is the a.out format specific routine).
   */
  
  bool err=false;

#if defined(TIMED_PARSE)
  struct timeval starttime;
  gettimeofday(&starttime, NULL);
#endif

  startup_printf("%s[%d]:  about to create image\n", FILE__, __LINE__);
  image *ret = new image(desc, err, mode, parseGaps); 
  startup_printf("%s[%d]:  created image\n", FILE__, __LINE__);

  if (ret->isSharedObject()) 
      startup_printf("%s[%d]: processing shared object\n", FILE__, __LINE__);
  else  
      startup_printf("%s[%d]: processing executable object\n", FILE__, __LINE__);
      

#if defined(TIMED_PARSE)
  struct timeval endtime;
  gettimeofday(&endtime, NULL);
  unsigned long lstarttime = starttime.tv_sec * 1000 * 1000 + starttime.tv_usec;
  unsigned long lendtime = endtime.tv_sec * 1000 * 1000 + endtime.tv_usec;
  unsigned long difftime = lendtime - lstarttime;
  double dursecs = difftime/(1000 );
  cout << __FILE__ << ":" << __LINE__ <<": parsing image "<< desc.file().c_str() <<" took "<<dursecs <<" msecs" << endl;
#endif

  if (err || !ret) {
     if (ret) {
         startup_printf("%s[%d]: error in processing, deleting image and returning\n",
                        FILE__, __LINE__);
         delete ret;
     }
     else {
        fprintf(stderr, "Failed to allocate memory for parsing %s!\n", 
                desc.file().c_str());
     }
     stats_parse.stopTimer(PARSE_SYMTAB_TIMER);
     return NULL;
  }

  allImages.push_back(ret);

  // start tracking new blocks after initial parse
  if ( BPatch_exploratoryMode == mode ||
       BPatch_defensiveMode == mode ) 
  {
      ret->trackNewBlocks_ = true;
  }

  // define all modules.

  statusLine("ready"); // this shouldn't be here, right? (cuz we're not done, right?)
  stats_parse.stopTimer(PARSE_SYMTAB_TIMER);

  return ret;
}

/*
 * Remove a parsed executable from the global list. Used if the old handle
 * is no longer valid.
 */
void image::removeImage(image *img)
{

  // Here's a question... do we want to actually delete images?
  // Pro: free up memory. Con: we'd just have to parse them again...
  // I guess the question is "how often do we serially open files".
  /* int refCount = */ img->destroy();
  

  /*
    // We're not deleting when the refcount hits 0, so we may as well
    // keep the vector. It's a time/memory problem. 
  if (refCount == 0) {
    std::vector<image*> newImages;
    // It's gone... remove from image vector
    for (unsigned i = 0; i < allImages.size(); i++) {
      if (allImages[i] != img)
	newImages.push_back(allImages[i]);
    }
    allImages = newImages;
  }
  */
}

int image::destroy() {
    refCount--;
    if (refCount == 0) {
        if (linkedFile->isExec()) {
            // a.out... destroy it
            //delete this;
            return 0;
        }
    }
    if (refCount < 0)
        assert(0 && "NEGATIVE REFERENCE COUNT FOR IMAGE!");
    return refCount; 
}

void image::analyzeIfNeeded() {
  if (parseState_ == symtab) {
      parsing_printf("ANALYZING IMAGE %s\n",
              file().c_str());
      analyzeImage();
	  // For defensive mode: we only care about incremental splitting and block
	  // creation, not ones noted during parsing (as we haven't created the int
	  // layer yet, so no harm no foul)
	  clearNewBlocks();
  }
}

static bool CheckForPowerPreamble(parse_block* entryBlock, Address &tocBase) {
    ParseAPI::Block::Insns insns;
    entryBlock->getInsns(insns);
    if (insns.size() < 2)
      return false;
    // Get the first two instructions
    auto iter = insns.begin();
    InstructionAPI::Instruction i1 = iter->second;
    ++iter;
    InstructionAPI::Instruction i2 = iter->second;

    const uint32_t * buf1 = (const uint32_t*) i1.ptr();
    const uint32_t * buf2 = (const uint32_t*) i2.ptr();

    uint32_t p1 = buf1[0] >> 16;
    uint32_t p2 = buf2[0] >> 16;

    // Check for two types of preamble
    // Preamble 1: used in executables
    // lis r2, IMM       bytes: IMM1 IMM2 40 3c 
    // addi r2, r2, IMM  bytes: IMM1 IMM2 42 38
    if (p1 == 0x3c40 && p2 == 0x3842) {
        tocBase = buf1[0] & 0xffff;
        tocBase <<= 16;
        tocBase += (int16_t)(buf2[0] & 0xffff);
        return true;
    }
    
    // Preamble 2: used in libraries
    // addis r2, r12, IMM   bytes: IMM1 IMM2 4c 3c
    // addi r2, r2,IMM      bytes: IMM1 IMM2 42 38
    if (p1 == 0x3c4c && p2 == 0x3842) {
        tocBase = buf1[0] & 0xffff;
        tocBase <<= 16;
        tocBase += (int16_t)(buf2[0] & 0xffff);
        // Base on the Power ABI V2, r12 should the entry address of the function     
        tocBase += entryBlock->start();
        return true;
    }
    return false;
}



void image::analyzeImage() {
#if defined(TIMED_PARSE)
    struct timeval starttime;
    gettimeofday(&starttime, NULL);
#endif
    stats_parse.startTimer(PARSE_ANALYZE_TIMER);


    assert(parseState_ < analyzed);
    if(parseState_ < symtab){
        fprintf(stderr, "Error: attempt to analyze incomplete image\n");
        goto done;
    }
    parseState_ = analyzing;

    obj_->parse();


#if defined(cap_stripped_binaries)
   {
       vector<CodeRegion *>::const_iterator rit = cs_->regions().begin();
       for( ; rit != cs_->regions().end(); ++rit)
       {
        SymtabCodeRegion * scr = static_cast<SymtabCodeRegion*>(*rit);
        if(parseGaps_ && scr->symRegion()->isText()) {
            obj_->parseGaps(scr);
        }
       } 
   }
#endif // cap_stripped_binaries
   
    
    parseState_ = analyzed;
  done:
    stats_parse.stopTimer(PARSE_ANALYZE_TIMER); 

#if defined(TIMED_PARSE)
    struct timeval endtime;
    gettimeofday(&endtime, NULL);
    unsigned long lstarttime = starttime.tv_sec * 1000*1000+starttime.tv_usec;
    unsigned long lendtime = endtime.tv_sec * 1000*1000+endtime.tv_usec;
    unsigned long difftime = lendtime - lstarttime;
    double dursecs = difftime/(1000 );
    cout << __FILE__ << ":" << __LINE__ <<": analyzeImage of " << name_ << " took "<<dursecs <<" msecs" << endl;
#endif
}

// Constructor for the image object. The fileDescriptor simply
// wraps (in the normal case) the object name and a relocation
// address (0 for a.out file). On the following platforms, we
// are handling a special case:


image::image(fileDescriptor &desc, 
             bool &err, 
             BPatch_hybridMode mode, 
             bool parseGaps) :
   desc_(desc),
   imageOffset_(0),
   imageLen_(0),
   dataOffset_(0),
   dataLen_(0),
   is_libdyninstRT(false),
   main_call_addr_(0),
   linkedFile(NULL),
#if defined(os_linux) || defined(os_freebsd)
   archive(NULL),
#endif
   obj_(NULL),
   cs_(NULL),
   filt(NULL),
   img_fact_(NULL),
   parse_cb_(NULL),
   cb_arg0_(NULL),
   nextBlockID_(0),
   pltFuncs(NULL),
   trackNewBlocks_(false),
   refCount(1),
   parseState_(unparsed),
   parseGaps_(parseGaps),
   mode_(mode),
   arch(Dyninst::Arch_none)
{
#if defined(os_linux) || defined(os_freebsd)
   string file = desc_.file().c_str();
   if( desc_.member().empty() ) {
       startup_printf("%s[%d]:  opening file %s\n", FILE__, __LINE__, file.c_str());
       if( !SymtabAPI::Symtab::openFile(linkedFile, file, (BPatch_defensiveMode == mode ? Symtab::Defensive : Symtab::NotDefensive)) ) {
           err = true;
           return;
       }
   }else{
       startup_printf("%s[%d]: opening archive member: %s(%s)\n", FILE__, __LINE__, file.c_str(),
               desc_.member().c_str());

       if( SymtabAPI::Archive::openArchive(archive, file) ) {
           if( !archive->getMember(linkedFile, const_cast<std::string&>(desc_.member())) ) {
               err = true;
               return;
           }
       }else{
           err = true;
           return;
       }
   }
#else
	std::string file = desc_.file();
   startup_printf("%s[%d]: opening file %s\n", FILE__, __LINE__, file.c_str());
   Symtab::def_t symMode = (BPatch_defensiveMode == mode) ? 
       Symtab::Defensive : Symtab::NotDefensive;
   if(desc.rawPtr()) {
       linkedFile = new Symtab((unsigned char*)desc.rawPtr(), 
                                          desc.length(), 
                                          desc.file(), 
                                          symMode,
                                          err);
   } 
   else if(!Symtab::openFile(linkedFile, file, symMode)) 
   {
      err = true;
      return;
   }
#endif

   err = false;

   name_ = extract_pathname_tail(string(desc.file().c_str()));

   //   fprintf(stderr,"img name %s\n",name_.c_str());
   pathname_ = desc.file().c_str();

   // initialize (data members) codeOffset_, dataOffset_,
   //  codeLen_, dataLen_.

   imageOffset_ = linkedFile->imageOffset();
   dataOffset_ = linkedFile->dataOffset();

   imageLen_ = linkedFile->imageLength();
   dataLen_ = linkedFile->dataLength();

   /*
    * When working with ELF .o's, it is a possibility that the region
    * which is referred to by imageOffset(), imageLength() could have a zero
    * length. This is okay for .o's
    */

   // if unable to parse object file (somehow??), try to
   //  notify user/calling process + return....    
   if (!imageLen_ && 
       linkedFile->getObjectType() != SymtabAPI::obj_RelocatableFile)
    {
      string msg = string("Parsing problem with executable file: ") + desc.file();
      statusLine(msg.c_str());
      msg += "\n";
      logLine(msg.c_str());
      err = true;
      return;
   }

   //Now add Main and Dynamic Symbols if they are not present
   startup_printf("%s[%d]:  before findMain\n", FILE__, __LINE__);
   if(findMain())
   {
        startup_printf("%s[%d]: ERROR: findMain analysis has failed!\n",
                FILE__, __LINE__);
   } else {
        startup_printf("%s[%d]: findMain analysis succeeded.\n",
                FILE__, __LINE__);
   }

   // Initialize ParseAPI 
   filt = NULL;

   /** Optionally eliminate some hints in which Dyninst is not
       interested **/
   struct filt_heap : SymtabCodeSource::hint_filt {
        bool operator()(SymtabAPI::Function * f) {
            return f && f->getModule() && f->getModule()->fileName() == "DYNINSTheap";
        }
    } nuke_heap;
    filt = &nuke_heap;

   bool parseInAllLoadableRegions = (BPatch_normalMode != mode_);
   cs_ = new SymtabCodeSource(linkedFile,filt,parseInAllLoadableRegions);

   // Continue ParseAPI init
//   fprintf(stderr, "#### create CodeObject for %s\n", desc.file().c_str());
   img_fact_ = new DynCFGFactory(this);
   parse_cb_ = new DynParseCallback(this);
   obj_ = new CodeObject(cs_,img_fact_,parse_cb_,BPatch_defensiveMode == mode);

     if (obj_->cs()->getArch() == Arch_ppc64) {
        // The PowerPC new ABI typically generate two entries per function.
        // Need special hanlding for them
        std::map<uint64_t, parse_func *> _findPower8Overlaps;
        for (auto fit = obj_->funcs().begin(); fit != obj_->funcs().end(); ++fit) {
            parse_func* funct = static_cast<parse_func*>(*fit);
            _findPower8Overlaps[funct->addr()] = funct;
        }
        for (auto fit = obj_->funcs().begin(); fit != obj_->funcs().end(); ++fit) {
            parse_func* funct = static_cast<parse_func*>(*fit);
            Address tocBase = 0;
            if(CheckForPowerPreamble(static_cast<parse_block*>(funct->entry()), tocBase)){
                funct->setPowerTOCBaseAddress(tocBase);
                funct->setContainsPowerPreamble(true);
                auto iter = _findPower8Overlaps.find(funct->addr() + 0x8);
                if (iter != _findPower8Overlaps.end()) {
                    funct->setNoPowerPreambleFunc(iter->second);
                } 
            }
        }
    }

   string msg;
   // give user some feedback....
   msg = string("Parsing object file: ") + desc.file();

   statusLine(msg.c_str());


   // Check if image is libdyninstRT
   startup_printf("%s[%d]:  before determineImageType\n", FILE__, __LINE__);
   determineImageType();
   if (isDyninstRTLib()) { // don't parse gaps in the runtime library
       parseGaps_ = false;
   }
            
   // And symtab variables
   addSymtabVariables();

   parseState_ = symtab;
}

image::~image() 
{
    unsigned i;

    for (map<Module *, pdmodule *>::iterator iter = mods_.begin();
         iter != mods_.end(); iter++) {
        delete (iter->second);
    }

    for (i = 0; i < everyUniqueVariable.size(); i++) {
        delete everyUniqueVariable[i];
    }
    everyUniqueVariable.clear();
    createdVariables.clear();
    exportedVariables.clear();

   
    for (i = 0; i < parallelRegions.size(); i++)
      delete parallelRegions[i];
    parallelRegions.clear();

    // Finally, remove us from the image list.
    allImages.erase(
        std::remove_if(allImages.begin(), allImages.end(),
            [this](image *img){ return this == img; }
        )
    );

    if (pltFuncs) {
       delete pltFuncs;
       pltFuncs = NULL;
    }

    if(obj_) delete obj_;
    if(cs_) delete cs_;
    if(img_fact_) delete img_fact_;
    if(parse_cb_) delete parse_cb_;

    if (linkedFile) { SymtabAPI::Symtab::closeSymtab(linkedFile); }
}

bool pdmodule::findFunction( const std::string &name, std::vector<parse_func *> &found ) {
    if (findFunctionByMangled(name, found))
        return true;
    return findFunctionByPretty(name, found);
}

bool pdmodule::findFunctionByMangled( const std::string &name,
                                      std::vector<parse_func *> &found)
{
    // For efficiency sake, we grab the image vector and strip out the
    // functions we want.
    // We could also keep them all in modules and ditch the image-wide search; 
    // the problem is that BPatch goes by module and internal goes by image. 
    unsigned orig_size = found.size();
    
    const std::vector<parse_func *> *obj_funcs = imExec()->findFuncVectorByMangled(name.c_str());
    if (!obj_funcs) {
        return false;
    }
    for (unsigned i = 0; i < obj_funcs->size(); i++) {
        if ((*obj_funcs)[i]->pdmod() == this)
            found.push_back((*obj_funcs)[i]);
    }
    if (found.size() > orig_size) {
        //exec()->analyzeIfNeeded();
        return true;
    }
    
    return false;
}


bool pdmodule::findFunctionByPretty( const std::string &name,
                                     std::vector<parse_func *> &found)
{
    // For efficiency sake, we grab the image vector and strip out the
    // functions we want.
    // We could also keep them all in modules and ditch the image-wide search; 
    // the problem is that BPatch goes by module and internal goes by image. 
    unsigned orig_size = found.size();
    
    const std::vector<parse_func *> *obj_funcs = imExec()->findFuncVectorByPretty(name);
    if (!obj_funcs) {
        return false;
    }
    for (unsigned i = 0; i < obj_funcs->size(); i++) {
        if ((*obj_funcs)[i]->pdmod() == this)
            found.push_back((*obj_funcs)[i]);
    }
    if (found.size() > orig_size) {
        //exec()->analyzeIfNeeded();
        return true;
    }
    
    return false;
}

void pdmodule::dumpMangled(std::string &prefix) const
{
  cerr << fileName() << "::dumpMangled("<< prefix << "): " << endl;

  const CodeObject::funclist & allFuncs = imExec()->getAllFunctions();
  CodeObject::funclist::const_iterator fit = allFuncs.begin();
  for( ; fit != allFuncs.end(); ++fit) {
      parse_func * pdf = (parse_func*)*fit;
      if (pdf->pdmod() != this) continue;

      if( ! strncmp( pdf->symTabName().c_str(), prefix.c_str(), strlen( prefix.c_str() ) ) ) {
          cerr << pdf->symTabName() << " ";
      }
      else {
          // bperr( "%s is not a prefix of %s\n", prefix, pdf->symTabName().c_str() );
      }
  }
  cerr << endl;
}

parse_func *image::addFunction(Address functionEntryAddr, const char *fName)
 {
     set<CodeRegion *> regions;
     CodeRegion * region;
     codeObject()->cs()->findRegions(functionEntryAddr,regions);
     if(regions.empty()) {
        parsing_printf("[%s:%d] refusing to create function in nonexistent "
                       "region at %lx\n",
            FILE__,__LINE__,functionEntryAddr);
        return NULL;
     }
     region = *(regions.begin()); // XXX pick one, throwing up hands. 

     auto *m = linkedFile->getContainingModule(functionEntryAddr);
     
     pdmodule *mod = getOrCreateModule(m);

     // copy or create function name
     char funcName[32];
     if (fName) {
         snprintf( funcName, 32, "%s", fName);
     } else {
         snprintf( funcName, 32, "entry_%lx", functionEntryAddr);	
     }
     Symbol *funcSym = new Symbol(funcName,
                                  Symbol::ST_FUNCTION,
                                  Symbol::SL_GLOBAL, 
                                  Symbol::SV_DEFAULT,
                                  functionEntryAddr, 
                                  mod->mod(),
                                  NULL,
                                  UINT_MAX);
     // create function stub, update datastructures
     if (!linkedFile->addSymbol( funcSym )) {
         return NULL;
     }
     
     // Adding the symbol finds or creates a Function object...
     assert(funcSym->getFunction());

     // Parse, but not recursively
     codeObject()->parse(functionEntryAddr, false); 

     parse_func * func = static_cast<parse_func*>(
            codeObject()->findFuncByEntry(region,functionEntryAddr));

     if(NULL == func) {
        parsing_printf("[%s:%d] failed to create function at %lx\n",
            FILE__,__LINE__,functionEntryAddr);
        return NULL;
     }

     func->getSymtabFunction()->setData((void *) func);

     // If this is a Dyninst dynamic heap placeholder, add it to the
     // list of inferior heaps...
     string compString = "DYNINSTstaticHeap";
     if (!func->symTabName().compare(0, compString.size(), compString)) {
         codeHeaps_.push_back(pair<string, Address>(func->symTabName(), func->getOffset()));
     }

     func->addSymTabName( funcName ); 
     func->addPrettyName( funcName );
     // funcsByEntryAddr[func->getOffset()] = func;
     //createdFunctions.push_back(func);

     return func;
}

const string &pdmodule::fileName() const
{
    return mod_->fileName();
}

SymtabAPI::supportedLanguages 
pdmodule::language() const
{
    return mod_->language();
}

Address pdmodule::addr() const
{
    return mod_->addr();
}

bool pdmodule::isShared() const
{
    return mod_->isShared();
}

Module *pdmodule::mod()
{
    return mod_;
}

pdmodule *image::getOrCreateModule(Module *mod) {
    if (mods_.find(mod) != mods_.end())
        return mods_[mod];

    pdmodule *pdmod = new pdmodule(mod, this);

    mods_[mod] = pdmod;
    modsByFileName[pdmod->fileName()] = pdmod;
    
    return pdmod;
}


/*********************************************************************/
/**** Function lookup (by name or address) routines               ****/
/****                                                             ****/
/**** Overlapping region objects MUST NOT use these routines(+)   ****/
/*********************************************************************/

int
image::findFuncs(const Address offset, set<Function *> & funcs) {
    analyzeIfNeeded();

    set<CodeRegion *> match;
    int cnt = cs_->findRegions(offset,match);
    if(cnt == 0)
        return 0;
    else if(cnt == 1)
        return obj_->findFuncs(*match.begin(),offset,funcs);

    fprintf(stderr,"[%s:%d] image::findFuncs(offset) called on "
            "overlapping-region object\n",
            FILE__,__LINE__);
    assert(0);
    return 0;
}

parse_func *image::findFuncByEntry(const Address &entry) {
    analyzeIfNeeded();

    set<CodeRegion *> match;
    int cnt = cs_->findRegions(entry,match);
    if(cnt == 0)
        return 0;
    else if(cnt == 1)
        return (parse_func*)obj_->findFuncByEntry(*match.begin(),entry);

    fprintf(stderr,"[%s:%d] image::findFuncByEntry(entry) called on "
                   "overlapping-region object\n",
        FILE__,__LINE__);
    assert(0);
    return 0;
}

int 
image::findBlocksByAddr(const Address addr, set<ParseAPI::Block *> & blocks ) 
{
    analyzeIfNeeded();

    set<CodeRegion *> match;
    int cnt = cs_->findRegions(addr,match);
    if(cnt == 0)
        return 0;
    else if(cnt == 1)
        return obj_->findBlocks(*match.begin(),addr,blocks);

    fprintf(stderr,"[%s:%d] image::findBlocks(offset) called on "
            "overlapping-region object\n",
            FILE__,__LINE__);
    assert(0);
    return 0;
}

// Return the vector of functions associated with a pretty (demangled) name
// Very well might be more than one!

const std::vector<parse_func *> *image::findFuncVectorByPretty(const std::string &name) {
    //Have to change here
    std::vector<parse_func *>* res = new std::vector<parse_func *>;
    vector<SymtabAPI::Function *> funcs;
    linkedFile->findFunctionsByName(funcs, name.c_str(), SymtabAPI::prettyName);

    for(unsigned index=0; index < funcs.size(); index++)
    {
        SymtabAPI::Function *symFunc = funcs[index];
        parse_func *imf = static_cast<parse_func *>(symFunc->getData());
        if (imf) {
            res->push_back(imf);
        }
    }		
    if(res->size())	
	return res;	    
    else {
        delete res;
    	return NULL;
    }
}

// Return the vector of functions associated with a mangled name
// Very well might be more than one! -- multiple static functions in different .o files

const std::vector <parse_func *> *image::findFuncVectorByMangled(const std::string &name)
{
    std::vector<parse_func *>* res = new std::vector<parse_func *>;

    vector<SymtabAPI::Function *> funcs;
    linkedFile->findFunctionsByName(funcs, name.c_str(), SymtabAPI::mangledName);

    for(unsigned index=0; index < funcs.size(); index++) {
        SymtabAPI::Function *symFunc = funcs[index];
        parse_func *imf = static_cast<parse_func *>(symFunc->getData());
        
        if (imf) {
            res->push_back(imf);
        }
    }

    if (res->empty()) {
        // Lookup PLT stubs
        auto it = plt_parse_funcs.find(name);
        if (it != plt_parse_funcs.end()) {
            res->push_back(it->second);
        }
    }

    if(res->size()) 
	return res;	    
    else {
        delete res;
    	return NULL;
    }   
}

const std::vector <image_variable *> *image::findVarVectorByPretty(const std::string &name)
{
    std::vector<image_variable *>* res = new std::vector<image_variable *>;

    vector<Variable *> vars;
    linkedFile->findVariablesByName(vars, name.c_str(), SymtabAPI::prettyName);
    
    for (unsigned index=0; index < vars.size(); index++) {
        Variable *symVar = vars[index];
        image_variable *imv = NULL;
        
        if (!symVar->getAnnotation(imv, ImageVariableUpPtrAnno)) {
            fprintf(stderr, "%s[%d]:  failed to getAnnotations here\n", FILE__, __LINE__);
            delete res;
            return NULL;
        }

       if (imv) {
           res->push_back(imv);
       }
    }	    
    if(res->size())	
	return res;	    
    else {
        delete res;
    	return NULL;
    }
}

const std::vector <image_variable *> *image::findVarVectorByMangled(const std::string &name)
{
    //    fprintf(stderr,"findVariableVectorByPretty %s\n",name.c_str());
    std::vector<image_variable *>* res = new std::vector<image_variable *>;

    vector<Variable *> vars;
    linkedFile->findVariablesByName(vars, name.c_str(), SymtabAPI::mangledName);
    
    for (unsigned index=0; index < vars.size(); index++) {
        Variable *symVar = vars[index];
        image_variable *imv = NULL;
        
        if (!symVar->getAnnotation(imv, ImageVariableUpPtrAnno)) {
            fprintf(stderr, "%s[%d]:  failed to getAnnotations here\n", FILE__, __LINE__);
            delete res;
            return NULL;
        }

        if (imv) {
            res->push_back(imv);
        }
    }	    
    if(res->size())	
	return res;	    
    else {
        delete res;
    	return NULL;
    }
  /*
  if (varsByMangled.defines(name)) {
      //analyzeIfNeeded();
      return varsByMangled[name];
  }
  return NULL;*/
}

bool pdmodule::getFunctions(std::vector<parse_func *> &funcs)  {
    unsigned curFuncSize = funcs.size();
    const CodeObject::funclist & allFuncs = imExec()->getAllFunctions();
    
    CodeObject::funclist::const_iterator fit = allFuncs.begin();
    for( ; fit != allFuncs.end(); ++fit) {
        parse_func *f = (parse_func*)*fit;
        if (f->pdmod() == this) 
	{
            funcs.push_back(f);
	}
    }
  
    return (funcs.size() > curFuncSize);
} /* end getFunctions() */

/* Instrumentable-only, by the last version's source. */
bool pdmodule::getVariables(std::vector<image_variable *> &vars)  {
    std::vector<image_variable *> allVars = imExec()->getAllVariables();
    unsigned curVarSize = vars.size();

    for (unsigned i = 0; i < allVars.size(); i++) {
        if (allVars[i]->pdmod() == this)
            vars.push_back(allVars[i]);
    }
  
    return (vars.size() > curVarSize);
} /* end getFunctions() */

/*
void *image::getPtrToDataInText( Address offset ) const {
    if( isData(offset) ) { return NULL; }
    if( ! isCode(offset) ) { return NULL; }
	
    Region *reg = linkedFile->findEnclosingRegion(offset);
    if(reg != NULL) {
        return (void*) ((Address)reg->getPtrToRawData() + offset 
                        - reg->getMemOffset());
    }
    return NULL;
} *//* end getPtrToDataInText() */
    
bool image::getExecCodeRanges(std::vector<std::pair<Address, Address> > &ranges)
{
   std::vector<Region *> regions;
   bool result = linkedFile->getCodeRegions(regions);
   if (!result)
      return false;
   Address cur_start = 0, cur_end = (Address)(-1);
   bool found_something = false;
   fprintf(stderr, "\n");
   for (std::vector<Region *>::iterator i = regions.begin(); i != regions.end(); i++)
   {
      Region *r = *i;
      if (!r->isStandardCode() && !codeObject()->defensiveMode())
         continue;
      if (!found_something) {
         cur_start = r->getDiskOffset();
         cur_end = cur_start + r->getDiskSize();
         found_something = true;
         continue;
      }

      if (r->getDiskOffset() <= cur_end) {
         cur_end = r->getDiskOffset() + r->getDiskSize();
      }
      else {
         ranges.push_back(std::pair<Address, Address>(cur_start, cur_end));
         cur_start = r->getDiskOffset();
         cur_end = cur_start + r->getDiskSize();
      }
   }
   if (!found_something) {
      return false;
   }
   ranges.push_back(std::pair<Address, Address>(cur_start, cur_end));
   return true;
}

Symbol *image::symbol_info(const std::string& symbol_name) {
   vector< Symbol *> symbols;
   if(!(linkedFile->findSymbol(symbols,symbol_name.c_str(),Symbol::ST_UNKNOWN, SymtabAPI::anyName))) 
       return NULL;

   return symbols[0];
}

bool image::findSymByPrefix(const std::string &prefix, std::vector<Symbol *> &ret) {
    unsigned start;
    vector <Symbol *>found;	
    std::string reg = prefix+std::string("*");
    if(!linkedFile->findSymbol(found, reg.c_str(), Symbol::ST_UNKNOWN, SymtabAPI::anyName, true))
    	return false;
    for(start=0;start< found.size();start++)
		ret.push_back(found[start]);
	return true;	
}

std::unordered_map<Address, std::string> *image::getPltFuncs()
{
   bool result;
   if (pltFuncs)
      return pltFuncs;


   vector<SymtabAPI::relocationEntry> fbt;
   result = getObject()->getFuncBindingTable(fbt);
   if (!result)
      return NULL;

   pltFuncs = new std::unordered_map<Address, std::string>;
   assert(pltFuncs);
   for(unsigned k = 0; k < fbt.size(); k++) {
      (*pltFuncs)[fbt[k].target_addr()] = fbt[k].name().c_str();
   }
   return pltFuncs;
}

void image::getPltFuncs(std::map<Address, std::string> &out)
{
   out.clear();
   vector<SymtabAPI::relocationEntry> fbt;
   bool result = getObject()->getFuncBindingTable(fbt);
   if (!result)
      return;

   for(unsigned k = 0; k < fbt.size(); k++) {
      out[fbt[k].target_addr()] = fbt[k].name();
   }
}

image_variable* image::createImageVariable(Offset offset, std::string name, int size, pdmodule *mod)
{
    // What to do here?
   auto iter = varsByAddr.find(offset);
   if (iter != varsByAddr.end()) return iter->second;

    Variable *sVar = getObject()->createVariable(name, offset, size, mod->mod());

    image_variable *ret = new image_variable(sVar, mod);

    extern AnnotationClass<image_variable> ImageVariableUpPtrAnno;
    if (!sVar->addAnnotation(ret, ImageVariableUpPtrAnno)) {
        fprintf(stderr, "%s[%d]:  failed to add annotation here\n", FILE__, __LINE__);
    }

    createdVariables.push_back(ret);
    everyUniqueVariable.push_back(ret);
    varsByAddr[offset] = ret;
    return ret;
}


const vector<parse_block*> & image::getNewBlocks() const
{
    return newBlocks_;
}
void image::clearNewBlocks()
{
    newBlocks_.clear();
}

void image::setImageLength(Address newlen)
{
    imageLen_ = newlen; 
}

void image::destroy(ParseAPI::Block *) {}
void image::destroy(ParseAPI::Edge *) {}
void image::destroy(ParseAPI::Function *) {}

void image::insertPLTParseFuncMap(const std::string & name, parse_func* f) {
    plt_parse_funcs[name] = f;
}
