/*
 *  Copyright 1993 Jeff Hollingsworth.  All rights reserved.
 *
 */

/*
 * resource.C - handle resource creation and queries.
 * 
 * $Log: DMresource.C,v $
 * Revision 1.24  1995/07/20 22:34:22  rbi
 * fixed descendancy evaluation bug
 *
 * Revision 1.23  1995/07/15  03:34:53  karavan
 * fixed "paradyn suppress searchChildren" command by checking for parent's
 * suppress value in resource constructor.
 *
 * Revision 1.22  1995/06/02  20:48:28  newhall
 * * removed all pointers to datamanager class objects from datamanager
 *    interface functions and from client threads, objects are now
 *    refered to by handles or by passing copies of DM internal data
 * * removed applicationContext class from datamanager
 * * replaced List and HTable container classes with STL containers
 * * removed global variables from datamanager
 * * remove redundant lists of class objects from datamanager
 * * some reorginization and clean-up of data manager classes
 * * removed all stringPools and stringHandles
 * * KLUDGE: there are PC friend members of DM classes that should be
 *    removed when the PC is re-written
 *
 * Revision 1.20  1995/03/02  04:23:21  krisna
 * warning and bug fixes.
 *
 * Revision 1.19  1995/02/16  08:17:30  markc
 * Changed Boolean to bool
 * Added function to convert char* lists to vector<string>
 *
 * Revision 1.18  1995/01/26  17:58:24  jcargill
 * Changed igen-generated include files to new naming convention; fixed
 * some bugs compiling with gcc-2.6.3.
 *
 * Revision 1.17  1994/11/07  08:24:40  jcargill
 * Added ability to suppress search on children of a resource, rather than
 * the resource itself.
 *
 * Revision 1.16  1994/11/04  08:46:00  jcargill
 * Made suppressSearch flag be inherited from parent resource.  Solves the
 * problem of having to wait for processes to be defined to suppress them.
 *
 * Revision 1.15  1994/09/30  21:17:44  newhall
 * changed convertToStringList method function return value from
 * stringHandle * to char**
 *
 * Revision 1.14  1994/09/30  19:17:51  rbi
 * Abstraction interface change.
 *
 * Revision 1.13  1994/09/22  00:57:16  markc
 * Entered stringHandles into stringPool rather than assigning from const char *
 * Added casts to remove compiler warnings
 *
 * Revision 1.12  1994/08/05  16:04:00  hollings
 * more consistant use of stringHandle vs. char *.
 *
 * Revision 1.11  1994/07/28  22:31:09  krisna
 * include <rpc/types.h>
 * stringCompare to match qsort prototype
 * proper prorotypes for starting DMmain
 *
 * Revision 1.10  1994/07/26  20:03:06  hollings
 * added suppressSearch.
 *
 * Revision 1.9  1994/07/14  23:45:31  hollings
 * Changed printf of resource to be TCL list like.
 *
 * Revision 1.8  1994/06/27  21:23:31  rbi
 * Abstraction-specific resources and mapping info
 *
 * Revision 1.7  1994/06/17  00:11:55  hollings
 * Fixed off by one error in string canonical string name code.
 *
 * Revision 1.6  1994/06/14  15:25:03  markc
 * Added new call (sameRoot) to the resource class.  This call is used to
 * determine if two resources have the same parent but are not in an
 * ancestor-descendant relationship.  Such a relationship implies a conflict
 * in the two foci.
 *
 * Revision 1.5  1994/06/02  16:08:17  hollings
 * fixed duplicate naming problem for printResources.
 *
 * Revision 1.4  1994/05/31  19:11:34  hollings
 * Changes to permit direct access to resources and resourceLists.
 *
 * Revision 1.3  1994/04/18  22:28:33  hollings
 * Changes to create a canonical form of a resource list.
 *
 * Revision 1.2  1994/02/03  23:26:59  hollings
 * Changes to work with g++ version 2.5.2.
 *
 * Revision 1.1  1994/02/02  00:42:35  hollings
 * Changes to the Data manager to reflect the file naming convention and
 * to support the integration of the Performance Consultant.
 *
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dataManager.thread.h"
#include "DMresource.h"

/*  TODO: remove this
char *resource::createname(vector<string>& res_name){

  char *tempName = new char[255];
  unsigned j = 0;
  for(unsigned i=0; i < res_name.size()-1;i++){
      sprintf(&(tempName[j]), "%s/", res_name[i].string_of());
      j += res_name[i].length()+1;
  }
  if(i>0){
     sprintf(&(tempName[j]),"%s",res_name[i].string_of());
  }
  return(tempName);
  tempName = 0;
}
*/

