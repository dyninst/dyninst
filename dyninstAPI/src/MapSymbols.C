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


/* 
 * $Id: MapSymbols.C,v 1.2 2004/03/23 01:11:59 eli Exp $
 */

//MapSymbols.cpp
//This file contains the class methods for 
//MapSymbols, defined in MapSymbols.h
//7 july 2000 -- ccw 
//14 july 2000 -- ccw - added code to parse out the loaded dll's and 
//						put them in a public accessible list
//					  - added freeList() to dealloc items from the lists
//					  - added addDLL(), getFirstDLL(), getNextDLL()



#include "MapSymbols.h"

MapSymbols::MapSymbols():fp(NULL), filename(NULL), headSymbols(NULL), currSymbols(NULL), 
headSection(NULL), currSection(NULL), headDLL(NULL), currDLL(NULL),numberOfDLL(0)
{
}

MapSymbols::MapSymbols(char *fname):fp(NULL), headSymbols(NULL), currSymbols(NULL),
headSection(NULL), currSection(NULL), headDLL(NULL), currDLL(NULL),numberOfDLL(0){
	filename = new char[strlen(fname)+1];
	strcpy(filename, fname);
}


void freeList(_list *head){
	_list *trail;
	_list *tmp;

	while(head != NULL){
		tmp = head;
		trail = NULL;
		while(tmp->next !=NULL){
			trail = tmp;
			tmp = tmp->next;
		}
		if(trail == NULL){
			//delete [] tmp->data->name;
			delete tmp->data;
			delete tmp;
			head = NULL;
		}else{
			//delete [] trail->next->data->name;
			delete trail->next->data;
			delete trail->next;
			trail->next = NULL;
		}
	}

}


MapSymbols::~MapSymbols()
{
	if(fp != NULL){
		fclose(fp);
	}


	
	freeList(headSymbols);
	freeList(headSection);
	freeList(headDLL);

	headSymbols = NULL;
	headSection = NULL;
	headDLL = NULL;
}


bool MapSymbols::setFilename(char *fname){
	filename = new char[strlen(fname)+1];
	strcpy(filename, fname);
	return true;
}


bool MapSymbols::addSymbol(char *name, unsigned int address, bool function, unsigned long length, bool staticN){

	if(headSymbols == NULL){
		headSymbols = new _list;
		currSymbols = headSymbols;
	}else{
		currSymbols->next = new _list;
		currSymbols = currSymbols->next;
	};
	currSymbols->next = NULL;
	currSymbols->data = new _symbols(name, address, function, length,staticN);
	return true;
}

bool MapSymbols::addSection(char *aName, int aStartSeg, unsigned long aStartOffset, unsigned long aLength, char *aType){
	
	if(headSection == NULL){
		headSection = new _list;
		currSection = headSection;
	}else{
		currSection->next = new _list;
		currSection = currSection->next;
	}
	currSection->next = NULL;
	currSection->data = new _section(aName, aStartSeg, aStartOffset, aLength, aType);
	return true;
}

bool MapSymbols::addDLL(char *aName){
	//this function differs from addXXX in that it makes sure aName is unique before
	//it adds it.

	_list *tmp;

	tmp = headDLL;
	while(tmp && strcmp(aName, ((_dllName*) tmp->data)->name)){
		tmp = tmp->next;
	}

	if(!tmp){
		if(headDLL == NULL){
			headDLL = new _list;
			currDLL = headDLL;
		}else{
			currDLL->next = new _list;
			currDLL = currDLL->next;
		}
		currDLL->next = NULL;
		currDLL->data = new _dllName(aName);
		numberOfDLL++;
	}
	return true;
}

int MapSymbols::findSegment(int seg, unsigned long offset){
	int retVal = other_seg;
	unsigned long segOffset, segLength;
	int segment;
	char name[1024];

	getFirstSection(name,&segment, &segOffset, &segLength);
	while(seg > segment || ( seg == segment && offset > segOffset + segLength)){
		getNextSection(name, &segment, &segOffset, &segLength);
	}

	if(!strcmp(name, ".text")){
		retVal = text_seg;
	}else if(!strcmp(name, ".data")){
		retVal = data_seg;
	}else if(!strcmp(name, ".bss")){
		retVal = bss_seg;
	}else if(strstr(name, ".idata")){
		retVal = idata_seg;
	}else if(!strcmp(name, ".rdata")){
		retVal = rdata_seg;
	}

	return retVal;
}

void MapSymbols::printSymbols(){
	char name[1024];
	unsigned long address, len;
	bool function, retVal;
	bool staticN;

	retVal = getFirstSymbol(name, &address, &function, &len, &staticN);
	while(retVal){
		printf("%s %lx %i %lx %s\n", name, address, function, len, (staticN? "LOCAL": "GLOBAL"));
		retVal = getNextSymbol(name, &address, &function, &len, &staticN);

	}
}

