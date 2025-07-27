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

#define BPATCH_FILE

#include "binaryEdit.h"
#include "addressSpace.h"
#include "inst.h"
#include "instPoint.h"
#include "function.h" // func_instance
#include "codeRange.h"

#include "mapped_module.h"

#include "BPatch_libInfo.h"
#include "BPatch.h"
#include "BPatch_addressSpace.h"

#include "BPatch_binaryEdit.h"
#include "BPatch_image.h"
#include "BPatch_thread.h"
#include "BPatch_function.h"
#include "debug.h"

#include "BPatch_private.h"
#include <queue>


#include "ast.h"

#include "sys/stat.h"
#include "mapped_object.h"
#include "Relocation/DynAddrSpace.h"

#if defined(DYNINST_CODEGEN_ARCH_AMDGPU_GFX908)
#include "amdgpu-prologue.h"
#include "amdgpu-epilogue.h"
#include <fstream>
#endif

#include "boost/filesystem.hpp"
using Dyninst::PatchAPI::DynAddrSpacePtr;
using Dyninst::PatchAPI::DynAddrSpace;

/*
 * BPatch_binaryEdit::BPatch_binaryEdit
 *
 * Creates a new BinaryEdit and associates it with the BPatch_binaryEdit
 * being created. Additionally, if specified, the dependencies of the
 * original BinaryEdit are opened and associated with the BPatch_binaryEdit
 *
 * path		     Pathname of the executable
 * openDependencies  if true, the dependencies of the original BinaryEdit are
 *                   also opened
 */
BPatch_binaryEdit::BPatch_binaryEdit(const char *path, bool openDependencies) :
   BPatch_addressSpace(),
   creation_error(false)
{
  pendingInsertions = new BPatch_Vector<batchInsertionRecord *>;

  startup_printf("[%s:%d] - Opening original file %s\n",
                 FILE__, __LINE__, path);
  origBinEdit = BinaryEdit::openFile(path ? path : "");

  if (!origBinEdit){
     startup_printf("[%s:%d] - Creation error opening %s\n",
                    FILE__, __LINE__, path);
     creation_error = true;
     return;
  }
  llBinEdits[path] = origBinEdit;

  if(openDependencies) {
    origBinEdit->getAllDependencies(llBinEdits);
  }
  std::map<std::string, BinaryEdit*>::iterator i, j;

  origBinEdit->getDyninstRTLibName();
  std::string rt_name = origBinEdit->dyninstRT_name;

  // Load the RT library and create the collection of BinaryEdits that represent it
  std::map<std::string, BinaryEdit *> rtlibs;
  origBinEdit->openResolvedLibraryName(rt_name, rtlibs);
  std::map<std::string, BinaryEdit *>::iterator rtlibs_it;
  for(rtlibs_it = rtlibs.begin(); rtlibs_it != rtlibs.end(); ++rtlibs_it) {
      if( !rtlibs_it->second ) {
          std::string msg("Failed to load Dyninst runtime library, check the environment variable DYNINSTAPI_RT_LIB");
          showErrorCallback(70, msg.c_str());
          creation_error = true;
          return;
      }
      rtLib.push_back(rtlibs_it->second);
      // Ensure that the correct type of library is loaded
      if(    rtlibs_it->second->getMappedObject()->isSharedLib()
          && origBinEdit->getMappedObject()->isStaticExec() )
      {
          std::string msg = std::string("RT Library is a shared library ") +
              std::string("when it should be a static library");
          showErrorCallback(70, msg.c_str());
          creation_error = true;
          return;
      }else if(     !rtlibs_it->second->getMappedObject()->isSharedLib()
                &&  !origBinEdit->getMappedObject()->isStaticExec() )
      {
          std::string msg = std::string("RT Library is a static library ") +
              std::string("when it should be a shared library");
          showErrorCallback(70, msg.c_str());
          creation_error = true;
          return;
      }
  }

  for(i = llBinEdits.begin(); i != llBinEdits.end(); i++) {
     (*i).second->setupRTLibrary(rtLib);
  }

  for(i = llBinEdits.begin(); i != llBinEdits.end(); i++) {
     BinaryEdit *llBinEdit = (*i).second;
     llBinEdit->registerFunctionCallback(createBPFuncCB);
     llBinEdit->registerInstPointCallback(createBPPointCB);
     llBinEdit->set_up_ptr(this);
     llBinEdit->setupRTLibrary(rtLib);
     llBinEdit->setMultiThreadCapable(isMultiThreadCapable());
     for (j = llBinEdits.begin(); j != llBinEdits.end(); j++) {
        llBinEdit->addSibling((*j).second);
     }
  }

  image = new BPatch_image(this);
}

