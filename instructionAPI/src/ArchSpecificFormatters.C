//
// Created by ssunny on 10/17/16.
//

#include "ArchSpecificFormatters.h"
#include <algorithm>
#include <iterator>
#include <sstream>
#include <iostream>

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <boost/algorithm/string/join.hpp>
#include <boost/shared_ptr.hpp>
#include "../../common/h/compiler_diagnostics.h"
#include "Architecture.h"
#include "registers/AMDGPU/amdgpu_gfx908_regs.h"
#include "registers/AMDGPU/amdgpu_gfx90a_regs.h"
#include "registers/AMDGPU/amdgpu_gfx940_regs.h"


using namespace Dyninst::InstructionAPI;

///////// Base Formatter

std::string ArchSpecificFormatter::getInstructionString(const std::vector<std::string> &operands) const
{
    // non-x86_64 operand formatter:  join non-empty operands strings with ", "
    return boost::algorithm::join_if(operands, ", ", [](const std::string &op){return !op.empty();});
}

std::string ArchSpecificFormatter::formatBinaryFunc(const std::string &left, const std::string &func, const std::string &right) const  {
    // if(isAdd())
    // {
    return left + " " + func + " " + right;
    // } else retVal << "NOT VALID FOR AT&T";
}

bool ArchSpecificFormatter::operandPrintOrderReversed() const
{
    return false;
}

///////////////////////////

///////// Formatter for PowerPC

PPCFormatter::PPCFormatter() {
}

std::string PPCFormatter::formatImmediate(const std::string &evalString) const  {
    size_t endPos;
    long long long_val = stoll(evalString, &endPos, 16);
    signed short val = static_cast<signed short>(long_val);
    return std::to_string(val);
}

