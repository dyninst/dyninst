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

#include <stdio.h>
#include <errno.h>

#include <vector>

#include "util.h"
#include "types.h"
#include "semanticDescriptor.h"
#include "database.h"

using namespace std;

string descriptorFileName = "ddb.db";
string paramsFileName = "params.db";
string unistdFileName = "unistd.db";

/* Build semantic descriptor database. */
bool DescriptorDatabase::build()
{
    /* Open the descriptor database file */
    FILE * descriptorFile;
    if ( (descriptorFile = fopen(descriptorFileName.c_str(), "r")) == NULL) {
        cerr << "Could not locate pattern database: " << descriptorFileName << endl;   
        return false;
    }

    int MAXLEN = 1024;

    char cur[MAXLEN];
    const char * delim = ";";
    const char * endDelim = "||";
    char cur2[MAXLEN];
    vector<char*> trapSets;
    while (!feof(descriptorFile)) {
        SemanticDescriptor id;
        
        /* Parse the current line */
        if (fgets(cur, MAXLEN, descriptorFile) == NULL) break;
        while (strstr(cur,endDelim) == NULL) {
            fgets(cur2, MAXLEN, descriptorFile);
            strcat(cur,cur2);
        }

        char * line = strtok(cur, endDelim);
        char * name = strtok(line, delim);
        string nameString(name);
        char * trapVector = strtok(NULL, delim);
        while (trapVector != NULL) {
            trapSets.push_back(trapVector);
            trapVector = strtok(NULL, delim);
        }

        /* Make the set of system calls */
        vector<char*>::iterator trapSetIter;
        for (trapSetIter = trapSets.begin();
                trapSetIter != trapSets.end();
                ++trapSetIter) {
            SemanticDescriptorElem curTraps = process(*trapSetIter);
            id.insert(curTraps);
        }

        /* Add id pattern to our database */
        db.insert(make_pair(id,nameString));
        trapSets.clear();
    }

    /* Close file */
    fclose(descriptorFile);
    
    return true;
}

/* 
 * Build syscall paramaters types database
 */
bool SyscallParamDatabase::build()
{
    /* Open the system call parameter info file */
    FILE * paramFile;
    if ( (paramFile = fopen(paramsFileName.c_str(), "r")) == NULL) {
        cerr << "Could not open param info file: " << paramsFileName << endl;   
        return false;
    }

    char buf[128];
    const char * delimComma = ",";
    const char * delimSpace = " ";

    vector<ParamType> params;

    while (!feof(paramFile)) {
        if (fgets(buf, 128, paramFile) == NULL) break;

        /* Build parameter information */
        char * sys = strtok(buf, delimComma);

        /* Always store the system call name as a string */
        params.push_back(_s);

        /* Parameters can have different types */
        char * paramString = strtok(NULL, delimComma);
        int numParams = 0;
        char * curParam = strtok(paramString, delimSpace);
        while (curParam != NULL) {
            numParams++;
            ParamType pType = getParamType(curParam);
            if (pType != _u) params.push_back(pType);
            curParam = strtok(NULL, delimSpace);
        }
        
        /* Insert syscall name and params into map */
        string sysNameString(sys);
        db.insert(make_pair(sysNameString, params));
        params.clear();
    }

    /* Close file */
    fclose(paramFile);
    
    return true;
}

/* 
 * Build syscall name -> number database
 */
