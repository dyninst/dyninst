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

// $Id: resource.C,v

#include <sys/types.h>
#include <signal.h>


#include "common/h/Types.h"
#include "paradynd/src/comm.h"
#include "paradynd/src/resource.h"
#include "common/h/String.h"
#include "common/h/Dictionary.h"
#include "paradynd/src/main.h"

extern MRN::Stream * defaultStream;

dictionary_hash<pdstring, resource*> resource::allResources(pdstring::hash);
dictionary_hash<unsigned, resource*> resource::res_dict(pd_uiHash);

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


resource *resource::newResource(resource *parent,
                                const pdstring& name,
                                unsigned id, 
                                ResourceType type,
				unsigned int mdlType)
{
  assert (name != (char*) NULL);
  assert ((name.c_str())[0] != '/');

	//	fprintf(stderr,"qqqqqqqqqqqq %d [%s:%d] name = %s\n",getpid(),__FILE__,__LINE__,name.c_str());

  pdstring res_string = parent->full_name() + slashStr + name;

  // first check to see if the resource has already been defined 
  resource *ret;
  if (allResources.find(res_string, ret)) // writes to ret if found
     return ret;

  pdstring abs;
  
  //ret = new resource(abs, name, 0.0, NULL, false, parent, v_names, type);
  ret = new resource(abs,
                        name,
                        timeStamp::ts1970(),
                        NULL,
                        false,
                        parent,
                        type,
                        mdlType);
  assert(ret);
  allResources[res_string] = ret;
  res_dict[id] = ret;
	// 	fprintf(stderr,"%u [%s:%u] resource id = %u\n",getpid(),__FILE__,__LINE__,ret);
  return(ret);
}

static pdvector<T_dyninstRPC::resourceInfoCallbackStruct> resourceInfoCallbackBuffer;

resource *resource::newResource(resource *parent, void *handle,
                                const pdstring &abstraction, 
                                const pdstring &name, timeStamp creation,
                                const pdstring &unique,
                                ResourceType type,
                                unsigned int mdlType,
                                bool /* send_it_now */)
{


	//	fprintf(stderr,"%u, [%s:%u] ----------\n",getpid(),__FILE__,__LINE__);
  assert (name != (char*) NULL);
  assert ((name.c_str())[0] != '/');

		
  static const pdstring leftBracket = "{";
  static const pdstring rightBracket = "}";
	
  pdstring unique_string(name);
	
  if (unique.length()) 
    {
			
      unique_string += leftBracket + unique + rightBracket;
			
    }
	
	
  pdstring res_string = parent->full_name() + slashStr + unique_string;
	
  // Has this resource already been defined?
  resource *ret;

	//	fprintf(stderr,"qqqqqqqqqqqq %d [%s:%d] name = %s\n",getpid(),__FILE__,__LINE__,res_string.c_str());

  if (allResources.find(res_string, ret)) // writes to 'ret' if found
    {
			return ret;
    }

	//	fprintf(stderr,"qqqqqqqqqqqq %d [%s:%d] name = %s\n",getpid(),__FILE__,__LINE__,res_string.c_str());
	
  // The components of this resource's name equals that of its parent, plus
  // the extra level not included with the parent.
	
  pdvector<pdstring> res_components = parent->names();
	
  res_components += unique_string;
	
	
  ret = new resource(abstraction,
                     unique_string,
                     creation,
                     handle,
                     false,
                     parent,
                     type,
                     mdlType);
	
	
  assert(ret);
	
  allResources[res_string] = ret;
	
	
  // TODO -- use pid here
  //logLine("newResource:  ");
  //logLine(P_strdup(name.c_str()));
  //logLine("\n");
	
  // generate a unique Id for this resource. Each daemon generates its own id
  // for the resource and report it to paradyn. If there are any conflicts
  // between ids generated by two daemons, paradyn will pick a new id and
  // send a resourceInfoResponse message to the daemons with the new id.
  // We generate Id's on the range 0..I32_MAX. Values from I32_MAX to UI32_MAX
  // are used by paradyn when there are conflicts
	
	
  unsigned id = pdstring::hash(res_string) % ((unsigned)I32_MAX);
  while (res_dict.defines(id)) 
     {
        id = (id + 1) % ((unsigned)I32_MAX);
     }
  res_dict[id] = ret;
	
  ret->set_id(id);
	
  //save for possible later transmittal
  T_dyninstRPC::resourceInfoCallbackStruct cbstruct;
  pdvector<pdstring> *ret_components = new pdvector<pdstring>;	
  *ret_components = ret->parent()->names();
  *ret_components += ret->name();
  cbstruct.temporaryId = ret->id();
  cbstruct.resource_name = *ret_components;
  cbstruct.abstraction = ret->abstraction();
  cbstruct.type = ret->type();
  cbstruct.mdlType = ret->mdlType();
	
  resourceInfoCallbackBuffer.push_back(cbstruct);
	
  return(ret);
}

