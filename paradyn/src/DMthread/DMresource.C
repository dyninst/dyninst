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

// $Id: DMresource.C,v 1.59 2002/12/20 07:50:01 jaw Exp $

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "CallGraph.h"
#include "dataManager.thread.h"
#include "DMresource.h"
#include "paradyn/src/met/metricExt.h"
#include "paradyn/src/DMthread/MagnifyManager.h"
#include "paradyn/src/DMthread/DMperfstream.h"
// Generate a new resource handle. The daemons generate resources id's (handles)
// in the range 0..INT_MAX. If there are conflicts between the handles generated
// by two daemons, paradyn generates a new id in the range INT_MAX..UINT_MAX
static unsigned handles = (unsigned) INT_MAX;

inline unsigned newResourceHandle() {
  return handles++;
}


//
// used only to construct root.
//
resource::resource()
{
    string temp = "ROOT"; 
    assert(allResources.size()==0);
    if(!allResources.defines(temp)){
        name = temp; 
	res_handle = 0;
        parent = res_handle; 
        suppressSearch = FALSE;
        suppressChildSearch = FALSE;
	suppressMagnify = false;
        retired = false;
        abstr = NULL;
        resource *res = this;
	resources[res_handle] = res;
        allResources[name] = res;
    }
}


resource::resource(resourceHandle p_handle, 
		   unsigned tempId,
		   pdvector<string>& resource_name,
		   string& r_name,
		   string& a, unsigned res_type) 
{
    
    if(!allResources.defines(r_name)){
        type = res_type;
        name = r_name;
	// the daemons generate an id for the resource. If there are no
	// conflicts between this id and the id for other resource, we
	// can keep the daemon id, otherwise we must generate a new
	// id.
	if (resources.defines(tempId))
	   res_handle = newResourceHandle();
	else
	   res_handle = tempId;
        parent = p_handle;
	fullName = resource_name;
	resource *p = resources[parent];
	 
	suppressSearch = p->getSuppressChildren();
	suppressChildSearch = suppressSearch; // check for suppress
					      // of parent's children
	suppressMagnify = false;
        retired = false;
        abstr = AMfind(a.c_str());
	resource *res = this;
	allResources[name] = res;
	resources[res_handle] = res;
        p->AddChild(res_handle);
    }
    else assert(0);
}

resource::resource(resourceHandle p_handle, 
		   pdvector<string>& resource_name,
		   string& r_name,
		   string& a, unsigned res_type) 
{
    
    if(!allResources.defines(r_name)){
        type = res_type;
        name = r_name;
	res_handle = string::hash(name);
	while (resources.defines(res_handle))
	  res_handle = (res_handle+1) % (unsigned)INT_MAX;
        parent = p_handle;
	fullName = resource_name;
	resource *p = resources[parent];
	 
	suppressSearch = p->getSuppressChildren();
	suppressChildSearch = suppressSearch; // check for suppress
					      // of parent's children
	suppressMagnify = false;
        retired = false;
        abstr = AMfind(a.c_str());
	resource *res = this;
	allResources[name] = res;
	resources[res_handle] = res;
        p->AddChild(res_handle);
    }
    else assert(0);
}


resource *resource::handle_to_resource(resourceHandle r_handle) {
     // Note: It would be better if this routine returns a reference, and
     // just asserts if the r_handle is bad.  Why?  Because it seems like
     // noone is checking for a NULL return value anyway!
     resource *res;
     if (resources.find(r_handle, res))
       return res;
     else
       return NULL;
}

