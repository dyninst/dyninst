//
// Created by ssunny on 10/17/16.
//

#include "ArchSpecificFormatters.h"
#include <algorithm>
#include <sstream>

using namespace Dyninst::InstructionAPI;

///////// Formatter for ARMv-8A

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

///////////////////////////