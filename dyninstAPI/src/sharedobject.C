#include "dyninstAPI/src/sharedobject.h"

// TODO: this should probably not be a shared_object method, but since
// for now it is only used by shared_objects it is
// from a string that is a complete path name to a function in a module
// (ie. "/usr/lib/libc.so.1/write") return a string with the function
// part removed.  return 0 on error
char *shared_object::getModulePart(string &full_path_name) {

    char *whole_name = P_strdup(full_path_name.string_of());
    char *next=0;
    char *last=next;
    if((last = P_strrchr(whole_name, '/'))){
        next = whole_name;
        for(u_int i=0;(next!=last)&&(i<full_path_name.length()); i++){
	    next++;
	    if(next == last){
		u_int size = i+2;
	        char *temp_str = new char[size];
	        if(P_strncpy(temp_str,whole_name,size-1)){
                    temp_str[size-1] = '\0';
		    delete whole_name;
		    return temp_str;
		    temp_str = 0;
	    } }
        }
    }
    delete whole_name;
    return 0;
}


// returns all the functions not excluded by exclude_lib or exclude_func
// mdl option
vector<pd_Function *> *shared_object::getSomeFunctions(){
    if(objs_image) {
      if(some_funcs) return some_funcs;
      // return (&(objs_image->mdlNormal));
      // check to see if this module occurs in the list of modules from
      // exclude_funcs mdl option...if it does then we need to check
      // function by function
      some_funcs = new vector<pd_Function *>;
      *some_funcs +=  *(getAllFunctions());
      vector<string> func_constraints;

      if(mdl_get_lib_constraints(func_constraints)) {
	for(u_int i=0; i < func_constraints.size(); i++) {

	  // if this is not a lib constraint of the form "module/function" 
	  // then this is one that specifies an entire library and  
	  // we ignore it here
	  char *blah = 0;
	  char *next = P_strdup(func_constraints[i].string_of());
          if(next && (blah = P_strchr(next,'/'))) {
	    char *mod_part = getModulePart(func_constraints[i]);
	    char *mod_name = P_strdup(name.string_of());
	    if(mod_name && mod_part && (P_strstr(mod_name,mod_part))){
	        // find corresponding function in the list of
	        // some_funcs and remove it
	        char *temp = P_strdup(func_constraints[i].string_of());
	        char *func_name = P_strrchr(temp,'/'); 
	        func_name++;
	        if(func_name){
	           string blah(func_name);
		   pd_Function *pf = findOneFunction(blah,false);
		   if(pf) {
		     // remove this function from the somefunctions list
		     u_int size = some_funcs->size();
		     for(u_int j=0; j < size; j++) {
		        if(pf == (*some_funcs)[j]){
		          (*some_funcs)[j] = (*some_funcs)[size - 1];
			  some_funcs->resize(size - 1);
			  break;
		   } } }
	        }
	    }
	    if(mod_name) delete mod_name;
	    if(mod_part) delete mod_part;
          }
	  if(next) free(next); // strdup allocs via malloc, so we use free() here

      } }

      return some_funcs;
    }
    some_funcs = 0;
    return 0;
}