// I don't want to parse for '/' more than once, thus the use of a string
// vector
resourceHandle resource::createResource(unsigned res_id, 
                                        pdvector<string>& resource_name,
                                        string& abstr, unsigned type) 
{
   static const string slashStr = "/";
   static const string baseStr = "BASE";

   resource *parent = NULL;
   unsigned r_size = resource_name.size();
   string p_name;


   switch (r_size) {
     case 0:
        // Should this case ever occur ?
        assert(0); break;
     case 1:
        parent = resource::getRootResource(); break;
     default:
        for (unsigned ri=0; ri<(r_size-1); ri++) 
           p_name += slashStr + resource_name[ri];
        parent = resource::string_to_resource(p_name);
        assert(parent);
        break;
   }
   if (!parent) assert(0);


   /* first check to see if the resource has already been defined */
   // resource *p = resource::resources[parent->getHandle()];
   string myName = p_name;
   myName += slashStr;
   myName += resource_name[r_size - 1];

   resource *child = resource::string_to_resource(myName.c_str());
   if(child) {
      return child->getHandle();
   }

   // if abstr is not defined then use default abstraction 
   if(!abstr.c_str()){
      abstr = baseStr;
   }

   /* then create it */
   resource *ret =  new resource(parent->getHandle(),res_id, resource_name,
                                 myName,abstr, type);

   // check to see if the suppressMagnify option should be set...if
   // this resource is specifed in the mdl exclude_lib option
   pdvector<string> shared_lib_constraints;
   pdvector<unsigned> constraint_flags;
   if(resource::get_lib_constraints(shared_lib_constraints, constraint_flags) &&
      (string(parent->getFullName()) == "/Code")) {
      for(u_int i=0; i < shared_lib_constraints.size(); i++){

         // grab the exclude flags
         bool checkCase = ((constraint_flags[i] & LIB_CONSTRAINT_NOCASE_FLAG) 
                           == 0);
         bool regex = ( constraint_flags[i] & LIB_CONSTRAINT_REGEX_FLAG ) != 0;

         // A regular expression will match any location within the string,
         // unless otherwise specified with ^ and $
         if( regex )
            shared_lib_constraints[i] = "^" + shared_lib_constraints[i] + "$";

         // By default (!regex), check using wildcardEquiv, if the REGEX flag
         // is set, then use regexEquiv, passing the NOCASE flag as needed

         if((regex && 
             shared_lib_constraints[i].regexEquiv(ret->getName(), checkCase))
            || (!regex && 
                shared_lib_constraints[i].wildcardEquiv(ret->getName(), 
                                                        checkCase))
           )
         {
            ret->setSuppressMagnify();
#ifdef notdef
            cerr << '\"' << ret->getName() << "\" hit against exclude \""
                 << shared_lib_constraints[i] << '\"';
            if( regex ) cerr << " using regex";
            cerr << endl;
#endif
         }
      }
   }

   // check to see if the suppressMagnify option should be set if the
   // resource is a function that is specified in the mdl exclude_func
   // options
   if(!ret->isMagnifySuppressed()){
      if(parent != resource::getRootResource()) {
         // get parent of parent, if it is "/Code" then check the list of
         // exculded_funcs
         resourceHandle pph = parent->getParent();
         resource *ppr = resource::handle_to_resource(pph);
         if( ppr && (string(ppr->getFullName()) == "/Code")) {
            pdvector< pdvector<string> > libs;
            constraint_flags.resize( 0 );
            if(resource::get_func_constraints(libs, constraint_flags)) {
               for(u_int i=0; i < libs.size(); i++){
                  // grab the exclude flags
                  bool checkCase = (constraint_flags[i] & 
                                    LIB_CONSTRAINT_NOCASE_FLAG ) == 0;
                  bool regex = (constraint_flags[i] & 
                                LIB_CONSTRAINT_REGEX_FLAG ) != 0;

                  // By default (!regex), check using wildcardEquiv, if the
                  // REGEX flag is set, then use regexEquiv, passing the
                  // NOCASE flag as needed

                  if( regex ) {
                     // A regular expression will match any location within
                     // the string, unless otherwise specified with ^ and $
                     (libs[i])[0] = "^" + (libs[i])[0] + "$";
                     (libs[i])[1] = "^" + (libs[i])[1] + "$";
                  }

                  if((regex && 
                      ((libs[i])[0].regexEquiv(parent->getName(),checkCase)) &&
                      ((libs[i])[1].regexEquiv(ret->getName(), checkCase)))
                     || (!regex &&
                         ((libs[i])[0].wildcardEquiv(parent->getName(), 
                                                     checkCase )) && 
                         ((libs[i])[1].wildcardEquiv(ret->getName(), 
                                                     checkCase)) 
                        )
                    )
                  {
                     ret->setSuppressMagnify(); 
#ifdef notdef
                     cerr << '\"' << parent->getName() << '/' << ret->getName()
                          << "\" hit against exclude \""
                          << (libs[i])[0] << '/' << (libs[i])[1] << '\"';
                     if( regex ) cerr << " using regex";
                     cerr << endl;
#endif
                  }
               } 
            } 
         }
      }
   }

   /* inform others about it if they need to know */
   performanceStream::psIter_t allS = performanceStream::getAllStreamsIter();
   perfStreamHandle h;
   performanceStream *ps;
   resourceHandle r_handle = ret->getHandle();
   string name = ret->getFullName(); 
   while(allS.next(h,ps)){
      ps->callResourceFunc(parent->getHandle(),r_handle,ret->getFullName(),
                           ret->getAbstractionName());
   }
   return(r_handle);
}