void resource::send_now() {

  tp->severalResourceInfoCallback(defaultStream, resourceInfoCallbackBuffer);
  resourceInfoCallbackBuffer.resize(0);
}

void resource::updateResource(resource * old, const pdstring & abstraction,
                              pdvector<pdstring>& name,
                              ResourceType * /* type */,
                        pdvector <pdstring>& displayname, int retired)
{

    if(retired){ /* if it's retiring, we're not going to change anything else */
       pdstring res = old->full_name();
       tp -> retiredResource(defaultStream,res);
       return;
    }
    //commented out because 'set_abstraction' and 'set_type' are no longer defined.
    /*if(abstraction != (char *)NULL)
        old->set_abstraction(abstraction);
    if(type != NULL)
        old->set_type(*type); 
    */
    if(displayname.size() != 0)
        old->set_displayname(displayname[displayname.size()-1]);
    tp -> resourceUpdateCallback(defaultStream, name, displayname, abstraction);
}

resource *resource::newResource_ncb(resource *parent, void *handle,
				    const pdstring &abstraction, 
				    const pdstring &name, timeStamp creation,
				    const pdstring &unique,
				    ResourceType type,
				    unsigned int mdlType)
{
  assert (name != (char*) NULL);
  assert ((name.c_str())[0] != '/');

	//	fprintf(stderr,"qqqqqqqqqqqq %d [%s:%d] name = %s\n",getpid(),__FILE__,__LINE__,name.c_str());

  pdstring unique_string(name);
  if (unique.length()) 
    unique_string += pdstring("{") + unique + pdstring("}");

  pdstring res_string = parent->full_name() + "/" + unique_string;

 // Has this resource already been defined?
  if (allResources.defines(res_string))
    return (allResources[res_string]);

  // The components of this resource's name equals that of its parent, plus
  // the extra level not included with the parent.
  pdvector<pdstring> res_components = parent->names();
  res_components += unique_string;

  resource *ret = new resource(abstraction, unique_string, creation, handle,
			       false, parent, type, mdlType);
  assert(ret);

	//	fprintf(stderr,"%u [%s:%u] resource id = %u\n",getpid(),__FILE__,__LINE__,ret);

  allResources[res_string] = ret;

  return(ret);
}

