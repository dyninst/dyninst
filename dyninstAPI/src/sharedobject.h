#if !defined(_shared_object_h)
#define _shared_object_h

#include "util/h/String.h"
#include "dyninstAPI/src/symtab.h"
#include "dyninstAPI/src/symtab.h"
#ifndef BPATCH_LIBRARY
#include "paradynd/src/mdld.h"
#endif

/*
 * A class for link map information about a shared object that is mmapped 
 * by the dynamic linker into the applicaitons address space at runtime. 
 */
#define 	SHAREDOBJECT_NOCHANGE	0
#define 	SHAREDOBJECT_ADDED	1
#define 	SHAREDOBJECT_REMOVED	2

class shared_object {

public:
    shared_object():name(0),base_addr(0),processed(false),
	 mapped(false),include_funcs(true), objs_image(0),some_funcs(0){}
    shared_object(string &n,u_int b, bool p,bool m, bool i, image *d):
	name(n), base_addr(b),processed(p),mapped(m),
	include_funcs(i), objs_image(d),some_funcs(0){ }
    shared_object(const shared_object &s_obj){
	name = s_obj.name;
	base_addr = s_obj.base_addr;
	processed = s_obj.processed;
	mapped = s_obj.mapped;
	include_funcs = s_obj.include_funcs;
	objs_image = s_obj.objs_image;
	some_funcs = s_obj.some_funcs;
    }
    ~shared_object(){ objs_image = 0;}

    const string &getName(){ return(name); }
    u_int getBaseAddress() { return(base_addr); }
    bool  isProcessed() { return(processed); }
    bool  isMapped() { return(mapped); }
    const image  *getImage() { return(objs_image); }
    u_int getImageId(){ return((u_int)objs_image); }
    bool includeFunctions(){ return(include_funcs); }
    void changeIncludeFuncs(bool flag){ include_funcs = flag; } 

    void  unMapped(){ mapped = false; }
    void  setBaseAddress(u_int new_ba){ base_addr = new_ba; }

    bool  getSymbolInfo(const string &n,Symbol &info) {
        if(objs_image) {
	    return (objs_image->symbol_info(n,info));
	}
	return false;
    }

    vector<pd_Function *> *getAllFunctions(){
        if(objs_image) {
	    // previously objs_image->mdlNormal....
	    return (&(objs_image->getAllFunctions()));
	}
	return 0;
    }

    // from a string that is a complete path name to a function in a module
    // (ie. "/usr/lib/libc.so.1/write") return a string with the function
    // part removed.  return 0 on error
    char *getModulePart(string &full_path_name) ;

#ifndef BPATCH_LIBRARY
    // get only the functions not excluded by the mdl options exclude_lib
    // or exclude_funcs
    vector<pd_Function *> *getSomeFunctions();
#endif

    // Get list of ALL modules, not just included ones.
    vector<pdmodule *> *getModules() {
        if(objs_image) {
	    return (&(objs_image->getAllModules()));
	}
	return 0;
    }

    bool  addImage(image *i){ 
	if(!processed && (objs_image == 0)) {
	    objs_image = i;
	    processed = true;
            return true;
	}
	else {
            return false;
	}
    }
    bool removeImage(){ return true;}

    pd_Function *findOneFunction(string f_name,bool check_excluded){
	if (f_name.string_of() == 0) return 0;
        if(objs_image) {
#ifndef BPATCH_LIBRARY
	    if(check_excluded){
		// only search the some_funcs list
		if(!some_funcs) getSomeFunctions();
		if(some_funcs) {
		    for(u_int i=0; i < some_funcs->size(); i++){
			if(((*some_funcs)[i])->prettyName() == f_name){
			    return (*some_funcs)[i];
			}
		    }
		    return 0;
		}
	    }
#endif
            return (objs_image->findOneFunction(f_name));
	}
	return 0;
    }

    pd_Function *findOneFunctionFromAll(string f_name,bool check_excluded){
	if (f_name.string_of() == 0) return 0;
        if(objs_image) {
#ifndef BPATCH_LIBRARY
	    if(check_excluded){
		// only search the some_funcs list
		if(!some_funcs) getSomeFunctions();
		if(some_funcs) {
		    for(u_int i=0; i < some_funcs->size(); i++){
			if(((*some_funcs)[i])->prettyName() == f_name){
			    return (*some_funcs)[i];
			}
		    }
		    return 0;
		}
	    }
#endif
            return (objs_image->findOneFunctionFromAll(f_name));
	}
	return 0;
    }


    pdmodule *findModule(string m_name,bool check_excluded){
        if(objs_image) {
	    if(check_excluded && !include_funcs){
		return 0;
	    }
            return (objs_image->findModule(m_name));
	}
	return 0;
    }
    pd_Function *findFunctionIn(Address adr,const process *p){
        if((adr >= base_addr) && objs_image){
            Address new_adr = adr - base_addr;
            return(objs_image->findFunctionIn(new_adr,p));
	}
	return (0);
    }
    pd_Function *findFunctionInInstAndUnInst(Address adr,const process *p){
        if((adr >= base_addr) && objs_image){
            Address new_adr = adr - base_addr;
            return(objs_image->findFunctionInInstAndUnInst(new_adr,p));
	}
	return (0);
    }


    //
    //     PRIVATE DATA MEMBERS
    //				
private:
    string  name;	// full file name of the shared object
    u_int   base_addr;  // base address of where the shared object is mapped
    bool    processed;  // if true, daemon has processed the shared obj. file
    bool    mapped;     // if true, the application has the shared obj. mapped
			// shared objects can be unmapped as the appl. runs
    bool include_funcs; // if true include the the functions from this shared
			// object in the set of all instrumentable functions
			// (this is for foci not refined on the Code heirarchy)
			// - Conceptually assumes that shared object has 1
			// and only 1 module.
    image  *objs_image; // pointer to image if processed is true 
    vector<pd_Function *> *some_funcs; // all functions not excluded by 
				       // exclude_func option
};

#endif