bool BPatch_binaryEdit::isMultiThreadCapable() const
{
  return origBinEdit->isMultiThreadCapable();
}

BPatch_image * BPatch_binaryEdit::getImage() {
	return image;
}

BPatch_binaryEdit::~BPatch_binaryEdit()
{
   if (image)
      delete image;

   image = NULL;

   if (pendingInsertions) {
     for (unsigned f = 0; f < pendingInsertions->size(); f++) {
       delete (*pendingInsertions)[f];
     }
     delete pendingInsertions;
     pendingInsertions = NULL;
   }

  std::map<std::string, BinaryEdit*>::iterator i = llBinEdits.begin();
  for(; i != llBinEdits.end(); i++) {
     delete (*i).second;
  }
  llBinEdits.clear();
  origBinEdit = NULL;

  assert(BPatch::bpatch != NULL);
}

#if defined(DYNINST_CODEGEN_ARCH_AMDGPU_GFX908)

constexpr int kernelInfoSize = 3;

struct AmdgpuKernelInfo {
  AmdgpuKernelInfo(std::vector<std::string> &words) {
    assert(words.size() == kernelInfoSize);
    kdName = words[0];
    kernargBufferSize = std::stoul(words[1]);
    kernargPtrRegister = std::stoul(words[2]);
  }

  std::string getKernelName() const {
    assert(kdName.length() > 3);
    return kdName.substr(0, kdName.length() - 3);
  }

  std::string kdName;
  unsigned kernargBufferSize;
  unsigned kernargPtrRegister;
};

static void getWords(const std::string &str, std::vector<std::string> &words) {
  std::stringstream ss(str);
  std::string word;
  while (ss >> word) {
    words.push_back(word);
  }
}

static void readKernelInfos(const std::string &filePath,
                            std::vector<AmdgpuKernelInfo> &kernelInfos) {
  std::ifstream infoFile(filePath);
  std::string line;

  assert(infoFile.is_open());

  std::vector<std::string> words;

  while (std::getline(infoFile, line)) {
    getWords(line, words);
    assert(words.size() == kernelInfoSize);

    AmdgpuKernelInfo info(words);
    kernelInfos.push_back(info);
  }
  infoFile.close();
}

static void writeInstrumentedFunctionNames(const std::string &filePath, std::vector<AmdgpuKernelInfo> &kernelInfos) {
  std::ofstream namesFile(filePath);
  assert(namesFile.is_open());

  for (const auto& function : BPatch_addressSpace::instrumentedFunctions) {
    std::string functionName = function->getMangledName();
    for (const auto &kernelInfo : kernelInfos) {
      if (functionName == kernelInfo.getKernelName()) {
        namesFile << function->getMangledName() << ' ' << kernelInfo.kernargBufferSize << std::endl;
      }
    }
  }

  namesFile.close();
}

