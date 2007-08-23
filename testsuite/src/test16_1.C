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

// $Id: test16_1.C,v 1.2 2007/08/23 18:27:28 rutar Exp $
/*
 * #Name: test1_2
 * #Desc: Mutator Side (call a four argument function)
 * #Arch: all
 * #Dep: 
 * #Notes: Uses mutateeFortran variable
 */

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"
#include "BPatch_image.h"

#include "test_lib.h"

#define PARALLEL_REGIONS 10
#define DO_FOR_REGIONS   5
#define SECTIONS_REGIONS 6
#define SINGLE_REGIONS   1
#define PARALLEL_DO_REGIONS 1
#define PARALLEL_SECTIONS_REGIONS 0
#define MASTER_REGIONS  1
#define CRITICAL_REGIONS 1
#define BARRIER_REGIONS 1
#define ATOMIC_REGIONS 1
#define ORDERED_REGIONS 1



//
// Start Test Case #1 - Counting the number of parsed regions
//
static int mutatorTest(BPatch_thread *appThread, BPatch_image *appImage)
{
  const char* testName = "OpenMP region count";

  BPatch_Vector< BPatch_parRegion * > *appParRegions = appImage->getParRegions();

  int regionNumbers[15];
  for (int i = 0; i < 15; i++)
    regionNumbers[i] = 0;

  for(int i = 0; i < appParRegions->size(); i++)
    {
      int regionType = (*appParRegions)[i]->getClause("REGION_TYPE");
      regionNumbers[regionType]++;
    }

  if (regionNumbers[OMP_PARALLEL] != PARALLEL_REGIONS){
    logerror("Incorrect Number of Parallel Regions, reported %d, should be %d\n", 
	     regionNumbers[OMP_PARALLEL], PARALLEL_REGIONS);
    return -1;
  }

  if (regionNumbers[OMP_DO_FOR] != DO_FOR_REGIONS){
    logerror("Incorrect Number of Do/For Regions, reported %d, should be %d\n", 
	     regionNumbers[OMP_DO_FOR], DO_FOR_REGIONS);
    return -1;
  }
  if (regionNumbers[OMP_SECTIONS] != SECTIONS_REGIONS){
    logerror("Incorrect Number of Sections Regions, reported %d, should be %d\n", 
	     regionNumbers[OMP_SECTIONS], SECTIONS_REGIONS);
    return -1;
  }

  if (regionNumbers[OMP_SINGLE] != SINGLE_REGIONS){
    logerror("Incorrect Number of Single Regions, reported %d, should be %d\n", 
	     regionNumbers[OMP_SINGLE], SINGLE_REGIONS);
    return -1;
  }

  if (regionNumbers[OMP_PAR_DO] != PARALLEL_DO_REGIONS){
    logerror("Incorrect Number of Parallel Do Regions, reported %d, should be %d\n", 
	     regionNumbers[OMP_PAR_DO], PARALLEL_DO_REGIONS);
    return -1;
  }

  if (regionNumbers[OMP_PAR_SECTIONS] != PARALLEL_SECTIONS_REGIONS){
    logerror("Incorrect Number of Parallel For Regions, reported %d, should be %d\n", 
	     regionNumbers[OMP_PAR_SECTIONS], PARALLEL_SECTIONS_REGIONS);
    return -1;
  }

  if (regionNumbers[OMP_MASTER] != MASTER_REGIONS){
    logerror("Incorrect Number of Master Regions, reported %d, should be %d\n", 
	     regionNumbers[OMP_MASTER], MASTER_REGIONS);
    return -1;
  }

  if (regionNumbers[OMP_CRITICAL] != CRITICAL_REGIONS){
    logerror("Incorrect Number of Critical Regions, reported %d, should be %d\n", 
	     regionNumbers[OMP_CRITICAL], CRITICAL_REGIONS);
    return -1;
  }

  if (regionNumbers[OMP_BARRIER] != BARRIER_REGIONS){
    logerror("Incorrect Number of Barrier Regions, reported %d, should be %d\n", 
	     regionNumbers[OMP_BARRIER], BARRIER_REGIONS);
    return -1;
  }

  if (regionNumbers[OMP_ATOMIC] != ATOMIC_REGIONS){
    logerror("Incorrect Number of Atomic Regions, reported %d, should be %d\n", 
	     regionNumbers[OMP_ATOMIC], ATOMIC_REGIONS);
    return -1;
  }

  if (regionNumbers[OMP_ORDERED] != ORDERED_REGIONS){
    logerror("Incorrect Number of Ordered Regions, reported %d, should be %d\n", 
	     regionNumbers[OMP_ORDERED], ORDERED_REGIONS);
    return -1;
  }

  logerror("Passed test #1 (OpenMP regions parsed correctly)\n");

  return 0;
}

// External Interface
extern "C" TEST_DLL_EXPORT int test16_1_mutatorMAIN(ParameterDict &param)
{
    BPatch *bpatch;
    bpatch = (BPatch *)(param["bpatch"]->getPtr());
    BPatch_thread *appThread = (BPatch_thread *)(param["appThread"]->getPtr());

    FILE *outlog = (FILE *)(param["outlog"]->getPtr());
    FILE *errlog = (FILE *)(param["errlog"]->getPtr());
    setOutputLog(outlog);
    setErrorLog(errlog);

    // Read the program's image and get an associated image object
    BPatch_image *appImage = appThread->getImage();
    
    // Run mutator code
    return mutatorTest(appThread, appImage);
}
