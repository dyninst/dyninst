/*
 * Copyright (c) 1996 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as Paradyn") on an AS IS basis, and do not warrant its
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

/*
 * resource.C - handle resource creation and queries.
 *
 * $Log: resource.C,v $
 * Revision 1.32  2000/07/28 17:22:33  pcroth
 * Updated #includes to reflect util library split
 *
 * Revision 1.31  2000/06/14 23:04:24  wylie
 * Tidy to remove compiler warnings
 *
 * Revision 1.30  2000/03/06 21:41:25  zhichen
 * Moved /Process hierarchy to /Machine hierarchy.
 *
 * Revision 1.29  1997/06/27 18:15:39  tamches
 * optimized newResource w.r.t. its igen behavior
 *
 * Revision 1.28  1997/06/23 17:04:36  tamches
 * used dictionary find() to achieve some speedup
 *
 * Revision 1.27  1997/04/29 23:17:44  mjrg
 * Changes for WindowsNT port
 * Delayed check for DYNINST symbols to allow linking libdyninst dynamically
 * Changed way paradyn and paradynd generate resource ids
 * Changes to instPoint class in inst-x86.C to reduce size of objects
 * Added initialization for process->threads to fork and attach constructors
 *
 * Revision 1.26  1997/04/14 20:08:03  zhichen
 * Added   enum index { machine, procedure, process, sync_object, memory };
 *         resource *memoryRoot; // shared-memory resource
 *         resource *memoryResource; // shared-memory resource
 *         resource *resource::newResource_ncb(...)
 * changed void resource::make_canonical...)
 *
 * Revision 1.25  1997/03/29 02:11:06  sec
 * Debugging stuff
 *
 * Revision 1.24  1997/02/26 23:46:48  mjrg
 * First part of WindowsNT port: changes for compiling with Visual C++;
 * moved unix specific code to unix.C file
 *
 * Revision 1.23  1997/02/21 20:16:03  naim
 * Moving files from paradynd to dyninstAPI + eliminating references to
 * dataReqNode from the ast class. This is the first pre-dyninstAPI commit! - naim
 *
 * Revision 1.22  1997/01/15 00:30:18  tamches
 * added some uses of dictionary find() method
 *
 * Revision 1.21  1996/09/26 18:59:12  newhall
 * added support for instrumenting dynamic executables on sparc-solaris
 * platform
 *
 * Revision 1.20  1996/08/16 21:19:46  tamches
 * updated copyright for release 1.1
 *
 * Revision 1.19  1996/06/01 00:04:10  tamches
 * const and refs added in appropriate places to enhance speed and compile
 * time error checking
 *
 * Revision 1.18  1996/03/01 22:35:57  mjrg
 * Added a type to resources.
 * Changes to the MDL to handle the resource hierarchy better.
 *
 * Revision 1.17  1995/12/20 16:10:23  tamches
 * minor changes for the new vector class (no constructor taking in T&)
 *
 * Revision 1.16  1995/07/24 03:53:13  tamches
 * The Procedure -- > Code commit
 *
 * Revision 1.15  1995/05/18  10:41:49  markc
 * Cache global ids supplied by paradyn
 * have a canonical form for the resource list
 *
 */

#include <limits.h>
#include "dyninstAPI/src/symtab.h"
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/dyninstP.h"
#include "dyninstAPI/src/util.h"
#include "paradynd/src/comm.h"
#include "common/h/String.h"
#include "common/h/Dictionary.h"
#include "paradynd/src/main.h"

u_int resource::num_outstanding_creates = 0;
dictionary_hash<string, resource*> resource::allResources(string::hash);
dictionary_hash<unsigned, resource*> resource::res_dict(uiHash);

/*
 * handle the notification of resource creation and deletion.
 *
 */

resource *rootResource;
resource *machineRoot;
resource *machineResource;
resource *processResource;
resource *moduleRoot;
resource *syncRoot;
resource *memoryRoot; // shared-memory resource
resource *memoryResource; // shared-memory resource


resource *resource::newResource(resource *parent, const string& name, unsigned id, 
				unsigned type) {
  assert (name != (char*) NULL);
  assert ((name.string_of())[0] != '/');

  string res_string = parent->full_name() + slashStr + name;

  // first check to see if the resource has already been defined 
  resource *ret;
  if (allResources.find(res_string, ret)) // writes to ret if found
     return ret;

  string abs;

  
  //ret = new resource(abs, name, 0.0, NULL, false, parent, v_names, type);
  ret = new resource(abs, name, 0.0, NULL, false, parent, type);
  assert(ret);

  allResources[res_string] = ret;
  res_dict[id] = ret;
  return(ret);
}

static vector<T_dyninstRPC::resourceInfoCallbackStruct> resourceInfoCallbackBuffer;

