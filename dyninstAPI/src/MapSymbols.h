/*
 * Copyright (c) 1998 Barton P. Miller
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


/* 
 * $Id: MapSymbols.h,v 1.2 2003/07/18 15:43:49 schendel Exp $
 */

//MapSymbols.h
//This file contains the definition of MapSymbols and
//necessary aux classes.  MapSymbols is used to read the
//.MAP file produced with the /MAP option on the Microsoft
//Compiler.  The user is provided with a list of debug
//symbols (name, address, function/variable?).
//7 july 2000 -- ccw 


#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <iostream>
#include <stdlib.h>
#include "common/h/Vector.h"
#include "common/h/Symbol.h"

#define text_seg 1
#define idata_seg 2
#define data_seg 3
#define bss_seg 4
#define rdata_seg 5
#define other_seg 100

class _symbols {
public:
	char *name;
	unsigned long address;
	bool function;
	unsigned long length;
	bool staticObj;

	_symbols() :name(NULL) {	};

	_symbols(char *aName, unsigned long aAddress, bool aFunction, unsigned long aLength=0, bool staticN=false) : address(aAddress), 
		function(aFunction), length(aLength), staticObj(staticN){
		name = new char[strlen(aName)+1];
		strcpy(name, aName);
	}

	_symbols(_symbols &obj): address(obj.address), 
		function(obj.function), length(obj.length), staticObj(obj.staticObj){
		name = new char[strlen(obj.name)+1];
		strcpy(name, obj.name);
	}

	~_symbols(){ delete [] name; }

};

class _dllName {
public:
		char *name;
	
		_dllName() :name(NULL) {}
		_dllName(char *aName){
			name = new char[strlen(aName)+1];
			strcpy(name, aName);
		}

		_dllName(_dllName &obj){
			name = new char[strlen(obj.name)+1];
			strcpy(name, obj.name);
		}

		~_dllName() { delete [] name; }

		bool operator== (const _dllName &obj) const {
			return !strcmp(obj.name, name);
		}
		
};

struct _list {
	void *data;
	struct _list *next;
};

class _section {
public:
	char name[50];
	int startSeg;
	unsigned long startOffset;
	unsigned long length;
	char type[10];

	_section(){};

	_section(char *aName, int aStartSeg, unsigned long aStartOffset, unsigned long aLength, char *aType) 
		: startSeg(aStartSeg), startOffset(aStartOffset), length(aLength){
		strcpy(name, aName);
		strcpy(type, aType);
	}

	_section(_section&obj):startSeg(obj.startSeg), startOffset(obj.startOffset), length(obj.length){
		strcpy(name, obj.name);
		strcpy(type, obj.type);
	}

	~_section(){ }
};

class MapSymbols  
{
private:
	FILE *fp;
	char *filename;
	struct _list *headSymbols;
	struct _list *currSymbols;

	struct _list *headSection;
	struct _list *currSection;

	struct _list *headDLL;
	struct _list *currDLL;


	int numberOfDLL;

	unsigned long preferredLoadAddr;

	bool addSymbol(char* name, unsigned int address, bool function, unsigned long aLength=0, bool staticN= false);
	bool addSection(char *aName, int aStartSeg, unsigned long aStartOffset, unsigned long aLength, char *aType);
	bool addDLL(char *aName);

	int findSegment(int seg, unsigned long offset);
	bool getFirstSection(char *name, int *segment, unsigned long *offset, unsigned long *len);
	bool getNextSection(char *name, int *segment, unsigned long *offset, unsigned long *len);
	void printSymbols(); //debug only!

public:
	MapSymbols();
	MapSymbols(char *fname);
	virtual ~MapSymbols();

	bool setFilename(char *fname);
	bool parseSymbols();
	unsigned long getPreferredLoadAddress();
	bool getFirstSymbol(char *name, unsigned long *address, bool *function, unsigned long *len, bool *staticN);
	bool getNextSymbol(char*name, unsigned long *address, bool *function, unsigned long *len, bool *staticN);

	bool getFirstDLL(char *name);
	bool getNextDLL(char*name);

	int getNumberOfDLL();


};