resourceHandle resource::createResource_ncb(pdvector<string>& resource_name, 
                                            string& abstr, unsigned type,
                                            resourceHandle &p_handle, 
                                            bool &exist) 
{
   resource *parent = NULL;
   unsigned r_size = resource_name.size();
   string p_name;


   switch (r_size) {
     case 0:
        // Should this case ever occur ?
        assert(0); break;
     case 1:
        parent = resource::getRootResource();  break;
     default:
        for (unsigned ri=0; ri<(r_size-1); ri++) 
           p_name += string("/") + resource_name[ri];
        parent = resource::string_to_resource(p_name);
        assert(parent);
        break;
   }
   if (!parent) assert(0);


   /* first check to see if the resource has already been defined */
   p_handle = parent->getHandle() ;
   resource *p = resource::handle_to_resource(parent->getHandle());
   string myName = p_name;
   myName += "/";
   myName += resource_name[r_size - 1];
   if(!exist) {
      resourceHandle *child = p->findChild(myName.c_str());
      if (child){
         return(*child); 
         delete child;
      }
   } else {
      exist = false ;
   }

   // if abstr is not defined then use default abstraction 
   if(!abstr.c_str()){
      abstr = string("BASE");
   }

   /* then create it */
   resource *ret =  new resource(parent->getHandle(),resource_name,
                                 myName,abstr, type);

   resourceHandle r_handle = ret->getHandle() ;
   return(r_handle);
}

pdvector<resourceHandle> *resource::getChildren(bool dontGetRetiredChildren) {
   // set member resource::retired to true when retired
   // query this member variable here

    pdvector<resourceHandle> *temp = new pdvector<resourceHandle>;
    for(unsigned i=0; i < children.size(); i++){
       resource *curRes = resources[children[i]];
       if(dontGetRetiredChildren == true && curRes->isRetired())  continue;
       *temp += curRes->getHandle();      
    }
    return(temp);
}

resourceHandle *resource::findChild(const char *nm) const {
    string temp = nm;
    for(unsigned i=0; i < children.size(); i++){
        if((resources[children[i]])->match(temp)){
	     resourceHandle *h = new resourceHandle;
	     *h = children[i];
	     return(h);
        }
    }
    return(0);  // not found
}


void resource::print()
{
    printf("%s ", name.c_str());
}

void
resource::saveHierarchiesToFile (ofstream& foo)
{
  if (resource::rootResource == NULL) {
    cout << "ROOT IS NULL" << endl;
    return;
  }
  resource *curr = resource::rootResource;
  curr->saveHierarchyToFile(foo);
}  
  
