#if !defined(_shared_object_h)
#define _shared_object_h

#include "util/h/String.h"
#include "paradynd/src/symtab.h"

/*
 * A class for link map information about a shared object that is mmapped 
 * by the dynamic linker into the applicaitons address space at runtime. 
 */

class shared_object {

public:
    shared_object():name(0),base_addr(0),size(0),processed(false),
		 mapped(false),include_funcs(true), objs_image(0){}
    shared_object(string &n,u_int b, u_int s, bool p,bool m, bool i, image *d):
		name(n), base_addr(b),size(s),processed(p),mapped(m),
		include_funcs(i), objs_image(d){ }
    ~shared_object(){}

    const string &getName(){ return(name); }
    u_int getBaseAddress() { return(base_addr); }
    bool  isProcessed() { return(processed); }
    bool  isMapped() { return(mapped); }
    const image  *getImage() { return(objs_image); }
    u_int getSize(){ return(size);}
    u_int getImageId(){ return((u_int)objs_image); }
    bool includeFunctions(){ return(include_funcs); }
    void changeIncludeFuncs(bool flag){ include_funcs = flag; } 

    void  unMapped(){ mapped = false; }
    void  setBaseAddress(u_int new_ba){ base_addr = new_ba; }

    bool  getSymbolInfo(string &n,Symbol &info) {
        if(objs_image) {
	    return (objs_image->symbol_info(n,info));
	}
	return false;
    }

    vector<pdFunction *> *getAllFunctions(){
        if(objs_image) {
	    return (&(objs_image->mdlNormal));
	}
	return 0;
    }

    vector<module *> *getModules() {
        if(objs_image) {
	    return (&(objs_image->mods));
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

    pdFunction *findOneFunction(string f_name){
        if(objs_image) {
            return (objs_image->findOneFunction(f_name));
	}
	return 0;
    }
    module *findModule(string m_name){
        if(objs_image) {
            return (objs_image->findModule(m_name));
	}
	return 0;
    }

				
private:
    string  name;	// full file name of the shared object
    u_int   base_addr;  // base address of where the shared object is mapped
    u_int   size; 	//  size of object
    bool    processed;  // if true, daemon has processed the shared obj. file
    bool    mapped;     // if true, the application has the shared obj. mapped
			// shared objects can be unmapped as the appl. runs
    bool include_funcs; // if true include the the functions from this shared
			// object in the set of all instrumentable functions
			// (this is for foci not refined on the Code heirarchy)
    image  *objs_image; // pointer to image if processed is true 
};

#endif
