
#include <iostream>
#include <random>

#include "amdisa/isa_explorer.h"

// Constants.
// Error constant string.
static const char* kStrErrorInvalidArgument = "Error: Requires one program argument.";

void print_instruction(const amdisa::explorer::Instruction& instruction)

{
    std::cout << "Name: " << instruction.Name() << std::endl;
    std::cout << "\tDescription: " << instruction.Description() << std::endl;
    std::cout << "\tIsBranch ?: " << instruction.IsBranch() << std::endl;
    std::cout << "\tIsConditionalBranch ?: " << instruction.IsConditionalBranch() << std::endl;
    std::cout << "\tIsIndirectBranch ?: " << instruction.IsIndirectBranch() << std::endl;
    std::cout << "\tFunctionalGroup: " << instruction.FuncGroup()->Name() << std::endl;
    std::cout << "\t\t Description: " << instruction.FuncGroup()->Description() << std::endl << std::endl;
    std::cout << "\tFunctionalSubgroup: " << instruction.FuncSubgroup()->Name() << std::endl;
}

int main(int argc, char** argv)
{
    // Check for valid Input Argument.
    if (argc != 2)
    {
        std::cerr << kStrErrorInvalidArgument << std::endl;
        return -1;
    }

    const std::string kPathToSpec = argv[1];

    amdisa::explorer::Handle explorer(kPathToSpec);

    // lookup an instruction by name
    auto v_mov_b32 = explorer.Instructions().at("V_MOV_B32");
    print_instruction(v_mov_b32);

    // choose a random instruction and print it
    std::random_device                 rd;
    std::mt19937                       gen(rd());
    std::uniform_int_distribution<int> dist(0, explorer.Instructions().size() - 1);

    auto it = explorer.Instructions().begin();
    std::advance(it, dist(gen));
    print_instruction(it->second);

    return 0;
}