void 
resource::saveHierarchyToFile (ofstream& foo)
{
  resource *curr;
  unsigned childSize = children.size();
  if (childSize == 0) {
    foo << name << endl;
  } else {
    for (unsigned i = 0; i < childSize; i++) {
      curr = resource::handle_to_resource(children[i]);
      curr->saveHierarchyToFile(foo);
    }
  }
}

bool resource::string_to_handle(const string &res, resourceHandle *h) {
    if(allResources.defines(res)){
       resource *temp = allResources[res];
       *h = temp->getHandle();
       return(TRUE);
    }
    else
       return(FALSE);
}

/*
 * Convenience function.
 *
 */
bool resource::isDescendent(resourceHandle child_handle) const {
   resourceHandle root_handle = rootResource->getHandle();
   resourceHandle this_handle = getHandle();
      
   if(CallGraph::isDescendentOfAny(handle_to_resource(child_handle),this))
      return TRUE;

   if (this_handle == child_handle)
      return FALSE;

   if (this_handle == root_handle)
      return TRUE;

   while (child_handle != root_handle) {
      if (child_handle == this_handle) {
         return TRUE;
      } else {
         child_handle = handle_to_resource(child_handle)->getParent();
      }
   }

   return FALSE;
}

// Convenience function:
bool resource::isDescendantOf(const resource &other) const {
   // NOTE: Should merge with the above routine...

   resourceHandle myHandle = getHandle();
   resourceHandle root_handle = rootResource->getHandle();

   if (myHandle == root_handle)
      // the root node is the descendant of noone.
      return false;

   // Keep moving "myHandle" upwards, until it reaches the root
   // It we see "other.getHandle()" along the way, then we return true.
   do {
      myHandle = handle_to_resource(myHandle)->getParent();
      if (myHandle == other.getHandle())
	 return true;
   } while (myHandle != root_handle);

   return false;
}


/*
 * Do the two resources have the same base?
 * Note, since the there is a common root for all nodes,
 * the test for a common base checks the node below the
 * common root.
 */
bool resource::sameRoot(resourceHandle other) const {
  const resource *myBase=0, *otherBase=0, *temp;

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
    resource *res;
    if (resources.find(h, res))
      return res->getName();
    else
      return 0;
}

const char *resource::getFullName(resourceHandle h){
    resource *res;
    if (resources.find(h, res))
      return res->getFullName();
    else
      return 0;
}

resource *resource::string_to_resource(const string &res) {
    if(allResources.defines(res)){
        return(allResources[res]);
    }
    return 0;
}

// get_lib_constraints: returns true if there is a list of lib constraints
// specified by the mdl exclude_lib option.  If the list has not yet been 
// created, this routine creates the list from the mdl_data list
bool resource::get_lib_constraints(pdvector<string> &list, pdvector<unsigned> &flags){
 
    if(!lib_constraints_built) {
        pdvector<string> temp;
		pdvector<unsigned> tmp_flags;
	// create list
        if(mdl_get_lib_constraints(temp, tmp_flags)){

	    for(u_int i=0; i < temp.size(); i++) {
                // if the string is of the form "blah/blah" then this
	        // is a function constraint so don't add it to the
	        // list of lib constraints
		char *next = P_strdup((temp[i].c_str()));
		if(next && (!P_strrchr(next, '/'))){
		    lib_constraints += string(next);
		    lib_constraint_flags += tmp_flags[i];
		}
	        delete next;
	    }
        }
    }
    for(u_int i=0; i < lib_constraints.size(); i++){
            list += lib_constraints[i];
			flags += lib_constraint_flags[i];
    }
    lib_constraints_built = true;
    return lib_constraints.size();
}


