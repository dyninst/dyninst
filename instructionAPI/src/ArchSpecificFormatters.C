//
// Created by ssunny on 10/17/16.
//

#include "ArchSpecificFormatters.h"
#include <algorithm>
#include <sstream>
#include <iostream>

#include <stdio.h>
#include <string.h>
#include <assert.h>

using namespace Dyninst::InstructionAPI;

///////// Base Formatter

std::string ArchSpecificFormatter::formatBinaryFunc(std::string left, std::string func, std::string right) {
    // if(isAdd())
    // {
    return left + " " + func + " " + right;
    // } else retVal << "NOT VALID FOR AT&T";
}

///////////////////////////

///////// Formatter for ARMv-8A

ArmFormatter::ArmFormatter() {
    binaryFuncModifier["<<"] = "lsl";
}

std::string ArmFormatter::formatImmediate(std::string evalString) {
    return "0x" + evalString;
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
    size_t pluspos = addrString.find("+");

    if(pluspos != std::string::npos && addrString.substr(0, pluspos - 1) == "PC") {
        out<<addrString.substr(pluspos + 2);
    } else {
        std::string left = addrString.substr(0, pluspos - 1);
        std::string right = addrString.substr(pluspos + 2);
        out<<"["<<left<<", "<<right<<"]";
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

std::string ArmFormatter::formatBinaryFunc(std::string left, std::string func, std::string right) {
    if(binaryFuncModifier.count(func) > 0)
	    return left + ", " + binaryFuncModifier[func] + " " + right;
    else if(left == "PC")
	    return right;
    else
        return left + " " + func + " " + right;
}

/////////////////////////// x86 Formatter functions

x86Formatter::x86Formatter()
{

}

std::string x86Formatter::formatImmediate(std::string evalString) 
{
	return "$0x" + evalString;
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

    return ss.str();
}

std::string x86Formatter::formatDeref(std::string addrString) 
{
    // fprintf(stderr, "Must format dereference: %s\n", addrString.c_str());

    /* If the address string isn't formatted, we have to add parentheses. */
    if(!addrString.compare(0, 1, "%"))
        return "(" + addrString + ")";
    else return addrString;
}

std::string x86Formatter::getInstructionString(std::vector<std::string> operands) 
{
    // fprintf(stderr, "Operands: ");
    // for(auto iter = operands.begin(); iter != operands.end(); iter++)
    // {
        // if(iter == operands.begin())
            // fprintf(stderr, "%s", (*iter).c_str());
        // else fprintf(stderr, ", %s", (*iter).c_str());
    // }

    /* We have to reorder the operands here */
    std::string source_ops = "";
    std::string dest_op = "";
    std::string kmask_op = "";

    /**
     * We have to convert the Intel syntax operand ordering to AT&T because
     * our tables are in Intel ordering
     */
    for(auto itr = operands.begin(); itr != operands.end(); itr++) 
    {
        std::string op = *itr;
        if(itr == operands.begin())
        {
            dest_op = op;
        } else if(!op.compare(0, 2, "%k"))
        {
            kmask_op = "{" + op + "}";
        } else if(!source_ops.compare(""))
        {
            source_ops = op;
        } else {
            source_ops += "," + op;
        }
    }

    /* Put the instruction together */
    std::stringstream ss;

    ss << source_ops;
    if(ss.str().compare(""))
        ss << ",";
    ss << dest_op;
    ss << kmask_op;

    /* Return the formatted instruction */
    return ss.str();
}

std::string x86Formatter::formatBinaryFunc(std::string left, std::string func, std::string right)
{
    // fprintf(stderr, "left: %s  func: %s  right: %s\n", left.c_str(), func.c_str(), right.c_str());

    /**
     * Prefixing a retval with ## deonstrates that additional processing
     * needs to be performed on that operand before it is complete. This
     * is only used for complex derefereces.
     */
    std::string retval;
    if(func == "+")
    {
        if(!right.compare(0, 2, "##"))
        {
            if(!left.compare(0, 1, "%"))
            {
                /* This should be 2 right pieces to a dereference */
                retval = "##" + left + "," + right.substr(2);
            } else if(!left.compare(0, 1, "$"))
            {
                /* This occurs when a base register isn't used e.g. 0x1(,%eax,1)*/
                retval = left.substr(1) + "(," + right.substr(2) + ")";
            } else {
                /* This should never occur, and is an internal dyninst problem */
                assert(!"AT&T x86 syntax problem.");
                return "";
            }
        } else if(!left.compare(0, 2, "##"))
        {
            /* This is when all 3 pieces have been passed */
            retval = right.substr(1) + "(" + left.substr(2) + ")";
        } else {
            /* This is the simplest for of a dereference e.g. 0x1(%eax) */
            retval = right.substr(1) + "(" + left + ")";
        }
    } else if(func == "*")
    {
        retval = "##" + left + "," + right.substr(3);
    } else {
        assert(!"Unknown syntax function");
    }

    return retval;
}

///////////////////////////