bool resource::foc_to_strings(pdvector< pdvector<pdstring> >& string_foc,
															const pdvector<u_int>& ids,
															bool print_err_msg) 
{
  unsigned id_size = ids.size();
  for (unsigned i=0; i<id_size; i++) 
    {
      resource *r;
      if (!res_dict.find(ids[i], r))
				{ // writes to "r" if found
					
					if (print_err_msg) 
						{
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
void resource::make_canonical(const pdvector< pdvector<pdstring> >& focus,
			      pdvector< pdvector<pdstring> >& ret) {
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

  pdvector<pdstring> temp(1); // 1 entry vector

  if (!Machine) {temp[0]="Machine"; ret[resource::machine] = temp;}
  if (!Procedure) {temp[0]="Code"; ret[resource::procedure] = temp;}
  if (!SyncObj) {temp[0]="SyncObject"; ret[resource::sync_object] = temp;}
  if (!Memory) {temp[0]="Memory"; ret[resource::memory] = temp;}
}
void resource::report_ChecksumToFE( void )
{
	extern MRN::Stream * equivClassReportStream;
	extern unsigned sdm_id;
	resource::ChecksumType checksum = 0;

  resourceInfoCallbackBuffer.resize(0);


	// add all resource's contributio to the checksum
	dictionary_hash_iter<pdstring, resource*> iter = allResources;
	
	//fprintf(stderr, "CCC: in report_ChecksumToFE(). %d resources\n", allResources.size() );

	for( ; iter != allResources.end(); iter++ ) 
		{
			resource* curResource = *iter;
			if( curResource->is_CodeResource() )
				{
					resource::ChecksumType cur_checksum;
					cur_checksum = curResource->get_Checksum();
					checksum += cur_checksum;
				}
			else
				{
					T_dyninstRPC::resourceInfoCallbackStruct cbstruct;
					pdvector<pdstring> *res_components = new pdvector<pdstring>;
					
					*res_components = curResource->parent()->names();
										
					*res_components += curResource->name();
					
					cbstruct.temporaryId = curResource->id();
					cbstruct.resource_name = *res_components;
					cbstruct.abstraction = curResource->abstraction();
					cbstruct.type = curResource->type();
					cbstruct.mdlType = curResource->mdlType();

					resourceInfoCallbackBuffer += cbstruct;
					if( resourceInfoCallbackBuffer.size()>=100 )
						{
							tp->resourceBatchMode(defaultStream, true);
							resource::send_now();
							tp->resourceBatchMode(defaultStream, false);
						}					
				}
			
		}
	
	if( resourceInfoCallbackBuffer.size() > 0 )
		{
			tp->resourceBatchMode(defaultStream, true);
			resource::send_now();
			tp->resourceBatchMode(defaultStream, false);
		}

	//checksum calculated, send to FE
	pdvector<T_dyninstRPC::equiv_class_entry> entries;
	T_dyninstRPC::equiv_class_entry entry;
	
	entry.val = checksum;
	entry.class_rep = sdm_id;
	entries.push_back( entry );

	tp->resourceEquivClassReportCallback( equivClassReportStream, entries );
}

resource::ChecksumType resource::calculate_Checksum( void )
{
    resource::ChecksumType ret = 0;

    ret += id();
    //fprintf( stderr, "\tEEE: chksum val after id: %u\n", ret );

    // ...and our full name, including any leftover bytes...

    const pdstring& fname = full_name();
    const ChecksumType* curr = (const ChecksumType*)(fname.c_str());
    unsigned int nFullElements = fname.length() / sizeof(ChecksumType);
    unsigned int nLeftoverBytes = fname.length() - 
                                    nFullElements * sizeof(ChecksumType); 
    for( unsigned int i = 0; i < nFullElements; i++ )
    {
      //fprintf( stderr, "\t\tFFF: adding %u to chksum\n", *curr );
        ret += *curr;
        curr++;
    }
    //fprintf( stderr, "\tEEE: chksum val after full elems: %u\n", ret );
    // Add in the leftover bytes.
    // We have to be careful here, because we have no guarantee that we
    // can access the addresses following the pdstring.  Instead, we copy
    // bytes into a separate location and add it into the sum.
    ChecksumType leftovers = 0;
    for( unsigned int j = 0; j < nLeftoverBytes; j++ )
    {
        memcpy( &leftovers, curr, nLeftoverBytes );
    }
    ret += leftovers;
    return ret;
}

void resource::report_ResourcesToFE( void )
{

    T_dyninstRPC::resourceInfoCallbackStruct cbstruct;
    //pdvector<pdstring> res_components;

    dictionary_hash_iter<pdstring, resource*> iter = resource::allResources ;

    tp->resourceBatchMode(defaultStream, true);

    for( ; iter != resource::allResources.end();iter++ )
      {
        resource * cur_resource = *iter;

	
        if( cur_resource->is_CodeResource() )
					{
						pdvector<pdstring> *res_components = new pdvector<pdstring>;
						
						*res_components = cur_resource->parent()->names();
						
						*res_components += cur_resource->name();
						
						cbstruct.temporaryId = cur_resource->id();
						cbstruct.resource_name = *res_components;
						cbstruct.abstraction = cur_resource->abstraction();
						cbstruct.type = cur_resource->type();
						cbstruct.mdlType = cur_resource->mdlType();
						resourceInfoCallbackBuffer += cbstruct;
						
					}
				if( resourceInfoCallbackBuffer.size()==100 )
					{
						resource::send_now();
					}
      }
    if( resourceInfoCallbackBuffer.size() > 0 )
      resource::send_now();
    
    tp->resourceBatchMode(defaultStream, false);
    
    tp->resourceReportsDone(defaultStream,0);
		extern bool ok_toSendNewResources;
		ok_toSendNewResources = true;
}
