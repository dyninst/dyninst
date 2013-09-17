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


// $Id: timing-nt.C,v 1.8 2007/05/30 19:20:33 legendre Exp $
#include "common/src/ntHeaders.h"   // for LARGE_INTEGERS
#include <assert.h>
#include <sys/types.h>
#include <sys/timeb.h>
#include <shlwapi.h>
#include "common/src/timing.h"

#include "common/src/int64iostream.h"

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
     return 0.0;
  }
  return cps;
}



