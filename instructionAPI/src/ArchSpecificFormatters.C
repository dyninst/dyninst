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

///////// Formatter for PowerPC

PPCFormatter::PPCFormatter() {
}

std::string PPCFormatter::formatImmediate(std::string evalString) {
    size_t endPos;
    long long long_val = stoll(evalString, &endPos, 16);
    signed short val = static_cast<signed short>(long_val);
    return std::to_string(val);
}

std::string PPCFormatter::formatRegister(std::string regName) {
    if (regName == "ppc64::pc"  || 
        regName == "ppc64::ctr" || 
        regName == "ppc64::lr"  ||
        regName == "ppc64::cr0") {

        return "";
    }
    std::string::size_type lastColon = regName.rfind(':');
    std::string ret = regName;

    if (lastColon != std::string::npos) {
        ret = ret.substr(lastColon + 1, ret.length());
    }

    return ret;
}

std::string PPCFormatter::formatDeref(std::string addrString) {
    size_t commaPos = addrString.find(",");
    if (commaPos == std::string::npos || commaPos > addrString.length() - 2) {
        return "(" + addrString + ")";
    }
    std::string base = addrString.substr(0, commaPos);
    std::string offset = addrString.substr(commaPos + 2);
    return offset + "(" + base + ")";
}

std::string PPCFormatter::getInstructionString(std::vector<std::string> operands) {
    std::string out;

    for(std::vector<std::string>::iterator itr = operands.begin(); itr != operands.end(); itr++) {
        if (*itr != "") {
            out += *itr;
            if(itr != operands.end() - 1) {
                out += ", ";
            }
        }
    }

    return out;
}

std::string PPCFormatter::formatBinaryFunc(std::string left, std::string func, std::string right) {
    if (left == "") {
        return right;
    }
    if (func == "+") {
        return left + ", " + right;
    }
    return left + " " + func + " " + right;
}

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
    for(char &c : ret) c = std::toupper(c);

    return ret;
}

std::string ArmFormatter::formatDeref(std::string addrString) {
    std::string out;
    size_t pluspos = addrString.find("+");

    if(pluspos != std::string::npos && addrString.substr(0, pluspos - 1) == "PC") {
        out += addrString.substr(pluspos + 2);
    } else if(pluspos != std::string::npos) {
        std::string left = addrString.substr(0, pluspos - 1);
        std::string right = addrString.substr(pluspos + 2);
        out += "[" + left + ", " + right + "]";
    } else {
        out += "[" + addrString + "]";
    }

    return out;
}

std::string ArmFormatter::getInstructionString(std::vector<std::string> operands) {
    std::string out;

    for(std::vector<std::string>::iterator itr = operands.begin(); itr != operands.end(); itr++) {
        out += *itr;
        if(itr != operands.end() - 1)
            out += ", ";
    }

    return out;
}

std::string ArmFormatter::formatBinaryFunc(std::string left, std::string func, std::string right) {
    if(binaryFuncModifier.count(func) > 0)
	    return left + ", " + binaryFuncModifier[func] + " " + right;
    /*else if(left == "PC")
	    return right;*/
    else
        return left + " " + func + " " + right;
}

///////// Formatter for AMDGPU

AmdgpuFormatter::AmdgpuFormatter() {
    binaryFuncModifier["<<"] = "lsl";
}

std::string AmdgpuFormatter::formatImmediate(std::string evalString) {
    return "0x" + evalString;
}

std::string AmdgpuFormatter::formatRegister(std::string regName) {
    std::string ret = regName;
    for(auto &c : ret ) c = ::toupper(c);
    return ret;
}

std::string AmdgpuFormatter::formatDeref(std::string addrString) {
    std::string out;
    size_t pluspos = addrString.find("+");

    if(pluspos != std::string::npos && addrString.substr(0, pluspos - 1) == "PC") {
        out += addrString.substr(pluspos + 2);
    } else if(pluspos != std::string::npos) {
        std::string left = addrString.substr(0, pluspos - 1);
        std::string right = addrString.substr(pluspos + 2);
        out += "[" + left + ", " + right + "]";
    } else {
        out += "[" + addrString + "]";
    }

    return out;
}

std::string AmdgpuFormatter::getInstructionString(std::vector<std::string> operands) {
    std::string out;

    for(std::vector<std::string>::iterator itr = operands.begin(); itr != operands.end(); itr++) {
        out += *itr;
        if(itr != operands.end() - 1)
            out += ", ";
    }

    return out;
}

std::string AmdgpuFormatter::formatBinaryFunc(std::string left, std::string func, std::string right) {
    if(binaryFuncModifier.count(func) > 0)
	    return "("+left + ", " + binaryFuncModifier[func] + " " + right+")";
    /*else if(left == "PC")
	    return right;*/
    else
        return "("+left + " " + func + " " + right+")";
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
    for(char &c : regName) c = std::tolower(c);

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
    std::string ss = "%" + regName;
    regName = ss;
    return ss;
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

        /* If we still have a leading ##, it's an indirect call or SIB expression */
        if(!op.compare(0, 2, "##"))
        {
            op = "0x0(" + op.substr(2) + ")";
        }

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
    std::string ret = source_ops;
    if (ret.compare("")) ret += ",";
    ret += dest_op;
    ret += kmask_op;
    return ret;
}

std::string x86Formatter::formatBinaryFunc(std::string left, std::string func, std::string right)
{
    // fprintf(stderr, "left: %s  func: %s  right: %s\n", left.c_str(), func.c_str(), right.c_str());

    /**
     * Prefixing a retval with ## demonstrates that additional processing
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
        } else if(!right.compare(0, 2, "0x")){
            /* This is most likely a call or jump. */

            /* We need to extract the first part of the binary function */
            const char* orig = right.c_str();
            char* edit = strdup(orig);
            char* edit_f = edit;
            while(*edit && *edit != '(')
                edit++;
            *edit = 0;

            /* Convert edit and left hand side to numbers */
            uintptr_t edit_l = strtoul(edit_f, NULL, 16);
            uintptr_t offset = strtoul(left.c_str() + 1, NULL, 16);
            uintptr_t result = edit_l + offset;

            std::string ss;
            char hex[20];
            snprintf(hex, 20, "%lx", result);
            ss = "0x" + std::string(hex) + "(%rip)";

            /* Free allocations */
            free(edit_f);

            /* Return the processed string */
            return ss;
            
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


    // fprintf(stderr, "Retval: %s\n", retval.c_str());
    return retval;
}

///////////////////////////
ArchSpecificFormatter& ArchSpecificFormatter::getFormatter(Architecture a)
{
    static dyn_tls std::map<Dyninst::Architecture, boost::shared_ptr<ArchSpecificFormatter> > theFormatters;
    auto found = theFormatters.find(a);
    if(found != theFormatters.end()) return *found->second;
    switch(a) {
        case Arch_amdgpu_vega:
            theFormatters[a] = boost::shared_ptr<ArchSpecificFormatter>(new AmdgpuFormatter());
            break;
        case Arch_aarch32:
        case Arch_aarch64:
            theFormatters[a] = boost::shared_ptr<ArchSpecificFormatter>(new ArmFormatter());
            break;
        case Arch_ppc32:
        case Arch_ppc64:
            theFormatters[a] = boost::shared_ptr<ArchSpecificFormatter>(new PPCFormatter());
            break;
        case Arch_x86:
        case Arch_x86_64:
        default:
            theFormatters[a] = boost::shared_ptr<ArchSpecificFormatter>(new x86Formatter());
            break;
    }
    return *theFormatters[a];
}
