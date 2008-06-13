/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */

#define BPATCH_FILE

#include "process.h"
#include "binaryEdit.h"
#include "addressSpace.h"
#include "EventHandler.h"
#include "mailbox.h"
#include "signalgenerator.h"
#include "inst.h"
#include "instP.h"
#include "instPoint.h"
#include "function.h" // int_function
#include "codeRange.h"
#include "dyn_thread.h"
#include "miniTramp.h"

#include "mapped_module.h"

#include "BPatch_libInfo.h"
#include "BPatch.h"
#include "BPatch_addressSpace.h"

#include "BPatch_binaryEdit.h"
#include "BPatch_image.h"
#include "BPatch_thread.h"
#include "BPatch_function.h"
#include "callbacks.h"

#include "BPatch_private.h"



#include "ast.h"

#include "sys/stat.h"

/*
 * BPatch_binaryEdit::BPatch_binaryEdit
 *
 * Starts a new process and associates it with the BPatch_binaryEdit being
 * constructed.  The new process is placed into a stopped state before
 * executing any code.
 *
 * path		Pathname of the executable to start.
 * argv		A list of pointers to character strings which are the
 *              arguments for the new process, terminated by a NULL pointer.
 * envp		A list of pointers to character strings which are the
 *              environment variables for the new process, terminated by a
 *              NULL pointer.  If NULL, the default environment will be used.
 */
BPatch_binaryEdit::BPatch_binaryEdit(const char *path, bool openDependencies) :
   BPatch_addressSpace(),
   llBinEdit(NULL),
   creation_error(false)
{
  func_map = new BPatch_funcMap();
  instp_map = new BPatch_instpMap();
  pendingInsertions = new BPatch_Vector<batchInsertionRecord *>;
 
  pdvector<std::string> argv_vec;
  pdvector<std::string> envp_vec;
  
  std::string directoryName = "";
  
  llBinEdit = BinaryEdit::openFile(std::string(path));
  if (!llBinEdit){
     creation_error = true;
     return;
  }
  
  startup_cerr << "Registering function callback..." << endl;
  llBinEdit->registerFunctionCallback(createBPFuncCB);
  
  startup_cerr << "Registering instPoint callback..." << endl;
  llBinEdit->registerInstPointCallback(createBPPointCB);
  
  llBinEdit->set_up_ptr(this);

  image = new BPatch_image(this);

  if (openDependencies && !openAllDependencies(path)) {
      creation_error = true;
  }
}

BPatch_image * BPatch_binaryEdit::getImageInt() {
	return image;
}

void BPatch_binaryEdit::BPatch_binaryEdit_dtor()
{
   if (image) 
      delete image;
   
   image = NULL;

   if (func_map)
      delete func_map;
   func_map = NULL;
   if (instp_map)
      delete instp_map;
   instp_map = NULL;

   if (pendingInsertions) {
     for (unsigned f = 0; f < pendingInsertions->size(); f++) {
       delete (*pendingInsertions)[f];
     }
     delete pendingInsertions;
     pendingInsertions = NULL;
   }

   delete llBinEdit;
   llBinEdit = NULL;

   // TODO: is this cleanup necessary?
   BPatch_image *img;
   while (depImages.size() > 0) {
      img = depImages[0];
      depImages.erase(depImages.begin());
      delete img;
   }

   assert(BPatch::bpatch != NULL);
}

