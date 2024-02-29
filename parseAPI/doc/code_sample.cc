/*
   Copyright (C) 2015 Alin Mindroc
   (mindroc dot alin at gmail dot com)

   This is a sample program that shows how to use InstructionAPI in order to
   6  print the assembly code and functions in a provided binary.


   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   11  License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.
   */
#include <iostream>
#include "CodeObject.h"
#include "InstructionDecoder.h"
using namespace std;
using namespace Dyninst;
using namespace ParseAPI;

using namespace InstructionAPI;
int main(int argc, char **argv){
    if(argc != 2){
	printf("Usage: %s <binary path>\n", argv[0]);
	return -1;
    }
    char *binaryPath = argv[1];

    SymtabCodeSource *sts;
    CodeObject *co;
    Instruction::Ptr instr;
    SymtabAPI::Symtab *symTab;
    std::string binaryPathStr(binaryPath);
    bool isParsable = SymtabAPI::Symtab::openFile(symTab, binaryPathStr);
    if(isParsable == false){
	const char *error = "error: file can not be parsed";
	cout << error;
	return - 1;
    }
    sts = new SymtabCodeSource(binaryPath);
    co = new CodeObject(sts);
    //parse the binary given as a command line arg
    co->parse();

    //get list of all functions in the binary
    const CodeObject::funclist &all = co->funcs();
    if(all.size() == 0){
	const char *error = "error: no functions in file";
	cout << error;
	return - 1;
    }
    auto fit = all.begin();
    Function *f = *fit;
    //create an Instruction decoder which will convert the binary opcodes to strings
    InstructionDecoder decoder(f->isrc()->getPtrToInstruction(f->addr()),
	    InstructionDecoder::maxInstructionLength,
	    f->region()->getArch());
    for(;fit != all.end(); ++fit){
	Function *f = *fit;
	//get address of entry point for current function

	Address crtAddr = f->addr();
	int instr_count = 0;
	instr = decoder.decode((unsigned char *)f->isrc()->getPtrToInstruction(crtAddr));
	auto fbl = f->blocks().end();
	fbl--;
	Block *b = *fbl;
	Address lastAddr = b->last();
	//if current function has zero instructions, don't output it
	if(crtAddr == lastAddr)
	    continue;
	cout << "\n\n\"" << f->name() << "\" :";
	while(crtAddr < lastAddr){
	    //decode current instruction
	    instr = decoder.decode((unsigned char *)f->isrc()->getPtrToInstruction(crtAddr));
	    cout << "\n" << hex << crtAddr;
	    cout << ": \"" << instr->format() << "\"";
	    //go to the address of the next instruction
	    crtAddr += instr->size();
	    instr_count++;
	}
    }
    return 0;
}