static void insertPrologueInInstrumentedFunctions(
    std::vector<AmdgpuKernelInfo> &kernelInfos) {
  // Go through information for instrumented kernels (functions) and set base
  // register to kernargPtrRegister and offset to kernargBufferSize. The
  // additional additional argument will be at the end of the kernarg buffer. We
  // set up the prologue to load s[94:95] with address of our additional memory
  // buffer that holds additional variables inserted during instrumentation.
  for (const auto &function : BPatch_addressSpace::instrumentedFunctions) {
    std::vector<BPatch_point *> entryPoints;
    function->getEntryPoints(entryPoints);

    assert(entryPoints.size() == 1);
    BPatch_point *entryPoint = entryPoints[0];

    for (auto &kernelInfo : kernelInfos) {
      if (kernelInfo.getKernelName() == function->getMangledName()) {
        auto prologuePtr = boost::make_shared<AmdgpuPrologueSnippet>(
            94, kernelInfo.kernargPtrRegister, kernelInfo.kernargBufferSize);
        auto prologueNodePtr =
            boost::make_shared<AmdgpuPrologueSnippetNode>(prologuePtr);

        auto addressSpace = entryPoint->getAddressSpace();
        BPatchSnippetHandle *handle =
            addressSpace->insertPrologue(prologueNodePtr, entryPoint);
        assert(handle);
      }
    }
  }
}

static void insertEpilogueInInstrumentedFunctions(
    std::vector<AmdgpuKernelInfo> &kernelInfos) {
  // Go through information for instrumented kernels (functions) and insert a s_dcache_wb instruction at the exit point.
  for (const auto &function : BPatch_addressSpace::instrumentedFunctions) {
    std::vector<BPatch_point *> exitPoints;
    function->getExitPoints(exitPoints);

    for (size_t i = 0; i < exitPoints.size(); ++i) {
      BPatch_point *exitPoint = exitPoints[i];

      for (auto &kernelInfo : kernelInfos) {
        if (kernelInfo.getKernelName() == function->getMangledName()) {
          auto epiloguePtr = boost::make_shared<AmdgpuEpilogueSnippet>();
          auto epilogueNodePtr =
              boost::make_shared<AmdgpuEpilogueSnippetNode>(epiloguePtr);

          auto addressSpace = exitPoint->getAddressSpace();
          BPatchSnippetHandle *handle =
              addressSpace->insertEpilogue(epilogueNodePtr, exitPoint);
          assert(handle);
        }
      }
    }
  }
}

#endif

bool BPatch_binaryEdit::writeFile(const char * outFile)
{
    assert(pendingInsertions);

    // Iterate over our AddressSpaces, triggering relocation
    // in each one.
    std::vector<AddressSpace *> as;
    getAS(as);
    bool ret = true;

#if defined(DYNINST_CODEGEN_ARCH_AMDGPU_GFX908)
    std::vector<AmdgpuKernelInfo> kernelInfos;
    std::string inputFileName = this->getImage()->getProgramFileName();
#endif

    /* PatchAPI stuffs */
    if (as.size() > 0) {
#if defined(DYNINST_CODEGEN_ARCH_AMDGPU_GFX908)
      readKernelInfos(inputFileName + ".info", kernelInfos);
      insertPrologueInInstrumentedFunctions(kernelInfos);
      insertEpilogueInInstrumentedFunctions(kernelInfos);
#endif
      ret = AddressSpace::patch(as[0]);
    }
    /* end of PatchAPI stuffs */


   // Now that we've instrumented we can see if we need to replace the
   // trap handler.
   replaceTrapHandler();

   for(std::map<std::string, BinaryEdit*>::iterator i = llBinEdits.begin();
       i != llBinEdits.end(); i++)
   {
      (*i).second->trapMapping.flush();
   }


   if( !origBinEdit->writeFile(outFile) ) return false;
#if defined(DYNINST_CODEGEN_ARCH_AMDGPU_GFX908)
   writeInstrumentedFunctionNames(inputFileName + ".instrumentedKernelNames", kernelInfos);
#endif

   std::map<std::string, BinaryEdit *>::iterator curBinEdit;
   for (curBinEdit = llBinEdits.begin(); curBinEdit != llBinEdits.end(); curBinEdit++) {
     BinaryEdit *bin = (*curBinEdit).second;
     if (bin == origBinEdit)
       continue;
     if (!bin->isDirty())
       continue;

     std::string newname = bin->getMappedObject()->fileName();
     if( !bin->writeFile(newname) ) return false;
   }
   return ret;
}

processType BPatch_binaryEdit::getType()
{
  return STATIC_EDITOR;
}

