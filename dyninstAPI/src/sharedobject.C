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


#ifndef BPATCH_LIBRARY
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

        vector<pd_Function *> temp = *(getAllFunctions());

        if (filter_excluded_functions(temp, *some_funcs, name) == FALSE) {
	    // WRONG!!!!  Leads to memory leak!!!!
            //  some_funcs = 0;
            // correct :
            delete some_funcs;
            some_funcs = NULL;
            return NULL;
        }
        return some_funcs;
    }
    return NULL;    
} 
#endif /* BPATCH_LIBRARY */