bool BPatch_binaryEdit::writeFileInt(const char * outFile)
{
    assert(pendingInsertions);

    // This should be a parameter...
    bool atomic = false;
   
    
    // Two loops: first addInst, then generate/install/link
    pdvector<miniTramp *> workDone;
    bool err = false;

    for (unsigned i = 0; i < pendingInsertions->size(); i++) {
        batchInsertionRecord *&bir = (*pendingInsertions)[i];
        assert(bir);

        // Don't handle thread inst yet...
        assert(!bir->thread_);

        if (!bir->points_.size()) {
          fprintf(stderr, "%s[%d]:  WARN:  zero points for insertion record\n", FILE__, __LINE__);
          fprintf(stderr, "%s[%d]:  failing to addInst\n", FILE__, __LINE__);
        }

        for (unsigned j = 0; j < bir->points_.size(); j++) {
            BPatch_point *bppoint = bir->points_[j];
            instPoint *point = bppoint->point;
            callWhen when = bir->when_[j];
            
            miniTramp *mini = point->addInst(*(bir->snip.ast_wrapper),
                                             when,
                                             bir->order_,
                                             bir->trampRecursive_,
                                             false);
            if (mini) {
                workDone.push_back(mini);
                // Add to snippet handle
                bir->handle_->addMiniTramp(mini);
            }
            else {
                err = true;
                if (atomic) break;
            }
        }
        if (atomic && err)
            break;
    }
    
   if (atomic && err) goto cleanup;

   // All generation first. Actually, all generation per function...
   // but this is close enough.
   for (unsigned int i = 0; i < pendingInsertions->size(); i++) {
       batchInsertionRecord *&bir = (*pendingInsertions)[i];
       assert(bir);
        if (!bir->points_.size()) {
          fprintf(stderr, "%s[%d]:  WARN:  zero points for insertion record\n", FILE__, __LINE__);
          fprintf(stderr, "%s[%d]:  failing to generateInst\n", FILE__, __LINE__);
        }
       for (unsigned j = 0; j < bir->points_.size(); j++) {
           BPatch_point *bppoint = bir->points_[j];
           instPoint *point = bppoint->point;

           point->optimizeBaseTramps(bir->when_[j]);
           if (!point->generateInst()) {
               err = true;
               if (atomic && err) break;
           }
       }
       if (atomic && err) break;
   }

   if (atomic && err) goto cleanup;

   //  next, all installing 
   for (unsigned int i = 0; i < pendingInsertions->size(); i++) {
       batchInsertionRecord *&bir = (*pendingInsertions)[i];
       assert(bir);
        if (!bir->points_.size()) {
          fprintf(stderr, "%s[%d]:  WARN:  zero points for insertion record\n", FILE__, __LINE__);
          fprintf(stderr, "%s[%d]:  failing to installInst\n", FILE__, __LINE__);
        }
       for (unsigned j = 0; j < bir->points_.size(); j++) {
           BPatch_point *bppoint = bir->points_[j];
           instPoint *point = bppoint->point;
             
           if (!point->installInst()) {
              err = true;
           }

           if (atomic && err) break;
       }
       if (atomic && err) break;
   }

   if (atomic && err) goto cleanup;

   //  finally, do all linking 
   for (unsigned int i = 0; i < pendingInsertions->size(); i++) {
       batchInsertionRecord *&bir = (*pendingInsertions)[i];
       assert(bir);
        if (!bir->points_.size()) {
          fprintf(stderr, "%s[%d]:  WARN:  zero points for insertion record\n", FILE__, __LINE__);
          fprintf(stderr, "%s[%d]:  failing to linklInst\n", FILE__, __LINE__);
        }
       for (unsigned j = 0; j < bir->points_.size(); j++) {
           BPatch_point *bppoint = bir->points_[j];
           instPoint *point = bppoint->point;
             
          if (!point->linkInst(false)) {
               err = true;
           }

           if (atomic && err) break;
       }
       if (atomic && err) break;
   }

   if (atomic && err) 
      goto cleanup;

   llBinEdit->trapMapping.flush();

  cleanup:
    bool ret = true;

    if (atomic && err) {
        // Something failed...   Cleanup...
        for (unsigned k = 0; k < workDone.size(); k++) {
            workDone[k]->uninstrument();
        }
        ret = false;
    }

    for (unsigned int i = 0; i < pendingInsertions->size(); i++) {
        batchInsertionRecord *&bir = (*pendingInsertions)[i];
        assert(bir);
        delete(bir);
    }

    pendingInsertions->clear();

    llBinEdit->writeFile(outFile);

    return ret;
}



bool BPatch_binaryEdit::getType()
{
  return STATIC_EDITOR;
}

AddressSpace * BPatch_binaryEdit::getAS()
{
  return llBinEdit;
}

/*
 * BPatch_addressSpace::beginInsertionSet
 * 
 * Starts a batch insertion set; that is, all calls to insertSnippet until
 * finalizeInsertionSet are delayed.
 *
 */

void BPatch_binaryEdit::beginInsertionSetInt() 
{
    return;
}


bool BPatch_binaryEdit::finalizeInsertionSetInt(bool atomic, bool *modified) 
{
    return true;
}

BPatch_Vector<BPatch_function*> *BPatch_binaryEdit::findFunctionInt(const char *name, 
      BPatch_Vector<BPatch_function*> &funcs)
{
    // look for the function in this image
	image->findFunction(name, funcs, false);

	// if we can't find it in this image, search all the dependencies
	for (unsigned int i=0; funcs.size()==0 && i<depImages.size(); i++) {
		depImages.at(i)->findFunction(name, funcs, false);
	}

	return &funcs;
}