bool SyscallNumbersDatabase::build()
{
    /* Open the system call numbers info file */
    FILE * unistdFile;
    if ( (unistdFile = fopen(unistdFileName.c_str(), "r")) == NULL) {
        cerr << "Could not open unistd file: " << unistdFileName << endl;   
        return false;
    }

    const char * header = "_ASM_I386_UNISTD_H";
    const char * define = "#define ";
    string skip = "__NR_";
    int syscallNumber;

    char buf[128];
    while (!feof(unistdFile)) {
        if (fgets(buf, 128, unistdFile) == NULL) break;

        /* Skip lines that don't define syscalls */
        if (strstr(buf, define) == NULL) continue;

        /* Skip the #define _H line */
        if (strstr(buf, header) != NULL) continue;

        string curLine(buf);

        /* Skip #define __NR_  */
        size_t toSkip = curLine.rfind(skip);
        curLine = curLine.substr(toSkip+5);

        /* Grab the syscall name */
        toSkip = curLine.find(" ");
        if (toSkip == string::npos) toSkip = curLine.find("\t");
        string syscallName = curLine.substr(0, toSkip);
        toSkip = syscallName.find("\t");
        if (toSkip != string::npos) syscallName = curLine.substr(0,toSkip);

        /* Grab the syscall number */
        toSkip = curLine.rfind(" ");
        if (toSkip == string::npos) toSkip = curLine.rfind("\t");
        string syscallNumberString = curLine.substr(toSkip);
        sscanf(syscallNumberString.c_str(), "%d", &syscallNumber);

        /* Insert syscall number & name pair into the map */
        db.insert(make_pair(syscallNumber, syscallName));
    }

    /* Close the file */
    fclose(unistdFile);

    return true;
}

/* 
 * Convert file representation of database entry into our database
 */
SemanticDescriptorElem DescriptorDatabase::process(char * _trapVector)
{
    char * trapVector = (char*)malloc(sizeof(char)*(strlen(_trapVector)+1));
    assert(trapVector);
    strncpy(trapVector, _trapVector, strlen(_trapVector)+1);
    SemanticDescriptorElem values;
    const char * delim = ",";

    /* Store system call name*/
    char * val = strtok(trapVector, delim);
    char * curVal = (char*)malloc(sizeof(char)*(strlen(val)+1));
    assert(curVal);
    strncpy(curVal, val, strlen(val)+1);
    values.push_back((void*)curVal);
    string curValString(curVal);

    /* Parse and store remaining param values */
    map<string, vector<ParamType> >::iterator pIter = spDB->find(curValString);
    vector<ParamType> params;
    if (pIter != spDB->end()) params = pIter->second;
    else return values;

    if (params.size() < 2) return values;
    else {
        int pos = 1;
        val = strtok(NULL, delim);
        while (val != NULL) {
            if (params[pos] == _i || params[pos] == _p || params[pos] == _o) {
                errno = 0;
                long int curVal = strtol(val, NULL, 10);
                if (errno != 0) //cerr << "strtol error" << endl;
                values.push_back((void*)curVal);
            } else if (params[pos] == _s) {
                char * newCurVal = (char*)malloc(sizeof(char)*(strlen(val)+1));
                assert(newCurVal);
                strncpy(newCurVal, val, strlen(val)+1);
                values.push_back((void*)newCurVal);
            } else {
                values.push_back((void*)val);
            }
            // Grab the next value
            val = strtok(NULL, delim);
            pos++;
        }
    }
    return values;
}

/* 
 * Setup the data structures we'll use during library fingerprinting
 */
bool Database::setup(SymtabAPI::Symtab * symtab,
        Mode mode,
        string relPath)
{
    bool ret = true;

    /* Update database paths based on relative path */
    if (relPath.length()) {
        descriptorFileName.insert(0, relPath);
        paramsFileName.insert(0, relPath);
        unistdFileName.insert(0, relPath);
    }

    /* Read system call parameter information and system call number
     * information */
    if (!spDB.build()) return false; 
    if (!snDB.build()) return false;

    /* Read the descriptor database */
    if (mode != _learn) {
        dDB = DescriptorDatabase(&(spDB));
        if (!dDB.build()) {
            cerr << "Building the descriptor database failed" << endl;   
        }
    }

    /* Locate the address through which indirect system calls are made */
    syscallTrampStore = getSyscallTrampStore(symtab);

    return ret;

}