// get_func_constraints: returns true if there is a list of func constraints
// specified by the mdl exclude_func option.  If the list has not yet been 
// created, this routine creates the list from the mdl_data list
bool resource::get_func_constraints(pdvector< pdvector<string> > &list, pdvector<unsigned> &flags){
 
    if(!func_constraints_built) {
        pdvector< string > temp;
		pdvector<unsigned> tmp_flags;
	// create list
        if(mdl_get_lib_constraints(temp, tmp_flags)){
	    for(u_int i=0; i < temp.size(); i++){
                // if the string is of the form "blah/blah" then this
	        // is a function constraint so add it to the list 
		char *next = P_strdup((temp[i].c_str()));
		if(next && (P_strrchr(next, '/'))) {
		  u_int where = 0;
		  u_int prev_where = where;
		  for(u_int j=0; j< temp[i].length();j++){
                    if(next[j] == '/'){
		       prev_where = where;
		       where = j+1; 
		    }
		  }
		  assert(where < temp[i].length());
		  assert(where > prev_where);
		  pdvector<string> func_consts;
		  // module name
		  u_int size = where-prev_where;
		  char *temp_str = new char[size]; 
	          if(P_strncpy(temp_str,&(next[prev_where]),size-1)){
		    temp_str[size-1] = '\0';
		    string blah(temp_str);
		    func_consts += blah; 

		    // function name
		    func_consts += string(&(next[where])); 
		    assert(func_consts.size() == 2);
		    func_constraints += func_consts; 

			// constraint flags
			func_constraint_flags += tmp_flags[i];
		  }
		  delete [] temp_str;
		}
	        if(next) delete next;
	    }
        }
    }
    for(u_int i=0; i < func_constraints.size(); i++){
            list += func_constraints[i];
			flags += func_constraint_flags[i];
    }
    func_constraints_built = true;

    return func_constraints.size();
}

bool resource::isStartFunction() {
   CallGraph *cg = CallGraph::FindCallGraph();
   return cg->isStartFunction(getHandle());
}

bool resource::isThreadType(unsigned *tid) {
   if(fullName.size() >= 4) {
      string thr_str = fullName[3];
      if(thr_str.prefixed_by("thr_")) {
	 string thrId_str = thr_str.substr(4, thr_str.length() - 4);
	 int thread_id = atoi(thrId_str.c_str());
	 *tid = thread_id;
	 return true;
      }
   }

   return false;
}

int DMresourceListNameCompare(const void *n1, const void *n2){
    
    const string *s1 = (const string*)n1, *s2 = (const string*)n2;
    if(*s1 > *s2)
       return(1);
    if(*s1 == *s2)
       return(0);
    else
       return(-1);

}

string resource::DMcreateRLname(const pdvector<resourceHandle> &res) {
    // create a unique name
    string temp;
    resource *next;

    pdvector <string> sorted_names;

    for(unsigned i=0; i < res.size(); i++){
	next = resource::handle_to_resource(res[i]);
	sorted_names += next->getFullName();
    }
    sorted_names.sort(DMresourceListNameCompare);

    for(unsigned j=0; j < (res.size() - 1); j++){
	temp += sorted_names[j].c_str();
	temp += ",";
    }
    if(res.size() > 0){
	temp += sorted_names[(res.size()-1)].c_str();
    }
    return(temp);
}

resourceList::resourceList(const pdvector<resourceHandle> &res){
    // create a unique name
    string temp = resource::DMcreateRLname(res);

    //cerr << "resourceList::resourceList(const pdvector<resourceHandle> &res)"
    // << " called" << endl;
    //cerr << " temp (name string) = " << temp << endl;

    if(!allFoci.defines(temp)){
        id = foci.size();
	resourceList *rl = this;
        allFoci[temp] = rl;
        fullName = temp;
        foci += rl;

        // create elements pdvector 
        for(unsigned i=0; i < res.size(); i++){
	    resource *r = resource::handle_to_resource(res[i]);
	    if(r){
	        elements += r;
		if(r->getSuppress()){
                    suppressed = true;
		}
	    }
    } }
    else {
        printf("ERROR: this resourceList already created: %s\n",temp.c_str());
    }
}

