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

// $Id: timing-nt.C,v 1.6 2004/03/23 01:11:54 eli Exp $
#include <windows.h>   // for LARGE_INTEGERS
#include <assert.h>
#include <sys/types.h>
#include <sys/timeb.h>
#include <shlwapi.h>
#include "common/h/timing.h"

#include "common/h/int64iostream.h"

// returns us since 1970 at ms granularity
int64_t getRawTime1970() {
  struct _timeb timebuffer;
  int64_t us1970;
  _ftime(&timebuffer);
  us1970 = timebuffer.time;
  us1970 *= 1000000;
  us1970 += timebuffer.millitm*1000;
  return us1970;
}

double calcCyclesPerSecond_sys() {
  HKEY hKey;
  #define REGLOC "HARDWARE\\DESCRIPTION\\System\\CentralProcessor"
  LONG result = RegOpenKeyEx(HKEY_LOCAL_MACHINE,TEXT(REGLOC),0,KEY_READ,&hKey);
  if(result != ERROR_SUCCESS) {
    return cpsMethodNotAvailable;
  }

  #define INITIALBUFSIZE 50
  TCHAR subKeyBuf[INITIALBUFSIZE], initialCPU[INITIALBUFSIZE];
  FILETIME ft;
  DWORD mhz = 0;
  for(DWORD curIndex = 0; ; curIndex++) {
    DWORD size = INITIALBUFSIZE;
    result = RegEnumKeyEx(hKey, curIndex, subKeyBuf, &size, NULL, NULL,
			  NULL, &ft);
    if(result == ERROR_NO_MORE_ITEMS) {
      break;
    } else if (result != ERROR_SUCCESS) {
      return cpsMethodNotAvailable;
    }

    DWORD valType, value, bufLen = sizeof(DWORD);
    HKEY subKey;
    result = RegOpenKeyEx(hKey, subKeyBuf, NULL, KEY_READ, &subKey);
    if(result != ERROR_SUCCESS) {
      return cpsMethodNotAvailable;
    }
    
    result = RegQueryValueEx(subKey, TEXT("~MHz"), NULL, &valType, 
			     reinterpret_cast<LPBYTE>(&value), &bufLen);
    if(result != ERROR_SUCCESS) {
      return cpsMethodNotAvailable;
    }

    if(mhz == 0)  {
      mhz = value;
      strcpy(initialCPU, subKeyBuf);
    }
    else if(value != mhz) {
      cerr << "Warning: processor " << subKeyBuf << " has cycle rate of "
	   << value << " while processor " << initialCPU
	   << "\n         has cycle rate of " << mhz 
	   << ".  Using cycle rate of " << mhz << ".\n";
    }
  }
  RegCloseKey(hKey);
  return mhz * 1000000.0;
}

double calcCyclesPerSecondOS()
{
  double cps;
  cps = calcCyclesPerSecond_sys();
  if(cps == cpsMethodNotAvailable) {
    cps = calcCyclesPerSecond_default();
  }
  return cps;
}



