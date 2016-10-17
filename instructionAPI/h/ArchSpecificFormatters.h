//
// Created by ssunny on 10/16/16.
//

#ifndef DYNINST_ARCHSPECIFICFORMATTERS_H
#define DYNINST_ARCHSPECIFICFORMATTERS_H

#include <vector>
#include <string>

namespace Dyninst {
    namespace InstructionAPI {

        class ArchSpecificFormatter {
        public:
            virtual std::string getInstructionString(std::vector <std::string>) = 0;

            virtual std::string formatImmediate(std::string) = 0;

            virtual std::string formatDeref(std::string) = 0;

            virtual std::string formatRegister(std::string) = 0;
        };

        class ArmFormatter : public ArchSpecificFormatter {
        public:
            virtual std::string getInstructionString(std::vector <std::string>);

            virtual std::string formatImmediate(std::string);

            virtual std::string formatDeref(std::string);

            virtual std::string formatRegister(std::string);
        };
    };
};

#endif //DYNINST_ARCHSPECIFICFORMATTERS_H