// this should be called with strings of fullNames for resources
// ex.  "/Procedure/blah.c/foo"  rather than "foo"
resourceList::resourceList(const pdvector<string> &names){
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
        // create elements pdvector 
        for(unsigned j=0; j < size; j++){
	    resource *r = resource::string_to_resource(names[j]);
	    if(r){
	        elements += r;
		if(r->getSuppress()){
                    suppressed = true;
		}
	    }
        } 
    }
    else {
        printf("ERROR: this resourceList already created: %s\n",temp.c_str());
    }
}


void resourceList::print()
{
    printf("{");
    for (unsigned i=0; i < elements.size(); i++) {
	if (i) printf(" ");
	printf("{");
	elements[i]->print();
	printf("}");
    }
    printf("}");
}

bool resourceList::convertToStringList(pdvector< pdvector<string> > &fs) {
    for (unsigned i=0; i < elements.size(); i++)
        fs += elements[i]->getParts();
    return true;
}

bool resourceList::convertToIDList(pdvector<resourceHandle> &fs) {
    for (unsigned i=0; i < elements.size(); i++){
        fs += elements[i]->getHandle();
    }
    return true;
}

bool resourceList::convertToIDList(resourceListHandle rh,
				   pdvector<resourceHandle> &rl){

    for(u_int i=0; i < foci.size(); i++){
        if(rh == foci[i]->getHandle()){
	    return(foci[i]->convertToIDList(rl));
    }}
    return false;
}

int resourceList::getThreadID() {
   for(unsigned i=0; i<elements.size(); i++) {
      unsigned tid;
      if(elements[i]->isThreadType(&tid) == true) {
	 return tid;
      }
   }
   return -1;
}

// This routine returns a list of foci which are the result of combining
// each child of resource rh with the remaining resources that make up the
// focus, otherwise it returns 0
pdvector<rlNameId> *resourceList::magnify(resourceHandle rh, magnifyType type,
					resource *currentPath){
    pdvector<resourceHandle> *children;
    pdvector<rlNameId> *return_list;

    // supported magnify types....
    assert(type == OriginalSearch || type == CallGraphSearch);

    // check to see if rh is a component of this resourceList
    unsigned not_found_indicator = elements.size();
    unsigned rIndex = not_found_indicator;
    for(unsigned i=0; i < elements.size(); i++){
        if(rh == elements[i]->getHandle()){
            rIndex = i;
	    break;
	}
    }
    if(rIndex == not_found_indicator)  return 0;

    return_list = new pdvector<rlNameId>;

    // calls elements[rIndex]->getChildren or CallGraph::getChildren
    // depending on the magnify type and the characteristics of the
    // resource....
    
#ifdef PCDEBUG
    printf("Calling magnifymanager::getChildren\n");
#endif
      
    children = MagnifyManager::getChildren(elements[rIndex], type);

    if(children->size()){ // for each child create a new focus
       pdvector<resourceHandle> new_focus; 
       for(unsigned i=0; i < elements.size(); i++){
	  new_focus += (elements[i])->getHandle();
       }
       rlNameId temp;
       for(unsigned j=0; j < children->size(); j++){
	  // check to see if this child can be magnified
	  // if so, create a new focus with this child
	  resource *child_res = resource::handle_to_resource((*children)[j]);

	  if(child_res == NULL || child_res->isMagnifySuppressed())
	     continue;

          // don't search down start functions that don't correspond to the
          // thread that we are currently in (if we are in a thread in the
          // search).  For example, don't search "main" start function if
          // we're in a thread since this function wouldn't be the start
          // function for the thread we're in.
	  if(type == CallGraphSearch){
	     if(child_res->isStartFunction()) {
		int tid = getThreadID();

		// if not thread specific, then use every available start func
		if(tid != -1) {
		   CallGraph *cg = CallGraph::FindCallGraph();
		   resource *thrStartFunc = NULL;
		   thrStartFunc = cg->getThreadStartFunc(tid);

		   resourceHandle thrStartFunc_handle = 
		      thrStartFunc->getHandle();
		   resourceHandle child_handle = (*children)[j];
		   if(thrStartFunc_handle != child_handle) {
		      //skip... a start function for a different thread
		      continue;  
		   }
		}
	     }
	  }

	  //Call Graph magnification requests should not return functions
	  //that are in excluded libraries.
	  if(type == CallGraphSearch){
	     assert(currentPath != NULL);
	     resourceHandle parent_handle = child_res->getParent();
	     resource *parent = resource::handle_to_resource(parent_handle);
	     assert(parent);
	     if(!parent->isMagnifySuppressed()){
		//Only return resources that are descendents of the current
		//path
		if(currentPath->isDescendent((*children)[j])){
		   new_focus[rIndex] = (*children)[j];
		   temp.id = resourceList::getResourceList(new_focus);
		   temp.res_name = resource::getName((*children)[j]);
		   *return_list += temp;
		}
	     }
          } 
          
          if(type != CallGraphSearch) {
             new_focus[rIndex] = (*children)[j];
             temp.id = resourceList::getResourceList(new_focus);
             temp.res_name = resource::getName((*children)[j]);
             *return_list += temp; 
          }
       }
       delete children;
       return return_list;
    }
    return 0;
}