resource *resource::newResource(resource *parent, void *handle,
				const string &abstraction, 
				const string &name, timeStamp creation,
				const string &unique,
				unsigned type,
				bool send_it_now)
{
  assert (name != (char*) NULL);
  assert ((name.string_of())[0] != '/');
  static const string leftBracket = "{";
  static const string rightBracket = "}";

  string unique_string(name);
  if (unique.length()) 
    unique_string += leftBracket + unique + rightBracket;

  string res_string = parent->full_name() + slashStr + unique_string;

  // Has this resource already been defined?
  resource *ret;
  if (allResources.find(res_string, ret)) // writes to 'ret' if found
     return ret;

  // The components of this resource's name equals that of its parent, plus
  // the extra level not included with the parent.
  vector<string> res_components = parent->names();
  res_components += unique_string;

  ret = new resource(abstraction, unique_string, creation, handle,
		     false, parent, type);
  assert(ret);
  allResources[res_string] = ret;

  // TODO -- use pid here
  //logLine("newResource:  ");
  //logLine(P_strdup(name.string_of()));
  //logLine("\n");

  // generate a unique Id for this resource. Each daemon generates its own id
  // for the resource and report it to paradyn. If there are any conflicts
  // between ids generated by two daemons, paradyn will pick a new id and
  // send a resourceInfoResponse message to the daemons with the new id.
  // We generate Id's on the range 0..INT_MAX. Values from INT_MAX to UINT_MAX
  // are used by paradyn when there are conflicts
  unsigned id = string::hash(res_string) % ((unsigned)INT_MAX);
  while (res_dict.defines(id)) {
    id = (id + 1) % ((unsigned)INT_MAX);
  }
  res_dict[id] = ret;

  T_dyninstRPC::resourceInfoCallbackStruct cbstruct;
  cbstruct.temporaryId = id;
  cbstruct.resource_name = res_components;
  cbstruct.abstraction = abstraction;
  cbstruct.type = type;
  resourceInfoCallbackBuffer += cbstruct;
  if (resourceInfoCallbackBuffer.size()==100)
     send_it_now = true;

  if (send_it_now)
     send_now();

  return(ret);
}

void resource::send_now() {
  tp->severalResourceInfoCallback(resourceInfoCallbackBuffer);
  resourceInfoCallbackBuffer.resize(0);
}

resource *resource::newResource_ncb(resource *parent, void *handle,
				const string &abstraction, 
				const string &name, timeStamp creation,
				const string &unique,
				unsigned type)
{
  assert (name != (char*) NULL);
  assert ((name.string_of())[0] != '/');

  string unique_string(name);
  if (unique.length()) 
    unique_string += string("{") + unique + string("}");

  string res_string = parent->full_name() + "/" + unique_string;

  // Has this resource already been defined?
  if (allResources.defines(res_string))
    return (allResources[res_string]);

  // The components of this resource's name equals that of its parent, plus
  // the extra level not included with the parent.
  vector<string> res_components = parent->names();
  res_components += unique_string;

  resource *ret = new resource(abstraction, unique_string, creation, handle,
			       false, parent, type);
  assert(ret);
  allResources[res_string] = ret;

  return(ret);
}

bool resource::foc_to_strings(vector< vector<string> >& string_foc,
			      const vector<u_int>& ids,
			      bool print_err_msg) {
  unsigned id_size = ids.size();
  for (unsigned i=0; i<id_size; i++) {
    resource *r;
    if (!res_dict.find(ids[i], r)) { // writes to "r" if found
       if (print_err_msg) {
	 cerr << "resource::foc_to_strings -- could not find resource entry for id " << ids[i] << endl;
       }
       return false;
    }

    string_foc += r->names();
  }

  return true;
}

// Other parts of the system depend on this order (mdl.C)
// Assume that there are 4 top level resources
void resource::make_canonical(const vector< vector<string> >& focus,
			      vector< vector<string> >& ret) {
  unsigned size = focus.size();
  bool Machine=false, Procedure=false, SyncObj=false, Memory=false;
  ret.resize(4); 
  for (unsigned f=0; f<size; f++) {
    assert(focus[f].size() > 0);
    if (focus[f][0] == "Machine") {
      Machine = true;
      ret[resource::machine] = focus[f];
    } else if (focus[f][0] == "Code") {      
      Procedure = true;
      ret[resource::procedure] = focus[f];
    } else if (focus[f][0] == "SyncObject") {
      SyncObj = true;
      ret[resource::sync_object] = focus[f];
    } else if (focus[f][0] == "Memory") {
      Memory = true;
      ret[resource::memory] = focus[f];
    }
  }

  vector<string> temp(1); // 1 entry vector

  if (!Machine) {temp[0]="Machine"; ret[resource::machine] = temp;}
  if (!Procedure) {temp[0]="Code"; ret[resource::procedure] = temp;}
  if (!SyncObj) {temp[0]="SyncObject"; ret[resource::sync_object] = temp;}
  if (!Memory) {temp[0]="Memory"; ret[resource::memory] = temp;}
}