void BPatch_binaryEdit::getAS(std::vector<AddressSpace *> &as)
{
   std::map<std::string, BinaryEdit*>::iterator i = llBinEdits.begin();
   as.push_back(origBinEdit);
   for(; i != llBinEdits.end(); i++) {
      if ((*i).second == origBinEdit)
         continue;
      as.push_back((*i).second);
   }
}


/*
 * BPatch_addressSpace::beginInsertionSet
 *
 * Starts a batch insertion set; that is, all calls to insertSnippet until
 * finalizeInsertionSet are delayed.
 *
 */

void BPatch_binaryEdit::beginInsertionSet()
{
    return;
}


bool BPatch_binaryEdit::finalizeInsertionSet(bool /*atomic*/, bool * /*modified*/)
{
    return true;
}

BPatch_object *BPatch_binaryEdit::loadLibrary(const char *libname, bool deps)
{
   boost::filesystem::path p(libname);
   string filename = p.filename().string();
   auto loaded = loadedLibrary.find(filename); 
   if (loaded != loadedLibrary.end()) {
       return loaded->second; 
   }
   std::map<std::string, BinaryEdit*> libs;
   mapped_object *obj = origBinEdit->openResolvedLibraryName(libname, libs);
   if (!obj) return NULL;

   std::map<std::string, BinaryEdit*>::iterator lib_it;
   for(lib_it = libs.begin(); lib_it != libs.end(); ++lib_it) {
      std::pair<std::string, BinaryEdit*> lib = *lib_it;
      
      if(!lib.second)
         return NULL;
      
      llBinEdits.insert(lib);
      /* PatchAPI stuffs */
      mapped_object* plib = lib.second->getAOut();
      assert(plib);
      dynamic_cast<DynAddrSpace*>(origBinEdit->mgr()->as())->loadLibrary(plib);
      lib.second->setMgr(origBinEdit->mgr());
      lib.second->setPatcher(origBinEdit->patcher());
      /* End of PatchAPi stuffs */


    lib.second->registerFunctionCallback(createBPFuncCB);
    lib.second->registerInstPointCallback(createBPPointCB);
    lib.second->set_up_ptr(this);
    lib.second->setupRTLibrary(rtLib);
    lib.second->setMultiThreadCapable(isMultiThreadCapable());

    if (deps)
      if( !lib.second->getAllDependencies(llBinEdits) ) return NULL;

   }
   origBinEdit->addLibraryPrereq(libname);
   BPatch_object * bpatch_obj = getImage()->findOrCreateObject(obj);
   loadedLibrary[filename] = bpatch_obj;
   return bpatch_obj;
}

// Here's the story. We may need to install a trap handler for instrumentation
// to work in the rewritten binary. This doesn't play nicely with trap handlers
// that the binary itself registers. So we're going to replace every call to
// sigaction in the binary with a call to our wrapper. This wrapper:
//   1) Ignores attempts to register a SIGTRAP
//   2) Passes everything else through to sigaction
// It's called "dyn_sigaction".
// This is just a multiplexing function over each child binaryEdit
// object because they're all individual.

bool BPatch_binaryEdit::replaceTrapHandler() {
    // Did we use a trap?

    bool usedATrap = false;

    std::map<std::string, BinaryEdit *>::iterator iter = llBinEdits.begin();
    for (; iter != llBinEdits.end(); iter++) {
        if (iter->second->usedATrap()) {
            usedATrap = true;
            break;
        }
    }

    if (!usedATrap) return true;

    // We used a trap, so go through and set up the replacement instrumentation.
    // However, don't let this be the first piece of instrumentation put into
    // a library.

    bool success = true;
    iter = llBinEdits.begin();
    for (; iter != llBinEdits.end(); iter++) {
        BinaryEdit *binEd = iter->second;

        // Did we instrument this already?
        if (!binEd->isDirty()) {
            continue;
        }

        // Okay, replace trap handler
        if (!binEd->replaceTrapHandler()) {
            success = false;
        }
    }
    return success;
}