// if resource rh is a decendent of a component of the focus, return a new
// focus consisting of rh replaced with it's corresponding entry, 
// otherwise return 0 
//
resourceListHandle *resourceList::constrain(resourceHandle rh){

    unsigned rIndex = elements.size(); 
    for(unsigned i=0; i < elements.size(); i++){
        if(elements[i]->isDescendent(rh)){
	    rIndex = i;
            break;
    }}
    if(rIndex < elements.size()){
	pdvector<resourceHandle> new_focus; 
	for(unsigned j=0; j < elements.size(); j++){
            new_focus += (elements[j])->getHandle();
	}
	new_focus[rIndex] = rh;
	resourceListHandle *new_handle = new resourceListHandle;
	*new_handle = this->getResourceList(new_focus);
	return new_handle;
    }
    return 0;
}

#ifdef ndef
// This routine returns a list of foci each of which is the result of combining
// each child of one of the resources with the remaining resource components of
// the focus. this is iterated over all resources in the focus.
// returns 0 if all resources in the focus have no children
pdvector<resourceListHandle> *resourceList::magnify(magnifyType type){
    
    pdvector<resourceListHandle> *return_list = new pdvector<resourceListHandle>;
    for(unsigned i=0; i < elements.size(); i++){
	pdvector<resourceListHandle> *next = 
				   this->magnify((elements[i])->getHandle(), type);
	if(next) *return_list += *next;
	delete next;
    }
    if(return_list->size()) return return_list;
    return 0;
}
#endif

// This routine returns a list of foci each of which is the result of combining
// each child of one of the resources with the remaining resource components of
// the focus. this is iterated over all resources in the focus.
// returns 0 if all resources in the focus have no children
/*pdvector<rlNameId> *resourceList::magnify(magnifyType type){
    
    pdvector<rlNameId> *return_list = new pdvector<rlNameId>;
    for(unsigned i=0; i < elements.size(); i++){
	pdvector<rlNameId> *next = 
	                 this->magnify((elements[i])->getHandle(), type);
        if(next) {
	  for (unsigned j=0; j < next->size(); j++){
	    *return_list += (*next)[j];
	  }
	  delete next;
	}
      }
    if(return_list->size()) return return_list;

    // memory leak, original was
    // return 0;
    // -matt
    delete return_list;
    return NULL;
    }*/


