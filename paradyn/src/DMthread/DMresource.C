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

// $Id: DMresource.C,v 1.68 2004/06/21 19:37:11 pcroth Exp $

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
#include "pdutil/h/mdlParse.h"
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
resource::resource( void )
  : flatName( "ROOT" ),
    type( CategoryResourceType ),
    mdlType( MDL_T_STRING ),
    res_handle( 0 ),
    parent( NULL ),
    suppressSearch( false ),
    suppressChildSearch( false ),
    suppressMagnify( false ),
    retired( false ),
    abstr( NULL )
{
    assert( allResources.size()==0 );
    assert( !allResources.defines( flatName ) );

    // register ourselves as a new resource
    resources[res_handle] = this;
    allResources[flatName] = this;
}


//
// used to construct all non-root resources
//
resource::resource( pdstring _resName,
                    ResourceType _resType,
                    unsigned int _mdlType,
                    resource* _parent,
                    unsigned int _id )
  : type( _resType ),
    mdlType( _mdlType ),
    res_handle( _id ),
    parent( _parent ),
    suppressSearch( _parent->getSuppressChildren() ),
    suppressChildSearch( suppressSearch ),
    suppressMagnify( false ),
    retired( false ),
    abstr( NULL )
{
    assert( parent != NULL );

    // ensure our names are set...
    // ...flat name...
    // Note: the root resource has a special name ("ROOT")
    // that we do not want to include in our flat name
    flatName = ((parent != rootResource) ?
                            parent->getFullName() : pdstring("")) +
                         "/" + _resName;
    assert( !allResources.defines( flatName ) );

    // ...and full (vector-based) name.
    // (Note: we build the full name into a temporary in reverse order
    // and then swap the order because we have neither the push_front()
    // nor the insert() method in our vector class.)
    assert( fullName.size() == 0 );
    pdvector<pdstring> reverseFullName;
    reverseFullName.push_back( _resName );
    resource* currPart = _parent;
    while( currPart != rootResource )
    {
        reverseFullName.push_back( currPart->getName() );
        currPart = currPart->parent;
    }
    for( unsigned int i = reverseFullName.size(); i > 0; i-- )
    {
        fullName.push_back( reverseFullName[i-1] );
    }

    // the daemons generate an id for the resource. If there are no
    // conflicts between this id and the id for other resource, we
    // can keep the daemon id, otherwise we must generate a new id.
    if( resources.defines(_id) )
    {
       res_handle = newResourceHandle();
    }
     
    // register ourselves as a new resource
    allResources[flatName] = this;
    resources[res_handle] = this;
    parent->AddChild( this );
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

resource*
resource::create( const pdvector<pdstring>& resource_name,
                        ResourceType type,
                        unsigned int mdlType,
                        unsigned int resId )
{
    // determine the proposed resource's parent based on 
    // the given resource name
    resource* parent = NULL;
    if( resource_name.size() > 1 )
    {
        // construct the parent's flat name
        pdstring parentFlatName;
        for( unsigned int i = 0; i < resource_name.size() - 1; i++ )
        {
            parentFlatName += "/";
            parentFlatName += resource_name[i];
        }

        // look up the parent
        parent = resource::string_to_resource( parentFlatName );
        if( parent == NULL )
        {
            cerr << "FE: parent NULL when trying to define resource: name =\n ";
            for( unsigned int i = 0; i < resource_name.size(); i++ )
            {
                cerr << "\t" << resource_name[i] << endl;
            }
            cerr << "\tparent name = " << parentFlatName
                << endl;
        }
        assert( parent != NULL );
    }
    else if( resource_name.size() == 1 )
    {
        // this is a top-level resource
        parent = resource::rootResource;
    }
    else
    {
        // we are being asked to create the root resource -
        // this should never happen
        assert( resource_name.size() >= 1 );
    }
    assert( parent != NULL );

    // extract the proposed resource's own name
    pdstring resName = resource_name[resource_name.size()-1];

    // create the resource
    return resource::create( resName, type, mdlType, parent, resId );
}

resource*
resource::create( pdstring resFullName,
                        ResourceType type,
                        unsigned int mdlType,
                        unsigned int resId )
{
    // find resource's parent based on its given full name
    resource* parent = NULL;

    // split its full name between its parent name and its own name
    pdstring name;
    const char* resFullNamePtr = resFullName.c_str();
    assert( resFullNamePtr[0] == '/' );
    const char* lastSepPtr = strrchr( resFullNamePtr, '/' );
    assert( lastSepPtr != NULL );

    if( lastSepPtr != resFullNamePtr )
    {
        pdstring parName = resFullName.substr( 0, (lastSepPtr - resFullNamePtr) );

        // it is not a child of the root resource
        // find its parent
        parent = resource::string_to_resource(parName);
    }
    else
    {
        // it is a child of the root resource
        parent = resource::getRootResource();
    }
    name = (lastSepPtr+1);
    assert( parent != NULL );
    assert( name.length() > 0 );

    return resource::create( name, type, mdlType, parent, resId );
}

resource*
resource::create( pdstring resName,
                    ResourceType type,
                    unsigned int mdlType,
                    resource* parent,
                    unsigned int resId )
{
    resource* ret = NULL;


    // check to see if the resource already exists
    // Note: the root resource has a special name ("ROOT")
    // that we don't want to include in our flat name
    pdstring resFlatName = ((parent != rootResource) ?
                            parent->getFullName() : pdstring("")) +
                         "/" + resName;
    if( resource::allResources.find( resFlatName, ret ) )
    {
        return ret;
    }

    // the resource does not already exist, so create it
    ret = new resource( resName, type, mdlType, parent, resId );

    // check to see if the suppressMagnify option should be set...if
    // this resource is specifed in the mdl exclude_lib option
    pdvector<pdstring> shared_lib_constraints;
    pdvector<unsigned> constraint_flags;
    if(resource::get_lib_constraints(shared_lib_constraints, constraint_flags) &&
       (parent->getFullName() == "/Code")) {
        for(u_int i=0; i < shared_lib_constraints.size(); i++){

            // grab the exclude flags
            bool checkCase =
                (constraint_flags[i] & LIB_CONSTRAINT_NOCASE_FLAG) == 0;
            bool regex =
                (constraint_flags[i] & LIB_CONSTRAINT_REGEX_FLAG ) != 0;

            // A regular expression will match any location within the string,
            // unless otherwise specified with ^ and $
            if( regex )
            {
                shared_lib_constraints[i] = "^" +
                                            shared_lib_constraints[i] + "$";
            }

            // By default (!regex), check using wildcardEquiv, if the REGEX flag is 
            // set, then use regexEquiv, passing the NOCASE flag as needed

            if( ( regex &&
                    shared_lib_constraints[i].regexEquiv( ret->getName(),
                        checkCase ) ) ||
                (!regex &&
                    shared_lib_constraints[i].wildcardEquiv( ret->getName(),
                        checkCase ) ) )
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

    // check to see if the suppressMagnify option should be set if
    // the resource is a function that is specified in the mdl exclude_func
    // options
    if(!ret->isMagnifySuppressed()){
      if(parent != resource::rootResource) {
      // get parent of parent, if it is "/Code" then check the 
      // list of exculded_funcs
      resource* ppr = parent->getParent();
      if( ppr && (ppr->getFullName() == "/Code")) {
          pdvector< pdvector<pdstring> > libs;
          constraint_flags.resize( 0 );
          if(resource::get_func_constraints(libs, constraint_flags)) {
              for(u_int i=0; i < libs.size(); i++){

                  // grab the exclude flags
                  bool checkCase = 
                    (constraint_flags[i] & LIB_CONSTRAINT_NOCASE_FLAG) == 0;
                  bool regex =
                    (constraint_flags[i] & LIB_CONSTRAINT_REGEX_FLAG ) != 0;

                  // By default (!regex), check using wildcardEquiv, if the REGEX flag is
                  // set, then use regexEquiv, passing the NOCASE flag as needed
                  if( regex ) {
                      // A regular expression will match any location within the string,
                      // unless otherwise specified with ^ and $
                      (libs[i])[0] = "^" + (libs[i])[0] + "$";
                      (libs[i])[1] = "^" + (libs[i])[1] + "$";
                  }

                  if(  ( regex &&
                         ( (libs[i])[0].regexEquiv( parent->getName(), checkCase )) &&
                         ( (libs[i])[1].regexEquiv( ret->getName(), checkCase )) )
                   || ( !regex &&
                         ( (libs[i])[0].wildcardEquiv( parent->getName(), checkCase )) &&
                         ( (libs[i])[1].wildcardEquiv( ret->getName(), checkCase )) ) )
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
    pdstring name = ret->getFullName();
    while(allS.next(h,ps)){
        ps->callResourceFunc(parent->getHandle(),
                                ret->getHandle(),
                                ret->getFullName().c_str(),
                                ret->getAbstractionName());
    }

    return ret;
}

resource*
resource::getRootResource( void )
{
    if( rootResource == NULL )
    {
        rootResource = new resource();
    }
    assert( rootResource != NULL );
    return rootResource;
}



pdvector<const resource*>
resource::getChildren(bool dontGetRetiredChildren) const
{
   // set member resource::retired to true when retired
   // query this member variable here

    pdvector<const resource*> ret;
    for(unsigned i=0; i < children.size(); i++){
       resource *curRes = children[i];
       if(dontGetRetiredChildren == true && curRes->isRetired())  continue;
       ret.push_back( curRes );
    }
    return ret;
}

resourceHandle *resource::findChild(const char *nm) const {
    pdstring temp = nm;
    for(unsigned i=0; i < children.size(); i++){
        if(children[i]->match(temp)){
	     resourceHandle *h = new resourceHandle;
	     *h = children[i]->getHandle();
	     return(h);
        }
    }
    return(0);  // not found
}


void resource::print()
{
    printf("%s ", flatName.c_str());
}

void
resource::saveHierarchiesToFile(std::ofstream& foo)
{
  if (resource::rootResource == NULL) {
    cout << "ROOT IS NULL" << endl;
    return;
  }
  resource *curr = resource::rootResource;
  curr->saveHierarchyToFile(foo);
}  
  
void 
resource::saveHierarchyToFile(std::ofstream& foo)
{
  resource *curr;
  unsigned childSize = children.size();
  if (childSize == 0) {
    foo << flatName << endl;
  } else {
    for (unsigned i = 0; i < childSize; i++) {
      curr = children[i];
      curr->saveHierarchyToFile(foo);
    }
  }
}

bool resource::string_to_handle(const pdstring &res, resourceHandle *h) {
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
bool resource::isDescendant(const resource* child) const {

    const resource* root = getRootResource();

    if(CallGraph::isDescendantOfAny(child,this))
      return true;

    if (child == this)
      return false;

    if (this == root)
      return true;

    while (child != root) {
      if (child == this) {
         return true;
      } else {
         child = child->getParent();
      }
    }

    return false;
}

// Convenience function:
bool resource::isDescendantOf(const resource* other) const
{
    const resource* curr = this;
    const resource* root = getRootResource();

    if (this == root)
    {
        // the root node is the descendant of noone.
        return false;
    }

    // Keep moving "curr" upwards, until it reaches the root
    // It we see "other.getHandle()" along the way, then we return true.
    do {
        curr = curr->getParent();
        if (curr == other)
        {
            return true;
        }
    } while (curr != root);

    return false;
}


/*
 * Do the two resources have the same base?
 * Note, since the there is a common root for all nodes,
 * the test for a common base checks the node below the
 * common root.
 */
bool resource::sameRoot(const resource* other) const {
  const resource *myBase=0, *otherBase=0, *temp;

  temp = this;
    const resource* root = getRootResource();
  while (temp != root) {
    myBase = temp;
    temp = temp->parent;
  }
  temp = other;
  while (temp != root) {
    otherBase = temp;
    temp = temp->parent;
  }

    return (myBase == otherBase);
}

pdstring resource::getName(resourceHandle h){
    resource *res;
    if (resources.find(h, res))
      return res->getName();
    else
      return "";
}

pdstring resource::getFullName(resourceHandle h){
    resource *res;
    if (resources.find(h, res))
      return res->getFullName();
    else
      return "";
}

resource *resource::string_to_resource(const pdstring &res) {
    if(allResources.defines(res)){
        return(allResources[res]);
    }
    return NULL;
}

// get_lib_constraints: returns true if there is a list of lib constraints
// specified by the mdl exclude_lib option.  If the list has not yet been 
// created, this routine creates the list from the mdl_data list
bool resource::get_lib_constraints(pdvector<pdstring> &list, pdvector<unsigned> &flags){
 
    if(!lib_constraints_built) {
        pdvector<pdstring> temp;
		pdvector<unsigned> tmp_flags;
	// create list
        if(mdl_get_lib_constraints(temp, tmp_flags)){

	    for(u_int i=0; i < temp.size(); i++) {
                // if the pdstring is of the form "blah/blah" then this
	        // is a function constraint so don't add it to the
	        // list of lib constraints
		char *next = P_strdup((temp[i].c_str()));
		if(next && (!P_strrchr(next, '/'))){
		    lib_constraints += pdstring(next);
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
bool resource::get_func_constraints(pdvector< pdvector<pdstring> > &list, pdvector<unsigned> &flags){
 
    if(!func_constraints_built) {
        pdvector< pdstring > temp;
		pdvector<unsigned> tmp_flags;
	// create list
        if(mdl_get_lib_constraints(temp, tmp_flags)){
	    for(u_int i=0; i < temp.size(); i++){
                // if the pdstring is of the form "blah/blah" then this
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
		  pdvector<pdstring> func_consts;
		  // module name
		  u_int size = where-prev_where;
		  char *temp_str = new char[size]; 
	          if(P_strncpy(temp_str,&(next[prev_where]),size-1)){
		    temp_str[size-1] = '\0';
		    pdstring blah(temp_str);
		    func_consts += blah; 

		    // function name
		    func_consts += pdstring(&(next[where])); 
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

bool resource::isStartFunction( void ) const {
   CallGraph *cg = CallGraph::FindCallGraph();
   return cg->isStartFunction(getHandle());
}

bool resource::isThreadType(unsigned *tid) const {
   if(fullName.size() >= 4) {
      pdstring thr_str = fullName[3];
      if(thr_str.prefixed_by("thr_")) {
	 pdstring thrId_str = thr_str.substr(4, thr_str.length() - 4);
	 int thread_id = atoi(thrId_str.c_str());
	 *tid = thread_id;
	 return true;
      }
   }

   return false;
}

int DMresourceListNameCompare(const void *n1, const void *n2){
    
    const pdstring *s1 = (const pdstring*)n1, *s2 = (const pdstring*)n2;
    if(*s1 > *s2)
       return(1);
    if(*s1 == *s2)
       return(0);
    else
       return(-1);

}

pdstring resource::DMcreateRLname(const pdvector<resourceHandle> &res) {
    // create a unique name
    pdstring temp;
    resource *next;

    pdvector <pdstring> sorted_names;

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
    pdstring temp = resource::DMcreateRLname(res);

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
resourceList::resourceList(const pdvector<pdstring> &names){
    // create a unique name
    unsigned size = names.size();
    pdstring temp;
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

bool resourceList::convertToStringList(pdvector< pdvector<pdstring> > &fs) {
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
pdvector<rlNameId> *resourceList::magnify(resource* res, magnifyType type,
					resource *currentPath){
    pdvector<rlNameId> *return_list;

    // supported magnify types....
    assert(type == OriginalSearch || type == CallGraphSearch);

    // check to see if rh is a component of this resourceList
    unsigned not_found_indicator = elements.size();
    unsigned rIndex = not_found_indicator;
    for(unsigned i=0; i < elements.size(); i++){
        if( res->getHandle() == elements[i]->getHandle()){
            rIndex = i;
            break;
        }
    }
    if(rIndex == not_found_indicator)  return NULL;

    return_list = new pdvector<rlNameId>;

    // calls elements[rIndex]->getChildren or CallGraph::getChildren
    // depending on the magnify type and the characteristics of the
    // resource....
    
#ifdef PCDEBUG
    printf("Calling magnifymanager::getChildren\n");
#endif
      
    pdvector<const resource*> children = 
        MagnifyManager::getChildren(elements[rIndex], type);

    if(children.size()){ // for each child create a new focus
       pdvector<resourceHandle> new_focus; 
       for(unsigned i=0; i < elements.size(); i++){
            new_focus.push_back( (elements[i])->getHandle() );
       }
       rlNameId temp;
       for(unsigned j=0; j < children.size(); j++){
	  // check to see if this child can be magnified
	  // if so, create a new focus with this child
        const resource* child_res = children[j];

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
		   resourceHandle child_handle = children[j]->getHandle();
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
            resource* parent = child_res->getParent();
	        assert(parent);
	     if(!parent->isMagnifySuppressed()){
		//Only return resources that are descendents of the current
		//path
		if(currentPath->isDescendant(children[j])){
		   new_focus[rIndex] = children[j]->getHandle();
		   temp.id = resourceList::getResourceList(new_focus);
		   temp.res_name = children[j]->getName();
		   *return_list += temp;
		}
	     }
          } 
          
          if(type != CallGraphSearch) {
             new_focus[rIndex] = children[j]->getHandle();
             temp.id = resourceList::getResourceList(new_focus);
             temp.res_name = children[j]->getName();
             return_list->push_back( temp );
          }
       }
       return return_list;
    }
    return NULL;
}

// if resource rh is a decendent of a component of the focus, return a new
// focus consisting of rh replaced with it's corresponding entry, 
// otherwise return 0 
//
resourceListHandle *resourceList::constrain(resource* res)
{
    unsigned rIndex = elements.size(); 
    for(unsigned i=0; i < elements.size(); i++){
        if(elements[i]->isDescendant( res )){
	    rIndex = i;
            break;
    }}
    if(rIndex < elements.size()){
	pdvector<resourceHandle> new_focus; 
	for(unsigned j=0; j < elements.size(); j++){
            new_focus += (elements[j])->getHandle();
	}
	new_focus[rIndex] = res->getHandle();
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


bool resourceList::getMachineNameReferredTo(pdstring &machName) const {
   // If this focus is specific to some machine, then fill in "machName" and return
   // true.  Else, leave "machName" alone and return false.
   // What does it mean for a focus to be specific to a machine?
   // For one, if the focus is a descendant of a machine, then it's obvious.
   // NOTE: If this routine gets confused or isn't sure whether the resource is
   // specific to a machine, it returns false.
   
   // Step 1: Obtain the resources for /Machine and /Process
   // Since these are expensive operations (the pdstring constructor is called,
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

   for (unsigned hierarchy=0; hierarchy < elements.size(); hierarchy++) {
      const resource *the_resource_ptr = elements[hierarchy];
      const resource &theResource = *the_resource_ptr;

      // Is "theResource" a descendant of "/Machine"?
      if (theResource.isDescendantOf(machine_resource_ptr)) {
         // bingo.  Now check out the resource's components.  The machine
         // name should be in the 2d component, and the first component
         // should be "Machine".  (For example, the resource "/Machine/goat"
         // has 2 components)
         const pdvector<pdstring> &components = theResource.getParts();
         
         // The following line is not fast; calls string's constructor which
         // calls malloc.  But it's really just an assert, so we could get
         // rid of it for speed.
         if (components[0] != "Machine") {
            // I am confused; I expected "Machine"
            cout << "getMachineNameReferredTo: expected Machine; found "
                 << components[0] << endl;
            return false;
         }
         if (components.size() < 2) {
            // I am confused; I expected something below "Machine"
            cout << "getMachineNameReferredTo: nothing below 'Machine'"
                 << endl;
            return false;
         }
         if (components.size() > 4) {
            // currently, there is only one level below "Machine" Maybe in
            // the future we can have stuff like "/Machine/cluster1/goat" But
            // for now this acts as a nice assert (in that if the following
            // error msg is ever seen, then we need to rethink how we extract
            // the machine
            // name)
            cout << "getMachineNameReferredTo: too much below 'Machine'"
                 << endl;
            return false;
         }
         
         // success!
         machName = components[1];
         return true;
      }
   }

   return false;
}

// returns true on success, false if it failed to fill in procName and pid
// will fill in procName and pid if the given pointers are non-null
bool resource::splitProcessResourceStr(const pdstring &proc_res_str,
                                       pdstring *procName, int *pid)
{
   const char *fullstr = proc_res_str.c_str();

   char process_name[200];
   int cur_index = 0;

   // fill in process_name and move up to the '{' character
   for(cur_index = 0; ; cur_index++) {
      char cur_char = fullstr[cur_index];
      if(cur_char == 0)
         return false;
      if(cur_char == '{')
         break;      

      process_name[cur_index] = cur_char;
   }

   process_name[cur_index] = 0;
   if(procName != NULL)
      (*procName) = process_name;

   cur_index++;   // get past the '{'
   char pid_str[20];
   int pid_index = 0;

   // get the pid string and move up to the '}'
   while(1) {
      char cur_char = fullstr[cur_index];
      if(cur_char == 0)
         return false;
      if(cur_char == '}')
         break;

      pid_str[pid_index] = cur_char;
      pid_index++;
      cur_index++;
   };

   pid_str[pid_index] = 0;
   if(pid != NULL)
      (*pid) = atoi(pid_str);

   return true;
}

// returns true if resourceList does specify a process
// will fill in procName and pid if the given pointers are non-null
bool resourceList::getProcessReferredTo(pdstring *procName, int *pid) const {
   // If this focus is specific to some process, then fill in "procName" and
   // "pid" and return true.  Else, leave "procName" and "pid" alone and
   // return false.
   
   // Step 1: Obtain the resources for /Machine and /Process
   // Since these are expensive operations (the pdstring constructor is called,
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

   for (unsigned hierarchy=0; hierarchy < elements.size(); hierarchy++) {
      const resource *the_resource_ptr = elements[hierarchy];
      const resource &theResource = *the_resource_ptr;

      // Is "theResource" a descendant of "/Machine"?
      if (theResource.isDescendantOf(machine_resource_ptr)) {
         // bingo.  Now check out the resource's components.  The machine
         // name should be in the 2d component, and the first component
         // should be "Machine".  (For example, the resource "/Machine/goat"
         // has 2 components)
         const pdvector<pdstring> &components = theResource.getParts();
         
         // The following line is not fast; calls string's constructor which
         // calls malloc.  But it's really just an assert, so we could get
         // rid of it for speed.
         if (components[0] != "Machine") {
            // I am confused; I expected "Machine"
            cout << "getMachineNameReferredTo: expected Machine; found "
                 << components[0] << endl;
            return false;
         }
         if (components.size() < 2) {
            // I am confused; I expected something below "Machine"
            cout << "getMachineNameReferredTo: nothing below 'Machine'"
                 << endl;
            return false;
         }
         if (components.size() > 4) {
            // currently, there is only one level below "Machine" Maybe in
            // the future we can have stuff like "/Machine/cluster1/goat" But
            // for now this acts as a nice assert (in that if the following
            // error msg is ever seen, then we need to rethink how we extract
            // the machine
            // name)
            cout << "getMachineNameReferredTo: too much below 'Machine'"
                 << endl;
            return false;
         }

         if(components.size() == 2) {
            // for example: /Machine and cham
            return false;  // machine referred to, but not process
         }

         // success!         
         bool result = resource::splitProcessResourceStr(components[2],
                                                         procName, pid);
         return result;
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

const resourceListHandle *resourceList::find(const pdstring &name){

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
    pdstring temp = resource::DMcreateRLname(h);
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
    pdstring temp = name;
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