//
// used only to construct root.
//
resource::resource()
{
    string temp = ""; 
    if(!allResources.defines(temp)){
        name = temp; 
        res_handle = resources.size();
        parent = res_handle; 
        suppressSearch = FALSE;
        suppressChildSearch = FALSE;
        abstr = NULL;
        resource *res = this;
        resources += res;
        allResources[name] = res;
    }
}

resource::resource(resourceHandle p_handle, 
		   vector<string>& resource_name,
		   string& r_name,
		   string& a) 
{
    
    if(!allResources.defines(r_name)){
	name = r_name;
	res_handle = resources.size();
        parent = p_handle;
	fullName = resource_name;
	resource *p = resources[parent];
	 
	suppressSearch = p->getSuppressChildren();
	suppressChildSearch = suppressSearch; // check for suppress
					      // of parent's children
        abstr = AMfind(a.string_of());
	resource *res = this;
	allResources[name] = res;
	resources += res;
        p->AddChild(res_handle);
    }
}

resource *resource::handle_to_resource(resourceHandle r_handle) {
     if (r_handle < resources.size()) {
         return(resources[r_handle]);    
     }
     return(NULL);
}
vector<resourceHandle> *resource::getChildren(){

    vector<resourceHandle> *temp = new vector<resourceHandle>;
    for(unsigned i=0; i < children.size(); i++){
        *temp += (resources[children[i]])->getHandle();      
    }
    return(temp);
}

resourceHandle *resource::findChild(const char *nm){
    string temp = nm;
    for(unsigned i=0; i < children.size(); i++){
        if((resources[children[i]])->match(temp)){
	     resourceHandle *h = new resourceHandle;
	     *h = children[i];
	     return(h);
	     h = 0;
        }
    }
    return(0);  // not found
}


void resource::print()
{
    printf("%s ", name.string_of());
}

bool resource::string_to_handle(string res,resourceHandle *h){
    if(allResources.defines(res)){
       resource *temp = allResources[res];
       *h = temp->getHandle();
       return(TRUE);
    }
    else
    return(FALSE);
}

/*
 * Convenience function with too many es
 *
 */
bool resource::isDescendent(resourceHandle child_handle)
{
    resourceHandle root_handle = rootResource->getHandle();
    resourceHandle this_handle = getHandle();

    if (child_handle == this_handle) {
      return FALSE;
    }
    if (this_handle == root_handle) {
      return TRUE;
    }
    while (child_handle != root_handle) {
        if (child_handle == this_handle) {
	  return TRUE;
        } else {
            child_handle = handle_to_resource(child_handle)->getParent();
        }
    }
    return FALSE;
}

/*
 * Do the two resources have the same base?
 * Note, since the there is a common root for all nodes,
 * the test for a common base checks the node below the
 * common root.
 */
bool resource::sameRoot(resourceHandle other)
{
  resource *myBase=0, *otherBase=0, *temp;

  temp = this;
  resourceHandle root = rootResource->getHandle(); 
  while (temp->getHandle() != root) {
    myBase = temp;
    temp = handle_to_resource(temp->parent);
  }
  temp = handle_to_resource(other);
  while (temp->getHandle() != root) {
    otherBase = temp;
    temp = handle_to_resource(temp->parent);
  }
  if (myBase == otherBase)
    return TRUE;
  else
    return FALSE;
}

const char *resource::getName(resourceHandle h){

    if(h < resources.size()){
        resource *res = resources[h];
	return(res->getName());
    }
    return 0;
}

resource *resource::string_to_resource(string res){

    if(allResources.defines(res)){
        return(allResources[res]);
    }
    return 0;
}