const char *resourceList::getName(resourceListHandle rh){

    if(rh < foci.size()){
        resourceList *rl = foci[rh];
        return(rl->getName());
    }
    return(NULL);
}


bool resourceList::getMachineNameReferredTo(string &machName) const {
   // If this focus is specific to some machine, then fill in "machName" and return
   // true.  Else, leave "machName" alone and return false.
   // What does it mean for a focus to be specific to a machine?
   // For one, if the focus is a descendant of a machine, then it's obvious.
   // NOTE: If this routine gets confused or isn't sure whether the resource is
   // specific to a machine, it returns false.
   
   // Step 1: Obtain the resources for /Machine and /Process
   // Since these are expensive operations (the string constructor is called,
   // which in turn calls new[]), we only do them once.
   static resource *machine_resource_ptr = NULL; // NULL --> not yet defined

   if (machine_resource_ptr == NULL) {
      machine_resource_ptr = resource::string_to_resource("/Machine");
      if (machine_resource_ptr == NULL) {
         cout << "getMachineNameReferredTo(): couldn't find /Machine" << endl;
         return false;
      }
   }

   assert(machine_resource_ptr);
   const resource &machineResource = *machine_resource_ptr;

   for (unsigned hierarchy=0; hierarchy < elements.size(); hierarchy++) {
      const resource *the_resource_ptr = elements[hierarchy];
      const resource &theResource = *the_resource_ptr;

      // Is "theResource" a descendant of "/Machine"?
      if (theResource.isDescendantOf(machineResource)) {
         // bingo.  Now check out the resource's components.  The machine name
         // should be in the 2d component, and the first component should be "Machine".
	 // (For example, the resource "/Machine/goat" has 2 components)
	 const pdvector<string> &components = theResource.getParts();

         // The following line is not fast; calls string's constructor which calls
	 // malloc.  But it's really just an assert, so we could get rid of it for
	 // speed.
         if (components[0] != "Machine") {
            // I am confused; I expected "Machine"
	    cout << "getMachineNameReferredTo: expected Machine; found "
                 << components[0] << endl;
	    return false;
         }
	 if (components.size() < 2) {
            // I am confused; I expected something below "Machine"
	    cout << "getMachineNameReferredTo: nothing below 'Machine'" << endl;
	    return false;
	 }
	 if (components.size() > 4) {
            // currently, there is only one level below "Machine"
	    // Maybe in the future we can have stuff like "/Machine/cluster1/goat"
	    // But for now this acts as a nice assert (in that if the following error
            // msg is ever seen, then we need to rethink how we extract the machine
	    // name)
	    cout << "getMachineNameReferredTo: too much below 'Machine'" << endl;
	    return false;
	 }

	 // success!
	 machName = components[1];
	 return true;
      }
   }

   return false;
}

pdvector<resourceHandle> *resourceList::getResourceHandles(resourceListHandle h){
    resourceList *focus = getFocus(h);
    if(focus){
        pdvector<resourceHandle> *handles = new pdvector<resourceHandle>;
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
    }
    return 0;


}

resourceListHandle resourceList::getResourceList(
				const pdvector<resourceHandle>& h){

    // does this resourceList already exist?
    string temp = resource::DMcreateRLname(h);
    if(allFoci.defines(temp)){
	resourceList *rl = allFoci[temp];
        return(rl->getHandle());
    }
    // create a new resourceList
    resourceList *res = new resourceList(h);
    assert(res);
    return(res->getHandle());
}

resourceList *resourceList::findRL(const char *name){
    string temp = name;
    if(allFoci.defines(name)){
        return allFoci[name];
    }
    return NULL;
}

void resource::printAllResources() {
    pdvector<resource *>allRes = resource::resources.values();
    for(unsigned i=0; i < allRes.size(); i++){
        cout << "{";
        (allRes[i])->print();
	cout << "}" << endl;
    }
}
