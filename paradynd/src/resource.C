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

#include "symtab.h"
#include "process.h"
#include "dyninstP.h"
#include "util.h"
#include "comm.h"
#include "util/h/String.h"
#include "util/h/Dictionary.h"
#include <strstream.h>
#include "main.h"

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

resource *resource::newResource(resource *parent, const string& name, unsigned id, 
				unsigned type) {
  assert (name != (char*) NULL);
  assert ((name.string_of())[0] != '/');

  string res_string = parent->full_name() + "/" + name;

  // first check to see if the resource has already been defined 
  if (allResources.defines(res_string))
    return (allResources[res_string]);

  vector<string> v_names = parent->names();
  v_names += name;
  string abs;
  resource *ret = new resource(abs, name, 0.0, NULL, false, parent, v_names, type);
  assert(ret);
  allResources[res_string] = ret;
  res_dict[id] = ret;
  return(ret);
}

resource *resource::newResource(resource *parent, void *handle,
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
			       false, parent, res_components, type);
  assert(ret);
  allResources[res_string] = ret;

  // TODO -- use pid here
  //logLine("newResource:  ");
  //logLine(P_strdup(name.string_of()));
  //logLine("\n");
  num_outstanding_creates++;
  tp->resourceInfoCallback(0, res_components, abstraction, type); 
  return(ret);
}

bool resource::foc_to_strings(vector< vector<string> >& string_foc, vector<u_int>& ids) {
  unsigned id_size = ids.size();
  for (unsigned i=0; i<id_size; i++) {
    if (!res_dict.defines(ids[i])) return false;
    resource *r = res_dict[ids[i]];
    string_foc += r->names();
  }
  return true;
}

// Other parts of the system depend on this order (mdl.C)
// Assume that there are 4 top level resources
void resource::make_canonical(const vector< vector<string> >& focus,
			      vector< vector<string> >& ret) {
  unsigned size = focus.size();
  bool machine=false, procedure=false, process=false, sync=false;
  ret.resize(4);
  for (unsigned f=0; f<size; f++) {
    assert(focus[f].size() > 0);
    if (focus[f][0] == "Machine") {
      machine = true;
      ret[resource::machine] = focus[f];
//    } else if (focus[f][0] == "Procedure") {      
    } else if (focus[f][0] == "Code") {      
      procedure = true;
      ret[resource::procedure] = focus[f];
    } else if (focus[f][0] == "Process") {
      process = true;
      ret[resource::process] = focus[f];
    } else if (focus[f][0] == "SyncObject") {
      sync = true;
      ret[resource::sync_object] = focus[f];
    }
  }

  vector<string> temp(1); // 1 entry vector

  if (!machine) {temp[0]="Machine"; ret[resource::machine] = temp;}
//  if (!procedure) ret[resource::procedure] = "Procedure";
  if (!procedure) {temp[0]="Code"; ret[resource::procedure] = temp;}
  if (!process) {temp[0]="Process"; ret[resource::process] = temp;}
  if (!sync) {temp[0]="SyncObject"; ret[resource::sync_object] = temp;}
}
