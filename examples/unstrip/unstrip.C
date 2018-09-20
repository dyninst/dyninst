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
 * A simple tool to perform library fingerprinting on a stripped binary.
 *
 * This tool uses parseAPI and symtabAPI to locate library wrapper
 * functions, identify them using a descriptor database, and output a
 * new binary with these new labels added to the symbtol table.
 */


#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>

#include <fstream>
#include <set>
#include <string>
#include <vector>

// local
#include "util.h"
#include "callback.h"
#include "types.h"
#include "predicates.h"
#include "semanticDescriptor.h"
#include "database.h"
#include "fingerprint.h"

// parseAPI
#include "CodeSource.h"
#include "CodeObject.h"
#include "CFG.h"

// symtabAPI
#include "Function.h"

// instructionAPI
#include "InstructionDecoder.h"
#include "Register.h"
#include "Dereference.h"
#include "Immediate.h"
#include "Result.h"

// dataflowAPI
#include "SymEval.h"
#include "slicing.h"

using namespace std;
using namespace Dyninst;

/* Globals */
Mode mode = (Mode)NULL;
string relPath;
char * prefix = NULL;
bool oneSymbol = false;
bool verbose = false;

/* Options */
char * OUT_FILE = NULL;
char * BIN_FILE = NULL;
int MAXLEN = 1024;

void usage(char* s) {
    printf("Usage: %s\n"
            "\t\t-f <binary>\n"
            "\t\t-o <output file> [optional]\n"
            "\t\t-l [learning mode, optional]\n"
            "\t\t-s [provide a single symbol per function, optional]\n",s);
}
//"\t\t--prefix str [optional]\n"

void parse_options(int argc, char** argv)
{
    /* Set the relative path */
    string tmp(argv[0]);
    size_t skip = tmp.rfind("/");
    if (skip != string::npos) {
        relPath = string(argv[0]);
        relPath = relPath.substr(0, skip);
        relPath = relPath.append("/");
    }

    int ch;

    static struct option long_options[] = {
        {"help",no_argument,0,'h' },
        {"prefix",required_argument,0,'p' }
    };

    int option_index = 0;

    while((ch=
        getopt_long(argc,argv,"f:o:hlsv",long_options,&option_index)) != -1)
    {
        switch(ch) {
            case 'f':
                BIN_FILE = optarg;
                break;
            case 'o':
                OUT_FILE = optarg;
                break;
            case 'p':
                prefix = optarg;
                printf("Setting prefix to %s\n", prefix);
                break;
            default:
                printf("Illegal option %c\n",ch);
            case 'h':
                usage(argv[0]);
                exit(1);
            case 'l':
                mode = _learn;
                break;
            case 's':
                oneSymbol = true;
                break;
            case 'v':
                verbose = true;
                break;
        }
    }

    if(!BIN_FILE) {
        usage(argv[0]);
        exit(1);
    }

    if (!mode) mode = _identify;
}

int main(int argc, char **argv)
{
    ParseAPI::SymtabCodeSource *sts;
    ParseAPI::CodeObject *co;
    InstrCallback * cb;
    struct stat sbuf;

    parse_options(argc,argv);
    
    if(0 != stat(BIN_FILE,&sbuf)) {
        fprintf(stderr,"Failed to stat %s: ",argv[1]);
        perror("");
        exit(1);
    }

    /* Open the specified binary */
    SymtabAPI::Symtab * symtab = NULL;
    if(!SymtabAPI::Symtab::openFile(symtab,BIN_FILE)) {
        fprintf(stderr,"couldn't open %s\n",BIN_FILE);
        exit(1);
    }

    /* Set up library fingerprinting structures */
    Database db;
    if (!db.setup(symtab, mode, relPath)) {
        cerr << "Database setup failed" << endl;
    }
    
    /* Create a new fingerprint generating object */
    Fingerprint * fingerprint;
    fingerprint = new Fingerprint(db, mode, relPath, oneSymbol, verbose);

    SymtabAPI::Module * defmod = NULL;
    if(!symtab->findModuleByName(defmod,BIN_FILE)) {
        fprintf(stderr,"can't find default module\n");
        exit(1);
    }
   
    /* Trigger parsing of the binary */ 
    sts = new ParseAPI::SymtabCodeSource(symtab);
    cb = new InstrCallback(db.syscallTrampStore, fingerprint);
    co = new ParseAPI::CodeObject(sts, NULL, cb);
    co->parse();

    /* Locate main */
    fingerprint->findMain(symtab, sts, defmod);
    
    /* Trigger gap parsing */
    vector<ParseAPI::CodeRegion*> const& regs = sts->regions();
    for(unsigned i=0;i<regs.size();++i) {
        co->parseGaps(regs[i]);
    }
  
    /* Retrieve functions from the parsed binary */ 
    const ParseAPI::CodeObject::funclist & all = co->funcs();
    ParseAPI::CodeObject::funclist::const_iterator fit = all.begin();

    for( ; fit != all.end(); ++fit) {
        ParseAPI::Function * f = *fit;

        vector<ParseAPI::FuncExtent *> const& exts = f->extents();
        size_t size = exts.back()->end() - f->addr();

        if(f->src() != ParseAPI::HINT && sts->linkage().find(f->addr()) == sts->linkage().end()) {
            symtab->createFunction(f->name(),f->addr(),size,defmod);
        }
    
    }

    /* Fingerprint wrapper functions */
    fingerprint->run(symtab);
   
    if (mode != _learn) {
        string outfile;
        if(OUT_FILE) 
            outfile = OUT_FILE;
        else 
            outfile = "foo.bin";
        symtab->emit(outfile);
    }
    
    return 0;
}
