/*
   Copyright (C) 2015 Alin Mindroc
   (mindroc dot alin at gmail dot com)

   This is a sample program that shows how to use InstructionAPI in order to
   print the assembly code and functions in a provided binary.
   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.
   */
#include <iostream>
#include <fstream>
#include "CodeObject.h"
#include "InstructionDecoder.h"
using namespace Dyninst;
using namespace ParseAPI;
using namespace InstructionAPI;

int main(int argc, char **argv){
    if(argc != 3){
        printf("Usage: %s <binary path> <output path>\n", argv[0]);
        return -1;
    }
    char *binaryPath = argv[1];
    std::ofstream of(argv[2]);
    if(!of){
      std::cerr << "Cannot open output file !" << std::endl;
      return 1;
    }
    std::string binaryPathStr(binaryPath);
    SymtabAPI::Symtab *symTab = nullptr;
    bool isParsable = SymtabAPI::Symtab::openFile(symTab, binaryPathStr);
    if(isParsable == false){
        const char *error = "error: file can not be parsed";
        of << error;
        return - 1;
    }
    auto sts = new SymtabCodeSource(binaryPath);
    auto co = new CodeObject(sts);
    //parse the binary given as a command line arg
    co->parse();

    //get list of all functions in the binary
    const CodeObject::funclist &funcs = co->funcs();
    if(funcs.size() == 0){
        const char *error = "error: no functions in file";
        of << error;
        return - 1;
    }
    for(auto f : funcs){
        //create an Instruction decoder which will convert the binary opcodes to strings
        InstructionDecoder decoder(f->isrc()->getPtrToInstruction(f->addr()),
                InstructionDecoder::maxInstructionLength,
                f->region()->getArch());
        //get address of entry point for current function
        of << "Parsing function " << f->name() << " at addreess 0x" << std::hex << f->addr() << std::endl;
        for(auto b : f->blocks()){
            of << "Parsing block ( " << std::hex << b->start() << "," << b->end() << ")" << std::endl;
            for (auto & edges : b->targets() ){
                Address branch_addr = edges->src()->lastInsnAddr();

                void * insn_ptr = f->isrc()->getPtrToInstruction(branch_addr);
                auto instr = decoder.decode((unsigned char * ) insn_ptr);
                std::string instr_str = instr.format(branch_addr);
                if( edges->type() == COND_TAKEN || edges->type() == DIRECT || edges->type() == CALL || edges->type() == INDIRECT){
                    std::locale loc;
                    of << std::hex << branch_addr << " : ";
                    for (std::string::size_type i =0 ; i < instr_str.length(); i++){
                        of << std::tolower(instr_str[i],loc) ;
                    }
                    of << " ( " << edges->trg()->start() << " ) " ;
                    of << std::endl;
                }
            }
        }
        //of << std::endl;
    }
    of.close();
    return 0;
}