#if defined(cap_save_the_world)
bool BPatch_binaryEdit::loadLibraryInt(const char *libname, bool reload)
#else
bool BPatch_binaryEdit::loadLibraryInt(const char *libname, bool)
#endif
{
    BPatch_binaryEdit *depBinEdit;

#ifdef DEBUG_PRINT
    fprintf(stdout, "DEBUG - opening/adding new library: %s\n", libname);
#endif

    // if we can find the file...
    std::string* fullPath = resolveLibraryName(libname);
    if (fullPath != NULL) {

        // create binary edit object
        BPatch_binaryEdit *binEdit = new BPatch_binaryEdit(fullPath->c_str(), false);
        if (!binEdit) {
            return false;
        }

        // extract image (used for findFunction)
        depImages.push_back(new BPatch_image(binEdit));

        // add to list of dependencies in BinaryEdit object (used
        // for code generation)
        llBinEdit->addDependentBinEdit(binEdit->llBinEdit);

        delete fullPath;
    }

    return true;
}

std::string* BPatch_binaryEdit::resolveLibraryName(const std::string &filename)
{
    char *libPathStr, *libPath;
    std::vector<std::string> libPaths;
    struct stat dummy;
    std::string* fullPath = NULL;
    bool found = false;

    // build list of library paths
    
    // prefer fully-qualified file paths
    libPaths.push_back("");

    // add paths from environment variables
    libPathStr = getenv("LD_LIBRARY_PATH");
    libPath = strtok(libPathStr, ":");
    while (libPath != NULL) {
        libPaths.push_back(std::string(libPath));
        libPath = strtok(NULL, ":");
    }
    libPaths.push_back(std::string(getenv("PWD")));

    // TODO: add paths from /etc/ld.so.conf ?

    // add hard-coded paths
    libPaths.push_back("/usr/local/lib");
    libPaths.push_back("/usr/share/lib");
    libPaths.push_back("/usr/lib");
    libPaths.push_back("/lib");

    // find actual shared object file by searching all the lib paths
    for (unsigned int i = 0; !found && i < libPaths.size(); i++) {
        fullPath = new std::string(libPaths.at(i) + "/" + filename);

        // if we've found the file...
        if (stat(fullPath->c_str(), &dummy) == 0) {
            found = true;
        } else {
            delete fullPath;
            fullPath = NULL;
        }
    }

    return fullPath;
}

bool BPatch_binaryEdit::openAllDependencies(const char *path)
{
    // extract list of dependencies
    BPatch_binaryEdit *depBinEdit;
    Symtab *st = llBinEdit->getAOut()->parse_img()->getObject();
    std::vector<std::string> depends = st->getDependencies();

    // for each dependency
    unsigned int count = depends.size();
    for (unsigned int i = 0; i < count; i++) {

#ifdef DEBUG_PRINT
        fprintf(stdout, "DEBUG - opening new dependency: %s\n", depends.at(i).c_str());
#endif

        // if we can find the file...
        std::string* fullPath = resolveLibraryName(depends.at(i));
        if (fullPath != NULL) {

            // get list of subdependencies
            Symtab *subst;
            Symtab::openFile(subst, fullPath->c_str());
            std::vector<std::string> subdepends = subst->getDependencies();

            // add any new dependencies to the main list
            for (unsigned int k = 0; k < subdepends.size(); k++) {
                bool found = false;
                for (int l = 0; !found && l < depends.size(); l++) {
                    if (depends.at(l).compare(subdepends.at(k))==0) {
                        found = true;
                    }
                }
                if (!found) {
                    depends.push_back(subdepends.at(k));
                    count++;
                }
            }

            // open dependency and extract image (used for findFunction)
            depBinEdit = new BPatch_binaryEdit(fullPath->c_str(), false);
            if (!depBinEdit) {
                return false;
            }
            BPatch_image *img = new BPatch_image(depBinEdit);
            depImages.push_back(img);

#ifdef DEBUG_PRINT
            // print all functions
            BPatch_Vector<BPatch_function *> *allFuncs = img->getProcedures();
            char buffer[512];
            for (int i=0; i<allFuncs->size(); i++) {
                (*allFuncs)[i]->getName(buffer,512);
                fprintf(stdout,"        - %s\n", buffer);
            }
#endif

            // add to list of dependencies in BinaryEdit object (used
            // for code generation)
            llBinEdit->addDependentBinEdit(depBinEdit->llBinEdit);

            delete fullPath;
        }
    }
    return true;
}