string DMcreateRLname(const vector<resourceHandle> &res){
    // create a unique name
    string temp;
    resource *next;
    for(unsigned i=0; i < (res.size() - 1); i++){
	next = resource::handle_to_resource(res[i]);
	temp += next->getFullName();
	temp += ",";
    }
    if(res.size() > 0){
	next = resource::handle_to_resource(res[i]);
	temp += next->getFullName();
    }

    return(temp);
}

resourceList::resourceList(const vector<resourceHandle> &res){
    // create a unique name
    string temp = DMcreateRLname(res);

    // see if this resourceList has been created already, if not add it
    if(!allFoci.defines(temp)){
        id = foci.size();
	resourceList *rl = this;
        allFoci[temp] = rl;
        fullName = temp;
        foci += rl;

        // create elements vector 
        for(unsigned i=0; i < res.size(); i++){
	    elements += resource::handle_to_resource(res[i]);
            // elements += r;
    } }
}

// this should be called with strings of fullNames for resources
// ex.  "/Procedure/blah.c/foo"  rather than "foo"
resourceList::resourceList(const vector<string> &names){
    // create a unique name
    unsigned size = names.size();
    string temp;
    for(unsigned i=0; i < size; i++){
       temp += names[i]; 
       if(i < (size-1)){
	   temp += ",";
       }
    }
    // see if this resourceList has been created already, if not add it
    if(!allFoci.defines(temp)){
        id = foci.size();
        resourceList *rl = this;
        allFoci[temp] = rl;
        fullName = temp;
        foci += rl;
        // create elements vector 
        for(i=0; i < size; i++){
	    elements += resource::string_to_resource(names[i]);
    } }
}


void resourceList::print()
{
    int i;

    printf("{");
    for (i=0; i < elements.size(); i++) {
	if (i) printf(" ");
	printf("{");
	elements[i]->print();
	printf("}");
    }
    printf("}");
}

#ifdef n_def
bool resourceList::convertToStringList(vector<string> &vs) {
    for (unsigned i=0; i < elements.size(); i++)
      vs += elements[i]->getFullName();
    return true;
}
#endif

bool resourceList::convertToStringList(vector< vector<string> > &fs) {
    for (unsigned i=0; i < elements.size(); i++)
        fs += elements[i]->getParts();
    return true;
}

bool resourceList::convertToIDList(vector<u_int> &fs) {
    for (unsigned i=0; i < elements.size(); i++){
        fs += elements[i]->getHandle();
    }
    return true;
}


const char *resourceList::getName(resourceListHandle rh){

    if(rh < foci.size()){
        resourceList *rl = foci[rh];
        return(rl->getName());
    }
    return(NULL);
}

vector<resourceHandle> *resourceList::getResourceHandles(resourceListHandle h){

    resourceList *focus = getFocus(h);
    if(focus){
        vector<resourceHandle> *handles = new vector<resourceHandle>;
        for(unsigned i=0; i < focus->elements.size(); i++){
	    resource *part = focus->elements[i];
            *handles += part->getHandle(); 
	}
	return(handles);
    }
    return(NULL);

}

const resourceListHandle *resourceList::find(const string &name){

    if(allFoci.defines(name)){
        resourceList *res_list = allFoci[name]; 
	const resourceListHandle *h = &res_list->id;
	return(h);
	res_list = 0;
    }
    return 0;


}

resourceListHandle resourceList::getResourceList(
				const vector<resourceHandle>& h){

    // does this resourceList already exist?
    string temp = DMcreateRLname(h);
    if(allFoci.defines(temp)){
	resourceList *rl = allFoci[temp];
        return(rl->getHandle());
    }
    // create a new resourceList
    resourceList *res = new resourceList(h);
    return(res->getHandle());
}

resourceList *resourceList::getFocus(resourceListHandle handle){

    if(handle < foci.size())
	return(foci[handle]);
    return 0;
}

resourceList *resourceList::findRL(const char *name){
    string temp = name;
    if(allFoci.defines(name)){
        return allFoci[name];
    }
    return NULL;
}

void printAllResources()
{
    for(unsigned i=0; i < resource::resources.size(); i++){
	printf("{");
        (resource::resources[i])->print();
	printf("}");
        printf("\n");
    }
}