std::string PPCFormatter::formatRegister(const std::string &regName) const  {
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

std::string PPCFormatter::formatDeref(const std::string &addrString) const  {
    size_t commaPos = addrString.find(",");
    if (commaPos == std::string::npos || commaPos > addrString.length() - 2) {
        return "(" + addrString + ")";
    }
    std::string base = addrString.substr(0, commaPos);
    std::string offset = addrString.substr(commaPos + 2);
    return offset + "(" + base + ")";
}

std::string PPCFormatter::formatBinaryFunc(const std::string &left, const std::string &func, const std::string &right) const  {
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

std::string ArmFormatter::formatImmediate(const std::string &evalString) const  {
    return "0x" + evalString;
}

std::string ArmFormatter::formatRegister(const std::string &regName) const  {
    std::string::size_type substr = regName.rfind(':');
    std::string ret = regName;

    if (substr != std::string::npos) {
        ret = ret.substr(substr + 1, ret.length());
    }
    for(char &c : ret) c = std::toupper(c);

    return ret;
}

std::string ArmFormatter::formatDeref(const std::string &addrString) const  {
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

std::string ArmFormatter::formatBinaryFunc(const std::string &left, const std::string &func, const std::string &right) const  {
    if(binaryFuncModifier.count(func) > 0)
	    return left + ", " + binaryFuncModifier.at(func) + " " + right;
    /*else if(left == "PC")
	    return right;*/
    else
        return left + " " + func + " " + right;
}

///////// Formatter for AMDGPU

AmdgpuFormatter::AmdgpuFormatter() {
    binaryFuncModifier["<<"] = "lsl";
}

std::string AmdgpuFormatter::formatImmediate(const std::string &evalString) const  {
    return "0x" + evalString;
}

std::string AmdgpuFormatter::formatRegister(const std::string &regName) const  {
    std::string ret = regName;
    for(auto &c : ret ) c = ::toupper(c);
    return ret;
}

std::string AmdgpuFormatter::formatDeref(const std::string &addrString) const  {
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

std::string AmdgpuFormatter::formatBinaryFunc(const std::string &left, const std::string &func, const std::string &right) const  {
    if(binaryFuncModifier.count(func) > 0)
	    return "("+left + ", " + binaryFuncModifier.at(func) + " " + right+")";
    /*else if(left == "PC")
	    return right;*/
    else
        return "("+left + " " + func + " " + right+")";
}

std::string AmdgpuFormatter::formatRegister(MachRegister  m_Reg, uint32_t m_num_elements, unsigned m_Low , unsigned m_High) {
    std::string name = m_Reg.name();
    std::string::size_type substr = name.rfind("::");
    if(substr != std::string::npos){
        name = name.substr(substr+2,name.length());
    }
    if( m_num_elements ==0 ){
        return "";
    }else if ( m_num_elements > 1){
        uint32_t id = m_Reg & 0xff ;
        uint32_t regClass = m_Reg.regClass();

        uint32_t  size = m_num_elements;

        DYNINST_DIAGNOSTIC_BEGIN_SUPPRESS_LOGICAL_OP

        if(regClass == amdgpu_gfx908::SGPR || regClass == amdgpu_gfx90a::SGPR || 
            regClass == amdgpu_gfx940::SGPR){
            return "S["+std::to_string(id) + ":" + std::to_string(id+size-1)+"]";
        }

        if(regClass == amdgpu_gfx908::VGPR || regClass == amdgpu_gfx90a::VGPR || 
            regClass == amdgpu_gfx940::VGPR){
            return "V["+std::to_string(id) + ":" + std::to_string(id+size-1)+"]";
        }

        if(regClass == amdgpu_gfx908::ACC_VGPR || regClass == amdgpu_gfx90a::ACC_VGPR ||
            regClass == amdgpu_gfx940::ACC_VGPR){
            return "ACC["+std::to_string(id) + ":" + std::to_string(id+size-1)+"]";
        }

        DYNINST_DIAGNOSTIC_END_SUPPRESS_LOGICAL_OP

        if(m_Reg == amdgpu_gfx908::vcc_lo || m_Reg == amdgpu_gfx90a::vcc_lo ||
            m_Reg == amdgpu_gfx940::vcc_lo)
            return "VCC";
        if(m_Reg == amdgpu_gfx908::exec_lo || m_Reg == amdgpu_gfx90a::exec_lo || 
            m_Reg == amdgpu_gfx940::exec_lo)
            return "EXEC";


    }else if ( m_High -m_Low > 32 && m_Reg.size()*8 != m_High - m_Low){

        // Size of base register * 8 != m_High - mLow ( in bits) when we it is a register vector
        uint32_t id = m_Reg & 0xff ;
        uint32_t regClass = m_Reg.regClass();
        uint32_t size = (m_High - m_Low ) / 32;

        // Suppress warning (for compilers where it is a false positive)
        // The values of the two *::SGPR constants are identical, as
        // are the two *::VGPR constants
        DYNINST_DIAGNOSTIC_BEGIN_SUPPRESS_LOGICAL_OP

        if(regClass == amdgpu_gfx908::SGPR || regClass == amdgpu_gfx90a::SGPR ||
            regClass == amdgpu_gfx940::SGPR){
            return "S["+std::to_string(id) + ":" + std::to_string(id+size-1)+"]";
        }

        if(regClass == amdgpu_gfx908::VGPR || regClass == amdgpu_gfx90a::VGPR || 
            regClass == amdgpu_gfx940::VGPR){
            return "V["+std::to_string(id) + ":" + std::to_string(id+size-1)+"]";
        }

        if(regClass == amdgpu_gfx908::ACC_VGPR || regClass == amdgpu_gfx90a::ACC_VGPR ||
            regClass == amdgpu_gfx940::ACC_VGPR){
            return "ACC["+std::to_string(id) + ":" + std::to_string(id+size-1)+"]";
        }

        DYNINST_DIAGNOSTIC_END_SUPPRESS_LOGICAL_OP

        if(m_Reg == amdgpu_gfx908::vcc_lo || m_Reg == amdgpu_gfx90a::vcc_lo || 
            m_Reg == amdgpu_gfx940::vcc_lo)
            return "VCC";

        if(m_Reg == amdgpu_gfx908::exec_lo || m_Reg == amdgpu_gfx90a::exec_lo || 
            m_Reg == amdgpu_gfx940::exec_lo)
            return "EXEC";

        name +=  "["+std::to_string(m_Low)+":"+std::to_string(m_High)+"]";
    }

    return name;
}


/////////////////////////// x86 Formatter functions

x86Formatter::x86Formatter()
{

}

std::string x86Formatter::formatImmediate(const std::string &evalString) const
{
	return "$0x" + evalString;
}

std::string x86Formatter::formatRegister(const std::string &regName) const
{
    std::string outReg{'%'};

    auto regNameOffset = regName.find("::");
    if (regNameOffset == std::string::npos)  {
	regNameOffset = 0;	// no "::", copy whole string
    }  else  {
	regNameOffset += 2;	// skip "::"
    }
    auto sBegin = regName.cbegin() + regNameOffset;
    auto sEnd = regName.cend();
    auto outRegInserter = std::back_inserter(outReg);
    std::transform(sBegin, sEnd, outRegInserter, [](unsigned char c){ return std::tolower(c);});

    return outReg;
}

std::string x86Formatter::formatDeref(const std::string &addrString) const
{
    // fprintf(stderr, "Must format dereference: %s\n", addrString.c_str());

    /* If the address string isn't formatted, we have to add parentheses. */
    if(!addrString.compare(0, 1, "%"))
        return "(" + addrString + ")";
    else return addrString;
}

std::string x86Formatter::getInstructionString(const std::vector<std::string> &operands) const
{
    std::string s;
    bool oneOperandAdded{false};

    // append the operands in reverse order to convert from Intel to AT&T syntax order
    for (auto i = operands.crbegin(); i != operands.crend(); ++i)  {
	if (oneOperandAdded)  {
	    s += ',';
	}
	s += *i;
	oneOperandAdded = true;
    }

    return s;
}

std::string x86Formatter::formatBinaryFunc(const std::string &left, const std::string &func, const std::string &right) const
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

bool x86Formatter::operandPrintOrderReversed() const
{
    return true;
}

///////////////////////////
ArchSpecificFormatter& ArchSpecificFormatter::getFormatter(Architecture a)
{
    static dyn_tls std::map<Dyninst::Architecture, boost::shared_ptr<ArchSpecificFormatter> > theFormatters;
    auto found = theFormatters.find(a);
    if(found != theFormatters.end()) return *found->second;
    switch(a) {
        case Arch_amdgpu_gfx908:
        case Arch_amdgpu_gfx90a:
        case Arch_amdgpu_gfx940:
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
