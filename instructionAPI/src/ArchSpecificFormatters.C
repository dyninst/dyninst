//
// Created by ssunny on 10/17/16.
//

#include "ArchSpecificFormatters.h"
#include <algorithm>
#include <sstream>
#include <iostream>

#include <stdio.h>
#include <string.h>

using namespace Dyninst::InstructionAPI;

///////// Formatter for ARMv-8A

std::string ArmFormatter::formatImmediate(std::string evalString) {
    return evalString;
}

std::string ArmFormatter::formatRegister(std::string regName) {
    std::string::size_type substr = regName.rfind(':');
    std::string ret = regName;

    if (substr != std::string::npos) {
        ret = ret.substr(substr + 1, ret.length());
    }
    std::transform(ret.begin(), ret.end(), ret.begin(), ::toupper);
    return ret;
}

std::string ArmFormatter::formatDeref(std::string addrString) {
    std::stringstream out;
    size_t pluspos;

    if((pluspos = addrString.find("+")) != std::string::npos) {
        std::string lhs = addrString.substr(0, pluspos - 1);
        if(lhs == "PC")
            out<<addrString.substr(pluspos + 2);
        else
            out<<"["<<addrString<<"]";
    } else {
        out<<"["<<addrString<<"]";
    }

    return out.str();
}

std::string ArmFormatter::getInstructionString(std::vector<std::string> operands) {
    std::stringstream out;

    for(std::vector<std::string>::iterator itr = operands.begin(); itr != operands.end(); itr++) {
        out<<*itr;
        if(itr != operands.end() - 1)
            out<<", ";
    }

    return out.str();
}

/////////////////////////// x86 Formatter functions

std::string x86Formatter::formatImmediate(std::string evalString) 
{
	return evalString;
}

std::string x86Formatter::formatRegister(std::string regName) 
{
    std::transform(regName.begin(), regName.end(), regName.begin(), ::tolower);

    char* orig = strdup(regName.c_str());

    /* Get rid of the Arch*:: prefix */
    int ccount = 0;
    char* pointer = orig;
    while(ccount < 2)
    {
        if(!*pointer) break;

        if(*pointer == ':')
            ccount++;
        pointer++;
    }

    /* convert to a standard string */
    regName = pointer;
    free(orig);

    std::stringstream ss;
    ss << "%" << regName;
    regName = ss.str();

    // if(!regName.compare(1, 1, "k"))
    // {
        // std::stringstream kss;
        // kss << "{" << regName << "}" ;
        // regName = kss.str();
    // }

    fprintf(stderr, "register name: %s\n", regName.c_str());

    return ss.str();
}

std::string x86Formatter::formatDeref(std::string addrString) 
{
    fprintf(stderr, "Must format dereference: %s\n", addrString.c_str());
    return "";
}

std::string x86Formatter::getInstructionString(std::vector<std::string> operands) 
{
    /* We have to reorder the operands here */
    std::string source_ops = "";
    std::string dest_op = "";
    std::string kmask_op = "";

    /**
     * We have to convert the Intel syntax operand ordering to AT&T because
     * our tables are in Intel ordering
     */

    int op_num = 0;
    for(auto itr = operands.begin(); itr != operands.end(); itr++) 
    {
        std::string op = *itr;
        if(op_num == 0)
        {
            dest_op = op;
        } else if(op.compare(0, 2, "{k"))
        {
            kmask_op = " " + op;
        } else if(!source_ops.compare(""))
        {
            source_ops = op;
        } else {
            source_ops += ", " + op;
        }

        op_num++;
    }

    /* Put the instruction together */
    std::stringstream ss;

    ss << source_ops;
    ss << dest_op;
    ss << kmask_op;

    /* Return the formatted instruction */
    return ss.str();
}
