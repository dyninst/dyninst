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

#include "paradynd/src/sharedobject.h"
#include "paradynd/src/solarisDL.h"
#include "paradynd/src/process.h"
#include "paradynd/src/symtab.h"

#include <link.h>
    
// findDynamicLinkingInfo: This routine is called on exit point of 
// of the exec system call. It checks if the a.out is dynamically linked,
// and if so, it sets the value of _DYNAMIC[DT_DEBUG].d_un.dptr to a non-zero
// value. This will tell the run-time linker to export mapping information
// about the shared objects it dynamically loads.  This has to be done before
// the dynamic linker first executes (at exit point of exec).
// version 2.6 of Solaris will have a beter interface to this information

bool dynamic_linking::findDynamicLinkingInfo(process *p){

    string dyn_str = string("DYNAMIC");
    internalSym *dyn_sym = p->findInternalSymbol(dyn_str,true);

    // find and set the value of _DYNAMIC[DT_DEBUG].d_un.dptr to 1 
    if(dyn_sym){

        p->setDynamicLinking();
	// get the DT_DEBUG entry of the _DYNAMIC array
        Elf32_Dyn next_elm;
	Address next_addr = dyn_sym->getAddr(); 
	Address dyn_debug_addr = 0;
	if(!p->readDataSpace((caddr_t) (next_addr), 
		sizeof(Elf32_Dyn),(caddr_t)&(next_elm),true)) {
	   logLine("errror in read next_elm\n");
        }
	while(next_elm.d_tag != DT_NULL){
	    if(next_elm.d_tag == DT_DEBUG){
	        dyn_debug_addr = next_addr;	
		break;
	    }
	    next_addr += sizeof(Elf32_Dyn); 
	    if(!p->readDataSpace((caddr_t) (next_addr), 
		sizeof(Elf32_Dyn),(caddr_t)&(next_elm),true)) {
	        logLine("errror in read next_elm\n");
		return false;
            }
	}

	// compute address of DYNAMIC[DT_DEBUG].d_un.d_ptr
	// and set its value
	link_map_addr = dyn_debug_addr + sizeof(Elf32_Sword);
	if(!(p->getImage()->isValidAddress(link_map_addr))){
	   return false;
	}
	int new_value = 1;

	if(!p->writeDataSpace((caddr_t)(link_map_addr),
				 sizeof(int), (caddr_t)&new_value)){
	   logLine("write DYNAMIC[DT_DEBUG].d_ptr falied\n");
	   return false;
	}	

	// check the value
	if(!p->readDataSpace((caddr_t)(link_map_addr),
	    sizeof(int),(caddr_t)&(new_value),true)) {
	   logLine("errror in read DYNAMIC[DT_DEBUG].d_ptr\n");
        }
	else {
	   // string tempstr = string("new value of d_ptr: ");
	   // tempstr += string(new_value);
	   // tempstr += ("\n");
	   // logLine(P_strdup(tempstr.string_of()));
	}
    }
    else {
	logLine("_DYNAMIC internal symbol not found\n");
	return false;
    }
    return true;
}

// getSharedObjects: This routine is called before main() to get and
// process all shared objects that have been mapped into the process's
// address space
// this routine reads the link maps from the application process
// to find the shared object file base mappings
vector<shared_object *> *dynamic_linking::getSharedObjects(process *p) {

    // get the value of _DYNAMIC[DT_DEBUG].d_un,d_ptr, if it is 1 then no
    // shared objects have been mapped
    if(link_map_addr){
	// get the address of the link maps
	Address link_maps;

	if(!p->readDataSpace((caddr_t)(link_map_addr),
	    sizeof(int),(caddr_t)&(link_maps),true)) {
	    logLine("read link_map_addr failed\n");
            return 0;
        }

	// read the r_debug struct 
	if(link_maps != 1){
	    r_debug debug_elm;
	    if(!p->readDataSpace((caddr_t)(link_maps),
	        sizeof(r_debug),(caddr_t)&(debug_elm),true)) {
	        logLine("read link_map_addr failed\n");
                return 0;
            }

	    // get each link_map object
	    bool first_time = true;
	    Link_map *next_link_map = debug_elm.r_map;
	    Address next_addr = (Address)next_link_map; 
	    vector<shared_object*> *shared_objects = new vector<shared_object*>;
	    while(next_addr != 0){
		Link_map link_elm;
	        if(!p->readDataSpace((caddr_t)(next_addr),
	            sizeof(Link_map),(caddr_t)&(link_elm),true)) {
	            logLine("read next_link_map failed\n");
		    return 0;
                }
		// get file name
		char f_name[256]; // assume no file names greater than 256 chars
		// check to see if reading 256 chars will go out of bounds
		// of data segment
		u_int f_amount = 256;
                bool done = false;
		for(u_int i=0; (i<256) && (!done); i++){
                    if(!p->readDataSpace((caddr_t)((u_int)(link_elm.l_name)+i),
			sizeof(char),(caddr_t)(&(f_name[i])),true)){
                    }
		    if(f_name[i] == '\0'){
			done = true;
			f_amount = i+1;
		    }
		}
                f_name[f_amount-1] = '\0';

                //string temp = string("done reading file: ");
		//temp += f_name;
		//temp += string(" address ");
		//temp += string(u_int(link_elm.l_name));
		//temp += string(" size ");
		//temp += string(f_amount);
		//temp += string("\n");
	        //logLine(P_strdup(temp.string_of()));
    
		string obj_name = string(f_name);
		// create a shared_object and add it to the list
		// kludge: ignore the entry if it has the same name as the
		// executable file...this seems to be the first link-map entry
		if((obj_name != p->getImage()->file()) && 
		   (obj_name != p->getImage()->name()) ){
		   // kludge for when an exec occurs...the first element
		   // in the link maps is the file name of the parent process
		   // so in this case, we ignore the first entry
		   if((!(p->wasExeced())) || (p->wasExeced() && !first_time)){ 
		        //string blah = string("adding so: ");
		        //blah += f_name;
		        //blah += string("\n");
		        //printf("%s",blah.string_of());
		        //logLine(P_strdup(blah.string_of()));
                        shared_object *newobj = new shared_object(obj_name,
				link_elm.l_addr,false,true,true,0);
		        *shared_objects += newobj;
		    }
		}
		first_time = false;
		next_addr = (u_int)link_elm.l_next;
	    }
	    return shared_objects;
	    shared_objects = 0;
	}
    }
    // else this a.out does not have a .dynamic section
    return 0;
}
