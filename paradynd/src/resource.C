/*
 *  Copyright 1993 Jeff Hollingsworth.  All rights reserved.
 *
 */

#ifndef lint
static char Copyright[] = "@(#) Copyright (c) 1993 Jeff Hollingsowrth\
    All rights reserved.";

static char rcsid[] = "@(#) $Header: /home/jaw/CVSROOT_20081103/CVSROOT/core/paradynd/src/resource.C,v 1.19 1996/06/01 00:04:10 tamches Exp $";
#endif

/*
 * resource.C - handle resource creation and queries.
 *
 * $Log: resource.C,v $
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
 * Revision 1.14  1995/02/16  08:54:13  markc
 * Corrected error in comments -- I put a "star slash" in the comment.
 *
 * Revision 1.13  1995/02/16  08:34:43  markc
 * Changed igen interfaces to use strings/vectors rather than char igen-arrays
 * Changed igen interfaces to use bool, not Boolean.
 * Cleaned up symbol table parsing - favor properly labeled symbol table objects
 * Updated binary search for modules
 * Moved machine dependnent ptrace code to architecture specific files.
 * Moved machine dependent code out of class process.
 * Removed almost all compiler warnings.
 * Use "posix" like library to remove compiler warnings
 *
 * Revision 1.12  1994/11/06  09:53:14  jcargill
 * Fixed early paradynd startup problem; resources sent by paradyn were
 * being added incorrectly at the root level.
 *
 * Revision 1.11  1994/11/02  11:16:57  markc
 * REplaced container classes.
 *
 * Revision 1.10  1994/09/22  02:24:46  markc
 * cast stringHandles
 *
 * Revision 1.9  1994/08/08  20:13:46  hollings
 * Added suppress instrumentation command.
 *
 * Revision 1.8  1994/07/28  22:40:45  krisna
 * changed definitions/declarations of xalloc functions to conform to alloc.
 *
 * Revision 1.7  1994/06/27  21:28:20  rbi
 * Abstraction-specific resources and mapping info
 *
 * Revision 1.6  1994/06/27  18:57:08  hollings
 * removed printfs.  Now use logLine so it works in the remote case.
 * added internalMetric class.
 * added extra paramter to metric info for aggregation.
 *
 * Revision 1.5  1994/06/02  23:28:00  markc
 * Replaced references to igen generated class to a new class derived from
 * this class to implement error handling for igen code.
 *
 * Revision 1.4  1994/05/16  22:31:54  hollings
 * added way to request unique resource name.
 *
 * Revision 1.3  1994/02/24  04:32:36  markc
 * Changed header files to reflect igen changes.  main.C does not look at the number of command line arguments now.
 *
 * Revision 1.2  1994/02/01  18:46:55  hollings
 * Changes for adding perfConsult thread.
 *
 * Revision 1.1  1994/01/27  20:31:41  hollings
 * Iinital version of paradynd speaking dynRPC igend protocol.
 *
 * Revision 1.3  1993/07/13  18:30:02  hollings
 * new include file syntax.
 * expanded tempName to 255 chars for c++ support.
 *
 * Revision 1.2  1993/06/08  20:14:34  hollings
 * state prior to bc net ptrace replacement.
 *
 * Revision 1.1  1993/03/19  22:45:45  hollings
 * Initial revision
 *
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