bool MapSymbols::parseSymbols(){
	char line[1024];
	char c[3];
	char symName[1024];
	unsigned long address;
	char module[1024];
	char bareName[1024];
	char cleanName[1024];
	char* start;
	int seg;
	unsigned long offset;
	unsigned long len;
	int segment;
	bool staticN = false;

	fp = fopen(filename,"r");
	if(!fp){
		return false;
	}

	fgets(line, 1024, fp); // module name;
	fgets(line, 1024, fp); //blank line
	fgets(line, 1024, fp); //timestamp
	fgets(line, 1024, fp); //blank line
	fscanf(fp, "%s %s %s %s %lx", &line[0], &line[100], &line[200], &line[300], &preferredLoadAddr); 
	//preferred load address
	fgets(line, 1024, fp); //newline
	fgets(line, 1024, fp); //blank line
	fgets(line, 1024, fp); //Start Length Name Class
	fgets(line, 1024, fp);

	//read section data.
	while(line[0] != 0x0a || line[1]!='\0'){
		//fgets(line, 1024, fp);
		//read section data here.
		sscanf(line,"%s %lx %c %s %s",bareName, &len, &c[0], symName, module);
		start = strchr(bareName, ':');
		start[0] = ' ';
		sscanf(bareName,"%ix", &seg);
		sscanf(start,"%lx",&offset);
		addSection(symName, seg, offset, len, module);
		fgets(line, 1024, fp);
	}

	fgets(line, 1024, fp); //blank line

	//read symbol data.
	while(true) {
		fscanf(fp, "%s %s %lx",&line[0], &symName, &address);
		if(!strcmp(line,"entry")){
			fgets(line, 1024, fp);
			fgets(line, 1024, fp);
			fgets(line, 1024, fp);
			fgets(line, 1024, fp);
			fscanf(fp, "%s %s %lx",&line[0], &symName, &address);
			staticN = true;
			//break;
		}

		//find the address for this symbol
		start = strchr(line, ':');
		if(!start){
			break;
		}
		start[0] = ' ';
		sscanf(line,"%ix", &seg);
		sscanf(start,"%lx",&offset);

		c[0] = getc(fp);
		c[1] = getc(fp);
		c[2] = getc(fp);
   		fscanf(fp,"%s", module);
		start = strchr(module, ':');
		if(!start){
			start = module;
		}
		if(strstr(start,".dll")){
			addDLL(&start[1]); // start[0] points to a ":"
		}

		fgets(line, 1024, fp); 

		//if we are in the .text segment this is a function
		//if we are in a .idataXX segment this is a variable if it has a ? in it
		  //if we are in the .idataXX segment we do not receive the actual address of the
		  //imported function or variable. just the address of the address of the function
		  //in the import table.
		  //HENCE: dont add this symbol to the list.

			// based on reviewing the debug symbols loaded and  used by dyninst,
			//anything not in the .text segment is an object (variable)
		//if we are in the .rdata, .data or .bss segment this is a variable

		start = strchr(symName, '?');
		start = start ? start : symName;
		UnDecorateSymbolName(start, bareName, 1024, UNDNAME_NAME_ONLY);

		segment = findSegment(seg, offset);
		if(segment == idata_seg && strchr(symName, '?')){
			UnDecorateSymbolName(start, cleanName, 1024, UNDNAME_COMPLETE);
			if(strchr(cleanName,'(')){
				//this is a function, it has a ( in it
				segment = other_seg;
			}
		}
		if(symName == strstr(symName, "_$$$") || symName == strstr(symName, "$$$") ){
			//branch target, ignore;
			segment = -1;
		}

		switch(segment){
		case text_seg:
			addSymbol(symName, address - preferredLoadAddr, true, 0,staticN);
			break;
		case idata_seg:
			//if(strchr(symName, '?')){
			addSymbol(symName, address- preferredLoadAddr, false, 0,staticN);
			//}
			//see note above.
			break;
		case rdata_seg:
		case data_seg:
		case bss_seg:
		case other_seg:
			addSymbol(symName, address- preferredLoadAddr, false,0, staticN);
 			break;
		default:
			segment=segment;
		}
	}

	fclose(fp);
	//printSymbols();
	//printf("--------\n");
	return true;
}

unsigned long MapSymbols::getPreferredLoadAddress(){
	return preferredLoadAddr;
}
bool MapSymbols::getFirstSymbol(char *name, unsigned long *address, bool *function, unsigned long *len, bool *staticN){
	currSymbols = headSymbols;

	return getNextSymbol(name, address, function, len, staticN);
}
bool MapSymbols::getNextSymbol(char*name, unsigned long *address, bool *function, unsigned long *len, bool *staticN){
	//name = new char[strlen( ((_symbols*) currSymbols->data)->name)+1];

 	if(currSymbols){
		strcpy(name, ((_symbols*) currSymbols->data)->name);
		*address = ((_symbols*) currSymbols->data)->address;
		*function = ((_symbols*) currSymbols->data)->function;
		*len = ((_symbols*) currSymbols->data)->length;
		*staticN = ((_symbols*) currSymbols->data)->staticObj;
		currSymbols = currSymbols ->next;
		return true;
	}
	return false;
}
bool MapSymbols::getFirstSection(char *name, int *segment, unsigned long *offset, unsigned long *len) {
	currSection = headSection;
	return getNextSection(name, segment, offset, len);
}

bool MapSymbols::getNextSection(char *name, int *segment, unsigned long *offset, unsigned long *len){

	if(currSection){
		strcpy(name, ((_section*) currSection->data)->name);
		*segment = ((_section*) currSection->data)->startSeg;
		*offset=((_section*) currSection->data)->startOffset;
		*len=((_section*) currSection->data)->length;
		currSection = currSection ->next;
		return true;
	}
	return false;
}


bool MapSymbols::getFirstDLL(char *name){
	currDLL = headDLL;
	return getNextDLL(name);
}

bool MapSymbols::getNextDLL(char*name){

	if(currDLL){
		strcpy(name, ((_dllName*) currDLL->data)->name);
		return true;
	}
	return false;
}

int MapSymbols::getNumberOfDLL(){
	return numberOfDLL;
}

