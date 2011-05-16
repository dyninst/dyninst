/*
 * Copyright (c) 1996-2011 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */


/* 
 * $Id: MapSymbols.h,v 1.7 2007/09/19 21:54:34 giri Exp $
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
#define _WINSOCKAPI_
#include <windows.h>
#include <iostream>
#include <stdlib.h>
#include "common/h/Vector.h"
#include "symtabAPI/h/Symbol.h"

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

